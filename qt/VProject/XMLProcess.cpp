#include "XMLProcess.h"

#include <QtXml/QXmlReader>
#include <QXmlStreamWriter>
#include <QProcess>

#include "common.h"
#include "GlobalSettings.h"

#include "../Logger/logger.h"

//--------------------------------------------------------------------------------------------------------------------
QQueryForExcelDataBind::QQueryForExcelDataBind(const QString &aName) :
    mName(aName), mType(TypeParent)
{

}

QQueryForExcelDataBind::QQueryForExcelDataBind(const QString &aName, const QVariant &aValue) :
    mName(aName), mType(TypeValue), mValue(aValue)
{

}

//--------------------------------------------------------------------------------------------------------------------
QQueryForExcelDataField::QQueryForExcelDataField(const QString &aName, enumFormat aFormatType, const QVariant &aNullValue) :
    mName(aName), mType(TypeBase), mOperField1(-1), mOperField2(-1),
    mFormatType(aFormatType),
    mFoundRowIndex(-1), mFoundColIndex(-1),
    mNullValue(aNullValue), mChangeStyleIdCallback(NULL),
    mGetValueCallback(NULL), mGetValueCallbackDataPtr(NULL)
{

}

QQueryForExcelDataField::QQueryForExcelDataField(const QString &aName, enumType aType,
                                                 int aOperField1, int aOperField2, enumFormat aFormatType, const QVariant &aNullValue) :
    mName(aName), mType(aType), mOperField1(aOperField1), mOperField2(aOperField2),
    mFormatType(aFormatType),
    mFoundRowIndex(-1), mFoundColIndex(-1),
    mNullValue(aNullValue), mChangeStyleIdCallback(NULL),
    mGetValueCallback(NULL), mGetValueCallbackDataPtr(NULL)
{

}

void QQueryForExcelDataField::SetChangeStyleIdCallback(void (*aChangeStyleIdCallback) (const QSqlQuery *, const QString &, QString &)) {
    mChangeStyleIdCallback = aChangeStyleIdCallback;
}

void QQueryForExcelDataField::SetGetValueCallback(void (*aGetValueCallback)(const QSqlQuery *, const QString &, int, QVariant &, void *)) {
    mGetValueCallback = aGetValueCallback;
}

void QQueryForExcelDataField::SetGetValueCallbackDataPtr(void *aDataPtr) {
    mGetValueCallbackDataPtr = aDataPtr;
}

//--------------------------------------------------------------------------------------------------------------------
QQueryForExcelData::QQueryForExcelData(const QString &aSqlText, const QList<QQueryForExcelDataField *> &aFields) :
    mFields(aFields), mQuery(new QSqlQuery(db)),
    mHasTotal(false), mSkipTotalIfOneRecord(false), mSkipAfterIfOneRecord(false), mError(false),
    mAllIsFoundInCur(false), mRecordCount(0),
    mDoGrouping(false), mDoDeleteIfNoChildren(false)
{
    mQuery->prepare(aSqlText);
    if (aSqlText.isEmpty()) mRecordCount = 1;
    if (mQuery->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Report builder - prepare"), *mQuery);
        mError = true;
    }

    for (int i = 0; i < mFields.length(); i++) {
        if (mFields.at(i)->mType == QQueryForExcelDataField::TypeChildSum) {
            if (!mQueriesWithSum.contains(mFields.at(i)->mOperField1)) mQueriesWithSum.append(mFields.at(i)->mOperField1);
        } else if (mFields.at(i)->mType == QQueryForExcelDataField::TypeTotal) {
            mHasTotal = true;
        }
    }
}

QQueryForExcelData::~QQueryForExcelData() {
    delete mQuery;
    qDeleteAll(mBinds);
    qDeleteAll(mFields);
    qDeleteAll(mQueryList);
}

QList <QQueryForExcelDataBind *> &QQueryForExcelData::BindsRef() {
    return mBinds;
}

QList <QQueryForExcelData *> &QQueryForExcelData::QueryListRef() {
    return mQueryList;
}

QList <QQueryForExcelDataField *> &QQueryForExcelData::FieldsRef() {
    return mFields;
}

const QSqlQuery *QQueryForExcelData::QueryConst() const {
    return mQuery;
}

