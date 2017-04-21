#define _CRT_SECURE_NO_WARNINGS

#define MatzTemplate "/common/templates/matz-v1.xls"

#include "PublishReport.h"
#include "ui_PublishReport.h"
#include "entersavename.h"
#include "oracle.h"
#include "GlobalSettings.h"
#include "MainWindow.h"

#include "common.h"

#include "../ProjectLib/ProjectData.h"

#include <QThread>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QFileDialog>

PublishReport::PublishReport(int aId, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PublishReport),
    mId(aId), mJustStarted(true)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
}

PublishReport::~PublishReport() {
    delete ui;
}

void PublishReport::ShowData() {
    QSqlQuery query("SELECT ID, DECODE(ROWNUM, 1, '(last) ' || PUB_NAME, PUB_NAME) FROM"
                    " (SELECT ID, START_DATE, TO_CHAR(START_DATE, 'DD.MM.YY HH24:MI:SS') || ' - ' || USERNAME PUB_NAME"
                    " FROM V_PUB_STATUS ORDER BY START_DATE DESC)"
                    " WHERE ROWNUM < 1001", db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "Publish report", query);
    } else {
        while (query.next()) {
            ui->mPublishList->addItem(query.value(1).toString(), query.value("id"));
            if (mId && query.value("id").toInt() == mId)
                ui->mPublishList->setCurrentIndex(ui->mPublishList->count() - 1);
        }
    }
}

void PublishReport::showEvent(QShowEvent * event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;
        QTimer::singleShot(0, this, SLOT(ShowData()));
    }
}

