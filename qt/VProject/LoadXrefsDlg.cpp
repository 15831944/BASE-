#include "LoadXrefsDlg.h"
#include "ui_LoadXrefsDlg.h"

#include "GlobalSettings.h"
#include "MainWindow.h"

#include "../ProjectLib/ProjectData.h"

#include <QMenu>
#include <QTimer>
#include <QFileDialog>
#include <QProcess>
#include <QClipboard>

LoadXrefsDlg::LoadXrefsDlg(int aIdProject, QWidget *parent) :
    QFCDialog(parent, false), AcadXchgDialog(),
    ui(new Ui::LoadXrefsDlg),
    mJustStarted(true), mPreloadParamsDlg(NULL), mIdProject(aIdProject)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    LoadXrefsListItemDelegate *lItemDelegate = new LoadXrefsListItemDelegate(ui->twMain, this);
    ui->twMain->setItemDelegate(lItemDelegate);
    connect(lItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
}

LoadXrefsDlg::~LoadXrefsDlg()
{
    delete ui;
}

void LoadXrefsDlg::FirstInit() {
    ProjectData *lProject = gProjects->FindByIdProject(mIdProject);
    if (!lProject
            || !SelectFiles()) {
        if (qobject_cast<QMdiSubWindow *>(parent())) {
            QTimer::singleShot(0, parent(), SLOT(close()));
        } else {
            QTimer::singleShot(0, this, SLOT(close()));
        }
    } else {
        ui->leIdProject->setText(QString::number(lProject->Id()));
        ui->leProjName->setText(lProject->FullShortName());

        if (mAnyDwg) {
            mPreloadParamsDlg = new PreloadParamsDlg(this);
            if (mPreloadParamsDlg->exec() == QDialog::Accepted) {
                // run preprocessing
                on_pbProceed_clicked();
            }
        }
//        if (qobject_cast<QMdiSubWindow *>(parent())) {
//            qobject_cast<QMdiSubWindow *>(parent())->nextInFocusChain()->setFocus();
//            qobject_cast<QMdiSubWindow *>(parent())->setFocus();
//            // this->setFocus();
//        }
    }
}

void LoadXrefsDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        QTimer::singleShot(0, this, SLOT(FirstInit()));
        mJustStarted = false;
    }
}

bool LoadXrefsDlg::nativeEvent(const QByteArray & eventType, void * message, long * result) {
    if (AcadXchgDialog::DoNativeEvent(eventType, message, result)) return true;
    return QFCDialog::nativeEvent(eventType, message, result);
}

void LoadXrefsDlg::RegenCodes() {
    QList<tPairIntString> lAddNums;
    for (int i = 0; i < ui->twMain->topLevelItemCount(); i++) {
        // here can be different project; now it is so - single project only
        ProjectData *lProject = gProjects->FindByIdProject(mIdProject);
        TreeDataRecord *lTreeData = static_cast<LoadXrefsListItem*>(ui->twMain->topLevelItem(i))->TreeData();
        if (lProject
                && lTreeData) {
            QString lCodeTempl = lTreeData->ActualCode();
            lProject->CodeTempleReplaceWithDataMain(lCodeTempl);
            lProject->CodeTempleReplaceWithDataSub(lCodeTempl);
            lCodeTempl.replace('%', '$'); // temporary line

            bool lIsFound = false;
            int lAddNum = 0;
            for  (int j = 0; j < lAddNums.length(); j++) {
                tPairIntString &lIS = lAddNums[j];
                if (lIS.second == lCodeTempl) {
                    lIsFound = true;
                    lAddNum = lIS.first;
                    lIS.first++;
                    break;
                }
            }
            if (!lIsFound) {
                lAddNums.append(qMakePair(1, lCodeTempl));
            }
            lCodeTempl = lProject->GenerateFixedCode(lCodeTempl, lAddNum, -1);
            ui->twMain->topLevelItem(i)->setText(lllLXLCode, lCodeTempl);
        }
    }
//    ui->twMain->resizeColumnToContents(lllLXLCode);
//    ui->twMain->setColumnWidth(lllLXLCode, ui->twMain->columnWidth(lllLXLCode) + 30);
    for (int i = 0; i < ui->twMain->topLevelItemCount(); i++) {
        ui->twMain->resizeColumnToContents(i);
        ui->twMain->setColumnWidth(i, ui->twMain->columnWidth(i) + 30);
    }
}

