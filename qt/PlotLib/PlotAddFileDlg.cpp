#include "PlotAddFileDlg.h"
#include "ui_PlotAddFileDlg.h"

#include "PlotAddFilesTreeItem.h"

#include "DwgData.h"
#include "PlotData.h"

#include <QMdiSubWindow>
#include <QMenu>
#include <QFileDialog>
#include <QScrollBar>

#include "../UsersDlg/UserData.h"
#include "../ProjectLib/ProjectData.h"

#include "../VProject/BlobMemCache.h"
#include "../VProject/FileUtils.h"
#include "../VProject/PlotListItemDelegate.h"
#include "../VProject/SelectColumnsDlg.h"

#include "../VProject/GlobalSettings.h"

#include "../SaveLoadLib/VPImageViewer.h"

bool PlotAddFileDlg::mBlockUpdateSignals = false;

PlotAddFileDlg::PlotAddFileDlg(PlotData * aPlot, PlotHistoryData * aHistory, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotAddFileDlg),
    mVPImageViewer(NULL),
    mJustStarted(true),
    mPlot(aPlot), mHistory(aHistory), mPrevWidth(0), mCurrentItemId(0), mCurrentColumn(0),
    mScrollPosVert(0), mScrollPosHoriz(0)
{
    ui->setupUi(this);

    mIdPlot = mPlot->Id();
    mIdProject = mPlot->IdProject();
    if (mHistory)
        mNumHist = mHistory->Num();
    else
        mNumHist = 0;

    ui->cbAutoUpdate->setChecked(true);
    InitInConstructor();
}

PlotAddFileDlg::PlotAddFileDlg(QSettings &aSettings, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotAddFileDlg),
    mVPImageViewer(NULL),
    mJustStarted(true),
    mHistory(NULL),
    mIdPlot(0), mIdProject(0), mPrevWidth(0)
{
    ui->setupUi(this);

    mPlot = gProjects->FindByIdPlot(aSettings.value("Document ID", 0).toInt());
    mNumHist = aSettings.value("HistoryNum", 0).toInt();
    if (mPlot) {
        mIdPlot = mPlot->Id();
        mIdProject = mPlot->IdProject();
        if (mNumHist) {
            for (int i = 0; i < mPlot->HistoryConst().length(); i++) {
                if (mPlot->HistoryConst().at(i)->Num() == mNumHist) {
                    mHistory = mPlot->HistoryConst().at(i);
                    break;
                }
            }
        }
    }

    mCurrentItemId = aSettings.value("Current item", 0).toInt();
    mCurrentColumn = aSettings.value("Current column", 0).toInt();

    QStringList lSelectedIdsStr;
    lSelectedIdsStr = aSettings.value("Selected").toString().split(';');
    for (int i = 0; i < lSelectedIdsStr.length(); i++)
        mSelectedIds.append(lSelectedIdsStr.at(i).toInt());

    mScrollPosVert = aSettings.value("ScrollPosVert", 0).toInt();
    mScrollPosHoriz = aSettings.value("ScrollPosHoriz", 0).toInt();

    ui->cbAutoUpdate->setChecked(aSettings.value("Auto update", true).toBool());

    mPrevWidth = aSettings.value("PrevWidth", 0).toInt();

    InitInConstructor();
}

PlotAddFileDlg::~PlotAddFileDlg() {
    delete ui;
}

void PlotAddFileDlg::SaveState(QSettings &aSettings) {
    SaveSettings(aSettings);
    aSettings.setValue("Document ID", mIdPlot);
    aSettings.setValue("HistoryNum", mNumHist);

    if (!ui->twAddFile->currentItem()) {
        aSettings.setValue("Current item", 0);
    } else {
        aSettings.setValue("Current item", static_cast<PlotAddFilesTreeItem *>(ui->twAddFile->currentItem())->AddFileConst()->Id());
        aSettings.setValue("Current column", ui->twAddFile->currentColumn());
    }

    QList<QTreeWidgetItem *> lSelectedItems = ui->twAddFile->selectedItems();
    QStringList lSelectedIds;
    for (int i = 0; i < lSelectedItems.length(); i++) {
        lSelectedIds.append(QString::number(static_cast<PlotAddFilesTreeItem *>(lSelectedItems.at(i))->AddFileConst()->Id()));
    }
    aSettings.setValue("Selected", lSelectedIds.join(';'));

    aSettings.setValue("Auto update", ui->cbAutoUpdate->isChecked());

    aSettings.setValue("ScrollPosVert", ui->twAddFile->verticalScrollBar()->value());
    aSettings.setValue("ScrollPosHoriz", ui->twAddFile->horizontalScrollBar()->value());

    aSettings.setValue("PrevWidth", mPrevWidth);
}

