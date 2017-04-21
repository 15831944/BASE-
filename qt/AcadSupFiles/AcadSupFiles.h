#ifndef ACADSUPFILES_H
#define ACADSUPFILES_H

#include "../VProject/qfcdialog.h"
#include "acadsupfiles_global.h"

#include <QStyledItemDelegate>

namespace Ui {
class AcadSupFiles;
}

class ACADSUPFILESSHARED_EXPORT AcadSupFiles : public QFCDialog
{
    Q_OBJECT

public:
    explicit AcadSupFiles(QWidget *parent = 0);
    ~AcadSupFiles();

    void SetSelectedIdDepartment(int aSelectedIdDepartment) { mSelectedIdDepartment = aSelectedIdDepartment; }
    void SetSelectedFileName(QString aSelectedFileName) { mSelectedFileName = aSelectedFileName; }

protected:
    virtual void showEvent(QShowEvent* event);

    void PopulateList();
    void SaveToDB(bool &aChanges, bool &aOk, bool aAskForSave);

private slots:
    void on_cbDepartment_currentIndexChanged(int index);

    void on_actionAddFiles_triggered();

    void on_pbUserRights_clicked();

    void Accept();
    void on_twList_customContextMenuRequested(const QPoint &pos);
    void on_actionDelFIles_triggered();

    void CommentsChanged(QWidget *editor);
private:
    Ui::AcadSupFiles *ui;

    int mType;

    int mUserRight;
    int mCurDepartment;

    int mSelectedIdDepartment;
    QString mSelectedFileName;

    bool mInSetDepartment;
    QList<int> mListForDel;
};

class QNoEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QNoEditDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class QEditNewOnlyDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QEditNewOnlyDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

ACADSUPFILESSHARED_EXPORT void ASF_Sync();

#endif // ACADSUPFILES_H
