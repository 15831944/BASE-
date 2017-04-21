//#include "dialog.h"
#include "common.h"

#include "../Login/Login.h"

//#include "PublishReport.h"
//#include "AuditReport.h"
//#include "../contract-pkz/ContractPkz.h"
#include "GlobalSettings.h"
//#include "../ProjectLib/ProjectCard.h"
//#include "../AcadSupFiles/AcadSupFiles.h"
//#include "PlotListDlg.h"
//#include "TaskProp.h"
//#include "SaveDialog.h"
#include "MainWindow.h"

#include "MSOffice.h"

#include "acad.h"
#include "HomeData.h"

#include <QMenuBar>

//#include <QClipboard>
//#include <QMimeData>
//#include <QByteArray>
#include <QFile>

#include <QApplication>
#include <QTranslator>
#include <QProcess>

#include <QStyleFactory>

#include <QHostInfo>
#include <QtSql/QSqlDriver>

//#include "../PrCa/PrCaMainWindow.h"
//#include "../UsersDlg/UserRight.h"
#include "../UsersDlg/UserData.h"
#include "../UsersDlg/ChangePassDlg.h"
#include "../UsersDlg/CustomerData.h"

//#include "XMLProcess.h"

int main(int argc, char *argv[]) {
    int res = 0;
    QApplication a(argc, argv);
    //QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QCoreApplication::setOrganizationDomain("itjust.me"); // ya ya it's just me motherfucker
    QCoreApplication::setOrganizationName("ItJustMe");
    QCoreApplication::setApplicationName("VProject");


//    //QString lTest("<Cell asd=123 StyleId=\"s196\">hz</Cell><Cell StyleId=\"s196\" ss:Par=\"value1\">some shit#TEST#</Cell>");
//    QString lTest(
//                "<Cell ss:StyleID=\"s119\"><Data ss:Type=\"String\">#PAY_SUM_NDS#</Data></Cell>"
//                "<Cell ss:StyleID=\"s119\"><Data ss:Type=\"String\">#PAY_SUM_FULL#</Data></Cell>"
//                "<Cell ss:StyleID=\"s119\"><Data ss:Type=\"String\">#PAY_SUM_IDX#</Data></Cell>"
//                "<Cell ss:StyleID=\"s119\"/>"
//                "</Row>");
//    // "capturing parenthesis"
//    lTest.replace(QRegExp("StyleID=\"[^>]*\"([^>]*><Data[^>]*)>([^>]*)#PAY_SUM_FULL#"), "StyleID=\"MotherFuckingStyle1\"\\1>\\2#PAY_SUM_FULL#");
//    //lTest.replace(QRegExp("StyleID=\"[^>]*\"([^>]*><Data[^>]*)>#PAY_SUM_FULL#"), "StyleID=\"MotherFuckingStyle1\"\\1>\\2#PAY_SUM_FULL#");
//    QMessageBox::critical(NULL, "", lTest);
//    return 1;


//    QString lTest("40702810638170109640");
//    if (lTest.contains(QRegExp("^[0-9]{20}$"))) {
//        QMessageBox::critical(NULL, "", "YES");
//    } else {
//        QMessageBox::critical(NULL, "", "NO");
//    }
//    return 1;


    // yes, it is work
    //QMessageBox::critical(NULL, "", QHostInfo::localDomainName());

//    QString lTest("CONF-JK-%NNNN%-DF");

//    QMessageBox::critical(NULL, "", lTest);
//    QMessageBox::critical(NULL, "", lTest.replace(QRegExp("%N.*$"), ""));
//    //if (lTest.contains(QRegExp("-[0-9]{6}\.dwg$"))) {
//    if (lTest.contains(QRegExp("%N*%"))) {
//        QMessageBox::critical(NULL, "", "YES");
//    }

//    return 0;



//    QString lTest("$CONT$-CN-XREF08");

//    if (lTest.contains(QRegExp("^\\$CONT\\$-CN-XREF[0-9]*$"))) {
//         QMessageBox::critical(NULL, "", "YES");
//     }

//     return 0;

//    QString lTest("$CONT$-CN-%NNN%");
//    lTest.replace(QRegExp("%N*%"), "[0-9]*");
//    QMessageBox::critical(NULL, "", lTest);
//    return 0;


    QString lLang;
    {
        QSettings lSettings;
        lSettings.beginGroup("LastLogin");
        lLang = lSettings.value("Lang").toString();
        lSettings.endGroup();
    }

    QTranslator trans1, trans2;
    if (trans1.load(QString("VProject_") + lLang, QCoreApplication::applicationDirPath())) {
        QCoreApplication::installTranslator(&trans1);
    }
    if (trans2.load(QString("VProject-cmn_") + lLang, QCoreApplication::applicationDirPath())) {
        QCoreApplication::installTranslator(&trans2);
    }

    //QFont lFont(a.font());
    //lFont.setFamily("Times New Roman");
    //QMessageBox::critical(NULL, "Document properties", lFont.toString());
    //a.setFont(lFont);


//    QClipboard *clipboard = QApplication::clipboard();
//    for (int i = 0; i < clipboard->mimeData()->formats().length(); i++)
//        QMessageBox::critical(NULL, "Document properties", clipboard->mimeData()->formats().at(i));

//    QFile file("c:/test.txt")    ;
//    if (file.open(QFile::WriteOnly)) {
//        file.write(clipboard->mimeData()->data("text/html"));
//        file.close();
//    }

    //QMessageBox::critical(NULL, "Document properties", clipboard->mimeData()->data());

    gLogger->SetDefaultTitle(QApplication::tr("Projects Base"));
    //gLogger->ShowSqlError("Title", "text", db);

    gMSOffice->IsWordInstalled();
    gMSOffice->IsExcelInstalled();

//    if (gMSOffice->IsWordInstalled() == 1) QMessageBox::critical(0, "Document properties", "Word installed");
//    if (gMSOffice->IsExcelInstalled() == 1) QMessageBox::critical(0, "Document properties", "Excel installed");
//    return 0;

    //QMessageBox::critical(NULL, "Database load driver", QSqlDatabase::drivers().join("\n"));

    if (!gSettings->Common.VisualStyle.isEmpty()) {
        QApplication::setStyle(QStyleFactory::create(gSettings->Common.VisualStyle));
        QApplication::setFont(QApplication::font()); // if don't then font in trees remains small
    }

    if (QProcessEnvironment::systemEnvironment().value("PROJ_SERVERTYPE").toLower() == "postgres") {
        db = QSqlDatabase::addDatabase("QPSQL");
    } else {
        db = QSqlDatabase::addDatabase("QOCI");
    }

    if (db.lastError().isValid()) {
        gLogger->ShowSqlError("Database load driver", db);
        return 1;
    }

//    if (db.driver()->hasFeature(QSqlDriver::EventNotifications)) {
//        QMessageBox::critical(NULL, "", "has");
//    } else {
//        QMessageBox::critical(NULL, "", "none");
//    }

//    if (argc > 6) {
//        db.setDatabaseName(argv[3]);
//        db.setUserName(argv[4]);
//        db.setPassword(argv[5]);

//        if (db.open()) {
//            // after connect only; in other case DB is not available
//            gLogger->SetDB(&db);

////            if (db.driver()->hasFeature(QSqlDriver::Transactions) == true) {
////                QMessageBox::critical(NULL, "Database query", "has");
////            };
//            if (gSettings->InitDB(argv[6])) {
//                //gSettings->InitAll();

//                // deprecated
//                //gSettings->gIs64 = atoi(argv[1]) == 64;

//                if (!strcmp(argv[2], "PublishReport")) {
//                    PublishReport w(0);
//                    w.exec();
//                } else if (!strcmp(argv[2], "PublishReportMakeXls")) {
//                    DoPublishReport(NULL, atoi(argv[7]));
//                } else if (!strcmp(argv[2], "AuditReport")) {
//                    AuditReport w;
//                    w.exec();
//                } else if (!strcmp(argv[2], "PlotSave")) {
//                    SaveDialog w;
//                    w.exec();
//                } else if (!strcmp(argv[2], "Main")) {
//                    gMainWindow->showMaximized();
//                    res = a.exec();
//                } else if (!strcmp(argv[2], "contracts")) {
//                    gSettings->InitNDS();
//                    ContractPkz w;
//                    w.exec();
//                } else if (!strcmp(argv[2], "ProjectCard") && argc > 7) {
//                    ProjectCard w;
//                    w.SetIdProject(atoi(argv[7]));
//                    w.exec();
////                } else if (!strcmp(argv[2], "AcadSupportFiles")) {
////                    if (gHasModule("AcadSupFiles")
////                            && gUserRight->CanSelect("v_as_file")
////                            && gHomeData->Get("PLOT_STYLES").toInt() == 1) {
////                        AcadSupFiles w;
////                        w.exec();
////                    }
////                } else if (!strcmp(argv[2], "ASFSync")) {
////                    if (gHasModule("AcadSupFiles")
////                            && gUserRight->CanSelect("v_as_file")
////                            && gHomeData->Get("PLOT_STYLES").toInt() == 1) {
////                        ASF_Sync();
////                    }
//                } else if (!strcmp(argv[2], "TaskNew")) {
//                    TaskProp w;
//                    if (argc > 7)
//                        w.SetIdTask(atoi(argv[7]));
//                    w.exec();
//                }
//            }
//        } else {
//            QMessageBox::critical(NULL, "Database connect error", db.lastError().text());
//        }
//    } else {
    {
//        //db.setDatabaseName("local");
//        db.setDatabaseName("test");
//        //db.setUserName("vladimir");
//        db.setUserName("acadtest");
//        db.setPassword("");
//        if (db.open()) {
//            if (gSettings->InitDB("acad")) {
//                QSettings settings;
//                settings.beginGroup("Windows");
//                settings.beginGroup("MainWindow");
//                if (settings.value("Maximized", true).toBool()) {
//                    gMainWindow->showMaximized();
//                } else {
//                    gMainWindow->show();
//                }
//                settings.endGroup();
//                settings.endGroup();

//                //try {
//                    res = a.exec();
//                //} catch (...) {
//                //    gSettings->SaveSettings();
//                //}
//            }
//        } else {
//            QMessageBox::critical(NULL, "Database connect error", db.lastError().text());
        //        }

        QString lSelectedLang, lSchemaName, lBaseName;
        if (Login(lSelectedLang, lSchemaName, lBaseName)
                && gSettings->InitDB(lSchemaName, lBaseName)) {
            gLogger->SetDB(&db);
            if (lSelectedLang != lLang) {
                QCoreApplication::removeTranslator(&trans1);
                QCoreApplication::removeTranslator(&trans2);

                if (trans1.load(QString("VProject_") + lSelectedLang, QCoreApplication::applicationDirPath())) {
                    QCoreApplication::installTranslator(&trans1);
                }
                if (trans2.load(QString("VProject-cmn_") + lSelectedLang, QCoreApplication::applicationDirPath())) {
                    QCoreApplication::installTranslator(&trans2);
                }
            }

            if (db.password().length() < 7) {
                // change password dialog
                UserData *lUser = gUsers->FindByLogin(db.userName());
                if (lUser) {
                    ChangePassDlg w(lUser, NULL);
                    if (w.exec() != QDialog::Accepted) {
                        return 1;
                    }
                } else {
                    return 1;
                }
            }

            { // it is for deleting settings after it is not needed
                QSettings settings;
                settings.beginGroup("Windows");
                settings.beginGroup("MainWindow");
                if (settings.value("Maximized", true).toBool()) {
                    gMainWindow->showMaximized();
                } else {
                    gMainWindow->show();
                }
                settings.endGroup();
                settings.endGroup();
            }
            gLogger->SetOwner(gMainWindow);


            // speed test
            // in ORACLE first variant is faster then second about ten times
//            {
//                qint64 mStartMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();
//                QSqlQuery qHomeData("select thevalue from homedata where thename = 'ACAD_EDIT_NAME_MODE'", db);
//                if (qHomeData.lastError().isValid()) {
//                    // no error - it is mean NULL (empty string for strings or 0 for numbers)
//                    //gLogger->ShowSqlError("get home data", qHomeData);
//                } else {
//                    if (qHomeData.next()) {
//                        QString lValue = qHomeData.value(0).toString();
//                        lValue += ';';
//                    }
//                }
//                gLogger->ShowErrorInList(NULL, "Processed in, sec", QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - mStartMSecsSinceEpoch)) / 1000));
//            }

//            {
//                qint64 mStartMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();
//                QSqlQuery qHomeData("select thevalue from homedata", db);
//                if (qHomeData.lastError().isValid()) {
//                    // no error - it is mean NULL (empty string for strings or 0 for numbers)
//                    //gLogger->ShowSqlError("get home data", qHomeData);
//                } else {
//                    while (qHomeData.next()) {
//                        QString lValue = qHomeData.value(0).toString();
//                        lValue += ';';
//                    }
//                }
//                gLogger->ShowErrorInList(NULL, "Processed in, sec", QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - mStartMSecsSinceEpoch)) / 1000));
//            }
            // speed test end

//            // select one customer example
//            CustomerData *lCustomerData = gCustomers->CustomerListConst().at(120);
//            lCustomerData = gCustomers->SelectCustomer(lCustomerData);
//            if (lCustomerData) QMessageBox::critical(NULL, "", lCustomerData->ShortNameConst());

//            // select many customers example
//            QList<CustomerData *> lCustomers;
//            lCustomers.append(gCustomers->CustomerListConst().at(120));
//            lCustomers.append(gCustomers->CustomerListConst().at(12));
//            lCustomers.append(gCustomers->CustomerListConst().at(201));
//            if (gCustomers->SelectCustomers(lCustomers)) {
//                QString lStr;
//                for (int i = 0; i < lCustomers.length(); i++) lStr += lCustomers.at(i)->ShortNameConst() + "\n";
//                QMessageBox::critical(NULL, "", lStr);
//            }


//            // select one person example
//            CustomerPerson *lCustomerPerson = gCustomers->GetPersonById(611/*10532*/);
//            lCustomerPerson = gCustomers->SelectCustomerPerson(lCustomerPerson);
//            if (lCustomerPerson) QMessageBox::critical(NULL, "", lCustomerPerson->LastNameConst());

//            // select many persons example
//            CustomerPerson *lCustomerPerson;
//            QList<CustomerPerson *> lCustomerPersons;
//            if (lCustomerPerson = gCustomers->GetPersonById(611)) lCustomerPersons.append(lCustomerPerson);
//            if (lCustomerPerson = gCustomers->GetPersonById(177)) lCustomerPersons.append(lCustomerPerson);
//            if (lCustomerPerson = gCustomers->GetPersonById(304)) lCustomerPersons.append(lCustomerPerson);
//            if (lCustomerPerson = gCustomers->GetPersonById(351)) lCustomerPersons.append(lCustomerPerson);
//            if (gCustomers->SelectCustomersPersons(lCustomerPersons)) {
//                QString lStr;
//                for (int i = 0; i < lCustomerPersons.length(); i++) lStr += lCustomerPersons.at(i)->FullName() + "\n";
//                QMessageBox::critical(NULL, "", lStr);
//            }

            gSettings->CheckLocalCache();
            res = a.exec();
        }
    }
    return res;
}
