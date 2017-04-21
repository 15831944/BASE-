#include "ContractPkz.h"
#include "ui_contractpkz.h"

#include "contract-pkz_local.h"
#include "../VProject/XMLProcess.h"
#include "ContractReportLib.h"

#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QDir>

#define ReportFullTemplate "/common/templates/contract-rep-Full.xml"
#define ReportSignedNotPayedTemplate "/common/templates/contract-rep-SignedNotPayed.xml"

void ContractPkz::ReportFull(bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lTemplate, lOutput, lTimeStamp;

    lTemplate = QCoreApplication::applicationDirPath();

    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lOutput = lTemplate;
    lTemplate += ReportFullTemplate;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lOutput += "/temp/data/ReportFull-" + lTimeStamp;
    QDir lDir(lOutput);
    if (!lDir.exists() && !lDir.mkpath(lOutput)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " + lOutput);
        return;
    }

    lOutput += "/ReportFull-" + lTimeStamp;

    QList <QQueryForExcelData *> lQueryList;
    QQueryForExcelData *lQueryProjects, *lQueryContracts, *lQueryTemp;

    MakeQueryProjects(aWithFeaturedPay, lQueryProjects, aProjects, "", 1, 0, true, false);
    lQueryList.append(lQueryProjects);

    MakeQueryRest(aWithFeaturedPay, lQueryTemp, " AND A.ID_PROJECT = :ID_PROJECT");
    lQueryTemp->BindsRef().append(new QQueryForExcelDataBind("ID_PROJECT"));
    lQueryProjects->QueryListRef().append(lQueryTemp);

    MakeQueryContracts(aWithFeaturedPay, lQueryContracts, "", 0);
    lQueryProjects->QueryListRef().append(lQueryContracts);

    MakeQueryPayments(aWithFeaturedPay, lQueryTemp, "", true);
    lQueryContracts->QueryListRef().append(lQueryTemp);

    ExcelUniHandler lExcelUniHandler(lTemplate, lOutput, lQueryList);
    // #FF1919
    // "<Font ss:FontName=\"Arial\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Color=\"#FFFFFF\"/>"
    lExcelUniHandler.SetStyleForAddRaw("<Style ss:ID=\"MotherFuckingStyle1\">"
                                       "<Alignment ss:Horizontal=\"Right\" ss:Vertical=\"Bottom\"/>"
                                       "<Borders>"
                                       "<Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                                       "<Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                                       "<Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                                       "</Borders>"
                                       "<Font ss:Color=\"#FFFFFF\"/>"
                                       "<Interior ss:Color=\"#FF1919\" ss:Pattern=\"Solid\"/>"
                                       "<NumberFormat ss:Format=\"Standard\"/>"
                                       "</Style>");
    QStringList lAddVBA;
    lAddVBA.append("oWsheet.Range(\"C4\").Select");
    lAddVBA.append("oExcel.ActiveWindow.FreezePanes = False");
    lAddVBA.append("oExcel.ActiveWindow.FreezePanes = True");
    lAddVBA.append("oWsheet.Range(\"A1\").Select");
    lExcelUniHandler.SetAddToVBAScript(lAddVBA);
    lExcelUniHandler.Process();
}

void ContractPkz::ReportSignedNotPayed(bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lTemplate, lOutput, lTimeStamp;

    lTemplate = QCoreApplication::applicationDirPath();

    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lOutput = lTemplate;
    lTemplate += ReportSignedNotPayedTemplate;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lOutput += "/temp/data/ReportSignedNotPayed-" + lTimeStamp;
    QDir lDir(lOutput);
    if (!lDir.exists() && !lDir.mkpath(lOutput)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " + lOutput);
        return;
    }

    lOutput += "/ReportSignedNotPayed-" + lTimeStamp;

    QList <QQueryForExcelData *> lQueryList;
    QQueryForExcelData *lQueryProjects, *lQueryContracts, *lQueryTemp;

    MakeQueryProjects(aWithFeaturedPay, lQueryProjects, aProjects, "", 0, 0, true, false);
    lQueryList.append(lQueryProjects);

    MakeQueryContracts(aWithFeaturedPay, lQueryContracts, "", 0);
    lQueryProjects->QueryListRef().append(lQueryContracts);

    MakeQueryPayments(aWithFeaturedPay, lQueryTemp, "", true);
    lQueryContracts->QueryListRef().append(lQueryTemp);

    ExcelUniHandler lExcelUniHandler(lTemplate, lOutput, lQueryList);
    // #FF1919
    // "<Font ss:FontName=\"Arial\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Color=\"#FFFFFF\"/>"
    lExcelUniHandler.SetStyleForAddRaw("<Style ss:ID=\"MotherFuckingStyle1\">"
                                       "<Alignment ss:Horizontal=\"Right\" ss:Vertical=\"Bottom\"/>"
                                       "<Borders>"
                                       "<Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                                       "<Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                                       "<Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                                       "</Borders>"
                                       "<Font ss:Color=\"#FFFFFF\"/>"
                                       "<Interior ss:Color=\"#FF1919\" ss:Pattern=\"Solid\"/>"
                                       "<NumberFormat ss:Format=\"Standard\"/>"
                                       "</Style>");
    QStringList lAddVBA;
    lAddVBA.append("oWsheet.Range(\"C4\").Select");
    lAddVBA.append("oExcel.ActiveWindow.FreezePanes = False");
    lAddVBA.append("oExcel.ActiveWindow.FreezePanes = True");
    lAddVBA.append("oWsheet.Range(\"A1\").Select");
    lExcelUniHandler.SetAddToVBAScript(lAddVBA);
    lExcelUniHandler.Process();
}
