#ifndef PRELOADPARAMSDLG_H
#define PRELOADPARAMSDLG_H

#include "qfcdialog.h"
#include <QComboBox>

#define NOMINMAX
#include <windows.h>

namespace Ui {
class PreloadParamsDlg;
}

class PreloadParamsDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit PreloadParamsDlg(QWidget *parent = 0);
    ~PreloadParamsDlg();

    ULONG ProcessType();
    ULONG ColorBlocks();
    ULONG ColorEntities();
    ULONG LWBlocks();
    ULONG LWEntities();

    QString Layer0Name();
    QString UserCommands();

private slots:
    void on_pbOK_clicked();

    void on_pbDefaults_clicked();

    void on_cbColorBlocks_toggled(bool checked);

    void on_cbColorEntities_toggled(bool checked);

    void on_cbLWBlocks_toggled(bool checked);

    void on_cbLWEntities_toggled(bool checked);

    void on_cbRename0_toggled(bool checked);

    void on_cbUserCommands_toggled(bool checked);

private:
    Ui::PreloadParamsDlg *ui;

    void FillLineWeights(QComboBox *aListLW);
};

#endif // PRELOADPARAMSDLG_H
