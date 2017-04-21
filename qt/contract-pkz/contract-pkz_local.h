#ifndef CONTRACTPKZ_LOCAL
#define CONTRACTPKZ_LOCAL

#define BorderWithStartEnd(aColStart, aColEnd) { \
    out << "For i = " << aColStart << " To UBound(nCols) - " << aColEnd << "\n"; \
    out << "  If Not nCols(i) Is Nothing Then\n"; \
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Borders(7).Weight = 2\n"; /* xlEdgeLeft, xlThin */ \
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Borders(8).Weight = 2\n"; /* xlEdgeTop, xlThin */ \
    out << "  End If\n"; \
    out << "Next\n";}

#define BorderWithStart(aColStart) { \
    out << "For i = " << aColStart << " To UBound(nCols)\n"; \
    out << "  If Not nCols(i) Is Nothing Then\n"; \
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Borders(7).Weight = 2\n"; /* xlEdgeLeft, xlThin */ \
    out << "    oWsheet.Cells(" << lCurRow << ", nCols(i).Column).Borders(8).Weight = 2\n"; /* xlEdgeTop, xlThin */ \
    out << "  End If\n"; \
    out << "Next\n";}

#define Border BorderWithStart(1)

#define ColNumToStr(aColNumName, AColNunNameStr) { \
    out << "If ("#aColNumName") < 27 Then\n"; \
    out << "  "#AColNunNameStr" = chr(nCols(0).Column + asc(\"A\") - 2 + ("#aColNumName"))\n"; \
    out << "Else\n"; \
    out << "  "#AColNunNameStr" = \"A\" + chr(nCols(0).Column + asc(\"A\") - 2 + ("#aColNumName") - 26)\n"; \
    out << "End If\n";}


#define ReportStart() \
    QFile data(lVbsName); \
    if (data.open(QFile::WriteOnly)) { \
    QTextStream out(&data); \
    out.setCodec("UTF-16LE"); \
    out << "On Error Resume Next\n" \
    "sGetPath = Left(Wscript.ScriptFullName, InStrRev(Wscript.ScriptFullName, \"\\\"))\n" \
    "Set oExcel = CreateObject(\"Excel.Application\")\n" \
    "'oExcel.visible = true\n" \
    "Set oWbook = oExcel.Workbooks.Open(\"" + lTemplateName + "\")\n" \
    "oWbook.SaveAs(sGetPath & \"temp-" + lOutName + "\")\n" \
    "Set oWsheet = oWbook.Worksheets(1)\n";

#define ReportEnd() \
    out << "If Err.number <> 0 Then\n" \
    "  MsgBox Err.source & \" - \" & Err.number & \": \" & Err.description\n" \
    "  oWbook.SaveAs(sGetPath & \"error.xls\")\n" \
    "Else" \
    "  oWbook.SaveAs(sGetPath & \"" + lOutName + "\")\n" \
    "End If\n" \
    "oWbook.Close True\n" \
    "oExcel.Quit\n"; \
    data.flush(); \
    data.close(); \
    RunAndShowReport(lVbsName, lOutName); \
    QFile::remove(lVbsName + "/temp-" + lOutName); \
    } else { \
    gLogger->ShowError(this, "חוזים", "Can't create VBS file!"); \
    }

#endif // CONTRACTPKZ_LOCAL

