#ifndef ORGANIZATIONSLISTDLG_H
#define ORGANIZATIONSLISTDLG_H

#include <QListWidgetItem>

#include "../VProject/qfcdialog.h"

class QHBoxLayout;
class CustomerData;
class CustomerPerson;
class QSignalMapper;

namespace Ui {
class OrganizationsListDlg;
}

class OrganizationsListDlg : public QFCDialog
{
    Q_OBJECT

public:
    enum DisplayTypeEnum { DTEdit = 0, DTView, DTSelectOneFirm, DTSelectManyFirm,
                           DTSelectOnePerson, DTSelectManyPerson, DTSelectMany };
protected:
    DisplayTypeEnum mDisplayType;
    QSignalMapper *mSignalMapper;

    // 0 - NO, 1 - on circuminates, 2 - YES
    int (*mCheckForInclude)(const CustomerData *aCustomer, void *apData);
    void *mpDataForCheckForInclude;

    int mSelectedCustomerId;
    int mCustomerScrollPos;
    CustomerData *mSelectedCustomer;
    QList<CustomerData *> mSelectedCustomers;

    int mSelectedPersonId;
    int mPersonScrollPos;
    CustomerPerson *mSelectedPerson;
    QList<CustomerPerson *> mSelectedPersons;

    bool mFilterWithPersons, mFilterWithData;
    bool mShowDeleted;
    bool mInResise, mInResizeTimer;

    bool mCanAddCustomer, mCanDelCustomer, mCanSeeCustProps, mCanAddPerson, mCanDelPerson;

    QPalette mPaletteAlwaysSelected, mPaletteAlwaysSelectedDis;
    QPalette mPaletteNorm, mPaletteDis;

    void InitInConstructor();
    virtual void showEvent(QShowEvent* event);
    virtual void StyleSheetChangedInSescendant();
    virtual void resizeEvent(QResizeEvent * event);

//    void wdSelectedCleanupInternal(QLayout *aLayout);
//    void wdSelectedCleanup();
    void wdSelectedRebuildCleanLayout(QLayout *aLayout);
    void wdSelectedAdd(bool aIsFirst, QHBoxLayout *&aLastHBoxLayout, int &aCurWidth,
                       CustomerData *aCustomer, CustomerPerson *aPerson);
public:
    explicit OrganizationsListDlg(DisplayTypeEnum aDisplayType, QWidget *parent = 0);
    explicit OrganizationsListDlg(QSettings &aSettings, QWidget *parent = 0);
    virtual ~OrganizationsListDlg();

    void SetCheckForInclude(int (*aCheckForInclude)(const CustomerData *, void *), void *apData);

    void SetSelectedCustomerId(int aIdCustomer);
    void SetSelectedCustomers(const QList<CustomerData *> &aSelectedCustomers);
    int GetSelectedCustomerId() const;
    CustomerData *GetSelectedCustomer() const;
    const QList<CustomerData *> &GetSelectedCustomers() const;

    void SetSelectedPersonId(int aIdPerson);
    void SetSelectedPersons(const QList<CustomerPerson *> &aSelectedPersons);
    int GetSelectedPersonId() const;
    CustomerPerson *GetSelectedPerson() const;
    const QList<CustomerPerson *> &GetSelectedPersons() const;


//    DisplayTypeEnum DisplayType() const { return mDisplayType; }
private slots:
    void ShowData();
    void MainScrolled(int aValue);
    void CommonScrolled(int aValue);
    void OnPressed(QWidget *aWidget);
    void wdSelectedRebuild();

    void OnCustListBeforeUpdate(int aIdCustomer);
    void OnCustListNeedUpdate(int aIdCustomer);

    void on_actionAdd_triggered();

    void on_listCustomers_customContextMenuRequested(const QPoint &pos);

    void on_listCustomers_doubleClicked(const QModelIndex &index);

    void on_actionDelete_triggered();

    void on_actionProperties_triggered();

    void on_twMain_itemSelectionChanged();

    void on_twCommon_itemSelectionChanged();

    void on_leFilter_textEdited(const QString &arg1);

    void on_twMain_customContextMenuRequested(const QPoint &pos);

    void on_twCommon_customContextMenuRequested(const QPoint &pos);

    void on_twMain_doubleClicked(const QModelIndex &index);

    void on_twCommon_doubleClicked(const QModelIndex &index);

    void on_actionRefresh_triggered();

    void on_leFilter_customContextMenuRequested(const QPoint &pos);

    void on_actionAdd_person_triggered();

    void on_actionProperties_of_person_triggered();

    void on_listCustomers_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_splitter_splitterMoved(int pos, int index);

    void on_twCustProps_customContextMenuRequested(const QPoint &pos);

    void on_actionSelect_triggered();

    void on_actionAdd_to_selection_triggered();

    void on_pbSelect_clicked();

    void on_actionSelectPerson_triggered();

    void on_actionAddPerson_to_selection_triggered();

private:
    int personRow;
    bool mJustStarted;
    Ui::OrganizationsListDlg *ui;
    QList<int> delCustomersID;
};

#endif // ORGANIZATIONSLISTDLG_H

