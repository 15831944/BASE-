#include "CustomerData.h"

#include "../VProject/common.h"
#include "../VProject/HomeData.h"

#include "UserRight.h"
#include "OrganizationsListDlg.h"

#include <QVariant>

CustomerData::CustomerData() :
    mId(0), mIsClient(0), mIsClientOrig(0), mIsProvider(0), mIsProviderOrig(0), mIsNew(true)
{
}

CustomerData::CustomerData(long aId, const QString &aShortName, const QString &aName, int aIsClient, int aIsProvider,
                           const QDateTime &aDelDate, const QString &aDelUser, const QString &aComments) :
    InitParRO(Id), InitPar(ShortName), InitPar(Name), InitPar(IsClient), InitPar(IsProvider),
    InitPar(DelDate), InitPar(DelUser), InitPar(Comments)
{
    mIsNew = !mId;
}

CustomerData::~CustomerData() {
    qDeleteAll(mProps);
}

void CustomerData::InitGrpData(int aGrpNum) {
    if (!mInited.contains(aGrpNum)) {
        switch (aGrpNum) {
        case 1:
            LoadProps();
            break;
        case 2:
            LoadPersons();
            break;
        }
    }
}

void CustomerData::LoadProps() {
    if (!mInited.contains(1)) mInited.append(1);

    qDeleteAll(mProps);
    mProps.clear();

    QSqlQuery query(db);
    query.prepare("select id, orderby, bmname, bmtitle, bmvalue, comments, required, regexp"
                  " from v_custdata where id_customer = :id_customer"
                  " union "
                  " select 0 as id, id as orderby, bmname, bmtitle, '' as bmvalue, '' as comments, required, regexp"
                  " from v_custdata_t where not exists (select id from v_custdata where id_customer = :id_customer)"
                  " order by 2");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("CustomerData::LoadProps", query);
    } else {
        query.bindValue(":id_customer", mId);
        if (!query.exec()) {
            gLogger->ShowSqlError("CustomerData::LoadProps", query);
        } else {
            while (query.next()) {
                mProps.append(new CustomerDataProp(qInt("id"), mId, qInt("orderby"), qString("bmname"), qString("bmtitle"), qString("bmvalue"), qString("comments"),
                                                     qInt("required"), qString("regexp")));
            }
        }
    }
}

void CustomerData::LoadPersons() {
    if (!mInited.contains(2)) mInited.append(2);

    qDeleteAll(mPersons);
    mPersons.clear();

    QSqlQuery query(db);
    query.prepare(QString("select id, last_name, first_name,")
                  + (gCustomers->PersonHasMidName()?" middle_name,":"")
                  + (gCustomers->PersonHasDP()?" fio_d, post_d, mast_d,":"")
                  + " post, tel1, tel2, tel_mob, email, deldate, deluser"
                  " from v_custperson where id_customer = :id_customer");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("CustomerData::LoadPersons", query);
    } else {
        query.bindValue(":id_customer", mId);
        if (!query.exec()) {
            gLogger->ShowSqlError("CustomerData::LoadPersons", query);
        } else {
            while (query.next()) {
                mPersons.append(new CustomerPerson(qInt("id"), mId, qString("last_name"), qString("first_name"),
                                                   gCustomers->PersonHasMidName()?qString("middle_name"):"",
                                                   qString("post"), qString("tel1"), qString("tel2"),
                                                   qString("tel_mob"), qString("email"),
                                                   gCustomers->PersonHasDP()?qString("fio_d"):"",
                                                   gCustomers->PersonHasDP()?qString("post_d"):"",
                                                   gCustomers->PersonHasDP()?qString("mast_d"):"",
                                                   query.value("deldate").toDateTime(), qString("deluser")));
            }
        }
    }
}

