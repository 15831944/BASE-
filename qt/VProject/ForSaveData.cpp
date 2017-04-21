#include "common.h"

#include "ForSaveData.h"
#include "BlobMemCache.h"

#include "../UsersDlg/UserRight.h"
#include "../PlotLib/DwgData.h"

#include <QCryptographicHash>

// this version of constructor is for fetching working version of xref
XrefForSaveData::XrefForSaveData(int aIdCommon, const QString &aBlockName) :
    PlotDwgData(0)
{
    mIdCommon = aIdCommon;
    mBlockName = aBlockName;

    QSqlQuery query(db);

//    // debug222 - need version_ext or needn't?
    query.prepare("select a.id id_plot, a.id_project, a.id_common, a.type_area, a.type,"
                  " a.version version_int, a.code, a.sheet_number, a.nametop, a.name,"
                  " a.edit_date, a.edit_user edit_user, a.comments,"
                  " d.id id_dwg_max, d.version dwg_version_max"
                  " from v_plot_simple a, v_dwg d"
                  " where a.id_common = ? and a.working = 1 and a.deleted = 0"
                  " and d.id_plot = a.id and d.version = (select max(version) from v_dwg where id_plot = a.id)");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("XrefForSaveData::XrefForSaveData", query);
    } else {
        query.addBindValue(aIdCommon);
        if (!query.exec()) {
            gLogger->ShowSqlError("XrefForSaveData::XrefForSaveData", query);
        } else {
            if (query.next()) {
                mId = query.value("id_plot").toInt();
                mIdProject = query.value("id_project").toInt();
                //mIdCommon = aIdCommon; at top
                mWorking = 1;
                mDeleted = 0;
                mVersionInt = query.value("version_int").toString();
                mTDArea = query.value("type_area").toInt();
                mTDId = query.value("type").toInt();
                mCode = query.value("code").toString();
                mSheet = query.value("sheet_number").toString();
                mNameTop = query.value("nametop").toString();
                mName = query.value("name").toString();
                mEditDate = query.value("edit_date").toDateTime();
                mEditUser = query.value("edit_user").toString();
                mNotes = query.value("comments").toString();
                mIdDwgMax = query.value("id_dwg_max").toInt();
                mDwgVersionMax = query.value("dwg_version_max").toInt();
                mUseWorking = 1;
                //mBlockName = aBlockName; at top

                mDwg->InitFromId(mIdDwgMax);
                InitEditStatus();

                // files and images list
                query.prepare("select b.id, a.file_name,"
                              " a.version, ftime, dbms_lob.getlength(data) data_length, a.grpname"
                              " from xref a, v_dwg b where a.id_dwg = :id1 and b.id = a.id_xref and a.deleted = 0");

                if (query.lastError().isValid()) {
                    gLogger->ShowSqlError("PlotForSaveData::InitFileList", query);
                } else {
                    query.bindValue(":id1", mIdDwgMax);
                    if (!query.exec()) {
                        gLogger->ShowSqlError("PlotForSaveData::InitFileList", query);
                    } else {
                        while (query.next()) {
                            int lIdDwg = query.value("id").toInt();
                            // Id, Filename, Version, FTime, DataLength, GroupName, InMain, InXref, Sha256

                            // load data from database; don't deicide yet but it is definitly required for images
                            // calc sha256
                            QCryptographicHash hash1(QCryptographicHash::Sha256);
                            hash1.addData(gBlobMemCache->GetData(BlobMemCache::Dwg, lIdDwg));


                            DwgForSaveData *lAddFile = new DwgForSaveData(lIdDwg, query.value("file_name").toString(), query.value("version").toInt(),
                                                                          query.value("ftime").toDateTime(), query.value("data_length").toLongLong(), query.value("grpname").toString(),
                                                                          false, true, QString(hash1.result().toHex()).toUpper());

                            if (lAddFile->FilenameConst().endsWith(".bmp", Qt::CaseInsensitive)
                                    || lAddFile->FilenameConst().endsWith(".dib", Qt::CaseInsensitive)
                                    || lAddFile->FilenameConst().endsWith(".gif", Qt::CaseInsensitive)
                                    || lAddFile->FilenameConst().endsWith(".jpeg", Qt::CaseInsensitive)
                                    || lAddFile->FilenameConst().endsWith(".jpg", Qt::CaseInsensitive)
                                    || lAddFile->FilenameConst().endsWith(".png", Qt::CaseInsensitive)
                                    || lAddFile->FilenameConst().endsWith(".tif", Qt::CaseInsensitive)
                                    || lAddFile->FilenameConst().endsWith(".tiff", Qt::CaseInsensitive)) {
                                // add to image list
                                mImages.append(lAddFile); // add to current list
                            } else {
                                // add to files list
                                mAddFiles.append(lAddFile); // add to current list
                            }
                        }
                    }
                }
            } else {
                gLogger->ShowError("XrefForSaveData::XrefForSaveData", QObject::tr("No data found!") + "\r\nIdCommon: " + QString::number(aIdCommon));
            }
        }
    }
}

