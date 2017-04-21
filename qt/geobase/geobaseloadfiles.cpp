#include "geobaseloadfiles.h"
#include "xreftypedata.h"
#include "ui_geobaseloadfiles.h"

#include "../VProject/oracle.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/FileUtils.h"
#include "../VProject/MainWindow.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QProcess>
#include <QFileDialog>
#include <QMenu>

extern QList <XrefTypeData> XrefTypeList;

XrefTypeItemDelegate::XrefTypeItemDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * XrefTypeItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QComboBox *cbXrefType;

    cbXrefType = new QComboBox(parent);

    //QMessageBox::critical(NULL, "Saving geobase file - geobase2plot", this->parent()->metaObject()->className());

    if (!XrefTypeList.count()) InitXrefTypeList(parent);

    cbXrefType->addItem("", 0);
    for (int i = 0; i < XrefTypeList.count(); i++) {
        cbXrefType->addItem(XrefTypeList.at(i).GetFilename(), XrefTypeList.at(i).GetId());
    }

    //cbXrefType->setParent(parent);
    return cbXrefType;
}

XrefCommentsItemDelegate::XrefCommentsItemDelegate(QWidget *parent)  :
    QStyledItemDelegate(parent)
{

}


//QWidget * XrefCommentsItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
//{
//    return new QLineEdit(parent);
//}

//----------------------------------------------------------------------------------------------------------------------------------------------

GeobaseLoadFiles::GeobaseLoadFiles(QWidget *parent) :
    QFCDialog(parent, true), AcadXchgDialog(),
    ui(new Ui::GeobaseLoadFiles),
    IdGeobase(0), IdProject(0)
{
    ui->setupUi(this);

    CurrentVersion = 3;

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    // deleted automatically
    XrefTypeItemDelegate *XrefTypeDelegate = new XrefTypeItemDelegate(this);
    ui->tbFiles->setItemDelegateForColumn(4, XrefTypeDelegate);
    connect(XrefTypeDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(XrefTypeCahnged(QWidget *)));

    XrefCommentsItemDelegate *XrefCommentsDelegate = new XrefCommentsItemDelegate(this);
    ui->tbFiles->setItemDelegateForColumn(5, XrefCommentsDelegate);
    connect(XrefCommentsDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(XrefTypeCahnged(QWidget *)));

    ui->tbFiles->setColumnHidden(9, true);
    ui->tbFiles->setColumnHidden(10, true);
}

GeobaseLoadFiles::~GeobaseLoadFiles()
{
    delete ui;
    gOracle->Clean();
}

void GeobaseLoadFiles::XrefTypeCahnged(QWidget *editor) {
    if (qobject_cast<QComboBox *> (editor)) {
        QComboBox *lCB = qobject_cast<QComboBox *> (editor);
        QList<QTableWidgetItem *> selected = ui->tbFiles->selectedItems();
        for (int i = 0; i < selected.count(); i++) {
            if (selected.at(i)->column() == 4) {
                ((XrefTypeItem *) selected.at(i))->SetXrefTypeId(lCB->itemData(lCB->currentIndex()).toInt());
                selected.at(i)->setText(lCB->currentText());

            }
        }
    } else {
        if (qobject_cast<QLineEdit *> (editor)) {
            QLineEdit *lLE = qobject_cast<QLineEdit *> (editor);
            QList<QTableWidgetItem *> selected = ui->tbFiles->selectedItems();
            for (int i = 0; i < selected.count(); i++) {
                if (selected.at(i)->column() == 5) {
                    selected.at(i)->setText(lLE->text());
                }
            }
        }
    }
}

void GeobaseLoadFiles::SetGeobaseData(int aIdGeobase, int aIdProject, const QString &aMaker, const QString &aOrder, const QString &aSite) {
    IdGeobase = aIdGeobase;
    IdProject = aIdProject;
    Maker = aMaker;
    Order = aOrder;
    Site = aSite;
}