void DoPublishReport(QWidget *aParent, int aId) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int i;

    int lIdProject = 0;
    QString lBlockName;


    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;
    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/maatz-v1-" + lTimeStamp;
    if (dir.mkpath(lVbsName)) {

        lVbsName += "/maatz-v1.vbs";
        lTemplateName += MatzTemplate;

        lOutName = "maatz-v1-" + lTimeStamp + ".xls";

        QFile data(lVbsName);
        if (data.open(QFile::WriteOnly)) {
            QTextStream out(&data);

            out.setCodec("UTF-16LE");
            out <<
                   "sGetPath = Left(Wscript.ScriptFullName, InStrRev(Wscript.ScriptFullName, \"\\\"))\r\n"
                   "Set oExcel = CreateObject(\"Excel.Application\")\r\n"
                   "'oExcel.visible = true\r\n"
                   "Set oWbook = oExcel.Workbooks.Open(\"" + lTemplateName + "\")\r\n"
                   "oWbook.SaveAs(sGetPath & \"temp-" + lOutName + "\")\r\n"
                   "Set oWsheet = oWbook.Worksheets(2)\r\n";


            QString qres;
            long lQ1Cnt = 0, lRowNum = 12, lColNum;
            QSqlQuery query2("SELECT DISTINCT "
                             " NVL(D.SHEETNAME, D.LAYOUT_NAME), NVL(D.SHEETFULLNAME, NVL(A.DESCRIPTION, C.NAME)),"
                             " SENDREASON, SCALE, REVISION, REPLACE(REVDATE, '.', '/'), C.CODE, C.ID_PROJECT"
                             " FROM V_PUB_DWG A, V_DWG B, V_PLOT_SIMPLE C, V_PUB_SHEET D"
                             " WHERE A.ID_PUB_STATUS = " + QString::number(aId) +
                             " AND A.ID_DWG = B.ID AND B.ID_PLOT = C.ID"
                             " AND A.ID = D.ID_PUB_DWG"
                             " ORDER BY 1", db);
            if (query2.lastError().isValid()) {
                gLogger->ShowSqlError(aParent, "Publish report", query2);
            } else {
                while (query2.next()) {
                    if (!lIdProject) {
                        lIdProject = query2.value("id_project").toInt();
                    }

                    if (!lBlockName.length()) {
                        qres = query2.value("code").toString();
                        if (qres[3] == '-' && qres[6] == '-' && qres[10] == '-') {
                            QString lStage;

                            qres = qres.toLower();
                            if (qres.indexOf("-pd-")) {
                                lStage = "pd";
                            } else if (qres.indexOf("-cd-")) {
                                lStage = "cd";
                            } else if (qres.indexOf("-dd-")) {
                                lStage = "dd";
                            };
                            //dwg-list_aaa-bb-ccc_ee-for_2011-05-01.xls
                            lBlockName = "dwg-list_" + qres.mid(0, 10) + "_" + lStage + "_" + QDate::currentDate().toString(Qt::ISODate);

                        };
                    };

                    if (lRowNum > 11) {
                        out << "oWsheet.Rows(\"" << lRowNum << ":" << lRowNum
                            << "\").Copy(oWsheet.Rows(\"" << (lRowNum + 1) << ":" << (lRowNum + 1) << "\"))\r\n";
                    };
                    out << "oWsheet.Cells(" << lRowNum << ", 1).Value = \"" << (lQ1Cnt + 1) << "\"\r\n";
                    for (lColNum = 0; lColNum < 6; lColNum++) {
                        qres = query2.value(lColNum).toString();
                        qres.replace("\n", "\" & vbLf & \"");
                        qres.replace("\"", "\"\"");
                        if (lColNum == 5
                                && qres.lastIndexOf(QString("20")) != (qres.length() - 4)) {
                            qres.insert(qres.length() - 2, "20");
                        };
                        // moved to project "loading" cos depend on attribute property "isMirroredInX"
                        //                                if (lColNum == 1 && !query2.value(7).isNull()) {
                        //                                    // rotate english text;
                        //                                    int lRStart = -1, lREnd;
                        //                                    for (int i = 0; i < qres.length(); i++) {
                        //                                        if (!(qres[i] >= 0x0590 && qres[i] <= 0x05ff)
                        //                                                && !(qres[i] == 0x20)
                        //                                                && !(qres[i] == ',')
                        //                                                && !(qres[i] == '.')
                        //                                                && !(qres[i] == ':')
                        //                                                ) {
                        //                                            // non hebrew
                        //                                            if (lRStart == -1) lRStart = i;
                        //                                        } else {
                        //                                            // hebrew
                        //                                            if (lRStart != -1) {
                        //                                                lREnd = i - 1;
                        //                                                while (lREnd > lRStart) {
                        //                                                    // swap
                        //                                                    QChar char1 = qres[lRStart];
                        //                                                    qres[lRStart] = qres[lREnd];
                        //                                                    qres[lREnd] = char1;
                        //                                                    lRStart++;
                        //                                                    lREnd--;
                        //                                                };
                        //                                                lRStart = -1;
                        //                                            };
                        //                                        };
                        //                                    };
                        //                                    if (lRStart != -1) {
                        //                                        lREnd = qres.length() - 1;
                        //                                        while (lREnd > lRStart) {
                        //                                            // swap
                        //                                            QChar char1 = qres[lRStart];
                        //                                            qres[lRStart] = qres[lREnd];
                        //                                            qres[lREnd] = char1;
                        //                                            lRStart++;
                        //                                            lREnd--;
                        //                                        };
                        //                                    };
                        //                                };
                        out << "oWsheet.Cells(" << lRowNum << ", " << (lColNum + 2) << ").Value = \"" << qres << "\"\r\n";
                        //if (lRowNum == 20) {
                        //    out << "MsgBox (\"" << lColNum << "\")\n";
                        //    out << "MsgBox (oWsheet.Rows(\"" << lRowNum << ":" << lRowNum << "\").Height)\r\n";
                        //};
                    }

                    qres = query2.value("code").toString();
                    qres += ".DWG";
                    out << "oWsheet.Cells(" << lRowNum << ", 9).Value = \"" << qres << "\"\r\n";

                    lRowNum++;
                    lQ1Cnt++;

                }
            }

            long lRowXrefNum = 12, lPrevOrder = -1;
            QSqlQuery query3("SELECT DISTINCT"
                             " A.BLOCK_NAME, A.DESCRIPTION, A.IS_BACKGROUND, E.AREA, E.ID, E.ORDER_BY"
                             " FROM V_PUB_DWG_XREF A, V_PUB_DWG B, V_DWG C, V_PLOT_SIMPLE D, V_TREEDATA E"
                             " WHERE A.ID_PUB_DWG = B.ID"
                             " AND B.ID_PUB_STATUS = " + QString::number(aId) +
                             " AND A.ID_DWG_XREF = C.ID AND C.ID_PLOT = D.ID"
                             " AND D.TYPE_AREA = E.AREA AND D.TYPE = E.ID"
                             " ORDER BY 3, 4, 6, 1", db);
            bool lQ2AnyRecord = false;
            if (query3.lastError().isValid()) {
                gLogger->ShowSqlError(aParent, "Publish report", query3);
            } else {
                while (query3.next()) {
                    if (lPrevOrder != (query3.value(3).toInt() * 1000 + query3.value(5).toInt())) {
                        if (lPrevOrder != -1) {
                            if (lRowXrefNum >= lRowNum) {
                                out << "oWsheet.Rows(\"" << lRowXrefNum << ":" << lRowXrefNum << "\").Copy(oWsheet.Rows(\""
                                    << (lRowXrefNum + 1) << ":" << (lRowXrefNum + 1) << "\"))\r\n";
                            };
                            lRowXrefNum++;
                        };
                        lPrevOrder = (query3.value(3).toInt() * 1000 + query3.value(5).toInt());
                    };
                    if (lRowXrefNum >= lRowNum) {
                        out << "oWsheet.Rows(\"" << lRowXrefNum << ":" << lRowXrefNum << "\").Copy(oWsheet.Rows(\""
                            << (lRowXrefNum + 1) << ":" << (lRowXrefNum + 1) << "\"))\r\n";
                    };

                    out << "oWsheet.Cells(" << lRowXrefNum << ", " << ((query3.value(2).toInt())?11:10)
                        << ").Value = \"" << query3.value(0).toString() << "\"\r\n";

                    qres = query3.value(1).toString();
                    qres.replace("\n", "\" & vbLf & \"");
                    qres.replace("\"", "\"\"");
                    out << "oWsheet.Cells(" << lRowXrefNum << ", 12).Value = \"" << qres << "\"\r\n";

                    lRowXrefNum++;
                    lQ2AnyRecord = true;
                };
                if (lQ2AnyRecord) lRowXrefNum--;
            };

            out << "oWbook.SaveAs(sGetPath & \"" + lOutName + "\")\r\n"
                   "oWbook.Close True\r\n"
                   "oExcel.Quit\r\n";

            data.flush();
            data.close();
        }

        QProcess proc1;

        // don't work in right way; it stucks when use "cmd /c" (you can kiil cmd then excel start working).
        // it is finished before the xls formed and saved
        //QMessageBox::critical(aParent, "Publish report", QString(qgetenv("COMSPEC")) + " /c " + lVbsName);
        proc1.start(QString(qgetenv("COMSPEC")));
        if (!proc1.waitForStarted(-1)) {
            QMessageBox::critical(aParent, "VBS wait for started", proc1.errorString());
        } else {
            i = 0;
            while (!QFile::exists(lVbsName) && i < 90) {
                QThread::msleep(1000);
                i++;
            }

            proc1.write(("start " + lVbsName + "\r\nexit\r\n").toLatin1());
            proc1.closeWriteChannel();

            if (!proc1.waitForFinished(-1)) {
                QMessageBox::critical(aParent, "VBS wait for finished", proc1.errorString());
            } else {
                QString lVbsNameForRemove(lVbsName);
                bool lOpenView = true;

                lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
                lVbsName += "/" + lOutName;

                QSqlQuery query4("SELECT TYPE_AREA, TYPE,"
                                 " PP.GETPLOTCODE(TYPE_AREA, TYPE, " + QString::number(lIdProject) + ", NULL) FROM DEF_DOCS WHERE ID = 19", db);

                if (query4.lastError().isValid()) {
                    QMessageBox::critical(aParent, "Publish report", query4.lastError().text());
                } else {
                    if (!query4.next()) {
                        QMessageBox::critical(aParent, "Publish report", query4.lastError().text());
                    } else {
                        bool lIsOk;
                        EnterSaveName lESN(aParent);

                        lESN.SetFilename(lBlockName);
                        lESN.SetCode(query4.value(2).toString());
                        lESN.SetNameTop(lBlockName);

                        do {
                            int lIdPlot;
                            lIsOk = false;
                            if (lESN.exec() == QDialog::Accepted) {

                                i = 0;
                                while (!QFile::exists(lVbsName) && i < 90) {
                                    QThread::msleep(1000);
                                    i++;
                                };

                                QFile::remove(lVbsNameForRemove);

                                if (QFile::exists(lVbsName)
                                        && CreateDocument(lIdPlot, lIdProject, query4.value(0).toInt(), query4.value(1).toInt(),
                                                   lESN.Filename(), lESN.Code(), lESN.NameTop(), lESN.Name(), "xls", lVbsName)) {
                                    lIsOk = true;
                                    lOpenView = false;

                                    lVbsName = QCoreApplication::applicationDirPath();
                                    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
                                    lVbsName += "/oraint.exe \"" + db.databaseName() + "\" \""
                                            + db.userName() + "\" \"" + db.password() + "\" \""
                                            + gSettings->CurrentSchema + "\" OpenNonDwg 1 " + QString::number(lIdPlot) + " 1";

                                    if (!QProcess::startDetached(lVbsName)) {
                                        QMessageBox::critical(aParent, QCoreApplication::translate("PublishReport", "Error"),
                                                              QCoreApplication::translate("PublishReport", QT_TRANSLATE_NOOP("PublishReport", "Document has been saved to Projects Base with id = ")) + QString::number(lIdPlot)
                                                              + "\r\n" + QCoreApplication::translate("PublishReport", QT_TRANSLATE_NOOP("PublishReport", "Can't open for editing")));
                                    };
                                };
                            } else lIsOk = true;
                        } while (!lIsOk);
                    }
                }

                if (lOpenView){
                    QProcess proc2;

                    i = 0;
                    while (!QFile::exists(lVbsName) && i < 90) {
                        QThread::msleep(1000);
                        i++;
                    }

                    proc2.start(QString(qgetenv("COMSPEC")) + " /c \"" + lVbsName + "\"");
                    if (!proc2.waitForStarted(-1)) {
                        QMessageBox::critical(aParent, "COMSPEC wait for started", proc2.errorString());
                    } else if (proc2.waitForFinished(-1)) {
                        QDir dir;
                        QFile::remove(lVbsName);

                        lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
                        dir.rmdir(lVbsName);
                    }
                }
            }
        }
    } else {
        gLogger->ShowError(aParent, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " +lVbsName);
    }
}

