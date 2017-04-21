#ifndef REPLACETEXTDLG_H
#define REPLACETEXTDLG_H

#include "qfcdialog.h"

namespace Ui {
class ReplaceTextDlg;
}

class ReplaceTextDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit ReplaceTextDlg(QWidget *parent = 0);
    ~ReplaceTextDlg();

    QString FindTextCopy() const;
    QString ReplaceWithCopy() const;
    long MoveType() const;
    double DX() const;
    double DY() const;

private slots:
    void Accept();
    void on_cbMoveType_currentIndexChanged(int index);

private:
    Ui::ReplaceTextDlg *ui;
};

#endif // REPLACETEXTDLG_H
