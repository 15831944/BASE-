#include "SaveDialog.h"
#include "ui_SaveDialog.h"
#include "DocTreeSettings.h"
#include "SelectColumnsDlg.h"
#include "GlobalSettings.h"
#include "BlobMemCache.h"
#include "PlotListItemDelegate.h"
#include "FileUtils.h"

#include "common.h"
#include "oracle.h"

#include "../UsersDlg/UserRight.h"
#include "../ProjectLib/ProjectData.h"

#include "../PlotLib/DwgData.h"

#include <QFileDialog>
#include <QMenu>

SaveDialog::SaveDialog(QWidget *parent) :
    QFCDialog(parent, false), mJustStarted(true),
    ui(new Ui::SaveDialog),
    mMaxAcadVersionForProcess(0), mHasAcadDocs(false)
{
    CurrentVersion = 3;
    ui->setupUi(this);

    mStartMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->cbSaveAcad->addItem("No changes", 0);
    ui->cbSaveAcad->addItem("2007", 7);
    ui->cbSaveAcad->addItem("2010", 10);
    ui->cbSaveAcad->setCurrentIndex(0);

    SDDocItemDelegate *DocItemDelegate = new SDDocItemDelegate(ui->twDocs);
    ui->twDocs->setItemDelegate(DocItemDelegate);
    connect(DocItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(DocNameChanged(QWidget *)));

    // just for draw
    ui->twImages->setItemDelegate(new PlotListItemDelegate(ui->twFiles));

    SDXrefsItemDelegate *XrefsItemDelegate = new SDXrefsItemDelegate(ui->twXrefs);
    ui->twXrefs->setItemDelegate(XrefsItemDelegate);
    connect(XrefsItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(CurVarChanged(QWidget *)));

    SDFilesItemDelegate *FilesItemDelegate = new SDFilesItemDelegate(ui->twFiles);
    ui->twFiles->setItemDelegate(FilesItemDelegate);
    connect(FilesItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(IdChanged(QWidget *)));
}

SaveDialog::~SaveDialog()
{
    delete ui;
    qDeleteAll(mPlotForSaveList);
    qDeleteAll(mXrefForSaveForDel);

    gBlobMemCache->Clean();
}

void SaveDialog::Accept()
{
    int i, j;

    if (ui->lePathName->text().isEmpty()) {
        QMessageBox::critical(this, tr("Saving documents"), tr("Directory for saving must be specified!"));
        ui->lePathName->setFocus();
        return;
    }

    // it is for saving user select
    if (ui->twImages->isVisible() && (ui->twImages->currentColumn() != 0)) {
        QModelIndex mi = ui->twImages->currentIndex();
        ui->twImages->setCurrentIndex(mi.sibling(mi.row(), 0));
    }

    if (ui->twXrefs->isVisible() && (ui->twXrefs->currentColumn() != 0)) {
        QModelIndex mi = ui->twXrefs->currentIndex();
        ui->twXrefs->setCurrentIndex(mi.sibling(mi.row(), 0));
    }

    if (ui->twFiles->isVisible() && (ui->twFiles->currentColumn() != 0)) {
        QModelIndex mi = ui->twFiles->currentIndex();
        ui->twFiles->setCurrentIndex(mi.sibling(mi.row(), 0));
    }

    // start checking ---------------------------------------------------------------
    // main docs
    for (i = 0; i < ui->twDocs->topLevelItemCount() - 1; i++) {
        for (j = i + 1; j < ui->twDocs->topLevelItemCount(); j++) {
            if (ui->twDocs->topLevelItem(i)->text(1) == ui->twDocs->topLevelItem(j)->text(1)) {
                ui->tabDocs->setCurrentIndex(0);
                ui->twDocs->setFocus();
                ui->twDocs->setCurrentItem(ui->twDocs->topLevelItem(i));
                QMessageBox::critical(this, tr("Saving documents"), tr("Files names are duplicated!"));
                return;
            }
        }
    }

    QList<AcadParamData *> &lAcadParams = gSettings->AcadParamsRef();
    const InstalledAcadData *lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(lAcadParams.at(ui->cbUseAutocad->currentData().toInt())->FullProductNameConst());

    if (mMaxAcadVersionForProcess > lInstalledAcadData->Version()) {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Saving documents"));
        mb.setIcon(QMessageBox::Question);
        mb.setText("Minimum required AutoCAD version is " + QString::number(mMaxAcadVersionForProcess) + ".\n\n" + "Continue?");
        mb.addButton(QMessageBox::Yes);
        mb.setDefaultButton(mb.addButton(QMessageBox::No));
        if (mb.exec() != QMessageBox::Yes) return;
    }

    // xrefs
    if (ui->cbXrefs->isChecked()) {
        // check for duplicate names
        for (i = 0; i < ui->twXrefs->topLevelItemCount() - 1; i++) {
            QTreeWidgetItem * itemI = ui->twXrefs->topLevelItem(i);
            QString nameI = itemI->text(1).isEmpty()?itemI->text(0):itemI->text(1);
            for (j = i + 1; j < ui->twXrefs->topLevelItemCount(); j++) {
                QTreeWidgetItem * itemJ = ui->twXrefs->topLevelItem(j);
                QString nameJ = itemJ->text(1).isEmpty()?itemJ->text(0):itemJ->text(1);
                if (nameI == nameJ && itemI->childCount() && itemJ->childCount()) {
                    ui->twXrefs->setFocus();
                    ui->twXrefs->setCurrentItem(itemJ, itemJ->text(1).isEmpty()?0:1);

                    itemI->setBackgroundColor(itemI->text(1).isEmpty()?0:1, MY_COLOR_WARNING);
                    itemJ->setBackgroundColor(itemJ->text(1).isEmpty()?0:1, MY_COLOR_WARNING);

                    QMessageBox::critical(this, tr("Saving documents"), tr("Xref's' names are duplicated!\nYou must rename xref!"));
                    return;
                }
            }
        }
    }

    // images
    if (!(ui->cbMakeSubdir->isVisible() && ui->cbMakeSubdir->isChecked())
            && ImageListCheckForDuplicate()) {
        ui->tabDocs->setCurrentIndex(1);
        ui->twImages->setFocus();
        QMessageBox::critical(this, tr("Saving documents"), tr("Files names are duplicated!"));;
        return;
    }

    // ---------------------------------------------------------------
    // check edit status at the end - it is long operation
    // maybe need update status of all and then message and exit (now message and exit after first occuring)
    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        TWIForSaveMainDoc * item = (TWIForSaveMainDoc *) ui->twDocs->topLevelItem(i);
        // reinit edit status
        item->PlotRef()->InitEditStatus();
        item->ShowStatus(item->PlotRef()->ES(), item->PlotRef()->ESUserConst());
        if (item->PlotRef()->ES() == PlotData::PESEditing) {
            ui->tabDocs->setCurrentIndex(0);
            ui->twDocs->setFocus();
            ui->twDocs->setCurrentItem(item);
            QMessageBox::critical(this, tr("Saving documents"), tr("Some document(s) is editing right now!"));;
            return;
        }
    }

    if (ui->cbXrefs->isChecked()) {
        // check for edit status
        for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
            TWIForSaveXrefTop * item = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i));
            // reinit edit status
            item->XrefRef()->InitEditStatus();
            item->ShowStatus(item->XrefRef()->ES(), item->XrefRef()->ESUserConst());
            if (item->XrefRef()->ES() == PlotData::PESEditing) {
                ui->tabDocs->setCurrentIndex(2);
                ui->twXrefs->setFocus();
                ui->twXrefs->setCurrentItem(item);
                QMessageBox::critical(this, tr("Saving documents"), tr("Some xref(s) is editing right now!"));
                return;
            }
        }
    }

    // ---------------------------------------------------------------
    // start saving

    QDir saveDir(ui->lePathName->text());

    if (!saveDir.exists()) {
        if (QMessageBox::question(this, tr("Saving documents"), "Output directory doesn't exist. Create?") == QMessageBox::Yes) {
            if (!saveDir.mkpath(saveDir.path())) {
                QMessageBox::critical(this, tr("Saving documents"), tr("Can't create output directory!"));
                ui->lePathName->setFocus();
                return;
            }
        } else {
            // no create
            ui->lePathName->setFocus();
            return;
        }
    }

    bool lForAll = false, lOverwrite = false;