#define ReportStart(aWorkSheet) \
    QFile data(lVbsName); \
    if (data.open(QFile::WriteOnly)) { \
    QTextStream out(&data); \
    out.setCodec("UTF-16LE"); \
    out << "On Error Resume Next\r\n" \
    "sGetPath = Left(Wscript.ScriptFullName, InStrRev(Wscript.ScriptFullName, \"\\\"))\r\n" \
    "Set oExcel = CreateObject(\"Excel.Application\")\r\n" \
    "'oExcel.visible = true\r\n" \
    "Set oWbook = oExcel.Workbooks.Open(\"" + lTemplateName + "\")\r\n" \
    "oWbook.SaveAs(sGetPath & \"temp-" + lOutName + "\")\r\n" \
    "Set oWsheet = oWbook.Worksheets("#aWorkSheet")\r\n";

#define ReportEnd() \
    out << "If Err.number <> 0 Then\r\n" \
    "  MsgBox Err.source & \" - \" & Err.number & \": \" & Err.description\r\n" \
    "  oWbook.SaveAs(sGetPath & \"error.xls\")\r\n" \
    "Else\r\n" \
    "  oWbook.SaveAs(sGetPath & \"" + lOutName + "\")\r\n" \
    "End If\r\n" \
    "oWbook.Close True\r\n" \
    "oExcel.Quit\r\n"; \
    data.flush(); \
    data.close(); \
    RunAndShowReport(lVbsName, lOutName); \
    QFile::remove(lVbsName + "/temp-" + lOutName); \
    } else { \
    gLogger->ShowError(gMainWindow, QObject::tr("Report"), "Can't create VBS file!"); \
    }