XrefForSaveData::XrefForSaveData(int aIdPlot, int aIdProject, int aIdCommon, int aIdDwg, int aWorking, int aDeleted, const QString &aVersionInt,
                                 const QString &aCode, const QString &aSheet, const QString &aNameTop, const QString &aName,
                                 const QDateTime aEditDate,  const QString &aEditUser, const QString &aNotes,
                                 int aIdDwgMax, int aDwgVersionMax,
                                 int aUseWorking, const QString &aBlockName) :
    PlotDwgData(aIdDwg)
{
    mId = aIdPlot;
    mIdProject = aIdProject;
    mIdCommon = aIdCommon;
    mWorking = aWorking;
    mDeleted = aDeleted;
    mVersionInt = aVersionInt;
    mCode = aCode;
    mSheet = aSheet;
    mNameTop = aNameTop;
    mName = aName;
    mEditDate = aEditDate;
    mEditUser = aEditUser;
    mNotes = aNotes;
    mIdDwgMax = aIdDwgMax;
    mDwgVersionMax = aDwgVersionMax;
    mUseWorking = aUseWorking;
    mBlockName = aBlockName;
}

// -----------------------------------------------------------------------------------------------------------------------------------
PlotForSaveData::PlotForSaveData(int aIdPlot, int aIdDwg) :
    PlotDwgData(aIdDwg)
{
    InitFromPlotId(aIdPlot);
    if (!aIdDwg) mDwg->InitFromId(IdDwgMax());
}

PlotForSaveData::~PlotForSaveData() {
    qDeleteAll(mXrefs);
    qDeleteAll(mXrefsProps);
}

void PlotForSaveData::InitInternal(int aId, InitInternalType aType) {
    QSqlQuery query(db);

    mId = 0;

    switch (aType) {
    case IITIdPlot:
        if (gHasVersionExt) {
            query.prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type,"
                          " a.code, a.sheet_number, a.nametop, a.name, a.version, a.version_ext, a.working, a.deleted, a.comments,"
                          " nvl(a.block_name, 'xref' || trim(to_char(a.id))) block_name, a.sentdate,"
                          " a.edit_date, a.edit_user,"
                          " a.code_chd, a.nametop_chd, a.name_chd, a.version_chd, a.sheet_chd,"
                          " b.name_chd ProjNameChd, b.stage_chd ProjStageChd"
                          " from v_plot_simple a, v_project b where a.id = ? and a.id_project = b.id");
        } else {
            query.prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type,"
                          " a.code, a.sheet_number, a.nametop, a.name, a.version, a.working, a.deleted, a.comments,"
                          " nvl(a.block_name, 'xref' || trim(to_char(a.id))) block_name, a.sentdate,"
                          " a.edit_date, a.edit_user,"
                          " a.code_chd, a.nametop_chd, a.name_chd, a.version_chd, a.sheet_chd,"
                          " b.name_chd ProjNameChd, b.stage_chd ProjStageChd"
                          " from v_plot_simple a, v_project b where a.id = ? and a.id_project = b.id");
        }
        break;
