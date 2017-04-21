#ifndef LISETTINGSDLG_H
#define LISETTINGSDLG_H

#include "../VProject/qfcdialog.h"

#include "saveloadlib_global.h"

namespace Ui {
class LISettingsDlg;
}

class SAVELOADLIBSHARED_EXPORT LISettingsDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit LISettingsDlg(QWidget *parent = 0);
    virtual ~LISettingsDlg();

private slots:
    void Accept();
private:
    bool mJustStarted;
    Ui::LISettingsDlg *ui;
};

#endif // LISETTINGSDLG_H
