#include "ContractPkz.h"
#include "ui_contractpkz.h"

#include "contract-pkz_local.h"

#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include "../VProject/XMLProcess.h"

#include <QDir>

#define ReportCommonTemplateOld "/common/templates/contract-rep-common.xls"
#define ReportMonthlyPaymentsTemplateOld "/common/templates/contract-rep-MonthlyPayments.xls"
#define ReportFullTemplateOld "/common/templates/contract-rep-Full.xls"
#define ReportProjByYearTemplateOld "/common/templates/contract-rep-ProjByYear.xls"

void ContractPkz::ReportCommonOld(bool aAll) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int i, j, lCurRow;

    QList<QContractTreeItem *> items;
    QList<int> forFullSum;


    for (i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        if (aAll && !ui->treeWidget->topLevelItem(i)->isHidden()
                || ui->treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked) {
            items.append((QContractTreeItem *) ui->treeWidget->topLevelItem(i));
        }
    }

    if (!items.length()) {
        for (i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
            if (ui->treeWidget->topLevelItem(i)->isSelected()) {
                items.append((QContractTreeItem *) ui->treeWidget->topLevelItem(i));
            }
        }
    }

    if (!items.length()) {
        if (ui->treeWidget->currentItem()
                && ((QContractTreeItem *) ui->treeWidget->currentItem())->IdProject())
            items.append((QContractTreeItem *) ui->treeWidget->currentItem());
    }

    if (!items.length()) {
        QMessageBox::critical(this, "Contracts - report", "Nothing selected!");
        return;
    }

    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportCommon-" + lTimeStamp;
    dir.setPath(lVbsName);
    if (!dir.exists() && !dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " +lVbsName);
        return;
    }

    lVbsName += "/ReportCommon.vbs";
    lTemplateName += ReportCommonTemplateOld;

    lOutName = "ReportCommon-" + lTimeStamp + ".xls";

    ReportStart();

    lCurRow = 4;

    out << "Dim nCols(15)' as Range\n";
    out << "Dim nColsAny(15)\n";

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
    out << "Set nCols(14) = oWsheet.Cells.Find(\"#REST_SUM#\")\n";
    out << "Set nCols(15) = oWsheet.Cells.Find(\"#REST_SUM_NEXT_YEAR#\")\n";

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

    //Selection.Rows.Group

    out << "If Not nCols(UBound(nCols)) Is Nothing Then\n";
    out << "  oWsheet.Cells(1, nCols(UBound(nCols)).Column).FormulaR1C1 = \"שארית\" & vbLf & \"" + QString::number(QDateTime::currentDateTime().date().year() + 1) + "\"\n";
    out << "End If\n";

    for (i = 0; i < items.length(); i++) {
        QContractTreeItem *projectItem = items.at(i);
        qlonglong color;
        int lRowProjectSum;

        color = gSettings->Contract.ProjectColor.blue();
        color <<= 8;
        color += gSettings->Contract.ProjectColor.green();
        color <<= 8;
        color += gSettings->Contract.ProjectColor.red();

        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << projectItem->ProjectName().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
        //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Interior.Color = " << QString::number(color) << "\n";

        lRowProjectSum = lCurRow;
        BorderWithStart(5);

        out << "For i = 8 To 13\n";
        out << "  nColsAny(i) = False\n";
        out << "Next\n";

        lCurRow++;
        //qlonglong lProjectSum = 0, lProjectSumNds = 0, lProjectSumFull = 0;
        for (j = 0; j < projectItem->childCount(); j++) {
            if (projectItem->child(j)->isHidden()) continue;
            QContractTreeItem *contractItem = (QContractTreeItem *) projectItem->child(j);

            out << "If Not nCols(1) Is Nothing Then\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << contractItem->Num().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4152\n"; // right
            out << "End If\n";

            out << "If Not nCols(2) Is Nothing Then\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).NumberFormat = \"@\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).FormulaR1C1 = \"" << contractItem->StartDate().toString("dd.MM.yyyy").replace("\"", "\"\"") << "\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).HorizontalAlignment = -4108\n"; // center
            out << "End If\n";

            out << "If Not nCols(3) Is Nothing Then\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).NumberFormat = \"@\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).FormulaR1C1 = \"" << contractItem->EndDate().toString("dd.MM.yyyy").replace("\"", "\"\"") << "\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).HorizontalAlignment = -4108\n"; // center
            out << "End If\n";

            out << "If Not nCols(4) Is Nothing Then\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << contractItem->Name().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).WrapText = False\n";
            out << "End If\n";

            qlonglong lSum = 0, lSumFull = 0;
            lSum = contractItem->Sum();
            lSumFull = contractItem->SumFull();

            //                lProjectSum += lSum;
            //                lProjectSumNds += lSumFull - lSum;
            //                lProjectSumFull += lSumFull;

            out << "If Not nCols(5) Is Nothing Then\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \"" << gSettings->FormatSumForList(lSum) << "\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
            out << "End If\n";

            out << "If Not nCols(6) Is Nothing Then\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \"" << gSettings->FormatSumForList(lSumFull - lSum) << "\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
            out << "End If\n";

            out << "If Not nCols(7) Is Nothing Then\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \"" << gSettings->FormatSumForList(lSumFull) << "\"\n";
            out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
            out << "End If\n";

            QSqlQuery query(db);
            if (contractItem->childCount()) {
                // via stages
                query.prepare(
                            "select nvl(sum(nvl(b.pay_sum_brutto, 0)), 0), nvl(sum(nvl(b.pay_sum_full, 0)), 0),"
                            " nvl(sum(nvl(nvl(b.pay_sum_brutto_indexed, b.pay_sum_brutto), 0)), 0),"
                            " nvl(sum(nvl(nvl(b.pay_sum_full_indexed, b.pay_sum_full), 0)), 0),"
                            " nvl(sum(nvl(b.pay_sum_brutto, decode(to_char(b.expect_date, 'yy'), to_char(sysdate, 'yy'), b.orig_sum_brutto, 0))), 0) forecast"
                            " from v_pkz_contract_stage a, v_pkz_hashbon b"
                            " where a.id_pkz_contract = " + QString::number(contractItem->IdContract()) +
                            " and b.id_contract_stage = a.id");
            } else {
                // no stages
                query.prepare(
                            "select nvl(sum(nvl(pay_sum_brutto, 0)), 0), nvl(sum(nvl(pay_sum_full, 0)), 0),"
                            " nvl(sum(nvl(nvl(pay_sum_brutto_indexed, pay_sum_brutto), 0)), 0),"
                            " nvl(sum(nvl(nvl(pay_sum_full_indexed, pay_sum_full), 0)), 0),"
                            " nvl(sum(nvl(pay_sum_brutto, decode(to_char(expect_date, 'yy'), to_char(sysdate, 'yy'), orig_sum_brutto, 0))), 0) forecast"
                            " from v_pkz_hashbon where id_contract = " + QString::number(contractItem->IdContract()));
            }

            if (query.lastError().isValid()) {
                gLogger->ShowSqlError(this, "Contracts - report", query);
            } else {
                if (!query.exec()) {
                    gLogger->ShowSqlError(this, "Contracts - report", query);
                } else {
                    if (!query.next()) {
                        gLogger->ShowError(this, "Contracts - report", "No data found");
                    } else {
                        qlonglong lPaySum, lPaySumFull, lPaySumIdx, lPaySumFullIdx, lPayForecast;

                        lPaySum = query.value(0).toLongLong();
                        lPaySumFull = query.value(1).toLongLong();
                        lPaySumIdx = query.value(2).toLongLong();
                        lPaySumFullIdx = query.value(3).toLongLong();
                        lPayForecast = query.value(4).toLongLong();

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

                        // rest
                        out << "If Not nCols(14) Is Nothing Then\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).NumberFormat = \"#,##0.00\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).FormulaR1C1 = \""
                            << gSettings->FormatSumForList(lSum - lPaySum) << "\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).HorizontalAlignment = -4152\n"; // right
                        out << "End If\n";

                        // rest next year
                        out << "If Not nCols(15) Is Nothing Then\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).NumberFormat = \"#,##0.00\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).FormulaR1C1 = \""
                            << gSettings->FormatSumForList(lSum - lPayForecast) << "\"\n";
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).HorizontalAlignment = -4152\n"; // right
                        out << "End If\n";
                    }
                }
            }

            Border;
            lCurRow++;
        }

        // summary for project
        out << "For i = 5 To UBound(nCols)\n";
        out << "  If Not nCols(i) Is Nothing Then\n";
        out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
        out << "    If nColsAny(i) Then\n";
        out << "      oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Value = \"=SUM(\" + chr(nCols(i).Column + asc(\"A\") - 1) + \""
            << lRowProjectSum + 1 << ":\" + chr(nCols(i).Column + asc(\"A\") - 1) + \"" << lCurRow - 1 << ")\"\n";
        out << "    End If\n";
        out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
        out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Bold = True\n";
        out << "  End If\n";
        out << "Next\n";

        forFullSum.append(lRowProjectSum);

        out << "oWsheet.Rows(\"" << lRowProjectSum + 1 << ":" << lCurRow << "\").Group\n";

        if (i != items.length() - 1) {
            out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
            lCurRow++;
        }
    }

    if (items.length() > 1) {
        //Border;
        lCurRow++;

        // it is normal version, not worked on 2003
        // summary for all
        QString cellList;
        /*cellList = "SUM(";
        for (i = 0; i < forFullSum.length(); i++) {
            if (i) {
                if (!(i % 24)) cellList += "),SUM(";
                else cellList += ",";
            }
            cellList += "R[" + QString::number(forFullSum.at(i) - lCurRow) + "]C[0]";
        }
        cellList += ")";

        out << "For i = 5 To UBound(nCols)\n";
        out << "  If Not nCols(i) Is Nothing Then\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Bold = True\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Italic = True\n";
        out << "  End If\n";
        out << "Next\n";*/

        // part for work in 2003
        //-----------------------------------------------------------------------
        // summary for all
        //QString cellList;
        cellList = "";
        int lAddRow = 1;
        QList<int> lPartSum;
        for (i = 0; i < forFullSum.length(); i++) {
            if (i) {
                if (!(i % 24)) {
                    out << "For i = 5 To UBound(nCols)\n";
                    out << "  If Not nCols(i) Is Nothing Then\n";
                    out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
                    out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                    out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).Font.Bold = True\n";
                    out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).Font.Italic = True\n";
                    out << "  End If\n";
                    out << "Next\n";

                    lPartSum.append(lCurRow + lAddRow);
                    cellList = "";
                    lAddRow++;
                } else cellList += ",";
            }
            cellList += "R[" + QString::number(forFullSum.at(i) - lCurRow - lAddRow) + "]C[0]";
        }

        if (!cellList.isEmpty()) {
            out << "For i = 5 To UBound(nCols)\n";
            out << "  If Not nCols(i) Is Nothing Then\n";
            out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
            out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
            out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
            out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).Font.Bold = True\n";
            out << "    oWsheet.Cells(" << lCurRow + lAddRow << ", nCols(i).Column).Font.Italic = True\n";
            out << "  End If\n";
            out << "Next\n";
            lPartSum.append(lCurRow + lAddRow);
        }

        cellList = "";
        for (i = 0; i < lPartSum.length(); i++) {
            if (i) cellList += ",";
            cellList += "R[" + QString::number(lPartSum.at(i) - lCurRow) + "]C[0]";
            out << "oWsheet.Rows(" << lPartSum.at(i) << ").Hidden = true\n";
        }
        out << "For i = 5 To UBound(nCols)\n";
        out << "  If Not nCols(i) Is Nothing Then\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Bold = True\n";
        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Italic = True\n";
        out << "  End If\n";
        out << "Next\n";
        //-----------------------------------------------------------------------

        BorderWithStart(5);;
    }

    ReportEnd();
}