/*    case IITIdDwg:
        // doesn't verify yet
        query.prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type,"
                      " a.code, a.sheet_number, a.nametop, a.name, a.version, a.working, a.deleted, a.comments,"
                      " nvl(a.block_name, 'xref' || trim(to_char(id))) block_name, a.sentdate,"
                      " a.edit_date, a.edit_user,"
                      " a.code_chd, a.nametop_chd, a.name_chd, a.version_chd, a.sheet_chd,"
                      " c.name_chd ProjNameChd, c.stage_chd ProjStageChd"
                      " from v_plot_simple a, v_dwg b, v_project c where a.id = b.id_plot and b.id = ? and a.id_project = c.id");
        break;*/
    case IITIdCommon:
        if (gHasVersionExt) {
            query.prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type,"
                          " a.code, a.sheet_number, a.nametop, a.name, a.version, a.version_ext, a.working, a.deleted, a.comments,"
                          " nvl(a.block_name, 'xref' || trim(to_char(a.id))) block_name, a.sentdate,"
                          " a.edit_date, a.edit_user,"
                          " a.code_chd, a.nametop_chd, a.name_chd, a.version_chd, a.sheet_chd,"
                          " b.name_chd ProjNameChd, b.stage_chd ProjStageChd"
                          " from v_plot_simple a, v_project b where a.id_common = ? and a.working = 1 and a.id_project = b.id");
        } else {
            query.prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type,"
                          " a.code, a.sheet_number, a.nametop, a.name, a.version, a.working, a.deleted, a.comments,"
                          " nvl(a.block_name, 'xref' || trim(to_char(a.id))) block_name, a.sentdate,"
                          " a.edit_date, a.edit_user,"
                          " a.code_chd, a.nametop_chd, a.name_chd, a.version_chd, a.sheet_chd,"
                          " b.name_chd ProjNameChd, b.stage_chd ProjStageChd"
                          " from v_plot_simple a, v_project b where a.id_common = ? and a.working = 1 and a.id_project = b.id");
        }
        break;
    }

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("PlotForSaveData::InitInternal", query);
    } else {
        query.addBindValue(aId);
        if (!query.exec()) {
            gLogger->ShowSqlError("PlotForSaveData::InitInternal", query);
        } else {
            if (query.next()) {
                mId = query.value("id").toInt();
                mIdProject = query.value("id_project").toInt();
                mIdCommon = query.value("id_common").toInt();
                mTDArea = query.value("type_area").toInt();
                mTDId = query.value("type").toInt();
                mCode = query.value("code").toString();
                mSheet = query.value("sheet_number").toString();
                // mExtension not used here, we can use extension from DwgData
                mNameTop = query.value("nametop").toString();
                mName = query.value("name").toString();
                mVersionInt = query.value("version").toString();
                if (gHasVersionExt) {
                    mVersionExt = query.value("version_ext").toString();
                }
                mWorking = query.value("working").toInt();
                mDeleted = query.value("deleted").toInt();
                mNotes = query.value("comments").toString();

                mBlockName = query.value("block_name").toString();
                mSentDate = query.value("sentdate").toDate();
                mEditDate = query.value("edit_date").toDateTime();
                mEditUser = query.value("edit_user").toString();

                mCodeChd = query.value("code_chd").toDateTime();
                mNameTopChd = query.value("nametop_chd").toDateTime();
                mNameChd = query.value("name_chd").toDateTime();
                mVersionChd = query.value("version_chd").toDateTime();
                mSheetChd = query.value("sheet_chd").toDateTime();

                mProjNameChd = query.value("ProjNameChd").toDateTime();
                mProjStageChd = query.value("ProjStageChd").toDateTime();

                InitIdDwgMax();
                InitEditStatus();
            } else {
                gLogger->ShowError("PlotForSaveData::InitInternal", "Data not found!");
            }
        }
    }

}

