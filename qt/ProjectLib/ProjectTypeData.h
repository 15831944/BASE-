#ifndef PROJECTTYPEDATA_H
#define PROJECTTYPEDATA_H

#include "../VProject/common.h"
#include "../VProject/CommonData.h"

#include "../PlotLib/PlotData.h"

#if defined(VPROJECT_MAIN_IMPORT)
    #define PROJECT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PROJECT_LIBRARY)
        #define PROJECT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PROJECT_LIBRARY_EXP_IMP
    #endif
#endif

class PROJECT_LIBRARY_EXP_IMP ProjectTypeData
{
    DeclareParRO(int, Id)
    DefParStrRO(TypeName)
    DeclareParRO(int, VerType)
    DeclareParRO(int, VerLen)
    DeclareParRO(bool, VerLenFixed)
    DefParStrRO(VerStart)

    DeclareParRO(int, SheetLen)
    DeclareParRO(int, SheetStart)
    DefParStrRO(DefTempl)
    DefParStrRO(NoNumTempl)

#pragma warning(push)
#pragma warning(disable:4005)

//#define DEFINE_GROUP_NUM 1
//    DefParComplRONum(QList<PlotData *>, Plots)

#undef DEFINE_GROUP_NUM
#pragma warning(pop)

protected:
    QStringList mStages; // simple string list of project stages

private:
    // it is for passing other history than maximum version
    //PlotHistoryData *mSelectedHistory;

    QList <int> mInited;
    void InitGrpData(int aGrpNum);

public:
    ProjectTypeData();
    ProjectTypeData(int aId, const QString &aTypeName, int aVerType, int aVerLen, bool aVerLenFixed, const QString &aVerStart,
                    int aSheetLen, int aSheetStart, const QString &aDefTempl, const QString &aNoNumTempl);

    // groups -------------------------

    // 1
//    void LoadPlots();

    const QStringList &StagesConst() const;
};

class PROJECT_LIBRARY_EXP_IMP ProjectTypeList
{
protected:
    const ProjectTypeData mDefaultProjectTypeData; // ProjectTypeData() used here!
    QList<ProjectTypeData *> mProjTypeList;

    explicit ProjectTypeList();
public:
    static ProjectTypeList * GetInstance();

    const QList<ProjectTypeData *> & ProjTypeListConst();
    const ProjectTypeData *GetById(int aId) const;
    const ProjectTypeData *DefaultProjectTypeData() const;
};

// this object did not self delete on program exit
#define gProjectTypes ProjectTypeList::GetInstance()

#endif // PROJECTTYPEDATA_H

