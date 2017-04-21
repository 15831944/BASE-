#ifndef CDWGLAYOUT_H
#define CDWGLAYOUT_H

#include <QList>
#include <QtSql/QSqlQuery>

class CDwgLayout;
class CDwgLayoutBlock;
class CDwgLBAttr;

class CDwgLayoutFactory1 {
protected:
    QSqlQuery *mInsertDwgLayout, *mInsertLayoutBlock, *mInsertDwgLBAttr;
    QSqlQuery *mSelectLayoutBlock, *mSelectLBAttr;

    CDwgLayoutFactory1();
public:
    static CDwgLayoutFactory1 * GetInstance();

    QSqlQuery *qInsertDwgLayout();
    QSqlQuery *qInsertLayoutBlock();
    QSqlQuery *qInsertDwgLBAttr();

    //--
    QSqlQuery *qSelectLayoutBlock();
    QSqlQuery *qSelectLBAttr();

    void Clean();
};


// some kind of repeating of class DwgLayoutData
// class DwgLayoutData is for manipulating data in Qt
// class CDwgLayout coming from C++, it is for receiving data from AutoCAD
class CDwgLayout {
private:
    struct s1Type {
        quint32 mHandleHigh, mHandleLo;
        quint32 mOrderNum;
        wchar_t mName[160], mSheet[16], mNameBottom[4000];
        wchar_t mNameAcad[160], mSheetAcad[16], mNameBottomAcad[4000];
    } s1;

    quint64 mId;
    QList<CDwgLayoutBlock *> mBlocks;
public:
    CDwgLayout(void *aS1);
    ~CDwgLayout();

    static long GetDataSize();
    //void *GetDataBuffer() { return &s1; }

    const QList<CDwgLayoutBlock *> &BlocksConst() const;
    QList<CDwgLayoutBlock *> &BlocksRef();

    bool InsertToBase(quint64 aIdDwg);
};

class CDwgLBAttr;

class CDwgLayoutBlock {
private:
    struct s1Type {
        quint32 mHandleHigh, mHandleLo;
        wchar_t mName[72];
    } s1;

    quint64 mId;
    QList<CDwgLBAttr *> mAttrs;
public:
    CDwgLayoutBlock(void *aS1);
    ~CDwgLayoutBlock();

    static long GetDataSize();
    //void *GetDataBuffer() { return &s1; }

    const QList<CDwgLBAttr *> &AttrsConst() const;
    QList<CDwgLBAttr *> &AttrsRef();

    bool InsertToBase(quint64 aIdDwgLayout);
};

class CDwgLBAttr {
private:
    struct s1Type {
        quint32 mHandleHigh, mHandleLo;
        quint32 mOrderNum;
        wchar_t mTag[72], mPrompt[320], mUFValue[4000], mEncValue[4000], mUFValueAcad[4000], mEncValueAcad[4000];
        wchar_t mTextStyle[64], mTextFont[64];
        quint32 mTextHorMode, mTextVertMode, mTextBackward, mTextUpsidedown;
        double mTextHeight, mTextWidth, mTextRotation, mTextOblique;
        wchar_t mPropLayer[320], mPropLinetype[320];
        quint32 mPropColor;
        qint32 mPropLineWeight;
        wchar_t mPropPlotStyle[320];
    } s1;

    quint64 mId;
public:
    CDwgLBAttr(void *aS1);
    ~CDwgLBAttr();

    static long GetDataSize();
    //void *GetDataBuffer() { return &s1; }

    bool InsertToBase(quint64 aIdDwgLB);
};

#endif // CDWGLAYOUT_H
