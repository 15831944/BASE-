#ifndef USERDATA_H
#define USERDATA_H

#include <QString>
#include <QDate>
#include <QMap>
#include <QObject>

#include "../VProject/CommonData.h"

#if defined(VPROJECT_MAIN_IMPORT)
    #define USERSDLG_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(USERSDLG_LIBRARY)
        #define USERSDLG_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define USERSDLG_LIBRARY_EXP_IMP
    #endif
#endif

class USERSDLG_LIBRARY_EXP_IMP UserData
{
    DeclareParRO(int, Id)
    DefParStr(Name)
    DefParStr(Login)
    DeclareParRO(int, HasLogin)
    DefParStr(PlotName)
    DefParStr(LongName)

    DeclarePar(int, IdCustomer)
    DeclarePar(int, IdDepartment)
    DefParStr(JobTitle)

    DefParCompl(QDate, HireDate)
    DeclarePar(uint, TrialPeriod) // monthes I suppose
    DeclarePar(int, Disabled)

    DefParStr(EMail)
    DefParStr(Phone1)
    DefParStr(Phone2)
    DefParStr(Phone3)
    DefParStr(Addr)
    DefParStr(Room)
    DefParStr(Comments)

    DefParCompl(QDate, BirthDate)
    DeclarePar(uint, EPH) // expense per hour

    DeclareParRO(int, IsBoss)
    DeclareParRO(int, IsGIP)

private:
    bool mIsNew;
public:
    explicit UserData(int aId, const QString &aName, const QString &aLogin, int aHasLogin, const QString &aPlotName, const QString &aLongName,
                      int aIdCustomer, int aIdDepartment, const QString &aJobTitle, const QDate &aHireDate, uint aTrialPeriod, int aDisabled,
                      const QString &aEMail, const QString &aPhone1, const QString &aPhone2, const QString &aPhone3,
                      const QString &aAddr, const QString &aRoom, const QString &aComments,
                      const QDate &aBirthDate, uint aEPH,
                      int aIsBoss, int aIsGIP);

    bool SaveData(bool &aNotChanged);
    void RollbackEdit();
    void CommitEdit();
};

Q_DECLARE_METATYPE(UserData *)

class USERSDLG_LIBRARY_EXP_IMP UserDataList : public QObject
{
    Q_OBJECT
protected:
//    DefParCompl(MapUserData, UserList)

    QString mTableName;
    bool mHasPlotName, mHasCompany, mHasDepartment, mHasHireDate, mHasTrialPeriod, mHasEPH;
    QString mSelectQuery;

    QList<UserData *> mUserList;

    UserDataList();

    void CheckListInternal();
public:
    static UserDataList * GetInstance() {
        static UserDataList * lUserDataList = NULL;
        if (!lUserDataList) lUserDataList = new UserDataList();
        return lUserDataList;
    }

    void InitUserList();

    QList<UserData *> &UsersRef();
    const QList<UserData *> &UsersConst();

    UserData * FindByName(const QString &aName);
    UserData * FindByLogin(const QString &aLogin);

    const QString & GetName(const QString &aLogin);
    const QString & GetLogin(const QString &aName);

    UserData *SelectUser(UserData *aSelectedUser, uint aListFlags = 0,
                         int (*aCheckForInclude)(const UserData *, void *) = NULL, void *apData = NULL) const;
    bool SelectUsers(QList<UserData *> &aSelected, const QList<UserData *> *aExcludedUsers);
    bool SelectUsers(QList<UserData *> &aSelected, const QStringList *aExcludedLogins);

    const QString &TableName() const;

    bool CreateUser(const QString &aLogin, const QString &aPassword) const;
    bool DropUser(const QString &aLogin, bool aShowError) const;
    bool LockUser(const QString &aLogin, bool aShowError) const;
    bool UnlockUser(const QString &aLogin) const;

    // columns
    bool HasPlotname() const;
    bool HasCompany() const;
    bool HasDepartment() const;
    bool HasHireDate() const;
    bool HasTrialPeriod() const;
    bool HasEPH() const;
signals:
    void UsersBeforeUpdate();
    void UsersNeedUpdate();
};

#define gUsers UserDataList::GetInstance()

#define UsersDlgShowGip     1
#define UsersDlgShowBoss    2

#undef USERSDLG_LIBRARY_EXP_IMP

#endif // USERDATA_H
