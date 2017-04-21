#include "ProjectTypeData.h"

#include "../VProject/common.h"

ProjectTypeData::ProjectTypeData() {
    mId = -1; // it os program default
    mVerType = 1;
    mVerLen = 0;
    mVerLenFixed = false;
    mVerStart = "1";
    mSheetLen = 0;
    mSheetStart = 1;
    mNoNumTempl = "!NO_CODE-%NNNN%";
}

ProjectTypeData::ProjectTypeData(int aId, const QString &aTypeName, int aVerType, int aVerLen, bool aVerLenFixed, const QString &aVerStart,
                int aSheetLen, int aSheetStart, const QString &aDefTempl, const QString &aNoNumTempl) :
    InitParRO(Id), InitParRO(TypeName), InitParRO(VerType), InitParRO(VerLen), InitParRO(VerLenFixed), InitParRO(VerStart),
    InitParRO(SheetLen), InitParRO(SheetStart), InitParRO(DefTempl), InitParRO(NoNumTempl)
{
    QSqlQuery query("select stagename from v_project_stage_d where id_project_type = " + QString::number(aId) + " order by order_num", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project data"), query);
    } else {
        while (query.next()) {
            mStages.append(qString("stagename"));
        }
    }
}

void ProjectTypeData::InitGrpData(int aGrpNum) {
    if (!mInited.contains(aGrpNum)) {
        switch (aGrpNum) {
        case 1:
//            LoadPlots();
            break;
        }
    }
}

//void ProjectTypeData::LoadPlots() {
//    if (!mInited.contains(1)) mInited.append(1);

//    qDeleteAll(mPlots);
//    mPlots.clear();

//    QSqlQuery query(db);
////    queryPlot.prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id id_dwg, b.version dwg_version,"
////                      " a.cancelled, a.cancdate, a.cancuser,"
////                      " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.stage, a.code, a.sheet_number, a.extension,"
////                      " a.nametop, a.name, a.block_name,"
////                      " a.crdate, a.cruser, a.edit_date, a.edit_user,"
////                      " dbms_lob.getlength(b.data) data_length,"
////                      " (select count(1) from v_xref2dwg where id_dwg_main = b.id) xrefs_cnt,"
////                      " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
////                      " a.comments"
////                      " from (select * from v_plot_simple where id_project = :IdProject and deleted = 0 and working = 1) a"
////                      " left outer join v_dwg b on b.id_plot = a.id"
////                      " where (b.version = (select max(version) from v_dwg where id_plot = a.id) or b.version is null)");

//    if (query.lastError().isValid()) {
//        gLogger->ShowSqlError("PlotData::LoadLayouts", query);
//    } else {
//        //query.bindValue(":id_dwg", mIdDwgMax);
//        if (!query.exec()) {
//            gLogger->ShowSqlError("PlotData::LoadLayouts", query);
//        } else {
//            while (query.next()) {
////                PlotData *lPlot = new PlotData(query.value("id").toInt(), mIdDwgMax, query.value("num").toInt(),
////                                          query.value("name").toString(), query.value("sheet").toString(), query.value("namebottom").toString());
////                mPlots.append(lPlot);
//            }
//        }
//    }
//}


const QStringList &ProjectTypeData::StagesConst() const {
    return mStages;
}

//------------------------------------
ProjectTypeList::ProjectTypeList() {
    QSqlQuery query("select id, typename, ver_type, ver_len, ver_len_fixed, ver_start,"
                    " sheet_len, sheet_start, def_template, nonum_template"
                    " from v_project_type_d order by order_num", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Project type data"), query);
    } else {
        while (query.next()) {
            mProjTypeList.append(
                        new ProjectTypeData(query.value("id").toInt(), query.value("typename").toString(),
                                            query.value("ver_type").toInt(), query.value("ver_len").toInt(),
                                            query.value("ver_len_fixed").toInt() == 1, query.value("ver_start").toString(),
                                            query.value("sheet_len").toInt(), query.value("sheet_start").toInt(),
                                            query.value("def_template").toString(), query.value("nonum_template").toString()));
        }
    }
}

ProjectTypeList * ProjectTypeList::GetInstance() {
    static ProjectTypeList * lProjectTypeList = NULL;
    if (!lProjectTypeList) {
        lProjectTypeList = new ProjectTypeList();
        //qAddPostRoutine(ProjectList::clean);
    }
    return lProjectTypeList;
}

const QList<ProjectTypeData *> & ProjectTypeList::ProjTypeListConst() {
    return mProjTypeList;
}

const ProjectTypeData *ProjectTypeList::GetById(int aId) const {
    for (int j = 0; j < 2; j++) {
        if (j) aId = 0; // not found with this id, use default (with id = 0)
        for (int i = 0; i < mProjTypeList.length(); i++) {
            if (mProjTypeList.at(i)->Id() == aId) {
                return mProjTypeList.at(i);
            }
        }
    }
    return &mDefaultProjectTypeData;
}

const ProjectTypeData *ProjectTypeList::DefaultProjectTypeData() const {
    return &mDefaultProjectTypeData;
}