#define AskOverwriteMB \
    QMessageBox mb(this); \
    mb.setWindowTitle(tr("Saving documents")); \
    mb.setIcon(QMessageBox::Question); \
    mb.setText(tr("File\n") + file.fileName() + tr("\nalready exists.\n\nOverwrite?")); \
    mb.setCheckBox(new QCheckBox("For all")); \
    mb.addButton(QMessageBox::Yes); \
    mb.setDefaultButton(mb.addButton(QMessageBox::No));

    // Images
    for (i = 0; i < ui->twImages->topLevelItemCount(); i++) {
        TWIForSaveAddFile * item = (TWIForSaveAddFile *) ui->twImages->topLevelItem(i);

        if (item->RecordType() != TWIForSaveAddFile::AcadImage) continue;

        for (j = 0; j < 2; j++) {
            if (!j && item->DwgConst()->InMain()
                    || j && item->DwgConst()->InXref()) {
                if (!j && !saveDir.mkpath("Images")
                        || j && !saveDir.mkpath("Xrefs/Images")) {
                    QMessageBox::critical(this, tr("Saving documents"), tr("Can't create directory for images!"));
                    ui->lePathName->setFocus();
                    return;
                }

                QFile file;
                file.setFileName(saveDir.path() + ((!j)?"/Images/":"/Xrefs/Images/") + (item->text(1).isEmpty()?item->text(0):item->text(1)));

                if (file.exists()) {
                    if (gFileUtils->IsFileChanged(file.fileName(), item->DwgConst()->DataLength(), item->DwgConst()->Sha256Const())) {
                        if (!lForAll) {
                            AskOverwriteMB;
                            lOverwrite = mb.exec() == QMessageBox::Yes;
                            lForAll = mb.checkBox()->isChecked();
                        }
                    } else lOverwrite = false; // not changed
                } else lOverwrite = true; // not exists

                if (lOverwrite) {
                    if (file.open(QFile::WriteOnly)) {
                        file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, item->text(3).toInt()));
                        file.close();
                        gFileUtils->SetFileTime(file.fileName(), item->DwgConst()->FTimeConst());
                    } else {
                        QMessageBox::critical(this, tr("Saving documents"), tr("Error creating file") + "\n" + file.fileName() + "\n" + file.errorString());
                        return;
                    }
                }
            }
        }
    }

    // add. files
    if (ui->cbAddFiles->isChecked()) {
        for (i = 0; i < ui->twFiles->topLevelItemCount(); i++) {
            TWIForSaveAddFile * item = (TWIForSaveAddFile *) ui->twFiles->topLevelItem(i);
            QString lAddPath;

            if (item->DwgConst()->GroupNameConst() == "Acad:Text") {
                lAddPath = "/Fonts";
            } else if (item->DwgConst()->GroupNameConst() == "Acad:PlotStyle") {
                lAddPath = "/Plot Styles";
            } else if (item->DwgConst()->GroupNameConst() == "Acad:PlotConfig") {
                lAddPath = "/Plotters";
            }

            if (!lAddPath.isEmpty()) {
                if (!saveDir.mkpath(saveDir.path() + lAddPath)) {
                    QMessageBox::critical(this, tr("Saving documents"), tr("Can't create subdirectory") + " \"" + lAddPath.right(-1) + "\"!");
                    return;
                }
            }

            QFile file(saveDir.path() + lAddPath + "/" + item->text(0));
            if (file.exists()) {
                if (gFileUtils->IsFileChanged(file.fileName(), item->DwgConst()->DataLength(), item->DwgConst()->Sha256Const())) {
                    if (!lForAll) {
                        AskOverwriteMB;
                        lOverwrite = mb.exec() == QMessageBox::Yes;
                        lForAll = mb.checkBox()->isChecked();
                    }
                } else lOverwrite = false; // not changed
            } else lOverwrite = true; // not exists

            if (lOverwrite) {
                if (file.open(QFile::WriteOnly)) {
                    file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, item->text(2).toInt()));
                    file.close();
                    //SetFileTime(file.fileName(), query.value("filedate").toDateTime());
                } else {
                    QMessageBox::critical(this, tr("Saving documents"), tr("Error creating file") + "\n" + file.fileName() + "\n" + file.errorString());
                    return;
                }
            }
        }
    }

    // xrefs
    MainDataForCopyToAcad lDataForAcad(0);

    if (ui->cbSaveAcad->isVisible())
        lDataForAcad.SetSaveAcadVersion(ui->cbSaveAcad->currentData().toInt());

    if (ui->cbXrefs->isChecked()
            && ui->twXrefs->topLevelItemCount()) {
        if (!saveDir.mkpath("Xrefs")) {
            QMessageBox::critical(this, tr("Saving documents"), tr("Can't create directory for xrefs!"));
            ui->lePathName->setFocus();
            return;
        }
        for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
            // xref item for save
            TWIForSaveXrefTop * itemXref = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i));

            if (!itemXref->childCount()) continue; // user drag-drop some childs, no drawing with this xref

            BlobMemCache::RecordType opRT;
            int opID;
            bool lNeedAcadProcess = false;

            QFile file(saveDir.path() + "/Xrefs/" + (itemXref->text(1).isEmpty()?itemXref->text(0):itemXref->text(1)) + "." + ui->cbXrefExt->currentText());
            if (file.exists()) {
                // debug222 need checking that files unequal
                if (!lForAll) {
                    AskOverwriteMB;
                    lOverwrite = mb.exec() == QMessageBox::Yes;
                    lForAll = mb.checkBox()->isChecked();
                }
            } else lOverwrite = true;

            if (lOverwrite) {
                if (itemXref->XrefConst()->DwgConst()->NeedNotProcess() > 0) {
                    // need not process
                    opRT = BlobMemCache::Dwg;
                    opID = itemXref->XrefConst()->DwgConst()->Id();
                } else if (itemXref->XrefConst()->DwgConst()->IdCache() > 0) {
                    // get from cache
                    opRT = BlobMemCache::DwgCache;
                    opID = itemXref->XrefConst()->DwgConst()->IdCache();
                } else {
                    // get from DWG and NEED PROCESS
                    opRT = BlobMemCache::Dwg;
                    opID = itemXref->XrefConst()->DwgConst()->Id();
                    lNeedAcadProcess = true;
                }

                if (!lNeedAcadProcess
                        && !itemXref->XrefConst()->DwgConst()->InSubs()
                        && !itemXref->XrefConst()->ImagesConst().isEmpty()) {
                    lNeedAcadProcess = true; // need process images that is not in subdir
                }

                if (!itemXref->XrefPropsConst()->XrefPropsConst().isEmpty()) {
                    lNeedAcadProcess = true; // xref has properties (now we don't cache xrefs with applied properties, but we can)
                }

                if (file.open(QFile::WriteOnly)) {
                    file.write(gBlobMemCache->GetData(opRT, opID));
                    file.close();

                    gFileUtils->SetFileTime(file.fileName(), itemXref->XrefConst()->DwgConst()->FTimeConst());

                    QList<XrefRenameData *> lImageRenameList;

                    for (j = 0; j < ui->twImages->topLevelItemCount(); j++) {
                        TWIForSaveAddFile * itemImage = static_cast<TWIForSaveAddFile *>(ui->twImages->topLevelItem(j));

                        if (!itemImage->text(1).isEmpty() && itemImage->text(0) != itemImage->text(1)) {
                            for (int k = 0; k < itemImage->childCount(); k++) {
                                TWIForSaveAddFile * itemImageSub = static_cast<TWIForSaveAddFile *>(itemImage->child(k));
                                //??? debug111
                                if (itemImageSub->PlotConst()->Id() == itemXref->XrefConst()->Id()) {
                                    // add to xref rename list
                                    lImageRenameList.append(new XrefRenameData(itemImage->text(0), itemImage->text(1)));
                                    lNeedAcadProcess = true; // need process for rename images' files
                                }
                            }
                        }
                    }

                    if (lNeedAcadProcess
                            || ui->cbSaveAcad->isVisible()
                                && ui->cbSaveAcad->currentIndex() > 0
                                && itemXref->XrefConst()->DwgConst()->AcadVer() > ui->cbSaveAcad->currentData().toInt()) {
                        lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eSP, file.fileName(), 1, itemXref->XrefPropsRef(), lImageRenameList, itemXref->XrefConst()->DwgConst()->Id()));
                    }
                } else {
                    QMessageBox::critical(this, tr("Saving documents"), tr("Error creating file") + "\n" + file.fileName() + "\n" + file.errorString());
                    return;
                }
            }
        }
    }

    // main docs
    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        TWIForSaveMainDoc * itemDoc = (TWIForSaveMainDoc *) ui->twDocs->topLevelItem(i);

        if ((itemDoc->PlotConst()->FileType() < 20 || itemDoc->PlotConst()->FileType() > 29)
                && itemDoc->PlotConst()->DwgConst()->ExtensionConst().toLower() == "dwg") {
            // it is AutoCAD part

            QFile file(saveDir.path() + "/" + itemDoc->text(1));
            if (file.exists()) {
                if (!lForAll) {
                    AskOverwriteMB;
                    lOverwrite = mb.exec() == QMessageBox::Yes;
                    lForAll = mb.checkBox()->isChecked();
                }
            } else lOverwrite = true;

            if (lOverwrite) {
                // write dwg_file
                if (!gOracle->InsertDwgFile(itemDoc->PlotConst()->DwgConst()->Id(), true, file.fileName(), itemDoc->PlotConst()->DwgConst()->DataLength(),
                                            itemDoc->PlotConst()->DwgConst()->FTimeConst())) {
                    return;
                }

                // save file
                if (file.open(QFile::WriteOnly)) {
                    file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, itemDoc->PlotConst()->DwgConst()->Id()));
                    file.close();

                    gFileUtils->SetFileTime(file.fileName(), itemDoc->PlotConst()->DwgConst()->FTimeConst());


                    QList<XrefRenameData *> lImageRenameList, lXrefRenameList;

                    for (j = 0; j < ui->twImages->topLevelItemCount(); j++) {
                        TWIForSaveAddFile * itemImage = (TWIForSaveAddFile *) ui->twImages->topLevelItem(j);

                        if (!itemImage->text(1).isEmpty() && itemImage->text(0) != itemImage->text(1)) {
                            for (int k = 0; k < itemImage->childCount(); k++) {
                                TWIForSaveAddFile * itemImageSub = (TWIForSaveAddFile *) itemImage->child(k);
                                if (itemImageSub->PlotConst()->Id() == itemDoc->PlotConst()->Id()) {
                                    // add to xref rename list
                                    lImageRenameList.append(new XrefRenameData(itemImage->text(0), itemImage->text(1)));
                                }

                            }
                        }

                    }

                    if (ui->cbXrefs->isChecked()
                            && ui->twXrefs->topLevelItemCount()) {
                        for (j = 0; j < ui->twXrefs->topLevelItemCount(); j++) {
                            QTreeWidgetItem * itemXref = ui->twXrefs->topLevelItem(j);
                            if (!itemXref->text(1).isEmpty() && itemXref->text(0) != itemXref->text(1)) {
                                for (int k = 0; k < itemXref->childCount(); k++) {
                                    TWIForSaveXrefChild * itemXrefSub = static_cast<TWIForSaveXrefChild *>(itemXref->child(k));
                                    // debug111
                                    if (itemXrefSub->PlotConst()->Id() == itemDoc->PlotConst()->Id()) {
                                        // add to xref rename list
                                        lXrefRenameList.append(new XrefRenameData(itemXref->text(0), itemXref->text(1)));
                                    }

                                }
                            }

                        }
                     }

                    // need process rename to subdirectories for Xrefs and Images
                    bool lNeedProcessSub = itemDoc->PlotConst()->DwgConst()->InSubs() == 0
                            && (!itemDoc->PlotConst()->ImagesConst().isEmpty()
                                || !itemDoc->PlotConst()->XrefsConst().isEmpty());

                    if (lNeedProcessSub
                            || itemDoc->PlotConst()->NeedUpdateFields()
                            || !lXrefRenameList.isEmpty()
                            || !lImageRenameList.isEmpty()
                            || ui->cbSaveAcad->isVisible()
                                && ui->cbSaveAcad->currentIndex() > 0
                                && itemDoc->PlotConst()->DwgConst()->AcadVer() > ui->cbSaveAcad->currentData().toInt()) {
                        lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eSP, file.fileName(),
                                                                           lNeedProcessSub?2:0
                                                                                           + (itemDoc->PlotConst()->NeedUpdateFields()?(itemDoc->PlotConst()->DwgConst()->VFDateConst().isNull()?12:4):0),
                                                                           lXrefRenameList, lImageRenameList,
                                                                           itemDoc->PlotConst()->DwgConst()->Id()));
                    }
                } else {
                    QMessageBox::critical(this, tr("Saving documents"), tr("Error creating file") + "\n" + file.fileName() + "\n" + file.errorString());
                    return;
                }
            }
        } else if (itemDoc->PlotConst()->FileType() > 19 && itemDoc->PlotConst()->FileType() < 30) {
            // kp3, strap, etc
            QString lAddPath;

            if (!itemDoc->text(1).isEmpty()) {
                if (!saveDir.mkpath(itemDoc->text(1))) {
                    QMessageBox::critical(this, tr("Saving documents"), tr("Can't create subdirectory") + " " + itemDoc->text(1) + "!");
                    ui->lePathName->setFocus();
                    return;
                }
                lAddPath = "/" + itemDoc->text(1);
            }

            // no main file, just save info

            // write dwg_file
            if (!gOracle->InsertDwgFile(itemDoc->PlotConst()->DwgConst()->Id(), true, saveDir.path() + lAddPath,
                                        itemDoc->PlotConst()->DwgConst()->DataLength(), itemDoc->PlotConst()->DwgConst()->FTimeConst()/*QDateTime::currentDateTime()*/)) {
                return;
            }

            // ------------------------------------------------------------------------------------------------------------
            for (j = 0; j < itemDoc->PlotConst()->ImagesConst().length(); j++) {
                QFile file(saveDir.path() + lAddPath + "/" + itemDoc->PlotConst()->ImagesConst().at(j)->FilenameConst());

                if (file.exists()) {
                    if (gFileUtils->IsFileChanged(file.fileName(), itemDoc->PlotConst()->ImagesConst().at(j)->DataLength(), itemDoc->PlotConst()->ImagesConst().at(j)->Sha256Const())) {
                        if (!lForAll) {
                            AskOverwriteMB;
                            lOverwrite = mb.exec() == QMessageBox::Yes;
                            lForAll = mb.checkBox()->isChecked();
                        }
                    } else lOverwrite = false;
                } else lOverwrite = true;

                if (lOverwrite) {
                    // save file
                    if (file.open(QFile::WriteOnly)) {
                        file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, itemDoc->PlotConst()->ImagesConst().at(j)->Id()));
                        file.close();

                        gFileUtils->SetFileTime(file.fileName(), itemDoc->PlotConst()->ImagesConst().at(j)->FTimeConst());

                    } else {
                        QMessageBox::critical(this, tr("Saving documents"), tr("Error creating file") + "\n" + file.fileName() + "\n" + file.errorString());
                        return;
                    }
                }
            }
            // ------------------------------------------------------------------------------------------------------------
        } else {
            // common documents (as excel, word, pdf, etc.)
            QString lAddPath;

            if (ui->twDocs->topLevelItemCount() > 1 && ui->cbMakeSubdir->isChecked()) {
                if (!itemDoc->text(1).isEmpty()) {
                    lAddPath = itemDoc->text(1).left(itemDoc->text(1).lastIndexOf('.'));
                    if (!saveDir.mkpath(lAddPath)) {
                        QMessageBox::critical(this, tr("Saving documents"), tr("Can't create subdirectory") + " \"" + lAddPath + "\"!");
                        ui->lePathName->setFocus();
                        return;
                    }
                    lAddPath = "/" + lAddPath;
                }
            }

            QFile file(saveDir.path() + lAddPath + "/" + itemDoc->text(1));
            if (file.exists()) {
                if (gFileUtils->IsFileChanged(file.fileName(), itemDoc->PlotConst()->DwgConst()->DataLength(), itemDoc->PlotConst()->DwgConst()->Sha256Const())) {
                    if (!lForAll) {
                        AskOverwriteMB;
                        lOverwrite = mb.exec() == QMessageBox::Yes;
                        lForAll = mb.checkBox()->isChecked();
                    }
                } else lOverwrite = false; // not changed
            } else lOverwrite = true; // not exists

            if (lOverwrite) {
                // write dwg_file
                if (!gOracle->InsertDwgFile(itemDoc->PlotConst()->DwgConst()->Id(), true, file.fileName(), itemDoc->PlotConst()->DwgConst()->DataLength(),
                                            itemDoc->PlotConst()->DwgConst()->FTimeConst())) {
                    return;
                }

                // save file
                if (file.open(QFile::WriteOnly)) {
                    file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, itemDoc->PlotConst()->DwgConst()->Id()));
                    file.close();

                    gFileUtils->SetFileTime(file.fileName(), itemDoc->PlotConst()->DwgConst()->FTimeConst());

                    // IT'S A COPY ------------------------------------------------------------------------------------------------------------
                    for (j = 0; j < itemDoc->PlotConst()->ImagesConst().length(); j++) {
                        QFile file(saveDir.path() + lAddPath + "/" + itemDoc->PlotConst()->ImagesConst().at(j)->FilenameConst());

                        if (file.exists()) {
                            if (gFileUtils->IsFileChanged(file.fileName(), itemDoc->PlotConst()->ImagesConst().at(j)->DataLength(), itemDoc->PlotConst()->ImagesConst().at(j)->Sha256Const())) {
                                if (!lForAll) {
                                    AskOverwriteMB;
                                    lOverwrite = mb.exec() == QMessageBox::Yes;
                                    lForAll = mb.checkBox()->isChecked();
                                }
                            } else lOverwrite = false;
                        } else lOverwrite = true;

                        if (lOverwrite) {
                            // save file
                            if (file.open(QFile::WriteOnly)) {
                                file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, itemDoc->PlotConst()->ImagesConst().at(j)->Id()));
                                file.close();

                                gFileUtils->SetFileTime(file.fileName(), itemDoc->PlotConst()->ImagesConst().at(j)->FTimeConst());

                            } else {
                                QMessageBox::critical(this, tr("Saving documents"), tr("Error creating file") + "\n" + file.fileName() + "\n" + file.errorString());
                                return;
                            }
                        }
                    }
                    // ------------------------------------------------------------------------------------------------------------
                } else {
                    QMessageBox::critical(this, tr("Saving documents"), tr("Error creating file") + "\n" + file.fileName() + "\n" + file.errorString());
                    return;
                }
            }
        }
    }

#undef AskOverwriteMB

    if (!lDataForAcad.ListConst().isEmpty()) {
        //for (i = 0; i < lDataForAcad.ListConst().length(); i++) {
        //    QMessageBox::critical(this, tr("Saving documents"), lDataForAcad.ListConst().at(i)->FN() + " - " + lDataForAcad.ListConst().at(i)->PT());
        //}

        gSettings->DoOpenDwgNew(lDataForAcad, ui->cbUseAutocad->currentData().toInt()); // process in AutoCAD
    }

    accept();
}

