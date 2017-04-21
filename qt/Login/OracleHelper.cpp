#include "OracleHelper.h"

#include "../Logger/logger.h"

extern QSqlDatabase db;

OracleHelper::OracleHelper() :
    mSqlInsertXref(NULL), mSqlFindXref(NULL), mSqlFindDwg(NULL), mSqlTryMoveToCommon(NULL),
    mIsNotEditing(NULL)
{

}

OracleHelper::~OracleHelper() {
    Clean(); // na hua? let it be
}

void OracleHelper::Clean() {
    if (mSqlInsertXref) {
        delete mSqlInsertXref;
        mSqlInsertXref = NULL;
    }

    if (mSqlFindXref) {
        delete mSqlFindXref;
        mSqlFindXref = NULL;
    }

    if (mSqlFindDwg) {
        delete mSqlFindDwg;
        mSqlFindDwg = NULL;
    }

    if (mSqlTryMoveToCommon) {
        delete mSqlTryMoveToCommon;
        mSqlTryMoveToCommon = NULL;
    }

    if (mIsNotEditing) {
        delete mIsNotEditing;
        mIsNotEditing = NULL;
    }
}

bool OracleHelper::GetSeqVal(QString aSeqName, const QString &aValType, int &aVal) const {
    bool res = false;
    QSqlQuery query((db.driverName() == "QPSQL")
                    ?"select " + aValType + "('" + aSeqName + "')"
                    :"select " + aSeqName + "." + aValType + " from dual", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Sequence " + aSeqName, query);
    } else {
        if (query.next()) {
            aVal = query.value(0).toInt();
            res = true;
        } else {
            gLogger->ShowSqlError("Sequence " + aSeqName, query);
        }
    }

    return res;

}

bool OracleHelper::GetSeqVal(QString aSeqName, const QString &aValType, qulonglong &aVal) const {
    bool res = false;
    QSqlQuery query((db.driverName() == "QPSQL")
                    ?"select " + aValType + "('" + aSeqName + "')"
                    :"select " + aSeqName + "." + aValType + " from dual", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Sequence " + aSeqName, query);
    } else {
        if (query.next()) {
            aVal = query.value(0).toULongLong();
            res = true;
        } else {
            gLogger->ShowSqlError("Sequence " + aSeqName, query);
        }
    }
    return res;
}

bool OracleHelper::GetSeqNextVal(const QString &aSeqName, int &aVal) const {
    return GetSeqVal(aSeqName, "nextval", aVal);
}

bool OracleHelper::GetSeqCurVal(const QString &aSeqName, int &aVal) const {
    return GetSeqVal(aSeqName, "currval", aVal);
}

bool OracleHelper::GetSeqNextVal(const QString &aSeqName, qulonglong &aVal) const {
    return GetSeqVal(aSeqName, "nextval", aVal);
}

bool OracleHelper::GetSeqCurVal(const QString &aSeqName, qulonglong &aVal) const {
    return GetSeqVal(aSeqName, "currval", aVal);
}

bool OracleHelper::GetSysTimeStamp(QDateTime &aDateTime) const {
    bool res = false;

    aDateTime = QDateTime::currentDateTime();

    QSqlQuery query("select current_timestamp from dual", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("GetSysTimeStamp", query);
    } else {
        if (query.next()) {
            aDateTime = query.value(0).toDateTime();
            res = true;
        } else {
            gLogger->ShowSqlError("GetSysTimeStamp", query);
        }
    }

    return res;
}

bool OracleHelper::InsertXref(quint64 &aId, quint64 aIdDwg, const QString &aFileName, quint64 aIdXref, int aVersion, const QString &aGrpName) {
    if (!mSqlInsertXref) {
        mSqlInsertXref = new QSqlQuery(db);
        mSqlInsertXref->prepare("insert into xref (id, id_dwg, file_name, id_xref, version, grpname)"
                        " values (:id, :id_dwg, :file_name, :id_xref, :version, :grpname)");
        if (mSqlInsertXref->lastError().isValid()) {
            gLogger->ShowSqlError("Insert xref - prepare", *mSqlInsertXref);
            delete mSqlInsertXref;
            mSqlInsertXref = NULL;
            return false;
        }
    }

    if (!aId && !GetSeqNextVal("xref_id_seq", aId)) return false;

    mSqlInsertXref->bindValue(":id", aId);
    mSqlInsertXref->bindValue(":id_dwg", aIdDwg);
    mSqlInsertXref->bindValue(":file_name", aFileName);
    mSqlInsertXref->bindValue(":id_xref", aIdXref);
    mSqlInsertXref->bindValue(":version", aVersion);
    mSqlInsertXref->bindValue(":grpname", aGrpName);

    if (!mSqlInsertXref->exec()) {
        gLogger->ShowSqlError("Insert xref - execute", *mSqlInsertXref);
        return false;
    } else {
        return true;
    }
}