bool CustomerData::RemoveFromDB() {
    bool res = false;
    QSqlQuery qDelete(db);
    qDelete.prepare("delete from v_customer where id = :id");
    if (qDelete.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Company data"), qDelete);
    } else {
        qDelete.bindValue(":id", mId);
        if (!qDelete.exec()) {
            // can't delete - try set flagse
            QSqlQuery qUpdate(db);
            qUpdate.prepare("update v_customer set deluser = user, deldate = sysdate where id = :id");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("Company data"), qUpdate);
            } else {
                qUpdate.bindValue(":id", mId);
                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Company data"), qUpdate);
                } else {
                    res = true;
                }
            }
        } else {
            res = true;
        }
    }
    return res;
}

bool CustomerData::SaveData(bool &aNoChanges) {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;
    aNoChanges = false;

    if (!mIsNew) {
        CheckParChange(ShortName, shortname);
        CheckParChange(Name, name);
        if (gCustomers->HasIsClient()) {
            CheckParChangeWithNull(IsClient, is_client, 0);
        }
        if (gCustomers->HasIsSupplier()) {
            CheckParChangeWithNull(IsProvider, is_provider, 0);
        }
        CheckParChange(Comments, comments);

        if (!lUpdStr.isEmpty()) {
            lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

            QSqlQuery qUpdate(db);

            qUpdate.prepare("update v_customer set " + lUpdStr + " where id = ?");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("Company data"), qUpdate);
            } else {
                for (int i = 0; i < lUpdValues.length(); i++) {
                    qUpdate.addBindValue(lUpdValues.at(i));
                }
                qUpdate.addBindValue(mId);
                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Company data"), qUpdate);
                } else {
                    res = true;
                }
            }
        } else {
            res = true;
            aNoChanges = true;
        }
    } else {
        QSqlQuery qInsert(db);

        if (!mId && !gOracle->GetSeqNextVal("customers_id_seq", mId)) return false;

        qInsert.prepare(QString("insert into v_customer (id, shortname, name,")
                        + (gCustomers->HasIsClient()?" is_client,":"")
                        + (gCustomers->HasIsSupplier()?" is_provider,":"")
                        + " comments)"
                        + " values (:id, :shortname, :name,"
                        + (gCustomers->HasIsClient()?" :is_client,":"")
                        + (gCustomers->HasIsSupplier()?" :is_provider,":"")
                        + " :comments)");
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Company data"), qInsert);
        } else {
            qInsert.bindValue(":id", mId);
            qInsert.bindValue(":shortname", mShortName);
            qInsert.bindValue(":name", mName);
            if (gCustomers->HasIsClient()) {
                qInsert.bindValue(":is_client", mIsClient);
            }
            if (gCustomers->HasIsSupplier()) {
                qInsert.bindValue(":is_provider", mIsProvider);
            }
            qInsert.bindValue(":comments", mComments);

            if (!qInsert.exec()) {
                gLogger->ShowSqlError(QObject::tr("Company data"), qInsert);
            } else {
                res = true;
            }
        }
    }
    if (res) {
        bool lAnyPropChanges = false;
        for (int i = 0; i < mProps.length(); i++) {
            if (mIsNew) {
                mProps.at(i)->setIdCustomer(mId);
            }
            bool lPropNoChanges;
            if (!mProps.at(i)->SaveData(lPropNoChanges)) {
                res = false;
                break;
            } else {
                if (!lPropNoChanges) {
                    // was changed
                    lAnyPropChanges = true;
                }
            }
        }
        aNoChanges = aNoChanges && !lAnyPropChanges;
    }

    return res;
}

void CustomerData::RollbackEdit() {
    RollbackPar(ShortName)
    RollbackPar(Name)
    RollbackPar(IsClient)
    RollbackPar(IsProvider)
    RollbackPar(Comments)
    for (int i = 0; i < mProps.length(); i++) {
        mProps.at(i)->RollbackEdit();
    }
}

void CustomerData::CommitEdit() {
    CommitPar(ShortName)
    CommitPar(Name)
    CommitPar(IsClient)
    CommitPar(IsProvider)
    CommitPar(Comments)
    mIsNew = false;
    for (int i = 0; i < mProps.length(); i++) {
        mProps.at(i)->CommitEdit();
    }
}

