#include "ContractPkz.h"

#include "../VProject/XMLProcess.h"
#include "ContractReportLib.h"

#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QDir>

#define ReportProjByYearTemplate "/common/templates/contract-rep-ProjectByYear.xml"
#define ReportSummaryByYearTemplate "/common/templates/contract-rep-SummaryByYear.xml"
#define ReportMonthlyPaymentsTemplate "/common/templates/contract-rep-MonthlyPayments.xml"
#define ReportByCustomerTemplate "/common/templates/contract-rep-ByCustomer.xml"

//Range("O46").Select
//With Selection.Validation
//    .Delete
//    .Add Type:=xlValidateInputOnly, AlertStyle:=xlValidAlertStop, Operator _
//    :=xlBetween
//    .IgnoreBlank = True
//    .InCellDropdown = True
//    .InputTitle = ""
//    .ErrorTitle = ""
//    .InputMessage = "14809.20"
//    .ErrorMessage = ""
//    .ShowInput = True
//    .ShowError = True
//End With

static void GetYearNumGetCurCallback(const QSqlQuery *aQuery, const QString &aFieldName, int aRecordNum, QVariant &aValue, void *aDataPtr) {
    aValue = ((PGetMonthNameDATA) aDataPtr)->Year;
}

void ContractPkz::SummaryByYear(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lTemplate, lOutput, lTimeStamp;

    lTemplate = QCoreApplication::applicationDirPath();

    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lOutput = lTemplate;
    lTemplate += ReportSummaryByYearTemplate;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lOutput += "/temp/data/ReportMonthlyPayments-" + lTimeStamp;
    QDir lDir(lOutput);
    if (!lDir.exists() && !lDir.mkpath(lOutput)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " + lOutput);
        return;
    }

    lOutput += "/" + QString::number(aYear) + "-ReportSumYear-" + lTimeStamp;


    QList <QQueryForExcelData *> lQueryList;
    QQueryForExcelData *lQueryYears, *lQueryProjects, *lQueryContracts, *lQueryPayments;
    QList <QQueryForExcelDataField *> lFields;
    QQueryForExcelDataField *lQueryForExcelDataField;


    GetMonthNameDATA lMonthData;

    lMonthData.Year = aYear; // no month in this report

    MakeQuerySingleYear(aWithFeaturedPay, lQueryYears, &lMonthData);

    lQueryList.append(lQueryYears);

    MakeQueryProjects(aWithFeaturedPay, lQueryProjects, aProjects, QString() +
                      " AND (EXISTS (SELECT 1 FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON D WHERE B.ID_PROJECT = A.ID AND D.ID_CONTRACT = B.ID" +
                      " AND " + (aWithFeaturedPay?"TO_NUMBER(TO_CHAR(NVL(D.PAY_DATE, D.EXPECT_DATE), 'YYYY')) = :YEAR":"TO_NUMBER(TO_CHAR(D.PAY_DATE, 'YYYY')) = :YEAR") +
                      ") OR EXISTS (SELECT 1 FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D WHERE B.ID_PROJECT = A.ID AND C.ID_PKZ_CONTRACT = B.ID AND D.ID_CONTRACT_STAGE = C.ID"
                      " AND " + (aWithFeaturedPay?"TO_NUMBER(TO_CHAR(NVL(D.PAY_DATE, D.EXPECT_DATE), 'YYYY')) = :YEAR":"TO_NUMBER(TO_CHAR(D.PAY_DATE, 'YYYY')) = :YEAR") +
                      "))", -1, -1, false, false);
    lQueryProjects->BindsRef().append(new QQueryForExcelDataBind("YEAR"));

    lQueryForExcelDataField = new QQueryForExcelDataField("CYEAR");
    lQueryForExcelDataField->SetGetValueCallback(&GetYearNumGetCurCallback);
    lQueryForExcelDataField->SetGetValueCallbackDataPtr(&lMonthData);
    lQueryProjects->FieldsRef().append(lQueryForExcelDataField);

    lQueryYears->QueryListRef().append(lQueryProjects);

    // -------------------------------------------------------------
    MakeQueryContracts(aWithFeaturedPay, lQueryContracts, QString() +
                       " AND (EXISTS (SELECT 1 FROM V_PKZ_HASHBON D WHERE D.ID_CONTRACT = A.ID"
                       " AND " + (aWithFeaturedPay?"TO_NUMBER(TO_CHAR(NVL(D.PAY_DATE, D.EXPECT_DATE), 'YYYY')) = :CYEAR":"TO_NUMBER(TO_CHAR(D.PAY_DATE, 'YYYY')) = :CYEAR") +
                       " ) OR EXISTS (SELECT 1 FROM V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D WHERE C.ID_PKZ_CONTRACT = A.ID AND D.ID_CONTRACT_STAGE = C.ID"
                       " AND " + (aWithFeaturedPay?"TO_NUMBER(TO_CHAR(NVL(D.PAY_DATE, D.EXPECT_DATE), 'YYYY')) = :CYEAR":"TO_NUMBER(TO_CHAR(D.PAY_DATE, 'YYYY')) = :CYEAR") +
                       "))", 0);
    lQueryContracts->BindsRef().append(new QQueryForExcelDataBind("CYEAR"));

    lQueryForExcelDataField = new QQueryForExcelDataField("PYEAR");
    lQueryForExcelDataField->SetGetValueCallback(&GetYearNumGetCurCallback);
    lQueryForExcelDataField->SetGetValueCallbackDataPtr(&lMonthData);
    lQueryContracts->FieldsRef().append(lQueryForExcelDataField);

    lQueryProjects->QueryListRef().append(lQueryContracts);

    // -------------------------------------------------------------

    MakeQueryPayments(aWithFeaturedPay, lQueryPayments,
                      aWithFeaturedPay?" AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = :PYEAR"
                        :" AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = :PYEAR", false);
    lQueryPayments->BindsRef().append(new QQueryForExcelDataBind("PYEAR"));
    lQueryContracts->QueryListRef().append(lQueryPayments);

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
    lExcelUniHandler.SetGroupShowLevel(2);
    lExcelUniHandler.Process();
}