#define MetroTemplate "/common/templates/metro-v1.xls"
void DoListForMetro(QWidget *aParent, const QList<const PlotData *> &aPlotList) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int i;

    QString lBlockName;

    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;
    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/Report-metro-v1-" + lTimeStamp;
    if (dir.mkpath(lVbsName)) {

        lVbsName += "/metro-v1.vbs";
        lTemplateName += MetroTemplate;

        lOutName = "metro-v1-" + lTimeStamp + ".xls";
        ReportStart(3);

        QStringList lIds;

        for (i = 0; i < aPlotList.length(); i++) {
            lIds.append(QString::number(aPlotList.at(i)->Id()));
        }

        long lRowNum = 5;
        QSqlQuery query("select a.id, a.id_common, a.name title, a.code dwg_name, c.name sheet_name,"
                        " a.version_ext, to_char(edit_date, 'dd.mm.yyyy') rev_date"
                        " from v_plot_simple a, v_dwg b, v_dwg_layout c"
                        " where b.id_plot = a.id"
                        " and b.version = (select max(version) from v_dwg where id_plot = a.id)"
                        " and c.id_dwg = b.id"
                        " and a.id in (" + lIds.join(", ") + ")"
                        " order by c.name", db);
        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(aParent, QObject::tr("Report"), query);
        } else {
            while (query.next()) {
                //out << "oWsheet.Rows(4).Copy(oWsheet.Rows(\"" << (lRowNum + 1) << ":" << (lRowNum + 1) << "\"))\r\n";
                out << "oWsheet.Rows(4).Copy(oWsheet.Rows(" << lRowNum << "))\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 1).Value = " << lRowNum - 4 << "\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 5).Value = \"" << qString("title") << "\"\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 6).Value = \"" << qString("dwg_name") << "\"\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 7).Value = \"" << qString("sheet_name") << "\"\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 8).Value = \"" << qString("sheet_name") + ".plt" << "\"\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 9).Value = \"" << qString("version_ext") << "\"\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 10).Value = \"" << qString("version_ext") << "\"\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 11).Value = \"" << qString("rev_date") << "\"\r\n";
                out << "oWsheet.Cells(" << lRowNum << ", 12).Value = \"New\"\r\n";
                lRowNum++;

                QSqlQuery query2("select a.id, a.id_common, a.name title, a.code dwg_name, c.name sheet_name,"
                                 " a.version_ext, to_char(edit_date, 'dd.mm.yyyy') rev_date"
                                 " from v_plot_simple a, v_dwg b, v_dwg_layout c"
                                 " where b.id_plot = a.id"
                                 " and b.version = (select max(version) from v_dwg where id_plot = a.id)"
                                 " and c.id_dwg = b.id"
                                 " and a.id_common = " + query.value("id_common").toString() +
                                 " and a.working = 0"
                                 " and c.name = '" + query.value("sheet_name").toString() + "'"
                                 " order by a.version_ext desc", db);
                if (query2.lastError().isValid()) {
                    gLogger->ShowSqlError(aParent, QObject::tr("Report"), query2);
                } else {
                    while (query2.next()) {
                        out << "oWsheet.Rows(4).Copy(oWsheet.Rows(" << lRowNum << "))\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 1).Value = " << lRowNum - 4 << "\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 5).Value = \"" << query2.value("title").toString() << "\"\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 6).Value = \"" << query2.value("dwg_name").toString() << "\"\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 7).Value = \"" << query2.value("sheet_name").toString() << "\"\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 8).Value = \"" << query2.value("sheet_name").toString() + ".plt" << "\"\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 9).Value = \"" << query2.value("version_ext").toString() << "\"\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 10).Value = \"" << query2.value("version_ext").toString() << "\"\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 11).Value = \"" << query2.value("rev_date").toString() << "\"\r\n";
                        out << "oWsheet.Cells(" << lRowNum << ", 12).Value = \"Old\"\r\n";
                        out << "oWsheet.Rows(" << lRowNum << ").Interior.Color = 12632256\r\n";
                        lRowNum++;
                    }
                }
            }
            out << "oWsheet.Rows(4).Delete(xlUp)\r\n";
            out << "oWsheet.PageSetup.PrintArea = \"$A$1:$N$" << lRowNum - 2 << "\"\r\n";
        }
        ReportEnd();
    } else {
        gLogger->ShowError(aParent, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " +lVbsName);
    }

}

