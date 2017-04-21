#include "ProjectCard.h"
#include "ui_ProjectCard.h"

#include "../VProject/entersavename.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/common.h"

#include "../UsersDlg/UserData.h"

#include <QDir>
#include <QProcess>
#include <QThread>

#define ReportCommonTemplate "/common/templates/ProjectCard.doc"

void ProjectCard::ShowInMSWord() {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QString lCrPost, lCrUser, lSignPost, lSignUser;
    QDir dir;
    int i, j, lCurRow;

    if (!DoSave()) return;

    if (ui->cbCreator->currentText().isEmpty()) {
        lCrUser = "-";
    } else {
        UserData *lUser;
        lCrUser = ui->cbCreator->currentText();
        if (!(lUser = gUsers->FindByName(lCrUser))) {
            QMessageBox::critical(this, "Карточка проекта - свойства", tr("User") + " " + lCrUser + " " + tr("not found!"));
            return;
        }
        lCrPost = lUser->JobTitleConst();
    }

    if (ui->cbSigner->currentText().isEmpty()) {
        lSignPost.clear();
        lSignUser = "-";
    } else {
        UserData *lUser;
        lSignUser = ui->cbSigner->currentText();
        if (!(lUser = gUsers->FindByName(lSignUser))) {
            QMessageBox::critical(this, "Карточка проекта - свойства", tr("User") + " " + lSignUser + " " + tr("not found!"));
            return;
        }
        lSignPost = lUser->JobTitleConst();
    }


    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportProjCard-" + lTimeStamp;
    dir.setPath(lVbsName);
    if (!dir.exists() && !dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, tr("Error"), tr("Can't create directory") + ": " +lVbsName);
        return;
    }

    lVbsName += "/ReportProjCard.vbs";
    lTemplateName += ReportCommonTemplate;

    lOutName = "ProjectCard-" + QString::number(mIdProject) + ".doc";

    QFile data(lVbsName);
    if (data.open(QFile::WriteOnly)) {
        QTextStream out(&data);

        out.setCodec("UTF-16LE");
        out <<
               "sGetPath = Left(Wscript.ScriptFullName, InStrRev(Wscript.ScriptFullName, \"\\\"))\n"
               "Set oWord = CreateObject(\"Word.Application\")\n"
               "'oWord.visible = true\n"
               "Set oDoc = oWord.Documents.Open(\"" + lTemplateName + "\")\n"
               "oDoc.SaveAs(sGetPath & \"temp-" + lOutName + "\")\n";

        for (j = 0; j < 5; j++) {
            out << "oDoc.Content.Find.Execute \"#created_post#\", False,,,,, True,,, \"" + lCrPost + "\", 2\n";
            out << "oDoc.Content.Find.Execute \"#created_user#\", False,,,,, True,,, \"" + lCrUser + "\", 2\n";
            out << "oDoc.Content.Find.Execute \"#sign_post#\", False,,,,, True,,, \"" + lSignPost + "\", 2\n";
            out << "oDoc.Content.Find.Execute \"#sign_user#\", False,,,,, True,,, \"" + lSignUser + "\", 2\n";
        }

        QTableWidgetItem *twi;
        QString sDocLabel, sNewValue;
        for (i = 0; i < ui->twProps->rowCount(); i++) {
            twi = ui->twProps->item(i, 1);
            if (twi && !twi->text().isEmpty()) {
                sDocLabel = twi->text();
                twi = ui->twProps->item(i, 2);
                if (twi && !twi->text().isEmpty()) {
                    sNewValue = twi->text().replace("\"", "\"\"").replace("\n", "\" & Chr(10) & \"");
                } else {
                    sNewValue = "-";
                }
                for (j = 0; j < 5; j++) {
                    // wdReplaceAll = 2 (last param) - don't work so just repeat five times
                    out << "oDoc.Content.Find.Execute \"#" + sDocLabel + "#\", False,,,,, True,,, \"" + sNewValue + "\", 2\n";
                }
            }
        }

        lCurRow = 2;
        for (i = 0; i < ui->twSR->rowCount(); i++) {
            out << "oDoc.Tables(2).Rows.Add\n";
            out << "oDoc.Tables(2).Borders.Enable = True\n";

            twi = ui->twSR->item(i, 0);
            if (twi && !twi->text().isEmpty()) {
                if (i) {
                    out << "oDoc.Tables(2).Rows.Add\n";
                    out << "oDoc.Tables(2).Borders.Enable = True\n";
                    lCurRow++;
                }
                out << "oDoc.Tables(2).Cell(" << lCurRow << ", 1).Range.Text = \"" + twi->text() + "\"\n";
                out << "oDoc.Tables(2).Cell(" << lCurRow << ", 2).Range.Bold = True\n";
            }

            for (j = 1; j < 5; j++) {
                twi = ui->twSR->item(i, j);
                if (twi && !twi->text().isEmpty()) {
                    out << "oDoc.Tables(2).Cell(" << lCurRow << ", " << j + 1 << ").Range.Text = \"" + twi->text().replace("\"", "\"\"").replace("\n", "\" & Chr(10) & \"") + "\"\n";
                }
            }
            lCurRow++;
        }

        out << "oDoc.SaveAs(sGetPath & \"" + lOutName + "\")\n"
               "oDoc.Close True\n"
               "oWord.Quit\n";

        data.flush();
        data.close();
    };

    QProcess proc1;

    // don't work in right way; it stucks when use "cmd /c" (you can kiil cmd then excel start working).
    // it is finished before the xls formed and saved
    //QMessageBox::critical(aParent, "Database query", QString(qgetenv("COMSPEC")) + " /c " + lVbsName);
    proc1.start(QString(qgetenv("COMSPEC")));
    if (!proc1.waitForStarted(-1)) {
        QMessageBox::critical(this, "VBS wait for started", proc1.errorString());
    } else {

        proc1.write(("start " + lVbsName + "\nexit\n").toLatin1());
        proc1.closeWriteChannel();

        if (!proc1.waitForFinished(-1)) {
            QMessageBox::critical(this, "VBS wait for finished", proc1.errorString());
        } else {
            //QMessageBox::critical(aParent, "Database query", QString(qgetenv("COMSPEC")) + " /c " + lVbsName);
            QString lNameForRemove(lVbsName), lNameForRemove2;
            bool lOpenView = true;

            lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
            lNameForRemove2 = lVbsName + "/temp-" + lOutName;
            lVbsName += "/" + lOutName;

            //            QSqlQuery query4("SELECT TYPE_AREA, TYPE,"
            //                             " PP.GETPLOTCODE(TYPE_AREA, TYPE, " + QString::number(lIdproject) + ", NULL) FROM DEF_DOCS WHERE ID = 19", db);
            QSqlQuery query4("SELECT -1, -2, 'CODE' FROM DUAL", db);


            if (query4.lastError().isValid()) {
                QMessageBox::critical(this, "Database query", query4.lastError().text());
            } else {
                if (!query4.next()) {
                    QMessageBox::critical(this, "Database query", query4.lastError().text());
                } else {
                    bool lIsOk;
                    EnterSaveName lESN;

                    lESN.SetFilename("lBlockName");
                    lESN.SetCode(query4.value(2).toString());
                    lESN.SetNameTop("lBlockName");

                    do {
                        int dlgResult;
                        lIsOk = false;

                        //dlgResult = lESN.exec();
                        dlgResult = QDialog::Rejected;

                        i = 0;
                        while (!QFile::exists(lVbsName) && i < 90) {
                            QThread::msleep(1000);
                            i++;
                        };

                        QFile::remove(lNameForRemove);
                        QFile::remove(lNameForRemove2);

                        if (dlgResult == QDialog::Accepted) {

//                            if (QFile::exists(lVbsName)
//                                    && CreateDocument(lIdPlot, lIdProject, query4.value(0).toInt(), query4.value(1).toInt(),
//                                                      lESN.Filename(), lESN.Code(), lESN.NameTop(), lESN.Name(), "xls", lVbsName)) {
//                                lIsOk = true;
//                                lOpenView = false;

//                                lVbsName = QCoreApplication::applicationDirPath();
//                                lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
//                                lVbsName += "/oraint.exe \"" + db.databaseName() + "\" \""
//                                        + db.userName() + "\" \"" + db.password() + "\" \""
//                                        + gSchemaName + "\" OpenNonDwg 1 " + QString::number(lIdPlot) + " 1";

//                                if (!QProcess::startDetached(lVbsName)) {
//                                    QMessageBox::critical(aParent, QCoreApplication::translate("PublishReport", "Error"),
//                                                          QCoreApplication::translate("PublishReport", QT_TRANSLATE_NOOP("PublishReport", "Document has been saved to Projects Base with id = ")) + QString::number(lIdPlot)
//                                                          + "\n" + QCoreApplication::translate("PublishReport", QT_TRANSLATE_NOOP("PublishReport", "Can't open for editing")));
//                                }
//                            }
                        } else lIsOk = true;
                    } while (!lIsOk);
                }
            }

            if (lOpenView){
                QProcess proc2;

                proc2.start(QString(qgetenv("COMSPEC")) + " /c \"" + lVbsName + "\"");
                if (!proc2.waitForStarted(-1)) {
                    QMessageBox::critical(this, "VBS wait for started", proc2.errorString());
                } else if (proc2.waitForFinished(-1)) {
                    QDir dir;
                    //QMessageBox::critical(NULL, "Contracts - report", lVbsName);
                    QFile::remove(lVbsName);

                    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
                    dir.rmdir(lVbsName);
                }
            }
        }
    }
}
