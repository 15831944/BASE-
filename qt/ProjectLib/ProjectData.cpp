#include "ProjectData.h"
#include "ProjectTypeData.h"

#include "ProjectPropDlg.h"
#include "ContsrPropDlg.h"

#include "../VProject/common.h"

#include "../VProject/MainWindow.h"
#include "../VProject/GlobalSettings.h"

#include <QInputDialog>

#include <QSqlRecord>

QDateTime gProjMaxModTime, gGroupMaxModTime, gProjMaxDelTime, gGroupMaxDelTime;

/*ProjectData::ProjectData(const QString &aShortName, long aId, long aIdParentProject, PDType aType, int aInUserList, int aRecently, int aArchived,
                         int aRights, ProjectData *aParent) :
    InitPar(ShortName), InitParRO(Id), InitPar(IdParentProject), InitPar(Type), InitParRO(InUserList), InitParRO(Recently), InitParRO(Archived),
    InitParRO(Rights), mParent(aParent)
{
}*/

ProjectData::ProjectData(const QString &aShortName, long aId, long aIdParentProject, PDType aType, int aInUserList, int aRecently,
                         int aArchived, int aRights,
                         long aIdGroup, long aIdCustomer, long aIdProjType,
                         const QString &aShortNum, const QString &aContract,
                         const QDate &aContractDate, const QString &aStage, const QString &aName, const QString &aGip,
                         const QDate &aStartDate, const QString &aCrUser, const QDate &aEndDate, const QString &aEndUser,
                         const QString &aComments, const QString &aCodeTemplate, int aSheetDigits,
                         const QDateTime &aPropChDate, ProjectData *aParent) :
    mIsNew(false),
    InitPar(ShortName), InitParRO(Id), InitPar(IdParentProject), InitParRO(Type), InitParRO(InUserList), InitParRO(Recently),
    InitParRO(Archived), InitParRO(Rights),
    InitPar(IdGroup), InitPar(IdCustomer), InitPar(IdProjType),
    InitPar(ShortNum), InitPar(Contract), InitPar(ContractDate), InitPar(Stage), InitPar(Name), InitPar(Gip),
    InitParRO(StartDate), InitParRO(CrUser), InitParRO(EndDate), InitParRO(EndUser),
    InitPar(Comments), InitPar(CodeTemplate), InitPar(SheetDigits),
    InitParRO(PropChDate),
    mParent(aParent)
{
    switch (mType) {
    case PDProject:
        if (mPropChDate > gProjMaxModTime) gProjMaxModTime = mPropChDate;
        break;
    case PDGroup:
        if (mPropChDate > gGroupMaxModTime) gGroupMaxModTime = mPropChDate;
        break;
    }
}

ProjectData::ProjectData(long aId, const QString &aShortName, const QDateTime &aPropChDate) :
    mIsNew(true),
    mType(PDGroup),
    InitParRO(Id), InitPar(ShortName), InitParRO(PropChDate),
    mInUserList(-1), mRecently(-1), mArchived(-1), mRights(-1),
    mParent(NULL)
{
    if (mPropChDate > gGroupMaxModTime) gGroupMaxModTime = mPropChDate;
}

ProjectData::ProjectData(long aId, PDType aType) :
    mIsNew(true),
    InitParRO(Id), InitParRO(Type),
    mInUserList(-1), mRecently(-1), mArchived(-1), mRights(-1),
    mParent(NULL)
{
    setIdParentProject(0, true);
    setIdGroup(0, true);
    setIdCustomer(0, true);
    setIdProjType(0, true);
    setSheetDigits(0, true);
}

ProjectData::~ProjectData() {
    qDeleteAll(mProjList);
    qDeleteAll(mPlotList);
    qDeleteAll(mNamedLists);
}

bool ProjectData::GetActualContract(QString &aContract, QDate &aContractDate) const {
    bool res = false;
    int i;

    if (mParent
            && mParent->mType == PDProject) {
        return mParent->GetActualContract(aContract, aContractDate);
    }

    switch (gSettings->ContractMode) {
    case 0: // from contract table (not realized yet)
        break;
    case 1: // from property or short name
        if (!mContract.isEmpty()) {
            // from project property
            aContract = mContract;
            aContractDate = mContractDate;
            res = true;
        } else {
            // get contract from short project name
            aContract = mShortName;
            for (i = 0; i < aContract.length(); i++) {
                if (!i && aContract[i] != 'P') { // must start with P
                    aContract.clear();
                    break;
                } else if (i <= 1 && (aContract[i] == '-' || aContract[i] >= '0' && aContract[i] <= '9')) {
                    // second character is "minus" or digit
                    continue;
                } else if (i > 1 && (aContract[i] < '0' || aContract[i] > '9') && aContract[i] != '.') {
                    // until not digit; get all left part (with "P" and minus if it exists)
                    aContract = aContract.left(i);
                    break;
                }
            }
            res = true;
        }

        break;
    }
    return res;
}

void ProjectData::CodeTempleReplaceWithDataMain(QString &aCodeTempl) {
    QString lContract;
    QDate lContractDate;

    GetActualContract(lContract, lContractDate);

    if (!lContract.isEmpty()) {
        aCodeTempl.replace("%CONT%", lContract);
    }
    while (aCodeTempl.indexOf("--") != -1) aCodeTempl.replace("--", "-");
}

void ProjectData::CodeTempleReplaceWithDataSub(QString &aCodeTempl) {
    aCodeTempl.replace("%CN%", mShortNum);
    while (aCodeTempl.indexOf("--") != -1) aCodeTempl.replace("--", "-");
}

QString ProjectData::GenerateFixedCode(const QString &aCodeTempl, int aAddNum, int aIgnoreIdCommon) {
    QString lStr1, /*lRes = aCodeTempl, */lCodeExisting, lTemplStart, lTemplEnd;

    CheckListsInternal();

    // it is RegExp string for comparing
    lStr1 = aCodeTempl;
    lStr1.replace(QRegExp("%N*%"), "[0-9]*");
    lStr1.replace(QRegExp("\\$N*\\$"), "[0-9]*"); // temporary line
    lStr1.replace("$", "\\$");
    lStr1 = "^" + lStr1 + "$";

    // it is template begin
    lTemplStart = aCodeTempl;
    lTemplStart.replace(QRegExp("%N.*$"), "");
    lTemplStart.replace(QRegExp("\\$N.*$"), ""); // temporary line

    // it is template end
    lTemplEnd = aCodeTempl;
    lTemplEnd.replace(QRegExp("^.*N%"), "");
    lTemplEnd.replace(QRegExp("^.*N\\$"), ""); // temporary line

    int lNum = 0;

    for (int i = 0; i < mPlotList.length(); i++) {
        if (mPlotList.at(i)->IdCommon() == aIgnoreIdCommon) continue;
        if (mPlotList.at(i)->CodeConst().contains(QRegExp(lStr1))) {
            lCodeExisting = mPlotList.at(i)->CodeConst();
            if (!lTemplStart.isEmpty()) {
                lCodeExisting = lCodeExisting.mid(lTemplStart.length());
            }
            if (!lTemplEnd.isEmpty()) {
                lCodeExisting = lCodeExisting.left(lCodeExisting.length() - lTemplEnd.length());
            }
            if (lCodeExisting.contains(QRegExp("^[0-9]*$"))) {
                if (lCodeExisting.toInt() > lNum) lNum = lCodeExisting.toInt();
            }
        }
    }

    QString lNumStr = QString::number(lNum + 1 + aAddNum);

    // 2 - for '%' sign on both ends
    while (lNumStr.length() < aCodeTempl.length() - lTemplStart.length() - lTemplEnd.length() - 2)
        lNumStr = "0" + lNumStr;

    return lTemplStart + lNumStr + lTemplEnd;
}

