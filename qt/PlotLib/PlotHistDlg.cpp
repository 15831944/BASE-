#include "PlotHistDlg.h"
#include "ui_PlotHistDlg.h"

#include "PlotHistTreeItem.h"

#include "PlotSimpleListDlg.h"
#include "DwgCmpSettingsDlg.h"

#include <QMdiSubWindow>
#include <QMenu>
#include <QDir>
#include <QProcess>
#include <QScrollBar>
#include <QFileDialog>

#include "../VProject/PlotListItemDelegate.h"
#include "../VProject/SelectColumnsDlg.h"
#include "../VProject/MainWindow.h"
#include "../VProject/SaveDialog.h"
#include "../VProject/GlobalSettings.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserPropDlg.h"

#include "../ProjectLib/ProjectData.h"

#include "../SaveLoadLib/VPImageViewer.h"

bool PlotHistDlg::mBlockUpdateSignals = false;

PlotHistDlg::PlotHistDlg(PlotData * aPlot, int aHistotyNum, bool aAutoUpdate, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotHistDlg),
    mVPImageViewer(NULL),
    mJustStarted(true), mCurNum(aHistotyNum), mCurColumn(0),
    mScrollPosVert(0), mScrollPosHoriz(0),
    mIdProject(0), mIdPlot(0),
    mPlot(aPlot)
{
    CurrentVersion = 3;
    ui->setupUi(this);

    ui->cbFullMode->setChecked(true);

    if (mPlot) {
        mIdPlot = mPlot->Id();
        mIdProject = mPlot->IdProject();
        mDeleted = mPlot->Deleted();
    }

    ui->cbAutoUpdate->setChecked(aAutoUpdate);
    InitInConstructor();
}

PlotHistDlg::PlotHistDlg(QSettings &aSettings, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotHistDlg),
    mVPImageViewer(NULL),
    mJustStarted(true),
    mIdProject(0), mIdPlot(0)
{
    CurrentVersion = 4;
    ui->setupUi(this);

    mIdPlot = aSettings.value("Document ID", 0).toInt();
    if (mPlot = gProjects->FindByIdPlot(mIdPlot)) {
        mIdProject = mPlot->IdProject();
    } else {
        mIdPlot = 0; // plot was not found, maybe it is deleted since last run or it is other database
    }

    mCurNum = aSettings.value("Current item", 0).toInt();
    mCurColumn = aSettings.value("Current column", 0).toInt();

    QStringList lSelectedIdsStr;
    lSelectedIdsStr = aSettings.value("Selected").toString().split(';');
    for (int i = 0; i < lSelectedIdsStr.length(); i++)
        mSelectedIds.append(lSelectedIdsStr.at(i).toInt());

    mScrollPosVert = aSettings.value("ScrollPosVert", 0).toInt();
    mScrollPosHoriz = aSettings.value("ScrollPosHoriz", 0).toInt();

    ui->cbFullMode->setChecked(aSettings.value("Full mode", false).toBool());
    ui->cbAutoUpdate->setChecked(aSettings.value("Auto update", true).toBool());

    InitInConstructor();
}

PlotHistDlg::~PlotHistDlg() {
    delete ui;
}

void PlotHistDlg::SaveState(QSettings &aSettings) {
    SaveSettings(aSettings);
    if (mPlot) {
        aSettings.setValue("Document ID", mPlot->Id());
    }
    if (!ui->twHist->currentItem()) {
        aSettings.setValue("Current item", 0);
    } else {
        aSettings.setValue("Current item", static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->HistoryConst()->Num());
        aSettings.setValue("Current column", ui->twHist->currentColumn());
    }

    QList<QTreeWidgetItem *> lSelectedItems = ui->twHist->selectedItems();
    QStringList lSelectedIds;
    for (int i = 0; i < lSelectedItems.length(); i++) {
        lSelectedIds.append(QString::number(static_cast<PlotHistTreeItem *>(lSelectedItems.at(i))->HistoryConst()->Id()));
    }
    aSettings.setValue("Selected", lSelectedIds.join(';'));

    aSettings.setValue("Full mode", ui->cbFullMode->isChecked());
    aSettings.setValue("Auto update", ui->cbAutoUpdate->isChecked());

    aSettings.setValue("ScrollPosVert", ui->twHist->verticalScrollBar()->value());
    aSettings.setValue("ScrollPosHoriz", ui->twHist->horizontalScrollBar()->value());
}

