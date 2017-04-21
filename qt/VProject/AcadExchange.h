#ifndef ACADEXCHANGE_H
#define ACADEXCHANGE_H

#include "XrefPropsData.h"
#include "../PlotLib/PlotData.h"

#include <QWidget>

#define NOMINMAX
#include <windows.h>

#include "def_expimp.h"

#define lpRemoveInvisible   0x000001
#define lpColorBlocks       0x000002
#define lpColorEntities     0x000004
#define lpLWBlocks          0x000008
#define lpLWEntities        0x000010
#define lpRenameLayer0      0x000020
#define lpRemoveAllXrefs    0x000040
#define lpClearAnnoScales   0x000080
#define lpPurgeRegapps      0x000100
#define lpPurgeAll          0x000200
#define lpExplodeAllProxies 0x000400
#define lpRemoveAllProxies  0x000800
#define lpAudit             0x001000
#define lpSwitchToModel     0x002000
#define lpSetWCS            0x004000
#define lpViewWCS           0x008000
#define lpZoomE             0x010000
#define lpUserCommands      0x020000
#define lpManualClose       0x040000
#define lpRestartAfterEach  0x080000

#define cmpWithXrefs        0x000001
#define cmpOutputDates      0x000002

class EXP_IMP RecordDataForCopyToAcad {
public:
    enum enumSP { eSP };
    enum enumVE { eVE };
    enum enumLP { eLP };
    enum enumAP { eAP };
    enum enumPUB { ePUB };
    enum enumCMP { eCMP };
    enum enumRT { eRT };
protected:
    struct s1Type {
        union {
            // for after save process
            struct {
                wchar_t FileName[1024];
                ULONG ProcessType; // flags what to process, see in project loading, file CSaveProcessIterator
                ULONG IdDwg; // for getting more info in AutoCAD; primary need of this is update fields
            } SP; // SP for save process

            // for view-edit
            struct {
                ULONG IdPlot, IdDwgForPlot, IdDwgEditForPlot;
                bool SkipWarning;
            } VE; // VE for view/edit

            // for load process
            struct {
                wchar_t FileName[1024];
            } LP; // LP for load process

            struct {
                ULONG IdPlot;
            } AP; // AP for audit & purge

            struct {
                ULONG IdPlot, IdDwgForPlot;
                quint64 Layouts; // layouts - by bits
                ULONG IdPubDwg; // it is not input data
            } PUB; // PUB for publish

            struct {
                ULONG IdPlotOld, IdDwgOld, HistNumOld, IdPlotNew, IdDwgNew, HistNumNew;
                wchar_t DateOld[24], DateNew[24];
            } CMP; // CMP for compare

            struct {
                ULONG IdPlot;
            } RT; // RT for replace text
        };
    } s1;

    XrefPropsData * mXrefPropsData; // it's a copy, do not delete it
    QList<XrefRenameData *> mXrefRenameList, mImageRenameList; // it is need to delete
public:
    // SP (twice)
    explicit RecordDataForCopyToAcad(enumSP, const QString &aFileName, long aProcessType, XrefPropsData * aXrefPropsData,
                                     const QList<XrefRenameData *> &aImageRenameList, ULONG aIdDwg = 0);
    explicit RecordDataForCopyToAcad(enumSP, const QString &aFileName, long aProcessType, const QList<XrefRenameData *> &aXrefRenameList,
                                     const QList<XrefRenameData *> &aImageRenameList, ULONG aIdDwg);
    // VE
    explicit RecordDataForCopyToAcad(enumVE, ULONG aIdPlot, ULONG aIdDwgForPlot, ULONG aIdDwgEditForPlot, bool aSkipWarning);
    // LP
    explicit RecordDataForCopyToAcad(enumLP, const QString &aFileName);
    // AP
    explicit RecordDataForCopyToAcad(enumAP, ULONG aIdPlot);
    // PUB
    explicit RecordDataForCopyToAcad(enumPUB, ULONG aIdPlot, ULONG aIdDwgForPlot, quint64 aLayouts);
    // CMP
    explicit RecordDataForCopyToAcad(enumCMP, const PlotHistoryData *aHistoryOld, const PlotHistoryData *aHistoryNew);
    // RT
    explicit RecordDataForCopyToAcad(enumRT, ULONG aIdPlot);
    virtual ~RecordDataForCopyToAcad();

    void *GetDataBuffer();
    static long GetDataSize();

    XrefPropsData * XrefPropsRef();
    const XrefPropsData * XrefPropsConst() const;

    QList<XrefRenameData *> &XrefRenameListRef();
    QList<XrefRenameData *> &ImageRenameListRef();
};