const ProjectTypeData *ProjectData::ProjectType() const {
    long lIdprojType = mIdProjType;
    const ProjectData *aParent = this;
    while ((aParent = aParent->mParent) && aParent->mType == ProjectData::PDProject) {
        lIdprojType = aParent->mIdProjType;
    }
    return gProjectTypes->GetById(lIdprojType);
}

// in fact it is for project only; some values is strictly unchanged (type & id), but it is as is
void ProjectData::SetAllData(const QString &aShortName, long aId, long aIdParentProject, PDType aType, int aInUserList, int aRecently,
                             int aArchived, int aRights,
                             long aIdGroup, long aIdCustomer, long aIdProjType,
                             const QString &aShortNum, const QString &aContract, const QDate &aContractDate,
                             const QString &aStage, const QString &aName, const QString &aGip,
                             const QDate &aStartDate, const QString &aCrUser, const QDate &aEndDate, const QString &aEndUser,
                             const QString &aComments, const QString &aCodeTemplate, int aSheetDigits,
                             const QDateTime &aPropChDate/*, ProjectData * aParent*/) {
    SetPar(ShortName);
    SetParRO(Id);
    SetPar(IdParentProject);
    SetParRO(Type);
    SetParRO(InUserList);
    SetParRO(Recently);
    SetParRO(Archived);
    SetParRO(Rights);
    SetPar(IdGroup);
    SetPar(IdCustomer);
    SetPar(IdProjType);
    SetPar(ShortNum);
    SetPar(Contract);
    SetPar(ContractDate);
    SetPar(Stage);
    SetPar(Name);
    SetPar(Gip);
    SetParRO(StartDate);
    SetParRO(CrUser);
    SetParRO(EndDate);
    SetParRO(EndUser);
    SetPar(Comments);
    SetPar(CodeTemplate);
    SetPar(SheetDigits);
    SetParRO(PropChDate);

    if (mPropChDate > gProjMaxModTime) gProjMaxModTime = mPropChDate;

//    mParent = aParent;
}

// for group
void ProjectData::SetAllData(long aId, const QString &aShortName, const QDateTime &aPropChDate) {
    SetParRO(Id);
    SetPar(ShortName);

    SetParRO(PropChDate);

    if (mPropChDate > gGroupMaxModTime) gGroupMaxModTime = mPropChDate;
}

void ProjectData::RefreshData() {
    if (mIsNew) return;

    QSqlQuery queryProject(db);

    if (mType == PDProject) {

        queryProject.prepare(
                    "SELECT 0 as type, ID, ID_PROJECT, SHORTNAME,"
                    " (SELECT COUNT(*) FROM V_PROJECT_USER WHERE ID_PROJECT = A.ID AND LOGIN = USER) as IN_USER_LIST,"
                    " CASE WHEN PLOT_CHDATE > CURRENT_DATE - 14 THEN 1 ELSE 0 END as RECENTLY,"
                    " ARCHIVED, projright, id_group, id_customer, id_projtype, shortnum, dogovor, dogdate, stage, name,"
                    " gip, startdate, cruser, enddate, enduser, comments,"
                    " code_template, sheet_digits, prop_chdate"
                    " FROM V_PROJECT A"
                    " WHERE ID = :ID");

        if (queryProject.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Project data"), queryProject);
        } else {
            queryProject.bindValue(":ID", mId);
            if (!queryProject.exec()) {
                gLogger->ShowSqlError(QObject::tr("Project data"), queryProject);
            } else {
                if (queryProject.next()) {
                    setIdParentProject(queryProject.value("id_project").toInt(), true);
                    setShortName(queryProject.value("shortname").toString(), true);

                    mInUserList = queryProject.value("in_user_list").toInt();
                    mRecently = queryProject.value("recently").toInt();
                    mArchived = queryProject.value("archived").toInt();
                    mRights = queryProject.value("projright").toInt();

                    setIdGroup(queryProject.value("id_group").toInt(), true);
                    setIdCustomer(queryProject.value("id_customer").toInt(), true);
                    setIdProjType(queryProject.value("id_projtype").toInt(), true);
                    setShortNum(queryProject.value("shortnum").toString(), true);

                    setContract(queryProject.value("dogovor").toString(), true);
                    setContractDate(queryProject.value("dogdate").toDate(), true);

                    setStage(queryProject.value("stage").toString(), true);
                    setName(queryProject.value("name").toString(), true);
                    setGip(queryProject.value("gip").toString(), true);

                    mStartDate = queryProject.value("startdate").toDate();
                    mCrUser = queryProject.value("cruser").toString();

                    mEndDate = queryProject.value("enddate").toDate();
                    mEndUser = queryProject.value("enduser").toString();

                    setComments(queryProject.value("comments").toString(), true);
                    setCodeTemplate(queryProject.value("code_template").toString(), true);
                    setSheetDigits(queryProject.value("sheet_digits").toInt(), true);

                    mPropChDate = queryProject.value("prop_chdate").toDateTime();
                    if (mPropChDate > gProjMaxModTime) gProjMaxModTime = mPropChDate;
                } else {
                    gLogger->ShowError(QObject::tr("Project data"),
                                          QObject::tr("Data not found") + "\nv_project: id = " + QString::number(mId));
                }
            }
        }
    }
}

void ProjectData::setParent(ProjectData * aParent) {
    if (mParent) {
        mParent->ProjListRef().removeAll(this);
    } else {
        gProjects->ProjListRef().removeAll(this);
    }
    mParent = aParent;
    if (mParent) {
        mParent->ProjListRef().append(this);
        std::sort(mParent->ProjListRef().begin(), mParent->ProjListRef().end(),
                  [] (const ProjectData * d1, const ProjectData * d2) { return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst()); });
    } else {
        gProjects->ProjListRef().append(this);
        std::sort(gProjects->ProjListRef().begin(), gProjects->ProjListRef().end(),
                  [] (const ProjectData * d1, const ProjectData * d2) { return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst()); });
    }
}

bool ProjectData::IsPlotListInited() {
    return mInited.contains(1);
}

QList<PlotData *> ProjectData::GetPlotsByTreeData(int aTreeDataArea, int aTreeDataId) {
    QList<PlotData *> lRes;

    CheckListsInternal();

    for (int i = 0; i < mPlotList.length(); i++)
        if (mPlotList.at(i)->TDArea() == aTreeDataArea && mPlotList.at(i)->TDId() == aTreeDataId)
            lRes.append(mPlotList.at(i));

    return lRes;
}

PlotData * ProjectData::GetPlotByIdNotLoad(int aIdPlot, bool aUseVersions) {
    int i, j;

    for (i = 0; i < mPlotList.length(); i++) {
        if (mPlotList.at(i)->Id() == aIdPlot)
            return mPlotList.at(i);
        if (aUseVersions)
            for (j = 0; j < mPlotList.at(i)->mVersions.length(); j++)
                if (mPlotList.at(i)->mVersions.at(j)->Id() == aIdPlot)
                    return mPlotList.at(i)->mVersions.at(j);
    }

    return NULL;
}

PlotData * ProjectData::GetPlotById(int aIdPlot, bool aUseVersions) {
    int i, j;
    CheckListsInternal();

    for (i = 0; i < mPlotList.length(); i++) {
        if (mPlotList.at(i)->Id() == aIdPlot)
            return mPlotList.at(i);
        if (aUseVersions)
            for (j = 0; j < mPlotList.at(i)->VersionsConst().length(); j++)
                if (mPlotList.at(i)->VersionsConst().at(j)->Id() == aIdPlot)
                    return mPlotList.at(i)->VersionsConst().at(j);
    }

    return NULL;
}