void ContractPkz::SummaryByYearOld(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int i, j, lCurRow;

    QList<int> forFullSum;


    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportSumYear-" + lTimeStamp;
    dir.setPath(lVbsName);
    if (!dir.exists() && !dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " +lVbsName);
        return;
    }

    lVbsName += "/ReportSumYear.vbs";
    lTemplateName += ReportCommonTemplateOld;

    lOutName = QString::number(aYear) + "-ReportSumYear-" + lTimeStamp + ".xls";

    ReportStart();

    lCurRow = 4;

    out << "oWsheet.Cells(1, 1).FormulaR1C1 = \"" << QString::number(aYear) << "\"\n";

    out << "Dim nCols(14)' as Range\n";
    out << "Dim nColsAny(14)\n";

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
    out << "Set nCols(14) = oWsheet.Cells.Find(\"#REST_SUM#\")\n";

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

    //Selection.Rows.Group

    //out << "oWsheet.Columns(chr(nCols(UBound(nCols)).Column + asc(\"A\")) + \":\" + chr(nCols(UBound(nCols)).Column + asc(\"A\"))).EntireColumn.Hidden = True\n";
    out << "oWsheet.Columns(chr(nCols(UBound(nCols)).Column + asc(\"A\")) + \":\" + chr(nCols(UBound(nCols)).Column + asc(\"A\"))).Delete\n";

    QSqlQuery query(db);

    if (!aWithFeaturedPay) {
        query.prepare("SELECT DISTINCT A.ID_PROJECT ID_PROJECT, PP.GETPROJECTSHORTNAME(A.ID_PROJECT) PROJNAME"
                      " FROM V_PKZ_CONTRACT A"
                      " WHERE (EXISTS"
                      "  (SELECT 1 FROM V_PKZ_HASHBON"
                      "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = " + QString::number(aYear) + ")"
                      "   OR EXISTS"
                      "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                      "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = " + QString::number(aYear) + "))"
                      " ORDER BY PP.GETPROJECTSHORTNAME(A.ID_PROJECT)");
    } else {
        query.prepare("SELECT DISTINCT A.ID_PROJECT ID_PROJECT, PP.GETPROJECTSHORTNAME(A.ID_PROJECT) PROJNAME"
                      " FROM V_PKZ_CONTRACT A"
                      " WHERE (EXISTS"
                      "  (SELECT 1 FROM V_PKZ_HASHBON"
                      "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = " + QString::number(aYear) + ")"
                      "   OR EXISTS"
                      "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                      "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = " + QString::number(aYear) + "))"
                      " ORDER BY PP.GETPROJECTSHORTNAME(A.ID_PROJECT)");
    }

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", query);
    } else {
        if (!query.exec()) {
            gLogger->ShowSqlError(this, "חוזים", query);
        } else {
            while (query.next()) {
                if (!aProjects.isEmpty()
                        && !aProjects.contains(query.value("ID_PROJECT").toInt())) continue;// skipped by list

                int lRowProjectSum;
                QList<int> forProjectSum;

                qlonglong color;
                color = gSettings->Contract.ProjectColor.blue();
                color <<= 8;
                color += gSettings->Contract.ProjectColor.green();
                color <<= 8;
                color += gSettings->Contract.ProjectColor.red();

                out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
                out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << query.value("PROJNAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
                //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
                out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
                out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Interior.Color = " << QString::number(color) << "\n";

                out << "For i = 8 To 13\n";
                out << "  nColsAny(i) = False\n";
                out << "Next\n";

                lRowProjectSum = lCurRow;

                BorderWithStart(8);
                lCurRow++;

                // in fact, no "rest" (debt) in this report
                QSqlQuery query2(db);
                if (!aWithFeaturedPay) {
                    query2.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                                   " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL,"
                                   " B.SUM_BRUTTO -"
                                   "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                                   "        WHERE ID_CONTRACT = A.ID)"
                                   "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                   "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST"
                                   " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                                   " WHERE A.ID_PROJECT = " + query.value("ID_PROJECT").toString() +
                                   " AND B.ID_CONTRACT = A.ID"
                                   " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                                   " AND (EXISTS"
                                   "  (SELECT 1 FROM V_PKZ_HASHBON"
                                   "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = " + QString::number(aYear) + ")"
                                   "   OR EXISTS"
                                   "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                                   "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = " + QString::number(aYear) + "))"
                                   " ORDER BY A.NUM");
                } else {
                    query2.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                                   " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL,"
                                   " B.SUM_BRUTTO -"
                                   "    ((SELECT NVL(SUM(NVL(DECODE(PAY_DATE, NULL, ORIG_SUM_BRUTTO, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                                   "        WHERE ID_CONTRACT = A.ID)"
                                   "   + (SELECT NVL(SUM(NVL(DECODE(PAY_DATE, NULL, ORIG_SUM_BRUTTO, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                   "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST"
                                   " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                                   " WHERE A.ID_PROJECT = " + query.value("ID_PROJECT").toString() +
                                   " AND B.ID_CONTRACT = A.ID"
                                   " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                                   " AND (EXISTS"
                                   "  (SELECT 1 FROM V_PKZ_HASHBON"
                                   "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = " + QString::number(aYear) + ")"
                                   "   OR EXISTS"
                                   "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                                   "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = " + QString::number(aYear) + "))"
                                   " ORDER BY A.NUM");
                }
                if (query2.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "חוזים", query2);
                    break;
                } else {
                    if (!query2.exec()) {
                        gLogger->ShowSqlError(this, "חוזים", query2);
                        break;
                    } else {
                        int lRowContractSum;
                        j = 0;

                        while (query2.next()) {
                            //if (!j) lFirstRow = lCurRow;
                            //if (j == projectItem->childCount() - 1) lLastRow = lCurRow;

                            out << "If Not nCols(1) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << query2.value("NUM").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4152\n"; // right
                            out << "End If\n";

                            out << "If Not nCols(2) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).NumberFormat = \"@\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).FormulaR1C1 = \"" << query2.value("START_DATE").toString() << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).HorizontalAlignment = -4108\n"; // center
                            out << "End If\n";

                            out << "If Not nCols(3) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).NumberFormat = \"@\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).FormulaR1C1 = \"" << query2.value("END_DATE").toString() << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).HorizontalAlignment = -4108\n"; // center
                            out << "End If\n";

                            out << "If Not nCols(4) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << query2.value("NAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).WrapText = False\n";
                            out << "End If\n";

                            out << "If Not nCols(5) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
                                << gSettings->FormatSumForList(query2.value("SUM").toLongLong()) << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
                            out << "End If\n";

                            out << "If Not nCols(6) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
                                << gSettings->FormatSumForList(query2.value("SUM_FULL").toLongLong() - query2.value("SUM").toLongLong()) << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
                            out << "End If\n";

                            out << "If Not nCols(7) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
                                << gSettings->FormatSumForList(query2.value("SUM_FULL").toLongLong()) << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
                            out << "End If\n";

                            // rest
                            out << "If Not nCols(14) Is Nothing Then\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).NumberFormat = \"#,##0.00\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).FormulaR1C1 = \""
                                << gSettings->FormatSumForList(query2.value("REST").toLongLong()) << "\"\n";
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).HorizontalAlignment = -4152\n"; // right
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).Font.Bold = True\n"; // right
                            out << "End If\n";

                            lRowContractSum = lCurRow;

                            Border;
                            lCurRow++;

                            QSqlQuery query3(db);

                            if (!aWithFeaturedPay) {
                                query3.prepare("SELECT PAY_DATE, TO_CHAR(PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                                               " PAY_SUM_BRUTTO, PAY_SUM_FULL,"
                                               " NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                                               " NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                                               " FROM V_PKZ_HASHBON"
                                               " WHERE ID_CONTRACT = " + query2.value("ID").toString() +
                                               " AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = " + QString::number(aYear) +
                                               " UNION SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                                               " A.PAY_SUM_BRUTTO, A.PAY_SUM_FULL,"
                                               " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                                               " NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                                               " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                                               " WHERE B.ID_PKZ_CONTRACT = " + query2.value("ID").toString() +
                                               " AND A.ID_CONTRACT_STAGE = B.ID"
                                               " AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = " + QString::number(aYear) +
                                               " ORDER BY 1");
                            } else {
                                query3.prepare("SELECT PAY_DATE, TO_CHAR(PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                                               " DECODE(PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                                               " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                                               " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_FULL, ORIG_SUM_FULL), PAY_SUM_FULL) PAY_SUM_FULL,"
                                               " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO), NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO)), NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                                               " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL), NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL)), NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                                               " FROM V_PKZ_HASHBON"
                                               " WHERE ID_CONTRACT = " + query2.value("ID").toString() +
                                               " AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = " + QString::number(aYear) +
                                               " UNION SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                                               " DECODE(A.PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                                               " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                                               " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                                               " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                                               " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                                               " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                                               " WHERE B.ID_PKZ_CONTRACT = " + query2.value("ID").toString() +
                                               " AND A.ID_CONTRACT_STAGE = B.ID"
                                               " AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = " + QString::number(aYear) +
                                               " ORDER BY 1");
                            }

                            if (query3.lastError().isValid()) {
                                gLogger->ShowSqlError(this, "Contracts - report", query3);
                                break;
                            } else {
                                if (!query3.exec()) {
                                    gLogger->ShowSqlError(this, "Contracts - report", query3);
                                    break;
                                } else {
                                    bool lContractHasExpected = false, lContractHasPayed = false;
                                    while (query3.next()) {
                                        qlonglong lPaySum, lPaySumFull, lPaySumIdx, lPaySumFullIdx;

                                        lPaySum = query3.value("PAY_SUM_BRUTTO").toLongLong();
                                        lPaySumFull = query3.value("PAY_SUM_FULL").toLongLong();
                                        lPaySumIdx = query3.value("PAY_SUM_BRUTTO_INDEXED").toLongLong();
                                        lPaySumFullIdx = query3.value("PAY_SUM_FULL_INDEXED").toLongLong();

                                        // pay date
                                        out << "If Not nCols(4) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << query3.value("PAY_DATE_STR").toString() << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).HorizontalAlignment = -4108\n"; // center
                                        out << "End If\n";

                                        // invoice
                                        out << "If Not nCols(5) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"@\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \"" << query3.value("INVOICE").toString() << "\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4108\n"; // center
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Italic = True\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Size = oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Size - 1\n";
                                        out << "End If\n";

                                        // pay invoice
                                        out << "If Not nCols(6) Is Nothing Then\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"@\"\n";
                                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \"" << query3.value("PAY_INVOICE").toString() << "\"\n";
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

                                        //                                // rest
                                        //                                out << "If Not nCols(14) Is Nothing Then\n";
                                        //                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).NumberFormat = \"#,##0.00\"\n";
                                        //                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).FormulaR1C1 = \""
                                        //                                    << gSettings->FormatSumForList(lSum - lPaySum) << "\"\n";
                                        //                                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).HorizontalAlignment = -4152\n"; // right
                                        //                                out << "End If\n";

                                        if (aWithFeaturedPay) {
                                            if (query3.value("PAY_DATE_ISNULL").toInt() == 1) {
                                                out << "' color expected payments\n";
                                                out << "For i = 8 To 13\n";
                                                out << "  If Not nCols(i) Is Nothing Then\n";
                                                out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 1645055\n";
                                                out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Color = 16777215\n";
                                                out << "  End If\n";
                                                out << "Next\n";
                                                lContractHasExpected = true;
                                            } else {
                                                lContractHasPayed = true;
                                            }
                                        }

                                        BorderWithStart(4);
                                        lCurRow++;
                                    }

                                    // summary for contract
                                    out << "For i = 8 To UBound(nCols) - 1\n";
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

                                    forProjectSum.append(lRowContractSum);

                                    out << "oWsheet.Rows(\"" << lRowContractSum + 1 << ":" << lCurRow << "\").Group\n";

                                    out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
                                    lCurRow++;
                                }
                            }
                        }
                    }
                }

                forFullSum.append(lRowProjectSum);
                // summary for project
                QString cellList;
                cellList = "SUM(";
                for (i = 0; i < forProjectSum.length(); i++) {
                    if (i) {
                        if (!(i % 24)) cellList += "),SUM(";
                        else cellList += ",";
                    }
                    cellList += "R[" + QString::number(forProjectSum.at(i) - lRowProjectSum) + "]C[0]";
                }
                cellList += ")";

                out << "For i = 8 To UBound(nCols) - 1\n";
                out << "  If Not nCols(i) Is Nothing Then\n";
                out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
                out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Bold = True\n";
                out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Italic = True\n";
                out << "  End If\n";
                out << "Next\n";

                out << "oWsheet.Rows(\"" << lRowProjectSum + 1 << ":" << lCurRow - 1 << "\").Group\n";
            }
        }
    }

    lCurRow++;

    // summary for all
    // compose SUM of SUM cos max parameter count is 31
    QString cellList;
    cellList = "SUM(";
    for (i = 0; i < forFullSum.length(); i++) {
        if (i) {
            if (!(i % 24)) cellList += "),SUM(";
            else cellList += ",";
        }
        cellList += "R[" + QString::number(forFullSum.at(i) - lCurRow) + "]C[0]";
    }
    cellList += ")";

    out << "For i = 8 To UBound(nCols) - 1\n";
    out << "  If Not nCols(i) Is Nothing Then\n";
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Bold = True\n";
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Italic = True\n";
    out << "  End If\n";
    out << "Next\n";

    BorderWithStart(8);

    out << "oWsheet.Columns(\"N:N\").Delete(xlToLeft)\n";

    ReportEnd();
}

