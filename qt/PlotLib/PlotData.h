#ifndef PLOTDATA_H
#define PLOTDATA_H

#include "../VProject/CommonData.h"

#include <QString>
#include <QDate>
#include <QList>
#include <QFileInfo>
#include <QTreeWidgetItem>

#if defined(VPROJECT_MAIN_IMPORT)
    #define PLOT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PLOT_LIBRARY)
        #define PLOT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PLOT_LIBRARY_EXP_IMP
    #endif
#endif

class PlotCommentData;
class PlotHistoryData;
class PlotAddFileData;

class DwgData;
typedef DwgData * DwgDataPtr;
class DwgForSaveData;
typedef DwgForSaveData * DwgForSaveDataPtr;

class CDwgLayout;

class DwgLayoutData;

class ProjectData;
class ProjectTypeData;

class FileType;
class TreeDataRecord;

class XchgFileDataList;

class PLOT_LIBRARY_EXP_IMP PlotData
{
    friend ProjectData;
public:
    enum enumPES { PESFree, PESEditing, PESError }; // Plot Edit Status
    enum PropType { MATIdProject, MATTreeType, MATComplect,
                    /*MATCode, MATSheet,*/ MATBlockName, MATNameTop, MATNameBottom,
                    MATCancelled, MATDeleted };

    enum PlotPropWithCode { PPWCVersionExt, PPWCComplect, PPWCStage, PPWCSheet };

    DeclareParRO(int, Id)
    DeclarePar(int, IdProject)
    DeclareParRO(int, IdCommon)
    DeclarePar(int, TDArea)
    DeclarePar(int, TDId)
    DeclareParRO(quint64, IdDwgMax)
    DeclareParRO(int, DwgVersionMax)

    DeclarePar(int, Working)

    DeclarePar(int, Cancelled)
    DefParComplRO(QDate, CancelDate)
    DefParStrRO(CancelUser)

    DeclarePar(int, Deleted)
    DefParComplRO(QDate, DeleteDate)
    DefParStrRO(DeleteUser)

    DefParStr(VersionInt)
    DefParStr(VersionExt)
    DefParCompl(QDate, SentDate)
    DefParStr(SentUser)
    DefParStr(Section)
    DefParStr(Stage)
    DefParStr(Code)
    DefParStr(Sheet)
    DefParStr(Extension)
    DefParStr(NameTop)
    DefParStr(Name)
    DefParStr(BlockName)

    DefParComplRO(QDateTime, CrDate)
    DefParStrRO(CrUser)

    DefParComplRO(QDateTime, EditDate)
    DefParStrRO(EditUser)

    DeclareParRO(qlonglong, DataLength)
    DeclareParRO(int, XrefsCnt)

    // "NA" means not available
    DeclareParRO(int, EditNA)
    DeclareParRO(int, LoadNA)
    DeclareParRO(int, EditPropNA)
    DeclareParRO(int, DeleteNA)
    DeclareParRO(int, ViewNA)
    DeclareParRO(int, NewVerNA)

    DefParStr(Notes)

#pragma warning(push)
#pragma warning(disable:4005)

#define DEFINE_GROUP_NUM 1
    DefParComplNum(QList<DwgLayoutData *>, Layouts)

#define DEFINE_GROUP_NUM 2
    DefParRONum(enumPES, ES) // das ist mean edit status
    DefParStrRONum(ESUser)

#define DEFINE_GROUP_NUM 3
    DefParRONum(int, AcadVer)

#define DEFINE_GROUP_NUM 4
    DefParComplNum(QList<PlotCommentData *>, Comments)

#define DEFINE_GROUP_NUM 5
    DefParComplNum(QList<PlotData *>, Versions)

#define DEFINE_GROUP_NUM 6
    DefParComplNum(QList<PlotHistoryData *>, History)

#define DEFINE_GROUP_NUM 7
    DefParComplNum(QList<PlotAddFileData *>, AddFiles)

#undef DEFINE_GROUP_NUM
#pragma warning(pop)

private:
    // it is for passing other history than maximum version
    //PlotHistoryData *mSelectedHistory;

    QList <int> mInited;
    void InitGrpData(int aGrpNum);
public:

    explicit PlotData(); // it is need in PlotDwgData
    explicit PlotData(int aId, int aIdProject, int aIdCommon, int aTDArea, int aTDId, int aIdDwgMax, int aDwgVersionMax,
                      int aWorking,
                      int aCancelled, const QDate &aCancelDate, const QString &aCancelUser,
                      int aDeleted, const QDate &aDeleteDate, const QString &aDeleteUser,
                      const QString &aVersionInt, const QString &aVersionExt, const QDate &aSentDate, const QString &aSentUser,
                      const QString &aSection, const QString &aStage, const QString &aCode, const QString &aSheet,
                      const QString &aExtension, const QString &aNameTop, const QString &aName, const QString &aBlockName,
                      const QDateTime &aCrDate, const QString &aCrUser,
                      const QDateTime &aEditDate, const QString &aEditUser,
                      qlonglong aDataLength, int aXrefsCnt,
                      int aEditNA, int aLoadNA, int aEditPropNA, int aDeleteNA, int aViewNA, int aNewVerNA,
                      const QString &aNotes);
    explicit PlotData(int aId);
    virtual ~PlotData();