PlotData * ProjectData::GetPlotByIdCommon(int aIdCommon) {
    int i;
    CheckListsInternal();

    for (i = 0; i < mPlotList.length(); i++)
        if (mPlotList.at(i)->IdCommon() == aIdCommon)
            return mPlotList.at(i);

    return NULL;
}

QString ProjectData::FullShortName(bool aNoGroup) const {
    QString lShortName = mShortName;
    const ProjectData *aParent = this;

    while (aParent = aParent->mParent) {
        if (aNoGroup && aParent->Type() == ProjectData::PDGroup) break;
        lShortName = aParent->mShortName + "/" + lShortName;
    }

    return lShortName;
}

int ProjectData::SheetDigitsActual() const {
    int lSheetDigits = mSheetDigits;
    const ProjectData *aParent = this;

    while ((aParent = aParent->mParent) && aParent->mType == ProjectData::PDProject) {
        lSheetDigits = aParent->mSheetDigits;
    }
    return lSheetDigits;
}

void ProjectData::InitGrpData(int aGrpNum) {
    if (!mInited.contains(aGrpNum)) {
        switch (aGrpNum) {
        case 1:
            ReinitLists();
            break;
        }
    }
}

void ProjectData::CheckListsInternal() {
    if (!mInited.contains(1))
        ReinitLists();
}

bool ProjectData::AddToMyListInternal(bool aWithSub) {
    bool lIsOk = true;
    if (!mInUserList) {
        QSqlQuery query(db);
        query.prepare("insert into v_project_user(id_project, login) values(:id_project, user)");
        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Adding project to my list"), query);
            lIsOk = false;
        } else {
            query.bindValue(":id_project", mId);
            if (!query.exec()) {
                gLogger->ShowSqlError(QObject::tr("Adding project to my list"), query);
                lIsOk = false;
            } else {
                if (aWithSub) {
                    query.prepare("insert into v_project_user (id_project, login)"
                                  " (select id, user from v_project b where id_project = :id_project"
                                  " and not exists (select 1 from v_project_user where id_project = b.id and login = user))");
                    if (query.lastError().isValid()) {
                        gLogger->ShowSqlError(QObject::tr("Adding project to my list"), query);
                        lIsOk = false;
                    } else {
                        query.bindValue(":id_project", mId);
                        if (!query.exec()) {
                            gLogger->ShowSqlError(QObject::tr("Adding project to my list"), query);
                            lIsOk = false;
                        }
                    }
                }
            }
        }
    }
    return lIsOk;
}

bool ProjectData::AddToMyList(bool aWithSub) {
    bool lRes = true;
    int i, j;
    if (db.transaction()) {
        bool lIsOk = false;
        if (mType == PDProject) {
            // project
            if (lIsOk = AddToMyListInternal(aWithSub)) {
                if (!db.commit()) {
                    gLogger->ShowSqlError(QObject::tr("Adding project to my list"), QObject::tr("Can't commit"), db);
                    lIsOk = false;
                    lRes = false;
                } else {
                    mInUserList = 1;
                    if (aWithSub) {
                        for (i = 0; i < mProjList.length(); i++) {
                            mProjList.at(i)->mInUserList = 1;
                        }
                    }
                }
            } else {
                lRes = false;
            }
        } else if(mType == PDGroup) {
            // group
            for (i = 0; i < mProjList.length(); i++) {
                if (!i) lIsOk = true;
                if (!mProjList.at(i)->AddToMyListInternal(aWithSub)) {
                    lIsOk = false;
                    break;
                }
                if (lIsOk) {
                    if (!db.commit()) {
                        gLogger->ShowSqlError(QObject::tr("Adding project to my list"), QObject::tr("Can't commit"), db);
                        lIsOk = false;
                        lRes = false;
                    } else {
                        mInUserList = 1;
                        for (i = 0; i < mProjList.length(); i++) {
                            mProjList.at(i)->mInUserList = 1;
                            if (aWithSub) {
                                for (j = 0; j < mProjList.at(i)->mProjList.length(); j++) {
                                    mProjList.at(i)->mProjList.at(j)->mInUserList = 1;
                                }
                            }
                        }
                    }
                }
            }

        }
        if (!lIsOk) {
            db.rollback();
        }
    } else {
        gLogger->ShowSqlError(QObject::tr("Adding documents to list"), QObject::tr("Can't start transaction"), db);
        lRes = false;
    }
    return lRes;
}

bool ProjectData::RemoveFromMyList() {
    bool res = true;
    int i, j;

    QSqlQuery query(db);

    if (db.transaction()) {
        query.prepare(QString("delete from v_project_user where id_project = :id_project and login =")
                      + ((db.driverName() == "QPSQL")?" session_user":" user"));
        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Project data"), query);
            res = false;
        } else {
            if (mType == PDProject) {
                query.bindValue(":id_project", mId);
                if (!query.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Project data"), query);
                    res = false;
                } else {
                    for (i = 0; i < mProjList.length(); i++) {
                        query.bindValue(":id_project", mProjList.at(i)->mId);
                        if (!query.exec()) {
                            gLogger->ShowSqlError(QObject::tr("Project data"), query);
                            res = false;
                            break;
                        }
                    }
                    if (res) {
                        mInUserList = 0;
                        for (i = 0; i < mProjList.length(); i++) {
                            mProjList.at(i)->mInUserList = 0;
                        }
                    }
                }
            } else if (mType == PDGroup) {
                for (i = 0; i < mProjList.length(); i++) {
                    query.bindValue(":id", mProjList.at(i)->mId);
                    if (!query.exec()) {
                        gLogger->ShowSqlError(QObject::tr("Project data"), query);
                        res = false;
                        break;
                    } else {
                        for (j = 0; j < mProjList.at(i)->ProjListConst().length(); j++) {
                            mProjList.at(i)->ProjListConst().at(j)->mInUserList = 0;
                            query.bindValue(":id_project", mProjList.at(i)->ProjListConst().at(j)->mId);
                            if (!query.exec()) {
                                gLogger->ShowSqlError(QObject::tr("Project data"), query);
                                res = false;
                                break;
                            }
                        }
                        if (!res) break;
                    }
                }
                if (res) {
                    for (i = 0; i < mProjList.length(); i++) {
                        mProjList.at(i)->mInUserList = 0;
                        for (j = 0; j < mProjList.at(i)->ProjListConst().length(); j++) {
                            mProjList.at(i)->ProjListConst().at(j)->mInUserList = 0;
                        }
                    }
                }
            }
        }

        if (res) {
            if (!db.commit()) {
                gLogger->ShowError(QObject::tr("Project data"), QObject::tr("Can't commit") + "\r\n" + db.lastError().text());
                res = false;
            }
        }
        if (!res) {
            db.rollback();
        }
    } else {
        gLogger->ShowError(QObject::tr("Project data"), QObject::tr("Can't start transaction") + "\r\n" + db.lastError().text());
    }
    return res;
}