void QQueryForExcelData::SetRowsBeforeMarker(const QString &aRowsBeforeMarker) {
    mRowsBeforeMarker = aRowsBeforeMarker;
}

void QQueryForExcelData::SetRowsAfterMarker(const QString &aRowsAfterMarker) {
    mRowsAfterMarker = aRowsAfterMarker;
}

void QQueryForExcelData::SetDoGrouping(bool aDoGrouping) {
    mDoGrouping = aDoGrouping;
}

void QQueryForExcelData::SetDoDeleteIfNoChildren(bool aDoDeleteIfNoChildren) {
    mDoDeleteIfNoChildren = aDoDeleteIfNoChildren;
}

void QQueryForExcelData::SetSkipTotalIfOneRecord(bool aSkipTotalIfOneRecord) {
    mSkipTotalIfOneRecord = aSkipTotalIfOneRecord;
}

void QQueryForExcelData::SetSkipAfterIfOneRecord(bool aSkipAfterIfOneRecord) {
    mSkipAfterIfOneRecord = aSkipAfterIfOneRecord;
}

void QQueryForExcelData::SetRecordCount(int aRecordCount) {
    mRecordCount = aRecordCount;
}

// in fact, only one field can be found... or not...
bool QQueryForExcelData::DoFindFields(const QString &aInputString, int aCurColumn,
                                      QList <QQueryForExcelData *> &aForAddRestStr,
                                      QList<QQueryForExcelData *> &aForAddRowsBefore,
                                      QList<QQueryForExcelData *> &aForAddRowsAfter) {

    bool lIsAnyFound = false;
    int i;

    if (aInputString.contains("#" + mRowsAfterMarker + "#")) {
        if (!aForAddRowsAfter.contains(this)) aForAddRowsAfter.append(this);
        lIsAnyFound = true;
    }

    if (aInputString.contains("#" + mRowsBeforeMarker + "#")) {
        if (!aForAddRowsBefore.contains(this)) aForAddRowsBefore.append(this);
        lIsAnyFound = true;
    }
    if (!mAllIsFoundInCur) {
        mAllIsFoundInCur = true;
        for (i = 0; i < mFields.length(); i++) {
            if (mFields.at(i)->mFoundRowIndex == -1
                    && mFields.at(i)->mType != QQueryForExcelDataField::TypeTotal) {
                if (aInputString.contains("#" + mFields.at(i)->mName + "#")) {
                    // mFoundRowsText will be added later in ExcelUniHandler::endElement
                    mFields.at(i)->mFoundRowIndex = mFoundRowsText.length();
                    mFields.at(i)->mFoundColIndex = aCurColumn;
                    if (!aForAddRestStr.contains(this)) aForAddRestStr.append(this);
                    lIsAnyFound = true;
                } else {
                    mAllIsFoundInCur = false; // some still not found
                }
            }
        }
    }

    for (i = 0; i < mQueryList.length(); i++) {
        if (mQueryList.at(i)->DoFindFields(aInputString, aCurColumn, aForAddRestStr, aForAddRowsBefore, aForAddRowsAfter)) {
            lIsAnyFound = true;
        }
    }

    return lIsAnyFound;
}

