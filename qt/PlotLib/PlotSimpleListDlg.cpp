#include "PlotSimpleListDlg.h"
#include "ui_PlotSimpleListDlg.h"

#include <QFileDialog>
#include <QMenu>

#include "DwgData.h"
#include "DwgLayoutData.h"

#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectTypeData.h"

#include "../VProject/MainWindow.h"

#include "../VProject/SelectColumnsDlg.h"
#include "../VProject/FileUtils.h"

/*PlotAddAdata::PlotAddAdata(const QString &aBlockName, bool aUseWorking) :
    mBlockName(aBlockName),
    mUseWorking(aUseWorking)
{

}
*/
PlotSimpleListDlg::PlotSimpleListDlg(NewDisplayType aDisplayType, const QList<PlotAndHistoryData> &aPlots, QWidget *parent) :
    QFCDialog(parent, true), AcadXchgDialog(),
    ui(new Ui::PlotSimpleListDlg),
    mDisplayType(aDisplayType),
    mCurrentItemId(0), mCurrentColumn(0),
    mOrigFileSize(0), mExistsListMenu(NULL), mFileType(NULL),
    mInListItemCahnged(false), mJustStarted(true)
{
    ui->setupUi(this);
    mPlots = aPlots;
    InitInConstructor();
}

PlotSimpleListDlg::PlotSimpleListDlg(NewDisplayType aDisplayType, PlotData *aPlot, PlotHistoryData *aHistory, bool aAutoUpdate, QWidget *parent) :
    QFCDialog(parent, true), AcadXchgDialog(),
    ui(new Ui::PlotSimpleListDlg),
    mDisplayType(aDisplayType),
    mCurrentItemId(0), mCurrentColumn(0),
    mOrigFileSize(0), mExistsListMenu(NULL), mFileType(NULL),
    mInListItemCahnged(false), mJustStarted(true)
{
    ui->setupUi(this);
    if (mDisplayType == NDTXrefs || mDisplayType == NDTXrefFor) {
        mPlot = aPlot;
        mPlotHistory = aHistory;
        ui->cbAutoUpdate->setChecked(aAutoUpdate);
    } else {
        mPlots.append(qMakePair(aPlot, aHistory));
    }
    InitInConstructor();
}


/*PlotSimpleListDlg::PlotSimpleListDlg(DisplayType aDisplayType, QWidget *parent) :
    QFCDialog(parent),
    ui(new Ui::PlotSimpleListDlg),
    mDisplayType(aDisplayType),
    mCurrentItemId(0), mCurrentColumn(0),
    mInListItemCahnged(false), mJustStarted(true)
{
    InitInConstructor();
}*/

PlotSimpleListDlg::~PlotSimpleListDlg() {
    delete ui;
    qDeleteAll(mPlotAddData);
}

QString PlotSimpleListDlg::AddToClassName() const {
    return QString::number(mDisplayType);
}

const QList <PlotAndHistoryData> & PlotSimpleListDlg::PlotsConst() const {
    return mPlots;
}

void PlotSimpleListDlg::InitInConstructor() {
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->twDocs->setRootIsDecorated(false);

    switch (mDisplayType) {
    case NDTNewVersion:
        setWindowTitle(tr("Creating new versions"));

        ui->swTop->setVisible(false);
        ui->swForMode->setVisible(true);
        ui->swForMode->setCurrentIndex(0);

        ui->cbNewVerComment->addItem("");
        ui->cbNewVerComment->addItem(tr("New file received") + " (" + QDate::currentDate().toString("dd.MM.yyyy") + ")");
        ui->cbNewVerComment->addItem(tr("Geometry changed"));
        ui->cbNewVerComment->addItem(tr("Geometry of road and elevations changed"));
        ui->cbNewVerComment->addItem(tr("Changes for builder"));

        ui->twDocs->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ui->twDocs->setSortingEnabled(false);
        ui->twDocs->InitColumns(PLTNoEditMenu | PLTNewVersion | PLTNoColors);
        // it is for reordering needs
        ui->twDocs->setDragDropMode(QAbstractItemView::InternalMove);

        ui->twDocs->setEditTriggers(ui->twDocs->editTriggers() | QAbstractItemView::AnyKeyPressed);

        ui->cbWhatToDo->addItem(tr("Do nothing"));
        ui->cbWhatToDo->addItem(tr("Start editing"));
        if (mPlots.length() == 1) {
            ui->cbWhatToDo->addItem(tr("Load from file"));
        }

        ui->wdSelectFile->setVisible(false);
        ui->lblAcadVersion->setVisible(false);
        ui->pbAlreadyInBase->setVisible(false);

        break;
    case NDTVersions:
        setWindowTitle(tr("Versions"));

        ui->swTop->setVisible(false);
        ui->swForMode->setVisible(false);

        ui->twDocs->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ui->twDocs->setSortingEnabled(true);
        ui->twDocs->InitColumns(/*PLTNoEditMenu | */PLTWorking | PLTVersions | PLTNoColors);

        ui->twDocs->setDragDropMode(QAbstractItemView::NoDragDrop);

        //ui->twDocs->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //ui->twDocs->setEditTriggers(ui->twDocs->editTriggers() | QAbstractItemView::AnyKeyPressed);
        ui->twDocs->setEditTriggers(QAbstractItemView::EditKeyPressed);
        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colWorking], false);

        connect(ui->twDocs, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(ListItemChanged(QTreeWidgetItem *, int)));

        break;
    case NDTXrefs:
        setWindowTitle(tr("Xrefs"));

        ui->swTop->setVisible(true);
        ui->swForMode->setVisible(false);

        ui->twDocs->setDragDropMode(QAbstractItemView::NoDragDrop);
        ui->twDocs->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ui->twDocs->setSortingEnabled(true);
        ui->twDocs->InitColumns(PLTWorking | PLTProject | PLTNoColors | PLTForXrefs);
        break;
    case NDTXrefFor:
        setWindowTitle(tr("Xref for"));

        ui->swTop->setVisible(true);
        ui->swForMode->setVisible(false);

        ui->twDocs->setDragDropMode(QAbstractItemView::NoDragDrop);
        ui->twDocs->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ui->twDocs->setSortingEnabled(true);
        ui->twDocs->InitColumns(PLTWorking | PLTProject | PLTNoColors | PLTForXrefs);
        break;
    }

    connect(gProjects, SIGNAL(PlotListBeforeUpdate(int)), this, SLOT(OnPlotListBeforeUpdate(int)));
    connect(gProjects, SIGNAL(PlotListNeedUpdate(int)), this, SLOT(OnPlotListNeedUpdate(int)));

    connect(gSettings, SIGNAL(DocTreeSettingsChanged()), this, SLOT(DoSettingsChanged()));

    ui->twDocs->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    disconnect(ui->twDocs->header(), SIGNAL(customContextMenuRequested(const QPoint &)), 0, 0);
    connect(ui->twDocs->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumns(const QPoint &)));
}

void PlotSimpleListDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        if (ReadVersion < CurrentVersion) {
            ui->twDocs->HideColumn(ui->twDocs->colIdCommon);
            /*ui->twDocs->HideColumn(ui->twDocs->colIdHist);
            ui->twDocs->HideColumn(ui->twDocs->colHist);
            ui->twDocs->HideColumn(ui->twDocs->colHistOrigin);*/
            ui->twDocs->HideColumn(ui->twDocs->colCancelDate);
            ui->twDocs->HideColumn(ui->twDocs->colCancelUser);
            ui->twDocs->HideColumn(ui->twDocs->colSentBy);
            ui->twDocs->HideColumn(ui->twDocs->colCreated);
            ui->twDocs->HideColumn(ui->twDocs->colCreatedBy);

            if (mDisplayType == NDTXrefs) {
                ui->twDocs->HideColumn(ui->twDocs->colVersionExt);
                ui->twDocs->HideColumn(ui->twDocs->colSentDate);
                ui->twDocs->HideColumn(ui->twDocs->colSection);
                ui->twDocs->HideColumn(ui->twDocs->colSheet);
            }
        }
        QTimer::singleShot(0, this, SLOT(ShowData()));
    }
}

void PlotSimpleListDlg::ShowData() {
    if (mDisplayType == NDTXrefs || mDisplayType == NDTXrefFor) {
        if (!mPlot) return;

        if (mDisplayType == NDTXrefs) {
            CollectXrefs();
        } else if (mDisplayType == NDTXrefFor) {
            CollectXrefFor();
        }

        /*mPlot->InitIdDwgMax();
        if (!mHistory) {
            mPlot->LoadAddFiles();
        } else {
            mHistory->LoadAddFiles();
        }*/

        ui->leIdProject->setText(QString::number(mPlot->IdProject()));
        ui->leProjName->setText(gProjects->ProjectFullShortName(mPlot->IdProject()));

        ui->leIdPlot->setText(QString::number(mPlot->Id()));
        if (!mPlotHistory) {
            ui->leHistory->setText(QString::number(mPlot->DwgVersionMax()));
        } else {
            ui->leHistory->setText(QString::number(mPlotHistory->Num())
                                   + "/" + QString::number(mPlot->DwgVersionMax()));
        }
        ui->leSection->setText(mPlot->SectionConst());
        ui->leCode->setText(mPlot->CodeConst());
        ui->leSheet->setText(mPlot->SheetConst());

        ui->leEdited->setText(mPlot->EditDateConst().toString("dd.MM.yy HH:mm:ss"));

        ui->cbWorking->setChecked(mPlot->Working());
        ui->cbDeleted->setVisible(mPlot->Deleted());

        ui->leNameTop->setText(mPlot->NameTopConst());
        ui->leNameBottom->setText(mPlot->NameConst());

        /*ui->twAddFile->clear();
        foreach (PlotAddFileData * lAddFile, mHistory?mHistory->AddFilesConst():mPlot->AddFilesConst()) {
            PlotAddFilesTreeItem * lItem = new PlotAddFilesTreeItem(lAddFile);
            ui->twAddFile->addTopLevelItem(lItem);
            if (lAddFile->Id() == mCurrentItemId) ui->twAddFile->setCurrentItem(lItem, mCurrentColumn);
        }*/

        /*for (int i = 0; i < ui->twAddFile->topLevelItemCount(); i++) {
            if (mSelectedIds.contains(static_cast<PlotAddFilesTreeItem *>(ui->twAddFile->topLevelItem(i))->AddFileConst()->Id()))
                ui->twAddFile->topLevelItem(i)->setSelected(true);
        }
        mSelectedIds.clear();*/

        /*if (mScrollPosVert) {
            ui->twAddFile->verticalScrollBar()->setValue(mScrollPosVert);
            mScrollPosVert = 0;
        }

        if (mScrollPosHoriz) {
            ui->twAddFile->horizontalScrollBar()->setValue(mScrollPosHoriz);
            mScrollPosHoriz = 0;
        }*/
    }

    ui->twDocs->clear();

    int i;
    bool lHasAnyHist = false, lHasAnyOldHist = false;

    for (i = 0; i < mPlots.length(); i++) {
        PlotListTreeItem * pd = new PlotListTreeItem(ui->twDocs, mPlots.at(i).first, mPlots.at(i).second, 0);
        pd->setFlags((pd->flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
        if (mDisplayType == NDTVersions)
            pd->setFlags(pd->flags() | Qt::ItemIsUserCheckable);

        if ((mDisplayType == NDTXrefs || mDisplayType == NDTXrefFor)
                && i < mPlotAddData.length()) {
            pd->setText(ui->twDocs->Cols()[ui->twDocs->colXREFBlockName], mPlotAddData.at(i)->BlockName());
            pd->setCheckState(ui->twDocs->Cols()[ui->twDocs->colXREFUseWorking], mPlotAddData.at(i)->UseWorking()?Qt::Checked:Qt::Unchecked);
            if (mDisplayType == NDTXrefs) {
                if (mPlotAddData.at(i)->History() != mPlots.at(i).first->DwgVersionMax()) {
                    pd->setText(ui->twDocs->Cols()[ui->twDocs->colXREFHist],
                            QString::number(mPlotAddData.at(i)->History()) + "/" + QString::number(mPlots.at(i).first->DwgVersionMax()));
                    lHasAnyOldHist = true;
                }
            } else if (mDisplayType == NDTXrefFor) {
                pd->setText(ui->twDocs->Cols()[ui->twDocs->colXREFHist],
                        QString::number(mPlotAddData.at(i)->History()) + "/" + QString::number(mPlot->DwgVersionMax()));
                if (mPlotAddData.at(i)->History() != mPlot->DwgVersionMax()) {
                    lHasAnyOldHist = true;
                }
            }
        }

        ui->twDocs->addTopLevelItem(pd);

        switch (mDisplayType) {
        case NDTNewVersion:
            PlotData *lPlot = mPlots.at(i).first;
            PlotHistoryData *lHistory = mPlots.at(i).second;

            if (lHistory) lHasAnyHist = true;
            TreeDataRecord * lTreeDataRecord = gTreeData->FindById(lPlot->TDArea(), lPlot->TDId());

            if (lTreeDataRecord && lTreeDataRecord->ActualIdGroup() == 2) {
                // for xrefs (maybe change it in the future, use other types too; maybe use one flag in table "treedata")
                QString lDate = QDate::currentDate().toString("dd.MM.yy");
                pd->setText(ui->twDocs->Cols()[PlotListTree::colVersionIntNew], lDate);
                pd->setText(ui->twDocs->Cols()[PlotListTree::colVersionExtNew], lDate);
            } else {
                // internal version
                bool lIsLetter = false, lIsNumber = false;
                QChar lMaxChar = 0;
                int lMaxNumber = 0;
                lPlot->LoadVersions();

                for (int j = -1; j < lPlot->VersionsConst().length(); j++) {
                    const PlotData * lPlotData;
                    if (j == -1) {
                        lPlotData = lPlot;
                    } else {
                        lPlotData = lPlot->VersionsConst().at(j);
                    }
                    if (lPlotData->Deleted()) continue; // skip deleted
                    if (lPlotData->VersionIntConst().length() == 1
                            && (lPlotData->VersionIntConst()[0] >= 'A'
                                && lPlotData->VersionIntConst()[0] <= 'Z'
                                || lPlotData->VersionIntConst()[0] >= 'А'
                                && lPlotData->VersionIntConst()[0] <= 'Я'
                                || lPlotData->VersionIntConst()[0] >= 'א'
                                && lPlotData->VersionIntConst()[0] <= 'ת')) {

                        lIsLetter = true;
                        if (lPlotData->VersionIntConst()[0] > lMaxChar) {
                            lMaxChar = lPlotData->VersionIntConst()[0];
                        }
                    }

                    for (int k = 0; k < lPlotData->VersionIntConst().length(); k++) {
                        bool lNumbersOnly = true;
                        if (!(lPlotData->VersionIntConst()[k] >= '0'
                              && lPlotData->VersionIntConst()[k] <= '9')) {
                            lNumbersOnly = false;
                            break;
                        }

                        if (lNumbersOnly) {
                            lIsNumber = true;
                            if (lPlotData->VersionIntConst().toInt() > lMaxNumber) {
                                lMaxNumber = lPlotData->VersionIntConst().toInt();
                            }
                        }
                    }
                }

                // +1 - always use next
                if (lIsNumber) {
                    pd->setText(ui->twDocs->Cols()[PlotListTree::colVersionIntNew], QString::number(lMaxNumber + 1));
                } else if (lIsLetter) {
                    pd->setText(ui->twDocs->Cols()[PlotListTree::colVersionIntNew], QChar(lMaxChar.unicode() + 1));
                }


                // external version
                lIsLetter = false;
                lIsNumber = false;
                lMaxChar = 0;
                lMaxNumber = 0;

                bool lIsLetterSent = false, lIsNumberSent = false;
                QChar lMaxCharSent = 0;
                int lMaxNumberSent = 0;

                for (int j = -1; j < lPlot->VersionsConst().length(); j++) {
                    const PlotData * lPlotData;
                    if (j == -1) {
                        lPlotData = lPlot;
                    } else {
                        lPlotData = lPlot->VersionsConst().at(j);
                    }
                    if (lPlotData->Deleted()) continue; // skip deleted
                    if (lPlotData->VersionExtConst().length() == 1
                            && (lPlotData->VersionExtConst()[0] >= 'A'
                                && lPlotData->VersionExtConst()[0] <= 'Z'
                                || lPlotData->VersionExtConst()[0] >= 'А'
                                && lPlotData->VersionExtConst()[0] <= 'Я'
                                || lPlotData->VersionExtConst()[0] >= 'א'
                                && lPlotData->VersionExtConst()[0] <= 'ת')) {

                        if (lPlotData->SentDateConst().isNull()) {
                            lIsLetter = true;
                            if (lPlotData->VersionExtConst()[0] > lMaxChar) {
                                lMaxChar = lPlotData->VersionExtConst()[0];
                            }
                        } else {
                            lIsLetterSent = true;
                            if (lPlotData->VersionExtConst()[0] > lMaxCharSent) {
                                lMaxCharSent = lPlotData->VersionExtConst()[0];
                            }
                        }
                    }

                    for (int k = 0; k < lPlotData->VersionExtConst().length(); k++) {
                        bool lNumbersOnly = true;
                        if (!(lPlotData->VersionExtConst()[k] >= '0'
                              && lPlotData->VersionExtConst()[k] <= '9')) {
                            lNumbersOnly = false;
                            break;
                        }

                        if (lNumbersOnly) {
                            if (lPlotData->SentDateConst().isNull()) {
                                lIsNumber = true;
                                if (lPlotData->VersionExtConst().toInt() > lMaxNumber) {
                                    lMaxNumber = lPlotData->VersionExtConst().toInt();
                                }
                            } else {
                                lIsNumberSent = true;
                                if (lPlotData->VersionExtConst().toInt() > lMaxNumberSent) {
                                    lMaxNumberSent = lPlotData->VersionExtConst().toInt();
                                }
                            }
                        }
                    }
                }

                QString lNewVerExt;

                if (lIsNumberSent) {
                    lNewVerExt = QString::number(lMaxNumberSent + 1);
                } else if (lIsLetterSent) {
                    lNewVerExt = QChar(lMaxCharSent.unicode() + 1);
                } else if (lIsNumber) {
                    // use current - it is not sent yet
                    lNewVerExt = QString::number(lMaxNumber);
                } else if (lIsLetter) {
                    // use current - it is not sent yet
                    lNewVerExt = lMaxChar;
                }

                ProjectData *lProject = gProjects->FindByIdProject(lPlot->IdProject());
                if (lProject)
                    while (lNewVerExt.length() < lProject->ProjectType()->VerLen()) lNewVerExt.prepend('0');

                pd->setText(ui->twDocs->Cols()[PlotListTree::colVersionExtNew], lNewVerExt);

                QString lCode = lPlot->CodeConst();
                lPlot->SetPropWithCodeForming(PlotData::PPWCVersionExt, lPlot->VersionExtConst(), lNewVerExt, lCode);
                pd->setText(ui->twDocs->Cols()[PlotListTree::colCode], lCode);
            }
            break;
        }
    }

    ui->twDocs->setColumnHidden(ui->twDocs->Cols()[ui->twDocs->colIdHist], !lHasAnyHist);
    ui->twDocs->setColumnHidden(ui->twDocs->Cols()[ui->twDocs->colHist], !lHasAnyHist);

    switch (mDisplayType) {
    case NDTNewVersion:
        break;
    case NDTVersions:
        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colWorking], false); // always show
        if (!mPlots.isEmpty()) {
            PlotData *lPlot = mPlots.at(0).first;
            for (i = 0; i < lPlot->VersionsConst().length(); i++) {
                if (lPlot->VersionsConst().at(i)->Deleted()) continue;
                PlotListTreeItem * pd = new PlotListTreeItem(ui->twDocs, lPlot->VersionsConst().at(i), NULL, 0);
                //pd->setFlags((pd->flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
                pd->setFlags(pd->flags() | Qt::ItemIsUserCheckable); // need to check working flag
                ui->twDocs->addTopLevelItem(pd);
            }
        } else {
            QMessageBox::critical(this, tr("Versions"), tr("List is empty!"));
        }
        break;
    case NDTXrefs:
    case NDTXrefFor:
        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[ui->twDocs->colXREFHist], !lHasAnyOldHist);
        break;
    }

    if (mCurrentItemId) {
        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            if (static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i))->PlotConst()
                    && static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i))->PlotConst()->Id() == mCurrentItemId) {
                ui->twDocs->setCurrentItem(ui->twDocs->topLevelItem(i), mCurrentColumn);
                break;
            }
        }
        mCurrentItemId = 0;
    }

    if (!mSelectedPlotIds.isEmpty()) {
        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            if (static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i))->PlotConst()
                    && mSelectedPlotIds.contains(static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i))->PlotConst()->Id())) {
                ui->twDocs->topLevelItem(i)->setSelected(true);
            }
        }
        mSelectedPlotIds.clear();
    }

    if (gSettings->DocumentTree.AutoWidth)
        for (int i = 0; i < ui->twDocs->columnCount(); i++) {
            ui->twDocs->resizeColumnToContents(i);
            if (!i) {
                ui->twDocs->setColumnWidth(i, ui->twDocs->columnWidth(i) + 20);
            }
        }
}

