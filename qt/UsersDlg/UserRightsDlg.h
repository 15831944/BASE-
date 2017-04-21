#ifndef USERRIGHTSDLG_H
#define USERRIGHTSDLG_H

#include <QStyledItemDelegate>

#include "../VProject/qfcdialog.h"

class UserData;

namespace Ui {
class UserRightsDlg;
}

class UserRightsDlg : public QFCDialog
{
    Q_OBJECT
protected:
    UserData *mUser;
    bool mCanGrantAnyRole;

    QList<QList<QPair<QString, QString>>> mAllRoles;

    virtual void showEvent(QShowEvent* event);
public:
    explicit UserRightsDlg(UserData *aUser, QWidget *parent = 0);
    ~UserRightsDlg();

    const QList<QList<QPair<QString, QString>>> &AllRolesConst() { return mAllRoles; }

private slots:
    void ShowData();
    void OnCommitData(QWidget *editor);
    void Accept();

private:
    bool mJustStarted;

    Ui::UserRightsDlg *ui;
};

class UserRightsItemDelegate : public QStyledItemDelegate
{
protected:
    UserRightsDlg *mUserRightsDlg;
public:
    explicit UserRightsItemDelegate(QWidget *parent, UserRightsDlg *aUserRightsDlg);
    //virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};


#endif // USERRIGHTSDLG_H
