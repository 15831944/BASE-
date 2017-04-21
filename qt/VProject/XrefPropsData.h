#ifndef XREFPROPSDATA_H
#define XREFPROPSDATA_H

#include <QString>
#include <QList>

class XrefOnePropData
{
private:
    struct s1Type {
        unsigned long mIdMain, mIdXref, mIdXrefPlot;
        long mObjType;
        wchar_t mName[36];
        long mDisabled;
        union {
            // 0 - drawing
            struct {
                long mAllBlocksColor, mAllEntitiesColor; // that's it
                long mAllBlocksLineweight, mAllEntitiesLineweight; // толщина линий, AcDb::LineWeight enum
                wchar_t mLayer0Name[36];
            };
            // 1 - layer
            struct {
                long mLayerPrintColor, mLayerEntitiesColor; // all print color and all color
                long mLayerEntitiesLineweight; // толщина линий, AcDb::LineWeight enum
                wchar_t mLayerNewName[36];
            };
        };

        bool mNeedProcess; // it is dummy field here
    } s1;

public:
    XrefOnePropData();

    bool operator==(const XrefOnePropData &other) const;

    void *GetDataBuffer() { return &s1; }
    static long GetDataSize() { return sizeof(s1Type); }

    long ObjType() const { return s1.mObjType; }
    void SetObjType(long aObjType);

#define GetSet(Type, Name) \
    Type Name() const { return s1.m##Name; } \
    void Set##Name(Type a##Name) { s1.m##Name = a##Name; }

    GetSet(unsigned long, IdMain)
    GetSet(unsigned long, IdXref)
    GetSet(unsigned long, IdXrefPlot)
    GetSet(bool, NeedProcess)

    GetSet(long, Disabled)

    // obj type == 0
    GetSet(long, AllBlocksColor)
    GetSet(long, AllEntitiesColor)
    GetSet(long, AllBlocksLineweight)
    GetSet(long, AllEntitiesLineweight)

    // obj type == 1
    GetSet(long, LayerPrintColor)
    GetSet(long, LayerEntitiesColor)
    GetSet(long, LayerEntitiesLineweight)
#undef GetSet

#define GetSetStr(Name) \
    QString Name() const { return QString::fromWCharArray(s1.m##Name); } \
    void Set##Name(const QString &a##Name) { \
            s1.m##Name[a##Name.length()] = 0; \
            a##Name.toWCharArray(s1.m##Name); \
    }

    // 0
    GetSetStr(Name)
    GetSetStr(Layer0Name)

    // 1
    GetSetStr(LayerNewName)
#undef GetSetStr

    bool IsDefault() const;

    QString GetAbrv() const ;
};

class XrefPropsData
{
protected:
    int mIdCommon;
    QList<XrefOnePropData *> mXrefProps;
public:
    XrefPropsData() : mIdCommon(0) {}
    XrefPropsData(int aIdCommon) : mIdCommon(aIdCommon) {}
    virtual ~XrefPropsData() {
        qDeleteAll(mXrefProps);
    }

    bool operator== (const XrefPropsData &other) const {
        return mIdCommon == other.mIdCommon && mXrefProps == other.mXrefProps;
    }

    int IdCommon() const { return mIdCommon; }
    void SetIdCommon(int aIdCommon) { mIdCommon = aIdCommon; }

    QList<XrefOnePropData *> & XrefPropsRef() { return mXrefProps; }
    const QList<XrefOnePropData *> & XrefPropsConst() const { return mXrefProps; }

    QString GetAbrv() const {
        QString str1, res;
        int i;
        for (i = 0; i < mXrefProps.length(); i++) {
            str1 = mXrefProps.at(i)->GetAbrv();
            if (!str1.isEmpty()) {
                res += str1;
            }

        }
        //if (!res.isEmpty()) res = res.left(res.length() - 1);
        return res;
    }
};

typedef XrefPropsData * XrefPropsDataPtr;

class XrefRenameData
{
protected:
    struct s1Type {
        wchar_t mOldName[72], mNewName[72];
    } s1;
public:
    XrefRenameData(const QString &aOldName, const QString &aNewName) {
        s1.mOldName[aOldName.length()] = 0;
        aOldName.toWCharArray(s1.mOldName);

        s1.mNewName[aNewName.length()] = 0;
        aNewName.toWCharArray(s1.mNewName);
    }

    void *GetDataBuffer() { return &s1; }
    static long GetDataSize() { return sizeof(s1Type); }
};

#endif // XREFPROPSDATA_H
