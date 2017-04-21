#ifndef PUBLISHDLG_H
#define PUBLISHDLG_H

#include "qfcdialog.h"

namespace Ui {
class PublishDlg;
}

class PublishDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit PublishDlg(QWidget *parent = 0);
    ~PublishDlg();

private slots:
    void Accept();
    void CheckAnySelected(int aDummy);
    void on_cbPLT_toggled(bool checked);

    void on_tbTreeSel_2_clicked();

private:
    Ui::PublishDlg *ui;
};

#endif // PUBLISHDLG_H