    PlotHistoryData * GetHistoryById(int aIdHistory);
    PlotHistoryData * GetHistoryByNum(int aHistoryNum);

    QString AcadVerStr();
    FileType * ActualFileType() const;
    int FileType() const;
    bool IsSent();

    bool IsPicture() const;

    void RefreshData();
    bool IsMainInited();

    const QString CodeSheetConst() const;

    void InitIdDwgMax();
    void SetIdDwgMax(quint64 aIdDwgMax);
    void SetDwgVersionMax(int aDwgVersionMax);

    bool MakeVersionActive();
    bool Undelete();

    void static SetPropWithCodeForming(PlotPropWithCode aPPWC, const ProjectTypeData *aProjectType, const QString &aOldVal, const QString &aNewVal,
                                      const QString &aCodeTemplate, QString &aCode);
    void SetPropWithCodeForming(PlotPropWithCode aPPWC, const QString &aNewVal, QString &aCode);
    void SetPropWithCodeForming(PlotPropWithCode aPPWC, const QString &aOldVal, const QString &aNewVal, QString &aCode);
    void static RegenCodeStatic(ProjectData *aProject, const TreeDataRecord *aTreeData,
                                const QString &aComplect, const QString &aStage, const QString &aVersionExt,
                                QStringList &aCodeList, QString &aCodeNew,
                                QStringList &aSheetList, QString &aSheet, bool aSheetSetted,
                                int aIgnoreIdCommon);
    int static CheckCodeDupStatic(ProjectData *aProject, const QString &aVersionExt, const QString &aCode, const QString &aSheet, bool aUseSheet,
                                  const QString &aNameTop, const QString &aNameBottom,
                                  int aIgnoreIdCommon);

    // groups -------------------------

    // 1
    void LoadLayouts();
    //void UninitLayouts();

    // 2
    void InitEditStatus();
    // 3
    void InitAcadVer();

    // 4
    void LoadComments();
    //void UninitComments();

    // 5
    bool LoadVersions();
    void UninitVersions();

    // 6
    void ReinitHistory();

    // 7
    void LoadAddFiles();
    void UninitAddFiles();
    // -------------------------

    bool SaveData();
    void RollbackEdit();
    void CommitEdit();

    // set (save if need) property of this record and all versions

    bool SetPropWithVersions(bool aStartTransaction, bool aSaveData, PropType aType, QList<QVariant> aValues);

    // meaning - use SaveData for this record and for all records from mVersion
    // inside there is now checking is current working or not, because
    // in SetPropWithVersions list mVersion built
    // previously calls of SetPropWithVersions must be with aStartTransaction = false
    bool SaveDataWithVersions();

    static bool INSERT(int &aId, int &aIdCommon, int aIdProject, int aTypeArea, int aType, const QString &aVersionInt, const QString &aVersionExt,
                       const QString &aComplect, const QString &aStage,
                       const QString &aCode, const QString &aSheetNumber, const QString &aNameTop, const QString &aName,
                       const QString &aBlockName, const QString &aComments);

    static bool LOADFROMFILE(bool aIsFile, int aIdPlot, quint64 &aIdDwgMain, quint64 aIdDWGMax, int aDWGMaxVersion, const QFileInfo &aOrigFileInfo, qint64 aOrigFileSize,
                             const QString &aOrigFileHash, const QByteArray &aMainFileData, const QString &aProcessedFileHash,
                             const QList<CDwgLayout *> &aDwgLayouts, XchgFileDataList &aAddFileData,
                             bool aDoNotInsertDwgFile, QWidget *aWaitOwner);

    static bool LOADFROMFILESIMPLE(int aIdPlot, int aDWGMaxVersion, const QString &aFileName);

    static bool LOCKIDPLOT(int aIdPlot);
    static bool STARTEDIT(qulonglong &aNewIdDwgEdit, int aIdPlot, int aIdDwgIn, const QString &aFilename);
    static bool ENDEDIT(qulonglong aIdDwgEdit);
};

typedef PlotData * PlotDataPtr;
Q_DECLARE_METATYPE(PlotData *)

class PlotDwgData : public PlotData
{
    // dwg data
    DefParComplRO(DwgForSaveDataPtr, Dwg)

