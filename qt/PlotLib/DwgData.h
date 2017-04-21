#ifndef DWGDATA_H
#define DWGDATA_H

#include <QString>
#include <QDate>
#include <QList>

#include "../VProject/CommonData.h"

class DwgData
{
    DeclarePar(int, Id)
    DeclarePar(int, IdPlot)
    DeclarePar(int, IdPlotEdge)
    DeclarePar(int, Version)
    DefParStr(Extension)
    DeclarePar(int, NeedNotProcess)
    DeclarePar(int, NestedXrefMode)
    DeclarePar(int, LayoutCnt)

    DefParCompl(QDateTime, FTime) // file date/time
    DeclarePar(int, InSubs)
    DefParCompl(QDateTime, VFDate) // field verification date/time

    DeclareParRO(qlonglong, DataLength)
    DeclareParRO(int, AcadVer)

    DeclareParRO(int, IdCache)

#define DEFINE_GROUP_NUM 1
    DefParComplNum(QList<int>, FieldTypes)

private:
    QList <int> mInited;
    void InitGrpData(int aGrpNum);

public:
    explicit DwgData(int aId);

    QString AcadVerStr() const;

    void InitFromId(int aId);

    // groups -------------------------

    // 1
    void LoadFieldTypes();
    void UninitFieldTypes();
    // -------------------------

    static bool INSERT(quint64 &aId, int aIdPlot, int aVersion, const QString &aExtension, const QString &aSha256, int aNeedNotProcess, int aLayoutCnt,
                        const QDateTime &aFTime, const QByteArray *aData);
    static bool CopyDwgXrefs(quint64 aIdOld, quint64 aIdNew);
    static bool CopyAllRefs(quint64 aIdOld, quint64 aIdNew);
};

typedef DwgData * DwgDataPtr;

class DwgForSaveData : public DwgData
{
    DefParStrRO(Filename)
    DefParStrRO(GroupName) // such as 'Acad:Text'; use in SaveDialog for subdirectory name

    DeclareParRO(bool, InMain)
    DeclareParRO(bool, InXref)

    DefParStrRO(Sha256)
public:
    explicit DwgForSaveData(int aId);
    explicit DwgForSaveData(int aId, const QString &aFilename, int aVersion, const QDateTime & aFTime,
                        qlonglong aDataLength, const QString &aGroupName, bool aInMain, bool aInXref, const QString &aSha256);

    void SetInMain(bool aInMain) { mInMain = aInMain; }
    void SetInXref(bool aInXref) { mInXref = aInXref; }

};

typedef DwgForSaveData * DwgForSaveDataPtr;

#endif // DWGDATA_H