void ContractPkz::ReportProjPayByYearsOld(bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int j, k, lCurRow, lRowContractSum, lRowYearSum, lRowProjectSum;
    qlonglong color;

    if (aProjects.isEmpty()) {
        QMessageBox::critical(this, "Contracts - report", "Please select at least one project!");
        return;
    }

    QString aProjectsStr;

    for (int i = 0; i < aProjects.length(); i++) {
        if (aProjectsStr.isEmpty()) {
            aProjectsStr = QString::number(aProjects.at(i));
        } else {
            aProjectsStr += ", " + QString::number(aProjects.at(i));
        }
    }

    QSqlQuery queryProject(db), queryContractRest(db), queryContract(db), queryHashbon(db);

    if (!aWithFeaturedPay) {
        queryProject.prepare("SELECT A.ID ID, PP.GETPROJECTSHORTNAME(A.ID) PROJNAME,"
                             " LEAST("
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000),"
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000)) MIN1,"
                             " GREATEST("
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 0),"
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 0)) MAX1,"
                             " (SELECT SUM(C.SUM_BRUTTO) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM,"
                             " (SELECT SUM(C.SUM_FULL) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM_FULL"
                             " FROM V_PROJECT A WHERE A.ID IN (" + aProjectsStr + ")"
                             " ORDER BY A.SHORTNAME");
    } else {
        queryProject.prepare("SELECT A.ID ID, PP.GETPROJECTSHORTNAME(A.ID) PROJNAME,"
                             " LEAST("
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000),"
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000)) MIN1,"
                             " GREATEST("
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 0),"
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 0)) MAX1,"
                             " (SELECT SUM(C.SUM_BRUTTO) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM,"
                             " (SELECT SUM(C.SUM_FULL) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM_FULL"
                             " FROM V_PROJECT A WHERE A.ID IN (" + aProjectsStr + ")"
                             " ORDER BY A.SHORTNAME");
    }

    if (queryProject.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryProject);
        return;
    }

    if (!aWithFeaturedPay) {
        queryContract.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                              " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL"
                              " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                              " WHERE A.ID_PROJECT = :ID_PROJECT"
                              " AND B.ID_CONTRACT = A.ID"
                              " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                              " AND (EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON"
                              "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = :YEAR1)"
                              "   OR EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                              "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = :YEAR2))"
                              " ORDER BY A.NUM");
    } else {
        queryContract.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                              " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL"
                              " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                              " WHERE A.ID_PROJECT = :ID_PROJECT"
                              " AND B.ID_CONTRACT = A.ID"
                              " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                              " AND (EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON"
                              "     WHERE ID_CONTRACT = A.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = :YEAR1)"
                              "   OR EXISTS"
                              "  (SELECT 1 FROM V_PKZ_HASHBON C, V_PKZ_CONTRACT_STAGE D"
                              "     WHERE D.ID_PKZ_CONTRACT = A.ID AND C.ID_CONTRACT_STAGE = D.ID AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = :YEAR2))"
                              " ORDER BY A.NUM");

    }
    if (queryContract.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryContract);
        return;
    }

    if (!aWithFeaturedPay) {
        queryContractRest.prepare("SELECT A.ID, A.CUSTOMER, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE, A.NAME,"
                                  " B.SUM_BRUTTO SUM, B.SUM_FULL,"
                                  "0 REST_THIS_YEAR,"

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
                                  " ORDER BY A.NUM");
    } else {
        queryContractRest.prepare("SELECT A.ID, A.CUSTOMER, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE, A.NAME,"
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
                                  " WHERE A.ID_PROJECT = :ID_PROJECT"
                                  " AND B.ID_CONTRACT = A.ID"
                                  " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                                  " AND B.SUM_BRUTTO -"
                                  "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                                  "        WHERE ID_CONTRACT = A.ID)"
                                  "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                  "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                                  " ORDER BY A.NUM");

    }
    if (queryContractRest.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryContractRest);
        return;
    }

    if (!aWithFeaturedPay) {
        queryHashbon.prepare("SELECT PAY_DATE, TO_CHAR(PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                             " PAY_SUM_BRUTTO, PAY_SUM_FULL,"
                             " NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                             " NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON"
                             " WHERE ID_CONTRACT = :ID_CONTRACT1"
                             " AND TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY')) = :YEAR1"
                             " UNION"
                             " SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                             " A.PAY_SUM_BRUTTO, A.PAY_SUM_FULL,"
                             " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                             " NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                             " WHERE B.ID_PKZ_CONTRACT = :ID_CONTRACT2"
                             " AND A.ID_CONTRACT_STAGE = B.ID"
                             " AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = :YEAR2"
                             " ORDER BY 1 DESC");
    } else {
        queryHashbon.prepare("SELECT PAY_DATE, TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                             " DECODE(PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                             " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                             " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_FULL, ORIG_SUM_FULL), PAY_SUM_FULL) PAY_SUM_FULL,"
                             " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO), NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO)), NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                             " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL), NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL)), NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON"
                             " WHERE ID_CONTRACT = :ID_CONTRACT1"
                             " AND TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY')) = :YEAR1"
                             " UNION"
                             " SELECT PAY_DATE, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                             " DECODE(A.PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                             " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                             " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                             " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                             " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                             " WHERE B.ID_PKZ_CONTRACT = :ID_CONTRACT2"
                             " AND A.ID_CONTRACT_STAGE = B.ID"
                             " AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = :YEAR2"
                             " ORDER BY 1 DESC");
    }

    if (queryHashbon.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryHashbon);
        return;
    }

    // queryProject is ready here
    if (!queryProject.exec()) {
        gLogger->ShowSqlError(this, "חוזים", queryProject);
        return;
    }

    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportProjByYear-" + lTimeStamp;
    dir.setPath(lVbsName);
    if (!dir.exists() && !dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " +lVbsName);
        return;
    }

    lVbsName += "/ReportProjByYear.vbs";
    lTemplateName += ReportProjByYearTemplateOld;

    lOutName = "ReportProjByYear-" + lTimeStamp + ".xls";

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

    //Selection.Rows.Group


    QList<int> forProjectSum;

    while (queryProject.next()) {
        color = gSettings->Contract.ProjectColor.blue();
        color <<= 8;
        color += gSettings->Contract.ProjectColor.green();
        color <<= 8;
        color += gSettings->Contract.ProjectColor.red();

        forProjectSum.clear();

        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << queryProject.value("PROJNAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf& \"") << "\"\n";
        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
        //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Interior.Color = " << QString::number(color) << "\n";

        out << "If Not nCols(5) Is Nothing Then\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
            << gSettings->FormatSumForList(queryProject.value("SUM").toLongLong()) << "\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Bold = True\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Italic = True\n";
        out << "End If\n";

        out << "If Not nCols(6) Is Nothing Then\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
            << gSettings->FormatSumForList(queryProject.value("SUM_FULL").toLongLong() - queryProject.value("SUM").toLongLong()) << "\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Bold = True\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Italic = True\n";
        out << "End If\n";

        out << "If Not nCols(7) Is Nothing Then\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
            << gSettings->FormatSumForList(queryProject.value("SUM_FULL").toLongLong()) << "\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).Font.Bold = True\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).Font.Italic = True\n";
        out << "End If\n";

        lRowProjectSum = lCurRow;
        BorderWithStart(5);
        lCurRow++;

        // debt -----------------------------------------------------------------------------------------------------------------------------
        queryContractRest.bindValue(":ID_PROJECT", queryProject.value("ID"));
        queryContractRest.exec();

        if (queryContractRest.lastError().isValid()) {
            gLogger->ShowSqlError(this, "חוזים", queryContractRest);
        } else {
            while (queryContractRest.next()) {
                if (lCurRow == lRowProjectSum + 1) {
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
                    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 14540253\n";
                    out << "  End If\n";
                    out << "Next\n";

                    Border;
                    lCurRow++;
                }

                out << "If Not nCols(1) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << queryContractRest.value("NUM").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(2) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).FormulaR1C1 = \"" << queryContractRest.value("START_DATE").toString() << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).HorizontalAlignment = -4108\n"; // center
                out << "End If\n";

                out << "If Not nCols(3) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).FormulaR1C1 = \"" << queryContractRest.value("END_DATE").toString() << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).HorizontalAlignment = -4108\n"; // center
                out << "End If\n";

                out << "If Not nCols(4) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << queryContractRest.value("NAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).WrapText = False\n";
                out << "End If\n";

                out << "If Not nCols(5) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("SUM").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(6) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("SUM_FULL").toLongLong() - queryContractRest.value("SUM").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(7) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("SUM_FULL").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(14) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("REST_THIS_YEAR").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(15) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("REST_FULL").toLongLong() - queryContractRest.value("REST_THIS_YEAR").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(16) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("REST_FULL").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";


                out << "For i = 1 To UBound(nCols)\n";
                out << "  If Not nCols(i) Is Nothing Then\n";
                out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 14540253\n";
                out << "  End If\n";
                out << "Next\n";

                Border;
                lRowContractSum = lCurRow;
                lCurRow++;

            }

            // summary for debt
            out << "' summary for debt\n";
            out << "For i = 14 To 16\n";
            out << "  If Not nCols(i) Is Nothing Then\n";
            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
            out << "    If nColsAny(i) Then\n";

            if (lCurRow > lRowProjectSum + 1) {
                out << "      oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Value = \"=SUM(\" + chr(nCols(i).Column + asc(\"A\") - 1) + \""
                    << lRowProjectSum + 2 << ":\" + chr(nCols(i).Column + asc(\"A\") - 1) + \"" << lCurRow - 1 << ")\"\n";
            } else {
                out << "      oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Value = 0\n";
            }

            out << "    End If\n";
            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Bold = True\n";
            out << "  End If\n";
            out << "Next\n";

            if (lCurRow > lRowProjectSum + 1) {
                // bug: with + 2 empty string with string "Debt" not collapsed
                // it can be + 1 instead of + 2 but it is looking strange in excel
                out << "oWsheet.Rows(\"" << lRowProjectSum + 1 << ":" << lCurRow << "\").Group\n";
                out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
                lCurRow++;
            }
        }
        // end of debt -----------------------------------------------------------------------------------------------------------------------------


        if (queryProject.value("MIN1").toInt() != 5000) {
            for (j = queryProject.value("MAX1").toInt(); j >= queryProject.value("MIN1").toInt(); j--) {
                QList<int> forYearSum;;

                lRowYearSum = lCurRow;

                queryContract.bindValue(":ID_PROJECT", queryProject.value("ID"));
                queryContract.bindValue(":YEAR1", j);
                queryContract.bindValue(":YEAR2", j);

                queryContract.exec();

                if (queryContract.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "חוזים", queryContract);
                } else {
                    while (queryContract.next()) {
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

                        queryHashbon.bindValue(":ID_CONTRACT1", queryContract.value("ID"));
                        queryHashbon.bindValue(":YEAR1", j);
                        queryHashbon.bindValue(":ID_CONTRACT2", queryContract.value("ID"));
                        queryHashbon.bindValue(":YEAR2", j);

                        queryHashbon.exec();

                        if (queryHashbon.lastError().isValid()) {
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
        }
        out << "oWsheet.Rows(\"" << lRowProjectSum + 1 << ":" << lCurRow << "\").Group\n";
        lCurRow++;
    }
    ReportEnd();
}

void ContractPkz::MonthlyPaymentsOld(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int i, k, lCurRow, lRowMonthSum, lRowYearSum;

    QList<int> forYearSum;


    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportMonthlyPayments-" + lTimeStamp;
    dir.setPath(lVbsName);
    if (!dir.exists() && !dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " +lVbsName);
        return;
    }

    lVbsName += "/ReportMonthlyPayments.vbs";
    lTemplateName += ReportMonthlyPaymentsTemplateOld;

    lOutName = QString::number(aYear) + "-ReportMonthlyPayments-" + lTimeStamp + ".xls";

    ReportStart();

    lRowYearSum = 4;
    lCurRow = 4;

    out << "Dim nCols(17)' as Range\n";
    out << "Dim nColsAny(17)\n";

    out << "Set nCols(0) = oWsheet.Cells.Find(\"#MONTH#\")\n";
    out << "Set nCols(1) = oWsheet.Cells.Find(\"#PAY_DATE#\")\n";
    out << "Set nCols(2) = oWsheet.Cells.Find(\"#NUM#\")\n";
    out << "Set nCols(3) = oWsheet.Cells.Find(\"#START_DATE#\")\n";
    out << "Set nCols(4) = oWsheet.Cells.Find(\"#END_DATE#\")\n";
    out << "Set nCols(5) = oWsheet.Cells.Find(\"#NAME#\")\n";
    out << "Set nCols(6) = oWsheet.Cells.Find(\"#SUM#\")\n";
    out << "Set nCols(7) = oWsheet.Cells.Find(\"#SUM_NDS#\")\n";
    out << "Set nCols(8) = oWsheet.Cells.Find(\"#SUM_FULL#\")\n";
    out << "Set nCols(9) = oWsheet.Cells.Find(\"#PAY_SUM#\")\n";
    out << "Set nCols(10) = oWsheet.Cells.Find(\"#PAY_SUM_NDS#\")\n";
    out << "Set nCols(11) = oWsheet.Cells.Find(\"#PAY_SUM_FULL#\")\n";
    out << "Set nCols(12) = oWsheet.Cells.Find(\"#PAY_SUM_IDX#\")\n";
    out << "Set nCols(13) = oWsheet.Cells.Find(\"#PAY_SUM_NDS_IDX#\")\n";
    out << "Set nCols(14) = oWsheet.Cells.Find(\"#PAY_SUM_FULL_IDX#\")\n";
    out << "Set nCols(15) = oWsheet.Cells.Find(\"#INVOICE#\")\n";
    out << "Set nCols(16) = oWsheet.Cells.Find(\"#PAY_INVOICE#\")\n";
    out << "Set nCols(17) = oWsheet.Cells.Find(\"#PROJ_NAME#\")\n";

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

    out << "oWsheet.Range(\"A4:B4\").Merge\n";
    out << "oWsheet.Cells(4, 1).FormulaR1C1 = \"" << QString::number(aYear) << "\"\n";
    out << "oWsheet.Cells(4, 1).Font.Bold = True\n";
    out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"4:\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"4\").Interior.Color = 8781311\n";
    out << "oWsheet.Cells(4, 1).HorizontalAlignment = -4152\n"; // right

    BorderWithStartEnd(9, 3);
    lCurRow++;

    QSqlQuery query(db);
    for (i = 12; i >= 1; i--) {
//        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
//        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << gSettings->MonthName(i) << "\"\n";
//        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
//        //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
//        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
//        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Interior.Color = 7531363\n";

//        BorderWithStartEnd(9, 3);
//        lRowMonthSum = lCurRow;
//        lCurRow++;


        if (!aWithFeaturedPay) {
            query.prepare("SELECT PAY_DATE PAY_DATE_FOR_SORT, TO_CHAR(A.PAY_DATE, 'DD') PAY_DATE,"
                          " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                          " B.ID_PROJECT ID_PROJECT,"
                          " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                          " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                          " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                          " A.PAY_SUM_BRUTTO PAY_SUM, A.PAY_SUM_FULL PAY_SUM_FULL,"
                          " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_INDEXED, NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                          " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                          " WHERE TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = :year1"
                          " AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'MM')) = :month1"
                          " AND A.ID_CONTRACT = B.ID"
                          " AND C.ID_CONTRACT = B.ID"
                          " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                          " UNION"
                          " SELECT PAY_DATE PAY_DATE_FOR_SORT, TO_CHAR(A.PAY_DATE, 'DD') PAY_DATE,"
                          " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                          " B.ID_PROJECT ID_PROJECT,"
                          " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                          " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                          " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                          " A.PAY_SUM_BRUTTO PAY_SUM, A.PAY_SUM_FULL PAY_SUM_FULL,"
                          " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_INDEXED, NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                          " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C, V_PKZ_CONTRACT_STAGE D"
                          " WHERE TO_NUMBER(TO_CHAR(A.PAY_DATE, 'YYYY')) = :year2"
                          " AND TO_NUMBER(TO_CHAR(A.PAY_DATE, 'MM')) = :month2"
                          " AND D.ID_PKZ_CONTRACT = B.ID"
                          " AND A.ID_CONTRACT_STAGE = D.ID"
                          " AND C.ID_CONTRACT = B.ID"
                          " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                          " ORDER BY 1 DESC, 3 DESC, 4 DESC");
        } else {
            // with featured payments (field expect date)
            query.prepare("SELECT NVL(A.PAY_DATE, A.EXPECT_DATE) PAY_DATE_FOR_SORT, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD') PAY_DATE,"
                          " DECODE(A.PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                          " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                          " B.ID_PROJECT ID_PROJECT,"
                          " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                          " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                          " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                          " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM,"
                          " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                          " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_INDEXED,"
                          " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                          " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                          " WHERE TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = :year1"
                          " AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'MM')) = :month1"
                          " AND A.ID_CONTRACT = B.ID"
                          " AND C.ID_CONTRACT = B.ID"
                          " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                          " UNION"
                          " SELECT NVL(A.PAY_DATE, A.EXPECT_DATE) PAY_DATE_FOR_SORT, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD') PAY_DATE,"
                          " DECODE(A.PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                          " A.INVOICE INVOICE, A.PAY_INVOICE PAY_INVOICE,"
                          " B.ID_PROJECT ID_PROJECT,"
                          " PP.GETPROJECTSHORTNAME(B.ID_PROJECT) PROJ_NAME,"
                          " TRIM(B.NUM) NUM, TO_CHAR(B.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(B.END_DATE, 'DD.MM.YYYY') END_DATE, B.NAME NAME,"
                          " C.SUM_BRUTTO SUM, C.SUM_FULL SUM_FULL,"
                          " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM,"
                          " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                          " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_INDEXED,"
                          " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                          " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C, V_PKZ_CONTRACT_STAGE D"
                          " WHERE TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'YYYY')) = :year2"
                          " AND TO_NUMBER(TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'MM')) = :month2"
                          " AND D.ID_PKZ_CONTRACT = B.ID"
                          " AND A.ID_CONTRACT_STAGE = D.ID"
                          " AND C.ID_CONTRACT = B.ID"
                          " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)"
                          " ORDER BY 1 DESC, 3 DESC, 4 DESC");
        }

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, "חוזים", query);
        } else {
            query.bindValue(":year1", aYear);
            query.bindValue(":month1", i);
            query.bindValue(":year2", aYear);
            query.bindValue(":month2", i);
            if (!query.exec()) {
                gLogger->ShowSqlError(this, "חוזים", query);
            } else {
                bool lFirstPayInMonth = true;
                while (query.next()) {
                    if (!aProjects.isEmpty()
                            && !aProjects.contains(query.value("ID_PROJECT").toInt())) continue;// skipped by list
                    if (lFirstPayInMonth) {
                        lFirstPayInMonth = false;

                        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
                        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << gSettings->MonthName(i) << "\"\n";
                        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
                        //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
                        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
                        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Interior.Color = 7531363\n";

                        BorderWithStartEnd(9, 3);
                        lRowMonthSum = lCurRow;
                        lCurRow++;
                    }

                    out << "If Not nCols(1) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << query.value("PAY_DATE").toString() << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4108\n"; // center
                    out << "End If\n";

                    out << "If Not nCols(15) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).FormulaR1C1 = \"" << query.value("INVOICE").toString() << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(15).Column).HorizontalAlignment = -4108\n"; // center
                    out << "End If\n";

                    out << "If Not nCols(16) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).FormulaR1C1 = \"" << query.value("PAY_INVOICE").toString() << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(16).Column).HorizontalAlignment = -4108\n"; // center
                    out << "End If\n";

                    out << "If Not nCols(17) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(17).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(17).Column).FormulaR1C1 = \""
                        << query.value("PROJ_NAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(17).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(2) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).FormulaR1C1 = \""
                        << query.value("NUM").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(3) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).FormulaR1C1 = \""
                        << query.value("START_DATE").toString().replace("\"", "\"\"") << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).HorizontalAlignment = -4108\n"; // center
                    out << "End If\n";

                    out << "If Not nCols(4) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \""
                        << query.value("END_DATE").toString().replace("\"", "\"\"") << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).HorizontalAlignment = -4108\n"; // center
                    out << "End If\n";

                    out << "If Not nCols(5) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"@\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
                        << query.value("NAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).WrapText = False\n";
                    out << "End If\n";

                    out << "If Not nCols(6) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("SUM").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(7) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("SUM_FULL").toLongLong() - query.value("SUM").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(8) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(8).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(8).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("SUM_FULL").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(8).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";



                    out << "If Not nCols(9) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(9).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(9).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("PAY_SUM").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(9).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(10) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(10).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(10).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("PAY_SUM_FULL").toLongLong() - query.value("PAY_SUM").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(10).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(11) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(11).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(11).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("PAY_SUM_FULL").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(11).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";



                    out << "If Not nCols(12) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(12).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(12).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("PAY_SUM_INDEXED").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(12).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(13) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(13).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(13).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("PAY_SUM_FULL_INDEXED").toLongLong() - query.value("PAY_SUM_INDEXED").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(13).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    out << "If Not nCols(14) Is Nothing Then\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).FormulaR1C1 = \""
                        << gSettings->FormatSumForList(query.value("PAY_SUM_FULL_INDEXED").toLongLong()) << "\"\n";
                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(14).Column).HorizontalAlignment = -4152\n"; // right
                    out << "End If\n";

                    if (aWithFeaturedPay && query.value("PAY_DATE_ISNULL").toInt() == 1) {
                        out << "' color expected payments\n";
                        out << "For i = 9 To 14\n";
                        out << "  If Not nCols(i) Is Nothing Then\n";
                        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 1645055\n";
                        out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Font.Color = 16777215\n";
                        out << "  End If\n";
                        out << "Next\n";
                    }

                    Border;
                    lCurRow++;
                }

                if (!lFirstPayInMonth) {
                //if (lCurRow > lRowMonthSum + 1) {
                    // summary for month
                    out << "' summary for contract\n";
                    out << "For i = 9 To UBound(nCols) - 3\n";
                    out << "  If Not nCols(i) Is Nothing Then\n";
                    out << "    oWsheet.Cells(" << lRowMonthSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                    out << "    If nColsAny(i) Then\n";
                    out << "      oWsheet.Cells(" << lRowMonthSum << ", nCols(i).Column).Value = \"=SUM(\" + chr(nCols(i).Column + asc(\"A\") - 1) + \""
                        << lRowMonthSum + 1 << ":\" + chr(nCols(i).Column + asc(\"A\") - 1) + \"" << lCurRow - 1 << ")\"\n";
                    out << "    End If\n";
                    out << "    oWsheet.Cells(" << lRowMonthSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                    out << "    oWsheet.Cells(" << lRowMonthSum << ", nCols(i).Column).Font.Bold = True\n";
                    out << "  End If\n";
                    out << "Next\n";

                    forYearSum.append(lRowMonthSum);

                    out << "oWsheet.Rows(\"" << lRowMonthSum + 1 << ":" << lCurRow - 1 << "\").Group\n";
                }
            }
        }
    }

    if (!forYearSum.isEmpty()) {
        // summary for year
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
        out << "For i = 9 To UBound(nCols) - 3\n";
        out << "  If Not nCols(i) Is Nothing Then\n";
        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).Font.Bold = True\n";
        out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).Font.Italic = True\n";
        out << "  End If\n";
        out << "Next\n";
    }

    ReportEnd();
}

