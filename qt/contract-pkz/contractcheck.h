#ifndef CONTRACTCHECK_H
#define CONTRACTCHECK_H

#include <QDate>
#include "../VProject/qfcdialog.h"

namespace Ui {
class ContractCheck;
}

class ContractCheck : public QFCDialog
{
    Q_OBJECT

public:
    explicit ContractCheck(QWidget *parent = 0);
    ~ContractCheck();

    void SetIdCheck(int aIdCheck) { mIdCheck = aIdCheck; }
    void SetIdContract(int aIdContract) { mIdContract = aIdContract; }
    void SetIdContractStage(int aIdContractStage) { mIdContractStage = aIdContractStage; }
    void SetContractDate(const QDate &aContractDate) { mContractDate = aContractDate; }

    void SetFullSum(qlonglong aFullSum) { mFullSum = aFullSum; }
    void SetMaxPercent(int aMaxPercent) { mMaxPercent = aMaxPercent; }
    void SetMaxSumBrutto(qlonglong aMaxSumBrutto) { mMaxSumBrutto = aMaxSumBrutto; }
    void SetIndexingType(int aIndexingType) { mIndexingType = aIndexingType; }
    void SetContractNum(const QString &aContractNum) { mContractNumForOut = aContractNum; }

    int GetUpdateId() const { return UpdateId; }
    int GetUpdateIdOther() const { return UpdateIdOther; }

protected:
    virtual void showEvent(QShowEvent* event);

private slots:
    void on_spinBox_valueChanged(int arg1);
    void Accept();

    void on_leSum_textEdited(const QString &arg1);

    void on_leNdsPerCent_textEdited(const QString &arg1);

    void on_leSumNds_textEdited(const QString &arg1);

    void on_gbSigned_toggled(bool arg1);

    void on_leSumFull_textEdited(const QString &arg1);

    void on_leSignSum_textEdited(const QString &arg1);

    void on_leSignNdsPerCent_textEdited(const QString &arg1);

    void on_leSignSumNds_textEdited(const QString &arg1);

    void on_leSignSumFull_textEdited(const QString &arg1);

    void on_toolButton_clicked();
    void on_leSum_customContextMenuRequested(const QPoint &pos);

    void on_actionCopy_URL_triggered();

    void on_leSumIndexed_textEdited(const QString &arg1);
    void on_leSumNdsIndexed_textEdited(const QString &arg1);

    void on_leSumFullIndexed_textEdited(const QString &arg1);

    void on_actionCopy_URLVat_triggered();

    void on_actionCopy_URLFull_triggered();

    void on_leSumNds_customContextMenuRequested(const QPoint &pos);

    void on_leSumFull_customContextMenuRequested(const QPoint &pos);

    void on_actionOpen_URL_triggered();

    void on_actionOpen_URLVat_triggered();

    void on_actionOpen_URLFull_triggered();

    void on_toolButton_2_clicked();

    void on_leSignSumIndexed_textEdited(const QString &arg1);

    void on_leSignSumNdsIndexed_textEdited(const QString &arg1);

    void on_leSignSumFullIndexed_textEdited(const QString &arg1);

    void on_lePaySum_textEdited(const QString &arg1);

    void on_lePayNdsPerCent_textEdited(const QString &arg1);

    void on_lePaySumNds_textEdited(const QString &arg1);

    void on_lePaySumFull_textEdited(const QString &arg1);

    void on_lePaySumIndexed_textEdited(const QString &arg1);

    void on_lePaySumNdsIndexed_textEdited(const QString &arg1);

    void on_lePaySumFullIndexed_textEdited(const QString &arg1);

    void on_toolButton_3_clicked();

    void on_gbPayed_toggled(bool arg1);

    void on_toolButton_4_clicked();

    void on_cbDeExpect_toggled(bool checked);

    void on_dePod_userDateChanged(const QDate &date);

    void on_deExpect_userDateChanged(const QDate &date);

    void on_deSignDate_userDateChanged(const QDate &date);

    void on_dePayDate_userDateChanged(const QDate &date);

private:
    Ui::ContractCheck *ui;

    QMap<QString, int> mStageList;

    int mIdCheck, mIdContract, mIdContractStage;
    QDate mContractDate;
    int mIndexingType;
    QString mContractNumForOut;

    qlonglong mFullSum, mMaxSumBrutto;
    int mMaxPercent;

    int origIdContract, origIdContractStage;
    //QString origStageNum;
    int origDonepercent;
    QDate origOrigDate, origExpectDate;
    QString origInvoice;
    qlonglong origOrigSumBrutto, origOrigSumFull, origOrigNdsPercent;
    qlonglong origOrigSumBruttoIndexed, origOrigSumFullIndexed;

    bool origSignDateIsNull;
    QDate origSignDate;
    qlonglong origSignSumBrutto, origSignSumFull, origSignNdsPercent;
    qlonglong origSignSumBruttoIndexed, origSignSumFullIndexed;

    bool origPayDateIsNull;
    QDate origPayDate;
    QString origPayInvoice;
    qlonglong origPaySumBrutto, origPaySumFull, origPayNdsPercent;
    qlonglong origPaySumBruttoIndexed, origPaySumFullIndexed;

    QString origComments;

    bool isNewRecord;
    int UpdateId, UpdateIdOther;
};

#endif // CONTRACTCHECK_H