//----------------------------------------------------------------------------------------------
CustomerDataProp::CustomerDataProp(int aId, long aIdCustomer, int aOrderBy, const QString &aName, const QString &aTitle, const QString &aValue,
                                   const QString &aComments, int aRequired, const QString &aRegExp) :
    InitParRO(Id), InitParRO(IdCustomer), InitParRO(OrderBy), InitParRO(Name), InitParRO(Title), InitPar(Value), InitPar(Comments),
    InitParRO(Required), InitParRO(RegExp)
{
    mIsNew = !mId;
}

bool CustomerDataProp::SaveData(bool &aNoChanges) {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;
    aNoChanges = false;

    if (!mIsNew) {
        CheckParChange(Value, bmvalue);
        CheckParChange(Comments, comments);

        if (!lUpdStr.isEmpty()) {
            lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

            QSqlQuery qUpdate(db);

            qUpdate.prepare("update v_custdata set " + lUpdStr + " where id = ?");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("Company data"), qUpdate);
            } else {
                for (int i = 0; i < lUpdValues.length(); i++) {
                    qUpdate.addBindValue(lUpdValues.at(i));
                }
                qUpdate.addBindValue(mId);
                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Company data"), qUpdate);
                } else {
                    res = true;
                }
            }
        } else {
            res = true;
            aNoChanges = true;
        }
    } else {
        QSqlQuery qInsert(db);

        if (!mId && !gOracle->GetSeqNextVal("custdata_id_seq", mId)) return false;

        qInsert.prepare("insert into v_custdata"
                        " (id, id_customer, bmname, bmtitle, bmvalue, comments,"
                        " orderby, required, regexp) values"
                        " (:id, :id_customer, :bmname, :bmtitle, :bmvalue, :comments,"
                        " :orderby, :required, :regexp)");
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Company data"), qInsert);
        } else {
            qInsert.bindValue(":id", mId);
            qInsert.bindValue(":id_customer", mIdCustomer);
            qInsert.bindValue(":bmname", mName);
            qInsert.bindValue(":bmtitle", mTitle);
            qInsert.bindValue(":bmvalue", mValue);
            qInsert.bindValue(":comments", mComments);
            qInsert.bindValue(":orderby", mOrderBy);
            if (mRequired)
                qInsert.bindValue(":required", mRequired);
            else
                qInsert.bindValue(":required", QVariant());
            qInsert.bindValue(":regexp", mRegExp);

            if (!qInsert.exec()) {
                gLogger->ShowSqlError(QObject::tr("Company data"), qInsert);
            } else {
                res = true;
            }
        }
    }

    return res;
}

void CustomerDataProp::RollbackEdit() {
    RollbackPar(Value)
    RollbackPar(Comments)
}

void CustomerDataProp::CommitEdit() {
    CommitPar(Value)
    CommitPar(Comments)
    mIsNew = false;
}

//----------------------------------------------------------------------------------------------
CustomerPerson::CustomerPerson(int aId, int aIdCustomer, const QString &aLastName, const QString &aFirstName, const QString &aMiddleName, const QString &aPost,
                               const QString &aTel1, const QString &aTel2, const QString &aTelMob, const QString &aEmail,
                               const QString &aFioD, const QString &aPostD, const QString &aMastD,
                               const QDateTime &aDelDate, const QString &aDelUser):
    InitParRO(Id), InitParRO(IdCustomer), InitPar(LastName), InitPar(FirstName), InitPar(MiddleName), InitPar(Post),
    InitPar(Tel1), InitPar(Tel2), InitPar(TelMob), InitPar(Email),
    InitPar(FioD), InitPar(PostD), InitPar(MastD),
    InitPar(DelDate), InitPar(DelUser)
{
    mIsNew = !mId;
}