QVariant QQueryForExcelData::FieldValue(int aIndex, int aRecordNum, bool aWithFormat) const {
    QVariant lRes;

    if (mFields.at(aIndex)->mGetValueCallback) {
        (*mFields.at(aIndex)->mGetValueCallback)(mQuery, mFields.at(aIndex)->mName, aRecordNum, lRes, mFields.at(aIndex)->mGetValueCallbackDataPtr);
    } else {
        switch (mFields.at(aIndex)->mType) {
        case QQueryForExcelDataField::TypeBase:
            if (mQuery->value(mFields.at(aIndex)->mName) != mFields.at(aIndex)->mNullValue) {
                lRes = mQuery->value(mFields.at(aIndex)->mName);
            }
            break;

        case QQueryForExcelDataField::TypeOperLLPlus: {
                int lIndex1 = (mFields.at(aIndex)->mOperField1 == -1)?aIndex - 2:mFields.at(aIndex)->mOperField1;
                int lIndex2 = (mFields.at(aIndex)->mOperField2 == -1)?aIndex - 1:mFields.at(aIndex)->mOperField2;
                qulonglong lResulonglong = mQuery->value(mFields.at(lIndex1)->mName).toLongLong()
                        + mQuery->value(mFields.at(lIndex2)->mName).toLongLong();
                if (lResulonglong != mFields.at(aIndex)->mNullValue) {
                    lRes = lResulonglong;
                }
            }
            break;

        case QQueryForExcelDataField::TypeOperLLMinus: {
                int lIndex1 = (mFields.at(aIndex)->mOperField1 == -1)?aIndex - 2:mFields.at(aIndex)->mOperField1;
                int lIndex2 = (mFields.at(aIndex)->mOperField2 == -1)?aIndex - 1:mFields.at(aIndex)->mOperField2;
                qulonglong lResulonglong = mQuery->value(mFields.at(lIndex1)->mName).toLongLong()
                        - mQuery->value(mFields.at(lIndex2)->mName).toLongLong();
                if (lResulonglong != mFields.at(aIndex)->mNullValue) {
                    lRes = lResulonglong;
                }
            }
            break;
        }
    }

    if (lRes.isNull() || !aWithFormat) return lRes;

    switch (mFields.at(aIndex)->mFormatType) {
    case QQueryForExcelDataField::FormatNone:
        return lRes;
        break;
    case QQueryForExcelDataField::FormatSum:
        return gSettings->FormatSumForEdit(lRes.toLongLong());
        //return gSettings->FormatSumForList(lRes.toLongLong()).replace(QChar(160), " ");
        break;
    }
    return lRes;
}

void QQueryForExcelData::RowsBeforeAppend(const QString &aStr) {
    mRowsBefore.append(aStr);
    mRowsBefore[mRowsBefore.length() - 1].remove("#" + mRowsBeforeMarker + "#");
}

void QQueryForExcelData::RowsAfterAppend(const QString &aStr) {
    mRowsAfter.append(aStr);
    mRowsAfter[mRowsAfter.length() - 1].remove("#" + mRowsAfterMarker + "#");
}

//--------------------------------------------------------------------------------------------------------------------
ExcelUniHandler::ExcelUniHandler(const QString &aTemplate, const QString &aOutputNameWOExt, const QList<QQueryForExcelData *> &aQueryList) :
    QXmlDefaultHandler(),
    mInErrorState(false), mOutputNameWOExt(aOutputNameWOExt), mInRow(false), mWaitFirstStyle(true), mBeforeStart(true), mCurRow(0),
    mQueryList(aQueryList), mGroupShowLevel(1)
{
    mInputFile = new QFile(aTemplate);
    mOutputFile = new QFile(mOutputNameWOExt + ".xml");

    if (!mInputFile->open(QFile::ReadOnly)) {
        gLogger->ShowError("CreateXMLExcelReport", QObject::tr("Error opening file") + ":\r\n" + mInputFile->fileName() + "\r\n" + QObject::tr("Error") +": " + mInputFile->errorString());
        mInErrorState = true;
        return;
    }
    if (!mOutputFile->open(QFile::WriteOnly)) {
        gLogger->ShowError("CreateXMLExcelReport", QObject::tr("Error creating file") + ":\r\n" + mOutputFile->fileName() + "\r\n" + QObject::tr("Error") +": " + mOutputFile->errorString());
        mInErrorState = true;
        return;
    }

    mOutputWriter = new QXmlStreamWriter(mOutputFile);
    mOutputWriter->setAutoFormatting(true);

    mRestStrWriter = new QXmlStreamWriter(&mRestStr);

    // we are in Before Start Data mode
    mPrevWriters.append(mOutputWriter);
    mCurWriter = mRestStrWriter;
}

ExcelUniHandler::~ExcelUniHandler() {
    mOutputFile->close();
    mInputFile->close();

    delete mOutputFile;
    delete mInputFile;

    delete mOutputWriter;
    delete mRestStrWriter;
    qDeleteAll(mQueryList);
}

void ExcelUniHandler::SetStyleForAddRaw(const QString &aStyleForAddRaw) {
    mStyleForAddRaw = aStyleForAddRaw;
}

void ExcelUniHandler::SetGroupShowLevel(uint aGroupShowLevel) {
    mGroupShowLevel = aGroupShowLevel;
}