bool LoadXrefsDlg::SelectFiles() {
    QMutexLocker lLocker(mJustStarted?0:gMainWindow->UpdateMutex());

    QFileDialog dlg;
    QStringList filters;
    filters << "AutoCAD drawings(*.dwg)"
            << "Images (*.jpg  *.jpeg *.png *.bmp *.tif *.tiff *.gif)"
            << "All files (*)";

    mAnyDwg = false;
    mAnyNonDwg = false;

    ProjectData *lProject = gProjects->FindByIdProject(mIdProject);
    if (!lProject) return false;

    dlg.setDirectory(gSettings->LoadFiles.LastDir);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilters(filters);

    if (dlg.exec() == QDialog::Accepted) {
        bool lIsFirst = true;
        QStringList lFiles = dlg.selectedFiles();

        std::sort(lFiles.begin(), lFiles.end(), CmpStringsWithNumbersNoCase);

        for (int i = 0; i < lFiles.length(); i++) {
            QFileInfo lFileInfo(lFiles.at(i));
            // directory name
            if (lIsFirst) {
                gSettings->LoadFiles.LastDir = lFileInfo.canonicalPath();
                ui->twMain->clear();
                lIsFirst = false;
            }

            if (lFileInfo.suffix().toLower() == "dwg")
                mAnyDwg = true;
            else
                mAnyNonDwg = true;

            LoadXrefsListItem *lItem = new LoadXrefsListItem(mIdProject, lFiles.at(i));

            if (lItem->IsError()) {
                delete lItem;
                ui->twMain->clear();
                return false;
            } else {
                ui->twMain->addTopLevelItem(lItem);
            }
        }

        if (!(mAnyDwg || mAnyNonDwg)) {
            ui->twMain->clear();
        }
    }

    ui->twMain->setColumnHidden(2, !mAnyDwg);
    ui->twMain->setColumnHidden(3, !mAnyDwg);
    ui->twMain->setColumnHidden(4, !mAnyDwg);
    ui->twMain->setColumnHidden(lllLXLBlockName, !mAnyDwg);

    RegenCodes();

    ui->pbProcessSettings->setVisible(mAnyDwg);
    ui->pbProceed->setVisible(mAnyDwg);

    return mAnyDwg || mAnyNonDwg;
}

/*XchgFileData *LoadXrefsDlg::FindFileData(const QString &aOrigFileName) {
    XchgFileData *lXchgFileData = NULL;
    for (int i = 0; i < mFiles.length(); i++) {
        if (!mFiles.at(i)->FileInfoOrigConst().canonicalFilePath().compare(aOrigFileName, Qt::CaseInsensitive)) {
            lXchgFileData = mFiles.at(i);
            break;
        }
    }
    return lXchgFileData;
}*/