void PublishReport::on_mGrid_customContextMenuRequested(const QPoint &) {
    QMenu popMenu(this);
    QList <QTableWidgetItem *> lSelected = ui->mGrid->selectedItems();

    if (lSelected.count()) {
        popMenu.addAction(ui->actionView_file);
        popMenu.setDefaultAction(ui->actionView_file);
        popMenu.addSeparator();

        popMenu.addAction(ui->actionView);
        popMenu.addAction(ui->actionView_w_o_xrefs_last_hist);
        popMenu.addAction(ui->actionProperties);

        if (lSelected.count() >= ui->mGrid->columnCount() * 2) {
            ui->actionGo_to_documents->setText(tr("Go to documents"));
        } else {
            ui->actionGo_to_documents->setText(tr("Go to document"));
        }
        popMenu.addAction(ui->actionGo_to_documents);

        popMenu.addSeparator();
    }
    popMenu.addAction(ui->actionSelect_directory);
    popMenu.exec(QCursor::pos());
}

bool PublishReport::DoSelectDirectory() {
    QFileDialog dlg(this);
    dlg.setFileMode((QFileDialog::DirectoryOnly));
    if (mDirName.length()) dlg.setDirectory(mDirName);
    if (dlg.exec()) {
        mDirName = dlg.selectedFiles()[0];
        return true;
    };
    return false;
}