void ProjectData::ReinitLists() {
    if (mIsNew) return;

    if (gMainWindow->AlertThread()) {
        gMainWindow->AlertThread()->AddIdProject(mId);
    }

    bool lNeedEmitUpdate = !gProjects->IsPlotListInUpdate(0)
            && !gProjects->IsPlotListInUpdate(mId);
    if (lNeedEmitUpdate) {
        gProjects->EmitPlotListBeforeUpdate(mId);
    }

    bool lIsOk = false;

    if (!mInited.contains(1)) mInited.append(1);

    qDeleteAll(mPlotList);
    mPlotList.clear();
    mComplectList.clear();

    qDeleteAll(mNamedLists);
    mNamedLists.clear();


    QSqlQuery queryPlot(db);

    //qint64 lStart = QDateTime::currentMSecsSinceEpoch();

    queryPlot.prepare(QString() + "select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id id_dwg, b.version dwg_version,"
                      " a.cancelled, a.cancdate, a.cancuser,"
                      " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.stage, a.code, a.sheet_number, a.extension,"
                      " a.nametop, a.name, a.block_name,"
                      " a.crdate, a.cruser, a.edit_date, a.edit_user,"
                      + ((db.driverName()== "QPSQL")?" length(b.data) as data_length,":" dbms_lob.getlength(b.data) data_length,")
                      + " (select count(1) from v_xref2dwg where id_dwg_main = b.id) xrefs_cnt,"
                      " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
                      " a.comments"
                      " from (select * from v_plot_simple where id_project = :IdProject and deleted = 0 and working = 1) a"
                      " left outer join v_dwg b on b.id_plot = a.id"
                      " where (b.version = (select max(version) from v_dwg where id_plot = a.id) or b.version is null)");

    if (queryPlot.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), queryPlot);
    } else {
        queryPlot.bindValue(":IdProject", mId);
        if (!queryPlot.exec()) {
            gLogger->ShowSqlError(QObject::tr("Project data"), queryPlot);
        } else {
            lIsOk = true;
            //qint64 lStart = QDateTime::currentMSecsSinceEpoch();
            while (queryPlot.next()) {
                PlotData *lPlotData = new PlotData(queryPlot.value("id").toInt(), queryPlot.value("id_project").toInt(), queryPlot.value("id_common").toInt(),
                                                   queryPlot.value("type_area").toInt(), queryPlot.value("type").toInt(),
                                                   queryPlot.value("id_dwg").toInt(), queryPlot.value("dwg_version").toInt(),
                                                   1/*working*/,
                                                   queryPlot.value("cancelled").toInt(), queryPlot.value("cancdate").toDate(), queryPlot.value("cancuser").toString(),
                                                   0/*deleted*/, QDate(), "",
                                                   queryPlot.value("version").toString(), queryPlot.value("version_ext").toString(),
                                                   queryPlot.value("sentdate").toDate(), queryPlot.value("sentuser").toString(),
                                                   queryPlot.value("section").toString(), queryPlot.value("stage").toString(),
                                                   queryPlot.value("code").toString(), queryPlot.value("sheet_number").toString(),
                                                   queryPlot.value("extension").toString(),
                                                   queryPlot.value("nametop").toString(), queryPlot.value("name").toString(),
                                                   queryPlot.value("block_name").toString(),
                                                   queryPlot.value("crdate").toDateTime(), queryPlot.value("cruser").toString(),
                                                   queryPlot.value("edit_date").toDateTime(), queryPlot.value("edit_user").toString(),
                                                   queryPlot.value("data_length").toLongLong(),
                                                   queryPlot.value("xrefs_cnt").toInt(),
                                                   queryPlot.value("edit_na").toInt(), queryPlot.value("load_na").toInt(), queryPlot.value("editprop_na").toInt(),
                                                   queryPlot.value("delete_na").toInt(), queryPlot.value("view_na").toInt(), queryPlot.value("newver_na").toInt(),
                                                   queryPlot.value("comments").toString());
                mPlotList.append(lPlotData);
                if (!queryPlot.value("section").toString().isEmpty()
                        && !mComplectList.contains(queryPlot.value("section").toString()))
                    mComplectList.append(queryPlot.value("section").toString());
            }
            std::sort(mComplectList.begin(), mComplectList.end(), CmpStringsWithNumbersNoCase);
            //QMessageBox::critical(gMainWindow, QObject::tr("Project data"), QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStart)) / 1000));
        }
    }
    //QMessageBox::critical(gMainWindow, QObject::tr("Project data"), QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStart)) / 1000));
    if (lIsOk) {
        lIsOk = false;
        QSqlQuery query(db);

        query.prepare("select id, name from v_pl_name where id_project = ? order by name");

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError("Plot lists names", query);
        } else {
            query.addBindValue(mId);
            if (!query.exec()) {
                gLogger->ShowSqlError("Plot lists names", query);
            } else {
                lIsOk = true;
                while (query.next()) {
                    mNamedLists.append(new PlotNamedListData(query.value("id").toInt(), query.value("name").toString(), true));
                }
            }
        }
    }

    if (lNeedEmitUpdate) {
        gProjects->EmitPlotListNeedUpdate(mId);
    }
}

bool ProjectData::RemoveFromDB() {
    bool res = false;

    QSqlQuery qDelete(db);
    switch (mType) {
    case PDProject:
        qDelete.prepare("delete from v_project where id = ?");
        break;
    case PDGroup:
        qDelete.prepare("delete from proj_group where id = ?");
        break;
    }

    if (qDelete.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), qDelete);
    } else {
        qDelete.addBindValue(mId);
        if (!qDelete.exec()) {
            gLogger->ShowSqlError(QObject::tr("Project data"), qDelete);
        } else {
            res = true;
        }
    }

    return res;
}