void SaveDialog::ProcessLists(int aMode)
{
    int i, j;
    bool lHasDirDocs = false, lHasSimpleDocs = false;
    int lMaxAcadVersionExists = 0;

    mMaxAcadVersionForProcess = 0;
    mHasAcadDocs = false;

//    // start checking ---------------------------------------------------------------
//    // main docs
//    for (i = 0; i < ui->twDocs->topLevelItemCount() - 1; i++) {
//        for (j = i + 1; j < ui->twDocs->topLevelItemCount(); j++) {
//            if (ui->twDocs->topLevelItem(i)->text(1) == ui->twDocs->topLevelItem(j)->text(1)) {
//                ui->tabDocs->setCurrentIndex(0);
//                ui->twDocs->setFocus();
//                ui->twDocs->setCurrentItem(ui->twDocs->topLevelItem(i));
//                QMessageBox::critical(this, tr("Saving documents"), "Files names are duplicated!");;
//                return;
//            }
//        }
//    }

//    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
//        if (((PlotTreeWidgetItemMore *) ui->twDocs->topLevelItem(i))->GetPlotDataMore().ES() == PlotData::PESEditing) {
//            ui->tabDocs->setCurrentIndex(0);
//            ui->twDocs->setFocus();
//            ui->twDocs->setCurrentItem(ui->twDocs->topLevelItem(i));
//            QMessageBox::critical(this, tr("Saving documents"), "Some document(s) is editing right now!");;
//            return;
//        }
//    }

//    // xrefs
//    if (ui->cbXrefs->isChecked()) {

//        for (i = 0; i < ui->twXrefs->topLevelItemCount() - 1; i++) {
//            XrefTreeWidgetItem * itemI = (XrefTreeWidgetItem *) ui->twXrefs->topLevelItem(i);
//            QString nameI = itemI->text(1).isEmpty()?itemI->text(0):itemI->text(1);
//            for (j = i + 1; j < ui->twXrefs->topLevelItemCount(); j++) {
//                XrefTreeWidgetItem * itemJ = (XrefTreeWidgetItem *) ui->twXrefs->topLevelItem(j);
//                QString nameJ = itemJ->text(1).isEmpty()?itemJ->text(0):itemJ->text(1);
//                if (nameI == nameJ) {
//                    ui->twXrefs->setFocus();
//                    ui->twXrefs->setCurrentItem(itemJ, itemJ->text(1).isEmpty()?0:1);

//                    itemI->setBackgroundColor(itemI->text(1).isEmpty()?0:1, MY_COLOR_WARNING);
//                    itemJ->setBackgroundColor(itemJ->text(1).isEmpty()?0:1, MY_COLOR_WARNING);

//                    QMessageBox::critical(this, tr("Saving documents"), "Xref's' names are duplicated!\nYou must rename xref!");;
//                    return;
//                }
//            }
//        }

//        for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
//            XrefTreeWidgetItem * item = (XrefTreeWidgetItem *) ui->twXrefs->topLevelItem(i);
//            if (item->GetPlotDataMore().ES() == PlotData::PESEditing) {
//                ui->tabDocs->setCurrentIndex(2);
//                ui->twXrefs->setFocus();
//                ui->twXrefs->setCurrentItem(item);
//                QMessageBox::critical(this, tr("Saving documents"), "Some xref(s) is editing right now!");;
//                return;
//            }
//        }
//    }

//    // images
//    if (ImageListCheckForDuplicate()) {
//        ui->tabDocs->setCurrentIndex(1);
//        ui->twImages->setFocus();
//        QMessageBox::critical(this, tr("Saving documents"), "Images names are duplicated!\n\nYou can set the check box on \"Images\" tab and try to save again");;
//        return;

//    }

    // ---------------------------------------------------------------

    // Images
//    for (i = 0; i < ui->twImages->topLevelItemCount(); i++) {
//        TWIForSaveAddFile * item = (TWIForSaveAddFile *) ui->twImages->topLevelItem(i);

//        for (j = 0; j < 2; j++) {
//            if (!j && item->DwgConst().InMain()
//                    || j && item->DwgConst().InXref()) {

//                // add to save path (((!j)?"/Images/":"/Xrefs/Images/") + (item->text(1).isEmpty()?item->text(0):item->text(1)));

//            }
//        }
//    }

    // add. files
//    if (ui->cbAddFiles->isChecked()) {
//        for (i = 0; i < ui->twFiles->topLevelItemCount(); i++) {
//            TWIForSaveAddFile * item = (TWIForSaveAddFile *) ui->twFiles->topLevelItem(i);
//            QString lAddPath;

//            if (item->DwgConst().GroupNameConst() == "Acad:Text") {
//                lAddPath = "/Fonts";
//            } else if (item->DwgConst().GroupNameConst() == "Acad:PlotStyle") {
//                lAddPath = "/Plot Styles";
//            } else if (item->DwgConst().GroupNameConst() == "Acad:PlotConfig") {
//                lAddPath = "/Plotters";
//            }


//            // add to save path (lAddPath + "/" + item->text(0));
//        }
//    }

    // xrefs
    if (ui->cbXrefs->isChecked()
            && ui->twXrefs->topLevelItemCount()) {

        for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
            TWIForSaveXrefTop * itemXref = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i));

            if (!itemXref->childCount()) {
                itemXref->setBackgroundColor(0, MY_COLOR_DISABLED);
                itemXref->setText(1, "");
                continue;
            }
            itemXref->setBackground(0, itemXref->background(1));

            BlobMemCache::RecordType opRT;
            int opID;
            bool lNeedAcadProcess = false;

            // add to save path ("/Xrefs/" + (item->text(1).isEmpty()?item->text(0):item->text(1)) + "." + ui->cbXrefExt->currentText());
            if (itemXref->XrefConst()->DwgConst()->NeedNotProcess() > 0) {
                // need not process
                opRT = BlobMemCache::Dwg;
                opID = itemXref->XrefConst()->DwgConst()->Id();
            } else if (itemXref->XrefConst()->DwgConst()->IdCache() > 0) {
                // get from cache
                opRT = BlobMemCache::DwgCache;
                opID = itemXref->XrefConst()->DwgConst()->IdCache();
            } else {
                // get from DWG and NEED PROCESS
                opRT = BlobMemCache::Dwg;
                opID = itemXref->XrefConst()->DwgConst()->Id();
                lNeedAcadProcess = true;
            }

            if (!lNeedAcadProcess
                    && !itemXref->XrefConst()->DwgConst()->InSubs()
                    && !itemXref->XrefConst()->ImagesConst().isEmpty()) {
                lNeedAcadProcess = true;
            }

            if (!itemXref->XrefPropsConst()->XrefPropsConst().isEmpty()) {
                lNeedAcadProcess = true; // need apply properties
            }

            if (!lNeedAcadProcess) {
                // we need not real list here, only fact that we neeed rename
                for (j = 0; j < ui->twImages->topLevelItemCount(); j++) {
                    TWIForSaveAddFile * itemImage = static_cast<TWIForSaveAddFile *>(ui->twImages->topLevelItem(j));

                    if (!itemImage->text(1).isEmpty() && itemImage->text(0) != itemImage->text(1)) {
                        for (int k = 0; k < itemImage->childCount(); k++) {
                            TWIForSaveAddFile * itemImageSub = static_cast<TWIForSaveAddFile *>(itemImage->child(k));
                            // debug111
                            if (itemImageSub->PlotConst()->Id() == itemXref->XrefConst()->Id()) {
                                // add to xref rename list
                                lNeedAcadProcess = true;
                                break;
                            }

                        }
                    }

                }
            }
            if (lNeedAcadProcess
                    || ui->cbSaveAcad->isVisible()
                        && ui->cbSaveAcad->currentIndex() > 0
                        && itemXref->XrefConst()->DwgConst()->AcadVer() > ui->cbSaveAcad->currentData().toInt()) {
                // need process as Xref
                if (itemXref->XrefConst()->DwgConst()->AcadVer() > mMaxAcadVersionForProcess)
                    mMaxAcadVersionForProcess = itemXref->XrefConst()->DwgConst()->AcadVer();
            }
            if (itemXref->XrefConst()->DwgConst()->AcadVer() > lMaxAcadVersionExists)
                lMaxAcadVersionExists = itemXref->XrefConst()->DwgConst()->AcadVer();

        }
    }

    // main docs
    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        TWIForSaveMainDoc * item = (TWIForSaveMainDoc *) ui->twDocs->topLevelItem(i);

        if ((item->PlotConst()->FileType() < 20 || item->PlotConst()->FileType() > 29)
                && item->PlotConst()->DwgConst()->ExtensionConst().toLower() == "dwg") {
            mHasAcadDocs = true;
        } else if (item->PlotConst()->FileType() > 19
                   && item->PlotConst()->FileType() < 30) {
            lHasDirDocs = true;
        } else {
            lHasSimpleDocs = true;
        }

        // add to save path ("/" + item->text(1)/* + "." + item->GetPlotDataMore().DwgConst().ExtensionConst()*/);
        // write dwg_file
        // save file

        bool lNeedRenameImage = false, lNeedRenameXref = false;

        for (j = 0; j < ui->twImages->topLevelItemCount(); j++) {
            TWIForSaveAddFile * itemImage = (TWIForSaveAddFile *) ui->twImages->topLevelItem(j);

            if (!itemImage->text(1).isEmpty() && itemImage->text(0) != itemImage->text(1)) {
                for (int k = 0; k < itemImage->childCount(); k++) {
                    TWIForSaveAddFile * itemImageSub = (TWIForSaveAddFile *) itemImage->child(k);
                    // debug111
                    if (itemImageSub->PlotConst()->Id() == item->PlotConst()->Id()) {
                        lNeedRenameImage = true;
                        break;
                    }

                }
            }

        }

        for (j = 0; j < ui->twXrefs->topLevelItemCount(); j++) {
            QTreeWidgetItem * itemXref = ui->twXrefs->topLevelItem(j);

            if (!itemXref->text(1).isEmpty() && itemXref->text(0) != itemXref->text(1)) {
                for (int k = 0; k < itemXref->childCount(); k++) {
                    TWIForSaveXrefChild  * itemXrefSub = static_cast<TWIForSaveXrefChild  *>(itemXref->child(k));
                    if (itemXrefSub->XrefConst()->Id() == item->PlotConst()->Id()) {
                        lNeedRenameXref = true;
                        break;
                    }

                }
            }

        }

        // need process rename to subdirectories for Xrefs and Images
        bool lNeedProcessSub = item->PlotConst()->DwgConst()->InSubs() == 0
                && (!item->PlotConst()->ImagesConst().isEmpty()
                    || !item->PlotConst()->XrefsConst().isEmpty());

        if (lNeedProcessSub
                || item->PlotConst()->NeedUpdateFields()
                || lNeedRenameImage
                || lNeedRenameXref
                || ui->cbSaveAcad->isVisible()
                    && ui->cbSaveAcad->currentIndex() > 0
                    && item->PlotConst()->DwgConst()->AcadVer() > ui->cbSaveAcad->currentData().toInt()) {
            // need process motherfucker
            if (item->PlotConst()->DwgConst()->AcadVer() > mMaxAcadVersionForProcess)
                mMaxAcadVersionForProcess = item->PlotConst()->DwgConst()->AcadVer();
        }
        if (item->PlotConst()->DwgConst()->AcadVer() > lMaxAcadVersionExists)
            lMaxAcadVersionExists = item->PlotConst()->DwgConst()->AcadVer();
    }

    ui->cbMakeSubdir->setVisible(ui->twDocs->topLevelItemCount() > 1
                                    && (lHasDirDocs || lHasSimpleDocs));
    ui->cbMakeSubdir->setDisabled(lHasDirDocs && ui->twDocs->topLevelItemCount() > 1);
    ui->cbMakeSubdir->setChecked(lHasDirDocs && ui->twDocs->topLevelItemCount() > 1);

    if (mMaxAcadVersionForProcess > 0) {
        ui->lblAcadRequired->setText("AutoCAD required: " + QString::number(mMaxAcadVersionForProcess));
        ui->lblAcadRequired->setVisible(true);
        ui->lblUseAtocad->setVisible(true);
        ui->cbUseAutocad->setVisible(true);

        on_cbUseAutocad_currentIndexChanged(ui->cbUseAutocad->currentIndex());
    } else {
        ui->lblAcadRequired->setVisible(false);
        ui->lblUseAtocad->setVisible(false);
        ui->cbUseAutocad->setVisible(false); // autocad needn't at all
    }

    ui->lblSaveAcad->setVisible(lMaxAcadVersionExists > 7);
    ui->cbSaveAcad->setVisible(lMaxAcadVersionExists > 7);
}

void SaveDialog::InitListHeader(QTreeWidget *tw)
{
    int nCol = 0;
    tw->setColumnWidth(nCol++, 180); // file name
    if (tw == ui->twImages) tw->setColumnWidth(nCol++, 180); // new file name
    tw->setColumnWidth(nCol++, 40); // type (extension)
    tw->setColumnWidth(nCol++, 80); // use id
    tw->setColumnWidth(nCol++, 80); // use size

    tw->setColumnWidth(nCol++, 60); // version
    tw->setColumnWidth(nCol++, 60); // version for customer
    tw->setColumnWidth(nCol++, 50); // history

    tw->setColumnHidden(nCol, true);
    tw->setColumnWidth(nCol++, 80); // sent date

    tw->setColumnWidth(nCol++, 150); // code
    tw->setColumnWidth(nCol++, 40); // sheet

    tw->setColumnWidth(nCol++, 280); // name top
    tw->setColumnWidth(nCol++, 280); // name bottom

    tw->setColumnWidth(nCol++, 80); // edit date
    tw->setColumnWidth(nCol++, 100); // edit user

    tw->setColumnHidden(nCol, true);
    tw->setColumnWidth(nCol++, 80); // size
}

void SaveDialog::AddImageToCommonList(PlotDwgData * aPlotData, DwgForSaveData * aDwgData, TWIForSaveAddFile::RecordTypeEnum aRecordType)
{
    bool lIsFound = false;

    for (int i = 0; i < ui->twImages->topLevelItemCount(); i++) {
        if (ui->twImages->topLevelItem(i)->text(0).toLower() == aDwgData->FilenameConst().toLower()
                /*don't understand && ((TWIForSaveAddFile *) ui->twImages->topLevelItem(i))->IsAcadImage()*/
                && ((TWIForSaveAddFile *) ui->twImages->topLevelItem(i))->DwgConst()->Sha256Const() == aDwgData->Sha256Const()) {

            // add subelement
            for (int j = 0; j < ui->twImages->topLevelItem(i)->childCount(); j++) {
                if (ui->twImages->topLevelItem(i)->child(j)->text(0) == QString::number(aPlotData->Id())) {
                    lIsFound = true;
                    break;
                }
            }

            if (!lIsFound) {
                TWIForSaveAddFile *sdi2 = new TWIForSaveAddFile(aPlotData, aDwgData, aRecordType);
                ui->twImages->topLevelItem(i)->addChild(sdi2);

                if (aDwgData->InMain())
                    ((TWIForSaveAddFile *) ui->twImages->topLevelItem(i))->DwgRef()->SetInMain(true);
                if (aDwgData->InXref())
                    ((TWIForSaveAddFile *) ui->twImages->topLevelItem(i))->DwgRef()->SetInXref(true);

                lIsFound = true;
            }
            break;
        }
    }
    if (!lIsFound) {
        // add new
        TWIForSaveAddFile *sdi = new TWIForSaveAddFile(aDwgData, aRecordType);
        ui->twImages->addTopLevelItem(sdi);
        // add subelement
        TWIForSaveAddFile *sdi2 = new TWIForSaveAddFile(aPlotData, aDwgData, aRecordType);
        sdi->addChild(sdi2);
    }
}