void ContractPkz::ReportFullOld(bool aWithFeaturedPay, const QList<int> &aProjects) {
    QString lVbsName, lTemplateName, lOutName, lTimeStamp;
    QDir dir;
    int k, lCurRow, lRowContractSum, lRowProjectSum;
    qlonglong color;

    if (aProjects.isEmpty()) {
        QMessageBox::critical(this, "Contracts - report", "Please select at least one project!");
        return;
    }

    QString aProjectsStr;

    for (int i = 0; i < aProjects.length(); i++) {
        if (aProjectsStr.isEmpty()) {
            aProjectsStr = QString::number(aProjects.at(i));
        } else {
            aProjectsStr += ", " + QString::number(aProjects.at(i));
        }
    }

    QSqlQuery queryProject(db), queryContractRest(db), queryContract(db), queryHashbon(db);

    // just so; min and max years need not but let it be
    if (!aWithFeaturedPay) {
        queryProject.prepare("SELECT A.ID ID, PP.GETPROJECTSHORTNAME(A.ID) PROJNAME,"
                             " LEAST("
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000),"
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000)) MIN1,"
                             " GREATEST("
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 0),"
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(PAY_DATE, 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 0)) MAX1,"
                             " (SELECT SUM(C.SUM_BRUTTO) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM,"
                             " (SELECT SUM(C.SUM_FULL) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM_FULL"
                             " FROM V_PROJECT A WHERE A.ID IN (" + aProjectsStr + ")"
                             " ORDER BY A.SHORTNAME");
    } else {
        queryProject.prepare("SELECT A.ID ID, PP.GETPROJECTSHORTNAME(A.ID) PROJNAME,"
                             " LEAST("
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000),"
                             "  NVL((SELECT MIN(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 5000)) MIN1,"
                             " GREATEST("
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_HASHBON C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND PAY_DATE IS NOT NULL), 0),"
                             "  NVL((SELECT MAX(TO_NUMBER(TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'YYYY'))) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C, V_PKZ_HASHBON D"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_PKZ_CONTRACT = B.ID"
                             "   AND D.ID_CONTRACT_STAGE = C.ID"
                             "   AND PAY_DATE IS NOT NULL), 0)) MAX1,"
                             " (SELECT SUM(C.SUM_BRUTTO) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM,"
                             " (SELECT SUM(C.SUM_FULL) FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_SUM C"
                             "   WHERE B.ID_PROJECT = A.ID"
                             "   AND C.ID_CONTRACT = B.ID"
                             "   AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = B.ID)) SUM_FULL"
                             " FROM V_PROJECT A WHERE A.ID IN (" + aProjectsStr + ")"
                             " ORDER BY A.SHORTNAME");
    }

    if (queryProject.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryProject);
        return;
    }

    // all contracts for selected projects
    if (!aWithFeaturedPay) {
        queryContract.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                              " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL"
                              " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                              " WHERE A.ID_PROJECT = :ID_PROJECT"
                              " AND B.ID_CONTRACT = A.ID"
                              " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                              " ORDER BY A.NUM");
    } else {
        queryContract.prepare("SELECT A.ID, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE,"
                              " NAME, B.SUM_BRUTTO SUM, B.SUM_FULL"
                              " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B"
                              " WHERE A.ID_PROJECT = :ID_PROJECT"
                              " AND B.ID_CONTRACT = A.ID"
                              " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                              " ORDER BY A.NUM");

    }
    if (queryContract.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryContract);
        return;
    }

    // rest by contracts; don't deicided yet
    if (!aWithFeaturedPay) {
        queryContractRest.prepare("SELECT A.ID, A.CUSTOMER, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE, A.NAME,"
                                  " B.SUM_BRUTTO SUM, B.SUM_FULL,"
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
                                  " ORDER BY A.NUM");
    } else {
        queryContractRest.prepare("SELECT A.ID, A.CUSTOMER, A.NUM, TO_CHAR(A.START_DATE, 'DD.MM.YYYY') START_DATE, TO_CHAR(A.END_DATE, 'DD.MM.YYYY') END_DATE, A.NAME,"
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
                                  " WHERE A.ID_PROJECT = :ID_PROJECT"
                                  " AND B.ID_CONTRACT = A.ID"
                                  " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                                  " AND B.SUM_BRUTTO -"
                                  "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                                  "        WHERE ID_CONTRACT = A.ID)"
                                  "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                                  "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) > 0"
                                  " ORDER BY A.NUM");

    }
    if (queryContractRest.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryContractRest);
        return;
    }

    // hashbons
    if (!aWithFeaturedPay) {
        queryHashbon.prepare("SELECT PAY_DATE, TO_CHAR(PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                             " ORIG_SUM_BRUTTO, ORIG_SUM_FULL,"
                             " NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO) ORIG_SUM_BRUTTO_INDEXED, NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL) ORIG_SUM_FULL_INDEXED,"

                             " SIGN_SUM_BRUTTO, SIGN_SUM_FULL,"
                             " NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO) SIGN_SUM_BRUTTO_INDEXED, NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL) SIGN_SUM_FULL_INDEXED,"

                             " PAY_SUM_BRUTTO, PAY_SUM_FULL,"
                             " NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                             " NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON"
                             " WHERE ID_CONTRACT = :ID_CONTRACT1"

                             " UNION"
                             " SELECT PAY_DATE, TO_CHAR(A.PAY_DATE, 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                             " A.ORIG_SUM_BRUTTO, A.ORIG_SUM_FULL,"
                             " A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_FULL_INDEXED,"

                             " A.SIGN_SUM_BRUTTO, A.SIGN_SUM_FULL,"
                             " A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_FULL_INDEXED,"

                             " A.PAY_SUM_BRUTTO, A.PAY_SUM_FULL,"
                             " NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO_INDEXED,"
                             " NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                             " WHERE B.ID_PKZ_CONTRACT = :ID_CONTRACT2"
                             " AND A.ID_CONTRACT_STAGE = B.ID"
                             " ORDER BY 1 DESC");
    } else {
        queryHashbon.prepare("SELECT PAY_DATE, TO_CHAR(NVL(PAY_DATE, EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, INVOICE, PAY_INVOICE,"
                             " ORIG_SUM_BRUTTO, ORIG_SUM_FULL,"
                             " NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO) ORIG_SUM_BRUTTO_INDEXED, NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL) ORIG_SUM_FULL_INDEXED,"

                             " SIGN_SUM_BRUTTO, SIGN_SUM_FULL,"
                             " NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO) SIGN_SUM_BRUTTO_INDEXED, NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL) SIGN_SUM_FULL_INDEXED,"

                             " DECODE(PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                             " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                             " DECODE(PAY_DATE, NULL, NVL(SIGN_SUM_FULL, ORIG_SUM_FULL), PAY_SUM_FULL) PAY_SUM_FULL,"
                             " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_BRUTTO), NVL(ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_BRUTTO)), NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                             " DECODE(PAY_DATE, NULL, NVL(NVL(SIGN_SUM_FULL_INDEXED, SIGN_SUM_FULL), NVL(ORIG_SUM_FULL_INDEXED, ORIG_SUM_FULL)), NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON"
                             " WHERE ID_CONTRACT = :ID_CONTRACT1"

                             " UNION"
                             " SELECT PAY_DATE, TO_CHAR(NVL(A.PAY_DATE, A.EXPECT_DATE), 'DD.MM.YYYY') PAY_DATE_STR, A.INVOICE, A.PAY_INVOICE,"
                             " A.ORIG_SUM_BRUTTO, A.ORIG_SUM_FULL,"
                             " A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_FULL_INDEXED,"

                             " A.SIGN_SUM_BRUTTO, A.SIGN_SUM_FULL,"
                             " A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_FULL_INDEXED,"

                             " DECODE(A.PAY_DATE, NULL, 1, 0) PAY_DATE_ISNULL,"
                             " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_BRUTTO, A.ORIG_SUM_BRUTTO), A.PAY_SUM_BRUTTO) PAY_SUM_BRUTTO,"
                             " DECODE(A.PAY_DATE, NULL, NVL(A.SIGN_SUM_FULL, A.ORIG_SUM_FULL), A.PAY_SUM_FULL) PAY_SUM_FULL,"
                             " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_BRUTTO_INDEXED, A.SIGN_SUM_BRUTTO), NVL(A.ORIG_SUM_BRUTTO_INDEXED, A.ORIG_SUM_BRUTTO)), NVL(A.PAY_SUM_BRUTTO_INDEXED, A.PAY_SUM_BRUTTO)) PAY_SUM_BRUTTO_INDEXED,"
                             " DECODE(A.PAY_DATE, NULL, NVL(NVL(A.SIGN_SUM_FULL_INDEXED, A.SIGN_SUM_FULL), NVL(A.ORIG_SUM_FULL_INDEXED, A.ORIG_SUM_FULL)), NVL(A.PAY_SUM_FULL_INDEXED, A.PAY_SUM_FULL)) PAY_SUM_FULL_INDEXED"
                             " FROM V_PKZ_HASHBON A, V_PKZ_CONTRACT_STAGE B"
                             " WHERE B.ID_PKZ_CONTRACT = :ID_CONTRACT2"
                             " AND A.ID_CONTRACT_STAGE = B.ID"
                             " ORDER BY 1 DESC");
    }

    if (queryHashbon.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", queryHashbon);
        return;
    }

    // queryProject is ready here
    if (!queryProject.exec()) {
        gLogger->ShowSqlError(this, "חוזים", queryProject);
        return;
    }

    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lTemplateName = lVbsName;

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportFull-" + lTimeStamp;
    dir.setPath(lVbsName);
    if (!dir.exists() && !dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, QObject::tr("Error"), QObject::tr("Can't create directory") + ": " + lVbsName);
        return;
    }

    lVbsName += "/ReportFull.vbs";
    lTemplateName += ReportFullTemplateOld;

    lOutName = "ReportFull-" + lTimeStamp + ".xls";

    ReportStart();

    lCurRow = 4;

    out << "Dim nCols(28)' as Range\n";
    out << "Dim nColsAny(28)\n";

    out << "Set nCols(0) = oWsheet.Cells.Find(\"#PROJECT#\")\n";
    out << "Set nCols(1) = oWsheet.Cells.Find(\"#NUM#\")\n";
    out << "Set nCols(2) = oWsheet.Cells.Find(\"#START_DATE#\")\n";
    out << "Set nCols(3) = oWsheet.Cells.Find(\"#END_DATE#\")\n";
    out << "Set nCols(4) = oWsheet.Cells.Find(\"#NAME#\")\n";
    out << "Set nCols(5) = oWsheet.Cells.Find(\"#SUM#\")\n";
    out << "Set nCols(6) = oWsheet.Cells.Find(\"#SUM_NDS#\")\n";
    out << "Set nCols(7) = oWsheet.Cells.Find(\"#SUM_FULL#\")\n";

    out << "Set nCols(8) = oWsheet.Cells.Find(\"#ORIG_SUM#\")\n";
    out << "Set nCols(9) = oWsheet.Cells.Find(\"#ORIG_SUM_NDS#\")\n";
    out << "Set nCols(10) = oWsheet.Cells.Find(\"#ORIG_SUM_FULL#\")\n";
    out << "Set nCols(11) = oWsheet.Cells.Find(\"#ORIG_SUM_IDX#\")\n";
    out << "Set nCols(12) = oWsheet.Cells.Find(\"#ORIG_SUM_NDS_IDX#\")\n";
    out << "Set nCols(13) = oWsheet.Cells.Find(\"#ORIG_SUM_FULL_IDX#\")\n";

    out << "Set nCols(14) = oWsheet.Cells.Find(\"#SIGN_SUM#\")\n";
    out << "Set nCols(15) = oWsheet.Cells.Find(\"#SIGN_SUM_NDS#\")\n";
    out << "Set nCols(16) = oWsheet.Cells.Find(\"#SIGN_SUM_FULL#\")\n";
    out << "Set nCols(17) = oWsheet.Cells.Find(\"#SIGN_SUM_IDX#\")\n";
    out << "Set nCols(18) = oWsheet.Cells.Find(\"#SIGN_SUM_NDS_IDX#\")\n";
    out << "Set nCols(19) = oWsheet.Cells.Find(\"#SIGN_SUM_FULL_IDX#\")\n";

    out << "Set nCols(20) = oWsheet.Cells.Find(\"#PAY_SUM#\")\n";
    out << "Set nCols(21) = oWsheet.Cells.Find(\"#PAY_SUM_NDS#\")\n";
    out << "Set nCols(22) = oWsheet.Cells.Find(\"#PAY_SUM_FULL#\")\n";
    out << "Set nCols(23) = oWsheet.Cells.Find(\"#PAY_SUM_IDX#\")\n";
    out << "Set nCols(24) = oWsheet.Cells.Find(\"#PAY_SUM_NDS_IDX#\")\n";
    out << "Set nCols(25) = oWsheet.Cells.Find(\"#PAY_SUM_FULL_IDX#\")\n";

    out << "Set nCols(26) = oWsheet.Cells.Find(\"#REST_THIS_YEAR#\")\n";
    out << "Set nCols(27) = oWsheet.Cells.Find(\"#REST_FUTURE#\")\n";
    out << "Set nCols(28) = oWsheet.Cells.Find(\"#REST_FULL#\")\n";

    out << "nColLast = 0\n";
    out << "For Each nCol in nCols\n";
    out << "  If Not nCol Is Nothing Then\n";
    out << "    nCol.Value = \"\"\n";
    out << "    If nCol.Column > nColLast Then\n";
    out << "      nColLast = nCol.Column\n";
    out << "    End If\n";
    out << "  End If\n";
    out << "Next\n";

    ColNumToStr(nColLast, nColLastStr)

    out << "For i = 0 To UBound(nCols)\n";
    out << "  nColsAny(i) = True\n";
    out << "Next\n";

    //MsgBox UBound(nCols)

    //Selection.Rows.Group


    QList<int> forProjectSum;

    while (queryProject.next()) {
        color = gSettings->Contract.ProjectColor.blue();
        color <<= 8;
        color += gSettings->Contract.ProjectColor.green();
        color <<= 8;
        color += gSettings->Contract.ProjectColor.red();

        forProjectSum.clear();

        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).NumberFormat = \"@\"\n";
        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).FormulaR1C1 = \"" << queryProject.value("PROJNAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
        out << "oWsheet.Cells(" << lCurRow << ", nCols(0).Column).HorizontalAlignment = -4152\n"; // right
        //out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + chr(nCols(0).Column + asc(\"A\") - 2 + nColLast) + \"" << lCurRow << "\").Merge\n";
        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + nColLastStr + \"" << lCurRow << "\").Borders(8).Weight = 2\n"; // xlEdgeTop, xlThin
        out << "oWsheet.Range(chr(nCols(0).Column + asc(\"A\") - 1) + \"" << lCurRow << ":\" + nColLastStr + \"" << lCurRow << "\").Interior.Color = " << QString::number(color) << "\n";

        out << "If Not nCols(5) Is Nothing Then\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
            << gSettings->FormatSumForList(queryProject.value("SUM").toLongLong()) << "\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Bold = True\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).Font.Italic = True\n";
        out << "End If\n";

        out << "If Not nCols(6) Is Nothing Then\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
            << gSettings->FormatSumForList(queryProject.value("SUM_FULL").toLongLong() - queryProject.value("SUM").toLongLong()) << "\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Bold = True\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).Font.Italic = True\n";
        out << "End If\n";

        out << "If Not nCols(7) Is Nothing Then\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
            << gSettings->FormatSumForList(queryProject.value("SUM_FULL").toLongLong()) << "\"\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).Font.Bold = True\n";
        out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).Font.Italic = True\n";
        out << "End If\n";

        lRowProjectSum = lCurRow;
        BorderWithStart(5);
        lCurRow++;

        // debt -----------------------------------------------------------------------------------------------------------------------------
        queryContractRest.bindValue(":ID_PROJECT", queryProject.value("ID"));
        queryContractRest.exec();

        if (queryContractRest.lastError().isValid()) {
            gLogger->ShowSqlError(this, "חוזים", queryContractRest);
        } else {
            while (queryContractRest.next()) {
                if (lCurRow == lRowProjectSum + 1) {
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
                    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 14540253\n";
                    out << "  End If\n";
                    out << "Next\n";

                    Border;
                    lCurRow++;
                }

                out << "If Not nCols(1) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << queryContractRest.value("NUM").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(2) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).FormulaR1C1 = \"" << queryContractRest.value("START_DATE").toString() << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(2).Column).HorizontalAlignment = -4108\n"; // center
                out << "End If\n";

                out << "If Not nCols(3) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).FormulaR1C1 = \"" << queryContractRest.value("END_DATE").toString() << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(3).Column).HorizontalAlignment = -4108\n"; // center
                out << "End If\n";

                out << "If Not nCols(4) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).NumberFormat = \"@\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).FormulaR1C1 = \"" << queryContractRest.value("NAME").toString().replace("\"", "\"\"").replace("\n", "\" & vbLf & \"") << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(4).Column).WrapText = False\n";
                out << "End If\n";

                out << "If Not nCols(5) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("SUM").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(5).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(6) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("SUM_FULL").toLongLong() - queryContractRest.value("SUM").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(6).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(7) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("SUM_FULL").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(7).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(26) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(26).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(26).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("REST_THIS_YEAR").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(26).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(27) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(27).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(27).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("REST_FULL").toLongLong() - queryContractRest.value("REST_THIS_YEAR").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(27).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";

                out << "If Not nCols(28) Is Nothing Then\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(28).Column).NumberFormat = \"#,##0.00\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(28).Column).FormulaR1C1 = \""
                    << gSettings->FormatSumForList(queryContractRest.value("REST_FULL").toLongLong()) << "\"\n";
                out << "  oWsheet.Cells(" << lCurRow << ", nCols(28).Column).HorizontalAlignment = -4152\n"; // right
                out << "End If\n";


                out << "For i = 1 To UBound(nCols)\n";
                out << "  If Not nCols(i) Is Nothing Then\n";
                out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Interior.Color = 14540253\n";
                out << "  End If\n";
                out << "Next\n";

                Border;
                lRowContractSum = lCurRow;
                lCurRow++;

            }

            // summary for debt
            out << "' summary for debt\n";
            out << "For i = 26 To 28\n";
            out << "  If Not nCols(i) Is Nothing Then\n";
            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
            out << "    If nColsAny(i) Then\n";

            if (lCurRow > lRowProjectSum + 1) {
                ColNumToStr(nCols(i).Column, Str1);
                out << "      oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Value = \"=SUM(\" + Str1 + \""
                    << lRowProjectSum + 2 << ":\" + Str1 + \"" << lCurRow - 1 << ")\"\n";
            } else {
                out << "      oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Value = 0\n";
            }

            out << "    End If\n";
            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
            out << "    oWsheet.Cells(" << lRowProjectSum << ", nCols(i).Column).Font.Bold = True\n";
            out << "  End If\n";
            out << "Next\n";

            if (lCurRow > lRowProjectSum + 1) {
                // bug: with + 2 empty string with string "Debt" not collapsed
                // it can be + 1 instead of + 2 but it is looking strange in excel
                out << "oWsheet.Rows(\"" << lRowProjectSum + 1 << ":" << lCurRow << "\").Group\n";
                out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
                lCurRow++;
            }
        }
        // end of debt -----------------------------------------------------------------------------------------------------------------------------
        //QList<int> forYearSum;

        queryContract.bindValue(":ID_PROJECT", queryProject.value("ID"));
        queryContract.exec();

        if (queryContract.lastError().isValid()) {
            gLogger->ShowSqlError(this, "חוזים", queryContract);
        } else {
            while (queryContract.next()) {
//                if (lCurRow == lRowYearSum) {
//                    out << "If Not nCols(1) Is Nothing Then\n";
//                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).NumberFormat = \"@\"\n";
//                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).FormulaR1C1 = \"" << QString::number(j) << "\"\n";
//                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).HorizontalAlignment = -4108\n"; // center
//                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).Font.Bold = True\n";
//                    out << "  oWsheet.Cells(" << lCurRow << ", nCols(1).Column).Font.Italic = True\n";
//                    out << "End If\n";

//                    Border;
//                    lCurRow++;
//                }

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

                queryHashbon.bindValue(":ID_CONTRACT1", queryContract.value("ID"));
                queryHashbon.bindValue(":ID_CONTRACT2", queryContract.value("ID"));

                queryHashbon.exec();

                if (queryHashbon.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "חוזים", queryHashbon);
                } else {
                    out << "lAnyOrig = False\n";
                    out << "lAnySign = False\n";
                    out << "lAnyPay = False\n";
                    while (queryHashbon.next()) {
                        qlonglong lOrigSum, lOrigSumFull, lOrigSumIdx, lOrigSumFullIdx;
                        qlonglong lSignSum, lSignSumFull, lSignSumIdx, lSignSumFullIdx;
                        qlonglong lPaySum, lPaySumFull, lPaySumIdx, lPaySumFullIdx;

                        lOrigSum = queryHashbon.value("ORIG_SUM_BRUTTO").toLongLong();
                        lOrigSumFull = queryHashbon.value("ORIG_SUM_FULL").toLongLong();
                        lOrigSumIdx = queryHashbon.value("ORIG_SUM_BRUTTO_INDEXED").toLongLong();
                        lOrigSumFullIdx = queryHashbon.value("ORIG_SUM_FULL_INDEXED").toLongLong();

                        lSignSum = queryHashbon.value("SIGN_SUM_BRUTTO").toLongLong();
                        lSignSumFull = queryHashbon.value("SIGN_SUM_FULL").toLongLong();
                        lSignSumIdx = queryHashbon.value("SIGN_SUM_BRUTTO_INDEXED").toLongLong();
                        lSignSumFullIdx = queryHashbon.value("SIGN_SUM_FULL_INDEXED").toLongLong();

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

#define OutSum(Name, StartCol) \
                        out << "If Not nCols(" << (StartCol + 0) << ") Is Nothing Then\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 0) << ").Column).NumberFormat = \"#,##0.00\"\n"; \
                        if (l##Name##Sum) { \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 0) << ").Column).FormulaR1C1 = \"" \
                                << gSettings->FormatSumForList(l##Name##Sum) << "\"\n"; \
                            out << "  nColsAny(" << (StartCol + 0) << ") = True\n"; \
                            out << "  lAny"#Name" = True\n"; \
                        } else \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 0) << ").Column).FormulaR1C1 = \"\"\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 0) << ").Column).HorizontalAlignment = -4152\n"; \
                        out << "End If\n"; \
\
                        out << "If Not nCols(" << (StartCol + 1) << ") Is Nothing Then\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 1) << ").Column).NumberFormat = \"#,##0.00\"\n"; \
                        if (l##Name##SumFull && l##Name##Sum) { \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 1) << ").Column).FormulaR1C1 = \"" \
                                << gSettings->FormatSumForList(l##Name##SumFull - l##Name##Sum) << "\"\n"; \
                            out << "  nColsAny(" << (StartCol + 1) << ") = True\n"; \
                        } else \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 1) << ").Column).FormulaR1C1 = \"\"\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 1) << ").Column).HorizontalAlignment = -4152\n"; \
                        out << "End If\n"; \
\
                        out << "If Not nCols(" << (StartCol + 2) << ") Is Nothing Then\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 2) << ").Column).NumberFormat = \"#,##0.00\"\n"; \
                        if (l##Name##SumFull) { \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 2) << ").Column).FormulaR1C1 = \"" \
                                << gSettings->FormatSumForList(l##Name##SumFull) << "\"\n"; \
                            out << "  nColsAny(" << (StartCol + 2) << ") = True\n"; \
                        } else \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 2) << ").Column).FormulaR1C1 = \"\"\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 2) << ").Column).HorizontalAlignment = -4152\n"; \
                        out << "End If\n"; \
\
                        out << "If Not nCols(" << (StartCol + 3) << ") Is Nothing Then\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 3) << ").Column).NumberFormat = \"#,##0.00\"\n"; \
                        if (l##Name##SumIdx) { \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 3) << ").Column).FormulaR1C1 = \"" \
                                << gSettings->FormatSumForList(l##Name##SumIdx) << "\"\n"; \
                            out << "  nColsAny(" << (StartCol + 3) << ") = True\n"; \
                        } else \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 3) << ").Column).FormulaR1C1 = \"\"\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 3) << ").Column).HorizontalAlignment = -4152\n"; \
                        out << "End If\n"; \
\
                        out << "If Not nCols(" << (StartCol + 4) << ") Is Nothing Then\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 4) << ").Column).NumberFormat = \"#,##0.00\"\n"; \
                        if (l##Name##SumFullIdx && l##Name##SumIdx) { \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 4) << ").Column).FormulaR1C1 = \"" \
                                << gSettings->FormatSumForList(l##Name##SumFullIdx - l##Name##SumIdx) << "\"\n"; \
                            out << "  nColsAny(" << (StartCol + 4) << ") = True\n"; \
                        } else \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 4) << ").Column).FormulaR1C1 = \"\"\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 4) << ").Column).HorizontalAlignment = -4152\n"; \
                        out << "End If\n"; \
\
                        out << "If Not nCols(" << (StartCol + 5) << ") Is Nothing Then\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 5) << ").Column).NumberFormat = \"#,##0.00\"\n"; \
                        if (l##Name##SumFullIdx) { \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 5) << ").Column).FormulaR1C1 = \"" \
                                << gSettings->FormatSumForList(l##Name##SumFullIdx) << "\"\n"; \
                            out << "  nColsAny(" << (StartCol + 5) << ") = True\n"; \
                        } else \
                            out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 5) << ").Column).FormulaR1C1 = \"\"\n"; \
                        out << "  oWsheet.Cells(" << lCurRow << ", nCols(" << (StartCol + 5) << ").Column).HorizontalAlignment = -4152\n"; \
                        out << "End If\n";

                        OutSum(Orig, 8)
                        OutSum(Sign, 14)
                        OutSum(Pay, 20)
