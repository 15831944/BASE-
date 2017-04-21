#ifndef TASKPROP_H
#define TASKPROP_H

#include "qfcdialog.h"

#include <QString>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>

namespace Ui {
class TaskProp;
}

class TaskProp : public QFCDialog
{
    Q_OBJECT

public:
    explicit TaskProp(QWidget *parent = 0);
    ~TaskProp();

    void SetIdTask(int aIdTask) { mIdTask = aIdTask; }
protected:
    virtual void showEvent(QShowEvent* event);

    void IdProjectChanged();
private slots:
    void Accept();
    void UserDataChanged(QWidget *editor);

    void on_twUsers_customContextMenuRequested(const QPoint &pos);

    void on_twDocs_customContextMenuRequested(const QPoint &pos);

    void on_toolButton_clicked();

    void on_leIdProject_editingFinished();

private:
    Ui::TaskProp *ui;

    bool mNewRecord;
    int mIdTask/*, mIdProject*/;

    QString mFromUser;
};

class TaskUserData : public QTreeWidgetItem
{
public:
    explicit TaskUserData(int aNumber, int aRights);
    explicit TaskUserData(int aNumber, int aId, const QString &aToUser, const QDate &aCompleteDate, const QDate &aVerifyDate,
                          const QString &aVerifyUser, int aRights);
    int Rights() const { return mRights; }
    int Id() const { return mId; }
    bool IsChanged() const;
    bool CompleteDateChanged() const;
    bool VerifyDateChanged() const;
private:
    int mRights;
    int mId;

    QString origToUser;
    QDate origCompleteDate, origVerifyDate;
    QString origVerifyUser, origComments;
};

class TaskUserDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TaskUserDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const;
};

#endif // TASKPROP_H