void ExcelUniHandler::SetAddToVBAScript(const QStringList &aAddToVBAScript) {
    mAddToVBAScript = aAddToVBAScript;
}


void ExcelUniHandler::Process() {
    if (mInErrorState) return;
    QXmlInputSource lSource(mInputFile);
    QXmlSimpleReader lReader;

    lReader.setDTDHandler(this);
    lReader.setDeclHandler(this);
    lReader.setContentHandler(this);
    lReader.setEntityResolver(this);

    bool res;
    res = lReader.parse(lSource);

    mOutputFile->flush();
    mOutputFile->close();
    mInputFile->close();

    if (res && !mInErrorState) {
        QFile lVBSScript(mOutputNameWOExt + ".vbs");
        if (lVBSScript.open(QFile::WriteOnly)) {
            QString lNameForExcel = mOutputNameWOExt;
            lNameForExcel.replace('/', '\\');
            QTextStream out(&lVBSScript);
            out.setCodec("UTF-16LE");
            out << "On Error Resume Next\n"
                   "sGetPath = Left(Wscript.ScriptFullName, InStrRev(Wscript.ScriptFullName, \"\\\"))\n"
                   "Set oExcel = CreateObject(\"Excel.Application\")\n"
                   "oExcel.visible = true\n"
                   "Set oWbook = oExcel.Workbooks.Open(\"" + lNameForExcel + ".xml" + "\")\n"
                   "oWbook.SaveAs \"" + lNameForExcel + ".xlsx.temp" + "\", 51\n"
                   "Set oWsheet = oWbook.Worksheets(1)\n";

            if (!mGroupRows.isEmpty()) {
                for (int i = 0; i < mGroupRows.length(); i++) {
                    out << "oWsheet.Rows(\"" << mGroupRows.at(i).first + 1 << ":" << mGroupRows.at(i).second << "\").Group\n";
                }
                if (mGroupShowLevel) out << "oWsheet.Outline.ShowLevels " + QString::number(mGroupShowLevel) + "\n";
            }

            if (!mAddToVBAScript.isEmpty()) {
                out << mAddToVBAScript.join('\n') + '\n';
            }

            // xlExcel8 - 56 (xls), xlOpenXMLWorkbook - 51 (xlsx), xlOpenXMLWorkbookMacroEnabled - 52 (xlsm), xlExcel7 - 39 (xls)
            out << "If Err.number <> 0 Then\n"
                   "  oExcel.visible = true\n"
                   "  MsgBox Err.source & \" - \" & Err.number & \": \" & Err.description\n"
                   "  oWbook.SaveAs(sGetPath & \"error.xls\")\n"
                   "Else\n"
                   "  oWbook.SaveAs \"" + lNameForExcel + ".xlsx" + "\", 51\n"
                   "'  oWbook.Save\n"
                   "End If\n"
                   "'oWbook.Close True\n"
                   "'oExcel.Quit\n";
            lVBSScript.flush();
            lVBSScript.close();

            // don't work in right way; it stucks when use "cmd /c" (you can kill cmd then excel start working).
            // it is finished before the xls formed and saved
            QProcess proc1;
            proc1.start(QString(qgetenv("COMSPEC")));
            if (!proc1.waitForStarted(-1)) {
                gLogger->ShowError("VBS wait for started", proc1.errorString());
            } else {
                proc1.write(("start " + mOutputNameWOExt + ".vbs\r\nexit\r\n").toLatin1());
                proc1.closeWriteChannel();
                if (!proc1.waitForFinished(-1)) {
                    gLogger->ShowError("VBS wait for finished", proc1.errorString());
                }
            }

            //RunAndShowReport(mOutputNameWOExt + ".vbs", QFile(mOutputNameWOExt + ".xlsx").fileName(), true);
            //QFile::remove(lVbsName + "/temp-" + lOutName);
        } else {
            gLogger->ShowError("CreateXMLExcelReport", "Can't create VBS file!");
            mInErrorState = true;
        }
    }
}

bool ExcelUniHandler::startDocument() {
    mOutputWriter->writeStartDocument();
    return true;
}

bool ExcelUniHandler::endDocument() {
    DoProcess();

    if (!mRestStr.isEmpty()) {
        mOutputWriter->device()->write(mRestStr.toUtf8());
        mRestStr.clear();
    }

    mOutputWriter->writeEndDocument();
    return true;
}

