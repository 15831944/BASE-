#ifndef WAITDLG_H
#define WAITDLG_H

#include "qfcdialog.h"

namespace Ui {
class WaitDlg;
}

#include "def_expimp.h"

class EXP_IMP WaitDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit WaitDlg(QWidget *parent, bool aDoNotDisable = false);
    virtual ~WaitDlg();

    void SetCanCancelled(bool aCanCancelled);
    bool CancelRequested();

    void SetMessage(const QString & aMessage);

private slots:
    void on_pbCancel_clicked();

private:
    QWidget *mWidgetForDisable;
    bool mCancelled;
    Ui::WaitDlg *ui;
};

#endif // WAITDLG_H
