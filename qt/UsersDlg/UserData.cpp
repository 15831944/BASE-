#include "UserData.h"
#include "UsersDlg.h"

#include "UserRight.h"

#include "../VProject/common.h"
#include "../VProject/MainWindow.h"

#include <QVariant>

UserData::UserData(int aId, const QString &aName, const QString &aLogin, int aHasLogin, const QString &aPlotName, const QString &aLongName,
                   int aIdCustomer, int aIdDepartment, const QString &aJobTitle, const QDate &aHireDate, uint aTrialPeriod, int aDisabled,
                   const QString &aEMail, const QString &aPhone1, const QString &aPhone2, const QString &aPhone3,
                   const QString &aAddr, const QString &aRoom, const QString &aComments,
                   const QDate &aBirthDate, uint aEPH,
                   int aIsBoss, int aIsGIP) :
    InitParRO(Id), InitPar(Name), InitPar(Login), InitParRO(HasLogin), InitPar(PlotName), InitPar(LongName),
    InitPar(IdCustomer), InitPar(IdDepartment), InitPar(JobTitle),
    InitPar(HireDate), InitPar(TrialPeriod), InitPar(Disabled),
    InitPar(EMail), InitPar(Phone1), InitPar(Phone2), InitPar(Phone3), InitPar(Addr), InitPar(Room), InitPar(Comments),
    InitPar(BirthDate), InitPar(EPH),
    InitParRO(IsBoss), InitParRO(IsGIP)
{
    mIsNew = !mId;
}