void PlotForSaveData::InitFromPlotId(int aId) {
    InitInternal(aId, IITIdPlot);
}

void PlotForSaveData::InitFromCommonId(int aId) {
    InitInternal(aId, IITIdCommon);
}

void PlotForSaveData::InitXrefList() {
    QSqlQuery query(db);

    qDeleteAll(mXrefs);
    mXrefs.clear();

    // debug222 - need version_ext or needn't?
    query.prepare("select a.id id_plot, a.id_project, a.id_common, b.id id_dwg, a.working, a.deleted,"
                  " a.version version_int, a.code, a.sheet_number, a.nametop, a.name,"
                  " a.edit_date, a.edit_user edit_user, a.comments,"
                  " c.use_working, nvl(c.block_name, 'xref' || trim(to_char(a.id))) block_name,"
                  " d.id id_dwg_max, d.version dwg_version_max"
                  " from v_plot_simple a, v_dwg b, v_xref2dwg c, v_dwg d"
                  " where a.id = b.id_plot and b.id = c.id_dwg_xref and c.id_dwg_main = ?"
                  " and d.id_plot = a.id and d.version = (select max(version) from v_dwg where id_plot = a.id)");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("PlotForSaveData::InitXrefList", query);
    } else {
        if (mDwg) {
            query.addBindValue(mDwg->Id());
        } else {
            query.addBindValue(mIdDwgMax);
        }
        if (!query.exec()) {
            gLogger->ShowSqlError("PlotForSaveData::InitXrefList", query);
        } else {
            while (query.next()) {
                mXrefs.append(new XrefForSaveData(query.value("id_plot").toInt(), query.value("id_project").toInt(), query.value("id_common").toInt(),
                                                  query.value("id_dwg").toInt(), query.value("working").toInt(), query.value("deleted").toInt(),
                                                  query.value("version_int").toString(), query.value("code").toString(), query.value("sheet_number").toString(),
                                                  query.value("nametop").toString(), query.value("name").toString(),
                                                  query.value("edit_date").toDateTime(), query.value("edit_user").toString(), query.value("comments").toString(),
                                                  query.value("id_dwg_max").toInt(), query.value("dwg_version_max").toInt(),
                                                  query.value("use_working").toInt(), query.value("block_name").toString()));
            }
        }
    }
}

