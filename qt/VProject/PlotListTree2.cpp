#include "PlotListTree.h"
#include "DocListSettingsDlg.h"
#include "BlobMemCache.h"

#include "common.h"

#include <QDir>

#include "../UsersDlg/UserData.h"
#include "../ProjectLib/ProjectData.h"

void PlotListTree::MakeXLS() {
    QList<QTreeWidgetItem *> lSelected  = selectedItems();

    if (lSelected.isEmpty()) return;

    ProjectData * lProject = gProjects->FindByIdProject(static_cast<PlotListTreeItem *>(lSelected[0])->PlotConst()->IdProject());

    DocListSettingsDlg ws(lProject, this);
    if (ws.exec() != QDialog::Accepted) return;

    QString lVbsName, lOutName, lTimeStamp;
    QDir dir;

    lVbsName = QCoreApplication::applicationDirPath();

    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));

    lTimeStamp = (QDateTime::currentDateTime()).toString("yyMMdd-hhmmss");
    lVbsName += "/temp/data/ReportDocList-" + lTimeStamp;

    if (!dir.mkpath(lVbsName)) {
        gLogger->ShowError(this, tr("Documents - make XLS"), tr("Can't create directory") + ": " + lVbsName);
        return;
    }

//    gLogger->ShowError(this, tr("Documents - make XLS"), tr("lVbsName") + ": " + lVbsName);
//    gLogger->ShowError(this, tr("Documents - make XLS"), tr("lTemplateName") + ": " + lTemplateName);
//    gLogger->ShowError(this, tr("Documents - make XLS"), tr("lOutName") + ": " + lOutName);

    if (!lProject) {
        gLogger->ShowError(this, tr("Documents - make XLS"), tr("Invalid data, project not found"));
        return;
    }

    lOutName = "DocList-" + lTimeStamp + ".xls";

    // save template to disk
    QFile lFile(lVbsName + "/temp-" + lOutName);
    if (lFile.open(QFile::WriteOnly)) {

        PlotData * lPlot = new PlotData(ws.IdTemplate());
        lPlot->InitIdDwgMax();
        lFile.write(gBlobMemCache->GetData(BlobMemCache::Dwg, lPlot->IdDwgMax()));
        delete lPlot;

        lFile.flush();
        lFile.close();
    } else {
        gLogger->ShowError(this, tr("Documents - make XLS"), tr("Error creating file") + "\r\n" + lFile.fileName() + "\r\n" + lFile.errorString());
        return;
    }

    if (!lFile.size()) {
        gLogger->ShowError(this, tr("Documents - make XLS"), tr("Error reading template"));
        return;
    }

    lVbsName += "/DocList.vbs";

    QFile lVbsFile(lVbsName);
    if (lVbsFile.open(QFile::WriteOnly)) {
        QTextStream out(&lVbsFile);

        int i;
        QList<tPlotAndTreeData> lDocs;

        for (i = 0; i < lSelected.length(); i++) {
            PlotData * lPlot = static_cast<PlotListTreeItem *>(lSelected[i])->PlotRef();
            TreeDataRecord * lTree = gTreeData->FindById(lPlot->TDArea(), lPlot->TDId());
            TreeDataRecord * lTreePrev = lTree;
            while (true) {
                if (lTree->IdGroup() == 1) {
                    lTree = lTreePrev;
                    break;
                }
                if (!lTree->Parent()) {
                    lTree = NULL;
                    break;
                }
                lTreePrev = lTree;
                lTree = lTree->Parent();
            }
            lDocs.append(qMakePair(lPlot, lTree));
        }

        std::sort(lDocs.begin(), lDocs.end(),
              [] (const tPlotAndTreeData & d1, const tPlotAndTreeData & d2)
        {
            if (d1.second && !d2.second) return true;
            if (!d1.second && d2.second) return false;
            if (d1.second && d2.second && d1.second->OrderBy() != d2.second->OrderBy()) return d1.second->OrderBy() < d2.second->OrderBy();
            if (!d1.second && !d2.second || d1.second && d2.second && d1.second->OrderBy() == d2.second->OrderBy()) {
                if (d1.first->CodeConst() == d2.first->CodeConst()) return CmpStringsWithNumbersNoCase(d1.first->SheetConst(), d2.first->SheetConst());
                return CmpStringsWithNumbersNoCase(d1.first->CodeConst(), d2.first->CodeConst());
            }
            return true;  // it is dummy, never called part; just for disable warning
        });

        out.setCodec("UTF-16LE");
        out <<
               "sGetPath = Left(Wscript.ScriptFullName, InStrRev(Wscript.ScriptFullName, \"\\\"))\r\n"
               "Set oExcel = CreateObject(\"Excel.Application\")\r\n"
               "'oExcel.visible = true\r\n"
               "Set oWbook = oExcel.Workbooks.Open(sGetPath & \"temp-\" & \"" + lOutName + "\")\r\n"
               "Set oWsheet = oWbook.Worksheets(1)\r\n";

        out << "Dim nCols(21)' as Range\r\n";

        out << "Set nCols(0) = oWsheet.Cells.Find(\"#PROJECT#\")\r\n";
        out << "Set nCols(1) = oWsheet.Cells.Find(\"#CONSTRUCTION#\")\r\n";
        out << "Set nCols(2) = oWsheet.Cells.Find(\"#USER_TOP#\")\r\n";
        out << "Set nCols(3) = oWsheet.Cells.Find(\"#DATE_TOP#\")\r\n";
        out << "Set nCols(4) = oWsheet.Cells.Find(\"#USER_BOTTOM#\")\r\n";
        out << "Set nCols(5) = oWsheet.Cells.Find(\"#DATE_BOTTOM#\")\r\n";
        out << "Set nCols(6) = oWsheet.Cells.Find(\"#TYPE#\")\r\n";
        out << "Set nCols(7) = oWsheet.Cells.Find(\"#N#\")\r\n";
        out << "Set nCols(8) = oWsheet.Cells.Find(\"#ID#\")\r\n";
        out << "Set nCols(9) = oWsheet.Cells.Find(\"#NAMETOP#\")\r\n";
        out << "Set nCols(10) = oWsheet.Cells.Find(\"#NAME#\")\r\n";
        out << "Set nCols(11) = oWsheet.Cells.Find(\"#CODE#\")\r\n";
        out << "Set nCols(12) = oWsheet.Cells.Find(\"#SH#\")\r\n";
        out << "Set nCols(13) = oWsheet.Cells.Find(\"#VERSION#\")\r\n";
        out << "Set nCols(14) = oWsheet.Cells.Find(\"#CANCELLED#\")\r\n";
        out << "Set nCols(15) = oWsheet.Cells.Find(\"#ST#\")\r\n";
        out << "Set nCols(16) = oWsheet.Cells.Find(\"#SENTDATE#\")\r\n";
        out << "Set nCols(17) = oWsheet.Cells.Find(\"#CRDATE#\")\r\n";
        out << "Set nCols(18) = oWsheet.Cells.Find(\"#CRUSER#\")\r\n";
        out << "Set nCols(19) = oWsheet.Cells.Find(\"#CHDATE#\")\r\n";
        out << "Set nCols(20) = oWsheet.Cells.Find(\"#CHUSER#\")\r\n";
        out << "Set nCols(21) = oWsheet.Cells.Find(\"#COMMENTS#\")\r\n";


        out << "If Not nCols(0) Is Nothing Then\r\n";
        out << "  oWsheet.Cells(nCols(0).Row, nCols(0).Column).FormulaR1C1 = \""
            << lProject->NameConst().trimmed().replace("\"", "\"\"")
               .replace("\n\r", "\" & vbLf & \"")
               .replace("\r\n", "\" & vbLf & \"")
               .replace("\n", "\" & vbLf & \"")
               .replace("\r", "\" & vbLf & \"")
            << "\"\r\n";
        out << "End If\r\n";

        out << "If Not nCols(1) Is Nothing Then\r\n";
        out << "  oWsheet.Cells(nCols(1).Row, nCols(1).Column).FormulaR1C1 = \""
            << lProject->ShortNameConst().trimmed().replace("\"", "\"\"")
               .replace("\n\r", "\" & vbLf & \"")
               .replace("\r\n", "\" & vbLf & \"")
               .replace("\n", "\" & vbLf & \"")
               .replace("\r", "\" & vbLf & \"")
            << "\"\r\n";
        out << "End If\r\n";

        out << "If Not nCols(2) Is Nothing Then\r\n";
        out << "  oWsheet.Cells(nCols(2).Row, nCols(2).Column).FormulaR1C1 = \"" << gUsers->GetName(db.userName()) << "\"\r\n";
        out << "End If\r\n";

        out << "If Not nCols(3) Is Nothing Then\r\n";
        out << "  oWsheet.Cells(nCols(3).Row, nCols(3).Column).FormulaR1C1 = \"" << (QDateTime::currentDateTime()).toString("dd.MM.yy") << "\"\r\n";
        out << "End If\r\n";

        out << "If Not nCols(4) Is Nothing Then\r\n";
        out << "  oWsheet.Cells(nCols(4).Row, nCols(4).Column).FormulaR1C1 = \"" << gUsers->GetName(db.userName()) << "\"\r\n";
        out << "End If\r\n";

        out << "If Not nCols(5) Is Nothing Then\r\n";
        out << "  oWsheet.Cells(nCols(5).Row, nCols(5).Column).FormulaR1C1 = \"" << (QDateTime::currentDateTime()).toString("dd.MM.yy") << "\"\r\n";
        out << "End If\r\n";


        out << "For i = 7 To UBound(nCols)\r\n";
        out << "  If Not nCols(i) Is Nothing Then\r\n";
        out << "    StartRow = nCols(i).Row\r\n";
        out << "    CurRow = nCols(i).Row\r\n";
        out << "    Exit For\r\n";
        out << "  End If\r\n";
        out << "Next\r\n";

        int lPrevArea = -1, lPrevId = -1;
        bool lPrevWasNull = false, lIsFirst = true;

        i = 1;
        foreach (tPlotAndTreeData lPlotAndTreeData, lDocs) {
            if (!lPlotAndTreeData.second
                    && !lPrevWasNull
                    || lPlotAndTreeData.second
                    && (lPlotAndTreeData.second->Area() != lPrevArea
                        || lPlotAndTreeData.second->Id() != lPrevId)) {
                if (lPrevArea == -1) {
                    out << "If Not nCols(6) Is Nothing Then\r\n";
                    if (lPlotAndTreeData.second)
                        out << "  oWsheet.Cells(nCols(6).Row, nCols(6).Column).FormulaR1C1 = \"" << lPlotAndTreeData.second->TextConst() << "\"\r\n";
                    else
                        out << "  oWsheet.Cells(nCols(6).Row, nCols(6).Column).FormulaR1C1 = \"Other\"\r\n";
                    out << "End If\r\n";
                } else {
                    out << "If Not nCols(6) Is Nothing Then\r\n";
                    out << "  oWsheet.Rows(CurRow).Insert\r\n";
                    out << "  oWsheet.Rows(nCols(6).Row).Copy(oWsheet.Rows(CurRow))\r\n";
                    if (lPlotAndTreeData.second)
                        out << "  oWsheet.Cells(CurRow, nCols(6).Column).FormulaR1C1 = \"" << lPlotAndTreeData.second->TextConst() << "\"\r\n";
                    else
                        out << "  oWsheet.Cells(CurRow, nCols(6).Column).FormulaR1C1 = \"Other\"\r\n";
                    out << "  CurRow = CurRow + 1\r\n";
                    out << "End If\r\n";
                }
                if (!lPlotAndTreeData.second) {
                    lPrevWasNull = true;
                } else {
                    lPrevArea = lPlotAndTreeData.second->Area();
                    lPrevId = lPlotAndTreeData.second->Id();
                }
            }

            if (lIsFirst) {
                lIsFirst = false;
            } else {
                out << "oWsheet.Rows(CurRow).Insert\r\n";
                out << "oWsheet.Rows(StartRow).Copy(oWsheet.Rows(CurRow))\r\n";
            }

            out << "If Not nCols(7) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(7).Column).FormulaR1C1 = " + QString::number(i) + "\r\n";
            out << "End If\r\n";

            out << "If Not nCols(8) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(8).Column).FormulaR1C1 = " + QString::number(lPlotAndTreeData.first->Id()) + "\r\n";
            out << "End If\r\n";

            out << "If Not nCols(9) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(9).Column).FormulaR1C1 = \""
                   + lPlotAndTreeData.first->NameTopConst().trimmed().replace("\"", "\"\"")
                   .replace("\n\r", "\" & vbLf & \"")
                   .replace("\r\n", "\" & vbLf & \"")
                   .replace("\n", "\" & vbLf & \"")
                   .replace("\r", "\" & vbLf & \"") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(10) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(10).Column).FormulaR1C1 = \""
                   + lPlotAndTreeData.first->NameConst().trimmed().replace("\"", "\"\"")
                   .replace("\n\r", "\" & vbLf & \"")
                   .replace("\r\n", "\" & vbLf & \"")
                   .replace("\n", "\" & vbLf & \"")
                   .replace("\r", "\" & vbLf & \"") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(11) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(11).Column).FormulaR1C1 = \""
                   + lPlotAndTreeData.first->CodeConst().trimmed().replace("\"", "\"\"")
                   .replace("\n\r", "\" & vbLf & \"")
                   .replace("\r\n", "\" & vbLf & \"")
                   .replace("\n", "\" & vbLf & \"")
                   .replace("\r", "\" & vbLf & \"") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(12) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(12).Column).FormulaR1C1 = \""
                   + lPlotAndTreeData.first->SheetConst().trimmed().replace("\"", "\"\"")
                   .replace("\n\r", "\" & vbLf & \"")
                   .replace("\r\n", "\" & vbLf & \"")
                   .replace("\n", "\" & vbLf & \"")
                   .replace("\r", "\" & vbLf & \"") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(13) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(13).Column).FormulaR1C1 = \""
                   + lPlotAndTreeData.first->VersionExtConst().trimmed().replace("\"", "\"\"")
                   .replace("\n\r", "\" & vbLf & \"")
                   .replace("\r\n", "\" & vbLf & \"")
                   .replace("\n", "\" & vbLf & \"")
                   .replace("\r", "\" & vbLf & \"") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(14) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(14).Column).FormulaR1C1 = " + QString::number(lPlotAndTreeData.first->Cancelled()) + "\r\n";
            out << "End If\r\n";

            out << "If Not nCols(15) Is Nothing Then\r\n";
            if (lPlotAndTreeData.first->Cancelled()) {
                out << "  oWsheet.Cells(CurRow, nCols(15).Column).FormulaR1C1 = \"C\"\r\n";
                out << "  oWsheet.Cells(CurRow, nCols(15).Column).Interior.COlorIndex = 15\n";
            } else {
                out << "  oWsheet.Cells(CurRow, nCols(15).Column).FormulaR1C1 = \"W\"\r\n";
            }
            out << "End If\n";

            out << "If Not nCols(16) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(16).Column).FormulaR1C1 = \"" + lPlotAndTreeData.first->SentDateConst().toString("dd.MM.yy") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(17) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(17).Column).FormulaR1C1 = \"" + lPlotAndTreeData.first->CrDateConst().toString("dd.MM.yy") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(18) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(18).Column).FormulaR1C1 = \"" + gUsers->GetName(lPlotAndTreeData.first->CrUserConst()) + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(19) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(19).Column).FormulaR1C1 = \"" + lPlotAndTreeData.first->EditDateConst().toString("dd.MM.yy") + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(20) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(20).Column).FormulaR1C1 = \"" + gUsers->GetName(lPlotAndTreeData.first->EditUserConst()) + "\"\r\n";
            out << "End If\r\n";

            out << "If Not nCols(21) Is Nothing Then\r\n";
            out << "  oWsheet.Cells(CurRow, nCols(21).Column).FormulaR1C1 = \""
                   + lPlotAndTreeData.first->NotesConst().trimmed().replace("\"", "\"\"")
                   .replace("\n\r", "\" & vbLf & \"")
                   .replace("\r\n", "\" & vbLf & \"")
                   .replace("\n", "\" & vbLf & \"")
                   .replace("\r", "\" & vbLf & \"") + "\"\r\n";
            out << "End If\r\n";

            out << "CurRow = CurRow + 1\r\n";
            i++;
        }

        out << "oWbook.SaveAs(sGetPath & \"" + lOutName + "\")\r\n"
               "oWbook.Close True\r\n"
               "oExcel.Quit\r\n";

        lVbsFile.flush();
        lVbsFile.close();
    } else {
        gLogger->ShowError(this, tr("Saving additional files"), tr("Error creating file") + "\r\n" + lVbsFile.fileName() + "\r\n" + lVbsFile.errorString());
    }

    RunAndShowReport(lVbsName, lOutName);
    lVbsName.resize(lVbsName.lastIndexOf(QChar('/')));
    QFile::remove(lVbsName + "/temp-" + lOutName);
}