bool GeobaseLoadFiles::SelectFiles() {
    QFileDialog dlg;
    QStringList filters;
    filters << "AutoCAD drawing(*.dwg)"
            //<< "Any files (*)"
               ;

    if (!XrefTypeList.count()) InitXrefTypeList(this);

    dlg.setDirectory(ui->leDirectory->text());
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilters(filters);
    if (dlg.exec() == QDialog::Accepted) {
        ui->tbFiles->setRowCount(0);

        qDeleteAll(mFiles);
        mFiles.clear();

        QStringList files = dlg.selectedFiles();
        QString str;
        QTableWidgetItem *twi;

        bool lAnyExisting = false;

        for (int i = 0; i < files.length(); i++) {
            qint64 lOrigFileSize; // it is dummy now; it is used as sum for directory

            XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(files.at(i)));

            if (!gFileUtils->InitDataForLoad(true, *lXchgFileData, lOrigFileSize)) {
                delete lXchgFileData;
                continue;
            }

//            QList<tPairIntIntString> lExistingIds;

//            if (!gOracle->CollectAlreadyLoaded(lXchgFileData->HashOrigConst(), lExistingIds)) {
//                delete lXchgFileData;
//                continue;
//            }

//            if (!lExistingIds.isEmpty()) {
//                QMessageBox mb(this);
//                mb.setWindowTitle(tr("Loading document"));
//                mb.setIcon(QMessageBox::Question);
//                mb.setText(tr("File already loaded in Projects Base"));
//                mb.addButton(tr("Abort loading and goto existing document"), QMessageBox::YesRole);
//                mb.addButton(tr("Ignore and continue loading"), QMessageBox::NoRole);
//                mb.setWindowFlags((mb.windowFlags() & ~(Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)) | Qt::CustomizeWindowHint);

//                QString lDetails;
//                foreach (tPairIntIntString lExistingId, lExistingIds) {
//                    lDetails += QString::number(lExistingId.first.first) + "/" + QString::number(lExistingId.first.second) + " - " + lExistingId.second + "\r\n";
//                }
//                mb.setDetailedText(lDetails);

//                // motherfucker motherfucker
//                QSpacerItem * horizontalSpacer = new QSpacerItem(1000, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
//                QGridLayout * layout = (QGridLayout *) mb.layout();
//                layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

//                // the return values is button number
//                if (!mb.exec()) {
//                    // "YES"
//                    gMainWindow->ShowPlotLists(lExistingIds);
//                    delete lXchgFileData;
//                    return;
//                }
//            }

            mFiles.append(lXchgFileData);

            QString FilenameOnly = files.at(i).section('/', -1);

            // directory name
            if (!i) ui->leDirectory->setText(files.at(i).section('/', 0, -2));

            ui->tbFiles->insertRow(i);

            // 0 - orig file name
            twi = new QTableWidgetItem(FilenameOnly/*files.at(i)*/);
            //if (lIsColor && !j) {
            //    twi->setBackgroundColor(redColor);
            //    twi->setToolTip(query2.value(mStatusField + 2).toString());
            //};
            //if (!j) twi->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            //else if (j == 2) twi->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            twi->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            ui->tbFiles->setItem(i, 0, twi);

            // 1 - autocad version
            twi = new QTableWidgetItem(QString::number(lXchgFileData->AcadVersionOrig()));
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            twi->setTextAlignment(Qt::AlignCenter);
            ui->tbFiles->setItem(i, 1, twi);

            // 2 - original size
            twi = new QTableWidgetItem(gSettings->FormatNumber(lXchgFileData->FileInfoOrigConst().size()));
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            twi->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
            ui->tbFiles->setItem(i, 2, twi);

            // 3 - name in base
            twi = new QTableWidgetItem();
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            ui->tbFiles->setItem(i, 3, twi);

            // 4 - xref type, 10 - edge number
            str.clear();
            int xrefType = 0;
            QString lEdgeNumber;
            for (int j = 0; j < XrefTypeList.count(); j++) {
                if (XrefTypeList.at(j).IsType(FilenameOnly, lEdgeNumber)) {
                    xrefType = XrefTypeList.at(j).GetId();
                    str = XrefTypeList.at(j).GetFilename();
                    break;
                }
            }
            twi = new XrefTypeItem(str, xrefType);
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            twi->setTextAlignment(Qt::AlignCenter);
            ui->tbFiles->setItem(i, 4, twi);
            twi = new QTableWidgetItem(lEdgeNumber);
            ui->tbFiles->setItem(i, 10, twi);

            // 5 - comment
            twi = new QTableWidgetItem();
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            //twi->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
            ui->tbFiles->setItem(i, 5, twi);

            // 6 - new version
            twi = new QTableWidgetItem();
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            twi->setTextAlignment(Qt::AlignCenter);
            ui->tbFiles->setItem(i, 6, twi);

            // 7 - new size
            twi = new QTableWidgetItem();
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            twi->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
            ui->tbFiles->setItem(i, 7, twi);

            // 8 - ID
            twi = new QTableWidgetItem();
//            if (lItem->ExistingId()) twi->setText(QString::number(lItem->ExistingId()));
//            if (lItem->ExistingId() > 0) lAnyExisting = true;
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            twi->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
            ui->tbFiles->setItem(i, 8, twi);

            // 9 - hash
            twi = new QTableWidgetItem(lXchgFileData->HashOrigConst());
            twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            ui->tbFiles->setItem(i, 9, twi);

            // 10 - edge number (inited with 4)
        }

        ui->tbFiles->setColumnHidden(8, !lAnyExisting);

        on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
        return true;
    } else {
        return false;
    }
}