void PlotHistDlg::InitInConstructor() {
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    // always selected in normal color (not gray)
    QPalette lPalette = ui->twHist->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->twHist->setPalette(lPalette);
    // ---------------------------------

    ui->twHist->setItemDelegate(new ROPlotListItemDelegate(ui->twHist));

    connect(gProjects, SIGNAL(PlotListBeforeUpdate(int)), this, SLOT(OnPlotListBeforeUpdate(int)));
    connect(gProjects, SIGNAL(PlotListNeedUpdate(int)), this, SLOT(OnPlotListNeedUpdate(int)));

    connect(gProjects, SIGNAL(PlotBeforeUpdate(PlotData *, int)), this, SLOT(OnPlotBeforeUpdate(PlotData *, int)));
    connect(gProjects, SIGNAL(PlotNeedUpdate(PlotData *, int)), this, SLOT(OnPlotNeedUpdate(PlotData *, int)));

    ui->twHist->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->twHist->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumns(const QPoint &)));
}

void PlotHistDlg::SaveCurrentStateForRefresh() {
    mSelectedIds.clear();
    QList<QTreeWidgetItem *> lSelected  = ui->twHist->selectedItems();
    for (int i = 0; i < lSelected.length(); i++) {
        PlotHistTreeItem *lItem = static_cast<PlotHistTreeItem *>(lSelected[i]);
        mSelectedIds.append(lItem->HistoryConst()->Id());
    }

    if (ui->twHist->currentItem()) {
        mCurNum = static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->HistoryConst()->Num();
    } else {
        mCurNum = -1;
    }
    mCurColumn = ui->twHist->currentColumn();

    mScrollPosVert = ui->twHist->verticalScrollBar()->value();
    mScrollPosHoriz = ui->twHist->horizontalScrollBar()->value();
}

void PlotHistDlg::ShowData() {
    if (!mPlot) {
        QTimer::singleShot(0, this, SLOT(close()));
        return;
    }

    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "", "PlotHistDlg::ShowData", false);

    ui->leIdProject->setText(QString::number(mPlot->IdProject()));
    ui->leProjName->setText(gProjects->ProjectFullShortName(mPlot->IdProject()));

    ui->leIdPlot->setText(QString::number(mPlot->Id()));
    ui->leSection->setText(mPlot->SectionConst());
    ui->leCode->setText(mPlot->CodeConst());
    ui->leSheet->setText(mPlot->SheetConst());
    ui->cbWorking->setChecked(mPlot->Working());
    ui->cbDeleted->setVisible(mPlot->Deleted());

    ui->leNameTop->setText(mPlot->NameTopConst());
    ui->leNameBottom->setText(mPlot->NameConst());

    bool lAnySheet = false, lAnyFileSize = false, lAnyWorking = false, lAnySavedFrom = false;
    ui->twHist->clear();

    PlotData *lPlot = mPlot;
    bool lCont = true;
    QList<int> lShowedIdPlot;
    int lStartWithVersion = 0;
    int lVerCount = 0;

    PlotHistTreeItem *lItemPrev = NULL;
    while (lCont) {
        int lFromIdPlot = 0, lFromVersion = 0, lLastType = 0;
        QDateTime lLastWhen;

        lVerCount++;

        lShowedIdPlot.append(lPlot->Id());
        for (int i = 0; i < lPlot->HistoryConst().length(); i++) {
            PlotHistoryData *lHistory = lPlot->HistoryConst().at(i);
            PlotHistTreeItem *lItem = new PlotHistTreeItem(lPlot, lHistory, lItemPrev);
            ui->twHist->addTopLevelItem(lItem);

            if (lStartWithVersion && lStartWithVersion < lHistory->Num()) {
                const QPalette &lPalette = palette();
                for (int j = 0; j < ui->twHist->columnCount(); j++) {
                    lItem->setBackgroundColor(j, lPalette.color(QPalette::Active, QPalette::Window));
                }
            }

            if (!lItem->text(6).isEmpty()) {
                lAnySheet = true;
            }

            if (!lItem->text(20).isEmpty()) {
                lAnyFileSize = true;
            }

            if (!lItem->text(lItem->columnCount() - 2).isEmpty()) {
                lAnyWorking = true;
            }

            if (lHistory->Type() == 100
                    || !lItem->text(lItem->columnCount() - 1).isEmpty()
                    && lItem->text(lItem->columnCount() - 1) != lItem->text(lItem->columnCount() - 2)) {
                lAnySavedFrom = true;
            }

            if (lHistory->Num() == mCurNum) {
                ui->twHist->setCurrentItem(lItem, mCurColumn);
            }
            lFromIdPlot = lHistory->FromIdPlot();
            lFromVersion = lHistory->FromVersion();
            lLastType = lHistory->Type();
            lLastWhen = lHistory->WhenConst();

            lItemPrev = lItem;
        }

        if (ui->cbFullMode->isChecked()) {
            lCont = (lFromIdPlot && (lPlot = gProjects->FindByIdPlot(lFromIdPlot)));
            lStartWithVersion = lFromVersion;
            if (!lCont && lLastType == 100) {
                lPlot = NULL;
                lStartWithVersion = 0;
                for (int j = 0; j < mPlot->VersionsConst().length(); j++) {
                    if (!lShowedIdPlot.contains(mPlot->VersionsConst().at(j)->Id())
                            && mPlot->VersionsConst().at(j)->HistoryConst().length()
                            && mPlot->VersionsConst().at(j)->HistoryConst().at(0)->WhenConst() < lLastWhen) {
                        // get with max "when"
                        if (!lPlot) {
                            lPlot = mPlot->VersionsConst().at(j);
                        } else {
                            if (mPlot->VersionsConst().at(j)->HistoryConst().at(0)->WhenConst() > lPlot->HistoryConst().at(0)->WhenConst()) {
                                lPlot = mPlot->VersionsConst().at(j);
                            }
                        }

                    }
                }
                lCont = lPlot;
            }

            // do not step through documents, only through versions
            if (lCont && lPlot->IdCommon() != mPlot->IdCommon()) lCont = false;
        } else {
            lCont = false;
        }
    }

    bool lIsFull = ui->cbFullMode->isChecked();

    ui->twHist->setColumnHidden(2, !lIsFull || lVerCount == 1);
    ui->twHist->setColumnHidden(3, !lIsFull || lVerCount == 1);
    ui->twHist->setColumnHidden(4, !lIsFull || lVerCount == 1);
    ui->twHist->setColumnHidden(5, !lIsFull || lVerCount == 1);
    ui->twHist->setColumnHidden(6, !lIsFull || !lAnySheet || lVerCount == 1);

    ui->twHist->setColumnHidden(20, !lAnyFileSize);

    ui->twHist->setColumnHidden(ui->twHist->columnCount() - 2, !lAnyWorking);
    ui->twHist->setColumnHidden(ui->twHist->columnCount() - 1, !lAnySavedFrom);

    mCurNum = -1;
    mCurColumn = 0;

    for (int i = 0; i < ui->twHist->topLevelItemCount(); i++) {
        ui->twHist->topLevelItem(i)->setSelected(mSelectedIds.contains(static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->HistoryConst()->Id()));
    }
    mSelectedIds.clear();

    if (mScrollPosVert) {
        ui->twHist->verticalScrollBar()->setValue(mScrollPosVert);
        mScrollPosVert = 0;
    }

    if (mScrollPosHoriz) {
        ui->twHist->horizontalScrollBar()->setValue(mScrollPosHoriz);
        mScrollPosHoriz = 0;
    }

    /*if (!lAnyFile) {
        // show temporary file name
        for (int i = 0; i < ui->twHist->topLevelItemCount(); i++) {
            static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->ShowFileName(true);
        }
    }*/

    if (gSettings->DocumentHistory.AutoWidth) {
        for (int i = 0; i < ui->twHist->columnCount(); i++) {
            ui->twHist->resizeColumnToContents(i);
        }
    }
    ui->twHist->setFocus();
}

