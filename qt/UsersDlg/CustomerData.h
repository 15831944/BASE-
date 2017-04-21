#ifndef CUSTOMERDATA_H
#define CUSTOMERDATA_H

#include <QString>
#include <QDate>
#include <QMap>
#include <QObject>

#include "../VProject/CommonData.h"

class CustomerDataProp;

class CustomerPerson;

class CustomerData
{
    DeclareParRO(int, Id)
    DefParStr(ShortName)
    DefParStr(Name)
    DeclarePar(int, IsClient)
    DeclarePar(int, IsProvider)
    DefParCompl(QDateTime, DelDate)
    DefParStr(DelUser)
    DefParStr(Comments)

#define DEFINE_GROUP_NUM 1
    DefParComplNum(QList<CustomerDataProp *>, Props)
#undef DEFINE_GROUP_NUM

#define DEFINE_GROUP_NUM 2
    DefParComplNum(QList<CustomerPerson *>, Persons)
#undef DEFINE_GROUP_NUM

private:
    QList <int> mInited;
    void InitGrpData(int aGrpNum);

    bool mIsNew;
public:
    explicit CustomerData();
    explicit CustomerData(long aId, const QString &aShortName, const QString &aName, int aIsClient, int aIsProvider,
                          const QDateTime &aDelDate, const QString &aDelUser, const QString &aComments);
    virtual ~CustomerData();

    void LoadProps();
    void LoadPersons();

    bool RemoveFromDB();
    bool SaveData(bool &aNoChanges);
    void RollbackEdit();
    void CommitEdit();
};

Q_DECLARE_METATYPE(CustomerData *)

class CustomerDataProp
{
    DeclareParRO(int, Id)
    DeclarePar(long, IdCustomer)
    DeclareParRO(int, OrderBy)
    DefParStrRO(Name)
    DefParStrRO(Title)
    DefParStr(Value)
    DefParStr(Comments)

    DeclareParRO(int, Required)

    DefParStrRO(RegExp)
private:
    bool mIsNew;
public:
    explicit CustomerDataProp(int aId, long aIdCustomer, int aOrderBy, const QString &aName, const QString &aTitle, const QString &aValue,
                              const QString &aComments, int aRequired, const QString &aRegExp);

    bool SaveData(bool &aNoChanges);
    void RollbackEdit();
    void CommitEdit();
};

class CustomerPerson
{
    DeclareParRO(int, Id)
    DeclareParRO(int, IdCustomer)
    DefParStr(LastName)
    DefParStr(FirstName)
    DefParStr(MiddleName)
    DefParStr(Post)
    DefParStr(Tel1)
    DefParStr(Tel2)
    DefParStr(TelMob)
    DefParStr(Email)
    DefParStr(FioD)
    DefParStr(PostD)
    DefParStr(MastD)
    DefParCompl(QDateTime, DelDate)
    DefParStr(DelUser)

private:
    bool mIsNew;
public:
    explicit CustomerPerson(int aId, int aIdCustomer, const QString &aLastName, const QString &aFirstName, const QString &aMiddleName, const QString &aPost,
                            const QString &aTel1, const QString &aTel2, const QString &aTelMob, const QString &aEmail,
                            const QString &aFioD, const QString &aPostD, const QString &aMastD,
                            const QDateTime &aDelDate, const QString &aDelUser);
    QString FullName() const;

    bool SaveData(bool &aNotChanged);
    void RollbackEdit();
    void CommitEdit();
};

Q_DECLARE_METATYPE(CustomerPerson *)

class CustomerDataList : public QObject
{
    Q_OBJECT
protected:
//    DefParCompl(MapUserData, UserList)
    bool mHasIsClient, mHasIsSupplier, mPersonHasMidName, mPersonHasDP;

    QList<CustomerData *> mCustomerList;

    CustomerDataList();

public:
    static CustomerDataList * GetInstance() {
        static CustomerDataList * lCustomerDataList = NULL;
        if (!lCustomerDataList) lCustomerDataList = new CustomerDataList();
        return lCustomerDataList;
    }

    void InitCustomerDataList();

    QList<CustomerData *> & CustomerList();
    const QList<CustomerData *> & CustomerListConst() const;

    CustomerData *GetCustomerByShortName(const QString &aShortName);
    CustomerData *GetCustomerById(int aId);

    CustomerPerson *GetPersonById(int aId);

    CustomerData *SelectCustomer(const CustomerData *aSelectedCustomer,
                                 int (*aCheckForInclude)(const CustomerData *, void *) = NULL, void *apData = NULL) const;
    int SelectCustomer(int aSelectedCustomer,
                       int (*aCheckForInclude)(const CustomerData *, void *) = NULL, void *apData = NULL) const;
    bool SelectCustomers(QList<CustomerData *> &aSelectedCustomers,
                         int (*aCheckForInclude)(const CustomerData *, void *) = NULL, void *apData = NULL) const;

    CustomerPerson *SelectCustomerPerson(const CustomerPerson *aCustomerPerson) const;
    int SelectCustomerPerson(int aCustomerPerson) const;
    bool SelectCustomersPersons(QList<CustomerPerson *> &aSelectedPersons) const;

    bool SelectCustomersPersons(QList<CustomerData *> &aSelectedCustomers, QList<CustomerPerson *> &aSelectedPersons) const;

    // columns
    bool HasIsClient() const;
    bool HasIsSupplier() const;
    bool PersonHasMidName() const;
    bool PersonHasDP() const;

    void EmitCustListBeforeUpdate(int aIdCustomer); // 0 - all
    void EmitCustListNeedUpdate(int aIdCustomer); // 0 - all
signals:

    void CustListBeforeUpdate(int aIdCustomer); // 0 - all
    void CustListNeedUpdate(int aIdCustomer); // 0 - all
};

#define gCustomers CustomerDataList::GetInstance()
#endif // CUSTOMERDATA_H