bool UserData::SaveData(bool &aNotChanged) {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;
    aNotChanged = false;

    if (!mIsNew) {
        // existing
        CheckParChange(Name, name)
        CheckParChange(Login, login)
        if (gUsers->HasPlotname()) {
            CheckParChange(PlotName, plotname)
        }
        CheckParChange(LongName, long_name)

        if (gUsers->HasCompany()) {
            CheckParChangeWithNull(IdCustomer, id_customer, 0)
        }
        if (gUsers->HasDepartment()) {
            CheckParChangeWithNull(IdDepartment, id_department, 0)
        }
        CheckParChange(JobTitle, jobtitle)

        if (gUsers->HasHireDate()) {
            CheckParChange(HireDate, hire_date)
        }
        if (gUsers->HasTrialPeriod()) {
            CheckParChangeWithNull(TrialPeriod, trial_period, 0)
        }
        CheckParChangeWithNull(Disabled, disabled, 0)

        CheckParChange(EMail, email)
        CheckParChange(Phone1, phone1)
        CheckParChange(Phone2, phone2)
        CheckParChange(Phone3, phone3)
        CheckParChange(Addr, addr)
        CheckParChange(Room, room)
        CheckParChange(Comments, remarks)

        CheckParChange(BirthDate, birthday)
        if (gUsers->HasEPH()) {
            CheckParChangeWithNull(EPH, eph, 0)
        }

        if (!lUpdStr.isEmpty()) {
            lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

            QSqlQuery qUpdate(db);

            qUpdate.prepare("update " + gUsers->TableName() + " set " + lUpdStr + " where id = ?");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("User data"), qUpdate);
            } else {
                for (int i = 0; i < lUpdValues.length(); i++) {
                    qUpdate.addBindValue(lUpdValues.at(i));
                }
                qUpdate.addBindValue(mId);
                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(QObject::tr("User data"), qUpdate);
                } else {
                    res = true;
                }
            }
        } else {
            res = true;
            aNotChanged = true;
        }
    } else {
        // new
        QSqlQuery qInsert(db);

        if (!mId) {
            // so we can refresh later
            if (!gOracle->GetSeqNextVal("users_id_seq", mId)) return false;
        }

        QString lInsertStr = "insert into " + gUsers->TableName() + " (id, name, login";

        if (gUsers->HasPlotname()) {
            lInsertStr += ", plotname";
        }
        lInsertStr += ", long_name";
        if (gUsers->HasCompany()) {
            lInsertStr += ", id_customer";
        }
        if (gUsers->HasDepartment()) {
            lInsertStr += ", id_department";
        }
        lInsertStr += ", jobtitle";
        if (gUsers->HasHireDate()) {
            lInsertStr += ", hire_date";
        }
        if (gUsers->HasTrialPeriod()) {
            lInsertStr += ", trial_period";
        }
        lInsertStr += ", email, phone1, phone2, phone3, addr, room, remarks, birthday";
        if (gUsers->HasEPH()) {
            lInsertStr += ", eph";
        }
        lInsertStr += ") values ";

        QString lVars = lInsertStr;
        lVars.remove(QRegExp("^.*\\(")).remove(QRegExp("\\).*$"));
        QRegExp lRegExp("(^|[^a-z_])[a-z]");
        int lPos = 0;
        while ((lPos = lRegExp.indexIn(lVars, lPos)) != -1) {
            if (!lPos) lPos--;
            lVars.insert(lPos + 1, ':');
            lPos += 2;
        }
        lInsertStr += "(" + lVars + ")";

        qInsert.prepare(lInsertStr);
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("User data"), qInsert);
        } else {
            qInsert.bindValue(":id", mId);
            qInsert.bindValue(":name", mName);
            qInsert.bindValue(":login", mLogin);
            if (gUsers->HasPlotname()) {
                qInsert.bindValue(":plotname", mPlotName);
            }
            qInsert.bindValue(":long_name", mLongName);

            if (gUsers->HasCompany()) {
                if (mIdCustomer) {
                    qInsert.bindValue(":id_customer", mIdCustomer);
                } else {
                    qInsert.bindValue(":id_customer", QVariant());
                }
            }
            if (gUsers->HasDepartment()) {
                if (mIdDepartment) {
                    qInsert.bindValue(":id_department", mIdDepartment);
                } else {
                    qInsert.bindValue(":id_department", QVariant());
                }
            }
            qInsert.bindValue(":jobtitle", mJobTitle);
            if (gUsers->HasHireDate()) {
                if (!mHireDate.isNull()) {
                    qInsert.bindValue(":hire_date", mHireDate);
                } else {
                    qInsert.bindValue(":hire_date", QVariant());
                }
            }
            if (gUsers->HasTrialPeriod()) {
                if (mTrialPeriod) {
                    qInsert.bindValue(":trial_period", mTrialPeriod);
                } else {
                    qInsert.bindValue(":trial_period", QVariant());
                }
            }

            qInsert.bindValue(":email", mEMail);
            qInsert.bindValue(":phone1", mPhone1);
            qInsert.bindValue(":phone2", mPhone2);
            qInsert.bindValue(":phone3", mPhone3);
            qInsert.bindValue(":addr", mAddr);
            qInsert.bindValue(":room", mRoom);
            qInsert.bindValue(":remarks", mComments);
            if (!mBirthDate.isNull()) {
                qInsert.bindValue(":birthday", mBirthDate);
            } else {
                qInsert.bindValue(":birthday", QVariant());
            }
            if (gUsers->HasEPH()) {
                if (mEPH) {
                    qInsert.bindValue(":eph", mEPH);
                } else {
                    qInsert.bindValue(":eph", QVariant());
                }
            }

            if (!qInsert.exec()) {
                gLogger->ShowSqlError(QObject::tr("User data"), qInsert);
            } else {
                res = true;
            }
        }
    }
    return res;
}

void UserData::RollbackEdit() {
    RollbackPar(Name)
    RollbackPar(Login)
    RollbackPar(PlotName)
    RollbackPar(LongName)

    RollbackPar(IdCustomer)
    RollbackPar(IdDepartment)
    RollbackPar(JobTitle)

    RollbackPar(HireDate)
    RollbackPar(TrialPeriod)
    RollbackPar(Disabled)

    RollbackPar(EMail)
    RollbackPar(Phone1)
    RollbackPar(Phone2)
    RollbackPar(Phone3)
    RollbackPar(Addr)
    RollbackPar(Room)
    RollbackPar(Comments)

    RollbackPar(BirthDate)
    RollbackPar(EPH)
}