#undef OutSum



                        if (aWithFeaturedPay) {
                            if (queryHashbon.value("PAY_DATE_ISNULL").toInt() == 1) {
                                out << "' color expected payments\n";
                                out << "For i = 20 To 25\n";
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
                        out << "  If i >= 8 And i < 14 And lAnyOrig Or i >= 14 And i < 20 And lAnySign Or i >= 20 And lAnyPay Then\n";
                        out << "    If Not nCols(i) Is Nothing Then\n";
                        out << "      oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
                        out << "      If nColsAny(i) Then\n";
                        out << "        oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).Value = \"=SUM(\" + chr(nCols(i).Column + asc(\"A\") - 1) + \""
                            << lRowContractSum + 1 << ":\" + chr(nCols(i).Column + asc(\"A\") - 1) + \"" << lCurRow - 1 << ")\"\n";
                        out << "      End If\n";
                        out << "      oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
                        out << "      oWsheet.Cells(" << lRowContractSum << ", nCols(i).Column).Font.Bold = True\n";
                        out << "    End If\n";
                        out << "  End If\n";
                        out << "Next\n";

                        out << "If Not nCols(28) Is Nothing And Not nCols(5) Is Nothing And Not nCols(20) Is Nothing Then\n";
                        out << "  oWsheet.Cells(" << lRowContractSum << ", nCols(28).Column).NumberFormat = \"#,##0.00\"\n";

                        ColNumToStr(nCols(5).Column, lStr1)
                        ColNumToStr(nCols(20).Column, lStr2)

                        //out << "  oWsheet.Cells(" << lRowContractSum << ", nCols(28).Column).Value = \"=\" + lStr1 + \"" << lRowContractSum << "- \" + lStr2 & " << lRowContractSum << "\"\n";
                        out << "  oWsheet.Cells(" << lRowContractSum << ", nCols(28).Column).Value = \"=\" + lStr1 + \"" << lRowContractSum << "-\" + lStr2 + \"" << lRowContractSum << "\"\n";

                        out << "End If\n";

                        //forYearSum.append(lRowContractSum);
                        forProjectSum.append(lRowContractSum);

                        out << "oWsheet.Rows(\"" << lRowContractSum + 1 << ":" << lCurRow - 1 << "\").Group\n";
                    }
                }
            }


