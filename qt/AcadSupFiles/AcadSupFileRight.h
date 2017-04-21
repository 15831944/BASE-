#ifndef ACADSUPFILERIGHT_H
#define ACADSUPFILERIGHT_H

#include "../VProject/qfcdialog.h"

#include <QStyledItemDelegate>

namespace Ui {
class AcadSupFileRight;
}

class AcadSupFileRight : public QFCDialog
{
    Q_OBJECT

public:
    explicit AcadSupFileRight(QWidget *parent = 0);
    ~AcadSupFileRight();

protected:
    virtual void showEvent(QShowEvent* event);
    void PopulateData();

private slots:
    void on_actionAdd_triggered();
    void Accept();
    void on_actionDel_triggered();

    void on_twRights_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    Ui::AcadSupFileRight *ui;

    QMap<QString, int> mDDList;
    QList<int> mListForDel;
};

class QPCRDepartmentItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
private:
    QMap<QString, int> mDDList;
public:
    explicit QPCRDepartmentItemDelegate(QMap<QString, int> aDDList, QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class QPCRPersonItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QPCRPersonItemDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // ACADSUPFILERIGHT_H