typedef struct {
    const QSqlQuery *YearSqlQuery;
} YearSqlQueryDATA, *PYearSqlQueryDATA;

void GetYearCallback(const QSqlQuery *aQuery, const QString &aFieldName, int aRecordNum, QVariant &aValue, void *aDataPtr) {
    if (aFieldName.toLower() == "cyear") {
        aValue = ((PYearSqlQueryDATA) aDataPtr)->YearSqlQuery->value("YEAR");
    }
}

void ContractPkz::ReportProjPayByYears(bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lTemplate, lOutput, lTimeStamp;

    lTemplate = QCoreApplication::applicationDirPath();

    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lOutput = lTemplate;
    lTemplate += ReportProjByYearTemplate;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lOutput += "/temp/data/ReportProjectByYear-" + lTimeStamp;
    QDir lDir(lOutput);
    if (!lDir.exists() && !lDir.mkpath(lOutput)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " + lOutput);
        return;
    }

    lOutput += "/ReportProjectByYear-" + lTimeStamp;

    QList <QQueryForExcelData *> lQueryList;
    QQueryForExcelData *lQueryProject, *lQueryYears, *lQueryContracts, *lQueryTemp;
    QQueryForExcelDataField *lQueryForExcelDataField;
    QList <QQueryForExcelDataField *> lFields;

    MakeQueryProjects(aWithFeaturedPay, lQueryProject, aProjects, "", 1, 0, true, false);

    lQueryList.append(lQueryProject);


    // --------------------------------------------------------------------------------------------------------
    // rest by contracts
    lFields.clear();
    lFields.append(new QQueryForExcelDataField("REST_CURTOMER"));
    lFields.append(new QQueryForExcelDataField("REST_NUM"));
    lFields.append(new QQueryForExcelDataField("REST_START_DATE"));
    lFields.append(new QQueryForExcelDataField("REST_END_DATE"));
    lFields.append(new QQueryForExcelDataField("REST_NAME"));

    lFields.append(new QQueryForExcelDataField("REST_SUM_FULL", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("REST_SUM", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("REST_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));

    lFields.append(new QQueryForExcelDataField("REST_FULL", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("REST_THIS_YEAR", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("REST_FUTURE", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));

    if (!aWithFeaturedPay) {
        lQueryTemp = new QQueryForExcelData("SELECT A.ID, A.CUSTOMER REST_CURTOMER, A.NUM REST_NUM,"
                " TO_CHAR(A.START_DATE, 'DD.MM.YYYY') REST_START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') REST_END_DATE, A.NAME REST_NAME,"
                " B.SUM_BRUTTO REST_SUM, B.SUM_FULL REST_SUM_FULL,"
                " 0 REST_THIS_YEAR,"

                " B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST_FULL"

                " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                " WHERE A.ID_PROJECT = :ID_PROJECT"
                " AND B.ID_CONTRACT = A.ID"
                " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                " AND B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                " ORDER BY A.NUM",
            lFields);
    } else {
        lQueryTemp = new QQueryForExcelData("SELECT A.ID, A.CUSTOMER REST_CURTOMER, A.NUM REST_NUM,"
                " TO_CHAR(A.START_DATE, 'DD.MM.YYYY') REST_START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') REST_END_DATE, A.NAME REST_NAME,"
                " B.SUM_BRUTTO REST_SUM, B.SUM_FULL REST_SUM_FULL,"
                " (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                "   WHERE TO_CHAR(EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                "     AND ID_CONTRACT = A.ID AND PAY_SUM_BRUTTO IS NULL)"
                "  + (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "      WHERE TO_CHAR(B.EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                "        AND C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID AND B.PAY_SUM_BRUTTO IS NULL) REST_THIS_YEAR,"

                " B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST_FULL"

                " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                " WHERE A.ID_PROJECT = :ID_PROJECT"
                " AND B.ID_CONTRACT = A.ID"
                " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                " AND B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                " ORDER BY A.NUM",
        lFields);
    }
    lQueryTemp->SetRowsBeforeMarker("REST_DUMMY_BEFORE");
    lQueryTemp->SetRowsAfterMarker("REST_DUMMY_AFTER");
    lQueryTemp->BindsRef().append(new QQueryForExcelDataBind("ID_PROJECT"));
    lQueryProject->QueryListRef().append(lQueryTemp);

    // --------------------------------------------------------------------------------------------------------
    // years from payments; reverse order
    MakeQueryYears(aWithFeaturedPay, lQueryYears, " AND C.ID_PROJECT = :ID_PROJECT");
    lQueryYears->BindsRef().append(new QQueryForExcelDataBind("ID_PROJECT"));
    lQueryProject->QueryListRef().append(lQueryYears);

    MakeQueryContracts(aWithFeaturedPay, lQueryContracts, QString() +
                       " AND (EXISTS"
                       " (SELECT 1"
                       " FROM V_PKZ_HASHBON C"
                       " WHERE C.ID_CONTRACT = A.ID"
                       " AND TO_NUMBER(TO_CHAR(" + (aWithFeaturedPay?"NVL(C.PAY_DATE, C.EXPECT_DATE)":"C.PAY_DATE") + ", 'YYYY'))" + " = :YEAR)"
                       " OR EXISTS"
                       " (SELECT 1"
                       " FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                       " WHERE C.ID_CONTRACT_STAGE = D.ID"
                       " AND D.ID_PKZ_CONTRACT = A.ID"
                       " AND TO_NUMBER(TO_CHAR(" + (aWithFeaturedPay?"NVL(C.PAY_DATE, C.EXPECT_DATE)":"C.PAY_DATE") + ", 'YYYY'))" + " = :YEAR))", 0);
    lQueryContracts->BindsRef().append(new QQueryForExcelDataBind("YEAR"));

    // and select CYEAR in this query - from previous query
    YearSqlQueryDATA lYearSqlQueryDATA;
    lYearSqlQueryDATA.YearSqlQuery = lQueryYears->QueryConst();
    lQueryForExcelDataField = new QQueryForExcelDataField("CYEAR");
    lQueryForExcelDataField->SetGetValueCallback(&GetYearCallback);
    lQueryForExcelDataField->SetGetValueCallbackDataPtr(&lYearSqlQueryDATA);
    lQueryContracts->FieldsRef().append(lQueryForExcelDataField);
    lQueryYears->QueryListRef().append(lQueryContracts);

    MakeQueryPayments(aWithFeaturedPay, lQueryTemp, QString() +
                      " AND TO_NUMBER(TO_CHAR(" + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + ", 'YYYY'))" + " = :CYEAR",
                      true);
    lQueryTemp->BindsRef().append(new QQueryForExcelDataBind("CYEAR"));
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
    lExcelUniHandler.SetGroupShowLevel(2);
    lExcelUniHandler.Process();
}

void ContractPkz::MonthlyPayments(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lTemplate, lOutput, lTimeStamp;

    lTemplate = QCoreApplication::applicationDirPath();

    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lOutput = lTemplate;
    lTemplate += ReportMonthlyPaymentsTemplate;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lOutput += "/temp/data/ReportMonthlyPayments-" + lTimeStamp;
    QDir lDir(lOutput);
    if (!lDir.exists() && !lDir.mkpath(lOutput)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " + lOutput);
        return;
    }

    lOutput += "/ReportMonthlyPayments-" + lTimeStamp;

    QStringList lProjectIds;
    QString lProjectsStr;

    if (!aProjects.isEmpty()) {
        for (int i = 0; i < aProjects.length(); i++) {
            lProjectIds.append(QString::number(aProjects.at(i)));
        }
        lProjectsStr = " AND B.ID_PROJECT IN (" + lProjectIds.join(',') + ")";
    }

    QList <QQueryForExcelData *> lQueryList;
    QQueryForExcelData *lQueryYears, *lQueryMonthes, *lQueryPayments;
    QList <QQueryForExcelDataField *> lFields;
    QQueryForExcelDataField *lQueryForExcelDataField;


    GetMonthNameDATA lMonthData;

    lMonthData.Year = aYear;
    lMonthData.MNForName = 13;
    lMonthData.MNForNum = 13;

    MakeQuerySingleYear(aWithFeaturedPay, lQueryYears, &lMonthData);

    lQueryList.append(lQueryYears);

    // --------------------------------------------------------------------------------------------------------
    // monthes
    lFields.clear();
    lQueryForExcelDataField = new QQueryForExcelDataField("MONTH");
    lQueryForExcelDataField->SetGetValueCallback(&GetMonthNameNumCallback);
    lQueryForExcelDataField->SetGetValueCallbackDataPtr(&lMonthData);
    lFields.append(lQueryForExcelDataField);
    lQueryForExcelDataField = new QQueryForExcelDataField("MONTH_NUM");
    lQueryForExcelDataField->SetGetValueCallback(&GetMonthNameNumCallback);
    lQueryForExcelDataField->SetGetValueCallbackDataPtr(&lMonthData);
    lFields.append(lQueryForExcelDataField);

    lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM_FULL", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM_NDS", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("MONTH_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    lQueryMonthes = new QQueryForExcelData("", lFields);
    lQueryMonthes->SetRecordCount(12); // month count
    lQueryMonthes->SetDoGrouping(true);
    lQueryMonthes->SetDoDeleteIfNoChildren(true); // it must be here
    lQueryYears->QueryListRef().append(lQueryMonthes);

    // --------------------------------------------------------------------------------------------------------
    // payments - complicated, with contract and project data
    lFields.clear();

    //lFields.append(new QQueryForExcelDataField("PAY_DATE_DAY"));
    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_DATE_DAY");
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    lFields.append(new QQueryForExcelDataField("INVOICE"));
    lFields.append(new QQueryForExcelDataField("PAY_INVOICE"));

    lFields.append(new QQueryForExcelDataField("PROJ_NAME"));
    lFields.append(new QQueryForExcelDataField("NUM"));
    lFields.append(new QQueryForExcelDataField("START_DATE"));
    lFields.append(new QQueryForExcelDataField("END_DATE"));
    lFields.append(new QQueryForExcelDataField("NAME"));

    lFields.append(new QQueryForExcelDataField("SUM_FULL", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("SUM", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));

    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_SUM_FULL", QQueryForExcelDataField::FormatSum);
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_SUM", QQueryForExcelDataField::FormatSum);
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum);
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_SUM_FULL_IDX", QQueryForExcelDataField::FormatSum);
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_SUM_IDX", QQueryForExcelDataField::FormatSum);
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum);
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    if (!aWithFeaturedPay) {
        lQueryPayments = new QQueryForExcelData(
                "SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD') PAY_DATE_DAY,"
                " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                " B.ID_PROJECT ID_PROJECT,"
                " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                " A.PAY_SUM_BRUTTO PAY_SUM, A.PAY_SUM_FULL PAY_SUM_FULL,"
                " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_IDX, NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_IDX"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                " WHERE TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = :YEAR"
                " AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'MM')) = :MONTH_NUM"
                " AND A.ID_CONTRACT = B.ID"
                " AND C.ID_CONTRACT = B.ID"
                + lProjectsStr +
                " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                " UNION"
                " SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD') PAY_DATE_DAY,"
                " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                " B.ID_PROJECT ID_PROJECT,"
                " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                " A.PAY_SUM_BRUTTO PAY_SUM, A.PAY_SUM_FULL PAY_SUM_FULL,"
                " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_IDX, NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_IDX"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C, V_PKZ_CONTRACT_STAGE D"
                " WHERE TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = :YEAR"
                " AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'MM')) = :MONTH_NUM"
                " AND D.ID_PKZ_CONTRACT = B.ID"
                " AND A.ID_CONTRACT_STAGE = D.ID"
                " AND C.ID_CONTRACT = B.ID"
                + lProjectsStr +
                " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                " ORDER BY 1 DESC, 3 DESC, 4 DESC",
            lFields);
    } else {
        lQueryPayments = new QQueryForExcelData(
                "SELECT A.PAY_DATE PAY_DATE, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD') PAY_DATE_DAY,"
                " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                " B.ID_PROJECT ID_PROJECT,"
                " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM,"
                " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_IDX,"
                " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_IDX"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                " WHERE TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = :YEAR"
                " AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'MM')) = :MONTH_NUM"
                " AND A.ID_CONTRACT = B.ID"
                " AND C.ID_CONTRACT = B.ID"
                + lProjectsStr +
                " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                " UNION"
                " SELECT NVL(A.PAY_DATE, A.EXPECT_DATE) PAY_DATE, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD') PAY_DATE_DAY,"
                " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                " B.ID_PROJECT ID_PROJECT,"
                " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM,"
                " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_IDX,"
                " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_IDX"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C, V_PKZ_CONTRACT_STAGE D"
                " WHERE TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = :YEAR"
                " AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'MM')) = :MONTH_NUM"
                " AND D.ID_PKZ_CONTRACT = B.ID"
                " AND A.ID_CONTRACT_STAGE = D.ID"
                " AND C.ID_CONTRACT = B.ID"
                + lProjectsStr +
                " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                " ORDER BY 1 DESC, 3 DESC, 4 DESC",
            lFields);
    }
    lQueryPayments->BindsRef().append(new QQueryForExcelDataBind("YEAR", aYear));
    lQueryPayments->BindsRef().append(new QQueryForExcelDataBind("MONTH_NUM"));
    lQueryMonthes->QueryListRef().append(lQueryPayments);

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
    lExcelUniHandler.SetGroupShowLevel(2);
    lExcelUniHandler.Process();
}

void ContractPkz::ReportPayByCusts(bool aWithFeaturedPay, QStringList aCustomers) {
    QString lTemplate, lOutput, lTimeStamp;

    lTemplate = QCoreApplication::applicationDirPath();

    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lTemplate.resize(lTemplate.lastIndexOf(QChar('/')));
    lOutput = lTemplate;
    lTemplate += ReportByCustomerTemplate;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lOutput += "/temp/data/ReportByCustomer-" + lTimeStamp;
    QDir lDir(lOutput);
    if (!lDir.exists() && !lDir.mkpath(lOutput)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " + lOutput);
        return;
    }

    lOutput += "/ReportByCustomer-" + lTimeStamp;

    QList <QQueryForExcelData *> lQueryList;
    QQueryForExcelData *lQueryProjects, *lQueryContracts, *lQueryYears, *lQueryTemp;
    QList <QQueryForExcelDataField *> lFields;
    QList <int> lProjectsDummy;

    QString lCustomersStrProject, lCustomersStrContract;
    if (!aCustomers.isEmpty()) {
        for (int i = 0; i < aCustomers.length(); i++) {
            aCustomers[i].replace("'", "''");
        }
        lCustomersStrContract = "('" + aCustomers.join("','") + "')";
        lCustomersStrProject = lCustomersStrContract;
        lCustomersStrContract.prepend(" AND A.CUSTOMER IN ");
    }

    MakeQueryProjects(aWithFeaturedPay, lQueryProjects, lProjectsDummy, "", 0, 0, true, true);
    lQueryList.append(lQueryProjects);

    MakeQueryContracts(aWithFeaturedPay, lQueryContracts, lCustomersStrContract, 0);
    lQueryProjects->QueryListRef().append(lQueryContracts);

    // years from payments; reverse order
    lFields.clear();
    lFields.append(new QQueryForExcelDataField("YEAR"));

    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_FULL", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    lQueryYears = new QQueryForExcelData(QString() +
                "SELECT DISTINCT TO_NUMBER(TO_CHAR(" + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + ", 'YYYY')) YEAR, :ID_CONTRACT ID_CONTRACT"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT C"
                " WHERE A.ID_CONTRACT = C.ID"
                " AND C.ID = :ID_CONTRACT"
                " AND " + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + " IS NOT NULL"
                " UNION"
                " SELECT DISTINCT TO_NUMBER(TO_CHAR(" + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + ", 'YYYY')) YEAR, :ID_CONTRACT ID_CONTRACT"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B, V_PKZ_CONTRACT C"
                " WHERE A.ID_CONTRACT_STAGE = B.ID"
                " AND B.ID_PKZ_CONTRACT = C.ID"
                " AND C.ID = :ID_CONTRACT"
                " AND " + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + " IS NOT NULL"
                " ORDER BY 1 DESC",
        lFields);

    lQueryYears->SetDoGrouping(true);
    //lQueryYears->SetRowsAfterMarker("YEAR_DUMMY_AFTER");
    lQueryYears->BindsRef().append(new QQueryForExcelDataBind("ID_CONTRACT"));
    lQueryContracts->QueryListRef().append(lQueryYears);

    MakeQueryPayments(aWithFeaturedPay, lQueryTemp, QString(" AND TO_NUMBER(TO_CHAR(") + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + ", 'YYYY')) = :YEAR", false);
    lQueryTemp->BindsRef().append(new QQueryForExcelDataBind("YEAR"));
    lQueryYears->QueryListRef().append(lQueryTemp);

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
    lExcelUniHandler.Process();
}