void PlotForSaveData::InitXrefPropsList() {
    QSqlQuery queryXref(db), queryObjProp(db);

    qDeleteAll(mXrefsProps);
    mXrefsProps.clear();

    queryXref.prepare(
                "select distinct a.id_dwg_xref, b.id_plot, c.id_common from v_xref2obj a, v_dwg b, v_plot_simple c"
                " where a.id_dwg_main = ?"
                " and a.id_dwg_xref = b.id"
                " and b.id_plot = c.id");

    if (queryXref.lastError().isValid()) {
        gLogger->ShowSqlError("PlotForSaveData::InitXrefPropsList - queryXref.prepare", queryXref);
    } else {
        if (mDwg) {
            queryXref.addBindValue(mDwg->Id());
        } else {
            queryXref.addBindValue(mIdDwgMax);
        }
        if (!queryXref.exec()) {
            gLogger->ShowSqlError("PlotForSaveData::InitXrefPropsList - queryXref.exec", queryXref);
        } else {
            // through xrefs
            while (queryXref.next()) {
                mXrefsProps.append(new XrefPropsData(queryXref.value("id_common").toInt()));

                // NB: order by a.type desc - it is correct, cos apply order is - blocks (hahaha dva raza), layers (hahaha) and then entire drawing (except previously applied blocks and layers).
                queryObjProp.prepare("select a.type, a.name ObjName, b.name PropName, b.value from v_xref2obj a, v_xref2objprop b"
                              " where b.id_xref2obj = a.id"
                              " and a.id_dwg_main = ? and a.id_dwg_xref = ?"
                              " order by a.type desc, decode(instr(a.name, '*'), 0, 10240) desc, b.name, b.value");
                if (queryObjProp.lastError().isValid()) {
                    gLogger->ShowSqlError("PlotForSaveData::InitXrefPropsList - queryObjProp.prepare", queryObjProp);
                    break;
                } else {
                    if (mDwg) {
                        queryObjProp.addBindValue(mDwg->Id());
                    } else {
                        queryObjProp.addBindValue(mIdDwgMax);
                    }
                    queryObjProp.addBindValue(queryXref.value("id_dwg_xref").toInt());
                    if (!queryObjProp.exec()) {
                        gLogger->ShowSqlError("PlotForSaveData::InitXrefPropsList - queryObjProp.exec", queryObjProp);
                        break;
                    } else {
                        // through one xref property
                        XrefOnePropData *lXrefOneProp = NULL; // mIdMain, mIdXref, mIdXrefPlot???
                        while (queryObjProp.next()) {
                            if (!lXrefOneProp) lXrefOneProp = new XrefOnePropData();

                            if (lXrefOneProp->ObjType() != queryObjProp.value("type").toInt()
                                    || lXrefOneProp->Name() != queryObjProp.value("ObjName").toString()) {

                                if (lXrefOneProp->ObjType() != -1 /* it is in constructor*/
                                        && !lXrefOneProp->IsDefault()) {
                                    // add to xrefs' props list
                                    mXrefsProps.last()->XrefPropsRef().append(lXrefOneProp);
                                    lXrefOneProp = new XrefOnePropData();
                                }

                                lXrefOneProp->SetObjType(queryObjProp.value("type").toInt());
                                lXrefOneProp->SetName(queryObjProp.value("ObjName").toString());
                            }

                            switch (lXrefOneProp->ObjType()) {
                            case 0:
                                if (queryObjProp.value("PropName").toString() == "0LayerName") {
                                    lXrefOneProp->SetLayer0Name(queryObjProp.value("value").toString());
                                } else if (queryObjProp.value("PropName").toString() == "AllBlocksColor") {
                                    lXrefOneProp->SetAllBlocksColor(queryObjProp.value("value").toInt());
                                } else if (queryObjProp.value("PropName").toString() == "AllEntitiesColor") {
                                    lXrefOneProp->SetAllEntitiesColor(queryObjProp.value("value").toInt());
                                } else if (queryObjProp.value("PropName").toString() == "Disabled") {
                                    lXrefOneProp->SetDisabled(1);
                                } else if (queryObjProp.value("PropName").toString() == "AllBlocksLineweight") {
                                    lXrefOneProp->SetAllBlocksLineweight(queryObjProp.value("value").toInt());
                                } else if (queryObjProp.value("PropName").toString() == "AllEntitiesLineweight") {
                                    lXrefOneProp->SetAllEntitiesLineweight(queryObjProp.value("value").toInt());
                                }
                                break;
                            case 1:
                                if (queryObjProp.value("PropName").toString() == "LayerEntColor") {
                                    lXrefOneProp->SetLayerEntitiesColor(queryObjProp.value("value").toInt());
                                } else if (queryObjProp.value("PropName").toString() == "LayerDisabled") {
                                    lXrefOneProp->SetDisabled(1);
                                } else if (queryObjProp.value("PropName").toString() == "LayerNewName") {
                                    lXrefOneProp->SetLayerNewName(queryObjProp.value("value").toString());
                                } else if (queryObjProp.value("PropName").toString() == "LayerEntLineweight") {
                                    lXrefOneProp->SetLayerEntitiesLineweight(queryObjProp.value("value").toInt());
                                }
                                break;
                            }
                        }

                        // add to xrefs' props list
                        if (lXrefOneProp
                                && lXrefOneProp->ObjType() != -1 /* it is in constructor*/
                                && !lXrefOneProp->IsDefault()) {
                            mXrefsProps.last()->XrefPropsRef().append(lXrefOneProp);
                        }
                    }
                }
            }
        }
    }
}

