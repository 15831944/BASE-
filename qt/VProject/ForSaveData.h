#ifndef FORSAVEDATA_H
#define FORSAVEDATA_H

#include "CommonData.h"
#include "../PlotLib/PlotData.h"
#include "XrefPropsData.h"

#include <QString>
#include <QDate>
#include <QList>
#include <QtSql/QSqlQuery>

//class DwgForSaveData;
//typedef DwgForSaveData * DwgForSaveDataPtr;

class XrefForSaveData : public PlotDwgData
{
//public:
//    enum enumPES { PESFree, PESEditing, PESError }; // Plot Edit Status
    // v_plot_simple part
    // id is IdPlot; now no id for xref reco4rd - we needn't it

    // v_xref2dwg part
    DeclareParRO(int, UseWorking)
public:
    explicit XrefForSaveData(int aIdCommon, const QString &aBlockName);
    // it is all data that we need and use in xref
    explicit XrefForSaveData(int aIdPlot, int aIdProject, int aIdCommon, int aIdDwg, int aWorking, int aDeleted, const QString &aVersionInt,
                             const QString &aCode, const QString &aSheet, const QString &aNameTop, const QString &aName,
                             const QDateTime aEditDate,  const QString &aEditUser, const QString &aNotes,
                             int aIdDwgMax, int aDwgVersionMax,
                             int aUseWorking, const QString &aBlockName);
};

typedef XrefForSaveData * XrefForSaveDataPtr;

class PlotForSaveData : public PlotDwgData
{
    DefParComplRO(QList<XrefForSaveData *>, Xrefs) // xrefs list - for autocad drawings only
    DefParComplRO(QList<XrefPropsData *>, XrefsProps) // xrefs props - for autocad drawings only

protected:
    QDateTime mCodeChd, mNameTopChd, mNameChd, mVersionChd, mSheetChd;
    QDateTime mProjNameChd, mProjStageChd;

    enum InitInternalType {
        IITIdPlot, /*IITIdDwg, */IITIdCommon
    };

    void InitInternal(int aId, InitInternalType aType);
public:
    explicit PlotForSaveData(int aIdPlot, int aIdDwg);
    virtual ~PlotForSaveData();

    void InitFromPlotId(int aId);
    void InitFromCommonId(int aId);



    void InitXrefList();
    void InitXrefPropsList();
    void InitFileList(bool aIsDwg); // load corresponding data from XREF; NB: IT IS ALSO LOAD DATA FOR XREFS!

    int NeedUpdateFields() const;

};

typedef PlotForSaveData * PlotForSaveDataPtr;

#endif // FORSAVEDATA_H