QString CustomerPerson::FullName() const {
    int lFNForm = gHomeData->Get("CUST_PERS_FULLNAME").toInt();
    QString lFullName;

    switch (lFNForm) {
    case 0:
        // judish form
        lFullName = mFirstName + " " + mLastName;
        break;
    case 1:
        lFullName = mLastName + " " + mFirstName + (mMiddleName.isEmpty()?"":(" " + mMiddleName));
        break;
    case 2:
        lFullName = mLastName + " ";
        if (mFirstName.length() == 2
                && mFirstName.endsWith('.')) {
            lFullName += mFirstName;
        } else {
            lFullName += mFirstName.at(0) + '.';
        }
        if (!mMiddleName.isEmpty()) {
            lFullName += ' ';
            if (mMiddleName.length() == 2
                    && mMiddleName.endsWith('.')) {
                lFullName += mMiddleName;
            } else {
                lFullName += mMiddleName.at(0) + '.';
            }
        }
        break;
    }
    return lFullName;
}

bool CustomerPerson::SaveData(bool &aNotChanged) {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;
    aNotChanged = false;

    if (!mIsNew) {
        CheckParChange(LastName, last_name);
        CheckParChange(FirstName, first_name);
        if (gCustomers->PersonHasMidName()) {
            CheckParChange(MiddleName, middle_name);
        }
        if (gCustomers->PersonHasDP()) {
            CheckParChange(FioD, fio_d);
            CheckParChange(PostD, post_d);
            CheckParChange(MastD, mast_d);
        }
        CheckParChange(Post, post);
        CheckParChange(Tel1, tel1);
        CheckParChange(Tel2, tel2);
        CheckParChange(TelMob, tel_mob);
        CheckParChange(Email, email);

        if (!lUpdStr.isEmpty()) {
            lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

            QSqlQuery qUpdate(db);

            qUpdate.prepare("update v_custperson set " + lUpdStr + " where id = ?");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("Empoloyee data"), qUpdate);
            } else {
                for (int i = 0; i < lUpdValues.length(); i++) {
                    qUpdate.addBindValue(lUpdValues.at(i));
                }
                qUpdate.addBindValue(mId);

                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Empoloyee data"), qUpdate);
                } else {
                    res = true;
                }
            }
        } else {
            res = true;
            aNotChanged = true;
        }
    } else {
        QSqlQuery qInsert(db);

        if (!mId && !gOracle->GetSeqNextVal("cust_persons_id_seq", mId)) return false;

        qInsert.prepare(QString("insert into v_custperson (id, id_customer, last_name, first_name,")
                        + (gCustomers->PersonHasMidName()?" middle_name,":"")
                        + (gCustomers->PersonHasDP()?" fio_d, post_d, mast_d,":"")
                        + " post, tel1, tel2, tel_mob, email)"
                        " values (:id, :id_customer, :last_name, :first_name,"
                        + (gCustomers->PersonHasMidName()?" :middle_name,":"")
                        + (gCustomers->PersonHasDP()?" :fio_d, :post_d, :mast_d,":"")
                        + " :post, :tel1, :tel2, :tel_mob, :email)");
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Empoloyee data"), qInsert);
        } else {
            qInsert.bindValue(":id", mId);
            qInsert.bindValue(":id_customer", mIdCustomer);
            qInsert.bindValue(":last_name", mLastName);
            qInsert.bindValue(":first_name", mFirstName);
            if (gCustomers->PersonHasMidName()) {
                qInsert.bindValue(":middle_name", mMiddleName);
            }
            if (gCustomers->PersonHasDP()) {
                qInsert.bindValue(":fio_d", mFioD);
                qInsert.bindValue(":post_d", mPostD);
                qInsert.bindValue(":mast_d", mMastD);
            }
            qInsert.bindValue(":post", mPost);
            qInsert.bindValue(":tel1", mTel1);
            qInsert.bindValue(":tel2", mTel2);
            qInsert.bindValue(":tel_mob", mTelMob);
            qInsert.bindValue(":email", mEmail);

            if (!qInsert.exec()) {
                gLogger->ShowSqlError(QObject::tr("Empoloyee data"), qInsert);
            } else {
                res = true;
            }
        }
    }

    return res;
}