bool OracleHelper::InsertXref(quint64 aIdDwg, const QString &aFileName, quint64 aIdXref, int aVersion, const QString &aGrpName) {
    quint64 lId = 0;

    return InsertXref(lId, aIdDwg, aFileName, aIdXref, aVersion, aGrpName);
}

bool OracleHelper::FindXref(quint64 aIdDwgMain, const QString &aFileName, const QString &aSha256, quint64 &aIdDwgXref, int &aVersion, bool &aShaIsEqual) {
    if (!mSqlFindXref) {
        mSqlFindXref = new QSqlQuery(db);
        //mSqlFindXref->prepare("select a.id_xref id_xref, a.version version, decode(gnusha256(b.data), :sha256, 1, 0) IsEqual from xref a, v_dwg b"
        mSqlFindXref->prepare("select a.id_xref id_xref, a.version version,"
                              " case when b.sha256 = :sha256 then 1 else 0 end IsEqual"
                              " from xref a, v_dwg b"
                              " where a.id_dwg = :id_dwg"
                              " and upper(a.file_name) = upper(:file_name)"
                              " and a.id_xref = b.id and a.deleted = 0");
        if (mSqlFindXref->lastError().isValid()) {
            gLogger->ShowSqlError("Find xref - prepare", *mSqlFindXref);
            delete mSqlFindXref;
            mSqlFindXref = NULL;
            return false;
        }
    }

    mSqlFindXref->bindValue(":id_dwg", aIdDwgMain);
    mSqlFindXref->bindValue(":file_name", aFileName);
    mSqlFindXref->bindValue(":sha256", aSha256);

    if (!mSqlFindXref->exec()) {
        gLogger->ShowSqlError("Find xref - execute", *mSqlFindXref);
        return false;
    } else {
        if (mSqlFindXref->next()) {
            aIdDwgXref = mSqlFindXref->value("id_xref").toULongLong();
            aVersion = mSqlFindXref->value("version").toInt();
            if (mSqlFindXref->value("IsEqual").toInt() == 1) {
                // sha's equals
                aShaIsEqual = true;
            } else {
                // sha is not equal, use next version number
                aVersion++;
                aShaIsEqual = false;
            }
        } else {
            aIdDwgXref = 0; // not found
            aVersion = 1; // versions start from 1
        }
        return true;
    }
}

bool OracleHelper::FindDwgBySha256(const QString &aSha256, quint64 &aIdDwg) {
    if (!mSqlFindDwg) {
        mSqlFindDwg = new QSqlQuery(db);
        mSqlFindDwg->prepare("select id from v_dwg"
                             " where id_part is null"
                             " and id_plot is null"
                             " and sha256 = :sha256"
                             " and pp.IsDwgRecordNOTEditingNow(id) = 1");
        if (mSqlFindDwg->lastError().isValid()) {
            gLogger->ShowSqlError("Find xref - prepare", *mSqlFindDwg);
            delete mSqlFindDwg;
            mSqlFindDwg = NULL;
            return false;
        }
    }

    mSqlFindDwg->bindValue(":sha256", aSha256);

    if (!mSqlFindDwg->exec()) {
        gLogger->ShowSqlError("Find xref - execute", *mSqlFindDwg);
        return false;
    } else {
        if (mSqlFindDwg->next()) {
            aIdDwg = mSqlFindDwg->value("id").toULongLong();
        } else {
            aIdDwg = 0; // not found
        }
        return true;
    }
}

