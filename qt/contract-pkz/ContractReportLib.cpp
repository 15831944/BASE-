#include "../VProject/XMLProcess.h"
#include "ContractReportLib.h"
#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

static void GetYearNumCallback(const QSqlQuery *aQuery, const QString &aFieldName, int aRecordNum, QVariant &aValue, void *aDataPtr) {
    aValue = ((PGetMonthNameDATA) aDataPtr)->Year;
}

void GetMonthNameNumCallback(const QSqlQuery *aQuery, const QString &aFieldName, int aRecordNum, QVariant &aValue, void *aDataPtr) {
    //PGetMonthNameDATA pData = (PGetMonthNameDATA) aDataPtr;
    // 12 month, start from end
    if (aFieldName.toLower() == "month") {
        aValue = gSettings->MonthName(13 - aRecordNum);
    } else if (aFieldName.toLower() == "month_num") {
        aValue = 13 - aRecordNum;
    }
}

void ColorToRedExceptPayment(const QSqlQuery *aQuery, const QString &aFieldName, QString &aNewStyleId) {
    if (aQuery->value("PAY_DATE").isNull()) {
        aNewStyleId = "MotherFuckingStyle1";
    }
}

static void AddTotalFields(QList <QQueryForExcelDataField *> &aFields) {
    aFields.append(new QQueryForExcelDataField("TOTAL_SUM", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_SUM_NDS", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_SUM_FULL", QQueryForExcelDataField::TypeTotal));

    aFields.append(new QQueryForExcelDataField("TOTAL_ORIG_SUM", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_ORIG_SUM_NDS", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_ORIG_SUM_FULL", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_ORIG_SUM_IDX", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_ORIG_SUM_NDS_IDX", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_ORIG_SUM_FULL_IDX", QQueryForExcelDataField::TypeTotal));

    aFields.append(new QQueryForExcelDataField("TOTAL_SIGN_SUM", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_SIGN_SUM_NDS", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_SIGN_SUM_FULL", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_SIGN_SUM_IDX", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_SIGN_SUM_NDS_IDX", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_SIGN_SUM_FULL_IDX", QQueryForExcelDataField::TypeTotal));

    aFields.append(new QQueryForExcelDataField("TOTAL_PAY_SUM", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_PAY_SUM_NDS", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_PAY_SUM_FULL", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_PAY_SUM_IDX", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_PAY_SUM_FULL_IDX", QQueryForExcelDataField::TypeTotal));

    aFields.append(new QQueryForExcelDataField("TOTAL_REST_THIS_YEAR", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_REST_FUTURE", QQueryForExcelDataField::TypeTotal));
    aFields.append(new QQueryForExcelDataField("TOTAL_REST_FULL", QQueryForExcelDataField::TypeTotal));
}

void MakeQuerySingleYear(bool aWithFeaturedPay, QQueryForExcelData *&aQueryYears, PGetMonthNameDATA aMonthData) {
    QList <QQueryForExcelDataField *> lFields;
    QQueryForExcelDataField *lQueryForExcelDataField;

    lQueryForExcelDataField = new QQueryForExcelDataField("YEAR");
    lQueryForExcelDataField->SetGetValueCallback(&GetYearNumCallback);
    lQueryForExcelDataField->SetGetValueCallbackDataPtr(aMonthData);
    lFields.append(lQueryForExcelDataField);

    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_FULL", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    aQueryYears = new QQueryForExcelData("", lFields);
    aQueryYears->SetDoGrouping(true);
    //aQueryYears->SetDoDeleteIfNoChildren(true);
}

void MakeQueryYears(bool aWithFeaturedPay, QQueryForExcelData *&aQueryYears, const QString &aAndWhere) {
    // years from payments; reverse order
    QList <QQueryForExcelDataField *> lFields;

    lFields.append(new QQueryForExcelDataField("YEAR"));

    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_FULL", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("YEAR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, 0, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    aQueryYears = new QQueryForExcelData(QString() +
                "SELECT DISTINCT TO_NUMBER(TO_CHAR(" + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + ", 'YYYY')) YEAR, :ID_PROJECT ID_PROJECT"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT C"
                " WHERE A.ID_CONTRACT = C.ID"
                + aAndWhere +
                " UNION"
                " SELECT DISTINCT TO_NUMBER(TO_CHAR(" + (aWithFeaturedPay?"NVL(A.PAY_DATE, A.EXPECT_DATE)":"A.PAY_DATE") + ", 'YYYY')) YEAR, :ID_PROJECT ID_PROJECT"
                " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B, V_PKZ_CONTRACT C"
                " WHERE A.ID_CONTRACT_STAGE = B.ID"
                " AND B.ID_PKZ_CONTRACT = C.ID"
                + aAndWhere +
                " ORDER BY 1 DESC",
        lFields);

    aQueryYears->SetDoGrouping(true);
    //lQueryYears->SetDoDeleteIfNoChildren(true);
    //lQueryYears->SetRowsAfterMarker("YEAR_DUMMY_AFTER");
}

void MakeQueryProjects(bool aWithFeaturedPay, QQueryForExcelData *&aQueryProjects, const QList<int> &aProjects,
                       const QString &aAndWhere, int aSumQueryNum, int aRestQueryNum, bool aWithTotal, bool aContractSumAsChildSum) {
    QStringList lProjectIds;
    QString lProjectsStr;

    if (!aProjects.isEmpty()) {
        for (int i = 0; i < aProjects.length(); i++) {
            lProjectIds.append(QString::number(aProjects.at(i)));
        }
        lProjectsStr = " AND A.ID IN (" + lProjectIds.join(',') + ")";
    }


    QList <QQueryForExcelDataField *> lFields;

    lFields.append(new QQueryForExcelDataField("PROJ_NAME"));

    if (aContractSumAsChildSum) {
        lFields.append(new QQueryForExcelDataField("PROJ_SUM_FULL", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
        lFields.append(new QQueryForExcelDataField("PROJ_SUM", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
        //lFields.append(new QQueryForExcelDataField("PROJ_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
        lFields.append(new QQueryForExcelDataField("PROJ_SUM_NDS", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    } else {
        lFields.append(new QQueryForExcelDataField("PROJ_SUM_FULL", QQueryForExcelDataField::FormatSum));
        lFields.append(new QQueryForExcelDataField("PROJ_SUM", QQueryForExcelDataField::FormatSum));
        lFields.append(new QQueryForExcelDataField("PROJ_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
    }

    lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM_FULL", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    //lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM_NDS", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    //lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_ORIG_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));

    lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM_FULL", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    //lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM_NDS", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    //lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_SIGN_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));

    lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM_FULL", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    //lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM_NDS", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));
    //lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum));

    lFields.append(new QQueryForExcelDataField("PROJ_REST_THIS_YEAR", QQueryForExcelDataField::TypeChildSum, aRestQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_REST_FUTURE", QQueryForExcelDataField::TypeChildSum, aRestQueryNum, -1, QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("PROJ_REST_FULL", QQueryForExcelDataField::TypeChildSum, aRestQueryNum, -1, QQueryForExcelDataField::FormatSum));

    if (aWithTotal) {
        AddTotalFields(lFields);
    }

    // no difference in first query for aWithFeaturedPay
    aQueryProjects = new QQueryForExcelData(
            "SELECT A.ID ID_PROJECT, PP.GETPROJECTSHORTNAME(A.ID) PROJ_NAME,"
            " (SELECT SUM(C.SUM_BRUTTO) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
            "   WHERE B.ID_PROJECT = A.ID"
            "   AND C.ID_CONTRACT = B.ID"
            "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) PROJ_SUM,"
            " (SELECT SUM(C.SUM_FULL) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
            "   WHERE B.ID_PROJECT = A.ID"
            "   AND C.ID_CONTRACT = B.ID"
            "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) PROJ_SUM_FULL"
            " FROM V_PROJECT A WHERE EXISTS (SELECT 1 FROM V_PKZ_CONTRACT WHERE ID_PROJECT = A.ID)"
            + lProjectsStr
            + aAndWhere
            + " ORDER BY PP.NUMSORT(PP.GETPROJECTSHORTNAME(A.ID))",
        lFields);
    if (aWithTotal) {
        aQueryProjects->SetRowsAfterMarker("TOTAL");
    }
    aQueryProjects->SetDoGrouping(true);
    aQueryProjects->SetDoDeleteIfNoChildren(true);
    aQueryProjects->SetSkipAfterIfOneRecord(true);
}

void MakeQueryRest(bool aWithFeaturedPay, QQueryForExcelData *&aQueryRests, const QString &aAndWhere) {
    QList <QQueryForExcelDataField *> lFields;

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
        aQueryRests = new QQueryForExcelData("SELECT A.ID, A.CUSTOMER REST_CURTOMER, A.NUM REST_NUM,"
                " TO_CHAR(A.START_DATE, 'DD.MM.YYYY') REST_START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') REST_END_DATE, A.NAME REST_NAME,"
                " B.SUM_BRUTTO REST_SUM, B.SUM_FULL REST_SUM_FULL,"
                " 0 REST_THIS_YEAR,"

                " B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST_FULL"

                " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                " WHERE B.ID_CONTRACT = A.ID"
                + aAndWhere +
                " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                " AND B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                " ORDER BY A.NUM",
            lFields);
    } else {
        aQueryRests = new QQueryForExcelData("SELECT A.ID, A.CUSTOMER REST_CURTOMER, A.NUM REST_NUM,"
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
                " WHERE B.ID_CONTRACT = A.ID"
                + aAndWhere +
                " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                " AND B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                " ORDER BY A.NUM",
        lFields);
    }
    aQueryRests->SetRowsBeforeMarker("REST_DUMMY_BEFORE");
    aQueryRests->SetRowsAfterMarker("REST_DUMMY_AFTER");
}

void MakeQueryContracts(bool aWithFeaturedPay, QQueryForExcelData *&aQueryContracts, const QString &aAndWhere, int aSumQueryNum) {
    QList <QQueryForExcelDataField *> lFields;
    lFields.append(new QQueryForExcelDataField("CONTR_NUM"));
    lFields.append(new QQueryForExcelDataField("CONTR_START_DATE"));
    lFields.append(new QQueryForExcelDataField("CONTR_END_DATE"));
    lFields.append(new QQueryForExcelDataField("CONTR_NAME"));

    lFields.append(new QQueryForExcelDataField("CONTR_SUM_FULL", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("CONTR_SUM", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("CONTR_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));

    lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM_FULL", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM_NDS", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("CONTR_ORIG_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM_FULL", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM_NDS", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("CONTR_SIGN_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM_FULL", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM_NDS", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM_FULL_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    //lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0))));
    lFields.append(new QQueryForExcelDataField("CONTR_PAY_SUM_NDS_IDX", QQueryForExcelDataField::TypeChildSum, aSumQueryNum, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));


    lFields.append(new QQueryForExcelDataField("CONTR_REST_FULL", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("CONTR_REST_THIS_YEAR", QQueryForExcelDataField::FormatSum));
    lFields.append(new QQueryForExcelDataField("CONTR_REST_FUTURE", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum));

    // no difference in this query for aWithFeaturedPay
    aQueryContracts = new QQueryForExcelData(QString() +
            "SELECT A.ID ID_CONTRACT, A.NUM CONTR_NUM,"
            " TO_CHAR(A.START_DATE, 'DD.MM.YYYY') CONTR_START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') CONTR_END_DATE,"
            " A.NAME CONTR_NAME, B.SUM_BRUTTO CONTR_SUM, B.SUM_FULL CONTR_SUM_FULL,"
            + (aWithFeaturedPay?" (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                                "   WHERE TO_CHAR(EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                                "     AND ID_CONTRACT = A.ID AND PAY_SUM_BRUTTO IS NULL)"
                                "  + (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                "      WHERE TO_CHAR(B.EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                                "        AND C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID AND B.PAY_SUM_BRUTTO IS NULL)":" 0") + " CONTR_REST_THIS_YEAR," +

            " B.SUM_BRUTTO -"
            "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
            "        WHERE ID_CONTRACT = A.ID)"
            "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
            "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) CONTR_REST_FULL"

            " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
            " WHERE A.ID_PROJECT = :ID_PROJECT"
            " AND B.ID_CONTRACT = A.ID"
            + aAndWhere +
            " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
            " ORDER BY A.NUM",
        lFields);
    aQueryContracts->BindsRef().append(new QQueryForExcelDataBind("ID_PROJECT"));
    aQueryContracts->SetRowsAfterMarker("DUMMY_AFTER_CONTRACTS");
    aQueryContracts->SetDoGrouping(true);
}

void MakeQueryPayments(bool aWithFeaturedPay, QQueryForExcelData *&aQueryPayments, const QString &aAndWhere, bool aSortDesc) {
    QQueryForExcelDataField *lQueryForExcelDataField;
    QList <QQueryForExcelDataField *> lFields;

    //lFields.append(new QQueryForExcelDataField("PAY_DATE_STR"));
    lQueryForExcelDataField = new QQueryForExcelDataField("PAY_DATE_STR");
    lQueryForExcelDataField->SetChangeStyleIdCallback(&ColorToRedExceptPayment);
    lFields.append(lQueryForExcelDataField);

    lFields.append(new QQueryForExcelDataField("INVOICE"));
    lFields.append(new QQueryForExcelDataField("PAY_INVOICE"));

    lFields.append(new QQueryForExcelDataField("ORIG_SUM_FULL", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("ORIG_SUM", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("ORIG_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("ORIG_SUM_FULL_IDX", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("ORIG_SUM_IDX", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("ORIG_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    lFields.append(new QQueryForExcelDataField("SIGN_SUM_FULL", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("SIGN_SUM", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("SIGN_SUM_NDS", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("SIGN_SUM_FULL_IDX", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("SIGN_SUM_IDX", QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));
    lFields.append(new QQueryForExcelDataField("SIGN_SUM_NDS_IDX", QQueryForExcelDataField::TypeOperLLMinus, -1, -1, QQueryForExcelDataField::FormatSum, QVariant((qulonglong) 0)));

    // ---------------------------------------------------------------------------------------------
    // still payments
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
        aQueryPayments = new QQueryForExcelData(
            "SELECT PAY_DATE, TO_CHAR(PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
            " ORIG_SUM_BRUTTO ORIG_SUM, ORIG_SUM_FULL,"
            " NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO) ORIG_SUM_IDX, NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL) ORIG_SUM_FULL_IDX,"

            " SIGN_SUM_BRUTTO SIGN_SUM, SIGN_SUM_FULL,"
            " NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO) SIGN_SUM_IDX, NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL) SIGN_SUM_FULL_IDX,"

            " PAY_SUM_BRUTTO PAY_SUM, PAY_SUM_FULL,"
            " NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO) PAY_SUM_IDX,"
            " NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL) PAY_SUM_FULL_IDX"
            " FROM V_PKZ_HASHBON A"
            " WHERE ID_CONTRACT = :ID_CONTRACT"
            + aAndWhere +

            " UNION"
            " SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
            " A.ORIG_SUM_BRUTTO ORIG_SUM, A.ORIG_SUM_FULL,"
            " A.ORIG_SUM_BRUTTO_INDEXED ORIG_SUM_IDX, A.ORIG_SUM_FULL_INDEXED ORIG_SUM_FULL_IDX,"

            " A.SIGN_SUM_BRUTTO SIGN_SUM, A.SIGN_SUM_FULL,"
            " A.SIGN_SUM_BRUTTO_INDEXED SIGN_SUM_IDX, A.SIGN_SUM_FULL_INDEXED SIGN_SUM_FULL_IDX,"

            " A.PAY_SUM_BRUTTO PAY_SUM, A.PAY_SUM_FULL,"
            " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_IDX,"
            " NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_IDX"
            " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
            " WHERE B.ID_PKZ_CONTRACT = :ID_CONTRACT"
            " AND A.ID_CONTRACT_STAGE = B.ID"
            + aAndWhere +
            " ORDER BY 1" +(aSortDesc?" DESC":""),
        lFields);
    } else {
        aQueryPayments = new QQueryForExcelData(
            "SELECT PAY_DATE, TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
            " ORIG_SUM_BRUTTO ORIG_SUM, ORIG_SUM_FULL,"
            " NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO) ORIG_SUM_IDX, NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL) ORIG_SUM_FULL_IDX,"

            " SIGN_SUM_BRUTTO SIGN_SUM, SIGN_SUM_FULL,"
            " NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO) SIGN_SUM_IDX, NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL) SIGN_SUM_FULL_IDX,"

            " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), PAY_SUM_BRUTTO) PAY_SUM,"
            " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_FULL, ORIG_SUM_FULL), PAY_SUM_FULL) PAY_SUM_FULL,"
            " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO), NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO)), NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO)) PAY_SUM_IDX,"
            " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL), NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL)), NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL)) PAY_SUM_FULL_IDX"
            " FROM V_PKZ_HASHBON A"
            " WHERE ID_CONTRACT = :ID_CONTRACT"
            + aAndWhere +

            " UNION"
            " SELECT PAY_DATE, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
            " A.ORIG_SUM_BRUTTO ORIG_SUM, A.ORIG_SUM_FULL,"
            " A.ORIG_SUM_BRUTTO_INDEXED ORIG_SUM_IDX, A.ORIG_SUM_FULL_INDEXED ORIG_SUM_FULL_IDX,"

            " A.SIGN_SUM_BRUTTO SIGN_SUM, A.SIGN_SUM_FULL,"
            " A.SIGN_SUM_BRUTTO_INDEXED SIGN_SUM_IDX, A.SIGN_SUM_FULL_INDEXED SIGN_SUM_FULL_IDX,"

            " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM,"
            " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
            " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_IDX,"
            " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_IDX"
            " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
            " WHERE B.ID_PKZ_CONTRACT = :ID_CONTRACT"
            " AND A.ID_CONTRACT_STAGE = B.ID"
            + aAndWhere +
            " ORDER BY 1" +(aSortDesc?" DESC":""),
        lFields);
    }
    aQueryPayments->BindsRef().append(new QQueryForExcelDataBind("ID_CONTRACT"));
    aQueryPayments->SetRowsAfterMarker("DUMMY_AFTER_PAYMENTS");
}
