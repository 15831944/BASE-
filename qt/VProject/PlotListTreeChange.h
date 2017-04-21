#ifndef PLOTLISTTREECHANGE_H
#define PLOTLISTTREECHANGE_H

#include "qfcdialog.h"
#include "PlotListTree.h"

namespace Ui {
class PlotListTreeChange;
}

class PlotListTreeChange : public QFCDialog
{
    Q_OBJECT

public:
    explicit PlotListTreeChange(QWidget *parent = 0);
    ~PlotListTreeChange();

    bool SetData(PlotListTree::PLTCols aColumn, const QList <QTreeWidgetItem *> & aItems);

    const QList<int> & ProjectIds() const { return mProjectIds; }

private:
    bool mJustStarted;
    PlotListTree::PLTCols mColumn;

    QList<int> mProjectIds;

    void OnChange();
protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    void on_leFrom_textEdited(const QString &arg1);

    void on_leTo_textEdited(const QString &arg1);

    void Accept();
private:
    Ui::PlotListTreeChange *ui;
};

#endif // PLOTLISTTREECHANGE_H
