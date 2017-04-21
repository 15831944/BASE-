#ifndef XMLPROCESS_H
#define XMLPROCESS_H

#include <QXmlDefaultHandler>

#include "def_expimp.h"

class QXmlStreamWriter;
class QSqlQuery;

class ExcelUniHandler;
class QQueryForExcelData;

class EXP_IMP QQueryForExcelDataBind
{
    friend ExcelUniHandler;
    friend QQueryForExcelData;
public:
private:
    enum enumType { TypeParent, TypeValue };
    QString mName; // bind variable equal to field from prev query for type == TypePrev
    enumType mType;
    QVariant mValue;
public:
    explicit QQueryForExcelDataBind(const QString &aName);
    explicit QQueryForExcelDataBind(const QString &aName, const QVariant &aValue);
};

class EXP_IMP QQueryForExcelDataField
{
    friend ExcelUniHandler;
    friend QQueryForExcelData;
public:
    enum enumType { TypeBase, TypeChildSum, TypeTotal, TypeOperLLPlus, TypeOperLLMinus };
    enum enumFormat { FormatNone, FormatSum };
private:
    QString mName;
    enumType mType;
    int mOperField1, mOperField2;

    enumFormat mFormatType;

    int mFoundRowIndex, mFoundColIndex;
    QVariant mNullValue;
    // same callback can be used for different fields - we have aFieldName here
    void (*mChangeStyleIdCallback) (const QSqlQuery *aQuery, const QString &aFieldName, QString &aNewStyleId);
    // aRecordNum start with 1
    void (*mGetValueCallback) (const QSqlQuery *aQuery, const QString &aFieldName, int aRecordNum, QVariant &aValue, void *aData);
    void *mGetValueCallbackDataPtr;
public:
    explicit QQueryForExcelDataField(const QString &aName, enumFormat aFormatType = FormatNone, const QVariant &aNullValue = QVariant());
    explicit QQueryForExcelDataField(const QString &aName, enumType aType,
                                     int aOperField1 = -1, int aOperField2 = -1, enumFormat aFormatType = FormatNone, const QVariant &aNullValue = QVariant());

    void SetChangeStyleIdCallback(void (*aChangeStyleIdCallback) (const QSqlQuery *, const QString &, QString &));
    void SetGetValueCallback(void (*aGetValueCallback) (const QSqlQuery *, const QString &, int, QVariant &, void *));
    void SetGetValueCallbackDataPtr(void *aDataPtr);
};

class EXP_IMP QQueryForExcelData
{
    friend ExcelUniHandler;
private:
    QList <QQueryForExcelDataBind *> mBinds;
    // fields for output; names in mBinds need not to be here
    QList <QQueryForExcelDataField *> mFields;

    QSqlQuery *mQuery;

    QList<int> mQueriesWithSum;
    bool mHasTotal, mSkipTotalIfOneRecord, mSkipAfterIfOneRecord;

    bool mError, mAllIsFoundInCur;
    int mRecordCount; // it is count of rows for non-sql queries; by default is 1

    QStringList mFoundRowsText; // text for found rows with FieldNames
    QString mRowsBeforeMarker, mRowsAfterMarker;
    QStringList mRowsBefore, mRowsAfter;

    bool mDoGrouping, mDoDeleteIfNoChildren;

    // subqueries - it is bound on parent query
    QList <QQueryForExcelData *> mQueryList;

    QList<int> mOccupiedRows;

    bool DoFindFields(const QString &aInputString, int aCurColumn,
                      QList <QQueryForExcelData *> &aForAddRestStr,
                      QList <QQueryForExcelData *> &aForAddRowsBefore,
                      QList <QQueryForExcelData *> &aForAddRowsAfter);
    // aIndex is index in mFields list
    QVariant FieldValue(int aIndex, int aRecordNum, bool aWithFormat = true) const; // after calling next()
    void RowsBeforeAppend(const QString &aStr);
    void RowsAfterAppend(const QString &aStr);
public:
    explicit QQueryForExcelData(const QString &aSqlText, const QList <QQueryForExcelDataField *> &aFields);
    virtual ~QQueryForExcelData();

    QList <QQueryForExcelDataBind *> &BindsRef();
    QList <QQueryForExcelData *> &QueryListRef();
    QList <QQueryForExcelDataField *> &FieldsRef();
    const QSqlQuery *QueryConst() const;
    void SetRowsBeforeMarker(const QString &aRowsBeforeMarker);
    void SetRowsAfterMarker(const QString &aRowsAfterMarker);
    void SetDoGrouping(bool aDoGrouping);
    void SetDoDeleteIfNoChildren(bool aDoDeleteIfNoChildren);
    void SetSkipTotalIfOneRecord(bool aSkipTotalIfOneRecord);
    void SetSkipAfterIfOneRecord(bool aSkipAfterIfOneRecord);
    void SetRecordCount(int aRecordCount);
};

class EXP_IMP ExcelUniHandler : public QXmlDefaultHandler {
private:
    bool mInErrorState;
    QString mOutputNameWOExt;
    bool mInRow, mWaitFirstStyle, mBeforeStart, mFoundInCurRow;
    int mCurCol, mCurRow;

    QFile *mInputFile, *mOutputFile;


    QString mStyleForAddRaw;
    QString mErrorStr;
    QString mRestStr;
    QXmlStreamWriter *mOutputWriter, *mRestStrWriter;

    QList<QXmlStreamWriter *> mPrevWriters;
    QXmlStreamWriter *mCurWriter;

    QList <QQueryForExcelData *> mForAddRestStr, mForAddRowsBefore, mForAddRowsAfter;

    QString mPropStr; // fucking microsoft

    QList <QQueryForExcelData *> mQueryList;

    QList <QPair<int, int>> mGroupRows;
    uint mGroupShowLevel; // start with 1 and 1 is default; 0 for none - all is expanded by default
    QStringList mAddToVBAScript;

    bool DoProcessInternal(QQueryForExcelData *aQuery, QStringList *aOutputStrings = NULL);
    bool DoProcess();
public:
    explicit ExcelUniHandler(const QString &aTemplate, const QString &aOutputNameWOExt, const QList<QQueryForExcelData *> &aQueryList);
    virtual ~ExcelUniHandler();

    void SetStyleForAddRaw(const QString &aStyleForAddRaw);
    void SetGroupShowLevel(uint aGroupShowLevel);
    void SetAddToVBAScript(const QStringList &aAddToVBAScript);
    void Process();

    bool startDocument();
    bool endDocument();
    bool processingInstruction(const QString & target, const QString & data);

    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);

    bool startPrefixMapping(const QString &prefix, const QString &uri);

    bool characters(const QString &ch);

    QString errorString() const;
};

//bool EXP_IMP CreateXMLExcelReport(const QString &aTemplate, const QString &aOutputNameWOExt, const QList<QQueryForExcelData *> &aQueryList);

#endif // XMLPROCESS_H