bool ExcelUniHandler::processingInstruction(const QString & target, const QString & data) {
    if (target.toLower() != "xml") {
        mOutputWriter->writeProcessingInstruction(target, data);
    }
    return true;
}

bool ExcelUniHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) {
    if (qName.toLower() == "row") {
        if (mInRow) {
            mErrorStr = "Unexpected start of Row";
            gLogger->ShowError("CreateXMLExcelReport", "Unexpected start of Row");
            return false;
        }
        mInRow = true;
        mFoundInCurRow = false;
        mCurCol = -1;

        if (mCurWriter != mRestStrWriter) {
            // we don't know yet, is data started in current row or not
            mPrevWriters.append(mCurWriter);
            mCurWriter = mRestStrWriter;
        }
    } else if (qName.toLower() == "cell") {
        if (!mInRow) {
            mErrorStr = "Unexpected start of Cell";
            gLogger->ShowError("CreateXMLExcelReport", "Unexpected start of Cell");
            return false;
        }
        mCurCol++;
    } else if (qName.toLower() == "style" && mWaitFirstStyle) {
        if (!mStyleForAddRaw.isEmpty()) {
            mRestStr += mStyleForAddRaw;
        }
        mWaitFirstStyle = false;
    }

    mCurWriter->writeStartElement(qName);

    // fucking microsoft
    if (!mPropStr.isEmpty()) {
        mCurWriter->writeAttribute("xmlns", mPropStr);
        mPropStr.clear();
    }

    for (int i = 0; i < atts.length(); i++) {
        if (!(qName.toLower() == "table"
                    && atts.qName(i).toLower() == "ss:expandedrowcount")
                && !(qName.toLower() == "row"
                    && atts.qName(i).toLower() == "ss:index")) {
            mCurWriter->writeAttribute(atts.qName(i), atts.value(i));
        }
    }

    return true;
}

bool ExcelUniHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName) {
    if (qName.toLower() == "row") {
        if (!mInRow) {
            mErrorStr = "Unexpected end of Row";
            gLogger->ShowError("CreateXMLExcelReport", "Unexpected end of Row");
            return false;
        }
        // it is probably mRestStrWriter
        if (mCurWriter != mRestStrWriter) {
            mErrorStr = "Unexpected writer in end of Row";
            gLogger->ShowError("CreateXMLExcelReport", "Unexpected writer in end of Row");
            return false;
        }
        mCurWriter->writeEndElement();

        if (!mRestStr.isEmpty()) {
            if (mBeforeStart) {
                // write to output file
                mOutputWriter->device()->write(mRestStr.toUtf8());
                mCurRow++; // skipped rows
            } else {
                // data started, save to founded
                for (int i = 0; i < mForAddRestStr.length(); i++) {
                    mForAddRestStr.at(i)->mFoundRowsText.append(mRestStr);
                }
                for (int i = 0; i < mForAddRowsBefore.length(); i++) {
                    mForAddRowsBefore.at(i)->RowsBeforeAppend(mRestStr);
                }
                for (int i = 0; i < mForAddRowsAfter.length(); i++) {
                    mForAddRowsAfter.at(i)->RowsAfterAppend(mRestStr);
                }
            }
            if (!mForAddRestStr.isEmpty()
                    || !mForAddRowsBefore.isEmpty()
                    || !mForAddRowsAfter.isEmpty()
                    || mBeforeStart) {
                mForAddRestStr.clear();
                mForAddRowsBefore.clear();
                mForAddRowsAfter.clear();
                mRestStr.clear();
            }
        }
        mInRow = false;

        if (mBeforeStart) {
            if (mPrevWriters.isEmpty()) {
                mErrorStr = "Previous writer is empty in end of Row";
                gLogger->ShowError("CreateXMLExcelReport", "Previous writer is empty in end of Row");
                return false;
            }
            mCurWriter = mPrevWriters.takeAt(mPrevWriters.length() - 1);
        }
    } else {
        mCurWriter->writeEndElement();
    }
    return true;
}

bool ExcelUniHandler::startPrefixMapping(const QString &prefix, const QString &uri) {
    if (prefix.isEmpty()) {
        mPropStr = uri;
    } else {
        mCurWriter->writeNamespace(uri, prefix);
    }
    return true;
}