void LoadXrefsDlg::on_twMain_customContextMenuRequested(const QPoint &pos) {
    QMutexLocker lLocker(gMainWindow->UpdateMutex());
    QMenu lMenu(this);
    QAction *lARes;

    PlotData *lPlot;
    PlotHistoryData *lHistory;
    QString lDocument;
    QStringList lDocuments;
    QList<PlotData *> lPlots;
    QList<PlotHistoryData *> lHistories;
    QAction *lAEditOriginal = NULL, *lAEditProcessed = NULL;
    QList<QAction *> lAView, lAViewNoXrefs, lAHistory, lAGoto;

    QList<QTreeWidgetItem *> lSelected = ui->twMain->selectedItems();
    switch (ui->twMain->currentColumn()) {
    case 0:
        if (lSelected.length()) {
            lAEditOriginal = lMenu.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Open original"));
        }

        for (int i = 0; i < lSelected.length() && !lAEditProcessed; i++) {
            if (static_cast<LoadXrefsListItem *>(lSelected.at(i))->Processed()) {
                lAEditProcessed = lMenu.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Open processed"));
                break;
            }
        }

        break;
    case 4: // documents in base
        if (lSelected.length() == 1) {
            if (lSelected.at(0)->text(ui->twMain->currentColumn()).indexOf(';') != -1) {
                lDocuments = lSelected.at(0)->text(ui->twMain->currentColumn()).split(';');
                foreach (lDocument, lDocuments) {
                    QMenu *lSubMenu = lMenu.addMenu(lDocument);
                    lPlot = gProjects->FindByIdPlot(lDocument.left(lDocument.indexOf('/')).toInt());
                    if (lPlot) {
                        //lPlot->InitIdDwgMax();
                        if (lHistory = lPlot->GetHistoryByNum(lDocument.mid(lDocument.indexOf('/') + 1).toInt())) {
                            lPlots.append(lPlot);
                            lHistories.append(lHistory);

                            lAView.append(lSubMenu->addAction(QIcon(":/some/ico/ico/view.png"), tr("View")));
                            if (lPlot->XrefsCnt())
                                lAViewNoXrefs.append(lSubMenu->addAction(QIcon(":/some/ico/ico/view.png"), tr("View w/o xrefs")));
                            lAHistory.append(lSubMenu->addAction(tr("History")));
                            lAGoto.append(lSubMenu->addAction(tr("Go to document")));
                        }
                    }
                }
            } else {
                lDocument = lSelected.at(0)->text(ui->twMain->currentColumn());
                lPlot = gProjects->FindByIdPlot(lDocument.left(lDocument.indexOf('/')).toInt());
                if (lPlot) {
                    //lPlot->InitIdDwgMax();
                    if (lHistory = lPlot->GetHistoryByNum(lDocument.mid(lDocument.indexOf('/') + 1).toInt())) {
                        lPlots.append(lPlot);
                        lHistories.append(lHistory);
                        lDocuments.append(lDocument);

                        lAView.append(lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View")));
                        if (lPlot->XrefsCnt())
                            lAViewNoXrefs.append(lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View w/o xrefs")));
                        lAHistory.append(lMenu.addAction(tr("History")));
                        lAGoto.append(lMenu.addAction(tr("Go to document")));
                    }
                }
            }

        }
        break;
    }

    if (!lMenu.actions().isEmpty()
            && (lARes = lMenu.exec(QCursor::pos()))) {
        if (lARes == lAEditOriginal) {
            bool lIsFirst = true;
            for (int i = 0; i < lSelected.length(); i++) {
                QProcess::startDetached(QString(qgetenv("COMSPEC")) + " /c \""
                                        + static_cast<LoadXrefsListItem *>(lSelected.at(i))->FileInfoOrigConst().canonicalFilePath().replace('/', '\\')
                                        + "\"");
                if (lIsFirst) {
                    for (int z = 0; z < 50; z++) {
                        QCoreApplication::processEvents();
                        QThread::msleep(100);
                    }
                    lIsFirst = false;
                }
            }
        } else if (lARes == lAEditProcessed) {
            bool lIsFirst = true;
            for (int i = 0; i < lSelected.length(); i++) {
                for (int j = 0; j < mFiles.length(); j++) {
                    if (!mFiles.at(j)->FileInfoOrigConst().canonicalFilePath().compare(lSelected.at(i)->toolTip(0), Qt::CaseInsensitive)) {
                        QProcess::startDetached(QString(qgetenv("COMSPEC")) + " /c start \"" + mFiles.at(j)->FileInfoPrcdConst().canonicalFilePath() + "\"");
                        if (lIsFirst) {
                            for (int z = 0; z < 50; z++) {
                                QCoreApplication::processEvents();
                                QThread::msleep(100);
                            }
                            lIsFirst = false;
                        }
                    }
                }
            }
        } else if (lAView.contains(lARes)) {
            lPlot = lPlots.at(lAView.indexOf(lARes));
            lHistory = lHistories.at(lAView.indexOf(lARes));
            MainDataForCopyToAcad lDataForAcad(1, false);
            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lPlot->Id(), lHistory->Id(), 0, false));
            gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
        } else if (lAViewNoXrefs.contains(lARes)) {
            lPlot = lPlots.at(lAViewNoXrefs.indexOf(lARes));
            lHistory = lHistories.at(lAViewNoXrefs.indexOf(lARes));
            MainDataForCopyToAcad lDataForAcad(1, true);
            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lPlot->Id(), lHistory->Id(), 0, false));
            gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
        } else if (lAHistory.contains(lARes)) {
            lHistory = lHistories.at(lAHistory.indexOf(lARes));
            gMainWindow->ShowPlotHist(lPlots.at(lAHistory.indexOf(lARes)), lHistory?lHistory->Num():0, true, false /* no modal */);
        } else if (lAGoto.contains(lARes)) {
            lPlot = lPlots.at(lAGoto.indexOf(lARes));
            lHistory = lHistories.at(lAGoto.indexOf(lARes));
            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlot->IdProject()), lPlot, lHistory);
        }

    }

    qDeleteAll(lAView);
    qDeleteAll(lAViewNoXrefs);
    qDeleteAll(lAGoto);
    qDeleteAll(lAHistory);
}

void LoadXrefsDlg::on_pbProcessSettings_clicked() {
    mPreloadParamsDlg->exec();
}

void LoadXrefsDlg::on_pbProceed_clicked() {
    if (!mAnyDwg) return;

    int i;

    mFiles.clear();

    QList<tPairIntInt> lList;

    for (i = 0; i < ui->twMain->topLevelItemCount(); i++) {
        LoadXrefsListItem *lItem = static_cast<LoadXrefsListItem *>(ui->twMain->topLevelItem(i));
        if (lItem->FileInfoOrigConst().suffix().toLower() == "dwg") {
            if (lItem->Processed()) {
                QFile::remove(lItem->XchgFileDataConst()->FileInfoPrcdConst().canonicalFilePath());
                lItem->SetNotProcessed();
            }
            mFiles.append(new XchgFileData(lItem->FileInfoOrigConst()));
            lList.append(qMakePair(i, mFiles.length() - 1));
        }
    }

    if (ProcessDwgsForLoad(mPreloadParamsDlg->ProcessType(), mPreloadParamsDlg->ColorBlocks(), mPreloadParamsDlg->ColorEntities(),
                           mPreloadParamsDlg->LWBlocks(), mPreloadParamsDlg->LWEntities(), mPreloadParamsDlg->Layer0Name(), mPreloadParamsDlg->UserCommands(),
                           winId())) {
        for (i = 0; i < lList.length(); i++) {
            LoadXrefsListItem *lItem = static_cast<LoadXrefsListItem *>(ui->twMain->topLevelItem(lList.at(i).first));
            XchgFileData *lXchgFileData = mFiles.at(lList.at(i).second);

            lItem->SetProcessed(lXchgFileData);
            lItem->ShowData();
        }

        RegenCodes();
    }

    mFiles.clear();
}

