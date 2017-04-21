#ifndef DWGLAYOUTDATA_H
#define DWGLAYOUTDATA_H

#include <QObject>

#include <QString>
#include <QDate>
#include <QList>

#include "../VProject/CommonData.h"

class DwgLayoutBlockData;

class DwgLayoutData
{
    DeclareParRO(quint64, Id)
    DeclareParRO(quint64, IdDwg)
    DeclareParRO(int, Num)

    DefParStr(Name)
    DefParStr(Sheet)
    DefParStr(Bottom)

    DefParCompl(QList<DwgLayoutBlockData *>, Blocks)

private:
    bool mIsNew;

    void CollectProps();
public:
    explicit DwgLayoutData(quint64 aId, quint64 aIdDwg, int aNum, const QString &aName, const QString &aSheet, const QString &aBottom);
    ~DwgLayoutData();

    bool HasAnyProp() const;

    void GetMatzDFTexts(QString &aDText, QString &aFText);

    void SetIdDwg(int aIdDwg) { if (mIsNew && !mIdDwg) mIdDwg = aIdDwg; }

    bool SaveData();
    void RollbackEdit();
    void CommitEdit();
};

class DwgLBAttrData;

class DwgLayoutBlockData
{
    DeclareParRO(int, Id)
    DefParStrRO(Name)
    DefParCompl(QList<DwgLBAttrData *>, Attrs)
private:
    QStringList mCodes, mRevisions, mRevDates;
    QStringList mNameTops, mNameBottoms;
    QStringList mSheetFileNames, mPlotFileNames;
    bool mHasAnyProp;
public:
    explicit DwgLayoutBlockData(long aId, const QString &aName);
    ~DwgLayoutBlockData();

    void ClearProps();
    bool HasAnyProp() const;

    void AddCode(const QString &aCode);
    void AddRevision(const QString &aRevision);
    void AddRevDate(const QString &aRevDate);
    void AddNameTop(const QString &aNameTop);
    void AddNameBottom(const QString &aNameBottom);
    void AddSheetFileName(const QString &aSheetFileName);
    void AddPlotFileName(const QString &aPlotFileName);

    const QStringList &Codes();
    const QStringList &Revisions();
    const QStringList &RevDates();
    const QStringList &NameTops();
    const QStringList &NameBottoms();
    const QStringList &SheetFileNames();
    const QStringList &PlotFileNames();

};

class DwgLBAttrData
{
    DeclareParRO(int, Id)
    DeclareParRO(int, OrderNum)
    DefParStrRO(Tag)
    DefParStrRO(Prompt)
    DefParStr(UFValue)
    DefParStr(EncValue)
    DefParStrRO(UFValueAcad)
    DefParStrRO(EncValueAcad)
    DefParStrRO(TextFont)
    DeclareParRO(int, TextBackward)
    DeclareParRO(int, FieldCode)

public:
    explicit DwgLBAttrData(int aId, int aOrderNum, const QString &aTag, const QString &aPrompt,
                           const QString &aUFValue, const QString &aEncValue,
                           const QString &aUFValueAcad, const QString &aEncValueAcad,
                           const QString &aTextFont, int aTextBackward, int aFieldCode);

    QString GetValue() const;
    void SetValue(QString aValue);

    bool SaveData();
    void RollbackEdit();
    void CommitEdit();
};

Q_DECLARE_METATYPE(DwgLayoutData *)
Q_DECLARE_METATYPE(DwgLayoutBlockData *)
Q_DECLARE_METATYPE(DwgLBAttrData *)

#endif // DWGLAYOUTDATA_H