bool ExcelUniHandler::characters(const QString &ch) {
    if (mInRow) {
        for (int i = 0; i < mQueryList.length(); i++) {
            if (mQueryList.at(i)->DoFindFields(ch, mCurCol, mForAddRestStr, mForAddRowsBefore, mForAddRowsAfter)) {
                mBeforeStart = false; // data started
            }
        }
    }
    mCurWriter->writeCharacters(ch);
    return true;
}

QString ExcelUniHandler::errorString() const {
    return mErrorStr;
}

bool ExcelUniHandler::DoProcessInternal(QQueryForExcelData *aQuery, QStringList *aOutputStrings) {
    int j, k, l;

    for (j = 0; j < aQuery->mBinds.length(); j++) {
        if (aQuery->mBinds.at(j)->mType == QQueryForExcelDataBind::TypeValue) {
            aQuery->mQuery->bindValue(":" + aQuery->mBinds.at(j)->mName, aQuery->mBinds.at(j)->mValue);
        }
    }

    if (!aQuery->mQuery->lastQuery().isEmpty()
            && !aQuery->mQuery->exec()) {
        gLogger->ShowSqlError(QObject::tr("Report builder - execute"), *aQuery->mQuery);
        aQuery->mError = true;
    } else {
        int lRecordNum = 0;
        aQuery->mOccupiedRows.clear();
        while (!mInErrorState
               && (!aQuery->mQuery->lastQuery().isEmpty() && aQuery->mQuery->next()
                   || aQuery->mQuery->lastQuery().isEmpty() && lRecordNum < aQuery->mRecordCount)) {
            lRecordNum++;
            if (aQuery->mOccupiedRows.isEmpty()) {
                // rows before
                if (!aQuery->mRowsBefore.isEmpty()) {
                    if (aOutputStrings) {
                        aOutputStrings->append(aQuery->mRowsBefore);
                    } else {
                        mOutputWriter->device()->write(aQuery->mRowsBefore.join("").toUtf8());
                    }
                    mCurRow += aQuery->mRowsBefore.length();
                }
            }

            int lCurRow = mCurRow;
            QStringList lOutputStrings;
            // for each string in query
            for (j = 0; j < aQuery->mFoundRowsText.length(); j++) {
                QString lOutputStr = aQuery->mFoundRowsText.at(j);
                bool lAnyFound = false;
                for (k = 0; k < aQuery->mFields.length(); k++) {
                    if (aQuery->mFields.at(k)->mFoundRowIndex == j) {
                        if (aQuery->mFields.at(k)->mType != QQueryForExcelDataField::TypeChildSum) {
                            QString lNewStyleId, lValue;
                            if (aQuery->mFields.at(k)->mChangeStyleIdCallback) {
                                (*aQuery->mFields.at(k)->mChangeStyleIdCallback)(aQuery->mQuery, aQuery->mFields.at(k)->mName, lNewStyleId);
                                if (!lNewStyleId.isEmpty()) {
                                    // "capturing parenthesis"
//                                    gLogger->ShowErrorInList(NULL, "", lOutputStr);
//                                    gLogger->ShowErrorInList(NULL, aQuery->mFields.at(k)->mName, lNewStyleId);
                                    lOutputStr.replace(QRegExp("StyleID=\"[^>]*\"([^>]*><Data[^>]*)>([^>]*)#" + aQuery->mFields.at(k)->mName + "#"), "StyleID=\"" + lNewStyleId + "\"\\1>\\2#" + aQuery->mFields.at(k)->mName + "#");
//                                    gLogger->ShowErrorInList(NULL, "", lOutputStr);
//                                    gLogger->ShowErrorInList(NULL, "", "---------------------");
                                }
                            }
                            lValue = aQuery->FieldValue(k, lRecordNum).toString();
                            if (aQuery->mFields.at(k)->mFormatType == QQueryForExcelDataField::FormatSum
                                    && !lValue.isEmpty()) {
                                lOutputStr.replace(QRegExp("<Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), "<Data ss:Type=\"Number\">\\1" + lValue + "\\2</Data>");
                            } else {
                                lOutputStr.replace(QRegExp("<Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), "<Data ss:Type=\"String\">\\1" + lValue + "\\2</Data>");
                            }
                        }
                        lAnyFound = true;
                    }
                }
                if (lAnyFound) {
                    if (!aQuery->mQueriesWithSum.isEmpty()) {
                        lOutputStrings.append(lOutputStr);
                    } else {
                        if (aOutputStrings) {
                            aOutputStrings->append(lOutputStr);
                        } else {
                            mOutputWriter->device()->write(lOutputStr.toUtf8());
                        }
                    }
                    aQuery->mOccupiedRows.append(mCurRow);
                    mCurRow++;
                }
            }

            int lOutputStringsLength = lOutputStrings.length();
            for (j = 0; j < aQuery->mQueryList.length(); j++) {
                QQueryForExcelData *lQuery = aQuery->mQueryList.at(j);
                // bind data
                for (k = 0; k < lQuery->mBinds.length(); k++) {
                    if (lQuery->mBinds.at(k)->mType == QQueryForExcelDataBind::TypeParent) {
                        bool lIsFound = false;
                        for (l = 0; l < aQuery->mFields.length(); l++) {
                            if (!aQuery->mFields.at(l)->mName.compare(lQuery->mBinds.at(k)->mName, Qt::CaseInsensitive)) {
                                lQuery->mQuery->bindValue(":" + lQuery->mBinds.at(k)->mName, aQuery->FieldValue(l, lRecordNum, false));
                                lIsFound = true;
                                break;
                            }
                        }
                        if (!lIsFound) {
                            lQuery->mQuery->bindValue(":" + lQuery->mBinds.at(k)->mName, aQuery->mQuery->value(lQuery->mBinds.at(k)->mName));
                        }
                    }
                }

                // process data
                DoProcessInternal(lQuery, aQuery->mQueriesWithSum.isEmpty()?aOutputStrings:&lOutputStrings);
                if (lQuery->mError) {
                    aQuery->mError = true;
                    mInErrorState = true;
                    break;
                }

                if (aQuery->mQueriesWithSum.contains(j)
                        || aQuery->mQueriesWithSum.contains(-1)) {
                    bool lSolidRegion = false;
                    if (lQuery->mOccupiedRows.length() > 1) {
                        lSolidRegion = true;
                        for (k = 0; k < lQuery->mOccupiedRows.length() - 1; k++) {
                            if (lQuery->mOccupiedRows.at(k) != lQuery->mOccupiedRows.at(k + 1) - 1) {
                                lSolidRegion = false;
                                break;
                            }
                        }
                    }
                    for (k = 0; k < aQuery->mFields.length(); k++) {
                        if (aQuery->mFields.at(k)->mType == QQueryForExcelDataField::TypeChildSum
                                && (aQuery->mFields.at(k)->mOperField1 == j || aQuery->mFields.at(k)->mOperField1 == -1)) {
                            for (l = 0; l < lOutputStringsLength; l++) {
                                if (lOutputStrings.at(l).contains("#" + aQuery->mFields.at(k)->mName + "#")) {
                                    if (lSolidRegion || lQuery->mOccupiedRows.length() == 1) {
                                        lOutputStrings[l].replace(QRegExp("><Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), " ss:Formula=\"=SUM(R[" + QString::number(lQuery->mOccupiedRows.first() - lCurRow) + "]C:R[" + QString::number(lQuery->mOccupiedRows.last() - lCurRow) + "]C)\"><Data ss:Type=\"Number\">\\1" + QString::number(lCurRow + l) + "\\2</Data>");
                                    } else if (!lQuery->mOccupiedRows.isEmpty()){
                                        QStringList lSumParams;
                                        for (int m = 0; m < lQuery->mOccupiedRows.length(); m++) {
                                            lSumParams.append("R[" + QString::number(lQuery->mOccupiedRows.at(m) - lCurRow) + "]C");
                                        }
                                        lOutputStrings[l].replace(QRegExp("><Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), " ss:Formula=\"=SUM(" + lSumParams.join(',') + ")\"><Data ss:Type=\"Number\">\\1" + QString::number(lCurRow + l) + "\\2</Data>");
                                    } else {
                                        lOutputStrings[l].replace(QRegExp("><Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), "><Data ss:Type=\"Number\">\\10\\2</Data>");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // here must be another replacemnt of QQueryForExcelDataField::TypeChildSum - if it was not replaced in previous steps

            // here should get internal array of string and write sums here and to Writer
            if (!aQuery->mQueriesWithSum.isEmpty()) {
                if (!aQuery->mDoDeleteIfNoChildren
                        || aQuery->mQueryList.isEmpty()
                        || lCurRow + 1 < mCurRow) {
                    if (aOutputStrings) {
                        aOutputStrings->append(lOutputStrings);
                    } else {
                        mOutputWriter->device()->write(lOutputStrings.join("").toUtf8());
                    }
                } else {
                    // skip this row
                    mCurRow--;
                    lCurRow--;
                    aQuery->mOccupiedRows.removeAll(mCurRow);
                }
            }
            if (aQuery->mDoGrouping
                    && lCurRow + 1 < mCurRow) {
                mGroupRows.append(qMakePair(lCurRow + 1, mCurRow));
            }
        }
        if (!aQuery->mOccupiedRows.isEmpty()) {
            // rows after
            if (!aQuery->mRowsAfter.isEmpty() && (!aQuery->mSkipAfterIfOneRecord || aQuery->mOccupiedRows.length() > 1)) {
                QString lRowsAfter;

                lRowsAfter = aQuery->mRowsAfter.join("");
                if (aQuery->mHasTotal) {
                    bool lSolidRegion = false;
                    if (aQuery->mOccupiedRows.length() > 1) {
                        lSolidRegion = true;
                        for (k = 0; k < aQuery->mOccupiedRows.length() - 1; k++) {
                            if (aQuery->mOccupiedRows.at(k) != aQuery->mOccupiedRows.at(k + 1) - 1) {
                                lSolidRegion = false;
                                break;
                            }
                        }
                    }
                    for (k = 0; k < aQuery->mFields.length(); k++) {
                        if (aQuery->mFields.at(k)->mType == QQueryForExcelDataField::TypeTotal) {
                            if (lRowsAfter.contains("#" + aQuery->mFields.at(k)->mName + "#")) {
                                if (!aQuery->mSkipTotalIfOneRecord || aQuery->mOccupiedRows.length() > 1) {
                                    if (lSolidRegion || aQuery->mOccupiedRows.length() == 1) {
                                        lRowsAfter.replace(QRegExp("><Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), " ss:Formula=\"=SUM(R[" + QString::number(aQuery->mOccupiedRows.first() - mCurRow - 1) + "]C:R[" + QString::number(aQuery->mOccupiedRows.last() - mCurRow - 1) + "]C)\"><Data ss:Type=\"Number\">\\1" + QString::number(mCurRow - 1) + "\\2</Data>");
                                    } else if (!aQuery->mOccupiedRows.isEmpty()){
                                        QStringList lSumParams;
                                        for (int m = 0; m < aQuery->mOccupiedRows.length(); m++) {
                                            lSumParams.append("R[" + QString::number(aQuery->mOccupiedRows.at(m) - mCurRow - 1) + "]C");
                                        }
                                        lRowsAfter.replace(QRegExp("><Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), " ss:Formula=\"=SUM(" + lSumParams.join(',') + ")\"><Data ss:Type=\"Number\">\\1" + QString::number(mCurRow - l) + "\\2</Data>");
                                    } else {
                                        lRowsAfter.replace(QRegExp("><Data[^>]*>([^>]*)#" + aQuery->mFields.at(k)->mName + "#([^<]*)</Data>"), "><Data ss:Type=\"Number\">\\10\\2</Data>");
                                    }
                                } else {
                                    lRowsAfter.remove("#" + aQuery->mFields.at(k)->mName + "#");
                                }
                            }
                        }
                    }
                }

                if (aOutputStrings) {
                    aOutputStrings->append(lRowsAfter);
                } else {
                    mOutputWriter->device()->write(lRowsAfter.toUtf8());
                }
                mCurRow += aQuery->mRowsAfter.length();
            }
        }
    }
    return true;
}

bool ExcelUniHandler::DoProcess() {
    int i;

    //run
    for (i = 0; i < mQueryList.length(); i++) {
        // process
        DoProcessInternal(mQueryList.at(i));
        if (mQueryList.at(i)->mError) break;
    }

    return true; // it is dummy
}