void PlotSimpleListDlg::CollectXrefs() {
    if (mDisplayType != NDTXrefs) return;

    mPlots.clear();
    qDeleteAll(mPlotAddData);
    mPlotAddData.clear();

    if (!mPlotHistory) mPlot->InitIdDwgMax();

    QSqlQuery query(db);
    query.prepare(
                QString("select c.id_project id_project, b.id_plot id_plot, b.version history,"
                " coalesce(a.block_name, c.block_name, 'xref' || trim(")
                + ((db.driverName()== "QPSQL")?"cast(c.id as varchar)":"to_char(c.id)")
                + ")) block_name, a.use_working use_working"
                " from v_xref2dwg a, v_dwg b, v_plot_simple c"
                " where a.id_dwg_main = :id_dwg_main"
                " and a.id_dwg_xref = b.id"
                " and b.id_plot = c.id");
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(tr("Xrefs"), query);
    } else {
        query.bindValue(":id_dwg_main", mPlotHistory?mPlotHistory->Id():mPlot->IdDwgMax());
        if (!query.exec()) {
            gLogger->ShowSqlError(tr("Xrefs"), query);
        } else {
            bool lDiffProj = false, lHasUseNonWokring = false, lHasNonWokring = false;
            while (query.next()) {
                ProjectData *lProject = gProjects->FindByIdProject(qInt("id_project"));
                if (lProject) {
                    PlotData *lPlot = lProject->GetPlotById(qInt("id_plot"), true);
                    if (lPlot) {
                        mPlots.append(qMakePair(lPlot, static_cast<PlotHistoryData *>(NULL)));
                        mPlotAddData.append(new PlotAddData(qString("block_name"), qInt("use_working") == 1, qInt("history")));
                        if (lPlot->IdProject() != mPlot->IdProject()) lDiffProj = true;
                        if (!qInt("use_working")) lHasUseNonWokring = true;
                        if (!lPlot->Working()) lHasNonWokring = true;
                    }
                }
            }
            //QMessageBox::critical(NULL, QString::number(ui->twDocs->colWorking), QString::number(ui->twDocs->Cols()[ui->twDocs->colWorking]));
            ui->twDocs->HideColumn(ui->twDocs->colXREFUseWorking, !lHasUseNonWokring);
            ui->twDocs->HideColumn(ui->twDocs->colWorking, !lHasNonWokring);

            ui->twDocs->HideColumn(ui->twDocs->colIdProject, !lDiffProj);
            ui->twDocs->HideColumn(ui->twDocs->colProject, !lDiffProj);
        }
    }
}