void SaveDialog::AddSuppFileToCommonList(PlotDwgData * aPlotData, DwgForSaveData * aDwgData)
{
    bool lIsFound = false;

    // just for fun
    QTreeWidget *tw = ui->twFiles;

    for (int i = 0; i < tw->topLevelItemCount(); i++) {
        if (tw->topLevelItem(i)->text(0).toLower() == aDwgData->FilenameConst().toLower()) {
            // add subelement
            for (int j = 0; j < tw->topLevelItem(i)->childCount(); j++) {
                if (tw->topLevelItem(i)->child(j)->text(0) == QString::number(aPlotData->Id())) {
                    lIsFound = true;
                    break;
                }
            }

            if (!lIsFound) {
                TWIForSaveAddFile *sdi2 = new TWIForSaveAddFile(aPlotData, aDwgData, TWIForSaveAddFile::AcadNonImage);
                //if (sdi2->sizeHint(0) == QSize())
                //    QMessageBox::critical(this, tr("Tasks"), QString::number(sdi2->sizeHint(0).width()) + " - " + QString::number(sdi2->sizeHint(0).height()));;
                //for (int i = 0; i < sdi2->columnCount(); i++)
                //    sdi2->setSizeHint(i, QSize());
                tw->topLevelItem(i)->addChild(sdi2);

                if (aDwgData->InMain())
                    ((TWIForSaveAddFile *) tw->topLevelItem(i))->DwgRef()->SetInMain(true);
                if (aDwgData->InXref())
                    ((TWIForSaveAddFile *) tw->topLevelItem(i))->DwgRef()->SetInXref(true);

                lIsFound = true;
            }
            break;
        }
    }
    if (!lIsFound) {
        // add new
        TWIForSaveAddFile *sdi = new TWIForSaveAddFile(aDwgData, TWIForSaveAddFile::AcadNonImage);
        //for (int i = 0; i < sdi->columnCount(); i++)
        //    sdi->setSizeHint(i, QSize());
        tw->addTopLevelItem(sdi);
        // add subelement
        TWIForSaveAddFile *sdi2 = new TWIForSaveAddFile(aPlotData, aDwgData, TWIForSaveAddFile::AcadNonImage);
        //for (int i = 0; i < sdi2->columnCount(); i++)
        //    sdi2->setSizeHint(i, QSize());
        sdi->addChild(sdi2);
    }
}

// input - (1) Drawing and (2) One of it xref
void SaveDialog::AddXrefToCommonList(PlotForSaveData * aPlot, XrefForSaveData * aXref) {
    int i;
    bool lIsFound = false;
    XrefPropsData * lXPFounded = NULL; // no properties by default

    // searching for this xref properties in all drawing xrefs properties
    for (i = 0; i < aPlot->XrefsPropsConst().count(); i++) {
        XrefPropsData * lXrefPropsData = aPlot->XrefsPropsConst().at(i);
        if (lXrefPropsData->IdCommon() == aXref->IdCommon()) {
            lXPFounded = lXrefPropsData; // properties found
            break;
        }
    }

    for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
        TWIForSaveXrefTop * itemXref = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i));
        if (itemXref->XrefConst()->IdCommon() == aXref->IdCommon()
                && itemXref->text(0).toLower() == aXref->BlockNameConst().toLower()
                && (*itemXref->XrefPropsConst() == XrefPropsData() && !lXPFounded
                    || lXPFounded && *itemXref->XrefPropsConst() == *lXPFounded)) {
            // existing xref found in list (same id-common, same name, same properties!)

            // debug333
            // looking that the main xref already exists in list (how can it be?)
            for (int j = 0; j < ui->twXrefs->topLevelItem(i)->childCount(); j++) {
                if (ui->twXrefs->topLevelItem(i)->child(j)->text(0) == QString::number(aPlot->Id())) {
                    lIsFound = true;
                    break;
                }
            }

            // main drawing not found in childs of xref, add it
            if (!lIsFound) {
                TWIForSaveXrefChild *itemChild = new TWIForSaveXrefChild(aXref, aPlot, ui->cbXrefNameCase->currentIndex());
                ui->twXrefs->topLevelItem(i)->addChild(itemChild);
                lIsFound = true;
            }

            break;
        }
    }

    // xref was not found, add it
    if (!lIsFound) {
        // add new
        // it is row for xref self
        TWIForSaveXrefTop * itemXref = new TWIForSaveXrefTop(aXref, lXPFounded, ui->cbXrefNameCase->currentIndex());
        ui->twXrefs->addTopLevelItem(itemXref);
        // add subelement
        // it is row for main drawing (aPlotDataXref is duplicate, cos can use data from parent item)
        TWIForSaveXrefChild * itemChild = new TWIForSaveXrefChild(aXref, aPlot, ui->cbXrefNameCase->currentIndex());
        itemXref->addChild(itemChild);
    }
}

bool SaveDialog::ImageListCheckForDuplicate()
{
    int i, j;
    bool lWasCahnged, res = false;

    ui->twImages->setColumnHidden(1, true);

    for (i = 0; i < ui->twImages->topLevelItemCount(); i++) {
        ui->twImages->topLevelItem(i)->setBackground(0, ui->twImages->topLevelItem(i)->background(2));
        ui->twImages->topLevelItem(i)->setBackground(1, ui->twImages->topLevelItem(i)->background(2));
    }

    do {
        lWasCahnged = false;
        for (i = 0; i < ui->twImages->topLevelItemCount() - 1; i++) {
            TWIForSaveAddFile * itemI = (TWIForSaveAddFile *)ui->twImages->topLevelItem(i);
            QString nameI = itemI->text(1).isEmpty()?itemI->text(0):itemI->text(1);
            int SNCount = 1;

            for (j = i + 1; j < ui->twImages->topLevelItemCount(); j++) {
                TWIForSaveAddFile * itemJ = (TWIForSaveAddFile *)ui->twImages->topLevelItem(j);
                QString nameJ = itemJ->text(1).isEmpty()?itemJ->text(0):itemJ->text(1);
                if (nameI == nameJ) {
                    QString ext;
                    if (itemJ->RecordType() == TWIForSaveAddFile::AcadImage) {
                        ext = nameJ.mid(nameJ.lastIndexOf("."));
                        nameJ = nameJ.left(nameJ.lastIndexOf("."));

                        nameJ = nameJ + "_" + QString::number(SNCount) + ext;
                        itemJ->setText(1, nameJ);
                        ui->twImages->setColumnHidden(1, false);
                        lWasCahnged = true;
                    } else if (itemI->RecordType() == TWIForSaveAddFile::AcadImage) {
                        nameI += "_" + QString::number(SNCount);
                        itemI->setText(1, nameI);
                        ui->twImages->setColumnHidden(1, false);
                        lWasCahnged = true;
                    } else {
                        itemI->setBackgroundColor(0, MY_COLOR_ERROR);;
                        itemJ->setBackgroundColor(0, MY_COLOR_ERROR);;
                        res = true;
                    }

                    SNCount++;
                }
            }
        }
    } while (lWasCahnged);

    return res;
}

bool SaveDialog::FileListCheckForDuplicate()
{
    bool res = false;
    int i, j;
    for (i = 0; i < ui->twFiles->topLevelItemCount(); i++) {
        QList<int> lIds;
        TWIForSaveAddFile * itemI = (TWIForSaveAddFile *) ui->twFiles->topLevelItem(i);

        lIds.append(itemI->text(2).toInt());

        for (j = 0; j < itemI->childCount(); j++) {
            TWIForSaveAddFile * itemJ = (TWIForSaveAddFile *) itemI->child(j);
            // if id is different
            if ((itemI->DwgConst()->Sha256Const().isEmpty()
                        || itemJ->DwgConst()->Sha256Const().isEmpty())
                    && itemI->text(2) != itemJ->text(2)
                    || !itemI->DwgConst()->Sha256Const().isEmpty()
                        && !itemJ->DwgConst()->Sha256Const().isEmpty()
                        &&  itemI->DwgConst()->Sha256Const() != itemJ->DwgConst()->Sha256Const()) {
                // add id to list
                if (!lIds.contains(itemJ->text(2).toInt()))
                    lIds.append(itemJ->text(2).toInt());
                res = true;
            }
        }

        if (lIds.length() > 1) {
            itemI->setExpanded(true); // expand
            // can edit...
            itemI->setFlags(itemI->flags() | Qt::ItemIsEditable);

            // color and tooltip
            itemI->setBackgroundColor(2, MY_COLOR_WARNING);
            itemI->setToolTip(2, "You can select appropriate version");

            itemI->SetIds(lIds); // this list used in delegate
        } else {
            // no difference, so clean columns - it is equal for all items and equal to this main item
            for (j = 0; j < itemI->childCount(); j++) {
                itemI->child(j)->setText(2, "");
                itemI->child(j)->setText(3, "");
            }
        }
    }

    ui->cbAddFiles->setVisible(ui->twFiles->topLevelItemCount());

    return res;
}

void SaveDialog::XrefListCheckForDuplicate()
{
    int i, j;
    bool lCanSaveNewAny = false;

    for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
        TWIForSaveXrefTop * itemXref = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i));
        bool lCanSaveNewThis = false, lHasUseNonWorking = false;

        QMap<QString, XrefForSaveData *> & lVarList = itemXref->VarList();
        XrefForSaveData * lXrefLastVersion = NULL;

        lVarList.clear();
        for (j = 0; j < itemXref->childCount(); j++) {
            TWIForSaveXrefChild * itemChild = static_cast<TWIForSaveXrefChild *>(itemXref->child(j));

            // some drawing use fixed version
            if (!itemChild->XrefConst()->Working()) lHasUseNonWorking = true;

            // if working and ver in dwg is last
            if (/*itemChild->XrefConst()->UseWorking() // did I need this first clause?
                    && */itemChild->XrefConst()->Working()
                    && itemChild->XrefConst()->DwgConst()->Version() == itemChild->XrefConst()->DwgVersionMax()) {
                lXrefLastVersion = itemChild->XrefRef();
            } else {
                lCanSaveNewThis = true; // we can switch to new version (in other case, all versions is new)
                lCanSaveNewAny = true; // we can switch to new version (in other case, all versions is new)
            }

            // String is "ID - CurHist/MaxHist"
            QString lStrId = QString::number(itemChild->XrefConst()->Id()) + " - " +
                    QString::number(itemChild->XrefConst()->DwgConst()->Version()) + "/" +
                    QString::number(itemChild->XrefConst()->DwgVersionMax());

            if (!lVarList.contains(lStrId))
                lVarList.insert(lStrId, itemChild->XrefConst());
        }

        if (!lXrefLastVersion && lCanSaveNewThis) {
            // we need Current Working Version for current xref
            // add last working version

            lXrefLastVersion = new XrefForSaveData(itemXref->XrefConst()->IdCommon(), itemXref->text(0)); // init from id_common
            if (lXrefLastVersion->DwgConst()->Id()) {

                mXrefForSaveForDel.append(lXrefLastVersion);

                lVarList.insert(QString::number(lXrefLastVersion->Id()) + " - " +
                                QString::number(lXrefLastVersion->DwgConst()->Version()) + "/" +
                                QString::number(lXrefLastVersion->DwgVersionMax()), lXrefLastVersion);
            } else {
                delete lXrefLastVersion;
                lXrefLastVersion = NULL;
            }
        }

        if (lVarList.size() < 2) {
            // no variants, so clear unneeded fields in childs - they all is equal
            for (j = 0; j < ui->twXrefs->topLevelItem(i)->childCount(); j++) {
                ui->twXrefs->topLevelItem(i)->child(j)->setText(3, "");
                ui->twXrefs->topLevelItem(i)->child(j)->setText(4, "");
            }
        } else {
            // was versions
            itemXref->setExpanded(true);

            if (ui->cbSaveNewXrefs->isChecked() && lXrefLastVersion) {
                // set last
                itemXref->SetPlotData(lXrefLastVersion, ui->cbXrefNameCase->currentIndex());
            }
            itemXref->setToolTip(3, "You can select appropriate version");
            itemXref->setBackgroundColor(3, MY_COLOR_WARNING);
        }
    }

    ui->cbXrefs->setVisible(ui->twXrefs->topLevelItemCount());
    ui->cbSaveNewXrefs->setVisible(ui->cbXrefs->isVisible() && lCanSaveNewAny);

    XrefRenameDuplicates();
}