void GeobaseLoadFiles::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (!SelectFiles()) {
        QTimer::singleShot(0, this, SLOT(hide()));
    }
}

void GeobaseLoadFiles::SaveAdditionalSettings(QSettings &settings) {
    settings.setValue("Pathname", ui->leDirectory->text());
    settings.setValue("Make filename", ui->cbMakeFilename->checkState());
    settings.setValue("Part splitter", ui->cbSplitter->currentIndex());
    settings.setValue("Changer", ui->cbChanger->currentIndex());
    settings.setValue("UseFNIfTypeNull", ui->cbUseFN->checkState());

    settings.setValue("Prefix", ui->lePrefix->text());
    settings.setValue("Part1", ui->cbFNPart1->currentIndex());
    settings.setValue("Part2", ui->cbFNPart2->currentIndex());
    settings.setValue("Part3", ui->cbFNPart3->currentIndex());
    settings.setValue("Postfix", ui->lePostfix->text());
}

void GeobaseLoadFiles::LoadAdditionalSettings(QSettings &settings) {
    ui->leDirectory->setText(settings.value("Pathname").toString());
    ui->cbMakeFilename->setCheckState((Qt::CheckState) settings.value("Make filename").toInt());
    ui->cbSplitter->setCurrentIndex(settings.value("Part splitter", 1).toInt());
    ui->cbChanger->setCurrentIndex(settings.value("Changer", 1).toInt());
    ui->cbUseFN->setCheckState((Qt::CheckState) settings.value("UseFNIfTypeNull", Qt::Checked).toInt());

    ui->lePrefix->setText(settings.value("Prefix").toString());
    ui->cbFNPart1->setCurrentIndex(settings.value("Part1", 0).toInt());
    ui->cbFNPart2->setCurrentIndex(settings.value("Part2", 2).toInt());
    ui->cbFNPart3->setCurrentIndex(settings.value("Part3", 4).toInt());
    ui->lePostfix->setText(settings.value("Postfix").toString());
}

bool GeobaseLoadFiles::nativeEvent(const QByteArray & eventType, void * message, long * result) {
    if (AcadXchgDialog::DoNativeEvent(eventType, message, result)) return true;
    return QFCDialog::nativeEvent(eventType, message, result);
}