//            if (!forYearSum.isEmpty()) {
//                // summary for year
//                forProjectSum.append(lRowYearSum);
//                QString cellList;
//                cellList = "SUM(";
//                for (k = 0; k < forYearSum.length(); k++) {
//                    if (k) {
//                        if (!(k % 24)) cellList += "),SUM(";
//                        else cellList += ",";
//                    }
//                    cellList += "R[" + QString::number(forYearSum.at(k) - lRowYearSum) + "]C[0]";
//                }

//                cellList += ")";

//                out << "' summary for year\n";
//                out << "For i = 8 To UBound(nCols) - 3\n";
//                out << "  If Not nCols(i) Is Nothing Then\n";
//                out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).NumberFormat = \"#,##0.00\"\n";
//                out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).FormulaR1C1 = \"=SUM(" << cellList << ")\"\n";
//                out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).HorizontalAlignment = -4152\n"; // right
//                out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).Font.Bold = True\n";
//                out << "    oWsheet.Cells(" << lRowYearSum << ", nCols(i).Column).Font.Italic = True\n";
//                out << "  End If\n";
//                out << "Next\n";

//                out << "oWsheet.Rows(\"" << lRowYearSum + 1 << ":" << lCurRow << "\").Group\n";

//                out << "oWsheet.Rows(" << lCurRow << ").RowHeight = 5\n";
//                lCurRow++;
//            }
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
        }
        out << "oWsheet.Rows(\"" << lRowProjectSum + 1 << ":" << lCurRow << "\").Group\n";
        lCurRow++;
    }
    ReportEnd();
}