void SaveDialog::XrefRenameDuplicates() {
    int i, j, k, l;
    bool lWasCahnged;

    ui->twXrefs->setColumnHidden(1, true); // filename
    ui->twXrefs->setColumnHidden(2, true); // properties

    for (i = 0; i < ui->twXrefs->topLevelItemCount() - 1; i++) {
        QTreeWidgetItem * itemI = ui->twXrefs->topLevelItem(i);
        itemI->setFlags(itemI->flags() & ~Qt::ItemIsDropEnabled);
        for (j = 0; j < itemI->childCount(); j++) {
            itemI->child(j)->setFlags(itemI->child(j)->flags() & ~(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
        }
    }

    do {
        lWasCahnged = false;
        // k for twice checking - in first we want not rename xref's with no properties;
        // in second - we must rename, no choice
        for (k = 0; k < 2; k++) {
            for (i = 0; i < ui->twXrefs->topLevelItemCount() - 1; i++) {
                TWIForSaveXrefTop * itemI = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i));
                QString nameI = itemI->text(1).isEmpty()?itemI->text(0):itemI->text(1);
                int SNCount = 1;

                if (*itemI->XrefPropsConst() == XrefPropsData() || k) {
                    for (j = i + 1; j < ui->twXrefs->topLevelItemCount(); j++) {
                        TWIForSaveXrefTop * itemJ = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(j));
                        QString nameJ = itemJ->text(1).isEmpty()?itemJ->text(0):itemJ->text(1);
                        if (nameI == nameJ && itemI->childCount() && itemJ->childCount()) {
                            if (*itemJ->XrefPropsConst() == XrefPropsData()) {
                                if (!(*itemI->XrefPropsConst() == XrefPropsData())) {
                                    nameI += "_" + QString::number(gSettings->maPrime2cHash(itemI->XrefPropsConst()->GetAbrv().toLatin1()), 16);
                                    itemI->setText(1, nameI);
                                } else {
                                    nameJ += "_" + QString::number(SNCount);
                                    itemJ->setText(1, nameJ);
                                }
                            } else {
                                nameJ += "_" + QString::number(gSettings->maPrime2cHash(itemJ->XrefPropsConst()->GetAbrv().toLatin1()), 16);
                                itemJ->setText(1, nameJ);
                            }


                            SNCount++;
                            lWasCahnged = true;
                        }

                        // enable drag&drop
                        //--------------------------------
                        if (itemI->XrefConst()->IdCommon() == itemJ->XrefConst()->IdCommon()) {
                            itemI->setFlags(itemI->flags() | Qt::ItemIsDropEnabled);
                            for (l = 0; l < itemI->childCount(); l++) {
                                itemI->child(l)->setFlags(itemI->child(l)->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                            }
                            itemJ->setFlags(itemJ->flags() | Qt::ItemIsDropEnabled);
                            for (l = 0; l < itemJ->childCount(); l++) {
                                itemJ->child(l)->setFlags(itemJ->child(l)->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                            }
                        }
                        //--------------------------------

                        if (!itemI->text(1).isEmpty()
                                || !itemJ->text(1).isEmpty()) {
                            ui->twXrefs->setColumnHidden(1, false);
                        }
                        if (!itemI->text(2).isEmpty()
                                || !itemJ->text(2).isEmpty()) {
                            ui->twXrefs->setColumnHidden(2, false);
                        }
                    }
                }
            }
        }
    } while (lWasCahnged);
}

//void SaveDialog::XrefListCheckForDuplicate()
//{
//    int i, j;
//    bool lCanSaveNew = false;
//    for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
//        XrefTreeWidgetItem * itemThis = (XrefTreeWidgetItem *)ui->twXrefs->topLevelItem(i);
//        bool lIsBad = false, lHasUseNonWorking = false, lHasLast = false;

//        QMap<QString, PlotDataMore> &lVarList = itemThis->VarList();
//        PlotDataMore lPlotDataLastVersion;

//        lVarList.clear();
//        for (j = 0; j < itemThis->childCount(); j++) {
//            XrefTreeWidgetItem * itemChild = (XrefTreeWidgetItem *) itemThis->child(j);

//            // some drawing use fixed version
//            if (!itemChild->GetPlotDataMoreOther().UseWorking()) lHasUseNonWorking = true;

//            // if working and ver in dwg is last
//            if (itemChild->GetPlotDataMoreOther().UseWorking() // did I need this first clause?
//                    && itemChild->GetPlotDataMoreOther().Working()
//                    && itemChild->GetPlotDataMoreOther().DwgConst().Version() == itemChild->GetPlotDataMoreOther().MaxDvgVersion()) {
//                lHasLast = true; // we already have newest version in the list
//                lPlotDataLastVersion = itemChild->GetPlotDataMoreOther();
//            } else {
//                lCanSaveNew = true; // we can switch to new version (in other case, all versions is new)
//            }

//            if (itemThis->GetPlotDataMore().IdCommon() != itemChild->GetPlotDataMoreOther().IdCommon()) {
//                // some xref with same name has different ID_COMMON - so it is different xrefs
//                lIsBad = true;
//            }

//            // String is "ID - CurHist/MaxHist"
//            QString lStrId = QString::number(itemChild->GetPlotDataMoreOther().Id()) + " - " +
//                    QString::number(itemChild->GetPlotDataMoreOther().DwgConst().Version()) + "/" +
//                    QString::number(itemChild->GetPlotDataMoreOther().MaxDvgVersion());

//            if (!lVarList.contains(lStrId))
//                lVarList.insert(lStrId, itemChild->GetPlotDataMoreOther());
//        }

//        if (lIsBad) {
//            // all is bad - same name for different xrefs
//            itemThis->setBackgroundColor(0, ERROR_COLOR);
//            itemThis->setToolTip(0, "Name conflict: same name for different xrefs");
//            itemThis->SetHasDiffIdCommon(true); // we can't save this drawing set
//        } else {
//            bool lCanCont = true;
//            if (lVarList.size() < 2) {
//                // no variants, so clear unneeded fields - they all is equal
//                for (j = 0; j < ui->twXrefs->topLevelItem(i)->childCount(); j++) {
//                    ui->twXrefs->topLevelItem(i)->child(j)->setText(3, "");
//                    ui->twXrefs->topLevelItem(i)->child(j)->setText(4, "");
//                }
//            } else {
//                itemThis->setExpanded(true);

//                if (lHasUseNonWorking) {
//                    // были разные, но у некоторых был UseNonWorking
//                    itemThis->setBackgroundColor(0, ERROR_COLOR);
//                    itemThis->setToolTip(0, "Some xrefs use non-current fixed version");
//                    itemThis->SetHasDiffIdCommon(true); // or do I need separate flag for this case?

//                    for (j = 0; j < itemThis->childCount(); j++) {
//                        XrefTreeWidgetItem * itemChild = (XrefTreeWidgetItem *) itemThis->child(j);
//                        if (!itemChild->GetPlotDataMoreOther().UseWorking()) {
//                            itemChild->setBackgroundColor(0, ERROR_COLOR);
//                            itemChild->setToolTip(0, "This xref use fixed version");
//                        }
//                    }
//                    lCanCont = false;
//                }
//            }

//            if (lCanCont) {
//                if (!lHasLast) {
//                    // add last working version
//                    PlotDataMore pdmNew;
//                    pdmNew.InitFromCommonId(itemThis->GetPlotDataMore().IdCommon());

//                    QString lStrId = QString::number(pdmNew.Id()) + " - " +
//                            QString::number(pdmNew.DwgConst().Version()) + "/" +
//                            QString::number(pdmNew.MaxDvgVersion());

//                    lVarList.insert(lStrId, pdmNew);
//                    lPlotDataLastVersion = pdmNew;
//                    lCanSaveNew = true;
//                }

//                if (lVarList.size() > 1) {
//                    // was versions
//                    if (ui->cbSaveNewXrefs->isChecked()) {
//                        // set last
//                        itemThis->SetPlotData(lPlotDataLastVersion, ui->cbXrefNameCase->currentIndex());
//                    }

//                    // can edit...
//                    itemThis->setFlags(itemThis->flags() | Qt::ItemIsEditable);
//                    // ... but through delegate
//                    SDXrefsIdsDelegate *IdsDelegate = new SDXrefsIdsDelegate(ui->twXrefs);
//                    ui->twXrefs->setItemDelegate(IdsDelegate);
//                    connect(IdsDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(CurVarChanged(QWidget *)));

//                    itemThis->setToolTip(1, "You can select appropriate version");
//                    itemThis->setBackgroundColor(3, WARNING_COLOR);
//                }
//            }
//        }
//    }
//    ui->cbXrefs->setVisible(ui->twXrefs->topLevelItemCount());
//    ui->cbSaveNewXrefs->setVisible(ui->cbXrefs->isVisible() && lCanSaveNew);
//}

void SaveDialog::CurVarChanged(QWidget *editor)
{
    if (qobject_cast<QLineEdit *> (editor)
            && ui->twXrefs->currentColumn() == 1) {
        int i, j;
        // if File name == Block name then clear it
        if (qobject_cast<QLineEdit *> (editor)->text() == ui->twXrefs->currentItem()->text(0)) {
            ui->twXrefs->currentItem()->setText(1, "");
        }
        // it is just checking that user rename xref correctly
        // clear all backgrounds
        for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
            ui->twXrefs->topLevelItem(i)->setBackground(0, ui->twXrefs->topLevelItem(i)->background(2));
            ui->twXrefs->topLevelItem(i)->setBackground(1, ui->twXrefs->topLevelItem(i)->background(2));
        }
        // check for duplicates
        for (i = 0; i < ui->twXrefs->topLevelItemCount() - 1; i++) {
            QTreeWidgetItem * itemI = ui->twXrefs->topLevelItem(i);
            QString nameI = itemI->text(1).isEmpty()?itemI->text(0):itemI->text(1);
            for (j = i + 1; j < ui->twXrefs->topLevelItemCount(); j++) {
                QTreeWidgetItem * itemJ = ui->twXrefs->topLevelItem(j);
                QString nameJ = itemJ->text(1).isEmpty()?itemJ->text(0):itemJ->text(1);
                if (nameI == nameJ) {
                    ui->twXrefs->setFocus();
                    ui->twXrefs->setCurrentItem(itemJ, itemJ->text(1).isEmpty()?0:1);

                    itemI->setBackgroundColor(itemI->text(1).isEmpty()?0:1, MY_COLOR_WARNING);
                    itemJ->setBackgroundColor(itemJ->text(1).isEmpty()?0:1, MY_COLOR_WARNING);
                }
            }
        }
    } else if (qobject_cast<QComboBox *> (editor)
            && ui->twXrefs->currentColumn() == 3) {
        QString lStr = ui->twXrefs->currentItem()->text(ui->twXrefs->currentColumn());
        QMap<QString, XrefForSaveData *> &lVarList = static_cast<TWIForSaveXrefTop *> (ui->twXrefs->currentItem())->VarList();

        QMap<QString, XrefForSaveData *>::const_iterator itr = lVarList.find(lStr);
        if (itr != lVarList.end() && itr.key() == lStr) {

            static_cast<TWIForSaveXrefTop *> (ui->twXrefs->currentItem())->SetPlotData(itr.value(), ui->cbXrefNameCase->currentIndex());
            // don't know why, but it is turned off
            ui->twXrefs->currentItem()->setFlags(ui->twXrefs->currentItem()->flags() | Qt::ItemIsEditable);

            if (!itr.value()->Working() || itr.value()->DwgVersionMax() > itr.value()->DwgConst()->Version()) {
                ui->cbSaveNewXrefs->setChecked(false);
            }

            //XrefRenameDuplicates(); // ??? do we need this?
            RecollectAdditionalFiles();
            ProcessLists(0);
        }
    }
    if (gSettings->DocumentTree.AutoWidth) {
        for (int i = 0; i < ui->twXrefs->columnCount(); i++)
            ui->twXrefs->resizeColumnToContents(i);
    }
}

void SaveDialog::DocNameChanged(QWidget *editor)
{
    if (qobject_cast<QLineEdit *> (editor)
            && ui->twDocs->currentColumn() == 1) {

        int i, j;
        TWIForSaveMainDoc *item = (TWIForSaveMainDoc *) ui->twDocs->currentItem();
        if (!qobject_cast<QLineEdit *> (editor)->text().endsWith("." + item->PlotConst()->DwgConst()->ExtensionConst(), Qt::CaseInsensitive)) {
            QString lFN = item->text(1) + ".";
            if (ui->cbExtCase->currentIndex() == 1) lFN += item->PlotConst()->DwgConst()->ExtensionConst().toLower();
            else if (ui->cbExtCase->currentIndex() == 2) lFN += item->PlotConst()->DwgConst()->ExtensionConst().toUpper();
            else lFN += item->PlotConst()->DwgConst()->ExtensionConst();
            item->setText(1, lFN);
        }
        // it is just checking that user rename xref correctly
        // clear all backgrounds
        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            ui->twDocs->topLevelItem(i)->setBackground(1, ui->twDocs->topLevelItem(i)->background(2));
        }
        for (i = 0; i < ui->twDocs->topLevelItemCount() - 1; i++) {
            QTreeWidgetItem * itemI = ui->twDocs->topLevelItem(i);
            QString nameI = itemI->text(1);
            for (j = i + 1; j < ui->twDocs->topLevelItemCount(); j++) {
                QTreeWidgetItem * itemJ = ui->twDocs->topLevelItem(j);
                QString nameJ = itemJ->text(1);
                if (nameI == nameJ) {
                    itemI->setBackgroundColor(1, MY_COLOR_ERROR);
                    itemJ->setBackgroundColor(1, MY_COLOR_ERROR);
                }
            }
        }

        if (gSettings->DocumentTree.AutoWidth) {
            for (int i = 0; i < ui->twDocs->columnCount(); i++)
                ui->twDocs->resizeColumnToContents(i);
        }
    }
}

void SaveDialog::AddDocument(int aIdPlot, int aIdDwg) {

    PlotForSaveData * lPlotForSave = new PlotForSaveData(aIdPlot, aIdDwg);

    mPlotForSaveList.append(lPlotForSave);

    TWIForSaveMainDoc *item = new TWIForSaveMainDoc(lPlotForSave);
    if (!item->PlotConst()->Id()
            || !item->PlotConst()->DwgConst()->Id()) {
        delete item;
    } else {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->twDocs->addTopLevelItem(item);
    }
}

void SaveDialog::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        if (!gHasVersionExt) {
            ui->twXrefs->setColumnHidden(6, true);
            ui->twImages->setColumnHidden(6, true);
            ui->twFiles->setColumnHidden(5, true);
        }
        if (ReadVersion < CurrentVersion) {
            ui->cbSaveNewXrefs->setChecked(true);

            int nCol = 0;
            // documents list - AutoCAD version
            ui->twDocs->setColumnWidth(nCol++, 80); // id
            ui->twDocs->setColumnWidth(nCol++, 180); // file name
            ui->twDocs->setColumnWidth(nCol++, 60); // version
            ui->twDocs->setColumnWidth(nCol++, 60); // version for customer
            ui->twDocs->setColumnWidth(nCol++, 50); // history

            ui->twDocs->setColumnHidden(nCol, true);
            ui->twDocs->setColumnWidth(nCol++, 80); // sent date
            ui->twDocs->setColumnWidth(nCol++, 150); // code
            ui->twDocs->setColumnWidth(nCol++, 40); // sheet

            //ui->twDocs->setColumnHidden(nCol, true);
            ui->twDocs->setColumnWidth(nCol++, 280); // name - top
            //ui->twDocs->setColumnHidden(nCol, true);
            ui->twDocs->setColumnWidth(nCol++, 280); // name - bottom

            ui->twDocs->setColumnWidth(nCol++, 80); // ch. date
            ui->twDocs->setColumnWidth(nCol++, 100); // ch. user

            ui->twDocs->setColumnWidth(nCol++, 80); // status
            ui->twDocs->setColumnWidth(nCol++, 40); // extension
            ui->twDocs->setColumnWidth(nCol++, 80); // data size
            ui->twDocs->setColumnWidth(nCol++, 50); // AutoCAD version
            ui->twDocs->setColumnWidth(nCol++, 25); // layouts count

            ui->twDocs->setColumnWidth(nCol++, 30); // need update fields

            ui->twDocs->setColumnWidth(nCol++, 30); // NNP
            ui->twDocs->setColumnWidth(nCol++, 60); // Cache id
            ui->twDocs->setColumnWidth(nCol++, 25); // InSubs

            //ui->twDocs->setColumnHidden(nCol, true);
            //ui->twDocs->setColumnWidth(nCol++, 180);// comments
            mBADocsSettingsAcad = ui->twDocs->header()->saveState();

            // documents list - non AutoCAD version
            nCol = 0;
            ui->twDocs->setColumnWidth(nCol++, 80); // id
            ui->twDocs->setColumnWidth(nCol++, 180); // file name
            ui->twDocs->setColumnWidth(nCol++, 60); // version
            ui->twDocs->setColumnWidth(nCol++, 60); // version for customer
            ui->twDocs->setColumnWidth(nCol++, 50); // history

            ui->twDocs->setColumnHidden(nCol, true);
            ui->twDocs->setColumnWidth(nCol++, 80); // sent date
            ui->twDocs->setColumnWidth(nCol++, 150); // code
            ui->twDocs->setColumnHidden(nCol++, true); // sheet

            //ui->twDocs->setColumnHidden(nCol, true);
            ui->twDocs->setColumnWidth(nCol++, 280); // name - top
            //ui->twDocs->setColumnHidden(nCol, true);
            ui->twDocs->setColumnWidth(nCol++, 280); // name - bottom

            ui->twDocs->setColumnWidth(nCol++, 80); // ch. date
            ui->twDocs->setColumnWidth(nCol++, 100); // ch. user

            ui->twDocs->setColumnWidth(nCol++, 80); // status
            ui->twDocs->setColumnWidth(nCol++, 40); // extension
            ui->twDocs->setColumnWidth(nCol++, 80); // data size
            ui->twDocs->setColumnHidden(nCol++, true); // AutoCAD version
            ui->twDocs->setColumnHidden(nCol++, true); // layouts count

            ui->twDocs->setColumnHidden(nCol++, true); // need update fields

            ui->twDocs->setColumnHidden(nCol++, true); // NNP
            ui->twDocs->setColumnHidden(nCol++, true); // Cache id
            ui->twDocs->setColumnHidden(nCol++, true); // InSubs

            //ui->twDocs->setColumnHidden(nCol, true);
            //ui->twDocs->setColumnWidth(nCol++, 180);// comments
            mBADocsSettingsNonAcad = ui->twDocs->header()->saveState();
            //ui->twDocs->header()->restoreState(baDocsSettingsAcad);

            //----------------------------------------------------------------
            nCol = 0;
            ui->twXrefs->setColumnWidth(nCol++, 180); // file name
            ui->twXrefs->setColumnWidth(nCol++, 180); // new file name
            ui->twXrefs->setColumnWidth(nCol++, 180); // properties
            ui->twXrefs->setColumnWidth(nCol++, 80); // use id
            ui->twXrefs->setColumnWidth(nCol++, 50); // use hist.
            ui->twXrefs->setColumnWidth(nCol++, 60); // version
            ui->twXrefs->setColumnWidth(nCol++, 60); // version for customer
            ui->twXrefs->setColumnWidth(nCol++, 50); // hist

            ui->twXrefs->setColumnHidden(nCol, true);
            ui->twXrefs->setColumnWidth(nCol++, 80); // sent date

            ui->twXrefs->setColumnWidth(nCol++, 150); // code
            ui->twXrefs->setColumnWidth(nCol++, 40); // sheet

            //ui->twXrefs->setColumnHidden(nCol, true);
            ui->twXrefs->setColumnWidth(nCol++, 280); // name - top
            //ui->twXrefs->setColumnHidden(nCol, true);
            ui->twXrefs->setColumnWidth(nCol++, 280); // name - bottom

            ui->twXrefs->setColumnWidth(nCol++, 80); // ch. date
            ui->twXrefs->setColumnWidth(nCol++, 100); // ch. user

            ui->twXrefs->setColumnWidth(nCol++, 80); // status
            ui->twXrefs->setColumnWidth(nCol++, 80); // size
            ui->twXrefs->setColumnWidth(nCol++, 50); // AutoCAD version

            ui->twXrefs->setColumnWidth(nCol++, 30); // nnp
            ui->twXrefs->setColumnWidth(nCol++, 60); // cache id

            ui->twXrefs->setColumnWidth(nCol++, 25); // Internal Version
            //----------------------------------------------------------------
            InitListHeader(ui->twImages);

            InitListHeader(ui->twFiles);
        }

        mViewModePrev = -1;

        // always start from "Document" tab
        ui->tabDocs->setCurrentIndex(0);

        RecollectData();
        ui->lbMS->setText(QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - mStartMSecsSinceEpoch)) / 1000));

        connect(ui->twXrefs, SIGNAL(TreeChanged()), this, SLOT(XrefTreeChanged()));
    }
}