// in this version - only one level of nested xrefs
void PlotForSaveData::InitFileList(bool aIsDwg) {
    QSqlQuery query(db);

    qDeleteAll(mImages);
    mImages.clear();

    qDeleteAll(mAddFiles);
    mAddFiles.clear();

    query.prepare("select b.id, a.file_name,"
                  " a.version, ftime, dbms_lob.getlength(data) data_length, a.grpname"
                  " from xref a, v_dwg b where a.id_dwg = :id1 and b.id = a.id_xref and a.deleted = 0");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("PlotForSaveData::InitFileList", query);
    } else {
        if (mDwg) {
            query.bindValue(":id1", mDwg->Id());
        } else {
            query.bindValue(":id1", mIdDwgMax);
        }
        if (!query.exec()) {
            gLogger->ShowSqlError("PlotForSaveData::InitFileList", query);
        } else {
            int lCurXref = 0;
            do {
                while (query.next()) {
                    int lIdDwg = query.value("id").toInt();
                    // Id, Filename, Version, FTime, DataLength, GroupName, InMain, InXref, Sha256

                    // load data from database; don't deicide yet but it is definitly required for images
                    // calc sha256
                    QCryptographicHash hash1(QCryptographicHash::Sha256);
                    hash1.addData(gBlobMemCache->GetData(BlobMemCache::Dwg, lIdDwg));

                    DwgForSaveData *lAddFile = new DwgForSaveData(lIdDwg, query.value("file_name").toString(), query.value("version").toInt(),
                                        query.value("ftime").toDateTime(), query.value("data_length").toLongLong(), query.value("grpname").toString(),
                                        !lCurXref, lCurXref, QString(hash1.result().toHex()).toUpper());

                    if (aIsDwg) {
                        if (lAddFile->FilenameConst().endsWith(".bmp", Qt::CaseInsensitive)
                                || lAddFile->FilenameConst().endsWith(".dib", Qt::CaseInsensitive)
                                || lAddFile->FilenameConst().endsWith(".gif", Qt::CaseInsensitive)
                                || lAddFile->FilenameConst().endsWith(".jpeg", Qt::CaseInsensitive)
                                || lAddFile->FilenameConst().endsWith(".jpg", Qt::CaseInsensitive)
                                || lAddFile->FilenameConst().endsWith(".png", Qt::CaseInsensitive)
                                || lAddFile->FilenameConst().endsWith(".tif", Qt::CaseInsensitive)
                                || lAddFile->FilenameConst().endsWith(".tiff", Qt::CaseInsensitive)) {
                            // add to image list
                            if (!lCurXref)
                                mImages.append(lAddFile); // add to current list
                            else
                                mXrefs[lCurXref - 1]->ImagesRef().append(lAddFile); // add to xref's list
                        } else {
                            // add to files list
                            if (!lCurXref)
                                mAddFiles.append(lAddFile); // add to current list
                            else {
                                // ignore all except fonts type of additional files for xrefs
                                if (lAddFile->GroupNameConst() == "Acad:Text") {
                                    mXrefs[lCurXref - 1]->AddFilesRef().append(lAddFile); // add to xref's list
                                }
                            }
                        }
                    } else {
                        mImages.append(lAddFile); // add to current list
                    }
                }

                // collect for xrefs too
                if (aIsDwg && lCurXref < mXrefs.length()) {
                    // collect for xrefs
                    query.bindValue(":id1", mXrefs.at(lCurXref)->DwgConst()->Id());
                    query.bindValue(":id2", mXrefs.at(lCurXref)->DwgConst()->Id());
                    if (!query.exec()) {
                        gLogger->ShowSqlError("PlotForSaveData::InitFileList", query);
                        break;
                    }
                    lCurXref++;
                } else {
                    break;
                }
            } while (true);
        }
    }
}