void PlotHistDlg::OnDeleteVPImageViewer(int) {
    mVPImageViewer = NULL;
}

void PlotHistDlg::OnPlotListBeforeUpdate(int aIdProject) {
    if (mDeleted) return;
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "PlotHistDlg::OnPlotListBeforeUpdate",
                                                             QString::number(aIdProject) + ", mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);
    if (!aIdProject || aIdProject == mIdProject) {
        SaveCurrentStateForRefresh();
    }
}

void PlotHistDlg::OnPlotListNeedUpdate(int aIdProject) {
    if (mDeleted) return;
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "PlotHistDlg::OnPlotListNeedUpdate",
                                                             QString::number(aIdProject) + ", mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);
    if (!aIdProject || aIdProject == mIdProject) {
        if (mPlot = gProjects->FindByIdPlot(mIdPlot)) {
            ShowData();
        } else {
            QTimer::singleShot(0, this, SLOT(close()));
        }
    }
}

void PlotHistDlg::OnPlotBeforeUpdate(PlotData *aPlot, int aType) {
    if (mDeleted) return;
    if (mBlockUpdateSignals) return;
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "PlotHistDlg::OnPlotBeforeUpdate", "mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);
    if (!aType) {
        SaveCurrentStateForRefresh();
    } else if (aType == 6) {
        for (int i = 0; i < ui->twHist->topLevelItemCount(); i++) {
            if (static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->IdPlot() == aPlot->Id()) {
                SaveCurrentStateForRefresh();
                break;
            }
        }
    }
}

void PlotHistDlg::OnPlotNeedUpdate(PlotData *aPlot, int aType) {
    if (mDeleted) return;
    if (mBlockUpdateSignals) return;
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "PlotHistDlg::OnPlotNeedUpdate", "mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);

    bool lDoUpdate = false;

    if (!aType) {
        lDoUpdate = true;
    } else if (aType == 6) {
        for (int i = 0; i < ui->twHist->topLevelItemCount(); i++) {
            if (static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->IdPlot() == aPlot->Id()) {
                lDoUpdate = true;
                break;
            }
        }
    }

    if (lDoUpdate) {
        if (mIdPlot == aPlot->Id()) {
            mPlot = aPlot;
            mIdProject = mPlot->IdProject();
        }
        ShowData();
    }
}