bool ProjectData::SaveData() {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;

    if (mType == PDProject) {
        // project
        if (mIsNew) {
            QSqlQuery qInsert(db);

            if (gSettings->ContractMode == 1) {
                qInsert.prepare(
                            "insert into v_project (id, id_group, id_project, id_customer, id_projtype, shortname, shortnum, dogovor, dogdate, stage, name, gip,"
                            " comments, code_template, sheet_digits) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            } else {
                // without "dogovor", "dogdate" (it means "contract" and "contract date" in russian)
                qInsert.prepare(
                            "insert into v_project (id, id_group, id_project, id_customer, id_projtype, shortname, shortnum, stage, name, gip,"
                            " comments, code_template, sheet_digits) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            }
            if (qInsert.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("Project data"), qInsert);
            } else {
                qInsert.addBindValue(mId);
                if (mIdGroup) {
                    qInsert.addBindValue(mIdGroup);
                } else {
                    qInsert.addBindValue(QVariant());
                }

                if (mIdParentProject) {
                    qInsert.addBindValue(mIdParentProject);
                } else {
                    qInsert.addBindValue(QVariant());
                }

                if (mIdCustomer) {
                    qInsert.addBindValue(mIdCustomer);
                } else {
                    qInsert.addBindValue(QVariant());
                }

                if (mIdProjType) {
                    qInsert.addBindValue(mIdProjType);
                } else {
                    qInsert.addBindValue(QVariant());
                }

                qInsert.addBindValue(mShortName);
                qInsert.addBindValue(mShortNum);

                if (gSettings->ContractMode == 1) {
                    qInsert.addBindValue(mContract);
                    qInsert.addBindValue(mContractDate);
                }

                qInsert.addBindValue(mStage);
                qInsert.addBindValue(mName);
                qInsert.addBindValue(mGip);
                qInsert.addBindValue(mComments);
                qInsert.addBindValue(mCodeTemplate);
                if (mSheetDigits) {
                    qInsert.addBindValue(mSheetDigits);
                } else {
                    qInsert.addBindValue(QVariant());
                }

                if (!qInsert.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Project data"), qInsert);
                } else {
                    res = true;
                }
            }
        } else {
            CheckParChange(ShortName, shortname);
            CheckParChangeWithNull(IdGroup, id_group, 0);

            CheckParChangeWithNull(IdCustomer, id_customer, 0);
            CheckParChangeWithNull(IdProjType, id_projtype, 0);
            CheckParChange(ShortNum, shortnum);
            if (gSettings->ContractMode == 1) {
                CheckParChange(Contract, dogovor);
                CheckParChange(ContractDate, dogdate);
            }
            CheckParChange(Stage, stage);
            CheckParChange(Name, name);
            CheckParChange(Gip, gip);
            CheckParChange(Comments, comments);

            CheckParChange(CodeTemplate, code_template);
            CheckParChange(SheetDigits, sheet_digits);
            if (!lUpdStr.isEmpty()) {
                lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

                QSqlQuery qUpdate(db);

                qUpdate.prepare("update v_project set " + lUpdStr + " where id = ?");
                if (qUpdate.lastError().isValid()) {
                    gLogger->ShowSqlError(QObject::tr("Project data"), qUpdate);
                } else {
                    for (int i = 0; i < lUpdValues.length(); i++) {
                        qUpdate.addBindValue(lUpdValues.at(i));
                    }
                    qUpdate.addBindValue(mId);
                    if (!qUpdate.exec()) {
                        gLogger->ShowSqlError(QObject::tr("Project data"), qUpdate);
                    } else {
                        res = true;
                    }
                }
            } else {
                res = true;
            }
        }
    } else if (mType == PDGroup) {
        // group
        if (mIsNew) {
            QSqlQuery qInsert(db);

            qInsert.prepare("insert into proj_group (id, name) values (?, ?)");
            if (qInsert.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("Project data"), qInsert);
            } else {
                qInsert.addBindValue(mId);
                qInsert.addBindValue(mShortName);
                if (!qInsert.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Project data"), qInsert);
                } else {
                    res = true;
                }
            }
        } else {
            if (mShortName != mShortNameOrig) {
                QSqlQuery qUpdate(db);

                qUpdate.prepare("update proj_group set name = ? where id = ?");
                if (qUpdate.lastError().isValid()) {
                    gLogger->ShowSqlError(QObject::tr("Project data"), qUpdate);
                } else {
                    qUpdate.addBindValue(mShortName);
                    qUpdate.addBindValue(mId);
                    if (!qUpdate.exec()) {
                        gLogger->ShowSqlError(QObject::tr("Project data"), qUpdate);
                    } else {
                        res = true;
                    }
                }
            } else {
                res = true;
            }
        }
    }
    return res;
}

void ProjectData::RollbackEdit() {
    RollbackPar(ShortName)
    if (mType == PDGroup) return;
    RollbackPar(IdParentProject)

    // full data
    RollbackPar(IdGroup)
    RollbackPar(IdCustomer)
    RollbackPar(IdProjType)
    RollbackPar(ShortNum)
    RollbackPar(Contract)
    RollbackPar(ContractDate)
    RollbackPar(Stage)
    RollbackPar(Name)
    RollbackPar(Gip)
    RollbackPar(Comments)

    RollbackPar(CodeTemplate)
    RollbackPar(SheetDigits)
}

void ProjectData::CommitEdit() {
    mIsNew = false;
    CommitPar(ShortName)
    if (mType == PDGroup) return;
    CommitPar(IdParentProject)

    // full data
    CommitPar(IdGroup)
    CommitPar(IdCustomer)
    CommitPar(IdProjType)
    CommitPar(ShortNum)
    CommitPar(Contract)
    CommitPar(ContractDate)
    CommitPar(Stage)
    CommitPar(Name)
    CommitPar(Gip)
    CommitPar(Comments)

    CommitPar(CodeTemplate)
    CommitPar(SheetDigits)
}

void ProjectData::ShowProps(QWidget *parent) {
    if (mType == PDProject) {
        if (!mIdParentProject) {
            // project
            ProjectPropDlg dlg(ProjectPropDlg::ProjectProps, this, parent?parent:gMainWindow);
            dlg.exec();
        } else {
            // construction
            ConstrPropDlg dlg(ConstrPropDlg::ConstrProps, this, parent?parent:gMainWindow);
            dlg.exec();
        }
    } else if (mType == PDGroup) {
        QInputDialog lDlg(parent?parent:gMainWindow);
        lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        lDlg.setSizeGripEnabled(true);
        lDlg.setWindowTitle(QObject::tr("Rename group"));
        lDlg.setLabelText(QObject::tr("Enter new name for group") + " '" + mShortName + "'");
        lDlg.setTextValue(mShortName);
        QRect lPos = lDlg.geometry();
        lPos.setTopLeft(QCursor::pos());
        lDlg.setGeometry(lPos);
aRenameGroup1:
        if (lDlg.exec() == QDialog::Accepted) {
            QString &lGroupName = lDlg.textValue();
            if (lGroupName.isEmpty()) {
                QMessageBox::critical(parent?parent:gMainWindow, QObject::tr("Rename group"), QObject::tr("Name must be specified!"));
                goto aRenameGroup1;
            } else {
                if (mShortName!= lGroupName) {
                    mShortName = lGroupName;
                    if (SaveData()) {
                        CommitEdit();
                        std::sort(gProjects->ProjListRef().begin(), gProjects->ProjListRef().end(),
                                  [] (const ProjectData * d1, const ProjectData * d2) { return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst()); });
                        //ui->treeWidget->SetSelectedGroup(lNewId); // make selected in this window
                        emit gProjects->ProjectListNeedUpdate();
                    } else {
                        RollbackEdit();
                        goto aRenameGroup1;
                    }
                }
            }
        }

    }
}

int ProjectData::NewGroup(QWidget *parent) {
    QInputDialog lDlg(parent?parent:gMainWindow);
    lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    lDlg.setSizeGripEnabled(true);
    lDlg.setWindowTitle(QObject::tr("New group"));
    lDlg.setLabelText(QObject::tr("Enter name for new group"));

    int lNewId; // don't waste sequence
aAddGroup1:
    lNewId = 0;
    if (lDlg.exec() == QDialog::Accepted) {
        QString &lGroupName = lDlg.textValue();
        if (lGroupName.isEmpty()) {
            QMessageBox::critical(parent?parent:gMainWindow, QObject::tr("New group"), QObject::tr("Name must be specified!"));
            goto aAddGroup1;
        } else {
            if (lNewId || gOracle->GetSeqNextVal("proj_group_id_seq", lNewId)) {
                ProjectData *lNewGroup = new ProjectData(lNewId, ProjectData::PDGroup);
                lNewGroup->ShortNameRef() = lGroupName;
                if (lNewGroup->SaveData()) {
                    lNewGroup->CommitEdit();
                    gProjects->ProjListRef().append(lNewGroup); // add to list
                    std::sort(gProjects->ProjListRef().begin(), gProjects->ProjListRef().end(),
                              [] (const ProjectData * d1, const ProjectData * d2) { return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst()); });
                    // update list after return - so need not update twice, byt the caller already knows ID before update
                    QTimer::singleShot(0, gProjects, SLOT(ProjectListNeedUpdate()));
                } else {
                    delete lNewGroup;
                    goto aAddGroup1;
                }
            } else {
                goto aAddGroup1;
            }
        }
    }
    return lNewId;
}

int ProjectData::NewProject(ProjectData *aParentGroup, QWidget *parent) {
    ProjectPropDlg dlg(ProjectPropDlg::ProjectNew, aParentGroup, parent?parent:gMainWindow);
    if (dlg.exec() == QDialog::Accepted)
        return dlg.ProjectId();
    else
        return 0;
}

int ProjectData::NewConstruction(QWidget *parent) {
    ConstrPropDlg dlg(ConstrPropDlg::ConstrNew, this, parent?parent:gMainWindow);
    if (dlg.exec() == QDialog::Accepted)
        return dlg.ProjectId();
    else
        return 0;
}