bool OracleHelper::InsertDwgFile(quint64 aIdDwg, bool aTrueForOut, const QString &aFileName, qint64 aFileSize, const QDateTime & aFileModified, const QString & aSha256) {
    QSqlQuery lSqlInsertDwgFile (db);
    lSqlInsertDwgFile.prepare("insert into dwg_file (id_dwg, inout, file_name, file_size, file_date, sha256)"
                              " values (:id_dwg, :inout, :file_name, :file_size, :file_date, :sha256)");
    if (lSqlInsertDwgFile.lastError().isValid()) {
        gLogger->ShowSqlError("Insert dwg_file - prepare", lSqlInsertDwgFile);
        return false;
    }

    QString lFilenameForLog = aFileName;
    lFilenameForLog.replace('/', '\\');

    lSqlInsertDwgFile.bindValue(":id_dwg", aIdDwg);
    lSqlInsertDwgFile.bindValue(":inout", aTrueForOut?1:0);
    lSqlInsertDwgFile.bindValue(":file_name", lFilenameForLog);
    lSqlInsertDwgFile.bindValue(":file_size", aFileSize);
    lSqlInsertDwgFile.bindValue(":file_date", aFileModified);
    lSqlInsertDwgFile.bindValue(":sha256", aSha256);

    if (!lSqlInsertDwgFile.exec()) {
        gLogger->ShowSqlError("Insert dwg_file - execute", lSqlInsertDwgFile);
        return false;
    } else {
        return true;
    }
}

bool OracleHelper::IsDwgRecordNOTEditingNow(quint64 aIdDwg) {
    if (mDwgNOTEditing.contains(aIdDwg)) return true;

    bool res = false;

    if (!mIsNotEditing) {
        mIsNotEditing = new QSqlQuery(db);
        mIsNotEditing->prepare("select pp.IsDwgRecordNOTEditingNow(:id_dwg) from dual");
        if (mIsNotEditing->lastError().isValid()) {
            gLogger->ShowSqlError("IsDwgRecordNOTEditingNow - prepare", *mIsNotEditing);
            delete mIsNotEditing;
            mIsNotEditing = NULL;
        }
    }

    if (mIsNotEditing) {
        mIsNotEditing->bindValue(":id_dwg", aIdDwg);

        if (!mIsNotEditing->exec()) {
            gLogger->ShowSqlError("IsDwgRecordNOTEditingNow - exec", *mIsNotEditing);
        } else {
            if (mIsNotEditing->next()) {
                res = mIsNotEditing->value(0).toInt() == 1;
            }
        }
    }

    if (res) {
        mDwgNOTEditing.append(aIdDwg);
    }
    return res;
}

bool OracleHelper::CollectAlreadyLoaded(const QString &aHash, QList<tPairIntIntString> &aExistingIds) {
    QSqlQuery query(db);

    query.prepare("select /*+ LEADING(A) USE_NL(C B A) */ b.id_plot, b.version,"
                  " case when c.sheet_number is null then c.code else c.code || '-' || c.sheet_number end code"
                  " from dwg_file a, v_dwg b, v_plot_simple c where a.id_dwg = b.id and a.inout = 0 and b.id_plot = c.id and a.sha256 = :sha256 and c.deleted = 0");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Loading document"), query);
        return false;
    } else {
        query.bindValue(":sha256", aHash);
        if (!query.exec()) {
            gLogger->ShowSqlError(QObject::tr("Loading document"), query);
            return false;
        } else {
            while (query.next()) {
                aExistingIds.append(qMakePair(qMakePair(query.value(0).toInt(), query.value(1).toInt()), query.value(2).toString()));
            }
        }
    }
    return true;
}

bool OracleHelper::CollectByBlockName(int aIdProject, const QString &aBlockName, QList<tPairIntString> &aExistingIds) {
    QSqlQuery query(db);

    if (aIdProject) {
        query.prepare("select id, case when sheet_number is null then code else code || '-' || sheet_number end code"
                      " from v_plot_simple where id_project = :id_project and lower(block_name) = lower(:block_name)");
    } else {
        query.prepare("select id, case when sheet_number is null then code else code || '-' || sheet_number end code"
                      " from v_plot_simple where lower(block_name) = lower(:block_name)");
    }

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Loading document"), query);
        return false;
    } else {
        if (aIdProject) {
            query.bindValue(":id_project", aIdProject);
        }
        query.bindValue(":block_name", aBlockName);
        if (!query.exec()) {
            gLogger->ShowSqlError(QObject::tr("Loading document"), query);
            return false;
        } else {
            while (query.next()) {
                aExistingIds.append(qMakePair(query.value(0).toInt(), query.value(1).toString()));
            }
        }
    }
    return true;
}