void PlotHistDlg::OnPlotBecameSelected(PlotData * aPlot) {
    //if (gProjects->IsProjectInUpdate(mIdProject)) return;
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "PlotHistDlg::OnPlotBecameSelected", "mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);
    if (aPlot) {
        mPlot = aPlot;
        mIdPlot = mPlot->Id();
        mIdProject = mPlot->IdProject();
        mDeleted = mPlot->Deleted();

        mBlockUpdateSignals = true;
        ShowData();
        mBlockUpdateSignals = false;
    }
    // not clear if is null, doesn't matter
}

void PlotHistDlg::DoSelectColumns(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    QList<int> lDis;
    lDis << 0 << 7 << 8 << 9; // #, origin, user, when

    if (!ui->cbFullMode->isChecked()) {
        lDis << 2 << 3 << 4 << 5 << 6;
    }

    w.SetHeaderView(ui->twHist->header());
    w.SetDisabledIndexes(lDis);
    if (w.exec() == QDialog::Accepted) {
        if (gSettings->DocumentHistory.AutoWidth) {
            for (int i = 0; i < ui->twHist->columnCount(); i++) {
                if (i != 6) {
                    ui->twHist->resizeColumnToContents(i);
                }
            }
        }
    }
}

void PlotHistDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        if (ReadVersion < CurrentVersion) {
            ui->twHist->setColumnHidden(1, true);
            ui->twHist->setColumnHidden(11, true);
            ui->twHist->setColumnHidden(17, true);

            ui->twHist->setColumnHidden(21, true);
            ui->twHist->setColumnHidden(22, true);
            ui->twHist->setColumnHidden(23, true);
        }

        // did n't catch update
        mBlockUpdateSignals = true;
        ShowData();
        mBlockUpdateSignals = false;

        mJustStarted = false;
    }
}

void PlotHistDlg::ViewSelected(bool aWithoutXrefs) {
    if (ui->twHist->currentItem()
            && static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->HistoryConst()->IsPicture()) {
        switch (gSettings->Image.ViewerType) {
        case 0:
            if (mVPImageViewer) return;
            mVPImageViewer = new VPImageViewer(ui->twHist, this/*it is parent, can be gMainWindow*/);
            connect(mVPImageViewer, SIGNAL(finished(int)), this, SLOT(OnDeleteVPImageViewer(int)));
            mVPImageViewer->setAttribute(Qt::WA_DeleteOnClose);
            mVPImageViewer->show();
            break;
        case 1:
        case 2:
            ImageViewerThread::ModalViewList(ui->twHist);
            break;
        }
        return;
    }

    QList<QTreeWidgetItem *> lSelected  = ui->twHist->selectedItems();
    if (!lSelected.count()) return;

    PlotData *lPlot;
    MainDataForCopyToAcad lDataForAcad(1, aWithoutXrefs);
    QList<int> lIdDwgMaxInited;
    for (int i = 0; i < lSelected.length(); i++) {
        PlotHistTreeItem *lItem = static_cast<PlotHistTreeItem *>(lSelected[i]);
        lPlot = lItem->PlotRef();
        if (!lIdDwgMaxInited.contains(lPlot->Id())) {
            lPlot->InitIdDwgMax();
            lIdDwgMaxInited.append(lPlot->Id());
        }
        if ((lPlot->FileType() < 20 || lPlot->FileType() > 29) && lItem->HistoryConst()->ExtConst().toLower() == "dwg") {
            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lPlot->Id(), (lPlot->IdDwgMax() == lItem->HistoryConst()->Id())?0:lItem->HistoryConst()->Id(), 0, false));
        } else {
            // view non-acad document
            if (lPlot->IdDwgMax() == lItem->HistoryConst()->Id()) {
                gSettings->DoOpenNonDwg(lPlot->Id(), 1 /*id_plot*/, 0 /*view*/, "");
            } else {
                gSettings->DoOpenNonDwg(lItem->HistoryConst()->Id(), 2 /*id_dwg*/, 0 /*view*/, "");
            }
        }
    }
    if (!lDataForAcad.ListConst().isEmpty()) {
        gSettings->DoOpenDwgNew(lDataForAcad);
    }
}

void PlotHistDlg::StyleSheetChangedInSescendant() {
    if (gSettings->DocumentHistory.AutoWidth) {
        for (int i = 0; i < ui->twHist->columnCount(); i++) {
            if (i != 6) {
                ui->twHist->resizeColumnToContents(i);
            }
        }
    }
}