void PlotAddFileDlg::InitInConstructor() {
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    // always selected in normal color (not gray)
    QPalette lPalette = ui->twAddFile->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->twAddFile->setPalette(lPalette);
    // ---------------------------------

    ui->twAddFile->setItemDelegate(new ROPlotListItemDelegate(ui->twAddFile));

    connect(gProjects, SIGNAL(PlotListBeforeUpdate(int)), this, SLOT(OnPlotListBeforeUpdate(int)));
    connect(gProjects, SIGNAL(PlotListNeedUpdate(int)), this, SLOT(OnPlotListNeedUpdate(int)));

    connect(gProjects, SIGNAL(PlotBeforeUpdate(PlotData *, int)), this, SLOT(OnPlotBeforeUpdate(PlotData *, int)));
    connect(gProjects, SIGNAL(PlotNeedUpdate(PlotData *, int)), this, SLOT(OnPlotNeedUpdate(PlotData *, int)));

    ui->twAddFile->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->twAddFile->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumns(const QPoint &)));

    if (!mPlot) {
        QTimer::singleShot(0, this, SLOT(close()));
    }
}

void PlotAddFileDlg::SaveCurrentStateForRefresh() {
    mSelectedIds.clear();
    QList<QTreeWidgetItem *> lSelected  = ui->twAddFile->selectedItems();
    for (int i = 0; i < lSelected.length(); i++) {
        PlotAddFilesTreeItem *lItem = static_cast<PlotAddFilesTreeItem *>(lSelected[i]);
        mSelectedIds.append(lItem->AddFileConst()->Id());
    }

    if (ui->twAddFile->currentItem()) {
        mCurrentItemId = static_cast<PlotAddFilesTreeItem *>(ui->twAddFile->currentItem())->AddFileConst()->Id();
    } else {
        mCurrentItemId = -1;
    }
    mCurrentColumn = ui->twAddFile->currentColumn();

    mScrollPosVert = ui->twAddFile->verticalScrollBar()->value();
    mScrollPosHoriz = ui->twAddFile->horizontalScrollBar()->value();
}

void PlotAddFileDlg::ShowData() {
    if (!mPlot) return;

    int i;

    mPlot->InitIdDwgMax();
    if (!mHistory) {
        mPlot->LoadAddFiles();
    } else {
        mHistory->LoadAddFiles();
    }

    ui->leIdProject->setText(QString::number(mPlot->IdProject()));
    ui->leProjName->setText(gProjects->ProjectFullShortName(mPlot->IdProject()));

    ui->leIdPlot->setText(QString::number(mPlot->Id()));
    if (!mHistory) {
        ui->leHistory->setText(QString::number(mPlot->DwgVersionMax()));
    } else {
        ui->leHistory->setText(QString::number(mHistory->Num())
                               + "/" + QString::number(mPlot->DwgVersionMax()));
    }
    ui->leSection->setText(mPlot->SectionConst());
    ui->leCode->setText(mPlot->CodeConst());
    ui->leSheet->setText(mPlot->SheetConst());
    ui->cbWorking->setChecked(mPlot->Working());
    ui->cbDeleted->setVisible(mPlot->Deleted());

    ui->leNameTop->setText(mPlot->NameTopConst());
    ui->leNameBottom->setText(mPlot->NameConst());

    ui->twAddFile->clear();
    for (i = 0; i < (mHistory?mHistory->AddFilesConst().length():mPlot->AddFilesConst().length()); i++) {
        PlotAddFileData * lAddFile = mHistory?mHistory->AddFilesConst().at(i):mPlot->AddFilesConst().at(i);
        PlotAddFilesTreeItem * lItem = new PlotAddFilesTreeItem(lAddFile);
        ui->twAddFile->addTopLevelItem(lItem);
        if (lAddFile->Id() == mCurrentItemId) ui->twAddFile->setCurrentItem(lItem, mCurrentColumn);
    }

    mCurrentItemId = 0;
    mCurrentColumn = 0;

    for (i = 0; i < ui->twAddFile->topLevelItemCount(); i++) {
        if (mSelectedIds.contains(static_cast<PlotAddFilesTreeItem *>(ui->twAddFile->topLevelItem(i))->AddFileConst()->Id()))
            ui->twAddFile->topLevelItem(i)->setSelected(true);
    }
    mSelectedIds.clear();

    if (mScrollPosVert) {
        ui->twAddFile->verticalScrollBar()->setValue(mScrollPosVert);
        mScrollPosVert = 0;
    }

    if (mScrollPosHoriz) {
        ui->twAddFile->horizontalScrollBar()->setValue(mScrollPosHoriz);
        mScrollPosHoriz = 0;
    }

//    if (gSettings->DocumentHistory.AutoWidth) {
//        for (int i = 0; i < ui->twHist->columnCount(); i++) {
//            if (i != 6) {
//                ui->twHist->resizeColumnToContents(i);
//            }
//        }
//    }
}

