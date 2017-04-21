#include "DwgData.h"

#include "../VProject/common.h"
#include "../VProject/FileUtils.h"

DwgData::DwgData(int aId) :
    mId(0), mIdPlot(0), mIdPlotEdge(0), mVersion(-1), mNeedNotProcess(0), mNestedXrefMode(0), mLayoutCnt(-1), mInSubs(0),
    mDataLength(0), mAcadVer(0), mIdCache(0)
{
    if (aId) InitFromId(aId);
}

QString DwgData::AcadVerStr() const {
    if (mAcadVer > 14) {
        return QString::number(mAcadVer);
    } else if (mAcadVer == 14) {
        return "R14";
    } else {
        return "";
    }
}

void DwgData::InitFromId(int aId) {
    QSqlQuery query(db);


    query.prepare(QString() + "select coalesce(id_plot, 0) id_plot, id_plotedge, version, extension,"
                        " coalesce(neednotprocess, 0) as neednotprocess, coalesce(nestedxrefmode, 0) as nestedxrefmode,"
                        " coalesce(layout_cnt, -1) as layout_cnt, ftime, coalesce(InSubs, 0) as InSubs, vf_date,"
                        + ((db.driverName()== "QPSQL")?" length(data) as data_length,":" dbms_lob.getlength(data) data_length,")
                        + ((db.driverName()== "QPSQL")?" substring(a.data from 1 for 6) as head6char,":" utl_raw.cast_to_varchar2(dbms_lob.substr(a.data, 6)) head6char,")
                        + " (select id from v_dwg_cache where id_dwg = a.id and coalesce(id_plotedge, 0) = coalesce(a.id_plotedge, 0)) as id_cache"
                        " from v_dwg a where id = ?");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("DWG data", query);
    } else {
        query.addBindValue(aId);
        if (!query.exec()) {
            gLogger->ShowSqlError("DWG data", query);
        } else {
            if (query.next()) {
                mId = aId;
                mIdPlot = query.value("id_plot").toInt();
                mIdPlotEdge = query.value("id_plotedge").toInt();
                mVersion = query.value("version").toInt();
                mExtension = query.value("extension").toString();
                mNeedNotProcess = query.value("neednotprocess").toInt();
                mNestedXrefMode = query.value("nestedxrefmode").toInt();
                mLayoutCnt = query.value("layout_cnt").toInt();
                mFTime = query.value("ftime").toDateTime();
                mInSubs = query.value("InSubs").toInt();
                mVFDate = query.value("vf_date").toDateTime();
                mDataLength = query.value("data_length").toLongLong();
                mAcadVer = gFileUtils->AcadVersion(query.value("head6char").toString());
                mIdCache = query.value("id_cache").toInt();

                // fields
                /*mFields.clear();
                query.prepare("select distinct type from v_dwg_field a where id_dwg = ?");

                if (query.lastError().isValid()) {
                    gLogger->ShowSqlError("DWG data", query);
                } else {
                    query.addBindValue(aId);
                    if (!query.exec()) {
                        gLogger->ShowSqlError("DWG data", query);
                    } else {
                        while (query.next()) {
                            mFields.append(query.value(0).toInt());
                        }
                    }
                }*/
            } else {
                gLogger->ShowError("DWG data", "Data not found!");
            }
        }
    }
}

void DwgData::InitGrpData(int aGrpNum) {
    if (!mInited.contains(aGrpNum)) {
        switch (aGrpNum) {
        case 1:
            LoadFieldTypes();
            break;
        }
    }
}

void DwgData::LoadFieldTypes() {

    if (!mInited.contains(1)) mInited.append(1);

    mFieldTypes.clear();

    QSqlQuery query(db);
    query.prepare("select distinct type from v_dwg_field a where id_dwg = ?");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("DwgData::LoadFieldTypes", query);
    } else {
        query.addBindValue(mId);
        if (!query.exec()) {
            gLogger->ShowSqlError(NULL, "DwgData::LoadFieldTypes", query);
        } else {
            while (query.next()) {
                mFieldTypes.append(query.value(0).toInt());
            }
        }
    }
}