void GeobaseLoadFiles::on_cbMakeFilename_stateChanged(int arg1) {
    QString lOrigFileName, lNewBaseName, lPart1, lPart2, lPart3;
    ui->frame->setVisible(arg1 == Qt::Checked);

    for (int i = 0; i < ui->tbFiles->rowCount(); i++) {
        lOrigFileName = ui->tbFiles->item(i, 0)->text();
        if (lOrigFileName.lastIndexOf('.') != -1) lOrigFileName = lOrigFileName.left(lOrigFileName.lastIndexOf('.'));
        if (arg1 == Qt::Checked) {
            // part 1
            switch (ui->cbFNPart1->currentIndex()) {
            case 0:
                lPart1 = Order;
                break;
            case 1:
                lPart1 = Site;
                break;
            case 2:
                if (ui->cbUseFN->isChecked()
                        && ui->tbFiles->item(i, 4)->text().isEmpty()) {
                    lPart1 = lOrigFileName;
                } else {
                    lPart1 = ui->tbFiles->item(i, 4)->text();
                }
                break;
            case 3:
                if (ui->cbUseFN->isChecked()
                        && ui->tbFiles->item(i, 4)->text().isEmpty()) {
                    lPart1 = lOrigFileName;
                } else {
                    lPart1 = ui->tbFiles->item(i, 4)->text() + ui->tbFiles->item(i, 10)->text();
                }
                break;
            case 4:
                lPart1 = lOrigFileName;
                break;
            default:
                lPart1.clear();
                break;
            }
            // part 2
            switch (ui->cbFNPart2->currentIndex()) {
            case 1:
                lPart2 = Order;
                break;
            case 2:
                lPart2 = Site;
                break;
            case 3:
                if (ui->cbUseFN->isChecked()
                        && ui->tbFiles->item(i, 4)->text().isEmpty()) {
                    lPart2 = lOrigFileName;
                } else {
                    lPart2 = ui->tbFiles->item(i, 4)->text();
                }
                break;
            case 4:
                if (ui->cbUseFN->isChecked()
                        && ui->tbFiles->item(i, 4)->text().isEmpty()) {
                    lPart2 = lOrigFileName;
                } else {
                    lPart2 = ui->tbFiles->item(i, 4)->text() + ui->tbFiles->item(i, 10)->text();
                }
                break;
            case 5:
                lPart2 = lOrigFileName;
                break;
            default:
                lPart2.clear();
                break;
            }
            // part 3
            switch (ui->cbFNPart3->currentIndex()) {
            case 1:
                lPart3 = Order;
                break;
            case 2:
                lPart3 = Site;
                break;
            case 3:
                if (ui->cbUseFN->isChecked()
                        && ui->tbFiles->item(i, 4)->text().isEmpty()) {
                    lPart3 = lOrigFileName;
                } else {
                    lPart3 = ui->tbFiles->item(i, 4)->text();
                }
                break;
            case 4:
                if (ui->cbUseFN->isChecked()
                        && ui->tbFiles->item(i, 4)->text().isEmpty()) {
                    lPart3 = lOrigFileName;
                } else {
                    lPart3 = ui->tbFiles->item(i, 4)->text() + ui->tbFiles->item(i, 10)->text();
                }
                break;
            case 5:
                lPart3 = lOrigFileName;
                break;
            default:
                lPart3.clear();
                break;
            }

            lNewBaseName = ui->lePrefix->text() + lPart1;
            if (lPart2.length()) {
                if (lNewBaseName.length()) {
                    switch (ui->cbSplitter->currentIndex()) {
                    case 1:
                        lNewBaseName += "-";
                        break;
                    case 2:
                        lNewBaseName += "_";
                        break;
                    }
                }
                lNewBaseName += lPart2;
            }
            if (lPart3.length()) {
                if (lNewBaseName.length()) {
                    switch (ui->cbSplitter->currentIndex()) {
                    case 1:
                        lNewBaseName += "-";
                        break;
                    case 2:
                        lNewBaseName += "_";
                        break;
                    }
                }
                lNewBaseName += lPart3;
            }
            lNewBaseName += ui->lePostfix->text();

            QString lReplaceTo;

            switch (ui->cbChanger->currentIndex()) {
            case 0:
                lReplaceTo = "-";
                break;
            case 1:
                lReplaceTo = "_";
                break;
            }

            lNewBaseName.replace("/", lReplaceTo);
            lNewBaseName.replace("\\", lReplaceTo);
            lNewBaseName.replace("\"", lReplaceTo);
            lNewBaseName.replace("'", lReplaceTo);
            lNewBaseName.replace("*", lReplaceTo);
            lNewBaseName.replace("?", lReplaceTo);
        } else {
            lNewBaseName = lOrigFileName;
        }
        ui->tbFiles->item(i, 3)->setText(lNewBaseName);
    }
    ui->tbFiles->resizeColumnsToContents();
    ui->tbFiles->resizeRowsToContents();

    if (ui->tbFiles->columnWidth(4) < 80) ui->tbFiles->setColumnWidth(4, 80);
}

void GeobaseLoadFiles::Accept() {
    bool lDoAccept = false;

    if (lDoAccept) accept();
}

void GeobaseLoadFiles::on_cbSplitter_editTextChanged(const QString &arg1) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_cbSplitter_currentIndexChanged(int index) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_lePrefix_textChanged(const QString &arg1) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_cbFNPart1_currentIndexChanged(int index) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_cbFNPart2_currentIndexChanged(int index) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_cbFNPart3_currentIndexChanged(int index) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_lePostfix_textChanged(const QString &arg1) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_cbChanger_currentIndexChanged(int index) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}