//-------------------------------------------------------------------------------------------------------
ProjectList::ProjectList() :
    QObject(),
    mProjListError(false)
{
    mQMainList = new QSqlQuery(db);
    mQSubProjects = new QSqlQuery(db);
    mQGroupChilds = new QSqlQuery(db);
    mQUpdateProjects = new QSqlQuery(db);
    mQUpdateGroups = new QSqlQuery(db);

    // main level
    mQMainList->prepare("SELECT 0 as type, ID, ID_PROJECT, SHORTNAME,"
                  " (SELECT COUNT(*) FROM V_PROJECT_USER WHERE ID_PROJECT = A.ID AND LOGIN = USER) as IN_USER_LIST,"
                  " CASE WHEN PLOT_CHDATE > CURRENT_DATE - 14 THEN 1 ELSE 0 END as RECENTLY,"
                  " ARCHIVED, projright, id_group, id_customer, id_projtype, shortnum, dogovor, dogdate, stage, name,"
                  " gip, startdate, cruser, enddate, enduser, comments,"
                  " code_template, sheet_digits, prop_chdate"
                  " FROM V_PROJECT A"
                  " WHERE ID_PROJECT IS NULL AND ID_GROUP IS NULL AND ID > 0"
                  " UNION"
                  " SELECT 1, ID, 0, NAME, -1, -1,"
                  " -1, -1, -1, -1, -1, NULL, NULL, NULL, NULL, NULL,"
                  " NULL, NULL, NULL, NULL, NULL, NULL,"
                  " NULL, NULL, update_date as prop_chdate"
                  " FROM PROJ_GROUP B");
    if (mQMainList->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *mQMainList);
        mProjListError = true;
    }

    // subprojects (constructions)
    mQSubProjects->prepare("SELECT 0 as type, ID, ID_PROJECT, SHORTNAME,"
                  " (SELECT COUNT(*) FROM V_PROJECT_USER WHERE ID_PROJECT = A.ID AND LOGIN = USER) as IN_USER_LIST,"
                  " CASE WHEN PLOT_CHDATE > CURRENT_DATE - 14 THEN 1 ELSE 0 END as RECENTLY,"
                  " ARCHIVED, projright, id_group, id_customer, id_projtype, shortnum, dogovor, dogdate, stage, name,"
                  " gip, startdate, cruser, enddate, enduser, comments,"
                  " code_template, sheet_digits, prop_chdate"
                  " FROM V_PROJECT A"
                  " WHERE ID_PROJECT = :ID AND ID > 0");
    if (mQSubProjects->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *mQSubProjects);
        mProjListError = true;
    }

    // child of groups
    mQGroupChilds->prepare("SELECT 0 as type, ID, ID_PROJECT, SHORTNAME,"
                  " (SELECT COUNT(*) FROM V_PROJECT_USER WHERE ID_PROJECT = A.ID AND LOGIN = USER) as IN_USER_LIST,"
                  " CASE WHEN PLOT_CHDATE > CURRENT_DATE - 14 THEN 1 ELSE 0 END as RECENTLY,"
                  " ARCHIVED, projright, id_group, id_customer, id_projtype, shortnum, dogovor, dogdate, stage, name,"
                  " gip, startdate, cruser, enddate, enduser, comments,"
                  " code_template, sheet_digits, prop_chdate"
                  " FROM V_PROJECT A"
                  " WHERE ID_GROUP = :ID AND ID > 0");
    if (mQGroupChilds->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *mQGroupChilds);
        mProjListError = true;
    }

    //
    mQUpdateProjects->prepare("SELECT ID, ID_PROJECT, SHORTNAME,"
                  " (SELECT COUNT(*) FROM V_PROJECT_USER WHERE ID_PROJECT = A.ID AND LOGIN = USER) as IN_USER_LIST,"
                  " CASE WHEN PLOT_CHDATE > CURRENT_DATE - 14 THEN 1 ELSE 0 END as RECENTLY,"
                  " ARCHIVED, projright, id_group, id_customer, id_projtype, shortnum, dogovor, dogdate, stage, name,"
                  " gip, startdate, cruser, enddate, enduser, comments,"
                  " code_template, sheet_digits, prop_chdate"
                  " FROM V_PROJECT A"
                  " WHERE prop_chdate > :prop_chdate AND ID > 0"
                  " ORDER BY prop_chdate");
    if (mQUpdateProjects->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *mQUpdateProjects);
        mProjListError = true;
    }


    mQUpdateGroups->prepare("SELECT ID, NAME, UPDATE_DATE"
                            " FROM PROJ_GROUP"
                            " WHERE update_date > :UPDATE_DATE"
                            " ORDER BY UPDATE_DATE");
    if (mQUpdateGroups->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *mQUpdateGroups);
        mProjListError = true;
    }

    if (!mProjListError) {
        InitProjectList(false);
    }
}

ProjectList::~ProjectList() {
    qDeleteAll(mProjList);
}

ProjectList * ProjectList::GetInstance() {
    static ProjectList * lProjectList = NULL;
    if (!lProjectList) {
        lProjectList = new ProjectList();
        //qAddPostRoutine(ProjectList::clean);
    }
    return lProjectList;
}

void ProjectList::InitProjectListInternal(ProjectData *aParent) {
    QSqlQuery *query;

    if (!aParent) {
        // main level
        query = mQMainList;
    } else {
        if (aParent->Type() == ProjectData::PDProject) {
            // subprojects (constructions)
            query = mQSubProjects;
        } else {
            // child of groups
            query = mQGroupChilds;
        }
    }

    if (aParent) {
        query->bindValue(":ID", aParent->Id());
    }

    if (!query->exec()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *query);
        mProjListError = true;
        return;
    }

    while (query->next()) {
        ProjectData *lProjectData = new ProjectData(query->value("shortname").toString(), query->value("id").toInt(), query->value("id_project").toInt(),
                                                    query->value("type").toInt()?ProjectData::PDGroup:ProjectData::PDProject,
                                                    query->value("in_user_list").toInt(), query->value("recently").toInt(), query->value("archived").toInt(),
                                                    query->value("projright").toInt(), query->value("id_group").toInt(),
                                                    query->value("id_customer").toInt(), query->value("id_projtype").toInt(),
                                                    query->value("shortnum").toString(),
                                                    query->value("dogovor").toString(), query->value("dogdate").toDate(),
                                                    query->value("stage").toString(), query->value("name").toString(),
                                                    query->value("gip").toString(),
                                                    query->value("startdate").toDate(), query->value("cruser").toString(),
                                                    query->value("enddate").toDate(), query->value("enduser").toString(),
                                                    query->value("comments").toString(),
                                                    query->value("code_template").toString(), query->value("sheet_digits").toInt(),
                                                    query->value("prop_chdate").toDateTime(),
                                                    aParent);
        if (!aParent) {
            mProjList.append(lProjectData);
        } else {
            aParent->ProjListRef().append(lProjectData);
        }
    }

    if (!aParent) {
        std::sort(mProjList.begin(), mProjList.end(),
            [] (const ProjectData *d1, const ProjectData *d2) {
                return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst());
            });
    } else {
        std::sort(aParent->ProjListRef().begin(), aParent->ProjListRef().end(),
            [] (const ProjectData *d1, const ProjectData *d2) {
                return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst());
            });
    }

    if (!aParent) {
        foreach (ProjectData * lProjectData, mProjList) {
            InitProjectListInternal(lProjectData);
        }
    } else {
        foreach (ProjectData * lProjectData, aParent->ProjListConst()) {
            InitProjectListInternal(lProjectData);
        }
    }
}

void ProjectList::InitProjectList(bool aClearError) {
    if (aClearError) {
        mProjListError = false;
    }
    if (mProjListError) return;

    gProjMaxModTime.setDate(QDate::currentDate().addYears(-10));
    gGroupMaxModTime = gProjMaxModTime;

    QDateTime lCurDateTime;
    bool lCurDateTimeOk = gOracle->GetSysTimeStamp(lCurDateTime);

    EmitPlotListBeforeUpdate(0);
    qDeleteAll(mProjList);
    mProjList.clear();
    InitProjectListInternal();
    emit ProjectListNeedUpdate(); // update opened projects lists
    EmitPlotListNeedUpdate(0); // update all opened documents lists

    if (lCurDateTimeOk) {
        gProjMaxDelTime = lCurDateTime;
        gGroupMaxDelTime = lCurDateTime;
    } else {
        gProjMaxDelTime = gProjMaxModTime;
        gGroupMaxDelTime = gGroupMaxModTime;
    }

//    QMessageBox::critical(NULL, "", gProjMaxModTime.toString("dd.MM.yy hh:mm.ss"));
//    QMessageBox::critical(NULL, "", gGroupMaxModTime.toString("dd.MM.yy hh:mm.ss"));
}

