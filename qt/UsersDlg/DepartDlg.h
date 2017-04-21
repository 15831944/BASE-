#ifndef DEPARTDLG_H
#define DEPARTDLG_H

#include <QTreeWidgetItem>
#include <QStyledItemDelegate>

#include "../VProject/qfcdialog.h"

namespace Ui {
class DepartDlg;
}

class DepartDlg : public QFCDialog
{
    Q_OBJECT

public:
    enum DisplayTypeEnum { DTEdit, DTView, DTSelect };
protected:
    int mCurrentId;
    DisplayTypeEnum mDisplayType;

    QString mStrFromEdit;

    void InitInConstructor();
    virtual void showEvent(QShowEvent* event);
public:
    explicit DepartDlg(DisplayTypeEnum aDisplayType, QWidget *parent = 0);
    explicit DepartDlg(QSettings &aSettings, QWidget *parent = 0);
    virtual ~DepartDlg();

    DisplayTypeEnum DisplayType() const { return mDisplayType; }

    virtual void SaveState(QSettings &aSettings);
    void ShowData();

private slots:
    void OnNeedUpdate();
    void DoSelectColumns(const QPoint &aPoint);

    void OnCommitDataTimer();
    void OnCommitData(QWidget *editor);

    void on_twDepart_customContextMenuRequested(const QPoint &pos);

    void on_actionNew_triggered();

    void on_actionRename_triggered();

    void on_actionDelete_triggered();

    void on_twDepart_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    bool mJustStarted;
    Ui::DepartDlg *ui;
};

class DepartListItemDelegate : public QStyledItemDelegate
{
public:
    explicit DepartListItemDelegate(QWidget *parent = 0);
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // DEPARTDLG_H