// no nested xrefs awhile, but just can call this function for every xref, right?
//void PlotForSaveData::InitFileList(bool aIsDwg) {
//    QSqlQuery query(db);

//    mImages.clear();
//    mAddFiles.clear();

//    query.prepare("select b.id, a.file_name,"
//                  " a.version, ftime, dbms_lob.getlength(data) data_length"
//                  " from xref a, v_dwg b where a.id_dwg = ? and b.id = a.id_xref and a.deleted = 0");

//    if (query.lastError().isValid()) {
//        QgLogger->ShowSqlError("PLOT data", query);
//    } else {
//        query.addBindValue(mDwg.mId);
//        query.addBindValue(mDwg.mId);
//        if (!query.exec()) {
//            gLogger->ShowSqlError("PLOT data", query);
//        } else {
//            BlobMemCache *bmc = gSettings->GetBlobMemCache();
//            while (query.next()) {
//                DwgData lAddFile;
//                lAddFile.mId = query.value("id").toInt();
//                lAddFile.mExtension = query.value("file_name").toString();
//                lAddFile.mVersion = query.value("version").toInt();
//                lAddFile.mFTime = query.value("ftime").toDateTime();
//                lAddFile.mDataLength = query.value("data_length").toLongLong();

//                // load data from database; don't deicide yet but it is definitly required for images
//                if (lAddFile.mId > 0)
//                    bmc->AddToCache(bmc->Dwg, lAddFile.mId);
//                else
//                    bmc->AddToCache(bmc->DwgRO, -lAddFile.mId);
//                //lAddFile.ReadDataFromDb();

//                if (aIsDwg && (lAddFile.mExtension.endsWith(".bmp", Qt::CaseInsensitive)
//                               || lAddFile.mExtension.endsWith(".dib", Qt::CaseInsensitive)
//                               || lAddFile.mExtension.endsWith(".gif", Qt::CaseInsensitive)
//                               || lAddFile.mExtension.endsWith(".jpeg", Qt::CaseInsensitive)
//                               || lAddFile.mExtension.endsWith(".jpg", Qt::CaseInsensitive)
//                               || lAddFile.mExtension.endsWith(".png", Qt::CaseInsensitive)
//                               || lAddFile.mExtension.endsWith(".tif", Qt::CaseInsensitive)
//                               || lAddFile.mExtension.endsWith(".tiff", Qt::CaseInsensitive))) {
//                    // add to image list
//                    mImages.append(lAddFile);
//                } else {
//                    // add to files list
//                    mAddFiles.append(lAddFile);
//                }
//            }
//        }
//    }
//}

int PlotForSaveData::NeedUpdateFields() const {
    int aRes = 0;

#define IsNeedUpdate(aIntId, aChd, aOrValue) \
    if (mDwg->VFDateConst().isNull() || !mDwg->VFDateConst().isValid() || mDwg->FieldTypesConst().contains(aIntId) && aChd > mDwg->VFDateConst()) aRes |= aOrValue;

    IsNeedUpdate(102, mCodeChd, 1);
    IsNeedUpdate(103, mNameTopChd, 2);
    IsNeedUpdate(104, mNameChd, 4);
    IsNeedUpdate(107, mProjNameChd, 8);
    IsNeedUpdate(108, mVersionChd, 0x10);
    IsNeedUpdate(109, mProjStageChd, 0x20);
    IsNeedUpdate(110, mSheetChd, 0x40);

    if (mDwg->VFDateConst().isNull() || !mDwg->VFDateConst().isValid()
            || mDwg->FieldTypesConst().contains(111) && (mCodeChd > mDwg->VFDateConst() || mSheetChd > mDwg->VFDateConst())) aRes |= 0x80;
#undef IsNeedUpdate
    return aRes;
}