void PlotAddFileDlg::OnDeleteVPImageViewer(int) {
    mVPImageViewer = NULL;
}

void PlotAddFileDlg::OnPlotListBeforeUpdate(int aIdProject) {
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(this, "PlotAddFileDlg::OnPlotListBeforeUpdate",
                                                             QString::number(aIdProject) + ", mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);
    if (!aIdProject || aIdProject == mIdProject) {
        SaveCurrentStateForRefresh();
    }
}

void PlotAddFileDlg::OnPlotListNeedUpdate(int aIdProject) {
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(this, "PlotAddFileDlg::OnPlotListNeedUpdate", QString::number(aIdProject), false);
    if (!aIdProject || aIdProject == mIdProject) {
        if (mPlot = gProjects->FindByIdPlot(mIdPlot)) {
            if (mNumHist) {
                mHistory = mPlot->GetHistoryByNum(mNumHist);
            } else {
                mHistory = NULL;
            }
            ShowData();
        } else {
            QTimer::singleShot(0, this, SLOT(close()));
        }
    }
}

void PlotAddFileDlg::OnPlotBeforeUpdate(PlotData *aPlot, int aType) {
    if (mBlockUpdateSignals) return;
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(this, "PlotHistDlg::OnPlotBeforeUpdate", "mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);
    if (!aType
            || aType == 7 && aPlot->Id() == mIdPlot) {
        SaveCurrentStateForRefresh();
    }
}

void PlotAddFileDlg::OnPlotNeedUpdate(PlotData *aPlot, int aType) {
    if (mBlockUpdateSignals) return;
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(this, "PlotHistDlg::OnPlotNeedUpdate", "mJustStarted = " + (mJustStarted?QString("true"):QString("false")), false);
    if (!aType
            || aType == 7 && aPlot->Id() == mIdPlot) {
        mPlot = aPlot;
        mIdProject = mPlot->IdProject();
        if (mNumHist) {
            mHistory = mPlot->GetHistoryByNum(mNumHist);
        } else {
            mHistory = NULL;
        }
        ShowData();
    }
}

void PlotAddFileDlg::OnPlotBecameSelected(PlotData * aPlot) {
    if (aPlot) {
        mPlot = aPlot;
        mHistory = NULL;
        mIdPlot = mPlot->Id();
        mIdProject = mPlot->IdProject();
        mNumHist = 0;

        mBlockUpdateSignals = true;
        ShowData();
        mBlockUpdateSignals = false;
    }
}

void PlotAddFileDlg::OnPlotHistoryBecameSelected(PlotData * aPlot, PlotHistoryData * aHistory) {
    if (aPlot) {
        mPlot = aPlot;
        mHistory = aHistory;
        mIdPlot = mPlot->Id();
        mIdProject = mPlot->IdProject();
        if (mHistory)
            mNumHist = mHistory->Num();
        else
            mNumHist = 0;
        mBlockUpdateSignals = true;
        ShowData();
        mBlockUpdateSignals = false;
    }
}

void PlotAddFileDlg::DoSelectColumns(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    QList<int> lDis;
    lDis << 2; // filename

    w.SetHeaderView(ui->twAddFile->header());
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

void PlotAddFileDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        if (ReadVersion < CurrentVersion) {
            ui->twAddFile->setColumnWidth(0, 80); // ID ref
            ui->twAddFile->setColumnWidth(1, 80); // ID data
            ui->twAddFile->setColumnWidth(2, 320); // Filename
            ui->twAddFile->setColumnWidth(3, 80); // ext
            ui->twAddFile->setColumnWidth(4, 80); // version
            ui->twAddFile->setColumnWidth(5, 80); // size
            ui->twAddFile->setColumnWidth(6, 100); // file time

            ui->twAddFile->setColumnHidden(0, true);
            ui->twAddFile->setColumnHidden(1, true);
        }

        // did n't catch update
        mBlockUpdateSignals = true;
        ShowData();
        mBlockUpdateSignals = false;

        mJustStarted = false;
    }

    if (!mPrevWidth) mPrevWidth = width();
}

void PlotAddFileDlg::resizeEvent(QResizeEvent * event) {
    QFCDialog::resizeEvent(event);

    if (mPrevWidth) {
        int lNewWidth = ui->twAddFile->columnWidth(2) + width() - mPrevWidth;
        if (lNewWidth >= 320) ui->twAddFile->setColumnWidth(2, lNewWidth);
    }
    mPrevWidth = width();
}

void PlotAddFileDlg::StyleSheetChangedInSescendant() {
    if (gSettings->DocumentHistory.AutoWidth) {
        for (int i = 0; i < ui->twAddFile->columnCount(); i++) {
            if (i != 6) {
                ui->twAddFile->resizeColumnToContents(i);
            }
        }
    }
}

void PlotAddFileDlg::on_cbAutoUpdate_toggled(bool checked) {
    if (checked) {
        connect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(OnPlotBecameSelected(PlotData *)));
        connect(gProjects, SIGNAL(PlotHistoryBecameSelected(PlotData *, PlotHistoryData *)), this, SLOT(OnPlotHistoryBecameSelected(PlotData *, PlotHistoryData *)));
    } else {
        disconnect(gProjects, SIGNAL(PlotBecameSelected(PlotData *)), this, SLOT(OnPlotBecameSelected(PlotData *)));
        disconnect(gProjects, SIGNAL(PlotHistoryBecameSelected(PlotData *, PlotHistoryData *)), this, SLOT(OnPlotHistoryBecameSelected(PlotData *, PlotHistoryData *)));
    }
}

