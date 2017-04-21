#ifndef CONTRACTDEFVAT_H
#define CONTRACTDEFVAT_H

#include "../VProject/qfcdialog.h"

namespace Ui {
class ContractDefVAT;
}

class ContractDefVAT : public QFCDialog
{
    Q_OBJECT

public:
    explicit ContractDefVAT(QWidget *parent = 0);
    ~ContractDefVAT();

    bool AnyChanged;

protected:
    virtual void showEvent(QShowEvent* event);

private slots:
    void Accept();
    void on_pbProject_clicked();

    void on_pbContract_clicked();

    void on_pbStage_clicked();

    void on_cbProject_stateChanged(int arg1);

    void on_cbContract_stateChanged(int arg1);

    void on_cbStage_stateChanged(int arg1);

private:
    Ui::ContractDefVAT *ui;

    QColor mProjectColor, mContractColor, mStageColor;
};

#endif // CONTRACTDEFVAT_H
