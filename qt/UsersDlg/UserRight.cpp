#include "UserRight.h"
#include "../VProject/common.h"

#include "../VProject/GlobalSettings.h"

UserRightData::UserRightData(UserRightAction aAction, const QString &aTable, const QString &aColumn) :
    mAction(aAction)
{
    mTable = aTable.toLower();
    mColumn = aColumn.toLower();
}

bool UserRightData::operator==(const UserRightData & other) const {
    return mAction == other.mAction
            && mTable == other.mTable
            && mColumn == other.mColumn;
}

bool UserRightData::operator<(const UserRightData & other) const {
     return (mAction < other.mAction)
             || (mAction == other.mAction && mTable < other.mTable)
             || (mAction == other.mAction && mTable == other.mTable && mColumn < other.mColumn);
}

//---------------------------------------------------------------------------------------------------------------------
UserRight::UserRight()
{

}

bool UserRight::CheckRightInternalInDb(UserRightData::UserRightAction aAction, const QString &aTableName, const QString &aColumnName)
{
    bool res = false;

    QSqlQuery query(db);
    if (aColumnName.isEmpty()) {
        if (db.driverName() == "QPSQL") {
            query.prepare("select count(*) from information_schema.role_table_grants"
                          " where table_schema = :owner and upper(table_name) = upper(:table_name) and privilege_type = :right_name");
        } else {
            query.prepare("select count(*) from all_tab_privs_recd"
                          " where owner = upper(:owner) and table_name = upper(:table_name) and privilege = :right_name");
        }
    } else {
        if (aColumnName != "--ANY--") {
            if (db.driverName() == "QPSQL") {
                query.prepare("select count(*) from information_schema.role_column_grants"
                              " where table_schema = :owner and upper(table_name) = upper(:table_name) and upper(column_name) = upper(:column_name) and privilege_type = :right_name");
            } else {
                query.prepare("select count(*) from all_col_privs_recd"
                              " where owner = upper(:owner) and table_name = upper(:table_name) and column_name = upper(:column_name) and privilege = :right_name");
            }
        } else {
            if (db.driverName() == "QPSQL") {
                query.prepare("select count(*) from information_schema.role_column_grants"
                              " where table_schema = :owner and upper(table_name) = upper(:table_name) and privilege_type = :right_name");
            } else {
                query.prepare("select count(*) from all_col_privs_recd"
                              " where owner = upper(:owner) and table_name = upper(:table_name) and privilege = :right_name");
            }
        }
    }

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("UserRight::CheckRight", query);
    } else {
        query.bindValue(":owner", gSettings->CurrentSchema);
        query.bindValue(":table_name", aTableName);
        switch (aAction) {
        case UserRightData::URAInsert:
            query.bindValue(":right_name", "INSERT");
            break;
        case UserRightData::URASelect:
            query.bindValue(":right_name", "SELECT");
            break;
        case UserRightData::URAUpdate:
            query.bindValue(":right_name", "UPDATE");
            break;
        case UserRightData::URADelete:
            query.bindValue(":right_name", "DELETE");
            break;
        }
        if (!aColumnName.isEmpty() && aColumnName != "--ANY--") {
            query.bindValue(":column_name", aColumnName);
        }

        if (!query.exec()) {
            gLogger->ShowSqlError("UserRight::CheckRight", query);
        } else {
            if (query.next()) {
                res = query.value(0).toInt() > 0;
            }
        }
    }
    return res;
}