bool PlotAddFileDlg::CreateDwgCopy() {
    bool res = false;


    QSqlQuery qSelect(db);

    qSelect.prepare(QString("select count(a.id) from dwg_edit a"
        " where a.id_dwgout = :id_dwg and a.type = 2 and a.username = user and a.session_id =")
        + ((db.driverName() == "QPSQL")
        ?" cast(pg_backend_pid() as varchar)"
        :" dbms_session.unique_session_id"));


    if (qSelect.lastError().isValid()) {
        gLogger->ShowSqlError(tr("Create DWG copy") + " - prepare DWG_EDIT select", qSelect);
        return false;
    } else {
        qSelect.bindValue(":id_dwg", mPlot->IdDwgMax()); // here is always max
        if (!qSelect.exec()) {
            gLogger->ShowSqlError(tr("Create DWG copy") + " - execute DWG_EDIT select", qSelect);
            return false;
        } else {
            if (qSelect.next()) {
                if (qSelect.value(0).toInt()) {
                    return true; // already is
                } else {
                    int aIdDwgNew;

                    if (!gOracle->GetSeqNextVal("dwg_id_seq", aIdDwgNew)) {
                        return false;
                    }

                    QSqlQuery qInsert(db);
                    qInsert.prepare("insert into v_dwg (id, id_plot, id_plotedge, version, extension, sha256,"
                                    " /*convert, */neednotprocess, nestedxrefmode, layout_cnt, ftime, InSubs, data)"
                                    " select :id_dwg_new, id_plot, id_plotedge, version + 1, extension, sha256,"
                                    " /*convert, */neednotprocess, nestedxrefmode, layout_cnt, ftime, InSubs, data from v_dwg b where id = :id_dwg_old");
                    if (qInsert.lastError().isValid()) {
                        gLogger->ShowSqlError(tr("Create DWG copy") + " - prepare DWG insert", qInsert);
                    } else {
                        qInsert.bindValue(":id_dwg_new", aIdDwgNew);
                        qInsert.bindValue(":id_dwg_old", mPlot->IdDwgMax());

                        if (!qInsert.exec()) {
                            gLogger->ShowSqlError(tr("Create DWG copy") + " - execute DWG insert", qInsert);
                        } else {
                            qInsert.prepare(QString("insert into dwg_edit (type, session_id, id_dwgin, id_dwgout, lastsave) values (2,")
                                            + ((db.driverName() == "QPSQL")
                                            ?" cast(pg_backend_pid() as varchar)"
                                            :" dbms_session.unique_session_id")
                                            + ", :id_dwg_old, :id_dwg_new, current_timestamp)");
                            if (qInsert.lastError().isValid()) {
                                gLogger->ShowSqlError(tr("Create DWG copy") + " - prepare DWG_EDIT insert", qInsert);
                            } else {
                                qInsert.bindValue(":id_dwg_old", mPlot->IdDwgMax());
                                qInsert.bindValue(":id_dwg_new", aIdDwgNew);

                                if (!qInsert.exec()) {
                                    gLogger->ShowSqlError(tr("Create DWG copy") + " - execute DWG_EDIT insert", qInsert);
                                } else {
                                    if (res = DwgData::CopyAllRefs(mPlot->IdDwgMax(), aIdDwgNew)) {
                                        mPlot->InitIdDwgMax();
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                gLogger->ShowSqlError(tr("Create DWG copy") + " - fetch DWG_EDIT select", qSelect);
                return false;
            }
        }
    }

    return res;
}

void PlotAddFileDlg::View() {
    if (ui->twAddFile->currentItem()
            && static_cast<PlotAddFilesTreeItem *>(ui->twAddFile->currentItem())->AddFileConst()->IsPicture()) {
        switch (gSettings->Image.ViewerType) {
        case 0:
            if (mVPImageViewer) return;
            mVPImageViewer = new VPImageViewer(ui->twAddFile, this/*it is parent, can be gMainWindow*/);
            connect(mVPImageViewer, SIGNAL(finished(int)), this, SLOT(OnDeleteVPImageViewer(int)));
            mVPImageViewer->setAttribute(Qt::WA_DeleteOnClose);
            mVPImageViewer->show();
            break;
        case 1:
        case 2:
            //ImageViewerThread::ModalViewList(ui->twHist);
            break;
        }
        return;
    }
}

void PlotAddFileDlg::Save() {

    bool lIsFile;
    QFileDialog dlg;
    PlotAddFilesTreeItem * lItem;
    const PlotAddFileData * lAddFile;

    if (!mHistory || mHistory->Id() == mPlot->IdDwgMax()) {
        mPlot->InitEditStatus();
        if (mPlot->ES() == PlotData::PESEditing) {
            QMessageBox::critical(this, tr("Saving additional files"),
                                  QString::number(mPlot->Id()) + " - " + mPlot->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(mPlot->ESUserConst()));
            return;
        }
    }

    if (false && mHistory && mHistory->Id() != mPlot->IdDwgMax()) {
        // ask - want to save old files
    }

    QList <QTreeWidgetItem *> selected = ui->twAddFile->selectedItems();

    if (selected.count() == 1) {
        lItem = static_cast<PlotAddFilesTreeItem *>(selected.at(0));
        lAddFile = lItem->AddFileConst();

        dlg.setAcceptMode(QFileDialog::AcceptSave);
        dlg.setFileMode(QFileDialog::AnyFile);
        dlg.selectFile(lAddFile->NameConst());

        lIsFile = true;
    } else {
        dlg.setAcceptMode(QFileDialog::AcceptOpen);
        dlg.setFileMode(QFileDialog::DirectoryOnly);

        // recommended - not work as usual
        //dlg.setFileMode(QFileDialog::Directory);
        //dlg.setOption(QFileDialog::ShowDirsOnly, true);

        lIsFile = false;
    }
    dlg.setDirectory(gSettings->SaveFiles.LastDir);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        if (files.length() == 1) {
            if (lIsFile) {
                gSettings->SaveFiles.LastDir = files.at(0).left(files.at(0).lastIndexOf('/'));

                // no additional checking - we can save it. right?

                QFile file(files.at(0));
                if (file.open(QFile::WriteOnly)) {
                    file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, lAddFile->IdLob()));
                    file.close();
                    gFileUtils->SetFileTime(file.fileName(), lAddFile->FTimeConst());
                }
            } else {
                gSettings->SaveFiles.LastDir = files.at(0);

                QMessageBox::StandardButton lOverwrite = QMessageBox::No;

                for (int i = 0; i < selected.length(); i++) {
                    lItem = static_cast<PlotAddFilesTreeItem *>(selected.at(i));

                    if (lAddFile = lItem->AddFileConst()) {
                        QFile file(files.at(0) + "/" + lAddFile->NameConst());

                        if (file.exists()) {
                            if (lOverwrite == QMessageBox::NoToAll) continue;
                            if (lOverwrite != QMessageBox::YesToAll) {
                                // check by sha256

                                QMessageBox mb(this);
                                //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                                mb.setIcon(QMessageBox::Question);
                                mb.setWindowTitle(tr("Saving additional files"));
                                mb.setText(tr("File\n") + file.fileName() + tr("\nalready exists.\nOverwrite?"));
                                mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);

                                lOverwrite = (QMessageBox::StandardButton) mb.exec();
                                if (lOverwrite == QMessageBox::NoToAll
                                        || lOverwrite == QMessageBox::No) continue;
                            }
                        } else {
                            lOverwrite = QMessageBox::Yes;
                        }

                        if ((lOverwrite == QMessageBox::Yes || lOverwrite == QMessageBox::YesToAll)) {
                            if (file.open(QFile::WriteOnly)) {
                                file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, lAddFile->IdLob()));
                                file.close();
                                gFileUtils->SetFileTime(file.fileName(), lAddFile->FTimeConst());
                            } else {
                                gLogger->ShowError(this, tr("Saving additional files"),
                                                   tr("Error creating file") + ":\n" + file.fileName() + "\n" + tr("Error") +": " + file.errorString());
                            }
                        }
                    }
                }
            }
        }
    }
}