void PublishReport::on_actionView_file_triggered() {
    QDir dir(mDirName);
    bool b;

    if (!mDirName.length() || !dir.exists()) {

        do {
            b = DoSelectDirectory();
            if (!b) return;
            dir.setPath(mDirName);
        } while (!dir.exists());
    }

    for (int i = 0; i < ui->mGrid->rowCount(); i++) {
        const QTableWidgetItem *twi = ui->mGrid->item(i, 5);
        const QTableWidgetItem *twiExt = ui->mGrid->item(i, 3);
        if (twi->isSelected() && twiExt->text() != "dummy") {
            QString lFilename;
            if (mDirName.endsWith('/')
                    || mDirName.endsWith('\\')) {
                lFilename = mDirName;
            } else {
                lFilename = mDirName + '\\';
            }
            if (twi->text().length()) {
                lFilename += twi->text() + "." + twiExt->text();
            } else {
                lFilename += ui->mGrid->item(i, 4)->text() + "." + twiExt->text();
            }
            if (QFile::exists(lFilename)) {
                //if (!QProcess::startDetached(QString(qgetenv("COMSPEC")) + " /c start \'" + lFilename + "\'")) {
                if (!QProcess::startDetached(QString(qgetenv("COMSPEC")) + " /c \"" + lFilename + "\"")) {
                    QMessageBox::critical(this, tr(QT_TR_NOOP("Error")), tr(QT_TR_NOOP("Can't open file: ")) + lFilename);
                }
            } else {
                QMessageBox::critical(this, tr(QT_TR_NOOP("Error")), tr(QT_TR_NOOP("File doesn't exists: ")) + lFilename);
            }
        }
    }
}

void PublishReport::on_mGrid_doubleClicked(const QModelIndex &) {
    on_actionView_file_triggered();
}

void PublishReport::on_actionSelect_directory_triggered() {
    DoSelectDirectory();
}