bool UserRight::CheckRightInternal(UserRightData::UserRightAction aAction, const QString &aTableName, const QString &aColumnName)
{
    if ((aAction == UserRightData::URAUpdate || aAction == UserRightData::URAInsert)
            && !aColumnName.isEmpty()) {
        // anyway, check for whole table
        UserRightData lUserRightData(aAction, aTableName, "");

        QMap<UserRightData, bool>::const_iterator itr = mRights.find(lUserRightData);
        if (itr != mRights.end() && itr.key() == lUserRightData) {
            // return only if true; if false - need check for column
            if (itr.value()) return true;
        } else {
            // not found
            bool res = CheckRightInternalInDb(aAction, aTableName, ""); // get it from db
            mRights[lUserRightData] = res; // store in memory cache

            // return only if true; if false - need check for column
            if (res) return true; // can update field if can update whole table
        }
    }

    UserRightData lUserRightData(aAction, aTableName, aColumnName);

    QMap<UserRightData, bool>::const_iterator itr = mRights.find(lUserRightData);
    if (itr != mRights.end() && itr.key() == lUserRightData) {
        return itr.value();
    } else {
        // not found
        bool res = CheckRightInternalInDb(aAction, aTableName, aColumnName); // get it from db
        mRights[lUserRightData] = res; // store in memory cache
        return res;
    }
}

bool UserRight::CanInsert(const QString &aTableName, const QString &aColumnName) {
    return CheckRightInternal(UserRightData::URAInsert, aTableName, aColumnName);
}

bool UserRight::CanSelect(const QString &aTableName) {
    return CheckRightInternal(UserRightData::URASelect, aTableName, "");
}

bool UserRight::CanUpdate(const QString &aTableName, const QString &aColumnName) {
    return CheckRightInternal(UserRightData::URAUpdate, aTableName, aColumnName);
}

bool UserRight::CanDelete(const QString &aTableName) {
    return CheckRightInternal(UserRightData::URADelete, aTableName, "");
}

bool UserRight::CanAnyColumn(UserRightData::UserRightAction aAction, const QString &aTableName) {
    if (CheckRightInternal(aAction, aTableName, "")) return true;

    UserRightData lUserRightData(aAction, aTableName, "--ANY--");

    QMap<UserRightData, bool>::const_iterator itr = mRights.find(lUserRightData);
    if (itr != mRights.end() && itr.key() == lUserRightData) {
        return itr.value();
    } else {
        // not found
        bool res = CheckRightInternalInDb(aAction, aTableName, "--ANY--"); // get it from db
        mRights[lUserRightData] = res; // store in memory cache
        return res;
    }
}

bool UserRight::CanUpdateAnyColumn(const QString &aTableName) {
    return CanAnyColumn(UserRightData::URAUpdate, aTableName);
}

bool UserRight::CanInsertAnyColumn(const QString &aTableName) {
    return CanAnyColumn(UserRightData::URAInsert, aTableName);
}

bool UserRight::FindTableName(const QString &aTableStart, QString &aTableName) {
    bool res = false;

    QMap<QString, QString>::const_iterator itr = mTablesFound.find(aTableStart);
    if (itr != mTablesFound.end() && itr.key() == aTableStart) {
        aTableName = itr.value();
        res = true;
    } else {
        QSqlQuery query(db);
        if (db.driverName() == "QPSQL") {
            query.prepare("select table_name from information_schema.role_table_grants"
                          " where table_schema = :owner and upper(table_name) like upper(:table_start) || '%' and privilege_type = 'SELECT'"
                          " order by table_name");
        } else {
            query.prepare("select table_name from all_tab_privs_recd"
                          " where owner = upper(:owner) and table_name like upper(:table_start) || '%' and privilege = 'SELECT'"
                          " order by table_name");
        }

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError("UserRight::FindTableName", query);
        } else {
            query.bindValue(":owner", gSettings->CurrentSchema);
            query.bindValue(":table_start", aTableStart);

            if (!query.exec()) {
                gLogger->ShowSqlError("UserRight::FindTableName", query);
            } else {
                if (query.next()) {
                    aTableName = qString("table_name");
                    mTablesFound[aTableStart] = aTableName; // store in memory cache
                    res = true;
                }
            }
        }
    }
    return res;
}

