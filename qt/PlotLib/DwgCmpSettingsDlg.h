#ifndef DWGCMPSETTINGSDLG_H
#define DWGCMPSETTINGSDLG_H

#include "../VProject/qfcdialog.h"

#if defined(VPROJECT_MAIN_IMPORT)
    #define PLOT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PLOT_LIBRARY)
        #define PLOT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PLOT_LIBRARY_EXP_IMP
    #endif
#endif

namespace Ui {
class DwgCmpSettingsDlg;
}

class PLOT_LIBRARY_EXP_IMP DwgCmpSettingsDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit DwgCmpSettingsDlg(QWidget *parent = 0);
    ~DwgCmpSettingsDlg();

protected:
    virtual void showEvent(QShowEvent* event);

private slots:
    void FirstInit();

    void on_pbOK_clicked();

    void on_tbSelectPath_clicked();

private:
    bool mJustStarted;

    Ui::DwgCmpSettingsDlg *ui;
};

#undef PLOT_LIBRARY_EXP_IMP

#endif // DWGCMPSETTINGSDLG_H
