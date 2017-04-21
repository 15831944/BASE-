#include "ContractPkz.h"
#include "ui_contractpkz.h"

#include "contract-pkz_local.h"

#include "../Login/Login.h"
#include "../VProject/GlobalSettings.h"
#include "../Logger/logger.h"

#include <QDir>

#include <QMessageBox>
#include <QCoreApplication>

#define ReportByCustomer "/common/templates/contract-rep-ByCustomer.xls"

void ContractPkz::ReportPayByCustsOld(bool aWithFeaturedPay, const QStringList & aCustomers) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int i, j, k, lCurRow;
    qlonglong color;

    //---------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // prepare queries

    QSqlQuery queryProject(db), queryDebtByContract(db), queryProjectDebt(db), queryContract(db), queryHashbon(db);

    // project list for customer
    queryProject.prepare("select distinct a.id_project, pp.GetProjectShortName(a.id_project) ProjName"
                         " from v_pkz_contract a where nvl(a.customer, '--NULL-') = nvl(:customer, '--NULL-')"
                         " order by 2");

    if (queryProject.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryProject);
        return;
    }

    // project debt, min and max years numbers, SUMS for project
    if (!aWithFeaturedPay) {
        queryProjectDebt.prepare("SELECT PP.GETPROJECTSHORTNAME(A.ID) PROJNAME,"
                                 " LEAST("
                                 "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer1, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND PAY_DATE IS NOT NULL), 5000),"
                                 "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer2, '--NULL-')"
                                 "   AND C.ID_PKZ_CONTRACT = B.ID"
                                 "   AND D.ID_CONTRACT_STAGE = C.ID"
                                 "   AND PAY_DATE IS NOT NULL), 5000)) MIN1,"
                                 " GREATEST("
                                 "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer3, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND PAY_DATE IS NOT NULL), 0),"
                                 "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer4, '--NULL-')"
                                 "   AND C.ID_PKZ_CONTRACT = B.ID"
                                 "   AND D.ID_CONTRACT_STAGE = C.ID"
                                 "   AND PAY_DATE IS NOT NULL), 0)) MAX1,"
                                 " (SELECT SUM(C.SUM_BRUTTO) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer5, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM,"
                                 " (SELECT SUM(C.SUM_FULL) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer6, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM_FULL"
                                 " FROM V_PROJECT A WHERE A.ID = :ID_PROJECT");
    } else {
        queryProjectDebt.prepare("SELECT PP.GETPROJECTSHORTNAME(A.ID) PROJNAME,"
                                 " LEAST("
                                 "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer1, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND (PAY_DATE IS NOT NULL OR EXPECT_DATE IS NOT NULL)), 5000),"
                                 "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer2, '--NULL-')"
                                 "   AND C.ID_PKZ_CONTRACT = B.ID"
                                 "   AND D.ID_CONTRACT_STAGE = C.ID"
                                 "   AND (PAY_DATE IS NOT NULL OR EXPECT_DATE IS NOT NULL)), 5000)) MIN1,"
                                 " GREATEST("
                                 "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer3, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND (PAY_DATE IS NOT NULL OR EXPECT_DATE IS NOT NULL)), 0),"
                                 "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer4, '--NULL-')"
                                 "   AND C.ID_PKZ_CONTRACT = B.ID"
                                 "   AND D.ID_CONTRACT_STAGE = C.ID"
                                 "   AND (PAY_DATE IS NOT NULL OR EXPECT_DATE IS NOT NULL)), 0)) MAX1,"
                                 " (SELECT SUM(C.SUM_BRUTTO) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer5, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM,"
                                 " (SELECT SUM(C.SUM_FULL) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                                 "   WHERE B.ID_PROJECT = A.ID"
                                 "   AND nvl(B.CUSTOMER, '--NULL-') = nvl(:customer6, '--NULL-')"
                                 "   AND C.ID_CONTRACT = B.ID"
                                 "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM_FULL"
                                 " FROM V_PROJECT A WHERE A.ID = :ID_PROJECT");
    }


    if (queryProjectDebt.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryProjectDebt);
        return;
    }

    // debt by contracts; it is simple list; params - IdProject and Customer
    if (!aWithFeaturedPay) {
        queryDebtByContract.prepare("SELECT A.ID, A.CUSTOMER, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE, A.NAME,"
                                    " B.SUM_BRUTTO SUM, B.SUM_FULL,"
                                    "0 REST_THIS_YEAR,"

                                    " B.SUM_BRUTTO -"
                                    "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                                    "        WHERE ID_CONTRACT = A.ID)"
                                    "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                    "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST_FULL"

                                    " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                                    " WHERE A.ID_PROJECT = :id_project"
                                    " AND nvl(A.CUSTOMER, '--NULL-') = nvl(:customer, '--NULL-')"
                                    " AND B.ID_CONTRACT = A.ID"
                                    " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                                    " AND B.SUM_BRUTTO -"
                                    "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                                    "        WHERE ID_CONTRACT = A.ID)"
                                    "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                    "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                                    " ORDER BY A.NUM");
    } else {
        queryDebtByContract.prepare("SELECT A.ID, A.CUSTOMER, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE, A.NAME,"
                                    " B.SUM_BRUTTO SUM, B.SUM_FULL,"

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
                                    " WHERE A.ID_PROJECT = :id_project"
                                    " AND nvl(A.CUSTOMER, '--NULL-') = nvl(:customer, '--NULL-')"
                                    " AND B.ID_CONTRACT = A.ID"
                                    " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                                    " AND B.SUM_BRUTTO -"
                                    "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                                    "        WHERE ID_CONTRACT = A.ID)"
                                    "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                    "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                                    " ORDER BY A.NUM");
    }


    if (queryDebtByContract.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryDebtByContract);
        return;
    }

    // contracts; params - IdProject, Customer, Year
    if (!aWithFeaturedPay) {
        queryContract.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                              " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL"
                              " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                              " WHERE A.ID_PROJECT = :id_project"
                              " AND nvl(A.CUSTOMER, '--NULL-') = nvl(:customer, '--NULL-')"
                              " AND B.ID_CONTRACT = A.ID"
                              " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                              " AND (EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON"
                              "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = :year1)"
                              "   OR EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                              "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = :year2))"
                              " ORDER BY A.NUM");
    } else {
        queryContract.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                              " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL"
                              " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                              " WHERE A.ID_PROJECT = :id_project"
                              " AND nvl(A.CUSTOMER, '--NULL-') = nvl(:customer, '--NULL-')"
                              " AND B.ID_CONTRACT = A.ID"
                              " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                              " AND (EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON"
                              "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = :year1)"
                              "   OR EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                              "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = :year2))"
                              " ORDER BY A.NUM");
    }

    if (queryContract.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryContract);
        return;
    }

    // checks (hashbons); params - IdContract, Year
    if (!aWithFeaturedPay) {
        queryHashbon.prepare("SELECT PAY_DATE, TO_CHAR(PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                             " PAY_SUM_BRUTTO, PAY_SUM_FULL,"
                             " NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                             " NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON"
                             " WHERE ID_CONTRACT = :id_contract1"
                             " AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = :year1"
                             " UNION"
                             " SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                             " A.PAY_SUM_BRUTTO, A.PAY_SUM_FULL,"
                             " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                             " NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                             " WHERE B.ID_PKZ_CONTRACT = :id_contract2"
                             " AND A.ID_CONTRACT_STAGE = B.ID"
                             " AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = :year2"
                             " ORDER BY 1");
    } else {
        queryHashbon.prepare("SELECT PAY_DATE, TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                             " DECODE(PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                             " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                             " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_FULL, ORIG_SUM_FULL), PAY_SUM_FULL) PAY_SUM_FULL,"
                             " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO), NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO)), NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                             " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL), NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL)), NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON"
                             " WHERE ID_CONTRACT = :id_contract1"
                             " AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = :year1"
                             " UNION"
                             " SELECT PAY_DATE, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                             " DECODE(A.PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                             " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                             " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                             " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                             " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                             " WHERE B.ID_PKZ_CONTRACT = :id_contract2"
                             " AND A.ID_CONTRACT_STAGE = B.ID"
                             " AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = :year2"
                             " ORDER BY 1");
    }

    if (queryHashbon.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryHashbon);
        return;
    }

    // end of prepare queries
    //---------------------------------------------------------------------------------------------------------------------------------------------------------------------


    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportByCustomer-" + lTimeStamp;
    dir.setPath(lVbsName);
    if (!dir.exists() && !dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " +lVbsName);
        return;
    }

    lVbsName += "/ReportByCustomer.vbs";
    lTemplateName += ReportByCustomer;

    lOutName = "ReportByCustomer-" + lTimeStamp + ".xls";

    ReportStart();

    lCurRow = 4;

    out << "Dim nCols(16)' as Range\n";
    out << "Dim nColsAny(16)\n";

    out << "Set nCols(0) = oWsheet.Cells.Find(\"#PROJECT#\")\n";
    out << "Set nCols(1) = oWsheet.Cells.Find(\"#NUM#\")\n";
    out << "Set nCols(2) = oWsheet.Cells.Find(\"#START_DATE#\")\n";
    out << "Set nCols(3) = oWsheet.Cells.Find(\"#END_DATE#\")\n";
    out << "Set nCols(4) = oWsheet.Cells.Find(\"#NAME#\")\n";
    out << "Set nCols(5) = oWsheet.Cells.Find(\"#SUM#\")\n";
    out << "Set nCols(6) = oWsheet.Cells.Find(\"#SUM_NDS#\")\n";
    out << "Set nCols(7) = oWsheet.Cells.Find(\"#SUM_FULL#\")\n";
    out << "Set nCols(8) = oWsheet.Cells.Find(\"#PAY_SUM#\")\n";
    out << "Set nCols(9) = oWsheet.Cells.Find(\"#PAY_SUM_NDS#\")\n";
    out << "Set nCols(10) = oWsheet.Cells.Find(\"#PAY_SUM_FULL#\")\n";
    out << "Set nCols(11) = oWsheet.Cells.Find(\"#PAY_SUM_IDX#\")\n";
    out << "Set nCols(12) = oWsheet.Cells.Find(\"#PAY_SUM_NDS_IDX#\")\n";
    out << "Set nCols(13) = oWsheet.Cells.Find(\"#PAY_SUM_FULL_IDX#\")\n";
    out << "Set nCols(14) = oWsheet.Cells.Find(\"#REST_THIS_YEAR#\")\n";
    out << "Set nCols(15) = oWsheet.Cells.Find(\"#REST_FUTURE#\")\n";
    out << "Set nCols(16) = oWsheet.Cells.Find(\"#REST_FULL#\")\n";

    out << "nColLast = 0\n";
    out << "For Each nCol in nCols\n";
    out << "  If Not nCol Is Nothing Then\n";
    out << "    nCol.Value = \"\"\n";
    out << "    If nCol.Column > nColLast Then\n";
    out << "      nColLast = nCol.Column\n";
    out << "    End If\n";
    out << "  End If\n";
    out << "Next\n";

    out << "For i = 0 To UBound(nCols)\n";
    out << "  nColsAny(i) = True\n";
    out << "Next\n";

    //MsgBox UBound(nCols)

    for (i = 0; i < aCustomers.length(); i++) {
        int lRowCustomerSum;
        QList<int> forCustomerSum, forCustomerSumDebt;
        QString lCustomer = aCustomers.at(i), lCustomerForExcel;

        lCustomerForExcel = lCustomer;
        lCustomerForExcel.replace("\"", "\"\"").replace("\n", "\" & vbLf & \"");

        queryProject.bindValue(":customer", lCustomer);

        if (!queryProject.exec()) {
            gLogger->ShowSqlError(this, "חוזים", queryProject);
        } else {
            // out customer
            color = gSettings->Contract.ProjectColor.blue();
            color <<= 8;
            color += gSettings->Contract.ProjectColor.green();
            color <<= 8;
            color += gSettings->Contract.ProjectColor.red();

            out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
            out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << lCustomerForExcel << "\"\n";
            out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
            out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).Font.Bold = True\n";
            //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
            out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
            out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Interior.Color = " << QString::number(color) << "\n";

            lRowCustomerSum = lCurRow;
            BorderWithStart(5);
            lCurRow++;
            // out customer end

            while (queryProject.next()) {
                QList<int> forProjectSum;;
                int lRowProjectSum;

                queryProjectDebt.bindValue(":id_project", queryProject.value("id_project").toInt());
                queryProjectDebt.bindValue(":customer1", lCustomer);
                queryProjectDebt.bindValue(":customer2", lCustomer);
                queryProjectDebt.bindValue(":customer3", lCustomer);
                queryProjectDebt.bindValue(":customer4", lCustomer);
                queryProjectDebt.bindValue(":customer5", lCustomer);
                queryProjectDebt.bindValue(":customer6", lCustomer);
                if (!queryProjectDebt.exec()) {
                    gLogger->ShowSqlError(this, "חוזים", queryProjectDebt);
                } else {
                    if (queryProjectDebt.next()) {
                        color = gSettings->Contract.ContractColor.blue();
                        color <<= 8;
                        color += gSettings->Contract.ContractColor.green();
                        color <<= 8;
                        color += gSettings->Contract.ContractColor.red();


                        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
                        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << queryProjectDebt.value("PROJNAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
                        //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
                        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
                        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Interior.Color = " << QString::number(color) << "\n";

                        out << "If Not nCols(5) Is Nothing Then\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
                            << gSettings->FormatSumForList(queryProjectDebt.value("SUM").toLongLong()) << "\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Bold = True\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Italic = True\n";
                        out << "End If\n";

                        out << "If Not nCols(6) Is Nothing Then\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
                            << gSettings->FormatSumForList(queryProjectDebt.value("SUM_FULL").toLongLong() - queryProjectDebt.value("SUM").toLongLong()) << "\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Bold = True\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Italic = True\n";
                        out << "End If\n";

                        out << "If Not nCols(7) Is Nothing Then\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
                            << gSettings->FormatSumForList(queryProjectDebt.value("SUM_FULL").toLongLong()) << "\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).Font.Bold = True\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).Font.Italic = True\n";
                        out << "End If\n";

                        lRowProjectSum = lCurRow; // it is the row where project debt output
                        BorderWithStart(5);
                        lCurRow++;

                        // debt -----------------------------------------------------------------------------------------------------------------------------
                        queryDebtByContract.bindValue(":id_project", queryProject.value("id_project").toInt());
                        queryDebtByContract.bindValue(":customer", lCustomer);
                        if (!queryDebtByContract.exec()) {
                            gLogger->ShowSqlError(this, "חוזים", queryDebtByContract);
                        } else {
                            color = gSettings->Contract.StageColor.blue();
                            color <<= 8;
                            color += gSettings->Contract.StageColor.green();
                            color <<= 8;
                            color += gSettings->Contract.StageColor.red();
                            while (queryDebtByContract.next()) {
                                if (lCurRow == lRowProjectSum + 1) {
                                    // it is first record, show hebrew "debt" for grouping
                                    QString sDebt("שארית");
                                    out << "If Not nCols(1) Is Nothing Then\n";
                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" + sDebt + "\"\n";
                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4108\n"; // center
                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).Font.Bold = True\n";
                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).Font.Italic = True\n";
                                    out << "End If\n";

                                    out << "For i = 1 To UBound(nCols)\n";
                                    out << "  If Not nCols(i) Is Nothing Then\n";
                                    //out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 14540253\n";
                                    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = " + QString::number(color) + "\n";
                                    out << "  End If\n";
                                    out << "Next\n";

                                    Border;
                                    lCurRow++;
                                }

                                out << "If Not nCols(1) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << queryDebtByContract.value("NUM").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4152\n"; // right
                                out << "End If\n";

                                out << "If Not nCols(2) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).NumberFormat = \"@\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).FormulaR1C1 = \"" << queryDebtByContract.value("START_DATE").toString() << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).HorizontalAlignment = -4108\n"; // center
                                out << "End If\n";

                                out << "If Not nCols(3) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).NumberFormat = \"@\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).FormulaR1C1 = \"" << queryDebtByContract.value("END_DATE").toString() << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).HorizontalAlignment = -4108\n"; // center
                                out << "End If\n";

                                out << "If Not nCols(4) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << queryDebtByContract.value("NAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).WrapText = False\n";
                                out << "End If\n";

                                out << "If Not nCols(5) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
                                    << gSettings->FormatSumForList(queryDebtByContract.value("SUM").toLongLong()) << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
                                out << "End If\n";

                                out << "If Not nCols(6) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
                                    << gSettings->FormatSumForList(queryDebtByContract.value("SUM_FULL").toLongLong() - queryDebtByContract.value("SUM").toLongLong()) << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
                                out << "End If\n";

                                out << "If Not nCols(7) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
                                    << gSettings->FormatSumForList(queryDebtByContract.value("SUM_FULL").toLongLong()) << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
                                out << "End If\n";

                                out << "If Not nCols(14) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).NumberFormat = \"#,##0.00\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).FormulaR1C1 = \""
                                    << gSettings->FormatSumForList(queryDebtByContract.value("REST_THIS_YEAR").toLongLong()) << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).HorizontalAlignment = -4152\n"; // right
                                out << "End If\n";

                                out << "If Not nCols(15) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).NumberFormat = \"#,##0.00\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).FormulaR1C1 = \""
                                    << gSettings->FormatSumForList(queryDebtByContract.value("REST_FULL").toLongLong() - queryDebtByContract.value("REST_THIS_YEAR").toLongLong()) << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).HorizontalAlignment = -4152\n"; // right
                                out << "End If\n";

                                out << "If Not nCols(16) Is Nothing Then\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).NumberFormat = \"#,##0.00\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).FormulaR1C1 = \""
                                    << gSettings->FormatSumForList(queryDebtByContract.value("REST_FULL").toLongLong()) << "\"\n";
                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).HorizontalAlignment = -4152\n"; // right
                                out << "End If\n";

                                out << "For i = 1 To UBound(nCols)\n";
                                out << "  If Not nCols(i) Is Nothing Then\n";
                                out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 14540253\n";
                                out << "  End If\n";
                                out << "Next\n";

                                Border;
                                lCurRow++;
                            }

                            // summary for debt on project
                            out << "' summary for debt on project\n";
                            out << "For i = 14 To 16\n";
                            out << "  If Not nCols(i) Is Nothing Then\n";
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                            out << "    If nColsAny(i) Then\n";
                            if (lCurRow > lRowProjectSum + 1) {
                                out << "      oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Value = \"=SUM(\" + chr(nCols(i).Column + asc(\"A\") - 1) + \""
                                    << lRowProjectSum + 2 << ":\" + chr(nCols(i).Column + asc(\"A\") - 1) + \"" << lCurRow - 1 << ")\"\n";
                            } else {
                                // no sum, there was not any line for debt - debt equal to zero
                                out << "      oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).FormulaR1C1 = 0\n";
                            }
                            out << "    End If\n";
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Bold = True\n";
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Italic = True\n";
                            out << "  End If\n";
                            out << "Next\n";

                            out << "oWsheet.Rows(\"" << lRowProjectSum + 2 << ":" << lCurRow << "\").Group\n";

                            out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";

                            forCustomerSumDebt.append(lRowProjectSum);
                            lCurRow++;
                        }
                        // end of debt -----------------------------------------------------------------------------------------------------------------------------




                        if (queryProjectDebt.value("MIN1").toInt() != 5000) {
                            for (j = queryProjectDebt.value("MAX1").toInt(); j >= queryProjectDebt.value("MIN1").toInt(); j--) {
                                int lRowYearSum = lCurRow;
                                QList<int> forYearSum;;

                                queryContract.bindValue(":id_project", queryProject.value("id_project").toInt());
                                queryContract.bindValue(":customer", lCustomer);
                                queryContract.bindValue(":year1", j);
                                queryContract.bindValue(":year2", j);

                                if (!queryContract.exec()) {
                                    gLogger->ShowSqlError(this, "חוזים", queryContract);
                                } else {
                                    while (queryContract.next()) {
                                        int lRowContractSum;

                                        if (lCurRow == lRowYearSum) {
                                            out << "If Not nCols(1) Is Nothing Then\n";
                                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << QString::number(j) << "\"\n";
                                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4108\n"; // center
                                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).Font.Bold = True\n";
                                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).Font.Italic = True\n";
                                            out << "End If\n";

                                            Border;
                                            lCurRow++;
                                        }

                                        out << "If Not nCols(1) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << queryContract.value("NUM").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4152\n"; // right
                                        out << "End If\n";

                                        out << "If Not nCols(2) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).NumberFormat = \"@\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).FormulaR1C1 = \"" << queryContract.value("START_DATE").toString() << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).HorizontalAlignment = -4108\n"; // center
                                        out << "End If\n";

                                        out << "If Not nCols(3) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).NumberFormat = \"@\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).FormulaR1C1 = \"" << queryContract.value("END_DATE").toString() << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).HorizontalAlignment = -4108\n"; // center
                                        out << "End If\n";

                                        out << "If Not nCols(4) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << queryContract.value("NAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).WrapText = False\n";
                                        out << "End If\n";

                                        out << "If Not nCols(5) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
                                            << gSettings->FormatSumForList(queryContract.value("SUM").toLongLong()) << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
                                        out << "End If\n";

                                        out << "If Not nCols(6) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
                                            << gSettings->FormatSumForList(queryContract.value("SUM_FULL").toLongLong() - queryContract.value("SUM").toLongLong()) << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
                                        out << "End If\n";

                                        out << "If Not nCols(7) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
                                            << gSettings->FormatSumForList(queryContract.value("SUM_FULL").toLongLong()) << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
                                        out << "End If\n";

                                        Border;
                                        lRowContractSum = lCurRow;
                                        lCurRow++;


                                        queryHashbon.bindValue(":id_contract1", queryContract.value("ID").toInt());
                                        queryHashbon.bindValue(":year1", j);
                                        queryHashbon.bindValue(":id_contract2", queryContract.value("ID").toInt());
                                        queryHashbon.bindValue(":year2", j);
                                        if (!queryHashbon.exec()) {
                                            gLogger->ShowSqlError(this, "חוזים", queryHashbon);
                                        } else {
                                            while (queryHashbon.next()) {
                                                qlonglong lPaySum, lPaySumFull, lPaySumIdx, lPaySumFullIdx;

                                                lPaySum = queryHashbon.value("PAY_SUM_BRUTTO").toLongLong();
                                                lPaySumFull = queryHashbon.value("PAY_SUM_FULL").toLongLong();
                                                lPaySumIdx = queryHashbon.value("PAY_SUM_BRUTTO_INDEXED").toLongLong();
                                                lPaySumFullIdx = queryHashbon.value("PAY_SUM_FULL_INDEXED").toLongLong();

                                                // pay date
                                                out << "If Not nCols(4) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << queryHashbon.value("PAY_DATE_STR").toString() << "\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).HorizontalAlignment = -4108\n"; // center
                                                out << "End If\n";

                                                // invoice
                                                out << "If Not nCols(5) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"@\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \"" << queryHashbon.value("INVOICE").toString() << "\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4108\n"; // center
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Italic = True\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Size = oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Size - 1\n";
                                                out << "End If\n";

                                                // pay invoice
                                                out << "If Not nCols(6) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"@\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \"" << queryHashbon.value("PAY_INVOICE").toString() << "\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4108\n"; // center
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Italic = True\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Size = oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Size - 1\n";
                                                out << "End If\n";

                                                // payed
                                                out << "If Not nCols(8) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(8).Column).NumberFormat = \"#,##0.00\"\n";
                                                if (lPaySum) {
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(8).Column).FormulaR1C1 = \""
                                                        << gSettings->FormatSumForList(lPaySum) << "\"\n";
                                                    out << "  nColsAny(8) = True\n";
                                                } else
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(8).Column).FormulaR1C1 = \"\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(8).Column).HorizontalAlignment = -4152\n"; // right
                                                out << "End If\n";

                                                out << "If Not nCols(9) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(9).Column).NumberFormat = \"#,##0.00\"\n";
                                                if (lPaySumFull && lPaySum) {
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(9).Column).FormulaR1C1 = \""
                                                        << gSettings->FormatSumForList(lPaySumFull - lPaySum) << "\"\n";
                                                    out << "  nColsAny(9) = True\n";
                                                } else
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(9).Column).FormulaR1C1 = \"\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(9).Column).HorizontalAlignment = -4152\n"; // right
                                                out << "End If\n";

                                                out << "If Not nCols(10) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(10).Column).NumberFormat = \"#,##0.00\"\n";
                                                if (lPaySumFull) {
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(10).Column).FormulaR1C1 = \""
                                                        << gSettings->FormatSumForList(lPaySumFull) << "\"\n";
                                                    out << "  nColsAny(10) = True\n";
                                                } else
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(10).Column).FormulaR1C1 = \"\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(10).Column).HorizontalAlignment = -4152\n"; // right
                                                out << "End If\n";

                                                // payed indexed
                                                out << "If Not nCols(11) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(11).Column).NumberFormat = \"#,##0.00\"\n";
                                                if (lPaySumIdx) {
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(11).Column).FormulaR1C1 = \""
                                                        << gSettings->FormatSumForList(lPaySumIdx) << "\"\n";
                                                    out << "  nColsAny(11) = True\n";
                                                } else
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(11).Column).FormulaR1C1 = \"\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(11).Column).HorizontalAlignment = -4152\n"; // right
                                                out << "End If\n";

                                                out << "If Not nCols(12) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(12).Column).NumberFormat = \"#,##0.00\"\n";
                                                if (lPaySumFullIdx && lPaySumIdx) {
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(12).Column).FormulaR1C1 = \""
                                                        << gSettings->FormatSumForList(lPaySumFullIdx - lPaySumIdx) << "\"\n";
                                                    out << "  nColsAny(12) = True\n";
                                                } else
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(12).Column).FormulaR1C1 = \"\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(12).Column).HorizontalAlignment = -4152\n"; // right
                                                out << "End If\n";

                                                out << "If Not nCols(13) Is Nothing Then\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(13).Column).NumberFormat = \"#,##0.00\"\n";
                                                if (lPaySumFullIdx) {
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(13).Column).FormulaR1C1 = \""
                                                        << gSettings->FormatSumForList(lPaySumFullIdx) << "\"\n";
                                                    out << "  nColsAny(13) = True\n";
                                                } else
                                                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(13).Column).FormulaR1C1 = \"\"\n";
                                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(13).Column).HorizontalAlignment = -4152\n"; // right
                                                out << "End If\n";

                                                if (aWithFeaturedPay) {
                                                    if (queryHashbon.value("PAY_DATE_ISNULL").toInt() == 1) {
                                                        out << "' color expected payments\n";
                                                        out << "For i = 8 To 13\n";
                                                        out << "  If Not nCols(i) Is Nothing Then\n";
                                                        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 1645055\n";
                                                        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Color = 16777215\n";
                                                        out << "  End If\n";
                                                        out << "Next\n";
                                                        //lContractHasExpected = true;
                                                    } else {
                                                        //lContractHasPayed = true;
                                                    }
                                                }

                                                BorderWithStart(4);
                                                lCurRow++;
                                            }

                                            if (lCurRow > lRowContractSum + 1) {

                                                // summary for contract
                                                out << "' summary for contract\n";
                                                out << "For i = 8 To UBound(nCols) - 3\n";
                                                out << "  If Not nCols(i) Is Nothing Then\n";
                                                out << "    oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                                                out << "    If nColsAny(i) Then\n";
                                                out << "      oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).Value = \"=SUM(\" + chr(nCols(i).Column + asc(\"A\") - 1) + \""
                                                    << lRowContractSum + 1 << ":\" + chr(nCols(i).Column + asc(\"A\") - 1) + \"" << lCurRow - 1 << ")\"\n";
                                                out << "    End If\n";
                                                out << "    oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                                                out << "    oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).Font.Bold = True\n";
                                                out << "  End If\n";
                                                out << "Next\n";

                                                forYearSum.append(lRowContractSum);

                                                out << "oWsheet.Rows(\"" << lRowContractSum + 1 << ":" << lCurRow - 1 << "\").Group\n";
                                            }
                                        }
                                    }


                                    if (!forYearSum.isEmpty()) {
                                        // summary for year
                                        forProjectSum.append(lRowYearSum);
                                        QString cellList;
                                        cellList = "SUM(";
                                        for (k = 0; k < forYearSum.length(); k++) {
                                            if (k) {
                                                if (!(k % 24)) cellList += "),SUM(";
                                                else cellList += ",";
                                            }
                                            cellList += "R[" + QString::number(forYearSum.at(k) - lRowYearSum) + "]C[0]";
                                        }

                                        cellList += ")";

                                        out << "' summary for year\n";
                                        out << "For i = 8 To UBound(nCols) - 3\n";
                                        out << "  If Not nCols(i) Is Nothing Then\n";
                                        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                                        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
                                        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                                        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).Font.Bold = True\n";
                                        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).Font.Italic = True\n";
                                        out << "  End If\n";
                                        out << "Next\n";

                                        out << "oWsheet.Rows(\"" << lRowYearSum + 1 << ":" << lCurRow << "\").Group\n";
                                        out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
                                        lCurRow++;
                                    }
                                }
                            }
                        }

                        if (!forProjectSum.isEmpty()) {
                            // summary for project
                            QString cellList;
                            cellList = "SUM(";
                            for (k = 0; k < forProjectSum.length(); k++) {
                                if (k) {
                                    if (!(k % 24)) cellList += "),SUM(";
                                    else cellList += ",";
                                }
                                cellList += "R[" + QString::number(forProjectSum.at(k) - lRowProjectSum) + "]C[0]";
                            }

                            cellList += ")";

                            out << "' summary for project\n";
                            out << "For i = 8 To UBound(nCols) - 3\n";
                            out << "  If Not nCols(i) Is Nothing Then\n";
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Bold = True\n";
                            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Italic = True\n";
                            out << "  End If\n";
                            out << "Next\n";

                            //out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";

                            forCustomerSum.append(lRowProjectSum);
                        }
                        out << "oWsheet.Rows(\"" << lRowProjectSum + 1 << ":" << lCurRow - 1 << "\").Group\n";
                        //lCurRow++;
                    } else {
                        gLogger->ShowError(this, "חוזים", "Project " + queryProject.value("id_project").toString() + " not found!");
                    }
                }
            }
        }



        if (!forCustomerSumDebt.isEmpty()) {
            // summary for year
            QString cellList;
            cellList = "SUM(";
            for (k = 0; k < forCustomerSumDebt.length(); k++) {
                if (k) {
                    if (!(k % 24)) cellList += "),SUM(";
                    else cellList += ",";
                }
                cellList += "R[" + QString::number(forCustomerSumDebt.at(k) - lRowCustomerSum) + "]C[0]";
            }

            cellList += ")";

            out << "' summary for customer\n";
            out << "For i = 5 To 7\n";
            out << "  If Not nCols(i) Is Nothing Then\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).Font.Bold = True\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).Font.Italic = True\n";
            out << "  End If\n";
            out << "Next\n";

            out << "For i = 14 To 16\n";
            out << "  If Not nCols(i) Is Nothing Then\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).Font.Bold = True\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).Font.Italic = True\n";
            out << "  End If\n";
            out << "Next\n";

            //                out << "oWsheet.Rows(\"" << lRowCustomerSum + 1 << ":" << lCurRow << "\").Group\n";
            //                out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
            //                lCurRow++;
        }


        if (!forCustomerSum.isEmpty()) {
            // summary for year
            QString cellList;
            cellList = "SUM(";
            for (k = 0; k < forCustomerSum.length(); k++) {
                if (k) {
                    if (!(k % 24)) cellList += "),SUM(";
                    else cellList += ",";
                }
                cellList += "R[" + QString::number(forCustomerSum.at(k) - lRowCustomerSum) + "]C[0]";
            }

            cellList += ")";

            out << "' summary for customer\n";
            out << "For i = 8 To UBound(nCols) - 3\n";
            out << "  If Not nCols(i) Is Nothing Then\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).Font.Bold = True\n";
            out << "    oWsheet.Cells(" << lRowCustomerSum << ", nCols(i).Column).Font.Italic = True\n";
            out << "  End If\n";
            out << "Next\n";

            //                out << "oWsheet.Rows(\"" << lRowCustomerSum + 1 << ":" << lCurRow << "\").Group\n";
            //                out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
            //                lCurRow++;
        }

        if (!forCustomerSumDebt.isEmpty()
                || !forCustomerSum.isEmpty()) {
            out << "oWsheet.Rows(\"" << lRowCustomerSum + 1 << ":" << lCurRow << "\").Group\n";
            out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
            lCurRow++;
        }


    }
    out << "oWsheet.Outline.ShowLevels(2)\n";

    ReportEnd();
}