void GeobaseLoadFiles::on_pushButton_clicked() {
    if (!mFiles.isEmpty()) {
        if (!ProcessDwgsForLoad(lpClearAnnoScales | lpPurgeRegapps | lpExplodeAllProxies | lpRemoveAllProxies | lpAudit, 0, 0, 0, 0, "", "", winId())) return;
        for (int i = 0; i < ui->tbFiles->rowCount() && i < mFiles.length(); i++) {
            if (mFiles.at(i)->ProcessStatus() == XchgFileData::Done
                    || mFiles.at(i)->ProcessStatus() == XchgFileData::DoneWithWarning) {
                ui->tbFiles->item(i, 6)->setText(QString::number(mFiles.at(i)->AcadVersionPrcd()));
                ui->tbFiles->item(i, 7)->setText(gSettings->FormatNumber(mFiles.at(i)->FileInfoPrcdConst().size()));
            }
        }
    }

    //ui->pbLoadToBase->setVisible(!mFiles.isEmpty());
    ui->pbLoadToBase->setEnabled(!mFiles.isEmpty());

    ui->tbFiles->resizeColumnsToContents();
    ui->tbFiles->resizeRowsToContents();
}

//void GeobaseLoadFiles::on_pushButton_clicked() {
//    int lIdList, lIdTP = 0;
//    bool lIsOk = false, lIsAny = false;
//    QTableWidgetItemFile *item;

//    if (gOracle->GetSeqNextVal("seq_id_list", lIdList)) {
//        for (int i = 0; i < ui->tbFiles->rowCount(); i++) {
//            if (!i) lIsOk = true;
//            item = (QTableWidgetItemFile *) ui->tbFiles->item(i, 0);


//            int lOraParam = gSettings->SetParam(item->GetFullFileName());

//            if (ui->tbFiles->item(i, 4)->text() == "tp"
//                    && !lIdTP) {
//                lIdTP = lOraParam;
//            }

//            lIsOk = false;
//            /*if (!lOraParam || !gSettings->InsertIntoIdList(lIdList, lOraParam)) {
//                lIsOk = false;
//                break;
//            } else {
//                item->SetParam(lOraParam);
//            }*/
//        }

//        if (lIsOk) {
//            QString cmdLine;
//            cmdLine = QCoreApplication::applicationDirPath();
//            cmdLine.resize(cmdLine.lastIndexOf(QChar('/')));
//            cmdLine += "/oraint.exe \"" + db.databaseName() + "\" \""
//                    + db.userName() + "\" \"" + db.password() + "\" \""
//                    + gSettings->CurrentSchema + "\" OpenDwg 3 " + QString::number(lIdList) + " 6 1 "
//                    + QString::number(lIdTP);
//            // 3 after "OpenDwg" not used in fact
//            // what to do = 6 - process for loading
//            // 1 - means remove all xrefs
//            // lIdTP - id for calcing edges (it is first TP from list)
//            //QMessageBox::critical(NULL, "Process drawing", cmdLine);

//            QProcess proc1;
//            proc1.start(cmdLine);
//            if (!proc1.waitForStarted(-1)) {
//                gLogger->ShowError(this, "AutoCAD wait for started", proc1.errorString());
//            } else {
//                if (!proc1.waitForFinished(-1)) {
//                    gLogger->ShowError(this, "AutoCAD wait for finished", proc1.errorString());
//                } else {
//                    for (int i = 0; i < ui->tbFiles->rowCount(); i++) {
//                        item = (QTableWidgetItemFile *) ui->tbFiles->item(i, 0);

//                        QSqlQuery query("SELECT A.RESULT, DBMS_LOB.GETLENGTH(B.DATA) FSIZE, PP.GETACADVER(B.DATA) VER"
//                                        " FROM PARAMS A, LOB_TEMP B"
//                                        " WHERE A.ID = " + QString::number(item->GetParam()) +
//                                        " AND A.RESULT = B.ID", db);

//                        if (query.lastError().isValid()) {
//                            gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
//                        } else {
//                            if (query.next()) {
//                                item->SetLobTempId(query.value("RESULT").toInt());
//                                switch (query.value("VER").toInt()) {
//                                case 0:
//                                    ui->tbFiles->item(i, 6)->setText("N/A");
//                                    break;
//                                case 1:
//                                    ui->tbFiles->item(i, 6)->setText("Acad");
//                                    break;
//                                case 2:
//                                    ui->tbFiles->item(i, 6)->setText("2000");
//                                    break;
//                                default:
//                                    ui->tbFiles->item(i, 6)->setText(QString::number(2000 + query.value("VER").toInt()));
//                                    break;
//                                }

//                                ui->tbFiles->item(i, 7)->setText(gSettings->FormatNumber(query.value("FSIZE").toLongLong()));

