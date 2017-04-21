#ifndef PROJECTDATA_H
#define PROJECTDATA_H

#include "../VProject/common.h"
#include "../VProject/CommonData.h"

#if defined(VPROJECT_MAIN_IMPORT)
    #define PROJECT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PROJECT_LIBRARY)
        #define PROJECT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PROJECT_LIBRARY_EXP_IMP
    #endif
#endif

class PlotData;
class PlotHistoryData;
typedef QPair<PlotData *, PlotHistoryData *> PlotAndHistoryData;

class PlotNamedListData;
class ProjectTypeData;

class PROJECT_LIBRARY_EXP_IMP ProjectData
{
public:
    enum PDType { PDProject, PDGroup };
private:
    bool mIsNew;
    ProjectData * mParent;
    // main data part
    DefParStr(ShortName)
    DeclareParRO(long, Id)
    DeclarePar(long, IdParentProject)
    DeclareParRO(PDType, Type)

    // for tree
    DeclareParRO(int, InUserList)
    DeclareParRO(int, Recently)
    DeclareParRO(int, Archived)
    DeclareParRO(int, Rights)

    // full data
#pragma warning(push)
#pragma warning(disable:4005)

    DeclarePar(long, IdGroup)
    DeclarePar(long, IdCustomer)
    DeclarePar(long, IdProjType)
    DefParStr(ShortNum)

    DefParStr(Contract)
    DefParCompl(QDate, ContractDate)

    DefParStr(Stage)
    DefParStr(Name)
    DefParStr(Gip)
    DefParComplRO(QDate, StartDate)
    DefParStrRO(CrUser)
    DefParComplRO(QDate, EndDate)
    DefParStrRO(EndUser)
    DefParStr(Comments)

    DefParStr(CodeTemplate)
    DeclarePar(int, SheetDigits)

    DefParComplRO(QDateTime, PropChDate)

    // nested projects
    DefParCompl(QList<ProjectData *>, ProjList)

#define DEFINE_GROUP_NUM 1
    // projects documents
    DefParComplNum(QList<PlotData *>, PlotList)
    // two list in one group, inited in one function
    DefParComplNum(QStringList, ComplectList)
    // named lists of documents
    DefParComplNum(QList<PlotNamedListData *>, NamedLists)
#undef DEFINE_GROUP_NUM
#pragma warning(pop)

private:
    QList <int> mInited;
    void InitGrpData(int aGrpNum);

    inline void CheckListsInternal();

    bool AddToMyListInternal(bool aWithSub);
public:
    // not used, commented
/*    explicit ProjectData(const QString &aShortName, long aId, long aIdParentProject, PDType aType, int aInUserList, int aRecently,
                         int aArchived, int aRights, ProjectData * aParent = NULL);*/
    // for all constructor (used union in select, so it has all data)
    explicit ProjectData(const QString &aShortName, long aId, long aIdParentProject, PDType aType, int aInUserList, int aRecently,
                         int aArchived, int aRights,
                         long aIdGroup, long aIdCustomer, long aIdProjType,
                         const QString &aShortNum, const QString &aContract, const QDate &aContractDate,
                         const QString &aStage, const QString &aName, const QString &aGip,
                         const QDate &aStartDate, const QString &aCrUser, const QDate &aEndDate, const QString &aEndUser,
                         const QString &aComments, const QString &aCodeTemplate, int aSheetDigits,
                         const QDateTime &aPropChDate, ProjectData * aParent = NULL);
    // this constructor used on group update
    explicit ProjectData(long aId, const QString &aShortName, const QDateTime &aPropChDate);
    explicit ProjectData(long aId, PDType aType);
    ~ProjectData();

    bool GetActualContract(QString &aContract, QDate &aContractDate) const;
    void CodeTempleReplaceWithDataMain(QString &aCodeTempl);
    void CodeTempleReplaceWithDataSub(QString &aCodeTempl);
    QString GenerateFixedCode(const QString &aCodeTempl, int aAddNum, int aIgnoreIdCommon);

    const ProjectTypeData *ProjectType() const;

    void SetAllData(const QString &aShortName, long aId, long aIdParentProject, PDType aType, int aInUserList, int aRecently,
                    int aArchived, int aRights,
                    long aIdGroup, long aIdCustomer, long aIdProjType,
                    const QString &aShortNum, const QString &aContract, const QDate &aContractDate,
                    const QString &aStage, const QString &aName, const QString &aGip,
                    const QDate &aStartDate, const QString &aCrUser, const QDate &aEndDate, const QString &aEndUser,
                    const QString &aComments, const QString &aCodeTemplate, int aSheetDigits,
                    const QDateTime &aPropChDate/*, ProjectData * aParent*/);
    void SetAllData(long aId, const QString &aShortName, const QDateTime &aPropChDate);
    void RefreshData();