void PlotAddFileDlg::Load() {
    QFileDialog dlg(this);

    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);

    dlg.setDirectory(gSettings->SaveFiles.LastDir);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        //QString str;

        if (files.length())
            gSettings->SaveFiles.LastDir = files.at(0).left(files.at(0).lastIndexOf('/'));

        //bool lAnyExisting = false;


        if (!db.transaction()) {
            gLogger->ShowSqlError(this, tr("Loading additional files"), tr("Can't start transaction"), db);
            return;
        }

        bool lOk;
        bool lIsFirst = true;
        quint64 lFoundIdDwgXref;
        int lXrefVersion;
        bool lShaIsEqual;
        QStringList lSkipped, lLoaded;

        for (int i = 0; i < files.length(); i++) {
            QFile file(files.at(i));
            QFileInfo lAddFileInfo(file);
            if (file.open(QFile::ReadOnly)) {
                QByteArray lFileData(file.readAll());
                file.close();

                QCryptographicHash lHash(QCryptographicHash::Sha256);
                QString lHashStr;

                lHash.addData(lFileData);
                lHashStr = QString(lHash.result().toHex()).toUpper();

                // the same thing - IdDwgMax; doen't matter is it updated or not
                if (!gOracle->FindXref(mPlot->IdDwgMax(), lAddFileInfo.fileName(), lHashStr, lFoundIdDwgXref, lXrefVersion, lShaIsEqual)) {
                    lOk = false; // it was error
                    break;
                }

                if (lFoundIdDwgXref) {
                    if (lShaIsEqual) {
                        // exists, no changes
                        lSkipped.append(files.at(i).mid(files.at(i).lastIndexOf('/') + 1));
                        continue;
                    } else {
                        if (lIsFirst) {
                            lIsFirst = false;
                            lOk = CreateDwgCopy();
                            if (!lOk) break;
                            mPlot->ReinitHistory(); // copy created
                        }
                        // need to delete (mark with DELETED = 1) record in XREF with ID = lFoundIdXref
                        QSqlQuery qUpdate(db);
                        qUpdate.prepare("begin pp.deletefromxref(:id_dwg, :id_xref); end;");
                        if (qUpdate.lastError().isValid()) {
                            gLogger->ShowSqlError(this, tr("Loading additional files"), tr("Error loading file to Projects Base"), qUpdate);
                        } else {
                            qUpdate.bindValue(":id_dwg", mPlot->IdDwgMax());
                            qUpdate.bindValue(":id_xref", lFoundIdDwgXref);
                            if (!qUpdate.exec()) {
                                gLogger->ShowSqlError(this, tr("Loading additional files"), tr("Error loading file to Projects Base"), qUpdate);
                            }
                        }
                    }
                }

                if (lIsFirst) {
                    lIsFirst = false;
                    lOk = CreateDwgCopy();
                    if (!lOk) break;
                    mPlot->ReinitHistory(); // copy created
                }

                quint64 lIdDwg = 0;
                if (!DwgData::INSERT(lIdDwg, 0, 0, lAddFileInfo.suffix(), lHashStr, 0, -1, lAddFileInfo.lastModified(), &lFileData)
                        || !gOracle->InsertXref(mPlot->IdDwgMax(), lAddFileInfo.fileName(), lIdDwg, lXrefVersion)) {
                    lOk = false;
                    break;
                } else {
                    lLoaded.append(files.at(i).mid(files.at(i).lastIndexOf('/') + 1));
                }
            } else {
                gLogger->ShowError(this, tr("Loading additional files"),
                                  tr("Error opening file") + ":\n" + file.fileName() + "\n" + tr("Error") +": " + file.errorString());
                lOk = false;
                break;
            }
        }

        if (!lIsFirst) {
            if (lOk) {
                if (!db.commit()) {
                    gLogger->ShowSqlError(this, tr("Loading additional files"), tr("Can't commit"), db);
                } else {
                    // files loaded successfully
                    gProjects->EmitPlotListBeforeUpdate(mIdProject);
                    gProjects->EmitPlotListNeedUpdate(mIdProject);

                    // result message
                    QMessageBox mb(this);
                    QString lOut;
                    mb.setWindowTitle(tr("Loading additional files"));
                    mb.setIcon(QMessageBox::Information);

                    if (!lLoaded.isEmpty()) {
                        lOut = tr("Loaded") + ":\n" + lLoaded.join('\n');
                    }
                    if (!lSkipped.isEmpty()) {
                        if (!lOut.isEmpty()) lOut += "\n\n";
                        lOut += tr("Skipped (no changes)") + ":\n" + lSkipped.join('\n');
                    }

                    mb.setText(lOut);
                    mb.addButton(QMessageBox::Close);
                    //mb.setDefaultButton(mb.addButton(QMessageBox::No));
                    mb.exec();
                }
            } else {
                db.rollback();
            }
        } else {
            db.rollback();
            QMessageBox::critical(this, tr("Loading additional files"), tr("All files skipped - no changes"));
        }
        gOracle->Clean();
    }
}