void ProjectList::UpdateProjectInList() {
    if (mProjList.isEmpty()) return;
    mQUpdateProjects->bindValue(":prop_chdate", gProjMaxModTime);
    if (!mQUpdateProjects->exec()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *mQUpdateProjects);
        mProjListError = true;
        return;
    }

    bool lAnyChanges = false;
    while (mQUpdateProjects->next()) {
        //bool lIsNew;
        ProjectData *lProject = FindByIdProject(mQUpdateProjects->value("id").toInt());
        ProjectData *lProjectParent = NULL;
        //ProjectData *lProjectParentPrev;

        if (mQUpdateProjects->value("id_group").toInt()) {
            lProjectParent = FindByIdGroup(mQUpdateProjects->value("id_group").toInt());
            if (!lProjectParent) {
                lAnyChanges = false;
                InitProjectList(true);
                break;
            }
        } else if (mQUpdateProjects->value("id_project").toInt()) {
            lProjectParent = FindByIdProject(mQUpdateProjects->value("id_project").toInt());
            if (!lProjectParent) {
                lAnyChanges = false;
                InitProjectList(true);
                break;
            }
        }

        if (lProject) {
            // update existing
//            lIsNew = false;
//            lProjectParentPrev = lProject->Parent();
            //QMessageBox::critical(NULL, "", QString::number(lProject->Id()));
            lProject->SetAllData(mQUpdateProjects->value("shortname").toString(), mQUpdateProjects->value("id").toInt(), mQUpdateProjects->value("id_project").toInt(),
                                 ProjectData::PDProject,
                                 mQUpdateProjects->value("in_user_list").toInt(), mQUpdateProjects->value("recently").toInt(), mQUpdateProjects->value("archived").toInt(),
                                 mQUpdateProjects->value("projright").toInt(), mQUpdateProjects->value("id_group").toInt(),
                                 mQUpdateProjects->value("id_customer").toInt(), mQUpdateProjects->value("id_projtype").toInt(),
                                 mQUpdateProjects->value("shortnum").toString(),
                                 mQUpdateProjects->value("dogovor").toString(), mQUpdateProjects->value("dogdate").toDate(),
                                 mQUpdateProjects->value("stage").toString(), mQUpdateProjects->value("name").toString(),
                                 mQUpdateProjects->value("gip").toString(),
                                 mQUpdateProjects->value("startdate").toDate(), mQUpdateProjects->value("cruser").toString(),
                                 mQUpdateProjects->value("enddate").toDate(), mQUpdateProjects->value("enduser").toString(),
                                 mQUpdateProjects->value("comments").toString(),
                                 mQUpdateProjects->value("code_template").toString(), mQUpdateProjects->value("sheet_digits").toInt(),
                                 mQUpdateProjects->value("prop_chdate").toDateTime()/*,
                                 NULL*/);
        } else {
            // create new
//            lIsNew = true;
//            lProjectParentPrev = NULL;
            lProject = new ProjectData(mQUpdateProjects->value("shortname").toString(), mQUpdateProjects->value("id").toInt(), mQUpdateProjects->value("id_project").toInt(),
                                       ProjectData::PDProject,
                                       mQUpdateProjects->value("in_user_list").toInt(), mQUpdateProjects->value("recently").toInt(), mQUpdateProjects->value("archived").toInt(),
                                       mQUpdateProjects->value("projright").toInt(), mQUpdateProjects->value("id_group").toInt(),
                                       mQUpdateProjects->value("id_customer").toInt(), mQUpdateProjects->value("id_projtype").toInt(),
                                       mQUpdateProjects->value("shortnum").toString(),
                                       mQUpdateProjects->value("dogovor").toString(), mQUpdateProjects->value("dogdate").toDate(),
                                       mQUpdateProjects->value("stage").toString(), mQUpdateProjects->value("name").toString(),
                                       mQUpdateProjects->value("gip").toString(),
                                       mQUpdateProjects->value("startdate").toDate(), mQUpdateProjects->value("cruser").toString(),
                                       mQUpdateProjects->value("enddate").toDate(), mQUpdateProjects->value("enduser").toString(),
                                       mQUpdateProjects->value("comments").toString(),
                                       mQUpdateProjects->value("code_template").toString(), mQUpdateProjects->value("sheet_digits").toInt(),
                                       mQUpdateProjects->value("prop_chdate").toDateTime()/*,
                                       NULL*/);
        }

        lProject->setParent(lProjectParent);

//        if (lIsNew || lProjectParentPrev != lProjectParent) {
//            if (!lIsNew ) {
//                if (!lProjectParentPrev) {
//                    mProjList.removeAll(lProject);
//                } else {
//                    lProjectParentPrev->ProjListRef().removeAll(lProject);
//                }
//            }

//            if (!lProjectParent) {
//                mProjList.append(lProject);
//                std::sort(mProjList.begin(), mProjList.end(),
//                          [] (const ProjectData * d1, const ProjectData * d2) { return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst()); });
//            } else {
//                lProjectParent->ProjListRef().append(lProject);
//                std::sort(lProjectParent->ProjListRef().begin(), lProjectParent->ProjListRef().end(),
//                          [] (const ProjectData * d1, const ProjectData * d2) { return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst()); });
//            }
//        }
        lAnyChanges = true;
        //QMessageBox::critical(NULL, "", mQUpdateProjects->value("id").toString());
    }

    if (lAnyChanges) {
        emit ProjectListNeedUpdate();
    }
}

void ProjectList::UpdateGroupInList() {
    if (mProjList.isEmpty()) return;
    mQUpdateGroups->bindValue(":UPDATE_DATE", gGroupMaxModTime);
    if (!mQUpdateGroups->exec()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), *mQUpdateGroups);
        mProjListError = true;
        return;
    }

    bool lAnyChanges = false;
    while (mQUpdateGroups->next()) {
        ProjectData *lGroup = FindByIdGroup(mQUpdateGroups->value("id").toInt());

        if (lGroup) {
            // update existing
            lGroup->SetAllData(mQUpdateGroups->value("id").toInt(), mQUpdateGroups->value("name").toString(), mQUpdateGroups->value("update_date").toDateTime());
        } else {
            // create new
            lGroup = new ProjectData(mQUpdateGroups->value("id").toInt(), mQUpdateGroups->value("name").toString(), mQUpdateGroups->value("update_date").toDateTime());
            mProjList.append(lGroup);
            std::sort(mProjList.begin(), mProjList.end(),
                      [] (const ProjectData * d1, const ProjectData * d2) { return CmpStringsWithNumbersNoCase(d1->ShortNameConst(), d2->ShortNameConst()); });
        }

        lAnyChanges = true;
    }

    if (lAnyChanges) {
        emit ProjectListNeedUpdate();
    }
}

void ProjectList::RemoveProjectFromList() {
    if (mProjList.isEmpty()) return;
    QSqlQuery qQueryRemoved(db);

    qQueryRemoved.prepare("SELECT ID, REMOVE_DATE"
                          " FROM PROJECT_JUST_REMOVED"
                          " WHERE remove_date > :remove_date"
                          " ORDER BY remove_date");
    if (qQueryRemoved.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), qQueryRemoved);
    } else {
        qQueryRemoved.bindValue(":remove_date", gProjMaxDelTime);
        if (!qQueryRemoved.exec()) {
            gLogger->ShowSqlError(QObject::tr("Project data"), qQueryRemoved);
        } else {
            bool lAnyChanges = false;

            while (qQueryRemoved.next()) {
                ProjectData *lProject = FindByIdProject(qQueryRemoved.value("id").toInt());
                if (lProject) {
                    if (lProject->Parent()) {
                        lProject->Parent()->ProjListRef().removeAll(lProject);
                    } else {
                        mProjList.removeAll(lProject);
                    }
                    delete lProject;
                    lAnyChanges = true;
                }
                if (qQueryRemoved.value("remove_date").toDateTime() > gProjMaxDelTime) gProjMaxDelTime = qQueryRemoved.value("remove_date").toDateTime();
            }

            if (lAnyChanges) {
                emit ProjectListNeedUpdate();
            }
        }
    }
}