void SaveDialog::SaveAdditionalSettings(QSettings &aSettings) {
    aSettings.setValue("Path", ui->lePathName->text());

    aSettings.setValue("Use AutoCAD version", ui->cbUseAutocad->currentText());

    aSettings.setValue("Make subdir", ui->cbMakeSubdir->checkState());

    aSettings.setValue("Save xrefs", ui->cbXrefs->checkState());
    aSettings.setValue("Save in AutoCAD version", ui->cbSaveAcad->currentIndex());

    aSettings.setValue("Save new xrefs", ui->cbSaveNewXrefs->checkState());
    aSettings.setValue("Save add. files", ui->cbAddFiles->checkState());

    aSettings.setValue("Changer", ui->cbChanger->currentText());
    aSettings.setValue("Name case", ui->cbNameCase->currentIndex());
    aSettings.setValue("Ext. case", ui->cbExtCase->currentIndex());

    aSettings.setValue("Prefix", ui->lePrefix->text());
    aSettings.setValue("Field 1", ui->cbFNPart1->currentIndex());
    aSettings.setValue("Middlefix", ui->leMiddlexif->text());
    aSettings.setValue("Field 2", ui->cbFNPart2->currentIndex());
    aSettings.setValue("Postfix", ui->lePostfix->text());
    aSettings.setValue("UseX", ui->cbUseX->currentIndex());

    aSettings.setValue("Xref name case", ui->cbXrefNameCase->currentIndex());
    aSettings.setValue("Xref extension", ui->cbXrefExt->currentIndex());

    // get current state (user can change it)
    switch (ui->cbViewMode->currentIndex()) {
    case 0:
        mBADocsSettingsAcad = ui->twDocs->header()->saveState();
        break;
    case 1:
        mBADocsSettingsNonAcad = ui->twDocs->header()->saveState();
        break;
    }

    aSettings.setValue("DocsSettingsAcad", mBADocsSettingsAcad);
    aSettings.setValue("DocsSettingsNonAcad", mBADocsSettingsNonAcad);

}

void SaveDialog::LoadAdditionalSettings(QSettings &aSettings) {
    ui->lePathName->setText(aSettings.value("Path").toString());

    QString lUseAutoCADVer = aSettings.value("Use AutoCAD version").toString();

    QList<AcadParamData *> &lAcadParams = gSettings->AcadParamsRef();

    for (int i = 0; i < lAcadParams.length(); i++) {
        ui->cbUseAutocad->addItem(lAcadParams.at(i)->FullDisplayName(), i);
        if (lAcadParams.at(i)->FullDisplayName() == lUseAutoCADVer) {
            ui->cbUseAutocad->setCurrentIndex(i);
        }
    }

    if (lAcadParams.length() < 2) {
        ui->cbUseAutocad->setVisible(false);
        ui->lblUseAtocad->setVisible(false);
    }
    //--------------------------------------------------------------

    ui->cbMakeSubdir->setCheckState((Qt::CheckState) aSettings.value("Make subdir").toInt());

    ui->cbXrefs->setCheckState((Qt::CheckState) aSettings.value("Save xrefs").toInt());
    ui->cbSaveAcad->setCurrentIndex(aSettings.value("Save in AutoCAD version").toInt());

    ui->cbSaveNewXrefs->setCheckState((Qt::CheckState) aSettings.value("Save new xrefs").toInt());
    ui->cbSaveNewXrefs->setEnabled(ui->cbXrefs->isChecked());

    ui->cbAddFiles->setCheckState((Qt::CheckState) aSettings.value("Save add. files").toInt());

    ui->cbChanger->setCurrentText(aSettings.value("Changer", "-").toString());
    ui->cbNameCase->setCurrentIndex(aSettings.value("Name case").toInt());
    ui->cbExtCase->setCurrentIndex(aSettings.value("Ext. case").toInt());

    if (aSettings.value("Prefix") != QVariant()
            || aSettings.value("Field 1") != QVariant()
            || aSettings.value("Middlefix") != QVariant()
            || aSettings.value("Field 2") != QVariant()
            || aSettings.value("Postfix") != QVariant()) {
        ui->lePrefix->setText(aSettings.value("Prefix").toString());
        ui->cbFNPart1->setCurrentIndex(aSettings.value("Field 1").toInt());
        ui->leMiddlexif->setText(aSettings.value("Middlefix").toString());
        ui->cbFNPart2->setCurrentIndex(aSettings.value("Field 2").toInt());
        ui->lePostfix->setText(aSettings.value("Postfix").toString());
        ui->cbUseX->setCurrentIndex(aSettings.value("UseX").toInt());
    } else {
        ui->cbFNPart1->setCurrentIndex(4);
        ui->leMiddlexif->setText("-");
        ui->cbFNPart2->setCurrentIndex(6);
    }

    ui->cbXrefNameCase->setCurrentIndex(aSettings.value("Xref name case").toInt());
    ui->cbXrefExt->setCurrentIndex(aSettings.value("Xref extension").toInt());

    mBADocsSettingsAcad = aSettings.value("DocsSettingsAcad").toByteArray();
    mBADocsSettingsNonAcad = aSettings.value("DocsSettingsNonAcad").toByteArray();
}

void SaveDialog::PopulateImages(TWIForSaveMain * item, bool aIsDwg, bool aAddToList)
{
    int j;
    QTreeWidgetItem *itemHeader;

    const QList<DwgForSaveData *> &mImages =
            (item->GetItemType() == TWIForSaveMain::ICTMain)?
                (static_cast<TWIForSaveMainDoc *>(item)->PlotConst()->ImagesConst()):
                (static_cast<TWIForSaveMainXref *>(item)->XrefConst()->ImagesConst());
    for (j = 0; j < mImages.length(); j++) {
        if (/*gSettings->SaveDialog.ShowImages*/ true) {
            if (!j) {
                if (/*gSettings->SaveDialog.ShowXrefs || gSettings->SaveDialog.ShowAddFiles*/ aIsDwg) {
                    itemHeader = new TWIForSaveHeader("Images");
                    item->addChild(itemHeader);
                    itemHeader->setFirstColumnSpanned(true);
                } else {
                    itemHeader = item;
                }
            }
            TWIForSaveMainAddFile *itemImage = new TWIForSaveMainAddFile(mImages.at(j));
            itemHeader->addChild(itemImage);
        }

        if (aAddToList)
            AddImageToCommonList((item->GetItemType() == TWIForSaveMain::ICTMain)?
                                     static_cast<PlotDwgData *>(static_cast<TWIForSaveMainDoc *>(item)->PlotConst()):
                                     static_cast<PlotDwgData *>(static_cast<TWIForSaveMainXref *>(item)->XrefConst()),
                                 mImages.at(j), aIsDwg?TWIForSaveAddFile::AcadImage:TWIForSaveAddFile::NonAcad);
    }
}

void SaveDialog::PopulateAddFiles(TWIForSaveMain * item)
{
    int j;
    QTreeWidgetItem *itemHeader;

    const QList<DwgForSaveData *> &lFiles =
            (item->GetItemType() == TWIForSaveMain::ICTMain)?
                (static_cast<TWIForSaveMainDoc *>(item)->PlotConst()->AddFilesConst()):
                (static_cast<TWIForSaveMainXref *>(item)->XrefConst()->AddFilesConst());
    for (j = 0; j < lFiles.length(); j++) {
        if (/*gSettings->SaveDialog.ShowAddFiles*/ true) {
            if (!j) {
                if (/*gSettings->SaveDialog.ShowImages || gSettings->SaveDialog.ShowXrefs*/ true) {
                    itemHeader = new TWIForSaveHeader("Support files");
                    item->addChild(itemHeader);
                    itemHeader->setFirstColumnSpanned(true);
                } else {
                    itemHeader = item;
                }
            }
            TWIForSaveMainAddFile *itemAddFile = new TWIForSaveMainAddFile(lFiles.at(j));
            itemHeader->addChild(itemAddFile);
        }
    }
}

void SaveDialog::RecollectData()
{
    int i, j;
    TWIForSaveMainDoc *itemDoc;
    QTreeWidgetItem *itemHeader;

    ui->twXrefs->clear();
    ui->twImages->clear();

    // it is data collect part through previously populated list
    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        itemDoc = (TWIForSaveMainDoc *) ui->twDocs->topLevelItem(i);
        while (itemDoc->childCount()) delete itemDoc->child(0);

        if ((itemDoc->PlotConst()->FileType() < 20 || itemDoc->PlotConst()->FileType() > 29)
                && itemDoc->PlotConst()->DwgConst()->ExtensionConst().toLower() == "dwg") {
            // Acad drawings, collect data
            itemDoc->PlotRef()->InitXrefList(); // xrefs
            itemDoc->PlotRef()->InitXrefPropsList(); // xrefs properties
            itemDoc->PlotRef()->InitFileList(true); // images and additional files ("true" means need to split on two list)

            // populate data
            PopulateImages(itemDoc, true, true);
            PopulateAddFiles(itemDoc);

            const QList<XrefForSaveData *> &lXrefs = itemDoc->PlotConst()->XrefsConst();
            for (j = 0; j < lXrefs.length(); j++) {
                if (/*gSettings->SaveDialog.ShowXrefs*/ true) {
                    if (!j) {
                        if (/*gSettings->SaveDialog.ShowImages || gSettings->SaveDialog.ShowAddFiles*/ true) {
                            itemHeader = new TWIForSaveHeader("Xref's");
                            itemDoc->addChild(itemHeader);
                            itemHeader->setFirstColumnSpanned(true);
                        } else {
                            itemHeader = itemDoc;
                        }
                    }
                    TWIForSaveMainXref *itemXref = new TWIForSaveMainXref(lXrefs.at(j));
                    itemHeader->addChild(itemXref);

                    PopulateImages(itemXref, true, true);
                    //PopulateAddFiles(item2); moved down, call AddSuppFileToCommonList in cycle
                }
                AddXrefToCommonList(itemDoc->PlotConst(), lXrefs.at(j));
            }
        } else {
            // no Acad drawings
            itemDoc->PlotRef()->InitFileList(false);
            // do not add to file list for complicated documents (documents which consist of directory)
            PopulateImages(itemDoc, false, itemDoc->PlotConst()->FileType() < 20 || itemDoc->PlotConst()->FileType() > 29);
        }
    }

    ImageListCheckForDuplicate();
    XrefListCheckForDuplicate();

    on_cbXrefs_toggled(ui->cbXrefs->isChecked());

    RecollectAdditionalFiles();

    ProcessLists(1);

    if (mHasAcadDocs) {
        if (ui->cbViewMode->currentIndex() != 0)
            ui->cbViewMode->setCurrentIndex(0);
        else
            on_cbViewMode_currentIndexChanged(0);
    } else {
        if (ui->cbViewMode->currentIndex() != 1)
            ui->cbViewMode->setCurrentIndex(1);
        else
            on_cbViewMode_currentIndexChanged(1);
    }

    QList<AcadParamData *> &lAcadParams = gSettings->AcadParamsRef();
    const InstalledAcadData *lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(lAcadParams.at(ui->cbUseAutocad->currentData().toInt())->FullProductNameConst());

    if (mMaxAcadVersionForProcess > lInstalledAcadData->Version()) {
        for (i = 0; i < ui->cbUseAutocad->count(); i++) {
            if (mMaxAcadVersionForProcess <= lInstalledAcadData->Version()) {
                ui->cbUseAutocad->setCurrentIndex(i);
                break;
            }
        }
    }

    //if (!ui->twFiles->topLevelItemCount()) ui->tabDocs->removeTab(3); changed - see next line for new version
    //if (!mHasAcadDocs) ui->tabDocs->removeTab(3);
    //if (!ui->twXrefs->topLevelItemCount()) ui->tabDocs->removeTab(2);

    GenerateFilenames();

    if (gSettings->DocumentTree.AutoWidth) {
        for (i = 0; i < ui->twImages->columnCount(); i++)
            ui->twImages->resizeColumnToContents(i);
        for (i = 0; i < ui->twXrefs->columnCount(); i++)
            ui->twXrefs->resizeColumnToContents(i);
    }
}

