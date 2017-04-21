#ifndef PLOTATTRDLG_H
#define PLOTATTRDLG_H

#include "../VProject/qfcdialog.h"
#include "../VProject/PlotListItemDelegate.h"

#include <QTreeWidgetItem>

class PlotData;
class PlotHistoryData;
typedef QPair<PlotData *, PlotHistoryData *> PlotAndHistoryData;

//class ProjectData;

namespace Ui {
class PlotAttrDlg;
}

class PlotAttrDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit PlotAttrDlg(QWidget *parent = 0);
    ~PlotAttrDlg();

    void SaveItemsForEdit();
protected:
    QList<PlotAndHistoryData> mData;
    QString mNewString;
    QList<QTreeWidgetItem *> mItemsForEdit;

    void InitInConstructor();

private slots:
    void ShowData();
    void PlotsSelectionChanged(QList<PlotAndHistoryData> aData);
    void DoSelectColumns(const QPoint &aPoint);

    void on_cbAutoUpdate_toggled(bool checked);

    void on_cbTag_currentIndexChanged(int index);

    void onCommitDataTimer();
    void onCommitData(QWidget *editor);

private:
    Ui::PlotAttrDlg *ui;
};


class PlotAttrItemDelegate : public ROPlotListItemDelegate
{
public:
    explicit PlotAttrItemDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // PLOTATTRDLG_H