void ProjectList::RemoveGroupFromList() {
    if (mProjList.isEmpty()) return;
    QSqlQuery qQueryRemoved(db);

    qQueryRemoved.prepare("SELECT ID, REMOVE_DATE"
                          " FROM PROJGROUP_JUST_REMOVED"
                          " WHERE remove_date > :remove_date"
                          " ORDER BY remove_date");
    if (qQueryRemoved.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), qQueryRemoved);
    } else {
        qQueryRemoved.bindValue(":remove_date", gProjMaxDelTime);
        if (!qQueryRemoved.exec()) {
            gLogger->ShowSqlError(QObject::tr("Project data"), qQueryRemoved);
        } else {
            bool lAnyChanges = false;

            while (qQueryRemoved.next()) {
                ProjectData *lProject = FindByIdGroup(qQueryRemoved.value("id").toInt());
                if (lProject) {
                    if (lProject->Parent()) {
                        // dummy - can't be parent for group
                        lProject->Parent()->ProjListRef().removeAll(lProject);
                    } else {
                        mProjList.removeAll(lProject);
                    }
                    delete lProject;
                    lAnyChanges = true;
                }
                if (qQueryRemoved.value("remove_date").toDateTime() > gGroupMaxDelTime) gGroupMaxDelTime = qQueryRemoved.value("remove_date").toDateTime();
            }

            if (lAnyChanges) {
                emit ProjectListNeedUpdate();
            }
        }
    }
}

void ProjectList::UpdatePlotList(long aIdProject) {
    if (mProjList.isEmpty()) return;
    ProjectData *lProject = FindByIdProject(aIdProject);
    if (lProject
            && lProject->IsPlotListInited()) {
        lProject->ReinitLists();
    }
}

QList<ProjectData *> & ProjectList::ProjListRef() {
    return mProjList;
}

const QList<ProjectData *> & ProjectList::ProjListConst() {
    return mProjList;
}

ProjectData * ProjectList::FindByIdProjectInternal(long aIdProject, const QList<ProjectData *> &aProjList, ProjectData::PDType aType) const {
    ProjectData *lProjectData;
    for (int i = 0; i < aProjList.length(); i++) {
        if (aProjList.at(i)->Type() == aType
                && aProjList.at(i)->Id() == aIdProject)
            return aProjList.at(i);
        lProjectData = FindByIdProjectInternal(aIdProject, aProjList.at(i)->ProjListConst(), aType);
        if (lProjectData) return lProjectData;
    }
    return NULL;
}

ProjectData * ProjectList::FindByIdProject(long aIdProject) {
    return FindByIdProjectInternal(aIdProject, mProjList, ProjectData::PDProject);
}

ProjectData * ProjectList::FindByIdGroup(long aIdGroup) {
    return FindByIdProjectInternal(aIdGroup, mProjList, ProjectData::PDGroup);
}

QString ProjectList::ProjectFullShortName(long aIdProject) {
    ProjectData * lProject = FindByIdProject(aIdProject);
    if (lProject) {
        return lProject->FullShortName();
    } else {
        return "";
    }
}

PlotData * ProjectList::FindByIdPlotInternal(long aIdPlot, const QList<ProjectData *> &aProjList, bool aUseVersions) {
    PlotData * lPlot = NULL;
    for (int i = 0; i < aProjList.length(); i++) {
        if (lPlot = aProjList.at(i)->GetPlotByIdNotLoad(aIdPlot, aUseVersions)) {
            return lPlot;
        }
        if (lPlot = FindByIdPlotInternal(aIdPlot, aProjList.at(i)->ProjListConst(), aUseVersions)) {
            return lPlot;
        }
    }
    return NULL;
}

PlotData * ProjectList::FindByIdPlot(long aIdPlot) {
    PlotData * lPlotData = FindByIdPlotInternal(aIdPlot, mProjList, true);

    if (lPlotData) return lPlotData;

    QSqlQuery queryPlot(db);

    queryPlot.prepare("select id_project from v_plot_simple where id = :Id");

    if (queryPlot.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Find document by id"), queryPlot);
    } else {
        queryPlot.bindValue(":Id", aIdPlot);
        if (!queryPlot.exec()) {
            gLogger->ShowSqlError(QObject::tr("Find document by id"), queryPlot);
        } else {
            if (queryPlot.next()) {
                ProjectData * lProjectData = FindByIdProject(queryPlot.value("id_project").toInt());
                if (lProjectData) {
                    // find through versions
                    lPlotData = lProjectData->GetPlotById(aIdPlot, true);
                    /*if (!lPlotData) {
                        lPlotData = lProjectData->GetPlotById(aIdPlot, true);
                    }*/
                }

                // it was version with reload
                // reloading is not easy and don't realized in dialog windows
//                bool lProjectListReinited = false;

//                do {
//                    ProjectData * lProjectData = FindByIdProject(queryPlot.value("id_project").toInt());
//                    if (lProjectData) {
//                        lPlotData = lProjectData->GetPlotById(aIdPlot);
//                        if (!lPlotData) {
//                            // find through versions
//                            lPlotData = lProjectData->GetPlotById(aIdPlot, true);
//                        }
//                        if (lPlotData) {
//                            break; // all is ok
//                        } else {
//                            // refresh and retry
//                            if (!lProjectListReinited) {
//                                InitProjectList(false);
//                                lProjectListReinited = true;
//                            } else {
//                                break; // not found after all refreshes
//                            }
//                        }
//                    } else {
//                        if (lProjectListReinited) break;
//                        InitProjectList(false);
//                        lProjectListReinited = true;
//                    }
//                } while (true);
            } else {
                // just not found, it is not an error
            }
        }
    }

    return lPlotData;
}

void ProjectList::EmitPlotListBeforeUpdate(int aIdProject) { // 0 - all projects
    mPlotListInUpdate.append(aIdProject);
    emit PlotListBeforeUpdate(aIdProject);
}

void ProjectList::EmitPlotListNeedUpdate(int aIdProject) { // 0 - all projects
    emit PlotListNeedUpdate(aIdProject);
    mPlotListInUpdate.removeAll(aIdProject);
}

bool ProjectList::IsPlotListInUpdate(int aIdProject) {
    if (mPlotListInUpdate.contains(0)) return true;
    return mPlotListInUpdate.contains(aIdProject);
}

void ProjectList::EmitPlotBeforeUpdate(PlotData *lPlot, int aType) { // aType = group in PlotData
    if (mPlotListInUpdate.contains(lPlot->IdProject())
            || mPlotListInUpdate.contains(0)) return;

    if (mPlotsInUpdate.contains(lPlot)) return;
    emit PlotBeforeUpdate(lPlot, aType);
}

void ProjectList::EmitPlotNeedUpdate(PlotData *lPlot, int aType) { // aType = group in PlotData
    if (mPlotListInUpdate.contains(lPlot->IdProject())
            || mPlotListInUpdate.contains(0)) return;

    if (!mPlotsInUpdate.contains(lPlot)) {
        mPlotsInUpdate.append(lPlot);
        emit PlotNeedUpdate(lPlot, aType);
        mPlotsInUpdate.removeAll(lPlot);
    }
}
