#ifndef PLOTDELETEDDLG_H
#define PLOTDELETEDDLG_H

#include <QtSql/QSqlQuery>

#include "../VProject/qfcdialog.h"

class PlotData;

namespace Ui {
class PlotDeletedDlg;
}

class PlotDeletedDlg : public QFCDialog
{
    Q_OBJECT
protected:
    QSqlQuery *mQuery;
    QList<PlotData *> mDeletedPlot; // we are owner here
public:
    explicit PlotDeletedDlg(QWidget *parent = 0);
    ~PlotDeletedDlg();

protected:
    virtual void showEvent(QShowEvent* event);

private slots:
    void ShowData();
    void RequeryData();

    void on_pbPrev_clicked();

    void on_pbNext_clicked();

    void on_twDocs_itemSelectionChanged();

    void on_sbShowCount_valueChanged(int arg1);

private:
    bool mJustStarted;
    Ui::PlotDeletedDlg *ui;
};

#endif // PLOTDELETEDDLG_H
