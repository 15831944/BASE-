#ifndef CONTRACTSTAGEPROP_H
#define CONTRACTSTAGEPROP_H

#include "../VProject/qfcdialog.h"

#include <QDateTime>

namespace Ui {
class ContractStageProp;
}

class ContractStageProp : public QFCDialog
{
    Q_OBJECT

public:
    explicit ContractStageProp(QWidget *parent = 0);
    ~ContractStageProp();

    void SetContractStageId(int aIdContractStage) { mIdContractStage = aIdContractStage; }
    void SetContractId(int aIdContract) { mIdContract = aIdContract; }
    void SetNum(int aNum) { mNum = aNum; }
    void SetMaxSumBrutto(qlonglong aMaxSumBrutto) { mMaxSumBrutto = aMaxSumBrutto; }

    int GetUpdateId() const { return UpdateId; }
protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    void Accept();
    void on_leSum_textEdited(const QString &arg1);

    void on_leNdsPerCent_textEdited(const QString &arg1);

    void on_leSumNds_textEdited(const QString &arg1);

    void on_leSumFull_textEdited(const QString &arg1);

    void on_dtStart_userDateChanged(const QDate &date);

private:
    Ui::ContractStageProp *ui;

    int mIdContract, mIdContractStage;
    int mNum;
    qlonglong mMaxSumBrutto;

    QDate origDate;
    QString origNum, origName;
    qlonglong origSumBrutto, origNdsPercent, origSumFull;
    QString origComments;

    bool isNewRecord;
    int UpdateId;
};

#endif // CONTRACTSTAGEPROP_H