void PlotHistDlg::on_twHist_customContextMenuRequested(const QPoint &pos) {
    MyMutexLocker lLocker(gMainWindow->UpdateMutex(), 0);
    if (!isModal() && !lLocker.IsLocked()) return; // something wrong

    QMenu lMenu(this);

    int i;

    QAction *lARes;
    QAction *lAEdit = NULL, *lAView = NULL, *lAViewNoXrefs = NULL, *lAProps = NULL, *lANewVersion = NULL,
            *lAGoto = NULL, *lAGotoHistory = NULL, *lAOpenWorking = NULL, *lAOpenLastSave = NULL, *lASave = NULL,
            *lASelectForCompare = NULL, *lACompareWithSelected = NULL, *lACompareInAutocad = NULL, *lACompareToPDF = NULL,
            * lAUserProps = NULL,
            /**lACompareWithPrev = NULL, */*lACopyToClipboard = NULL;

    QString lFileNameWorking, lFileNameLastSave;

    // it is selected for "Compare two"
    int lIdPlotForCmp, lHistNumForCmp;

    //if (!ui->twHist->currentItem()) return;
    //PlotHistTreeItem * item = static_cast<PlotHistTreeItem *>(ui->twHist->currentItem());
    QList<QTreeWidgetItem *> lSelected  = ui->twHist->selectedItems();
    if (!lSelected.count()) return;

    PlotData *lPlot = NULL;
    int lDWGCnt = 0;

    if (lSelected.count() == 1) {
        lPlot = static_cast<PlotHistTreeItem *>(lSelected.at(0))->PlotRef();
        if (static_cast<PlotHistTreeItem *>(lSelected.at(0))->HistoryConst()->ExtConst().toLower() == "dwg")
            lDWGCnt = 1;
    } else {
        for (i = 0; i < lSelected.count(); i++) {
            if (static_cast<PlotHistTreeItem *>(lSelected.at(i))->HistoryConst()->ExtConst().toLower() == "dwg") {
                lDWGCnt++;
            }
        }
    }

    //if (!(mPLTFlags & PLTNoEditMenu))
    // edit - you can edit only one
    if (lPlot
            && !lPlot->Deleted()
            && !lPlot->EditNA())
        lAEdit = lMenu.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Edit"));

    // view
    if (lPlot
            || !gSettings->DocumentTree.OpenSingleDocument) {
        lAView = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View"));
        lMenu.setDefaultAction(lAView);

        for (int i = 0; i < lSelected.count(); i++) {
            if (static_cast<PlotHistTreeItem *>(lSelected.at(i))->HistoryConst()->XrefsCnt()) {
                lAViewNoXrefs = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View w/o xrefs"));
                break;
            }
        }
    }

    bool lBeforeSaveSeparatorAdded = false;
    if (lSelected.count() == 1) {
        // Properties... --------------------------------------------------
        lAProps = lMenu.addAction(QIcon(":/some/ico/ico/DocProps.png"), tr("Properties..."));
        lMenu.addSeparator();
        lANewVersion = lMenu.addAction(tr("New version..."));

        // Go to document
        const PlotHistoryData * lHistory = static_cast<PlotHistTreeItem *>(lSelected.at(0))->HistoryConst();
        if (lHistory->Type() == 0
                && lHistory->FromIdPlot()
                && lHistory->IdPlot() != lHistory->FromIdPlot()) {
            lMenu.addSeparator();
            lAGoto = lMenu.addAction(tr("Go to document"));
            lAGotoHistory = lMenu.addAction(tr("Go to history"));
        }

        if (ui->twHist->currentColumn() == ui->twHist->columnCount() - 2
                && !lHistory->WorkingFileNameConst().isEmpty()) {
            QString lDirName;
            lFileNameWorking = lHistory->WorkingFileNameConst();
            lFileNameWorking.replace('\\', '/');
            lDirName = lFileNameWorking;
            lDirName.resize(lDirName.lastIndexOf('/'));
            QDir lDir(lDirName);
            if (QFile::exists(lFileNameWorking) || lDir.exists()) {
                if (!lBeforeSaveSeparatorAdded) {
                    lMenu.addSeparator();
                    lBeforeSaveSeparatorAdded = true;
                }
                lAOpenWorking = lMenu.addAction(tr("Open working directory..."));
            }
        }

        if (ui->twHist->currentColumn() == ui->twHist->columnCount() - 1
                && !lHistory->SavedFromFileNameConst().isEmpty()) {
            lFileNameLastSave = lHistory->SavedFromFileNameConst();
            if (QFile::exists(lFileNameLastSave)) {
                if (!lBeforeSaveSeparatorAdded) {
                    lMenu.addSeparator();
                    lBeforeSaveSeparatorAdded = true;
                }
                lAOpenLastSave = lMenu.addAction(tr("Open directory..."));
            }
        }
    }

    if (!lBeforeSaveSeparatorAdded) {
        lMenu.addSeparator();
    }
    lASave = lMenu.addAction(QIcon(":/some/ico/ico/SaveFromDatabase.png"), tr("Save..."));

    lMenu.addSeparator();

    QMenu *lCmpMenu = NULL;

    if (lPlot && lDWGCnt == 1) {
        if (!lCmpMenu) {
            lCmpMenu = lMenu.addMenu(tr("Comparing"));
        }
        lASelectForCompare = lCmpMenu->addAction(/*QIcon(":/some/ico/ico/SaveFromDatabase.png"),*/ tr("Select for compare"));
        if (gMainWindow->GetPlotForCmp(lIdPlotForCmp, lHistNumForCmp)) {
            lACompareWithSelected = lCmpMenu->addAction(/*QIcon(":/some/ico/ico/SaveFromDatabase.png"),*/ tr("Compare with")
                                                    + " " + QString::number(lIdPlotForCmp) + "/" + QString::number(lHistNumForCmp));
        }
    }

    if (lDWGCnt > 1) {
        if (!lCmpMenu) {
            lCmpMenu = lMenu.addMenu(tr("Comparing"));
        }
        lACompareInAutocad = lCmpMenu->addAction(/*QIcon(":/some/ico/ico/SaveFromDatabase.png"),*/ tr("Compare in AutoCAD"));
        lACompareToPDF = lCmpMenu->addAction(/*QIcon(":/some/ico/ico/SaveFromDatabase.png"),*/ tr("Compare to PDF"));
    }



/*    if (lSelected.count() == 2) {
        if ((lHistory1 = static_cast<PlotHistTreeItem *>(lSelected.at(0))->HistoryConst())
                && (lHistory2 = static_cast<PlotHistTreeItem *>(lSelected.at(1))->HistoryConst())
                && lHistory1->ExtConst().toLower() == "dwg"
                && lHistory2->ExtConst().toLower() == "dwg") {
            if (lHistory1->WhenConst() > lHistory2->WhenConst()) qSwap(lHistory1, lHistory2);
        }
    } else if (lSelected.count() > 2) {
        int lDwgCnt;
    }
*/
//    for (i = 0; i < lSelected.count(); i++) {
//        if (static_cast<PlotHistTreeItem *>(lSelected.at(i))->HistoryConst()->Num() > 1) {
//            lACompareWithPrev = lMenu.addAction(/*QIcon(":/some/ico/ico/SaveFromDatabase.png"),*/ tr("Compare with previous"));
//            break;
//        }
//    }

    if (ui->twHist->currentItem()
            && ui->twHist->currentColumn() == 8) {
        lMenu.addSeparator();
        lAUserProps = lMenu.addAction(/*QIcon(":/some/ico/ico/copy.png"), */tr("User properties"));
    }

    lMenu.addSeparator();
    lACopyToClipboard = lMenu.addAction(QIcon(":/some/ico/ico/copy.png"), tr("Copy to clipboard"));

    if (lARes = lMenu.exec(QCursor::pos())) {
        if (lARes == lAEdit) {
            lPlot->InitEditStatus();
            if (lPlot->ES() == PlotData::PESEditing) {
                // skipped - editing now
                QMessageBox::critical(this, tr("Document history - opening for editing"),
                                      QString::number(lPlot->Id()) + " - " + lPlot->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(lPlot->ESUserConst()));
                return;
            }

            if (!lPlot->Working()
                    && QMessageBox::question(this, tr("Document history - opening for editing"), tr("Are you sure you want to edit non-active version of this document?")) != QMessageBox::Yes) {
                return;
            }

            PlotHistTreeItem *lItem = static_cast<PlotHistTreeItem *>(lSelected[0]);
            lPlot->InitIdDwgMax();
            if (lPlot->IdDwgMax() != lItem->HistoryConst()->Id()
                    && QMessageBox::question(this, tr("Document history - opening for editing"), tr("Do you want to edit old version of document?")) != QMessageBox::Yes) {
                return;
            }
            // start editing now
            if ((lPlot->FileType() < 20 || lPlot->FileType() > 29) && lItem->HistoryConst()->ExtConst().toLower() == "dwg") {
                MainDataForCopyToAcad lDataForAcad(2);
                lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lPlot->Id(), (lPlot->IdDwgMax() == lItem->HistoryConst()->Id())?0:lItem->HistoryConst()->Id(), 0, false));
                gSettings->DoOpenDwgNew(lDataForAcad);
            } else {
                // edit non-AutoCAD
                if (lPlot->IdDwgMax() == lItem->HistoryConst()->Id()) {
                    gSettings->DoOpenNonDwg(lPlot->Id(), 1 /*id_plot*/, 1 /*edit*/, "");
                } else {
                    gSettings->DoOpenNonDwg(lItem->HistoryConst()->Id(), 2 /*id_dwg*/, 1 /*edit*/, "");
                }
            }

        } else if (lARes == lAView
                   || lARes == lAViewNoXrefs) {
            ViewSelected(lARes == lAViewNoXrefs);
        } else if (lARes == lAProps) {
            gMainWindow->ShowPlotProp(lPlot, static_cast<PlotHistTreeItem *>(lSelected[0])->HistoryRef());
        } else if (lARes == lANewVersion) {
            PlotHistTreeItem *lItem = static_cast<PlotHistTreeItem *>(lSelected[0]);
            PlotHistoryData *lHistory = NULL;

            lPlot->InitIdDwgMax();

            if (lPlot->IdDwgMax() != lItem->HistoryConst()->Id()) {
                lHistory = lItem->HistoryRef();
            }

            PlotSimpleListDlg w(PlotSimpleListDlg::NDTNewVersion, lPlot, lHistory, this);
            if (w.exec()) {
                // new version was succesfully created, reload all
                gProjects->EmitPlotListBeforeUpdate(lPlot->IdProject());
                //mSelectedPlots = w.Plots(); // for this widnow
                // update all windows with this project
                gProjects->EmitPlotListNeedUpdate(lPlot->IdProject());
            }
        } else if (lARes == lAGoto) {
            PlotHistoryData * lHistory = static_cast<PlotHistTreeItem *>(lSelected.at(0))->HistoryRef();
            PlotData * lPlot = gProjects->FindByIdPlot(lHistory->FromIdPlot());
            PlotHistoryData * lHistoryGoto = NULL;

            foreach (PlotHistoryData * lHistoryFound, lPlot->HistoryConst()) {
                if (lHistoryFound->Num() == lHistory->FromVersion()) {
                    lHistoryGoto = lHistoryFound;
                    break;
                }
            }

            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlot->IdProject()), lPlot, lHistoryGoto);
        } else if (lARes == lAGotoHistory) {
            PlotHistoryData * lHistory = static_cast<PlotHistTreeItem *>(lSelected.at(0))->HistoryRef();
            gMainWindow->ShowPlotHist(gProjects->FindByIdPlot(lHistory->FromIdPlot()), lHistory->FromVersion(), false, false);
        } else if (lARes == lAOpenWorking) {
            lFileNameWorking.replace('/', '\\');
            QProcess::startDetached("Explorer /select, \"" + lFileNameWorking + "\"");
        } else if (lARes == lAOpenLastSave) {
            lFileNameLastSave.replace('/', '\\');
            QProcess::startDetached("Explorer /select, \"" + lFileNameLastSave + "\"");
        } else if (lARes == lASave) {
            SaveDialog lSaveDlg(this);

            for (int i = 0; i < lSelected.length(); i++) {
                lSaveDlg.AddDocument(static_cast<PlotHistTreeItem *>(lSelected[i])->HistoryConst()->IdPlot(),
                                     static_cast<PlotHistTreeItem *>(lSelected[i])->HistoryConst()->Id());
            }
            lSaveDlg.exec();

        } else if (lARes == lASelectForCompare) {
            gMainWindow->SetPlotForCmp(lPlot->Id(), static_cast<PlotHistTreeItem *>(lSelected.at(0))->HistoryConst()->Num());
        } else if (lARes == lACompareWithSelected) {
            PlotHistoryData * lHistoryOld = static_cast<PlotHistTreeItem *>(lSelected.at(0))->HistoryRef();
            PlotData *lPlot = gProjects->FindByIdPlot(lIdPlotForCmp);
            if (lPlot) {
                PlotHistoryData *lHistoryNew = lPlot->GetHistoryByNum(lHistNumForCmp);
                if (lHistoryNew) {
                    MainDataForCopyToAcad lDataForAcad(1/*version*/, cmpWithXrefs, "");
                    lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eCMP, lHistoryOld, lHistoryNew));
                    gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
                }
            }
        } else if (lARes == lACompareInAutocad) {
            MainDataForCopyToAcad lDataForAcad(1 /*version*/, /*cmpWithXrefs*/0, "");

            for (i = 0; i < ui->twHist->topLevelItemCount() - 1; i++) {
                if (ui->twHist->topLevelItem(i)->isSelected()
                        && static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->HistoryConst()->ExtConst().toLower() == "dwg") {
                    for (int j = i + 1; j < ui->twHist->topLevelItemCount(); j++) {
                        if (ui->twHist->topLevelItem(j)->isSelected()
                                && static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(j))->HistoryConst()->ExtConst().toLower() == "dwg") {
                            // old is first
                            lDataForAcad.ListRef().append(
                                        new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eCMP,
                                                                    static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(j))->HistoryConst(),
                                                                    static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->HistoryConst()));
                            break;
                        }
                    }
                }
            }

            if (!lDataForAcad.ListConst().isEmpty()) {
                gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
            }
        } else if (lARes == lACompareToPDF) {
            // parameters at first
            bool lParamsIsOk;
            if (!gSettings->Compare.Readed
                    || gSettings->Compare.OldColor == gSettings->Compare.NewColor
                    || gSettings->Compare.AlwaysAskAll) {
                DwgCmpSettingsDlg w(this);
                lParamsIsOk = w.exec() == QDialog::Accepted;
            } else if (gSettings->Compare.AlwaysAskOutputDir
                       || gSettings->Compare.OutputDir.isEmpty()) {
                QFileDialog dlg(this);
                dlg.setFileMode(QFileDialog::DirectoryOnly);

                dlg.setDirectory(gSettings->Compare.OutputDir);
                if (lParamsIsOk = (dlg.exec() == QDialog::Accepted)) {
                    gSettings->Compare.OutputDir = dlg.selectedFiles().at(0);
                }
            } else {
                lParamsIsOk = true;
            }

            if (!lParamsIsOk) return;

            QDir lDir(gSettings->Compare.OutputDir);

            if (!lDir.exists()) {
                if (!lDir.mkpath(gSettings->Compare.OutputDir)) {
                    QMessageBox::critical(this, tr("Document history - comparing"), tr("Can't create directory") + "\n" + gSettings->Compare.OutputDir);
                    return;
                }
            }

            MainDataForCopyToAcad lDataForAcad(3 /*version*/, /*cmpWithXrefs*/ 0, gSettings->Compare.OutputDir);

            for (i = 0; i < ui->twHist->topLevelItemCount() - 1; i++) {
                if (ui->twHist->topLevelItem(i)->isSelected()
                        && static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->HistoryConst()->ExtConst().toLower() == "dwg") {
                    for (int j = i + 1; j < ui->twHist->topLevelItemCount(); j++) {
                        if (ui->twHist->topLevelItem(j)->isSelected()
                                && static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(j))->HistoryConst()->ExtConst().toLower() == "dwg") {
                            lDataForAcad.ListRef().append(
                                        // old is first
                                        new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eCMP,
                                                                    static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(j))->HistoryConst(),
                                                                    static_cast<PlotHistTreeItem *>(ui->twHist->topLevelItem(i))->HistoryConst()));
                            break;
                        }
                    }
                }
            }

            if (!lDataForAcad.ListConst().isEmpty()) {
                gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
            }