class EXP_IMP MainDataForCopyToAcad {
protected:
    struct s1Type {
        // 0 - process drawing for (after, in fact) save, 1 - view, 2 - edit, 3 - simple (old) process for loading,
        // 4 - audit/purge, 5 - publish, 6 - recover, 7 - compare, 8 - replace text
        long mWhatToDo;
        quint64 mWinId; // the size of HWND in 64 bit is 64 bit

        union {
            struct {
                long SaveAcadVersion;
            } SAVE;
            struct {
                long WithoutXrefs;
            } VIEW;
            struct {
                ULONG ProcessType; // flags what to process, #define lpXXXXX
                ULONG SaveType; // 0 - open oroginal then _SAVEAS; 1 - copy from original to temp, open it, then _QSAVE
                // here can be version of AutoCAD for save (it is can be only with SaveType = 0)
                ULONG ColorBlocks, ColorEntities, LWBlocks, LWEntities;
                wchar_t Layer0Name[40], UserCommands[1030];
            } LOAD;
            struct {
                long PurgeRegapps, PurgeAll, ExplodeProxy, RemoveProxy, Audit;
            } AUDIT;
            struct {
                long PDF, DWF, PLT/*, DWG, MakeXls*/;
                long DontScale;
                ULONG PDFDiagn, DWFDiagn, PLTDiagn;
                long UseVersion, CTBType;
                wchar_t CTBName[64], PlotterName[256], OutDir[512];
            } PUBLISH;
            struct {
                ULONG Version, Type;
                ULONG OldColor, NewColor;
                wchar_t OutDir4Pdf[512];
            } COMPARE;
            struct {
                wchar_t FindWhat[1024], ReplaceWith[1024];
                long MoveType; // 0 - no, 1 - absolutely, 2 - text height multiplier
                double DX, DY;
            } REPLACETEXT;
        };
    } s1;

    QList<RecordDataForCopyToAcad *> mList;
public:
    explicit MainDataForCopyToAcad(long aWhatToDo, WId aWinId = 0);
    // for view-edit
    explicit MainDataForCopyToAcad(long aWhatToDo, bool aWithoutXrefs);
    // preload
    explicit MainDataForCopyToAcad(ULONG aProcessType, ULONG aColorBlocks, ULONG aColorEntities, ULONG aLWBlocks, ULONG aLWEntities,
                                   const QString &aLayer0Name,  const QString &aUserCommands,  WId aWinId = 0);
    // audit constructor
    explicit MainDataForCopyToAcad(long aPurgeRegapps, long aPurgeAll, long aExplodeProxy, long aRemoveProxy, long aAudit, WId aWinId = 0);
    // publish constructor
    explicit MainDataForCopyToAcad(long aPDF, long aDWF, long aPLT, long aDontScale, long aUseVersion, long aCTBType,
                                   const QString &aCTBName, const QString &aPlotterName, const QString &aOutDir, WId aWinId = 0);
    // compare constructor
    explicit MainDataForCopyToAcad(ULONG aVersion, ULONG aType, const QString &aOutDit4Pdf, WId aWinId = 0);
    // replace text constructor
    explicit MainDataForCopyToAcad(const QString &aFindWhat, const QString &aReplaceWith,
                                   long aMoveType, double aDX, double aDY, WId aWinId = 0);
    virtual ~MainDataForCopyToAcad();

    void *GetDataBuffer();
    static long GetDataSize();

    long WhatToDo() const;

    void SetSaveAcadVersion(long aSaveAcadVersion);
    QList<RecordDataForCopyToAcad *> &ListRef();
    const QList<RecordDataForCopyToAcad *> &ListConst() const;
};


class RecordDataFromAcad {
public:
    enum enumDataType { MainFile = 0, MainFileEnd, AdditionalFile, XrefFile, PublishData };
protected:
    struct s1Type {
        enumDataType mDataType;
        union {
            // main file
            struct {
                // new file name in temp directory
                wchar_t mFileNameOld[1024], mFileNameNew[1024];
            } MF; // MF for main file

            // additional file
            struct {
                wchar_t mFileName[1024], mGroup[72];
            } AF; // AF for additional file

            // xref file
            struct {
                // not done yet
            } XF; // XF for xref file

            struct {
                int Id, WhatToDo;
            } PUB; // id for publish
        };
    } s1;

public:
    explicit RecordDataFromAcad();

    void *GetDataBuffer();
    static long GetDataSize();

    enumDataType DataType() const;

    QString MFFileNameOld() const;
    QString MFFileNameNew() const;

    QString AFFileName() const;
    QString AFGroup() const;

    int PUBId() const;
    int PUBWhatToDo() const;
};

#endif // ACADEXCHANGE_H