    DefParComplRORef(QList<DwgForSaveData *>, Images)
    DefParComplRORef(QList<DwgForSaveData *>, AddFiles)
public:
    PlotDwgData(int aIdDwg);
    ~PlotDwgData();
};

typedef PlotDwgData * PlotDwgDataPtr;

class PlotCommentData
{
    DeclareParRO(int, Id)
    DeclareParRO(int, IdPlot)

    DefParComplRO(QDate, Date)
    DefParStrRO(User)
    DefParStrRO(Comment)
private:
    bool mIsNew;
public:
    explicit PlotCommentData(int aId, const QDate &aDate, const QString &aUser, const QString &aComment);
    explicit PlotCommentData(int aIdPlot, const QString &aComment);

    bool SaveData();
    bool RefreshData();
};

class PLOT_LIBRARY_EXP_IMP PlotHistoryData
{
    DeclareParRO(int, Id) // it is id from dwg
    DeclareParRO(int, Num) // version field from dwg
    DeclareParRO(int, IdPlot)

    DeclareParRO(int, Type) // -1 - error, 0 - type from dwg_edit, 100 - dwg_file

    DefParStrRO(User)
    DefParComplRO(QDateTime, When)
    DefParStrRO(Computer)
    DefParStrRO(IpAddr)

    DefParComplRO(QDateTime, StartTime)
    DefParComplRO(QDateTime, EndTime)
    DeclareParRO(int, IdleSec)

    DeclareParRO(int, EntChanged)
    DeclareParRO(int, EntAdded)
    DeclareParRO(int, EntDeleted)

    DeclareParRO(int, SaveCount)
    DefParComplRO(QDateTime, LastSave)

    DefParComplRO(QDateTime, FTime)

    DeclareParRO(qlonglong, DataLength)
    DeclareParRO(int, XrefsCnt)

    DefParStrRO(Ext)
    DefParStrRO(WorkingFileName)
    DefParStrRO(SavedFromFileName)
    DeclareParRO(qlonglong, FileSize)

    DeclareParRO(int, NeedNotProcess)

    DeclareParRO(int, FromIdPlot)
    DeclareParRO(int, FromVersion)

#pragma warning(push)
#pragma warning(disable:4005)

#define DEFINE_GROUP_NUM 1
    DefParComplNum(QList<PlotAddFileData *>, AddFiles)

#define DEFINE_GROUP_NUM 2
    DefParRONum(int, AcadVer)

#undef DEFINE_GROUP_NUM
#pragma warning(pop)

private:
    QList <int> mInited;
    void InitGrpData(int aGrpNum);
public:
    explicit PlotHistoryData(int aId, int aNum, int aIdPlot, int aType, const QString &aUser, const QDateTime &aWhen,
                             const QString &aComputer, const QString &aIpAddr,
                             const QDateTime &aStartTime, const QDateTime &aEndTime, int aIdleSec, int aEntChanged, int aEntAdded, int aEntDeleted,
                             int aSaveCount, const QDateTime &aLastSave, const QDateTime &aFTime, qlonglong aDataLength, int aXrefsCnt,
                             const QString &aExt, const QString &aWorkingFileName, const QString &aSavedFromFileName, qlonglong aFileSize, int aNeedNotProcess,
                             int aFromIdPlot, int aFromVersion);

    bool IsPicture() const;
    QString AcadVerStr();

    // groups -------------------------

    // 1
    void LoadAddFiles();
    void UninitAddFiles();

    // 2
    void InitAcadVer();
};

class PLOT_LIBRARY_EXP_IMP PlotAddFileData
{
    DeclareParRO(int, Id)
    DeclareParRO(int, IdLob)
    DefParStr(Name)
    DeclareParRO(int, Version)
    DeclareParRO(qlonglong, DataLength)
    DefParComplRO(QDateTime, FTime)
public:
    explicit PlotAddFileData(int aId, int aIdLob, const QString &aName, int aVersion, qlonglong aDataLength, const QDateTime &aFTime);

    bool IsPicture() const;
};

typedef QPair<PlotData *, PlotHistoryData *> PlotAndHistoryData;
typedef QPair<PlotData *, TreeDataRecord *> tPlotAndTreeData;

Q_DECLARE_METATYPE(PlotHistoryData *)

class PlotNamedListData
{
    DeclareParRO(int, Id)
    DefParStr(Name)

    DefParCompl(QList<int>, IdsCommon)
public:
    explicit PlotNamedListData(int aId, const QString &aName, bool aGetList);

    static PlotNamedListData * INSERT(int &aId, int aIdProject, const QString &aName);
    static bool DODELETE(int aId);

    bool SaveData();
    void RollbackEdit();
    void CommitEdit();
};

Q_DECLARE_METATYPE(PlotNamedListData *)

#undef PLOT_LIBRARY_EXP_IMP

#endif // PLOTDATA_H