//        } else if (lARes == lACompareWithPrev) {
//            MainDataForCopyToAcad lDataForAcad(1/*version*/, cmpWithXrefs, "");
//            for (i = 0; i < lSelected.count(); i++) {
//                const PlotHistoryData *lHistoryNew = static_cast<PlotHistTreeItem *>(lSelected.at(i))->HistoryConst();
//                if (lHistoryNew->Num() > 1) {
//                    const PlotHistoryData *lHistoryOld = lPlot->GetHistoryByNum(lHistoryNew->Num() - 1);
//                    lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(lHistoryOld, lHistoryNew));
//                }
//            }
//            gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
        } else if (lARes == lAUserProps) {
            UserPropDlg w(gUsers->FindByLogin(static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->HistoryConst()->UserConst()), this);
            if (w.exec() == QDialog::Accepted) {
                //renew
                gUsers->InitUserList();
            }
        } else if (lARes == lACopyToClipboard) {
            gSettings->CopyToClipboard(ui->twHist);
        }
    }
}

void PlotHistDlg::on_cbAutoUpdate_toggled(bool checked) {
    if (checked) {
        connect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(OnPlotBecameSelected(PlotData *)));
    } else {
        disconnect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(OnPlotBecameSelected(PlotData *)));
    }
}

void PlotHistDlg::on_twHist_itemSelectionChanged() {
    if (ui->twHist->selectedItems().count() == 1
            && ui->twHist->currentItem()) {
        emit gProjects->PlotHistoryBecameSelected(static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->PlotRef(),
                                                  static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->HistoryRef());
    }
}

void PlotHistDlg::on_twHist_doubleClicked(const QModelIndex &index) {
    ViewSelected(false);
}

void PlotHistDlg::on_tbReload_clicked() {
    int i;
    QList<PlotData *> lPlots;

    for (i = 0; i < ui->twHist->topLevelItemCount(); i++) {
        if (!lPlots.contains(static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->PlotRef())) {
            lPlots.append(static_cast<PlotHistTreeItem *>(ui->twHist->currentItem())->PlotRef());
        }
    }

    for (i = 0; i < lPlots.length(); i++){
        lPlots.at(i)->ReinitHistory();
    }
}

void PlotHistDlg::on_cbFullMode_toggled(bool checked) {
    if (!mJustStarted) ShowData();
}

void PlotHistDlg::on_twHist_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (mVPImageViewer
            && current) mVPImageViewer->ShowImage();
}