void UserData::CommitEdit() {
    CommitPar(Name)
    CommitPar(Login)
    CommitPar(PlotName)
    CommitPar(LongName)

    CommitPar(IdCustomer)
    CommitPar(IdDepartment)
    CommitPar(JobTitle)

    CommitPar(HireDate)
    CommitPar(TrialPeriod)
    CommitPar(Disabled)

    CommitPar(EMail)
    CommitPar(Phone1)
    CommitPar(Phone2)
    CommitPar(Phone3)
    CommitPar(Addr)
    CommitPar(Room)
    CommitPar(Comments)

    CommitPar(BirthDate)
    CommitPar(EPH)

    mIsNew = false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
UserDataList::UserDataList() :
    QObject()
{
    if (gUserRight->FindTableName("v_users", mTableName)) {
        mHasPlotName = gUserRight->HasColumn(mTableName, "plotname");
        mHasCompany = gUserRight->HasColumn(mTableName, "id_customer");
        mHasDepartment = gUserRight->HasColumn(mTableName, "id_department");
        mHasHireDate = gUserRight->HasColumn(mTableName, "hire_date");
        mHasTrialPeriod = gUserRight->HasColumn(mTableName, "trial_period");
        mHasEPH = gUserRight->HasColumn(mTableName, "eph");

        // ---
        if (db.driverName() == "QPSQL") {
            mSelectQuery = "select id, name, login, (select count(*) from pg_user where lower(usename) = lower(login)) as has_login,";
        } else {
            mSelectQuery = "select id, name, login, (select count(*) from all_users where lower(username) = lower(login)) has_login,";
        }
        if (mHasPlotName)
            mSelectQuery += " plotname,";
        mSelectQuery += " long_name,";
        if (mHasCompany)
            mSelectQuery += " id_customer,";
        if (mHasDepartment)
            mSelectQuery += " id_department,";

        mSelectQuery += " jobtitle, ";

        if (mHasHireDate)
            mSelectQuery += " hire_date,";
        if (mHasTrialPeriod)
            mSelectQuery += " trial_period,";
        mSelectQuery += " disabled, email, phone1, phone2, phone3, addr, room, remarks, birthday,";
        if (mHasEPH)
            mSelectQuery += "eph,";

        mSelectQuery += " (select count(*) from user_right where login = a.login and rightname = 'BOSS') IsBoss,"
                        " (select count(*) from user_right where login = a.login and rightname = 'GIP') IsGip"
                        " from " + mTableName + " a order by name";

        InitUserList();
    } else {
        gLogger->ShowError(QObject::tr("User data"), "No permissions for select!");
    }
}

void UserDataList::CheckListInternal() {
    if (mUserList.isEmpty()
            && !mTableName.isEmpty()) InitUserList();
}

void UserDataList::InitUserList() {
    emit UsersBeforeUpdate();

    qDeleteAll(mUserList);
    mUserList.clear();

    QSqlQuery query(mSelectQuery, db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("User data"), query);
    } else {
        while (query.next()) {
            mUserList.append(new UserData(qInt("id"), qString("name"),
                                          qString("login"), qInt("has_login"), qString("plotname"), qString("long_name"), qInt("id_customer"),
                                          qInt("id_department"), qString("jobtitle"), qDate("hire_date"), qInt("trial_period"), qInt("disabled"),
                                          qString("email"), qString("phone1"), qString("phone2"), qString("phone3"), qString("addr"), qString("room"),
                                          qString("remarks"), qDate("birthday"), qUInt("eph"),
                                          qInt("IsBoss"), qInt("IsGip")));
        }
    }
    emit UsersNeedUpdate();
}

QList<UserData *> &UserDataList::UsersRef() {
    CheckListInternal();
    return mUserList;
}

const QList<UserData *> &UserDataList::UsersConst() {
    CheckListInternal();
    return mUserList;
}

UserData * UserDataList::FindByName(const QString &aName) {
    CheckListInternal();

    foreach(UserData *lUserData, mUserList) {
        if (lUserData->NameConst().toLower() == aName.toLower())
            return lUserData;
    }

    return NULL;
}

UserData * UserDataList::FindByLogin(const QString &aLogin) {
    CheckListInternal();

    foreach(UserData *lUserData, mUserList) {
        if (lUserData->LoginConst().toLower() == aLogin.toLower())
            return lUserData;
    }

    return NULL;
}

const QString & UserDataList::GetName(const QString &aLogin) {
    if (aLogin.isEmpty()) return aLogin;

    CheckListInternal();

    UserData * lUserData;
    if (lUserData = FindByLogin(aLogin)) {
        return lUserData->NameConst();
    } else {
        return aLogin;
    }
}

const QString & UserDataList::GetLogin(const QString &aName) {
    if (aName.isEmpty()) return aName;

    CheckListInternal();

    UserData * lUserData;
    if (lUserData = FindByName(aName)) {
        return lUserData->LoginConst();
    } else {
        return aName;
    }
}