void PublishReport::on_mPublishList_currentIndexChanged(int index) {
    long i, j;
    QColor redColor(176, 0, 0);
    QColor grayColor(176, 176, 176);
    QPalette palette;
    QString lTemplateName;

    QTableWidgetItem *twi;
    QSqlQuery query("SELECT TO_CHAR(START_DATE, 'DD.MM.YY HH24:MI:SS'), TO_CHAR(END_DATE, 'DD.MM.YY HH24:MI:SS'),"
                    " PDF, DWF, PLT, DWG, PLOTTER_NAME, ACAD_VERSION + 2000, USERNAME, COMPUTER, DIR_NAME"
                    " FROM V_PUB_STATUS WHERE ID = " + ui->mPublishList->itemData(index).toString(), db);

    ui->mGrid->setRowCount(0);
    ui->mGrid->setSortingEnabled(false);

    mDirName.clear();

    lTemplateName = QCoreApplication::applicationDirPath();
    lTemplateName.resize(lTemplateName.lastIndexOf(QChar('/')));
    lTemplateName.resize(lTemplateName.lastIndexOf(QChar('/')));
    lTemplateName.push_back(MatzTemplate);

    ui->pbMakeXls->setVisible(QFile::exists(lTemplateName));

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "Database query", query.lastError().text());
        return;
    };

    if (query.next()) {
        palette = ui->eAutocad->palette();
        if (query.value(1).isNull()) {
            palette.setColor(QPalette::Base, redColor);
        };
        ui->eEndTime->setPalette(palette);

        i = 1;
        ui->eEndTime->setText(query.value(i++).toString());

        ui->cbPdf->setChecked(query.value(i++).toInt());
        ui->cbDwf->setChecked(query.value(i++).toInt());
        ui->cbPlt->setChecked(query.value(i++).toInt());
        ui->cbDwg->setChecked(query.value(i++).toInt());
        ui->ePlotName->setText(query.value(i++).toString());

        ui->eAutocad->setText(query.value(i++).toString());
        ui->eUser->setText(query.value(i++).toString());
        ui->eComputer->setText(query.value(i++).toString());

        ui->eFolder->setText(query.value(i).toString());
        mDirName = query.value(i++).toString();

        QSqlQuery query2("SELECT "
                         " C.ID, /*A.ID_PUB_STATUS, A.ID, D.ID,*/"
                         " NULL,"
                         " C.CODE, D.OUTTYPE, D.LAYOUT_NAME, D.SHEETNAME, D.ORIG_CTB, D.PLOT_CTB,"
                         " D.PAPERWIDTH || 'x' || D.PAPERHEIGHT, D.ERROR_TEXT, A.STATUS, D.PLOT_STATUS, B.VERSION,"
                         " (SELECT MAX(VERSION) FROM V_DWG WHERE ID_PLOT = C.ID),"
                         " D.LAYOUT_NAME_WITHDUPLICATE, D.ID_PUB_SHEET_WITHDUPLICATE"
                         " FROM V_PUB_DWG A, V_DWG B, V_PLOT_SIMPLE C, V_PUB_SHEET D"
                         " WHERE A.ID_PUB_STATUS = " + ui->mPublishList->itemData(index).toString() +
                         " AND A.ID_DWG = B.ID AND B.ID_PLOT = C.ID"
                         " AND A.ID = D.ID_PUB_DWG(+)"
                         " ORDER BY D.ORDER_NUM"
                         , db);

        if (query2.lastError().isValid()) {
            QMessageBox::critical(this, "Database query", query2.lastError().text());
        } else {
            long mStatusField = 10;
            bool lIsColor, lAnyError = false;

            i = 0;
            while (query2.next()) {
                j = 0;
                ui->mGrid->insertRow(i);

                lIsColor = (query2.value(mStatusField + 1).isNull() || query2.value(mStatusField + 1).toInt() != 0);

                for (j = 0; j < mStatusField; j++) {
                    QString str;
                    if (!query2.value(j).isNull()) {
                        str = query2.value(j).toString().trimmed();
                    }
                    twi = new QTableWidgetItem(str);
                    twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                    switch (j) {
                    case 0:
                        if (lIsColor) {
                            twi->setBackgroundColor(redColor);
                            twi->setToolTip(query2.value(mStatusField - 1).toString());
                        }
                        twi->setData(Qt::UserRole, query2.value(mStatusField + 2));
                        break;
                    case 1:
                        bool lWasChanged;
                        lWasChanged = query2.value(mStatusField + 2).toInt() < query2.value(mStatusField + 3).toInt();
                        twi->setText(query2.value(mStatusField + 2).toString() + "/" + query2.value(mStatusField + 3).toString());
                        if (lWasChanged) {
                            twi->setBackgroundColor(grayColor);
                        }
                        break;
                    case 5:
                        if (!query2.value(mStatusField + 4).isNull() || !query2.value(mStatusField + 5).isNull()) {
                            QString tip;
                            twi->setBackgroundColor(redColor);
                            if (!query2.value(mStatusField + 4).isNull()) {
                                tip = tr(QT_TR_NOOP("Renamed, sheet name already exists on layout \"%1\"")).arg(query2.value(mStatusField + 4).toString());
                            } else if (!query2.value(mStatusField + 5).isNull()) {
                                QSqlQuery query3(
                                            "SELECT D.CODE, C.ID_PLOT, A.LAYOUT_NAME FROM V_PUB_SHEET A, V_PUB_DWG B, V_DWG C, V_PLOT_SIMPLE D"
                                            " WHERE A.ID_PUB_DWG = B.ID AND B.ID_DWG = C.ID AND C.ID_PLOT = D.ID AND A.ID = " + query2.value(mStatusField + 5).toString());
                                if (query3.lastError().isValid()) {
                                    QMessageBox::critical(this, "Database query", query3.lastError().text());
                                } else {
                                    if (query3.next()) {
                                        tip = tr(QT_TR_NOOP("Renamed, sheet name already exists in drawing with code \"%1\" (id = %2) on layout \"%3\"")).
                                                arg(query3.value(0).toString()).
                                                arg(query3.value(1).toString()).
                                                arg(query3.value(2).toString());
                                    }
                                }
                            }
                            twi->setToolTip(tip);
                        }
                        break;
                    case 9:
                        if (!str.isEmpty()) {
                            lAnyError = true;
                        }
                        break;
                    }

                    twi->setTextAlignment(ui->mGrid->horizontalHeaderItem(j)->textAlignment());
                    //if (!j || (j == 10)) twi->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                    ui->mGrid->setItem(i, j, twi);
                }

                i++;
            }
            ui->mGrid->setColumnHidden(mStatusField - 1, !lAnyError);
            ui->mGrid->resizeColumnsToContents();
            ui->mGrid->resizeRowsToContents();
            for (i = 0; i < ui->mGrid->rowCount(); i++) {
                ui->mGrid->setRowHeight(i, ui->mGrid->rowHeight(i) - 5);
            }
        }
    }

    ui->mGrid->setSortingEnabled(true);
}