void PlotAddFileDlg::Delete() {
    QList <QTreeWidgetItem *> selected = ui->twAddFile->selectedItems();

    if (!selected.count()) return; // some internal shit happened

    if (!mHistory || mHistory->Id() == mPlot->IdDwgMax()) {
        mPlot->InitEditStatus();
        if (mPlot->ES() == PlotData::PESEditing) {
            QMessageBox::critical(this, tr("Deleting additional files"),
                                  QString::number(mPlot->Id()) + " - " + mPlot->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(mPlot->ESUserConst()));
            return;
        }
    }

    if (QMessageBox::question(this, tr("Deleting additional files"),
                              (selected.count() == 1)?tr("Delete file") + " " + static_cast<PlotAddFilesTreeItem *>(selected.at(0))->AddFileConst()->NameConst() + "?":tr("Delete selected files?")) != QMessageBox::Yes) {
        return;
    }

    if (!db.transaction()) {
        gLogger->ShowSqlError(this, tr("Deleting additional files"), tr("Can't start transaction"), db);
        return;
    }

    gProjects->EmitPlotListBeforeUpdate(mIdProject);

    const PlotAddFileData * lAddFile;
    bool lOk = false;


    QSqlQuery qUpdate(db);
    bool lPrepared = false;

    for (int i = 0; i < selected.count(); i++) {
        lAddFile = static_cast<PlotAddFilesTreeItem *>(selected.at(i))->AddFileConst();
        if (!i) {
            lOk = CreateDwgCopy();
            if (!lOk) break;
        }

        // xref
        if (!lPrepared) {
            qUpdate.prepare("update xref set deleted = 1 where id_dwg = :id_dwg_main and id_xref = :id_xref");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(this, tr("Deleting additional files"), "Prepare - ", qUpdate);
                lOk = false;
                break;
            }
            lPrepared = true;
        }
        qUpdate.bindValue(":id_xref", lAddFile->IdLob());

        qUpdate.bindValue(":id_dwg_main", mPlot->IdDwgMax());
        if (!qUpdate.exec()) {
            gLogger->ShowSqlError(this, tr("Deleting additional files"), "Execute delete", qUpdate);
            lOk = false;
            break;
        }
        lOk = true;
    }

    if (lOk) {
        if (!db.commit()) {
            gLogger->ShowSqlError(this, tr("Saving additional files"), tr("Can't commit"), db);
        } else {
            gProjects->EmitPlotListNeedUpdate(mIdProject);
        }
    } else {
        db.rollback();
    }
}