bool UserRight::HasColumn(const QString &aTableName, const QString &aColumnName) {
    // using the same QMap and combination URASelect with Column Name
    UserRightData lUserRightData(UserRightData::URASelect, aTableName, aColumnName);

    QMap<UserRightData, bool>::const_iterator itr = mRights.find(lUserRightData);
    if (itr != mRights.end() && itr.key() == lUserRightData) {
        return itr.value();
    } else {
        // not found in cache
        bool res = false;

        QSqlQuery query(db);
        if (db.driverName() == "QPSQL") {
            query.prepare("select count(*) from information_schema.role_column_grants"
                          " where table_schema = :owner and upper(table_name) = upper(:table_name) and upper(column_name) = upper(:column_name)");
        } else {
            query.prepare("select count(*) from all_tab_cols"
                          " where owner = upper(:owner) and table_name = upper(:table_name) and column_name = upper(:column_name)");
        }

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError("UserRight::HasColumn", query);
        } else {
            query.bindValue(":owner", gSettings->CurrentSchema);
            query.bindValue(":table_name", aTableName);
            query.bindValue(":column_name", aColumnName);

            if (!query.exec()) {
                gLogger->ShowSqlError("UserRight::HasColumn", query);
            } else {
                if (query.next()) {
                    res = query.value(0).toInt() > 0;
                }
            }
        }
        mRights[lUserRightData] = res; // store in memory cache
        return res;
    }
}

bool UserRight::HasPrivelege(const QString &aPrivelegeName) {
    UserRightData lUserRightData(UserRightData::URAPriv, aPrivelegeName, "");

    QMap<UserRightData, bool>::const_iterator itr = mRights.find(lUserRightData);
    if (itr != mRights.end() && itr.key() == lUserRightData) {
        return itr.value();
    } else {
        // not found in cache
        bool res = false;

        QSqlQuery query(db);
        if (db.driverName() == "QPSQL") {
            if (CanSelect("v_my_role_granted")) {
                //query.prepare("select rolsuper from pg_roles where rolname = session_user");
                query.prepare("select rolsuper from pg_roles a, v_my_role_granted b"
                              " where lower(a.rolname) = lower(b.role_name) and b.grantee = session_user"
                              " and rolsuper = true");
                if (query.lastError().isValid()) {
                    gLogger->ShowSqlError("UserRight::HasPrivelege", query);
                } else {
                    if (!query.exec()) {
                        gLogger->ShowSqlError("UserRight::HasPrivelege", query);
                    } else {
                        if (query.next()) {
                            res = query.value(0).toBool();
                        }
                    }
                }
            }
        } else {
            query.prepare("select privilege from session_privs"
                          " where privilege = upper(:privilege)");
            if (query.lastError().isValid()) {
                gLogger->ShowSqlError("UserRight::HasPrivelege", query);
            } else {
                query.bindValue(":privilege", aPrivelegeName);

                if (!query.exec()) {
                    gLogger->ShowSqlError("UserRight::HasPrivelege", query);
                } else {
                    if (query.next()) {
                        res = true;
                    }
                }
            }
        }
        mRights[lUserRightData] = res; // store in memory cache
        return res;
    }
}

bool UserRight::CanCreateUser() {
    return HasPrivelege("create user");
}

bool UserRight::CanManageUser() {
    return HasPrivelege("alter user");
}

bool UserRight::CanDropUser() {
    return HasPrivelege("drop user");
}

bool UserRight::CanGrantAnyRole() {
    return HasPrivelege("grant any role");
}

void UserRight::UpdateRightsOnServer() {
    QSqlQuery qUpdate(db);
    if (db.driverName() == "QPSQL") {
        qUpdate.prepare("select pp_UpdateRightsTable()");
    } else {
        qUpdate.prepare("begin pp.UpdateRightsTable(); end;");
    }
    if (qUpdate.lastError().isValid()) {
        gLogger->ShowSqlError("UserRight::UpdateRightsOnServer", qUpdate);
    } else {
        if (!qUpdate.exec()) {
            gLogger->ShowSqlError("UserRight::UpdateRightsOnServer", qUpdate);
        }
    }
}
