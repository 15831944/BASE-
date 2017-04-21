#ifndef USERSDLG_H
#define USERSDLG_H

#include "UserData.h"

#include "../VProject/qfcdialog.h"

namespace Ui {
class UsersDlg;
}

class UsersDlg : public QFCDialog
{
    Q_OBJECT

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void StyleSheetChangedInSescendant();
public:
    enum DisplayTypeEnum { DTEdit = 0, DTSelectOne, DTSelectMany };

    explicit UsersDlg(DisplayTypeEnum aDisplayType, QWidget *parent = 0);
    explicit UsersDlg(QSettings &aSettings, QWidget *parent = 0);
    virtual ~UsersDlg();

    virtual void SaveState(QSettings &aSettings);

    void SetCheckForInclude(int (*aCheckForInclude)(const UserData *, void *), void *apData);
    void SetListFlags(uint aListFlags);
    void SetExcludedUsers(const QList<UserData *> *aExcludedUsers);
    void SetExcludedLogins(const QStringList *aExcludedLogins);

    void SetSelectedUserId(int aSelectedUserId);
    UserData *GetSelectedUser() const;
    const QList<UserData *> &GetSelectedUsers() const;

    void FillRightPart(int aRow, const UserData *aUser);
public slots:
    virtual void done(int r);
private slots:
    void ShowData();
    void Accept();

    void MainScrolled(int aValue);
    void CommonScrolled(int aValue);
    void MainSortChanged(int logicalIndex, Qt::SortOrder order);
    void CommonSortChanged(int logicalIndex, Qt::SortOrder order);

    void on_twMain_itemSelectionChanged();

    void on_twCommon_itemSelectionChanged();

    void on_twCommon_customContextMenuRequested(const QPoint &pos);

    void on_cbType_currentIndexChanged(int index);

    void on_leFilter_textEdited(const QString &arg1);

    void on_tbReload_clicked();

    void on_actionProperties_triggered();

    void on_twMain_customContextMenuRequested(const QPoint &pos);

    void on_twMain_cellDoubleClicked(int row, int column);

    void on_twCommon_cellDoubleClicked(int row, int column);

    void on_actionFire_block_triggered();

    void on_actionCopy_triggered();

    void on_actionRights_triggered();

    void on_actionChange_password_triggered();

    void on_actionAdd_triggered();

    void on_pbUpdateRights_clicked();

    void on_actionRestore_triggered();

private:
    bool mJustStarted, mInShowData;

    // 0 - NO, 1 - on circuminates, 2 - YES
    int (*mCheckForInclude)(const UserData *aUser, void *apData);
    void *mpDataForCheckForInclude;

    DisplayTypeEnum mDisplayType;
    uint mListFlags;

    int mSelectedUserId;
    UserData *mSelectedUser;

    QList<UserData *> mSelectedUsers;
    const QList<UserData *> *mExcludedUsers;
    const QStringList *mExcludedLogins;

    void InitInConstructor();
    void ContextMenuForUsers();

    Ui::UsersDlg *ui;
};

#endif // USERSDLG_H