bool DwgData::INSERT(quint64 &aId, int aIdPlot, int aVersion, const QString &aExtension, const QString &aSha256, int aNeedNotProcess, int aLayoutCnt,
                      const QDateTime &aFTime, const QByteArray *aData) {

    if (!aId && !gOracle->GetSeqNextVal("dwg_id_seq", aId)) {
        return false;
    }

    bool res = false;
    QSqlQuery qInsert(db);

    if (aData && !aData->isNull())
        qInsert.prepare("insert into v_dwg (id, id_plot, version, extension, sha256, neednotprocess, layout_cnt, ftime, data)"
                        " values (:id, :id_plot, :version, :extension, :sha256, :neednotprocess, :layout_cnt, :ftime, :data)");
    else
        qInsert.prepare("insert into v_dwg (id, id_plot, version, extension, sha256, neednotprocess, layout_cnt, ftime)"
                        " values (:id, :id_plot, :version, :extension, :sha256, :neednotprocess, :layout_cnt, :ftime)");
    if (qInsert.lastError().isValid()) {
        gLogger->ShowSqlError("DWG data - prepare", qInsert);
    } else {
        qInsert.bindValue(":id", aId);

        if (aIdPlot)
            qInsert.bindValue(":id_plot", aIdPlot);
        else
            qInsert.bindValue(":id_plot", QVariant());
        if (aVersion)
            qInsert.bindValue(":version", aVersion);
        else
            qInsert.bindValue(":version", QVariant());

        qInsert.bindValue(":extension", aExtension);
        qInsert.bindValue(":sha256", aSha256);
        if (aNeedNotProcess == 1)
            qInsert.bindValue(":neednotprocess", aNeedNotProcess);
        else
            qInsert.bindValue(":neednotprocess", QVariant());

        if (aLayoutCnt != -1)
            qInsert.bindValue(":layout_cnt", aLayoutCnt);
        else
            qInsert.bindValue(":layout_cnt", QVariant());

        qInsert.bindValue(":ftime", aFTime);
        if (aData)
            qInsert.bindValue(":data", *aData);

        if (!qInsert.exec()) {
            gLogger->ShowSqlError("DWG data - execute", qInsert);
        } else {
            res = true;
        }
    }

    return res;
}

bool DwgData::CopyDwgXrefs(quint64 aIdOld, quint64 aIdNew) {
//    QSqlQuery qInsert1(db), qInsert2(db), qSelect(db);

//    // copy xrefs
//    qInsert1.prepare("insert into v_xref2dwg(id_dwg_main, id_dwg_xref, id_dwg_xref_version, use_working, block_name, attach_mode, hidden)"
//                     " (select :id_new, id_dwg_xref, id_dwg_xref_version, use_working, block_name, attach_mode, hidden from v_xref2dwg"
//                     " where id_dwg_main = :id_old)");
//    if (qInsert1.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert1);
//        return false;
//    }
//    qInsert1.bindValue(":id_old", aIdOld);
//    qInsert1.bindValue(":id_new", aIdNew);

//    if (!qInsert1.exec()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert1);
//        return false;
//    }

//    // copy xref2obj (with child xref2objprop)
//    qInsert1.prepare(QString("insert into v_xref2obj(id, id_dwg_main, id_dwg_xref, type, name) values (")
//                     + ((db.driverName() == "QPSQL")
//                     ?"nextval('xref2obj_id_seq')"
//                     :"xref2obj_id_seq.nextval")
//                     + ", :id_new, :id_dwg_xref, :type, :name)");
//    if (qInsert1.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert1);
//        return false;
//    }
//    qInsert1.bindValue(":id_new", aIdNew);

//    qInsert2.prepare(QString("insert into v_xref2objprop (id_xref2obj, name, value) (select ")
//                     + ((db.driverName() == "QPSQL")
//                     ?"currval('xref2obj_id_seq')"
//                     :"xref2obj_id_seq.currval")
//                     + ", name, value from v_xref2objprop where id_xref2obj = :id_xref2obj_old)");
//    if (qInsert2.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert2);
//        return false;
//    }

//    qSelect.prepare("select id, id_dwg_xref, type, name from v_xref2obj where id_dwg_main = :id_old");
//    if (qSelect.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qSelect);
//        return false;
//    }

//    qSelect.bindValue(":id_old", aIdOld);
//    if (!qSelect.exec()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qSelect);
//        return false;
//    }
//    while (qSelect.next()) {
//        qInsert1.bindValue(":id_dwg_xref", qSelect.value("id_dwg_xref"));
//        qInsert1.bindValue(":type", qSelect.value("type"));
//        qInsert1.bindValue(":name", qSelect.value("name"));
//        if (!qInsert1.exec()) {
//            gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert1);
//            return false;
//        }
//        qInsert2.bindValue(":id_xref2obj_old", qSelect.value("id"));
//        if (!qInsert2.exec()) {
//            gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert2);
//            return false;
//        }
//    }

//    // copy xref2order
//    qInsert1.prepare("insert into v_xref2order(id_dwg_main, id_common, draw_order)"
//                     " (select :id_new, id_common, draw_order from v_xref2order"
//                     " where id_dwg_main = :id_old)");
//    if (qInsert1.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert1);
//        return false;
//    }
//    qInsert1.bindValue(":id_old", aIdOld);
//    qInsert1.bindValue(":id_new", aIdNew);

//    if (!qInsert1.exec()) {
//        gLogger->ShowSqlError(QObject::tr("Copy xrefs"), qInsert1);
//        return false;
//    }

//    return true;

    bool res = false;
    QSqlQuery qCopyDwgXrefs(db);

    qCopyDwgXrefs.prepare(
                (db.driverName() == "QPSQL")
                ?"perform pp_CopyDwgXrefs(:id_old, :id_new);"
                :"begin pp.CopyDwgXrefs(:id_old, :id_new); end;");
    if (qCopyDwgXrefs.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Copy dwg xrefs") + " - prepare CopyDwgXrefs", qCopyDwgXrefs);
    } else {
        qCopyDwgXrefs.bindValue(":id_old", aIdOld);
        qCopyDwgXrefs.bindValue(":id_new", aIdNew);

        if (!qCopyDwgXrefs.exec()) {
            gLogger->ShowSqlError(QObject::tr("Copy dwg xrefs") + " - execute CopyDwgXrefs", qCopyDwgXrefs);
        } else {
            res = true;
        }
    }
    return res;
}