    ProjectData *Parent() { return mParent; }
    void setParent(ProjectData * aParent);

    bool IsPlotListInited();

    QList<PlotData *> GetPlotsByTreeData(int aTreeDataArea, int aTreeDataId);
    PlotData * GetPlotById(int aIdPlot, bool aUseVersions = false);
    PlotData * GetPlotByIdNotLoad(int aIdPlot, bool aUseVersions = false);
    PlotData * GetPlotByIdCommon(int aIdCommon);

    QString FullShortName(bool aNoGroup = false) const;
    int SheetDigitsActual() const;

    bool AddToMyList(bool aWithSub);
    bool RemoveFromMyList(); // true means "something updated"

    void ReinitLists();

    bool RemoveFromDB();
    bool SaveData();
    void RollbackEdit();
    void CommitEdit();

    // --
    void ShowProps(QWidget *parent = 0);

    static int NewGroup(QWidget *parent = 0);
    static int NewProject(ProjectData *aParentGroup, QWidget *parent = 0);
    int NewConstruction(QWidget *parent = 0);
};

// QObject cos we use signal-slot system
class PROJECT_LIBRARY_EXP_IMP ProjectList : public QObject
{
    Q_OBJECT
protected:
    QSqlQuery *mQMainList, *mQSubProjects, *mQGroupChilds;
    QSqlQuery *mQUpdateProjects, *mQUpdateGroups;
    //DefParCompl(QList<ProjectData *>, ProjList)
    bool mProjListError;
    QList<ProjectData *> mProjList;

    QList<int> mPlotListInUpdate;
    QList<PlotData *> mPlotsInUpdate;

    explicit ProjectList();
    virtual ~ProjectList();

//    static void clean() {
//        delete GetInstance();
//    }

    void ClearError();
    void InitProjectListInternal(ProjectData *aParent = NULL);
    ProjectData * FindByIdProjectInternal(long aIdProject, const QList<ProjectData *> &aProjList, ProjectData::PDType aType) const;

    PlotData * FindByIdPlotInternal(long aIdPlot, const QList<ProjectData *> &aProjList, bool aUseVersions);
public:
    static ProjectList * GetInstance();

    void InitProjectList(bool aClearError);

    void UpdateProjectInList();
    void UpdateGroupInList();
    void RemoveProjectFromList();
    void RemoveGroupFromList();
    void UpdatePlotList(long aIdProject);

    QList<ProjectData *> & ProjListRef();
    const QList<ProjectData *> & ProjListConst();

    ProjectData * FindByIdProject(long aIdProject);
    ProjectData * FindByIdGroup(long aIdGroup);
    QString ProjectFullShortName(long aIdProject);

    PlotData * FindByIdPlot(long aIdPlot);

    void EmitPlotListBeforeUpdate(int aIdProject); // 0 - all projects
    void EmitPlotListNeedUpdate(int aIdProject); // 0 - all projects
    bool IsPlotListInUpdate(int aIdProject);

    void EmitPlotBeforeUpdate(PlotData *lPlot, int aType); // aType = group in PlotData
    void EmitPlotNeedUpdate(PlotData *lPlot, int aType); // aType = group in PlotData
signals:
    // update all opened projects list (only lists, no documents on it or something else)
    // it is used after adding/removing projects and after adding removing from lists, etc
    void ProjectListNeedUpdate();

    void PlotListBeforeUpdate(int aIdProject); // 0 - all projects
    void PlotListNeedUpdate(int aIdProject); // 0 - all projects

    void PlotBeforeUpdate(PlotData *lPlot, int aType); // aType = group in PlotData
    void PlotNeedUpdate(PlotData *lPlot, int aType); // aType = group in PlotData

    void PlotsNamedListNeedUpdate(PlotNamedListData *);

    void PlotBecameSelected(PlotData *);
    void PlotHistoryBecameSelected(PlotData *, PlotHistoryData *);

    void PlotsBecameSelected(QList<PlotAndHistoryData>);
};

// this object did not self delete on program exit
#define gProjects ProjectList::GetInstance()

#endif // PROJECTDATA_H
