#ifndef FINDDLG_H
#define FINDDLG_H

#include <QtSql/QSqlQuery>

#include "qfcdialog.h"

namespace Ui {
class FindDlg;
}

class FindDlg : public QFCDialog
{
    Q_OBJECT

protected:
    QSqlQuery * qSelect;
    bool qSelectPrepared;

    virtual void showEvent(QShowEvent* event);
    bool ShowData(int aStart, int aCount);
    void Clear();
public:
    explicit FindDlg(QWidget *parent = 0);
    ~FindDlg();

private slots:
    void on_pbFind_clicked();

    void on_cbEdited_toggled(bool checked);

    void on_pbPrev_clicked();

    void on_pbNext_clicked(bool checked);

    void on_pbClear_clicked();

    void on_leIdProject_editingFinished();

    void on_toolButton_clicked();

private:
    bool mJustStarted;
    int mIdProject;
    Ui::FindDlg *ui;
};

#endif // FINDDLG_H