bool DwgData::CopyAllRefs(quint64 aIdOld, quint64 aIdNew) {
//    QSqlQuery qInsert1(db), qInsert2(db), qInsert3(db), qSelect1(db), qSelect2(db);

//    // copy xrefs
//    qInsert1.prepare("insert into xref(id_dwg, path, file_name, version, id_xref, grpname)"
//                     " (select :id_new, path, file_name, version, id_xref, grpname from xref"
//                     " where id_dwg = :id_old and deleted = 0)");
//    if (qInsert1.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert1);
//        return false;
//    }
//    qInsert1.bindValue(":id_old", aIdOld);
//    qInsert1.bindValue(":id_new", aIdNew);

//    if (!qInsert1.exec()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert1);
//        return false;
//    }

//    // copy xrefs
//    if (!CopyDwgXrefs(aIdOld, aIdNew)) return false;


//    // copy dwg layouts
//    qInsert1.prepare(QString("insert into v_dwg_layout(id, id_dwg, num, name, sheet, namebottom, name_acad, sheet_acad, namebottom_acad,"
//                     " updatefromacad, updatefromgui, handle_hi, handle_lo)"
//                     " values (")
//                     + ((db.driverName() == "QPSQL")
//                     ?"nextval('dwg_layout_id_seq')"
//                     :"dwg_layout_id_seq.nextval")
//                     + ", :id_new, :num, :name, :sheet, :namebottom, :name_acad, :sheet_acad, :namebottom_acad,"
//                     " :updatefromacad, :updatefromgui, :handle_hi, :handle_lo)");
//    if (qInsert1.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert1);
//        return false;
//    }
//    qInsert1.bindValue(":id_new", aIdNew);

//    qInsert2.prepare(QString("insert into v_dwg_layout_block(id, id_dwg_layout, name, handle_hi, handle_lo) values (")
//                     + ((db.driverName() == "QPSQL")
//                     ?"nextval('dwg_layout_block_id_seq')"
//                     :"dwg_layout_block_id_seq.nextval")
//                     + ", "
//                     + ((db.driverName() == "QPSQL")
//                     ?"currval('dwg_layout_id_seq')"
//                     :"dwg_layout_id_seq.currval")
//                     + ", :name, :handle_hi, :handle_lo)");
//    if (qInsert2.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert2);
//        return false;
//    }

//    qInsert3.prepare(QString("insert into v_dwg_lb_attr(id_dwg_lb, ordernum, tag, prompt, userfriendly_value, encoded_value,"
//                     " userfriendly_value_acad, encoded_value_acad,"
//                     " text_style, text_font, text_hormode, text_vertmode, text_backward, text_upsidedown, text_height, text_width,"
//                     " text_rotation, text_oblique, prop_layer, prop_linetype, prop_color, prop_lineweight, prop_plotstyle,"
//                     " updatefromacad, updatefromgui, handle_hi, handle_lo)"
//                     " (select ")
//                     + ((db.driverName() == "QPSQL")
//                     ?"currval('dwg_layout_block_id_seq')"
//                     :"dwg_layout_block_id_seq.currval")
//                     + ", ordernum, tag, prompt, userfriendly_value, encoded_value,"
//                     " userfriendly_value_acad, encoded_value_acad,"
//                     " text_style, text_font, text_hormode, text_vertmode, text_backward, text_upsidedown, text_height, text_width,"
//                     " text_rotation, text_oblique, prop_layer, prop_linetype, prop_color, prop_lineweight, prop_plotstyle,"
//                     " updatefromacad, updatefromgui, handle_hi, handle_lo from v_dwg_lb_attr where id_dwg_lb = :id_dwg_lb)");
//    if (qInsert3.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert3);
//        return false;
//    }

//    qSelect1.prepare("select id, num, name, sheet, namebottom, name_acad, sheet_acad, namebottom_acad,"
//                     " updatefromacad, updatefromgui, handle_hi, handle_lo from v_dwg_layout"
//                     " where id_dwg = :id_old");
//    if (qSelect1.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qSelect1);
//        return false;
//    }

//    qSelect2.prepare("select id, name, handle_hi, handle_lo from v_dwg_layout_block"
//                     " where id_dwg_layout = :id_dwg_layout");
//    if (qSelect2.lastError().isValid()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qSelect2);
//        return false;
//    }

//    qSelect1.bindValue(":id_old", aIdOld);
//    if (!qSelect1.exec()) {
//        gLogger->ShowSqlError(QObject::tr("Copy all refs"), qSelect1);
//        return false;
//    }
//    while (qSelect1.next()) {
//        qInsert1.bindValue(":num", qSelect1.value("num"));
//        qInsert1.bindValue(":name", qSelect1.value("name"));
//        qInsert1.bindValue(":sheet", qSelect1.value("sheet"));
//        qInsert1.bindValue(":namebottom", qSelect1.value("namebottom"));
//        qInsert1.bindValue(":name_acad", qSelect1.value("name_acad"));
//        qInsert1.bindValue(":sheet_acad", qSelect1.value("sheet_acad"));
//        qInsert1.bindValue(":namebottom_acad", qSelect1.value("namebottom_acad"));
//        qInsert1.bindValue(":updatefromacad", qSelect1.value("updatefromacad"));
//        qInsert1.bindValue(":updatefromgui", qSelect1.value("updatefromgui"));
//        qInsert1.bindValue(":handle_hi", qSelect1.value("handle_hi"));
//        qInsert1.bindValue(":handle_lo", qSelect1.value("handle_lo"));
//        if (!qInsert1.exec()) {
//            gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert1);
//            return false;
//        }

//        qSelect2.bindValue(":id_dwg_layout", qSelect1.value("id"));
//        if (!qSelect2.exec()) {
//            gLogger->ShowSqlError(QObject::tr("Copy all refs"), qSelect2);
//            return false;
//        }
//        while (qSelect2.next()) {
//            qInsert2.bindValue(":name", qSelect2.value("name"));
//            qInsert2.bindValue(":handle_hi", qSelect2.value("handle_hi"));
//            qInsert2.bindValue(":handle_lo", qSelect2.value("handle_lo"));
//            if (!qInsert2.exec()) {
//                gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert2);
//                return false;
//            }
//            qInsert3.bindValue(":id_dwg_lb", qSelect2.value("id"));
//            if (!qInsert3.exec()) {
//                gLogger->ShowSqlError(QObject::tr("Copy all refs"), qInsert3);
//                return false;
//            }
//        }
//    }

//    return true;

    bool res = false;
    QSqlQuery qCopyAllRefs(db);

    qCopyAllRefs.prepare((db.driverName() == "QPSQL")
                         ?"perform pp_CopyAllRefs(:id_old, :id_new);"
                         :"begin pp.CopyAllRefs(:id_old, :id_new); end;");
    if (qCopyAllRefs.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Copy all refs") + " - prepare CopyAllRefs", qCopyAllRefs);
    } else {
        qCopyAllRefs.bindValue(":id_old", aIdOld);
        qCopyAllRefs.bindValue(":id_new", aIdNew);

        if (!qCopyAllRefs.exec()) {
            gLogger->ShowSqlError(QObject::tr("Copy all refs") + " - execute CopyAllRefs", qCopyAllRefs);
        } else {
            res = true;
        }
    }

    return res;
}

// -----------------------------------------------------------------------------------------------------------------------------------
DwgForSaveData::DwgForSaveData(int aId) :
    DwgData(aId),
    mInMain(false), mInXref(false)
{
}

DwgForSaveData::DwgForSaveData(int aId, const QString &aFilename, int aVersion, const QDateTime & aFTime,
                       qlonglong aDataLength, const QString &aGroupName, bool aInMain, bool aInXref, const QString &aSha256) :
    DwgData(0),
    /*mId(aId), */mFilename(aFilename), /*mVersion(aVersion), mFTime(aFTime), mDataLength(aDataLength),*/
    mGroupName(aGroupName), mInMain(aInMain), mInXref(aInXref), mSha256(aSha256)
{
    mId = aId;

    mVersion = aVersion;
    mFTime = aFTime;
    mDataLength = aDataLength;
}
