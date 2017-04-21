#ifndef CONTRACTPROP_H
#define CONTRACTPROP_H

#include "../VProject/qfcdialog.h"

#include <QDateTime>

namespace Ui {
class ContractProp;
}

class ContractProp : public QFCDialog
{
    Q_OBJECT

public:
    explicit ContractProp(QWidget *parent = 0);
    ~ContractProp();

    void SetContractId(int aIdContract) { mIdContract = aIdContract; }
    void SetProjectIdForNew(int aIdProject) { mIdProjectForNew = aIdProject; }
    int GetUpdateId() const { return UpdateId; }

protected:
    int mFileToDo;
    QString mAttachFileName;

    virtual void showEvent(QShowEvent* event);

private slots:
    void on_toolButton_clicked();
    void IdProjectChanged();
    void Accept();

    void on_leSumOrig_textEdited(const QString &arg1);

    void on_leNdsPerCentOrig_textEdited(const QString &arg1);

    void on_leSumNdsOrig_textEdited(const QString &arg1);

    void on_leSumFullOrig_textEdited(const QString &arg1);

    void on_leSumAct_textEdited(const QString &arg1);

    void on_leNdsPerCentAct_textEdited(const QString &arg1);

    void on_leSumNdsAct_textEdited(const QString &arg1);

    void on_leSumFullAct_textEdited(const QString &arg1);

    void on_pbAttach_clicked();

    void on_pbRemove_clicked();

    void on_pbView_clicked();

    void on_dtStart_userDateChanged(const QDate &date);

private:
    Ui::ContractProp *ui;

    int mIdContract, mIdProjectForNew;

    int mIdPlot;

    int origProjectId, newProjectId;
    QString origCustomer, origNum;
    QDate origStartDate, origEndDate;
    QString origName;
    qlonglong origSumBruttoOrig, origNdsPercentOrig, origSumFullOrig;
    qlonglong origSumBruttoAct, origNdsPercentAct, origSumFullAct;
    int origIndexingType;
    QString origComments;

    bool isNewRecord;
    int UpdateId;
};

#endif // CONTRACTPROP_H