UserData *UserDataList::SelectUser(UserData *aSelectedUser, uint aListFlags,
                                   int (*aCheckForInclude)(const UserData *, void *), void *apData) const {
    UsersDlg w(UsersDlg::DTSelectOne, gMainWindow);
    w.SetCheckForInclude(aCheckForInclude, apData);
    if (aSelectedUser) w.SetSelectedUserId(aSelectedUser->Id());
    w.SetListFlags(aListFlags);
    if (w.exec() == QDialog::Accepted) {
        return w.GetSelectedUser();
    }
    return NULL;
}

bool UserDataList::SelectUsers(QList<UserData *> &aSelected, const QList<UserData *> *aExcludedUsers) {
    UsersDlg w(UsersDlg::DTSelectMany, gMainWindow);
    w.SetExcludedUsers(aExcludedUsers);
    if (w.exec() == QDialog::Accepted) {
        aSelected = w.GetSelectedUsers();
        return true;
    }
    return false;
}

bool UserDataList::SelectUsers(QList<UserData *> &aSelected, const QStringList *aExcludedLogins) {
    UsersDlg w(UsersDlg::DTSelectMany, gMainWindow);
    w.SetExcludedLogins(aExcludedLogins);
    if (w.exec() == QDialog::Accepted) {
        aSelected = w.GetSelectedUsers();
        return true;
    }
    return false;
}

const QString &UserDataList::TableName() const {
    return mTableName;
}

bool UserDataList::CreateUser(const QString &aLogin, const QString &aPassword) const {
    QSqlQuery qCreate;
    bool lExecRes;
    if (db.driverName() == "QPSQL") {
        lExecRes = qCreate.exec("create role " + aLogin + " nosuperuser login encrypted password '" + aPassword + "'");
    } else {
        lExecRes = qCreate.exec("create user " + aLogin + " identified by " + aPassword);
    }
    if (!lExecRes
            || qCreate.lastError().isValid()) {
        gLogger->ShowSqlError(tr("Creating user"), qCreate);
        return false;
    } else {
        return true;
    }
}

bool UserDataList::DropUser(const QString &aLogin, bool aShowError) const {
    QSqlQuery qDrop;
    if (!qDrop.exec("drop user " + aLogin)
            || qDrop.lastError().isValid()) {
        if (aShowError) {
            gLogger->ShowSqlError(tr("Dropping user"), qDrop);
        } else {
            gLogger->LogError("Dropping user - log only: " + qDrop.lastError().text());
        }
        return false;
    } else {
        return true;
    }
}

bool UserDataList::LockUser(const QString &aLogin, bool aShowError) const {
    QSqlQuery qLock;
    bool lExecRes;
    if (db.driverName() == "QPSQL") {
        lExecRes = qLock.exec("alter role " + aLogin + " valid until 'May 4 12:00:00 2015 +1'");
    } else {
        lExecRes = qLock.exec("alter user " + aLogin + " account lock");
    }
    if (!lExecRes
            || qLock.lastError().isValid()) {
        if (aShowError) {
            gLogger->ShowSqlError(tr("Locking user"), qLock);
        } else {
            gLogger->LogError("Locking user - log only: " + qLock.lastError().text());
        }
        return false;
    } else {
        return true;
    }
}

bool UserDataList::UnlockUser(const QString &aLogin) const {
    QSqlQuery qUnlock;
    bool lExecRes;
    if (db.driverName() == "QPSQL") {
        lExecRes = qUnlock.exec("alter role " + aLogin + " valid until 'infinity'");
    } else {
        lExecRes = qUnlock.exec("alter user " + aLogin + " account unlock");
    }
    if (!lExecRes
            || qUnlock.lastError().isValid()) {
        gLogger->ShowSqlError(tr("Unlocking user"), qUnlock);
        return false;
    } else {
        return true;
    }
}

bool UserDataList::HasPlotname() const {
    return mHasPlotName;
}

bool UserDataList::HasCompany() const {
    return mHasCompany;
}

bool UserDataList::HasDepartment() const {
    return mHasDepartment;
}

bool UserDataList::HasHireDate() const {
    return mHasHireDate;
}

bool UserDataList::HasTrialPeriod() const {
    return mHasTrialPeriod;
}

bool UserDataList::HasEPH() const {
    return mHasEPH;
}
