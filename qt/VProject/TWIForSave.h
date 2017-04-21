#ifndef TWIFORSAVE_H
#define TWIFORSAVE_H

#pragma warning(disable:4100)

#include "ForSaveData.h"

// main list (list with documents)
class TWIForSaveMain : public QTreeWidgetItem
{
public:
    enum ItemType { ICTMain, ICTXref, ICTAddFile, ICTXrefTop, ICTXrefChild };
protected:
    ItemType mItemType;

    int mColSentDate, mColEditDate, mColStatus, mColDatalength;

public:
    explicit TWIForSaveMain(ItemType aItemType, QTreeWidget * parent);
    virtual bool operator<(const QTreeWidgetItem & other) const;

    ItemType GetItemType() const;
    void ShowStatus(PlotData::enumPES aES, const QString &aUserLogin);
};

// documents in main list
class TWIForSaveMainDoc : public TWIForSaveMain
{
private:
    DefParComplRORef(PlotForSaveDataPtr, Plot)
public:
    explicit TWIForSaveMainDoc(PlotForSaveData * aPlot, QTreeWidget * parent = 0);

    QString GetSheetWithX(bool aUpper) const;
};

// xrefs in main list
class TWIForSaveMainXref : public TWIForSaveMain
{
private:
    DefParComplRO(XrefForSaveDataPtr, Xref)
public:
    explicit TWIForSaveMainXref(XrefForSaveData * aXref, QTreeWidget * parent = 0);
};

// additional files in main list
class TWIForSaveMainAddFile : public TWIForSaveMain
{
private:
    DefParComplRORef(DwgForSaveDataPtr, Dwg)
public:
    explicit TWIForSaveMainAddFile(DwgForSaveData * aDwg, QTreeWidget * parent = 0);
};

//-----------------------------------------------------------------------------------
// headers ("Images", "Xrefs", "Add. files") in main list for save dialog
// main reason for exists - operator <
class TWIForSaveHeader : public QTreeWidgetItem
{
public:
    explicit TWIForSaveHeader(const QString &aHeader);
    virtual bool operator<(const QTreeWidgetItem & other) const;
};


//-----------------------------------------------------------------------------------
class TWIForSaveXrefTop : public TWIForSaveMain
{
private:

    DefParComplRORef(XrefForSaveDataPtr, Xref)

    XrefPropsData lXrefPropsDataNULL;
    DefParComplRORef(XrefPropsDataPtr, XrefProps)

    QMap<QString, XrefForSaveData *> mVarList;

    void InitColumns(int aCase);
public:
    explicit TWIForSaveXrefTop(XrefForSaveData * aXref, XrefPropsData * aXrefProps, int aCase, QTreeWidget * parent = 0);

    virtual bool operator<(const QTreeWidgetItem & other) const;

    void SetPlotData(XrefForSaveData * aXref, int aCase) {
        mXref = aXref;

        InitColumns(aCase);
    }

    QMap<QString, XrefForSaveData *> &VarList() { return mVarList; }
signals:

public slots:

};

class TWIForSaveXrefChild : public TWIForSaveMain
{
private:
    DefParComplRORef(XrefForSaveDataPtr, Xref)
    DefParComplRORef(PlotDwgDataPtr, Plot)
public:
    explicit TWIForSaveXrefChild(XrefForSaveData * aXref, PlotDwgData * aPlot, int aCase, QTreeWidget * parent = 0);
    virtual bool operator<(const QTreeWidgetItem & other) const;
signals:

public slots:

};

//-----------------------------------------------------------------------------------
class TWIForSaveAddFile : public QTreeWidgetItem
{
public:
    enum RecordTypeEnum {
        AcadImage, AcadNonImage, NonAcad
    };
protected:
    RecordTypeEnum mRecordType;
    QList<int> mIds;

    DefParComplRO(PlotDwgDataPtr, Plot) // it is main file, that uses the additional file; NULL for top items
    DefParComplRORef(DwgForSaveDataPtr, Dwg) // it is addition file

    void InitColumns(int aInitType); // 0 - image/add. file self, 1 - main file
public:
    explicit TWIForSaveAddFile(DwgForSaveData * aDwgData, RecordTypeEnum aRecordType);
    explicit TWIForSaveAddFile(PlotDwgData * aPlotData, DwgForSaveData * aDwgData, RecordTypeEnum aRecordType);
    virtual bool operator<(const QTreeWidgetItem & other) const;

    RecordTypeEnum RecordType() const { return mRecordType; }

    void SetIds(QList<int> aIds) { mIds = aIds; }
    const QList<int> &Ids() const { return mIds; }
};

#endif // TWIFORSAVE_H