void PlotAddFileDlg::on_twAddFile_customContextMenuRequested(const QPoint &pos) {
    QMenu lMenu(this);
    QAction *lARes, *lAView = NULL, *lASave = NULL, *lALoad = NULL, *lADelete = NULL, *lACopyToClipboard = NULL;
    QList <QTreeWidgetItem *> selected = ui->twAddFile->selectedItems();;

    if (ui->twAddFile->currentItem()) {
        lAView = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View"));
        lMenu.setDefaultAction(lAView);
    }

    if (selected.count()) lASave = lMenu.addAction(QIcon(":/some/ico/ico/SaveFromDatabase.png"), tr("Save..."));

    if (!mPlot->Deleted()
            && (!mHistory || mHistory->Id() == mPlot->IdDwgMax())
            && mPlot->SentDateConst().isNull()
            && !mPlot->EditNA()) {
        lALoad = lMenu.addAction(QIcon(":/some/ico/ico/LoadToDatabase.png"), tr("Load..."));
        if (selected.count())  {
            lMenu.addSeparator();
            lADelete = lMenu.addAction(QIcon(":/some/ico/ico/minus.png"), tr("Delete"));
        }
    }

    if (selected.count()) {
        lMenu.addSeparator();
        lACopyToClipboard = lMenu.addAction(QIcon(":/some/ico/ico/copy.png"), tr("Copy to clipboard"));
    }

    if (!lMenu.actions().isEmpty()
            && (lARes = lMenu.exec(QCursor::pos()))) {
        if (lARes == lAView) {
            View();
        } else if (lARes == lASave) {
            Save();
        } else if (lARes == lALoad) {
            Load();
        } else if (lARes == lADelete) {
            Delete();
        } else if (lARes == lACopyToClipboard) {
            gSettings->CopyToClipboard(ui->twAddFile);
        }
    }
}

void PlotAddFileDlg::on_twAddFile_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (mVPImageViewer
            && current) mVPImageViewer->ShowImage();
}

void PlotAddFileDlg::on_twAddFile_doubleClicked(const QModelIndex &index) {
    if (ui->twAddFile->currentItem()) {
        View();
    }
}