void PublishReport::on_pbMakeXls_clicked() {
    DoPublishReport(this, ui->mPublishList->itemData(ui->mPublishList->currentIndex()).toInt());
}

void PublishReport::DoView(bool aWithoutXrefs) {
    int i;
    QList <int> lPlots;

    for (i = 0; i < ui->mGrid->rowCount(); i++) {
        const QTableWidgetItem *twi = ui->mGrid->item(i, 0);
        if (twi->isSelected()
                && !lPlots.contains(twi->text().toInt())) {
            lPlots.append(twi->text().toInt());
        }
    }

    MainDataForCopyToAcad lDataForAcad(1, aWithoutXrefs);
    for (i = 0; i < lPlots.length(); i++) {
        PlotData *lPlot = gProjects->FindByIdPlot(lPlots.at(i));
        if (lPlot
                && (lPlot->FileType() < 20 || lPlot->FileType() > 29) && lPlot->ExtensionConst().toLower() == "dwg") {
            // AutoCAD drawing
            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lPlot->Id(), /*lHistory?lHistory->Id():0*/0, 0, false));
        }
    }
    if (!lDataForAcad.ListConst().isEmpty())
        gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
}

void PublishReport::on_actionView_triggered() {
    DoView(false);
}

void PublishReport::on_actionView_w_o_xrefs_last_hist_triggered() {
    DoView(true);
}

void PublishReport::on_actionProperties_triggered() {
    if (ui->mGrid->currentItem()) {
        gMainWindow->ShowPlotProp(gProjects->FindByIdPlot(ui->mGrid->item(ui->mGrid->currentItem()->row(), 0)->text().toInt()), NULL);
    }
}

void PublishReport::on_actionGo_to_documents_triggered() {
    int i;
    QList <int> lPlots, lHistNums;

    for (i = 0; i < ui->mGrid->rowCount(); i++) {
        const QTableWidgetItem *twi = ui->mGrid->item(i, 0);
        if (twi->isSelected()
                && !lPlots.contains(twi->text().toInt())) {
            lPlots.append(twi->text().toInt());
            lHistNums.append(twi->data(Qt::UserRole).toInt());
        }
    }

    for (i = 0; i < lPlots.length(); i++) {
        PlotData *lPlot = gProjects->FindByIdPlot(lPlots.at(i));
        if (lPlot) {
            PlotHistoryData *lHistory = lPlot->GetHistoryByNum(lHistNums.at(i));
            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlot->IdProject()), lPlot, lHistory);
        }
    }
}