void SaveDialog::RecollectAdditionalFiles() {
    int i, j;

    ui->twFiles->clear();
    ui->twImages->clear();
    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        //TWIForSaveMainDoc * itemDoc = static_cast<TWIForSaveMainDoc *>(ui->twDocs->topLevelItem(i));
        PlotForSaveData * lPlot = static_cast<TWIForSaveMainDoc *>(ui->twDocs->topLevelItem(i))->PlotRef();

        const QList<DwgForSaveData *> &lFiles = lPlot->AddFilesConst();
        for (j = 0; j < lFiles.length(); j++) {
            AddSuppFileToCommonList(lPlot, lFiles.at(j));
        }

        // images list - for dwg only
        if ((lPlot->FileType() < 20 || lPlot->FileType() > 29)
                && lPlot->DwgConst()->ExtensionConst().toLower() == "dwg") {
            const QList<DwgForSaveData *> &lImages = lPlot->ImagesConst();
            for (j = 0; j < lImages.length(); j++) {
                AddImageToCommonList(lPlot, lImages.at(j), TWIForSaveAddFile::AcadImage);
            }
        }
    }
    for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
        XrefForSaveData * lXref = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i))->XrefRef();

        const QList<DwgForSaveData *> &lFiles = lXref->AddFilesConst();
        for (j = 0; j < lFiles.length(); j++) {
            AddSuppFileToCommonList(lXref, lFiles.at(j));
        }

        // xrefs is always dwg
        const QList<DwgForSaveData *> &lImages = lXref->ImagesConst();
        for (j = 0; j < lImages.length(); j++) {
            AddImageToCommonList(lXref, lImages.at(j), TWIForSaveAddFile::AcadImage);
        }
    }
    FileListCheckForDuplicate();
    on_cbAddFiles_toggled(ui->cbAddFiles->isChecked());

    ui->tabDocs->setTabEnabled(1, ui->twImages->topLevelItemCount());

    if (gSettings->DocumentTree.AutoWidth) {
        for (i = 0; i < ui->twImages->columnCount(); i++)
            ui->twImages->resizeColumnToContents(i);
        for (i = 0; i < ui->twFiles->columnCount(); i++)
            ui->twFiles->resizeColumnToContents(i);
    }
}

void SaveDialog::XrefTreeChanged() {
    XrefListCheckForDuplicate();
    ProcessLists(1);
}

void SaveDialog::StyleSheetChangedInSescendant() {
    if (gSettings->DocumentTree.AutoWidth) {
        int i;
        for (i = 0; i < ui->twDocs->columnCount(); i++)
            ui->twDocs->resizeColumnToContents(i);
        for (i = 0; i < ui->twImages->columnCount(); i++)
            ui->twImages->resizeColumnToContents(i);
        for (i = 0; i < ui->twXrefs->columnCount(); i++)
            ui->twXrefs->resizeColumnToContents(i);
        for (i = 0; i < ui->twFiles->columnCount(); i++)
            ui->twFiles->resizeColumnToContents(i);
    }
}

void SaveDialog::GenerateFilenames() {
    int i, j;
    QList <TWIForSaveMainDoc *> lList;
    QString lReplaceTo = ui->cbChanger->currentText();

    if (!gHasVersionExt && ui->cbFNPart1->currentIndex() == 6) ui->cbFNPart1->setCurrentIndex(5);
    if (!gHasVersionExt && ui->cbFNPart2->currentIndex() == 6) ui->cbFNPart2->setCurrentIndex(5);

    if (ui->cbFNPart1->currentIndex() == 3 || ui->cbFNPart1->currentIndex() == 4
            || ui->cbFNPart2->currentIndex() == 3 || ui->cbFNPart2->currentIndex() == 4) {
        bool lIsAny = false;
        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            TWIForSaveMainDoc * item = static_cast<TWIForSaveMainDoc *>(ui->twDocs->topLevelItem(i));
            if (!item->PlotConst()->SheetConst().isEmpty()
                    && item->PlotConst()->DwgConst()->LayoutCnt() > 1) {
                lIsAny = true;
                break;
            }
        }
        ui->lblUseX->setVisible(lIsAny);
        ui->cbUseX->setVisible(lIsAny);
    } else {
        ui->lblUseX->setVisible(false);
        ui->cbUseX->setVisible(false);
    }


    // it can change order because sort; so we collect it to list
    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        lList.append(static_cast<TWIForSaveMainDoc *>(ui->twDocs->topLevelItem(i)));
    }

    for (i = 0; i < lList.length(); i++) {
        TWIForSaveMainDoc * item = lList.at(i);
        QString lFN = ui->lePrefix->text();

        switch (ui->cbFNPart1->currentIndex()) {
        case 1:
            lFN += QString::number(item->PlotConst()->Id());
            break;
        case 2:
            lFN += item->PlotConst()->CodeConst();
            break;
        case 3:
            if (ui->cbUseX->currentIndex()) {
                lFN += item->GetSheetWithX(ui->cbUseX->currentIndex() == 2);
            } else {
                lFN += item->PlotConst()->SheetConst();
            }
            break;
        case 4:
            if (item->PlotConst()->SheetConst().isEmpty())
                lFN += item->PlotConst()->CodeConst();
            else {
                lFN += item->PlotConst()->CodeConst() + "-";
                if (ui->cbUseX->currentIndex()) {
                    lFN += item->GetSheetWithX(ui->cbUseX->currentIndex() == 2);
                } else {
                    lFN += item->PlotConst()->SheetConst();
                }
            }
            break;
        case 5:
            lFN += item->PlotConst()->VersionIntConst();
            break;
        case 6:
            lFN += item->PlotConst()->VersionExtConst().rightJustified(2, '0');
            break;
        case 7:
            lFN += item->PlotConst()->NameTopConst();
            break;
        case 8:
            lFN += item->PlotConst()->NameConst();
            break;
        case 9:
            lFN += item->PlotConst()->NameTopConst();
            if (lFN.right(1) != ".") lFN += ".";
            if (lFN.right(1) != " ") lFN += " ";
            lFN += item->PlotConst()->NameConst();
            break;
        case 10:
            lFN += item->PlotConst()->BlockNameConst();
            break;
        case 11:
            lFN += QString::number(item->PlotConst()->DwgConst()->Version());
            break;
        }

        if (ui->cbFNPart2->currentIndex()) lFN += ui->leMiddlexif->text();

        switch (ui->cbFNPart2->currentIndex()) {
        case 1:
            lFN += QString::number(item->PlotConst()->Id());
            break;
        case 2:
            lFN += item->PlotConst()->CodeConst();
            break;
        case 3:
            if (ui->cbUseX->currentIndex()) {
                lFN += item->GetSheetWithX(ui->cbUseX->currentIndex() == 2);
            } else {
                lFN += item->PlotConst()->SheetConst();
            }
            break;
        case 4:
            if (item->PlotConst()->SheetConst().isEmpty())
                lFN += item->PlotConst()->CodeConst();
            else {
                lFN += item->PlotConst()->CodeConst() + "-";
                if (ui->cbUseX->currentIndex()) {
                    lFN += item->GetSheetWithX(ui->cbUseX->currentIndex() == 2);
                } else {
                    lFN += item->PlotConst()->SheetConst();
                }
            }
            break;
        case 5:
            lFN += item->PlotConst()->VersionIntConst();
            break;
        case 6:
            lFN += item->PlotConst()->VersionExtConst().rightJustified(2, '0');;
            break;
        case 7:
            lFN += item->PlotConst()->NameTopConst();
            break;
        case 8:
            lFN += item->PlotConst()->NameConst();
            break;
        case 9:
            lFN += item->PlotConst()->NameTopConst();
            if (lFN.right(1) != ".") lFN += ".";
            if (lFN.right(1) != " ") lFN += " ";
            lFN += item->PlotConst()->NameConst();
            break;
        case 10:
            lFN += item->PlotConst()->BlockNameConst();
            break;
        case 11:
            lFN += QString::number(item->PlotConst()->DwgConst()->Version());
            break;
        }

        if (ui->cbNameCase->currentIndex() == 1) lFN = lFN.toLower();
        else if (ui->cbNameCase->currentIndex() == 2) lFN = lFN.toUpper();

        lFN += ui->lePostfix->text();

        lFN.replace("\n\r", " ");
        lFN.replace("\r\n", " ");
        lFN.replace('\n', " ");
        lFN.replace('\r', " ");
        lFN.replace('\t', " ");
        lFN.replace('\\', lReplaceTo);
        lFN.replace('/', lReplaceTo);
        lFN.replace(':', lReplaceTo);
        lFN.replace('*', lReplaceTo);
        lFN.replace('?', lReplaceTo);
        lFN.replace('"', lReplaceTo);
        lFN.replace('<', lReplaceTo);
        lFN.replace('>', lReplaceTo);
        lFN.replace('|', lReplaceTo);

        //lFN.replace('\'', lReplaceTo);

        if (item->PlotConst()->FileType() > 19 && item->PlotConst()->FileType() < 30) {
            if (lList.count() == 1 && ui->cbMakeSubdir->isChecked()
                    || lList.count() > 1) {
                item->setText(1, lFN); // name (need not extension)
            } else {
                item->setText(1, ""); // none name - in current directory
            }
        } else {
            if (lFN.right(1) != ".") lFN += "."; // point before extension

            // extension
            if (ui->cbExtCase->currentIndex() == 1) lFN += item->PlotConst()->DwgConst()->ExtensionConst().toLower();
            else if (ui->cbExtCase->currentIndex() == 2) lFN += item->PlotConst()->DwgConst()->ExtensionConst().toUpper();
            else lFN += item->PlotConst()->DwgConst()->ExtensionConst();

            item->setText(1, lFN);
        }

        item->setBackground(1, item->background(0));
    }

    for (i = 0; i < ui->twDocs->topLevelItemCount() - 1; i++) {
        for (j = i + 1; j < ui->twDocs->topLevelItemCount(); j++) {
            if (ui->twDocs->topLevelItem(i)->text(1) == ui->twDocs->topLevelItem(j)->text(1)) {
                //ui->twDocs->setFocus();
                ui->twDocs->topLevelItem(i)->setBackgroundColor(1, MY_COLOR_ERROR);
                ui->twDocs->topLevelItem(j)->setBackgroundColor(1, MY_COLOR_ERROR);
            }
        }

    }
    if (gSettings->DocumentTree.AutoWidth) {
        for (i = 0; i < ui->twDocs->columnCount(); i++)
            ui->twDocs->resizeColumnToContents(i);
    }
}

void SaveDialog::IdChanged(QWidget *editor)
{
    QTreeWidget *tw = NULL;

    switch (ui->tabDocs->currentIndex()) {
    case 1:
        tw = ui->twImages;
        break;
    case 3:
        tw = ui->twFiles;
        break;
    }

    if (!tw) return;

    for (int i = 0; i < tw->currentItem()->childCount(); i++) {
        if (tw->currentItem()->child(i)->text(2) == tw->currentItem()->text(2)) {
            tw->currentItem()->setText(3, tw->currentItem()->child(i)->text(3));
            break;
        }
    }

//    if (qobject_cast<QComboBox *> (editor)) {
//        QComboBox *lCB = qobject_cast<QComboBox *> (editor);
//        QList<QTableWidgetItem *> selected = ui->tbFiles->selectedItems();
//        for (int i = 0; i < selected.count(); i++) {
//            if (selected.at(i)->column() == 4) {
//                ((XrefTypeItem *) selected.at(i))->SetXrefTypeId(lCB->itemData(lCB->currentIndex()).toInt());
//                selected.at(i)->setText(lCB->currentText());

//            }
//        }
//    }
}

void SaveDialog::on_twDocs_customContextMenuRequested(const QPoint &pos)
{
    if (pos.x() < ui->twDocs->indentation()) {
        QMenu popMenu(this);
        popMenu.addAction(ui->actionExpand_all);
        popMenu.addAction(ui->actionCollapse_all);
        popMenu.exec(QCursor::pos());
    } else {
        QMenu popup(this)/*, *popup2*/;
        //QAction *actColumns, *actShowImages, *actShowXrefs, *actAddFiles, *actRes;
        QAction *actColumns, *actRes;
        QPoint p = QCursor::pos();
        //bool chkShowImages, chkShowXrefs, chkAddFiles, chkChanged;

        //popup2 = popup.addMenu(tr("Show"));
        //popup.addSeparator();
        actColumns = popup.addAction(tr("Columns..."));

        if (actRes = popup.exec(p)) {
            if (actRes == actColumns) {
                QList<int> lDis;
                lDis << 0 << 1;
                if (!gHasVersionExt) lDis << 3;

                SelectColumnsDlg w(this);
                w.SetHeaderView(ui->twDocs->header());
                w.SetDisabledIndexes(lDis);
                w.exec();
            }
        }
    }
}

void SaveDialog::on_actionCollapse_all_triggered()
{
    switch (ui->tabDocs->currentIndex()) {
    case 0:
        ui->twDocs->collapseAll();
        break;
    case 1:
        ui->twImages->collapseAll();
        break;
    case 2:
        ui->twXrefs->collapseAll();
        break;
    case 3:
        ui->twFiles->collapseAll();
        break;
    }

    if (gSettings->DocumentTree.AutoWidth) {
        int i;
        switch (ui->tabDocs->currentIndex()) {
        case 0:
            for (i = 0; i < ui->twDocs->columnCount(); i++)
                ui->twDocs->resizeColumnToContents(i);
            break;
        case 1:
            for (i = 0; i < ui->twImages->columnCount(); i++)
                ui->twImages->resizeColumnToContents(i);
            break;
        case 2:
            for (i = 0; i < ui->twXrefs->columnCount(); i++)
                ui->twXrefs->resizeColumnToContents(i);
            break;
        case 3:
            for (i = 0; i < ui->twFiles->columnCount(); i++)
                ui->twFiles->resizeColumnToContents(i);
            break;
        }
    }
}