void CustomerPerson::RollbackEdit() {
    RollbackPar(LastName)
    RollbackPar(FirstName)
    RollbackPar(MiddleName)
    RollbackPar(Post)
    RollbackPar(Tel1)
    RollbackPar(Tel2)
    RollbackPar(TelMob)
    RollbackPar(Email)
    RollbackPar(FioD)
    RollbackPar(PostD)
    RollbackPar(MastD)
}

void CustomerPerson::CommitEdit() {
    CommitPar(LastName)
    CommitPar(FirstName)
    CommitPar(MiddleName)
    CommitPar(Post)
    CommitPar(Tel1)
    CommitPar(Tel2)
    CommitPar(TelMob)
    CommitPar(Email)
    CommitPar(FioD)
    CommitPar(PostD)
    CommitPar(MastD)
    mIsNew = false;
}

//----------------------------------------------------------------------------------------------
CustomerDataList::CustomerDataList() {
    mHasIsClient = gUserRight->HasColumn("v_customer", "is_client");
    mHasIsSupplier = gUserRight->HasColumn("v_customer", "is_provider");
    mPersonHasMidName = gUserRight->HasColumn("v_custperson", "middle_name");
    mPersonHasDP = gUserRight->HasColumn("v_custperson", "fio_d");

    InitCustomerDataList();
}

void CustomerDataList::InitCustomerDataList() {
    EmitCustListBeforeUpdate(0);

    mCustomerList.clear();

    QSqlQuery query(
                QString("select id, shortname, name,") + (mHasIsClient?" is_client,":"") + (mHasIsSupplier?" is_provider,":"")
                + " deldate, deluser, comments from v_customer order by shortname", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Company data", query);
    } else {
        while (query.next()) {
            mCustomerList.append(new CustomerData(qInt("id"), qString("shortname"),
                                                  qString("name"), mHasIsClient?qInt("is_client"):1, mHasIsSupplier?qInt("is_provider"):1,
                                                  query.value("deldate").toDateTime(), qString("deluser"),
                                                  qString("comments")));
        }
    }

    EmitCustListNeedUpdate(0); // update all opened documents lists
}

QList<CustomerData *> &CustomerDataList::CustomerList() {
    return mCustomerList;
}

const QList<CustomerData *> &CustomerDataList::CustomerListConst()  const {
    return mCustomerList;
}

CustomerData *CustomerDataList::GetCustomerByShortName(const QString &aShortName) {
    for (int i = 0; i < mCustomerList.length(); i++) {
        if (mCustomerList.at(i)->ShortNameConst() == aShortName) {
            return mCustomerList.at(i);
        }
    }
    return NULL;
}

CustomerData *CustomerDataList::GetCustomerById(int aId) {
    for (int i = 0; i < mCustomerList.length(); i++) {
        if (mCustomerList.at(i)->Id() == aId) {
            return mCustomerList.at(i);
        }
    }
    return NULL;
}

CustomerPerson *CustomerDataList::GetPersonById(int aId) {
    int i, j;
    for (i = 0; i < mCustomerList.length(); i++) {
        for (j = 0; j < mCustomerList.at(i)->PersonsConst().length(); j++) {
            if (mCustomerList.at(i)->PersonsConst().at(j)->Id() == aId) {
                return mCustomerList.at(i)->PersonsConst().at(j);
            }
        }
    }
    return NULL;
}

CustomerData *CustomerDataList::SelectCustomer(const CustomerData *aSelectedCustomer,
                                               int (*aCheckForInclude)(const CustomerData *, void *), void *apData) const {
    OrganizationsListDlg lDlg(OrganizationsListDlg::DTSelectOneFirm);
    lDlg.SetCheckForInclude(aCheckForInclude, apData);
    if (aSelectedCustomer) {
        lDlg.SetSelectedCustomerId(aSelectedCustomer->Id());
    }
    if (lDlg.exec() == QDialog::Accepted) {
        return lDlg.GetSelectedCustomer();
    }
    return NULL;
}