void PlotSimpleListDlg::CollectXrefFor() {
    if (mDisplayType != NDTXrefFor) return;

    mPlots.clear();
    qDeleteAll(mPlotAddData);
    mPlotAddData.clear();

    //if (!mPlotHistory)
    mPlot->InitIdDwgMax();

    QSqlQuery query(db);
    query.prepare(
            QString("select e.id_project id_project, e.id id_plot, max(b.version) history,"
                " coalesce(c.block_name, b1.block_name, 'xref' || trim(")
                + ((db.driverName()== "QPSQL")?"cast(b1.id as varchar)":"to_char(b1.id)")
                + ")) block_name, c.use_working use_working"
              " from v_dwg a, v_dwg b, v_plot_simple b1, v_xref2dwg c,"
                " v_dwg d, v_plot_simple e"
            " where a.id = :id_dwg_xref"
            " and a.id_plot = b.id_plot"
            " and b.id_plot = b1.id"
            " and b.id = c.id_dwg_xref"
            " and (c.id_dwg_xref_version is null"
                 " or c.id_dwg_xref_version is not null"
                   " and b.version = c.id_dwg_xref_version)"
            " and c.id_dwg_main = d.id"
            " and d.id_plot = e.id"
            " and d.version = (select max(version) from v_dwg where id_plot = d.id_plot)"
            " group by e.id_project, e.id,"
                " coalesce(c.block_name, b1.block_name, 'xref' || trim("
                + ((db.driverName()== "QPSQL")?"cast(b1.id as varchar)":"to_char(b1.id)")
                + ")), c.use_working");
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(tr("Xref for"), query);
    } else {
        query.bindValue(":id_dwg_xref", mPlotHistory?mPlotHistory->Id():mPlot->IdDwgMax());
        if (!query.exec()) {
            gLogger->ShowSqlError(tr("Xref for"), query);
        } else {
            bool lDiffProj = false, lHasUseNonWokring = false, lHasNonWokring = false;
            while (query.next()) {
                ProjectData *lProject = gProjects->FindByIdProject(qInt("id_project"));
                if (lProject) {
                    PlotData *lPlot = lProject->GetPlotById(qInt("id_plot"), true);
                    if (lPlot) {
                        mPlots.append(qMakePair(lPlot, static_cast<PlotHistoryData *>(NULL)));
                        mPlotAddData.append(new PlotAddData(qString("block_name"), qInt("use_working") == 1, qInt("history")));
                        if (lPlot->IdProject() != mPlot->IdProject()) lDiffProj = true;
                        if (!qInt("use_working")) lHasUseNonWokring = true;
                        if (!lPlot->Working()) lHasNonWokring = true;
                    }
                }
            }
            //QMessageBox::critical(NULL, QString::number(ui->twDocs->colWorking), QString::number(ui->twDocs->Cols()[ui->twDocs->colWorking]));
            ui->twDocs->HideColumn(ui->twDocs->colXREFUseWorking, !lHasUseNonWokring);
            ui->twDocs->HideColumn(ui->twDocs->colWorking, !lHasNonWokring);

            ui->twDocs->HideColumn(ui->twDocs->colIdProject, !lDiffProj);
            ui->twDocs->HideColumn(ui->twDocs->colProject, !lDiffProj);
        }
    }
}