//                                lIsAny = true;
//                            } else {
//                                //QMessageBox::critical(this, tr("Геоподосновы"), "Данные не найдены");
//                                ui->tbFiles->item(i, 6)->setText("err");
//                                ui->tbFiles->item(i, 7)->setText("");
//                            }
//                        }
//                        QSqlQuery qDel("DELETE FROM PARAMS WHERE ID = " + QString::number(item->GetParam()), db);
//                        if (!qDel.lastError().isValid()) {
//                            qDel.exec();
//                        }
//                    }
//                }
//            }
//        }
//    }
//    //ui->pbLoadToBase->setVisible(lIsAny);
//    ui->pbLoadToBase->setEnabled(lIsAny);

//    ui->tbFiles->resizeColumnsToContents();
//    ui->tbFiles->resizeRowsToContents();
//}

void GeobaseLoadFiles::on_actionDelete_from_list_triggered() {
    ui->tbFiles->removeRow(ui->tbFiles->currentRow());
}

void GeobaseLoadFiles::on_tbFiles_customContextMenuRequested(const QPoint &pos) {
    QMenu popMenu(this);
    popMenu.addAction(ui->actionDelete_from_list);
    //popMenu.addSeparator();
    popMenu.exec(QCursor::pos());
}

void GeobaseLoadFiles::on_pbLoadToBase_clicked() {
    int i;
    int lArea, lType, lNewIdPlot, lNewIdDwg;
    QString lCode;
    bool lIsOk;

    if (GetTreeTypeForGroup(17, lArea, lType)) {
        for (i = ui->tbFiles->rowCount() - 1; i >= 0; i--) {
            if (GetNextPlotCode(lArea, lType, IdProject, lCode)
                    && gOracle->GetSeqNextVal("plot_id_seq", lNewIdPlot)
                    && gOracle->GetSeqNextVal("dwg_id_seq", lNewIdDwg)) {
                if (db.transaction()) {
                    int lIdCommonDummy = 0;
                    XchgFileData *lXchgFileData = mFiles.at(i);
                    lIsOk = false;
                    if (PlotData::INSERT(lNewIdPlot, lIdCommonDummy, IdProject, lArea, lType, "1", "1", "", "", lCode, "",
                                         Site.length()?(Order + " - " + Site + " " + Maker):(Order + " " + Maker),
                                         ui->tbFiles->item(i, 3)->text(), ui->tbFiles->item(i, 3)->text(), "")
                            && PlotData::LOADFROMFILE(true, lNewIdPlot, lNewIdDwg, 0, 0,
                                                      lXchgFileData->FileInfoOrigConst(), lXchgFileData->FileInfoOrigConst().size(), lXchgFileData->HashOrigConst(),
                                                      *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                                      lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef())) {

                        QSqlQuery qInsert(db);

                        qInsert.prepare(
                                    "insert into v_geobase2plot (id_geobase, id_plot, id_xreftype, comments)"
                                    " values (:id_geobase, :id_plot, :id_xreftype, :comments)");
                        if (qInsert.lastError().isValid()) {
                            gLogger->ShowSqlError(this, tr("Загрузка чертежа геоподосновы"), qInsert.lastError());
                        } else {
                            qInsert.bindValue(":id_geobase", IdGeobase);
                            qInsert.bindValue(":id_plot", lNewIdPlot);
                            if (((XrefTypeItem *) ui->tbFiles->item(i, 4))->XrefTypeId()) {
                                qInsert.bindValue(":id_xreftype", ((XrefTypeItem *) ui->tbFiles->item(i, 4))->XrefTypeId());
                            } else {
                                qInsert.bindValue(":id_xreftype", QVariant());
                            }
                            qInsert.bindValue(":comments", ui->tbFiles->item(i, 5)->text());
                            if (!qInsert.exec()) {
                                gLogger->ShowSqlError(this, tr("Загрузка чертежа геоподосновы"), qInsert.lastError());
                            } else {
                                if (!db.commit()) {
                                    gLogger->ShowSqlError(this, tr("Загрузка чертежа геоподосновы"), tr("Can't commit"), db.lastError());
                                } else {
                                    lIsOk = true;
                                }
                            }
                        }
                    }

                    if (!lIsOk) {
                        db.rollback();
                    } else {
                        ui->tbFiles->removeRow(i);
                        mFiles.removeAt(i);
                    }
                } else {
                    gLogger->ShowSqlError(this, tr("Saving geobase file"), tr("Can't start transaction"), db.lastError());
                }
            }
        }
    }
    if (!ui->tbFiles->rowCount()) accept();
}

void GeobaseLoadFiles::on_cbUseFN_stateChanged(int arg1) {
    on_cbMakeFilename_stateChanged(ui->cbMakeFilename->checkState());
}