int CustomerDataList::SelectCustomer(int aSelectedCustomer,
                                     int (*aCheckForInclude)(const CustomerData *, void *), void *apData) const {
    OrganizationsListDlg lDlg(OrganizationsListDlg::DTSelectOneFirm);
    lDlg.SetCheckForInclude(aCheckForInclude, apData);
    lDlg.SetSelectedCustomerId(aSelectedCustomer);
    if (lDlg.exec() == QDialog::Accepted) {
        return lDlg.GetSelectedCustomerId();
    }
    return 0;
}

bool CustomerDataList::SelectCustomers(QList<CustomerData *> &aSelectedCustomers,
                                       int (*aCheckForInclude)(const CustomerData *, void *), void *apData) const {
    OrganizationsListDlg lDlg(OrganizationsListDlg::DTSelectManyFirm);
    lDlg.SetCheckForInclude(aCheckForInclude, apData);
    lDlg.SetSelectedCustomers(aSelectedCustomers);
    if (lDlg.exec() == QDialog::Accepted) {
        aSelectedCustomers = lDlg.GetSelectedCustomers();
        return true;
    }
    return false;
}

CustomerPerson *CustomerDataList::SelectCustomerPerson(const CustomerPerson *aCustomerPerson) const {
    OrganizationsListDlg lDlg(OrganizationsListDlg::DTSelectOnePerson);
    if (aCustomerPerson) {
        lDlg.SetSelectedPersonId(aCustomerPerson->Id());
    }
    if (lDlg.exec() == QDialog::Accepted) {
        return lDlg.GetSelectedPerson();
    }
    return NULL;
}

int CustomerDataList::SelectCustomerPerson(int aCustomerPerson) const {
    OrganizationsListDlg lDlg(OrganizationsListDlg::DTSelectOnePerson);
    lDlg.SetSelectedPersonId(aCustomerPerson);
    if (lDlg.exec() == QDialog::Accepted) {
        return lDlg.GetSelectedPersonId();
    }
    return 0;
}

bool CustomerDataList::SelectCustomersPersons(QList<CustomerPerson *> &aSelectedPersons) const {
    OrganizationsListDlg lDlg(OrganizationsListDlg::DTSelectManyPerson);
    lDlg.SetSelectedPersons(aSelectedPersons);
    if (lDlg.exec() == QDialog::Accepted) {
        aSelectedPersons = lDlg.GetSelectedPersons();
        return true;
    }
    return false;
}

bool CustomerDataList::SelectCustomersPersons(QList<CustomerData *> &aSelectedCustomers, QList<CustomerPerson *> &aSelectedPersons) const {
    OrganizationsListDlg lDlg(OrganizationsListDlg::DTSelectMany);
    lDlg.SetSelectedCustomers(aSelectedCustomers);
    lDlg.SetSelectedPersons(aSelectedPersons);
    if (lDlg.exec() == QDialog::Accepted) {
        aSelectedCustomers = lDlg.GetSelectedCustomers();
        aSelectedPersons = lDlg.GetSelectedPersons();
        return true;
    }
    return false;
}

bool CustomerDataList::HasIsClient() const {
    return mHasIsClient;
}
bool CustomerDataList::HasIsSupplier() const {
    return mHasIsSupplier;
}

bool CustomerDataList::PersonHasMidName() const {
    return mPersonHasMidName;
}

bool CustomerDataList::PersonHasDP() const {
    return mPersonHasDP;
}

void CustomerDataList::EmitCustListBeforeUpdate(int aIdCustomer) { // 0 - all
    emit CustListBeforeUpdate(aIdCustomer);
}

void CustomerDataList::EmitCustListNeedUpdate(int aIdCustomer) { // 0 - all
    emit CustListNeedUpdate(aIdCustomer);
}