void PlotSimpleListDlg::Accept() {
    switch (mDisplayType) {
    case NDTNewVersion:
        int i;
        QList<PlotListTreeItem *> lForDel;
        XchgFileDataList lFilesForDel;

        if (ui->cbWhatToDo->currentIndex() == 2) {
            if (!mFileType) {
                gLogger->ShowError(this, tr("Creating new versions"), tr("Can't determine file type!"));
                return;
            }

            if (mFiles.length() != 1) {
                gLogger->ShowError(this, tr("Creating new versions"), tr("Input file not specified!"));
                return;
            }
        }

        if (ui->cbNewVerComment->currentText().isEmpty()) {
            QMessageBox::critical(this, tr("Creating new versions"), tr("Comment must be specified!"));
            ui->cbNewVerComment->setFocus();
            return;
        }

        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            ui->twDocs->topLevelItem(i)->setSelected(false);
        }

        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            PlotListTreeItem * lItem = static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i));
            PlotData *lPlotOld = lItem->PlotRef();
            if (lPlotOld->VersionIntConst().trimmed() == lItem->text(ui->twDocs->Cols()[PlotListTree::colVersionIntNew]).trimmed()) {
                QMessageBox::critical(this, tr("Creating new versions"), tr("Internal version is not unique!"));
                lItem->setSelected(true);
                ui->twDocs->setCurrentItem(lItem, ui->twDocs->Cols()[PlotListTree::colVersionIntNew]);
                return;
            }

            if (!lPlotOld->LoadVersions()) return; // error

            for (int j = 0; j < lPlotOld->VersionsConst().length(); j++) {
                PlotData *lPlotVersion = lPlotOld->VersionsConst().at(j);
                if (lPlotVersion->VersionIntConst().trimmed() == lItem->text(ui->twDocs->Cols()[PlotListTree::colVersionIntNew]).trimmed()) {
                    QMessageBox::critical(this, tr("Creating new versions"), tr("Internal version is not unique!"));
                    lItem->setSelected(true);
                    ui->twDocs->setCurrentItem(lItem, ui->twDocs->Cols()[PlotListTree::colVersionIntNew]);
                    return;
                }
            }
        }

        int lWhatToDo = ui->cbWhatToDo->currentIndex()?ui->cbWhatToDo->currentIndex():
                ((QMessageBox::question(this, tr("Creating new versions"), tr("Start editing of new versions?")) == QMessageBox::Yes)?1:0);

        mPlots.clear();

        MainDataForCopyToAcad lDataForAcad(2, false);

        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i));
            PlotData *lPlotDataOld = lItem->PlotRef();
            PlotHistoryData *lPlotHistoryDataOld = lItem->PlotHistoryRef();

            int lNewIdPlot = lItem->data(1, Qt::UserRole + 0).toInt(); // don't waist ids if was error
            int lNewIdDwg = lItem->data(1, Qt::UserRole + 1).toInt(); // don't waist ids if was error
            int lNewIdDwgEdit = lItem->data(1, Qt::UserRole + 2).toInt(); // don't waist ids if was error

            if ((lNewIdPlot || gOracle->GetSeqNextVal("plot_id_seq", lNewIdPlot))
                    && (lNewIdDwg || gOracle->GetSeqNextVal("dwg_id_seq", lNewIdDwg))
                    && (lNewIdDwgEdit || gOracle->GetSeqNextVal("dwg_edit_id_seq", lNewIdDwgEdit))) {

                lItem->setData(1, Qt::UserRole + 0, lNewIdPlot);
                lItem->setData(1, Qt::UserRole + 1, lNewIdDwg);

                if (!db.transaction()) {
                    gLogger->ShowSqlError(this, tr("Creating new versions"), tr("Can't start transaction"), db);
                } else {
                    bool lIsOk = false;
                    quint64 lMainIdDwg; // return value from PlotData::LOADFROMFILE when lWhatToDo == 2; in other case not needed

                    QSqlQuery qInsert(db);

                    qInsert.prepare(
                                "insert into v_plot_simple"
                                " (id, id_project, id_common, type_area, type, section, restrict_rights,"
                                " code, sheet_number, extension, nametop, name, version, version_ext, comments,"
                                " working, xrefmode, reason, block_name, id_work)"
                                " (select :id_new, id_project, id_common, type_area, type, section, restrict_rights,"
                                " code, sheet_number, extension, nametop, name, :version, :version_ext, comments,"
                                " 1, xrefmode, :reason, block_name, id_work"
                                " from v_plot_simple where id = :id_old)");
                    if (qInsert.lastError().isValid()) {
                        gLogger->ShowSqlError(this, tr("Creating new versions") + " - prepare v_plot_simple", qInsert);
                    } else {
                        qInsert.bindValue(":id_new", lNewIdPlot);
                        qInsert.bindValue(":version", lItem->text(ui->twDocs->Cols()[PlotListTree::colVersionIntNew]));
                        qInsert.bindValue(":version_ext", lItem->text(ui->twDocs->Cols()[PlotListTree::colVersionExtNew]));
                        qInsert.bindValue(":reason", ui->cbNewVerReason->currentIndex());
                        qInsert.bindValue(":id_old", lPlotDataOld->Id());
                        if (!qInsert.exec()) {
                            gLogger->ShowSqlError(this, tr("Creating new versions") + " - execute v_plot_simple", qInsert);
                        } else {
                            // insert new comment
                            qInsert.prepare("insert into v_plot_comments (id_plot, comments) values (:id_plot, :comments)");
                            if (qInsert.lastError().isValid()) {
                                gLogger->ShowSqlError(this, tr("Creating new versions") + " - prepare v_plot_comments", qInsert);
                            } else {
                                qInsert.bindValue(":id_plot", lNewIdPlot);
                                qInsert.bindValue(":comments", ui->cbNewVerComment->currentText());
                                if (!qInsert.exec()) {
                                    gLogger->ShowSqlError(this, tr("Creating new versions") + " - execute v_plot_comments", qInsert);
                                } else {
                                    // insert users rights
                                    qInsert.prepare("insert into v_plot_user(id_plot, login, admin_option)"
                                                    " select :id_new, login, admin_option"
                                                    " from v_plot_user where id_plot = :id_old");
                                    if (qInsert.lastError().isValid()) {
                                        gLogger->ShowSqlError(this, tr("Creating new versions") + " - prepare v_plot_user", qInsert);
                                    } else {
                                        qInsert.bindValue(":id_new", lNewIdPlot);
                                        qInsert.bindValue(":id_old", lPlotDataOld->Id());

                                        if (!qInsert.exec()) {
                                            gLogger->ShowSqlError(this, tr("Creating new versions") + " - execute v_plot_user", qInsert);
                                        } else {
                                            bool lMakeActive = false;
                                            int lOldIdDwg;
                                            if (lPlotHistoryDataOld) {
                                                lOldIdDwg = lPlotHistoryDataOld->Id();
                                            } else {
                                                lPlotDataOld->InitIdDwgMax();
                                                lOldIdDwg = lPlotDataOld->IdDwgMax();
                                            }

                                            if (lWhatToDo == 2) {
                                                // load from file
                                                XchgFileData *lXchgFileData = mFiles.at(i);

                                                if (mFileType->LoadMode() != 3
                                                        && lXchgFileData->FileInfoOrigConst().suffix().toLower() == "dwg") {
                                                    // AutoCAD drawing
                                                    if (!ProcessDwgsForLoad(lpClearAnnoScales | lpPurgeRegapps | lpExplodeAllProxies | lpRemoveAllProxies | lpAudit, 0, 0, 0, 0, "", "", winId())) return;
                                                }

                                                //gProjects->EmitProjectBeforeUpdate(aPlot->IdProject());

                                                if (PlotData::LOADFROMFILE(mFileType->LoadMode() != 3, lNewIdPlot, lMainIdDwg, lOldIdDwg, 0,
                                                                           lXchgFileData->FileInfoOrigConst(), mOrigFileSize, lXchgFileData->HashOrigConst(),
                                                                           *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                                                           lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef(), false, this)
                                                        && DwgData::CopyDwgXrefs(lOldIdDwg, lMainIdDwg)) {
                                                    lMakeActive = true;
                                                }
                                            } else {
                                                // dwg
                                                qInsert.prepare("insert into v_dwg (id, id_plot, extension, sha256, /*convert, */neednotprocess, nestedxrefmode, layout_cnt, ftime, InSubs, data)"
                                                                " (select :id_new, :id_new_plot, extension, sha256, /*convert, */neednotprocess, nestedxrefmode, layout_cnt, ftime, InSubs, data from v_dwg"
                                                                " where id = :id_old)");
                                                if (qInsert.lastError().isValid()) {
                                                    gLogger->ShowSqlError(this, tr("Creating new versions") + " - prepare v_dwg", qInsert);
                                                } else {
                                                    qInsert.bindValue(":id_new", lNewIdDwg);
                                                    qInsert.bindValue(":id_new_plot", lNewIdPlot);
                                                    qInsert.bindValue(":id_old", lOldIdDwg);

                                                    if (!qInsert.exec()) {
                                                        gLogger->ShowSqlError(this, tr("Creating new versions") + " - execute v_dwg", qInsert);
                                                    } else {
                                                        // dwg_edit
                                                        if (lWhatToDo == 1) {
                                                            qInsert.prepare("insert into dwg_edit(id, id_dwgin, id_dwgout, starttime)"
                                                                            " values(:id, :id_old, :id_new, current_timestamp)");
                                                        } else {
                                                            qInsert.prepare("insert into dwg_edit(id, id_dwgin, id_dwgout, starttime, endtime, savecount)"
                                                                            " values(:id, :id_old, :id_new, current_timestamp, current_timestamp, 1)");
                                                        }
                                                        if (qInsert.lastError().isValid()) {
                                                            gLogger->ShowSqlError(this, tr("Creating new versions") + " - prepare dwg_edit", qInsert);
                                                        } else {
                                                            qInsert.bindValue(":id", lNewIdDwgEdit);
                                                            qInsert.bindValue(":id_old", lOldIdDwg);
                                                            qInsert.bindValue(":id_new", lNewIdDwg);

                                                            if (!qInsert.exec()) {
                                                                gLogger->ShowSqlError(this, tr("Creating new versions") + " - execute dwg_edit", qInsert);
                                                            } else {
                                                                // copy xrefs
                                                                if (DwgData::CopyAllRefs(lOldIdDwg, lNewIdDwg)) {
                                                                    lMakeActive = true;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            if (lMakeActive) {
                                                ProjectData * lProjectData;
                                                if (lProjectData = gProjects->FindByIdProject(lPlotDataOld->IdProject())) {
                                                    PlotData * lPlotDataForDel;
                                                    if (lPlotDataForDel = lProjectData->GetPlotByIdCommon(lPlotDataOld->IdCommon())) {
                                                        lProjectData->PlotListRef().removeAll(lPlotDataForDel);
                                                        try {
                                                            PlotData * lPlotDataNew = new PlotData(lNewIdPlot);
                                                            // IsMainInited - it is flag that data loaded successfully
                                                            if (lPlotDataNew->IsMainInited()) {
                                                                if (lPlotDataNew->MakeVersionActive()) {
                                                                    delete lPlotDataForDel;
                                                                    lProjectData->PlotListRef().append(lPlotDataNew);
                                                                    lIsOk = true;
                                                                    mPlots.append(qMakePair(lPlotDataNew, static_cast<PlotHistoryData *>(NULL)));
                                                                }
                                                            }
                                                        } catch (...) {
                                                            gLogger->ShowError(this, tr("Creating new versions"), tr("Document data error"));
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (lIsOk) {
                        if (!db.commit()) {
                            gLogger->ShowSqlError(this, tr("Creating new versions"), tr("Can't commit"), db);
                            lIsOk = false;
                        } else {
                            if (lWhatToDo == 1) {
                                lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lNewIdPlot, 0, lNewIdDwgEdit, true));
                            } else if (lWhatToDo == 2) {
                                // local cache
                                XchgFileData *lXchgFileData = mFiles.at(i);

                                if (mFileType->LoadMode() != 3) {
                                    gSettings->SaveToLocalCache(lMainIdDwg, *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst());
                                }
                                for (int j = 0; j < lXchgFileData->AddFilesConst().length(); j++) {
                                    gSettings->SaveToLocalCache(lXchgFileData->AddFilesConst().at(j)->IdDwg(),
                                                                *lXchgFileData->AddFilesConst().at(j)->BinaryDataConst(),
                                                                lXchgFileData->AddFilesConst().at(j)->HashPrcdConst());
                                }

                                // ???
                                //aPlot->RefreshData();
                                //gProjects->EmitProjectNeedUpdate(aPlot->IdProject());
                                lFilesForDel.append(lXchgFileData);
                            }
                            lForDel.append(lItem);
                        }
                    }
                    if (!lIsOk) {
                        db.rollback();
                    }
                }
            }
        }

        qDeleteAll(lForDel);

        for (i = 0; i < lFilesForDel.length(); i++) {
            XchgFileData *lXchgFileData = lFilesForDel.at(i);
            mFiles.removeAll(lXchgFileData);
            delete lXchgFileData;
        }
        lFilesForDel.clear(); // it is necessary!

        if (!lDataForAcad.ListConst().isEmpty()) {
            gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
        }

        if (!ui->twDocs->topLevelItemCount()) accept();

        break;
    }
}

void PlotSimpleListDlg::TimerForUpdate() {
    mProjectForTimer->ReinitLists();
    //gProjects->EmitProjectNeedUpdate(mProjectForTimer->Id());
}

void PlotSimpleListDlg::ListItemChanged(QTreeWidgetItem * item, int column) {
    if (mDisplayType != NDTVersions) return;
    if (mInListItemCahnged) return;
    mInListItemCahnged = true;
    // "column" is fuckedup; need check that checked box state changed
    if (column == ui->twDocs->Cols()[PlotListTree::colWorking]
            && (item->checkState(column) == Qt::Checked
                && !static_cast<PlotListTreeItem *>(item)->PlotConst()->Working()
                || item->checkState(column) == Qt::Unchecked
                && static_cast<PlotListTreeItem *>(item)->PlotConst()->Working())) {
        if (item->checkState(column) != Qt::Checked) {
            // user can't uncheck, he must check other (unchecked) item
            item->setCheckState(column, Qt::Checked);
        } else {
            if (static_cast<PlotListTreeItem *>(item)->PlotConst()
                    && QMessageBox::question(this, tr("Versions"), tr("Make version") + " '" + static_cast<PlotListTreeItem *>(item)->PlotConst()->VersionIntConst() + "' " + tr("active?")) == QMessageBox::Yes) {
                for (int i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
                    if (ui->twDocs->topLevelItem(i) != item) {
                        ui->twDocs->topLevelItem(i)->setCheckState(column, Qt::Unchecked);
                    }
                }
                // make active muthafucku
                if (mProjectForTimer = gProjects->FindByIdProject(static_cast<PlotListTreeItem *>(item)->PlotConst()->IdProject())) {
                    static_cast<PlotListTreeItem *>(item)->PlotRef()->MakeVersionActive();
                    QMessageBox::information(this, tr("Versions"), tr("Done"));
                    //gProjects->EmitProjectBeforeUpdate(mProjectForTimer->Id());
                    QTimer::singleShot(0, this, SLOT(TimerForUpdate()));
                }
            } else {
                item->setCheckState(column, Qt::Unchecked);
            }
        }
    }
    mInListItemCahnged = false;
}

void PlotSimpleListDlg::DoSettingsChanged() {
    ShowData();
}

void PlotSimpleListDlg::OnPlotListBeforeUpdate(int aIdProject) {
    int i;

    mProjectIds.clear();
    mPlotIds.clear();
    mHistoryIds.clear();

    mSelectedPlotIds.clear();
    /*if (ui->twDocs->topLevelItemCount()
            && (!aIdProject || static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(0))->PlotConst()
                && aIdProject == static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(0))->PlotConst()->IdProject())) {
        mIdProjects.append(static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(0))->PlotConst()->IdProject());*/

    for (i = 0; i < mPlots.length(); i++) {
        const PlotAndHistoryData &lPlot = mPlots.at(i);
        mProjectIds.append(lPlot.first->IdProject());
        if (mDisplayType != NDTVersions) {
            mPlotIds.append(lPlot.first->Id());
        } else {
            mPlotIds.append(lPlot.first->IdCommon());
        }
        if (lPlot.second) {
            mHistoryIds.append(lPlot.second->Id());
        } else {
            mHistoryIds.append(0);
        }
    }

    for (int i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        if (ui->twDocs->topLevelItem(i)->isSelected()) {
            mSelectedPlotIds.append(static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i))->PlotConst()->Id());
        }
    }
    if (ui->twDocs->currentItem()) {
        mCurrentItemId = static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()->Id();
        mCurrentColumn = ui->twDocs->currentColumn();
    } else {
        mCurrentItemId = 0;
    }
    //}
}

void PlotSimpleListDlg::OnPlotListNeedUpdate(int aIdProject) {
    if (!aIdProject || mProjectIds.contains(aIdProject)) {
        mPlots.clear();
        for (int i = 0; i < mPlotIds.length(); i++) {
            ProjectData * lProject = gProjects->FindByIdProject(mProjectIds.at(i));
            if (lProject) {
                PlotData * lPlot;
                if (mDisplayType != NDTVersions) {
                    lPlot = lProject->GetPlotById(mPlotIds.at(i), true);
                } else {
                    lPlot = lProject->GetPlotByIdCommon(mPlotIds.at(i));
                }
                if (lPlot) {
                    if (!mHistoryIds.at(i)) {
                        mPlots.append(qMakePair(lPlot, static_cast<PlotHistoryData *>(NULL)));
                    } else {
                        mPlots.append(qMakePair(lPlot, lPlot->GetHistoryById(mHistoryIds.at(i))));
                    }
                }
            }
        }
        ShowData();
    }
    mProjectIds.clear();
    mPlotIds.clear();
    mHistoryIds.clear();
}

void PlotSimpleListDlg::on_twDocs_itemSelectionChanged() {
    if (ui->twDocs->currentItem()) {
        PlotData *lPlot = static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotRef();
        if (!gProjects->IsPlotListInUpdate(lPlot->IdProject())) {
            if ((mDisplayType == NDTXrefs || mDisplayType == NDTXrefFor)
                    && ui->cbAutoUpdate->isChecked()) {
                disconnect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(PlotSelectionChanged(PlotData *)));
                disconnect(gProjects, SIGNAL(PlotHistoryBecameSelected(PlotData *, PlotHistoryData *)), this, SLOT(PlotHistorySelectionChanged(PlotData *, PlotHistoryData *)));
            }
            emit gProjects->PlotBecameSelected(lPlot);
            if ((mDisplayType == NDTXrefs || mDisplayType == NDTXrefFor)
                    && ui->cbAutoUpdate->isChecked()) {
                connect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(PlotSelectionChanged(PlotData *)));
                connect(gProjects, SIGNAL(PlotHistoryBecameSelected(PlotData *, PlotHistoryData *)), this, SLOT(PlotHistorySelectionChanged(PlotData *, PlotHistoryData *)));
            }
        }
    }
    // it is just copy from main documents window
    //    if (mJustLoaded) return;
//    if (ui->twType->ProjectConst()
//            && gProjects->IsProjectInUpdate(ui->twType->ProjectConst()->Id())) return;

//    // item selection changed
//    if (ui->twDocs->currentItem()) {
//        // emit "new selected plot"
//        if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryConst()) {
//            emit gProjects->PlotHistoryBecameSelected(static_cast<PlotListTreeItem *>(ui->twDocs->currentItem()->parent())->PlotRef(),
//                                                      static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryRef());
//        } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()) {
//            emit gProjects->PlotBecameSelected(static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotRef());
//        }
//    }

//    QList<QTreeWidgetItem *> lSelected = ui->twDocs->selectedItems();
//    QList<PlotAndHistoryData> lData;

//    for (int i = 0; i < lSelected.length(); i++) {
//        if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()) {
//            // static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryRef() IS NULL IN FACT
//            lData.append(qMakePair(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotRef(), static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryRef()));
//        } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()
//                   && lSelected.at(i)->parent()) {
//            lData.append(qMakePair(static_cast<PlotListTreeItem *>(lSelected.at(i)->parent())->PlotRef(), static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryRef()));
//        }
//    }

//    emit gProjects->PlotsBecameSelected(lData);

}

void PlotSimpleListDlg::PlotSelectionChanged(PlotData * aPlot) {
    mPlot = aPlot;
    mPlotHistory = NULL;
    ShowData();
}

void PlotSimpleListDlg::PlotHistorySelectionChanged(PlotData * aPlot, PlotHistoryData * aHistory) {
    mPlot = aPlot;
    mPlotHistory = aHistory;
    ShowData();
}

void PlotSimpleListDlg::on_cbAutoUpdate_toggled(bool checked) {
    if (checked) {
        connect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(PlotSelectionChanged(PlotData *)));
        connect(gProjects, SIGNAL(PlotHistoryBecameSelected(PlotData *, PlotHistoryData *)), this, SLOT(PlotHistorySelectionChanged(PlotData *, PlotHistoryData *)));
    } else {
        disconnect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(PlotSelectionChanged(PlotData *)));
        disconnect(gProjects, SIGNAL(PlotHistoryBecameSelected(PlotData *, PlotHistoryData *)), this, SLOT(PlotHistorySelectionChanged(PlotData *, PlotHistoryData *)));
    }
}

void PlotSimpleListDlg::DoSelectColumns(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    QList<int> lDis;

    lDis.append(ui->twDocs->Cols()[ui->twDocs->colID]);
    if (mDisplayType == NDTVersions) {
        lDis.append(ui->twDocs->Cols()[ui->twDocs->colWorking]);
    }
    if (mDisplayType == NDTVersions
            || mDisplayType == NDTNewVersion) {
        lDis.append(ui->twDocs->Cols()[ui->twDocs->colVersionInt]);
        lDis.append(ui->twDocs->Cols()[ui->twDocs->colVersionExt]);
    }
    if (mDisplayType == NDTNewVersion) {
        lDis.append(ui->twDocs->Cols()[ui->twDocs->colVersionIntNew]);
        lDis.append(ui->twDocs->Cols()[ui->twDocs->colVersionExtNew]);
    }

    w.SetHeaderView(ui->twDocs->header());
    w.SetDisabledIndexes(lDis);
    if (w.exec() == QDialog::Accepted) {
//        if (gSettings->DocumentHistory.AutoWidth) {
//            for (int i = 0; i < ui->twHist->columnCount(); i++) {
//                if (i != 6) {
//                    ui->twHist->resizeColumnToContents(i);
//                }
//            }
//        }
    }
}

void PlotSimpleListDlg::on_tbSelFile_clicked() {
    const PlotListTreeItem * lItem;
    const PlotData *lPlot;
    const TreeDataRecord *lTreeData;

    mFileType = NULL;
    if (ui->twDocs->topLevelItemCount() != 1
            || !(lItem = static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(0)))
            || !(lPlot = lItem->PlotConst())
            || !(lTreeData = gTreeData->FindById(lPlot->TDArea(), lPlot->TDId()))
            || !(mFileType = gFileTypeList->FindById(lTreeData->ActualFileType()))) {
        return;
    }

    QFileDialog dlg;

    dlg.setAcceptMode(QFileDialog::AcceptOpen);

    if (!mFileType->FileMasks_QTConst().isEmpty())
        dlg.setNameFilters(mFileType->FileMasks_QTConst().split(';'));

    switch (mFileType->LoadMode()) {
    case 0:
        dlg.setFileMode(QFileDialog::ExistingFile);
        break;
    case 1:
    case 3: // don't know the difference; i think 3 is not needed, but 1 not used now
        dlg.setFileMode(QFileDialog::DirectoryOnly);

        // recommended - not work as usual
        //dlg.setFileMode(QFileDialog::Directory);
        //dlg.setOption(QFileDialog::ShowDirsOnly, true);
        break;
    case 2:
        //dlg.setFileMode(QFileDialog::ExistingFiles);
        dlg.setFileMode(QFileDialog::ExistingFile);
        break;
    }

    if (!ui->leFilename->text().isEmpty())
        dlg.selectFile(ui->leFilename->text());
    else
        dlg.setDirectory(gSettings->LoadFiles.LastDir);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        if (files.length() == 1) {
            if (mFileType->LoadMode() != 3) {
                // file selected, save dir
                gSettings->LoadFiles.LastDir = files.at(0).left(files.at(0).lastIndexOf('/'));
            } else {
                // dir selected, save it
                gSettings->LoadFiles.LastDir = files.at(0);
            }

            ui->leFilename->setText(files.at(0));

            qDeleteAll(mFiles);
            mFiles.clear();

            mExistingIds.clear();
            ui->pbAlreadyInBase->setVisible(false);
            ui->lblAcadVersion->setVisible(false);

            XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(files.at(0)));
            if (!gFileUtils->InitDataForLoad(mFileType->LoadMode() != 3, *lXchgFileData, mOrigFileSize)) {
                delete lXchgFileData;
                return;
            }

            if (mFileType->LoadMode() == 3) {
                if (lXchgFileData->AddFilesConst().isEmpty()) {
                    QMessageBox::critical(this, tr("Loading document"), tr("No files exist in this directory!"));
                    delete lXchgFileData;
                    return;
                }
            }

            if (!gOracle->CollectAlreadyLoaded(lXchgFileData->HashOrigConst(), mExistingIds)) {
                delete lXchgFileData;
                return;
            }

            mFiles.append(lXchgFileData);

            if (!mExistingIds.isEmpty()) {
                ui->pbAlreadyInBase->setVisible(true);
                if (mExistingIds.count() == 1) {
                    // single, just
                    ui->pbAlreadyInBase->setText(QString::number(mExistingIds.at(0).first.first) + "/" + QString::number(mExistingIds.at(0).first.second) + " - "
                                                 + mExistingIds.at(0).second);
                    ui->pbAlreadyInBase->setArrowType(Qt::NoArrow);

                } else {
                    // submenu
                    if (mExistsListMenu) delete mExistsListMenu;
                    mExistsListMenu = new QMenu(this);
                    for (int i = 0; i < mExistingIds.length(); i++) {
                        const tPairIntIntString &mExistingId = mExistingIds.at(i);
                        QAction *lAction;
                        lAction = mExistsListMenu->addAction(QString::number(mExistingId.first.first) + "/" + QString::number(mExistingId.first.second) + " - "
                                                             + mExistingId.second);
                        lAction->setCheckable(true);
                    }
                    mExistsListMenu->addSeparator();
                    ui->actionGo_to_selected->setEnabled(false);
                    mExistsListMenu->addAction(ui->actionGo_to_selected);

                    ui->pbAlreadyInBase->setText(tr("Already loaded"));
                    ui->pbAlreadyInBase->setArrowType(Qt::DownArrow);
                }

            } else {
                ui->pbAlreadyInBase->setVisible(false);
            }

            if (lXchgFileData->AcadVersionOrig()) {
                ui->lblAcadVersion->setText("AutoCAD: " + QString::number(lXchgFileData->AcadVersionOrig()));
                ui->lblAcadVersion->setVisible(true);
            } else {
                ui->lblAcadVersion->setVisible(false);
            }
        }
    }
}

void PlotSimpleListDlg::on_cbWhatToDo_currentIndexChanged(int index) {
    bool lPrev = ui->wdSelectFile->isVisible();
    ui->wdSelectFile->setVisible(index == 2);
    if (!lPrev
            && ui->wdSelectFile->isVisible()
            && ui->leFilename->text().isEmpty()) {
        // select file
        on_tbSelFile_clicked();
    }
}

void PlotSimpleListDlg::on_pbAlreadyInBase_clicked() {
    int i;
    QList<int> lIndexes;

    if (mExistingIds.count() == 1
            && QMessageBox::question(this, tr("New document"), tr("Close this window and go to document")
                                     + " " + QString::number(mExistingIds.at(0).first.first) + "/" + QString::number(mExistingIds.at(0).first.second) + " - "
                                     + mExistingIds.at(0).second + "?") == QMessageBox::Yes) {
        lIndexes.append(0);
    } else {
        if (mExistsListMenu) {
            QAction *qActRes;
            do {
                qActRes = mExistsListMenu->exec(ui->pbAlreadyInBase->mapToGlobal(ui->pbAlreadyInBase->rect().bottomLeft()));

                bool b = false;
                for (i = 0; i < mExistsListMenu->actions().length(); i++) {
                    if (mExistsListMenu->actions().at(i)->isChecked()) {
                        b = true;
                        break;
                    }
                }
                ui->actionGo_to_selected->setEnabled(b);

            } while (qActRes && qActRes != ui->actionGo_to_selected);
            if (qActRes == ui->actionGo_to_selected
                    && QMessageBox::question(this, tr("New document"), tr("Close this window and go to selected documents?")) == QMessageBox::Yes)
                for (int i = 0; i < mExistsListMenu->actions().count(); i++)
                    if (mExistsListMenu->actions().at(i)->isChecked())
                        lIndexes.append(i);
        }
    }
    for (i = 0; i < lIndexes.length(); i++) {
        int lIndex = lIndexes.at(i);
        PlotData * lPlotGoto = gProjects->FindByIdPlot(mExistingIds.at(lIndex).first.first);
        if (lPlotGoto) {
            PlotHistoryData * lHistoryGoto = NULL;
            lPlotGoto->ReinitHistory(); // reinit, ya
            for (int j = 0; j < lPlotGoto->HistoryConst().length(); j++) {
                if (lPlotGoto->HistoryConst().at(j)->Num() == mExistingIds.at(lIndex).first.second) {
                    lHistoryGoto = lPlotGoto->HistoryConst().at(j);
                    break;
                }
            }
            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, lHistoryGoto);
        } else {
            QMessageBox::critical(this, tr("Project data"), tr("Can't find document with ID = ") + QString::number(mExistingIds.at(lIndex).first.first));
        }
    }
    if (!lIndexes.isEmpty())
        done(QDialog::Rejected);
}

bool PlotSimpleListDlg::nativeEvent(const QByteArray & eventType, void * message, long * result) {
    if (AcadXchgDialog::DoNativeEvent(eventType, message, result)) return true;
    return false;
}