void SaveDialog::on_actionExpand_all_triggered()
{
    switch (ui->tabDocs->currentIndex()) {
    case 0:
        ui->twDocs->expandAll();
        break;
    case 1:
        ui->twImages->expandAll();
        break;
    case 2:
        ui->twXrefs->expandAll();
        break;
    case 3:
        ui->twFiles->expandAll();
        break;
    }

    if (gSettings->DocumentTree.AutoWidth) {
        int i;
        switch (ui->tabDocs->currentIndex()) {
        case 0:
            for (i = 0; i < ui->twDocs->columnCount(); i++)
                ui->twDocs->resizeColumnToContents(i);
            break;
        case 1:
            for (i = 0; i < ui->twImages->columnCount(); i++)
                ui->twImages->resizeColumnToContents(i);
            break;
        case 2:
            for (i = 0; i < ui->twXrefs->columnCount(); i++)
                ui->twXrefs->resizeColumnToContents(i);
            break;
        case 3:
            for (i = 0; i < ui->twFiles->columnCount(); i++)
                ui->twFiles->resizeColumnToContents(i);
            break;
        }
    }

}

void SaveDialog::on_tbSelectPath_clicked() {
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::DirectoryOnly);

    dlg.setDirectory(ui->lePathName->text());
    if (dlg.exec() == QDialog::Accepted) {
        ui->lePathName->setText(dlg.selectedFiles().at(0));
    }
}

//--------------------------------------------------------------------------------------------------------------------
void SaveDialog::on_lePrefix_textEdited(const QString &arg1)
{
    GenerateFilenames();
}

void SaveDialog::on_cbFNPart1_currentIndexChanged(int index)
{
    GenerateFilenames();
}

void SaveDialog::on_leMiddlexif_textEdited(const QString &arg1)
{
    GenerateFilenames();
}

void SaveDialog::on_cbFNPart2_currentIndexChanged(int index)
{
    GenerateFilenames();
}

void SaveDialog::on_lePostfix_textEdited(const QString &arg1)
{
    GenerateFilenames();
}

void SaveDialog::on_cbChanger_currentIndexChanged(int index)
{
    GenerateFilenames();
}

void SaveDialog::on_cbChanger_editTextChanged(const QString &arg1)
{
    GenerateFilenames();
}

void SaveDialog::on_cbNameCase_currentIndexChanged(int index)
{
    GenerateFilenames();
}

void SaveDialog::on_cbExtCase_currentIndexChanged(int index)
{
    GenerateFilenames();
}

void SaveDialog::on_twImages_customContextMenuRequested(const QPoint &pos)
{
    if (pos.x() < ui->twDocs->indentation()) {
        QMenu popMenu(this);
        popMenu.addAction(ui->actionExpand_all);
        popMenu.addAction(ui->actionCollapse_all);
        popMenu.exec(QCursor::pos());
    } else {
        QMenu popup(this);
        QAction *actColumns, *actRes;
        QPoint p = QCursor::pos();

        actColumns = popup.addAction(tr("Columns..."));

        if (actRes = popup.exec(p)) {
            if (actRes == actColumns) {
                QList<int> lDis;
                lDis << 0;
                if (!gHasVersionExt) lDis << 6;

                SelectColumnsDlg w(this);
                w.SetHeaderView(ui->twImages->header());
                w.SetDisabledIndexes(lDis);
                w.exec();
            }
        }
    }
}

void SaveDialog::on_twFiles_customContextMenuRequested(const QPoint &pos)
{
    if (pos.x() < ui->twFiles->indentation()) {
        QMenu popMenu(this);
        popMenu.addAction(ui->actionExpand_all);
        popMenu.addAction(ui->actionCollapse_all);
        popMenu.exec(QCursor::pos());
    } else {
        QMenu popup(this);
        QAction *actRemove = NULL, *actColumns = NULL, *actRes;
        QPoint p = QCursor::pos();

        int i;
        bool lIsFound = false;
        for (i = 0; i < ui->twFiles->topLevelItemCount(); i++) {
            if (ui->twFiles->topLevelItem(i)->isSelected()) {
                lIsFound = true;
                break;
            }
        }

        if (lIsFound) {
            actRemove = popup.addAction(tr("Remove from list"));
            popup.addSeparator();
        }
        actColumns = popup.addAction(tr("Columns..."));

        if (actRes = popup.exec(p)) {
            if (actRes == actRemove) {
                if (QMessageBox::question(this, tr("Saving documents"), "Remove selected support files from list?") == QMessageBox::Yes) {
                    for (i = ui->twFiles->topLevelItemCount() - 1; i >= 0; i--) {
                        if (ui->twFiles->topLevelItem(i)->isSelected()) {
                            delete ui->twFiles->topLevelItem(i);
                        }
                    }
                    if (gSettings->DocumentTree.AutoWidth) {
                        for (i = 0; i < ui->twFiles->columnCount(); i++)
                            ui->twFiles->resizeColumnToContents(i);
                    }
                }
            } else if (actRes == actColumns) {
                QList<int> lDis;
                lDis << 0 << 2;
                if (!gHasVersionExt) lDis << 5;

                SelectColumnsDlg w(this);
                w.SetHeaderView(ui->twFiles->header());
                w.SetDisabledIndexes(lDis);
                w.exec();
            }
        }
    }
}

void SaveDialog::on_twXrefs_customContextMenuRequested(const QPoint &pos)
{
    if (pos.x() < ui->twXrefs->indentation()) {
        QMenu popMenu(this);
        popMenu.addAction(ui->actionExpand_all);
        popMenu.addAction(ui->actionCollapse_all);
        popMenu.exec(QCursor::pos());
    } else {
        QMenu popup(this);
        QAction *actColumns, *actRes;
        QPoint p = QCursor::pos();

        actColumns = popup.addAction(tr("Columns..."));

        if (actRes = popup.exec(p)) {
            if (actRes == actColumns) {
                QList<int> lDis;
                lDis << 0 << 1 << 2 << 3 << 4;
                if (!gHasVersionExt) lDis << 6;

                SelectColumnsDlg w(this);
                w.SetHeaderView(ui->twXrefs->header());
                w.SetDisabledIndexes(lDis);
                w.exec();
            }
        }
    }
}

void SaveDialog::on_cbAddFiles_toggled(bool checked) {
    if (!checked && ui->tabDocs->currentIndex() == 3) {
        ui->tabDocs->setCurrentIndex(0);
    }

    ui->tabDocs->setTabEnabled(3, checked && (ui->twFiles->topLevelItemCount() > 0));
}

void SaveDialog::on_cbXrefs_toggled(bool checked) {
    if (!checked && ui->tabDocs->currentIndex() == 2) {
        ui->tabDocs->setCurrentIndex(0);
    }

    ui->tabDocs->setTabEnabled(2, checked && (ui->twXrefs->topLevelItemCount() > 0));
    ui->gbXrefNames->setVisible(checked && (ui->twXrefs->topLevelItemCount() > 0));
    ui->cbSaveNewXrefs->setEnabled(checked && (ui->twXrefs->topLevelItemCount() > 0));
}

void SaveDialog::on_cbXrefNameCase_currentIndexChanged(int index) {
    int i;
    for (i = 0; i < ui->twXrefs->topLevelItemCount(); i++) {
        TWIForSaveXrefTop * item = static_cast<TWIForSaveXrefTop *>(ui->twXrefs->topLevelItem(i));
        switch (index) {
        case 0:
            item->setText(0, item->XrefConst()->BlockNameConst());
            break;
        case 1:
            item->setText(0, item->XrefConst()->BlockNameConst().toLower());
            break;
        case 2:
            item->setText(0, item->XrefConst()->BlockNameConst().toUpper());
            break;
        }
    }
}

void SaveDialog::on_cbSaveNewXrefs_toggled(bool checked) {
    if (checked) {
        XrefListCheckForDuplicate();
        RecollectAdditionalFiles();
    }
}

void SaveDialog::on_cbViewMode_currentIndexChanged(int index)
{
    switch (mViewModePrev) {
    case 0:
        mBADocsSettingsAcad = ui->twDocs->header()->saveState();
        break;
    case 1:
        mBADocsSettingsNonAcad = ui->twDocs->header()->saveState();
        break;
    }

    switch (index) {
    case 0:
        ui->twDocs->header()->restoreState(mBADocsSettingsAcad);
        break;
    case 1:
        ui->twDocs->header()->restoreState(mBADocsSettingsNonAcad);
        break;
    }

    if (!gHasVersionExt) {
        ui->twDocs->setColumnHidden(3, true);
    }

    mViewModePrev = index;
}

void SaveDialog::on_cbMakeSubdir_toggled(bool checked)
{
    GenerateFilenames();
}

void SaveDialog::on_cbUseX_currentIndexChanged(int index)
{
    GenerateFilenames();
}

void SaveDialog::on_cbUseAutocad_currentIndexChanged(int index) {
    QList<AcadParamData *> &lAcadParams = gSettings->AcadParamsRef();
    const InstalledAcadData *lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(lAcadParams.at(ui->cbUseAutocad->currentData().toInt())->FullProductNameConst());

    int lSelectedVersion = lInstalledAcadData->Version();
/*    int i;
    bool lIsFound;*/

    if (!mMaxAcadVersionForProcess) return;

    while (mMaxAcadVersionForProcess > lSelectedVersion
           && ui->cbUseAutocad->currentIndex() < ui->cbUseAutocad->count() - 1) {
        ui->cbUseAutocad->setCurrentIndex(ui->cbUseAutocad->currentIndex() + 1);
        lSelectedVersion = lInstalledAcadData->Version();
    }

    if (mMaxAcadVersionForProcess > lSelectedVersion) {
        QMessageBox::critical(NULL, tr("Saving documents"), tr("Minimum required AutoCAD version is") + " " + QString::number(mMaxAcadVersionForProcess));
    }
/*    if (lSelectedVersion >= 2010 && mMaxAcadVersionForProcess > 7) {
        lIsFound = false;
        for (i = 0; i < ui->cbSaveAcad->count(); i++) {
            if (ui->cbSaveAcad->itemData(i) == 7) {
                lIsFound = true;
                break;
            }
        }
        if (!lIsFound) {
            ui->cbSaveAcad->addItem("2007", 7);
        }
    } else {
        // < 2010
        for (i = ui->cbSaveAcad->count() - 1; i >= 0; i--) {
            if (ui->cbSaveAcad->itemData(i) == 7) {
                ui->cbSaveAcad->removeItem(i);
            }
        }
    }

    if (lSelectedVersion >= 2013 && mMaxAcadVersionForProcess > 10) {
        lIsFound = false;
        for (i = 0; i < ui->cbSaveAcad->count(); i++) {
            if (ui->cbSaveAcad->itemData(i) == 10) {
                lIsFound = true;
                break;
            }
        }
        if (!lIsFound) {
            ui->cbSaveAcad->addItem("2010", 10);
        }
    } else {
        // < 2013
        for (i = ui->cbSaveAcad->count() - 1; i >= 0; i--) {
            if (ui->cbSaveAcad->itemData(i) == 10) {
                ui->cbSaveAcad->removeItem(i);
            }
        }
    }

    ui->lblSaveAcad->setVisible(ui->cbSaveAcad->count() > 1);
    ui->cbSaveAcad->setVisible(ui->cbSaveAcad->count() > 1);*/
}

void SaveDialog::on_pbSettings_clicked()
{
    DocTreeSettings w(this);

    w.SetShowGridLines(gSettings->DocumentTree.ShowGridLines);
    w.SetAutoWidth(gSettings->DocumentTree.AutoWidth);

    if (w.exec() == QDialog::Accepted) {
        gSettings->DocumentTree.ShowGridLines = w.ShowGridLines();
        gSettings->DocumentTree.AutoWidth = w.AutoWidth();

        if (gSettings->DocumentTree.AutoWidth) {
            int i;
            for (i = 0; i < ui->twDocs->columnCount(); i++)
                ui->twDocs->resizeColumnToContents(i);
            for (i = 0; i < ui->twImages->columnCount(); i++)
                ui->twImages->resizeColumnToContents(i);
            for (i = 0; i < ui->twXrefs->columnCount(); i++)
                ui->twXrefs->resizeColumnToContents(i);
            for (i = 0; i < ui->twFiles->columnCount(); i++)
                ui->twFiles->resizeColumnToContents(i);
        }

        //ui->twDocs->setUniformRowHeights(gSettings->DocumentTree.UniformRowHeights);
        //ui->twImages->setUniformRowHeights(gSettings->DocumentTree.UniformRowHeights);
        //ui->twXrefs->setUniformRowHeights(gSettings->DocumentTree.UniformRowHeights);
        //ui->twFiles->setUniformRowHeights(gSettings->DocumentTree.UniformRowHeights);
    }
}

void SaveDialog::on_twDocs_expanded(const QModelIndex &index)
{
//    if (gSettings->DocumentTree.AutoWidth) {
//        for (int i = 0; i < ui->twDocs->columnCount(); i++)
//            ui->twDocs->resizeColumnToContents(i);
//    }
}

void SaveDialog::on_twImages_expanded(const QModelIndex &index)
{
//    if (gSettings->DocumentTree.AutoWidth) {
//        for (int i = 0; i < ui->twImages->columnCount(); i++)
//            ui->twImages->resizeColumnToContents(i);
//    }
}

void SaveDialog::on_twXrefs_expanded(const QModelIndex &index)
{
//    if (gSettings->DocumentTree.AutoWidth) {
//        for (int i = 0; i < ui->twXrefs->columnCount(); i++)
//            ui->twXrefs->resizeColumnToContents(i);
//    }
}

void SaveDialog::on_twFiles_expanded(const QModelIndex &index)
{
//    if (gSettings->DocumentTree.AutoWidth) {
//        for (int i = 0; i < ui->twFiles->columnCount(); i++)
//            ui->twFiles->resizeColumnToContents(i);
//    }
}

void SaveDialog::on_cbSaveAcad_currentIndexChanged(int index)
{
    ProcessLists(0);
    if (!mMaxAcadVersionForProcess) return;

    QList<AcadParamData *> &lAcadParams = gSettings->AcadParamsRef();
    const InstalledAcadData *lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(lAcadParams.at(ui->cbUseAutocad->currentData().toInt())->FullProductNameConst());

    int lSelectedVersion = lInstalledAcadData->Version();
    while (mMaxAcadVersionForProcess > lSelectedVersion
           && ui->cbUseAutocad->currentIndex() < ui->cbUseAutocad->count() - 1) {
        ui->cbUseAutocad->setCurrentIndex(ui->cbUseAutocad->currentIndex() + 1);
        lSelectedVersion = lInstalledAcadData->Version();
    }
    if (mMaxAcadVersionForProcess > lSelectedVersion) {
        QMessageBox::critical(NULL, tr("Saving documents"), tr("Minimum required AutoCAD version is") + " " + QString::number(mMaxAcadVersionForProcess));
    }
}