void LoadXrefsDlg::OnCommitData(QWidget *editor) {
    if (qobject_cast<QComboBox *>(editor)) {
        switch (ui->twMain->currentColumn()) {
        case lllLXLColStatus:
            for (int i = 0; i < ui->twMain->selectedItems().length(); i++) {
                static_cast<LoadXrefsListItem *>(ui->twMain->selectedItems().at(i))->SetWhatToDo(qobject_cast<QComboBox *>(editor)->currentIndex());
                ui->twMain->selectedItems().at(i)->setText(lllLXLColStatus, qobject_cast<QComboBox *>(editor)->currentText());
            }
            break;
        case lllLXLColType:
            for (int i = 0; i < ui->twMain->selectedItems().length(); i++) {
                int aTDArea, aTDId;
                aTDArea = qobject_cast<QComboBox *>(editor)->currentData().toInt() / 10000;
                aTDId = qobject_cast<QComboBox *>(editor)->currentData().toInt() % 10000;
                if (gTreeData->FindById(aTDArea, aTDId)) {
                    static_cast<LoadXrefsListItem *>(ui->twMain->selectedItems().at(i))->SetTreeData(aTDArea, aTDId);
                    ui->twMain->selectedItems().at(i)->setText(lllLXLColType, qobject_cast<QComboBox *>(editor)->currentText());
                }
            }
            break;
        }
    }
}

LoadXrefsListItemDelegate::LoadXrefsListItemDelegate(QWidget *parent, LoadXrefsDlg *aLoadXrefsDlg) :
    ROPlotListItemDelegate(parent),
    mLoadXrefsDlg(aLoadXrefsDlg)
{
}

void LoadXrefsListItemDelegate::ProcessTreeData(const TreeDataRecord *aTreeData, QComboBox *aComboBox, int aLevel) const {
    QString lStr = aTreeData->TextConst();
    int i;

    for (i = 0; i < aLevel; i++) {
        lStr = " " + lStr;
    }

    aComboBox->addItem(lStr, aTreeData->Area() * 10000 + aTreeData->Id());

    for (i = 0; i < aTreeData->LeafsConst().length(); i++) {
        ProcessTreeData(aTreeData->LeafsConst().at(i), aComboBox, aLevel + 1);
    }
}

QWidget *LoadXrefsListItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    if (qobject_cast<QTreeWidget *> (this->parent())
            && qobject_cast<QTreeWidget *> (this->parent())->currentItem()) {

        switch (index.column()) {
        case lllLXLColStatus:
            {
                QComboBox *l = new QComboBox(parent);
                l->addItem(tr("Don't load"), 0);
                l->addItem(tr("Load as new version"), 1);
                l->addItem(tr("Load in existing document"), 2);
                l->addItem(tr("Load as new document"), 3);
                l->setCurrentIndex(static_cast<LoadXrefsListItem *>(qobject_cast<QTreeWidget *> (this->parent())->currentItem())->WhatToDo());
                return l;
            }
            break;
        case lllLXLColType:
            {
                TreeDataRecord *lTreeData = static_cast<LoadXrefsListItem *>(qobject_cast<QTreeWidget *> (this->parent())->currentItem())->TreeData();
                if (lTreeData) {
                    int lCurTDFullId = lTreeData->Area() * 10000 + lTreeData->Id();
                    if (lTreeData->ActualIdGroup() == 2
                            && (lTreeData = gTreeData->FindByGroupId(2))) {
                        QComboBox *l = new QComboBox(parent);
                        ProcessTreeData(lTreeData, l);

                        for (int i = 0; i < l->count(); i++) {
                            if (l->itemData(i) == lCurTDFullId) {
                                l->setCurrentIndex(i);
                                break;
                            }
                        }

                        return l;
                    }
                }
            }
            break;
        case lllLXLNameTop:
        case lllLXLNameTop + 1:
        case lllLXLNameTop + 2:
            {
                QLineEdit *l = new QLineEdit(parent);
                return l;
            }
            break;
        }
    }

    return ROPlotListItemDelegate::createEditor(parent, option, index);
}
