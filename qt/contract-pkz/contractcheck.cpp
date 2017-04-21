#include "contractcheck.h"
#include "ui_contractcheck.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/oracle.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QMenu>

ContractCheck::ContractCheck(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ContractCheck),
    mIdCheck(0), mIdContract(0), mIdContractStage(0),
    isNewRecord(false), UpdateId(0), UpdateIdOther(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    QDoubleValidator *vForSum = new QDoubleValidator(0, 1e12, 2, this);
    vForSum->setNotation(QDoubleValidator::StandardNotation);
    ui->leSumNds->setValidator(vForSum);
    ui->leSumFull->setValidator(vForSum);

    vForSum = new QDoubleValidator(0, 30, 2, this);
    vForSum->setNotation(QDoubleValidator::StandardNotation);
    ui->leNdsPerCent->setValidator(vForSum);
}

ContractCheck::~ContractCheck()
{
    gSettings->CalcMadadAbort();
    delete ui;
}

void ContractCheck::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    QString lStageNum;

    ui->spinBox->setMaximum(mMaxPercent);

    if (mIdCheck) {
        QSqlQuery query(
                    "SELECT NVL(ID_CONTRACT, 0) ID_CONTRACT, NVL(ID_CONTRACT_STAGE, 0) ID_CONTRACT_STAGE,"
                    " (SELECT TO_CHAR(ORDER_NUM) FROM V_PKZ_CONTRACT_STAGE WHERE ID = A.ID_CONTRACT_STAGE) STAGE_NUM,"
                    " CASE WHEN ID_CONTRACT IS NOT NULL"
                    "  THEN (SELECT NUM FROM V_PKZ_CONTRACT WHERE ID = A.ID_CONTRACT)"
                    "  ELSE (SELECT B.NUM FROM V_PKZ_CONTRACT B, V_PKZ_CONTRACT_STAGE C"
                    "          WHERE C.ID = A.ID_CONTRACT_STAGE AND B.ID = C.ID_PKZ_CONTRACT)"
                    " END CONTRACT_NUM,"
                    " DONEPERCENT, ORIG_DATE, EXPECT_DATE, INVOICE, ORIG_SUM_BRUTTO, ORIG_SUM_FULL, ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_FULL_INDEXED, ORIG_NDS_PERCENT,"
                    " SIGN_DATE, SIGN_SUM_BRUTTO, SIGN_SUM_FULL, SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_FULL_INDEXED, SIGN_NDS_PERCENT,"
                    " PAY_DATE, PAY_INVOICE, PAY_SUM_BRUTTO, PAY_SUM_FULL, PAY_SUM_BRUTTO_INDEXED, PAY_SUM_FULL_INDEXED, PAY_NDS_PERCENT,"
                    " COMMENTS FROM V_PKZ_HASHBON A"
                    " WHERE ID = " + QString::number(mIdCheck), db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, "חשבון", query);
        } else {
            if (query.next()) {
                origIdContract = query.value("ID_CONTRACT").toInt();
                origIdContractStage = query.value("ID_CONTRACT_STAGE").toInt();
                ui->leContract->setText(query.value("CONTRACT_NUM").toString());

                lStageNum = query.value("stage_num").toString();

                origDonepercent = query.value("DONEPERCENT").toInt();
                ui->spinBox->setValue(origDonepercent);

                origOrigDate = query.value("ORIG_DATE").toDate();
                ui->dePod->setDate(origOrigDate);

                origExpectDate = query.value("EXPECT_DATE").toDate();
                if (origExpectDate.isNull()) {
                    ui->cbDeExpect->setChecked(false);
                    ui->deExpect->setEnabled(false);
                    ui->deExpect->setDate(QDate::currentDate().addMonths(2));
                } else {
                    ui->cbDeExpect->setChecked(true);
                    ui->deExpect->setEnabled(true);
                    ui->deExpect->setDate(origExpectDate);
                }

                origInvoice = query.value("INVOICE").toString();
                ui->leInvoice->setText(origInvoice);

                origOrigSumBrutto = query.value("ORIG_SUM_BRUTTO").toLongLong();
                origOrigSumFull = query.value("ORIG_SUM_FULL").toLongLong();
                origOrigNdsPercent = query.value("ORIG_NDS_PERCENT").toLongLong();

                if (origOrigSumBrutto) ui->leSum->setText(gSettings->FormatSumForEdit(origOrigSumBrutto));
                if (origOrigNdsPercent) ui->leNdsPerCent->setText(gSettings->FormatNdsForEdit(origOrigNdsPercent));
                if (origOrigSumFull && origOrigSumBrutto) ui->leSumNds->setText(gSettings->FormatSumForEdit(origOrigSumFull - origOrigSumBrutto));
                if (origOrigSumFull) ui->leSumFull->setText(gSettings->FormatSumForEdit(origOrigSumFull));

                origOrigSumBruttoIndexed = query.value("ORIG_SUM_BRUTTO_INDEXED").toLongLong();
                origOrigSumFullIndexed = query.value("ORIG_SUM_FULL_INDEXED").toLongLong();
                if (origOrigSumBruttoIndexed)
                    ui->leSumIndexed->setText(gSettings->FormatSumForEdit(origOrigSumBruttoIndexed));
                if (origOrigSumBruttoIndexed && origOrigSumFullIndexed)
                    ui->leSumNdsIndexed->setText(gSettings->FormatSumForEdit(origOrigSumFullIndexed - origOrigSumBruttoIndexed));
                if (origOrigSumFullIndexed)
                    ui->leSumFullIndexed->setText(gSettings->FormatSumForEdit(origOrigSumFullIndexed));


                if (!query.value("SIGN_DATE").isNull()) {
                    origSignDateIsNull = false;
                    origSignDate = query.value("SIGN_DATE").toDate();
                    ui->deSignDate->setDate(origSignDate);

                    origSignSumBrutto = query.value("SIGN_SUM_BRUTTO").toLongLong();
                    origSignSumFull = query.value("SIGN_SUM_FULL").toLongLong();
                    origSignNdsPercent = query.value("SIGN_NDS_PERCENT").toLongLong();

                    if (origSignSumBrutto) ui->leSignSum->setText(gSettings->FormatSumForEdit(origSignSumBrutto));
                    if (origSignNdsPercent) ui->leSignNdsPerCent->setText(gSettings->FormatNdsForEdit(origSignNdsPercent));
                    if (origSignSumFull && origSignSumBrutto) ui->leSignSumNds->setText(gSettings->FormatSumForEdit(origSignSumFull - origSignSumBrutto));
                    if (origSignSumFull) ui->leSignSumFull->setText(gSettings->FormatSumForEdit(origSignSumFull));

                    origSignSumBruttoIndexed = query.value("SIGN_SUM_BRUTTO_INDEXED").toLongLong();
                    origSignSumFullIndexed = query.value("SIGN_SUM_FULL_INDEXED").toLongLong();
                    if (origSignSumBruttoIndexed)
                        ui->leSignSumIndexed->setText(gSettings->FormatSumForEdit(origSignSumBruttoIndexed));
                    if (origSignSumBruttoIndexed && origSignSumFullIndexed)
                        ui->leSignSumNdsIndexed->setText(gSettings->FormatSumForEdit(origSignSumFullIndexed - origSignSumBruttoIndexed));
                    if (origSignSumFullIndexed)
                        ui->leSignSumFullIndexed->setText(gSettings->FormatSumForEdit(origSignSumFullIndexed));
                } else {
                    origSignDateIsNull = true;
                    ui->gbSigned->setChecked(false);
                }

                if (!query.value("PAY_DATE").isNull()) {
                    ui->lblDeExpect->setVisible(false);
                    ui->cbDeExpect->setVisible(false);
                    ui->deExpect->setVisible(false);

                    origPayDateIsNull = false;
                    origPayDate = query.value("PAY_DATE").toDate();
                    ui->dePayDate->setDate(origPayDate);

                    origPayInvoice = query.value("PAY_INVOICE").toString();
                    ui->lePayInvoice->setText(origPayInvoice);

                    origPaySumBrutto = query.value("PAY_SUM_BRUTTO").toLongLong();
                    origPaySumFull = query.value("PAY_SUM_FULL").toLongLong();
                    origPayNdsPercent = query.value("PAY_NDS_PERCENT").toLongLong();

                    if (origPaySumBrutto) ui->lePaySum->setText(gSettings->FormatSumForEdit(origPaySumBrutto));
                    if (origPayNdsPercent) ui->lePayNdsPerCent->setText(gSettings->FormatNdsForEdit(origPayNdsPercent));
                    if (origPaySumFull && origPaySumBrutto) ui->lePaySumNds->setText(gSettings->FormatSumForEdit(origPaySumFull - origPaySumBrutto));
                    if (origPaySumFull) ui->lePaySumFull->setText(gSettings->FormatSumForEdit(origPaySumFull));

                    origPaySumBruttoIndexed = query.value("PAY_SUM_BRUTTO_INDEXED").toLongLong();
                    origPaySumFullIndexed = query.value("PAY_SUM_FULL_INDEXED").toLongLong();
                    if (origPaySumBruttoIndexed)
                        ui->lePaySumIndexed->setText(gSettings->FormatSumForEdit(origPaySumBruttoIndexed));
                    if (origPaySumBruttoIndexed && origPaySumFullIndexed)
                        ui->lePaySumNdsIndexed->setText(gSettings->FormatSumForEdit(origPaySumFullIndexed - origPaySumBruttoIndexed));
                    if (origPaySumFullIndexed)
                        ui->lePaySumFullIndexed->setText(gSettings->FormatSumForEdit(origPaySumFullIndexed));
                } else {
                    ui->lblDeExpect->setVisible(true);
                    ui->cbDeExpect->setVisible(true);
                    ui->deExpect->setVisible(true);

                    origPayDateIsNull = true;
                    ui->gbPayed->setChecked(false);
                }

                origComments = query.value("COMMENTS").toString();
                ui->ptComments->setPlainText(origComments);
            } else {
                QMessageBox::critical(this, "חשבון",
                                      QString("חשבון id = ") + QString::number(mIdCheck) + " לא קיימ!");
            }
        }
        isNewRecord = false;
        setWindowTitle("חשבון");
    } else {
        // new
        origIdContractStage = mIdContractStage;

        ui->lblDeExpect->setVisible(true);
        ui->cbDeExpect->setVisible(true);
        ui->cbDeExpect->setChecked(false);
        ui->deExpect->setEnabled(false);
        ui->deExpect->setVisible(true);
        ui->deExpect->setDate(QDate::currentDate().addMonths(2));

        ui->dePod->setDate(QDate::currentDate());
        ui->leNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(ui->dePod->date())));

        ui->gbSigned->setChecked(false);
        ui->deSignDate->setDate(QDate::currentDate());

        ui->gbPayed->setChecked(false);
        ui->dePayDate->setDate(QDate::currentDate());

        //ui->spinBox->setValue(mMaxPercent);
        //on_spinBox_valueChanged(ui->spinBox->value());
        ui->leSum->setText(gSettings->FormatSumForEdit(mMaxSumBrutto));
        on_leSum_textEdited("");

        isNewRecord = true;
        setWindowTitle("חשבון חדש");
    }


    if (origIdContractStage) {
        // stage list exists
        QSqlQuery queryStages("select a.order_num, a.id from v_pkz_contract_stage a"
                              " where a.id_pkz_contract ="
                              " (select id_pkz_contract from v_pkz_contract_stage where id = " + QString::number(origIdContractStage) + ")"
                              " order by order_num", db);

        mStageList.clear();
        ui->cbStage->clear();

        if (queryStages.lastError().isValid()) {
            gLogger->ShowSqlError(this, "חשבון", queryStages);
        } else {
            while (queryStages.next()) {
                mStageList.insert(queryStages.value("order_num").toString(), queryStages.value("id").toInt());
                ui->cbStage->addItem(queryStages.value("order_num").toString());
                if (origIdContractStage == queryStages.value("id").toInt()) {
                    ui->cbStage->setCurrentIndex(ui->cbStage->count() - 1);
                    if (lStageNum.isEmpty()) lStageNum = queryStages.value("order_num").toString();
                }
            }
        }

        if (mStageList.count() > 1) {
            ui->cbStage->setVisible(true);
            ui->leStageSimple->setVisible(false);
        } else {
            // something wrong with stage list
            ui->cbStage->setVisible(false);
            ui->leStageSimple->setText(lStageNum);
            ui->leStageSimple->setVisible(true);
        }
        ui->labelStage->setVisible(true);
    } else {
        // no stages, bound directly to contract
        ui->cbStage->setVisible(false);
        ui->leStageSimple->setVisible(false);
        ui->labelStage->setVisible(false);

    }
    if (ui->leContract->text().isEmpty())
        ui->leContract->setText(mContractNumForOut);



    QDoubleValidator *v = new QDoubleValidator(0, ((double) mMaxSumBrutto) / 100.0, 2, this);
    v->setNotation(QDoubleValidator::StandardNotation);
    ui->leSum->setValidator(v);
}


void ContractCheck::on_spinBox_valueChanged(int arg1) {
    // now it is in reverse - user edit money amount and percent will be calculated

//    qlonglong lSum = (5000L + ((qlonglong) arg1 * 100) * mSum) / 10000L;

//    if (lSum > mMaxSumBrutto) lSum = mMaxSumBrutto;
//    ui->leSum->setText(gSettings->FormatSumForEdit(lSum));
//    on_leSum_textEdited("");
}

void ContractCheck::Accept() {
    bool lDoAccept = false;
    QString lFields;
    QList <QVariant> lValues;
    int newIdContractStage;

    //tr(QT_TR_NOOP("Error"));

    // there is no amount in some records
//    if (ui->leSum->text().isEmpty()
//            || ui->leNdsPerCent->text().isEmpty()
//            || ui->leSumNds->text().isEmpty()
//            || ui->leSumFull->text().isEmpty()) {
//        QMessageBox::critical(this, "חוזה", "Amount must be specified!");
//        ui->leSum->setFocus();
//        return;
//    }

    if (ui->cbStage->isVisible()) {
        QMap<QString, int>::const_iterator itr = mStageList.find(ui->cbStage->currentText());
        if (itr != mStageList.end() && itr.key() == ui->cbStage->currentText()) {
            newIdContractStage = itr.value();
        } else {
            QMessageBox::critical(this, "חוזה", "Stage " + ui->cbStage->currentText() + " doesn't exist!");
            ui->cbStage->setFocus();
            return;
        }
    } else {
        newIdContractStage = origIdContractStage;
    }

    if (gSettings->GetSumFromEdit(ui->lePaySum->text()).toLongLong() > mMaxSumBrutto) {
        QMessageBox::critical(this, "חוזה", "Maximum amount is " + gSettings->FormatSumForEdit(mMaxSumBrutto) + "!");
        ui->lePaySum->setFocus();
        return;
    }

    if (ui->lePaySum->text().isEmpty()
            && gSettings->GetSumFromEdit(ui->leSignSum->text()).toLongLong() > mMaxSumBrutto) {
        QMessageBox::critical(this, "חוזה", "Maximum amount is " + gSettings->FormatSumForEdit(mMaxSumBrutto) + "!");
        ui->leSignSum->setFocus();
        return;
    }

    if (isNewRecord) {
        // try to insert new record
        QString lPH;

        if (ui->lePaySum->text().isEmpty()
                && ui->leSignSum->text().isEmpty()
                && gSettings->GetSumFromEdit(ui->leSum->text()).toLongLong() > mMaxSumBrutto) {
            QMessageBox::critical(this, "חוזה", "Maximum amount is " + gSettings->FormatSumForEdit(mMaxSumBrutto) + "!");
            ui->leSum->setFocus();
            return;
        }

        if (mIdContractStage) {
            lFields += "id_contract_stage, ";
            lPH += "?, ";
            lValues.append(newIdContractStage);
        } else {
            lFields = "id_contract, ";
            lPH = "?, ";
            lValues.append(mIdContract);
        }

        lFields += "donepercent, ";
        lPH += "?, ";
        lValues.append(ui->spinBox->value());

        lFields += "orig_date, ";
        lPH += "?, ";
        lValues.append(ui->dePod->date());

        if (!ui->gbPayed->isChecked()
                && ui->cbDeExpect->isChecked()) {
            lFields += "expect_date, ";
            lPH += "?, ";
            lValues.append(ui->deExpect->date());
        }

        lFields += "invoice, ";
        lPH += "?, ";
        lValues.append(ui->leInvoice->text());

        lFields += "orig_sum_brutto, ";
        lPH += "?, ";
        lValues.append(gSettings->GetSumFromEdit(ui->leSum->text()));

        lFields += "orig_nds_percent, ";
        lPH += "?, ";
        lValues.append(gSettings->GetSumFromEdit(ui->leNdsPerCent->text()));

        lFields += "orig_sum_full, ";
        lPH += "?, ";
        lValues.append(gSettings->GetSumFromEdit(ui->leSumFull->text()));

        lFields += "orig_sum_brutto_indexed, ";
        lPH += "?, ";
        lValues.append(gSettings->GetSumFromEdit(ui->leSumIndexed->text()));

        lFields += "orig_sum_full_indexed, ";
        lPH += "?, ";
        lValues.append(gSettings->GetSumFromEdit(ui->leSumFullIndexed->text()));

        if (ui->gbSigned->isChecked()) {
            lFields += "sign_date, ";
            lPH += "?, ";
            lValues.append(ui->deSignDate->date());

            lFields += "sign_sum_brutto, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSignSum->text()));

            lFields += "sign_nds_percent, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSignNdsPerCent->text()));

            lFields += "sign_sum_full, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSignSumFull->text()));

            lFields += "sign_sum_brutto_indexed, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSignSumIndexed->text()));

            lFields += "sign_sum_full_indexed, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSignSumFullIndexed->text()));
        }

        if (ui->gbPayed->isChecked()) {
            lFields += "pay_date, ";
            lPH += "?, ";
            lValues.append(ui->dePayDate->date());

            lFields += "pay_invoice, ";
            lPH += "?, ";
            lValues.append(ui->lePayInvoice->text());

            lFields += "pay_sum_brutto, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->lePaySum->text()));

            lFields += "pay_nds_percent, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->lePayNdsPerCent->text()));

            lFields += "pay_sum_full, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->lePaySumFull->text()));

            lFields += "pay_sum_brutto_indexed, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->lePaySumIndexed->text()));

            lFields += "pay_sum_full_indexed, ";
            lPH += "?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->lePaySumFullIndexed->text()));
        }

        lFields += "comments";
        lPH += "?";
        lValues.append(ui->ptComments->toPlainText());

        QSqlQuery qInsert(db);

        qInsert.prepare(QString("INSERT INTO V_PKZ_HASHBON(") + lFields + ") VALUES (" + lPH + ")");
        for (int i = 0; i < lValues.length(); i++)
            qInsert.addBindValue(lValues.at(i));

        if (!qInsert.exec()) {
            gLogger->ShowSqlError(this, "חשבון", qInsert);
        } else {
            if (gOracle->GetSeqCurVal("SEQ_PKZ_HASHBON_ID", UpdateId)) {
                lDoAccept = true;
            }
        }
    } else {
        // try to update existing record

        if (ui->cbStage->isVisible()) {
            if (origIdContractStage != newIdContractStage) {
                lFields += "id_contract_stage = ?, ";
                lValues.append(newIdContractStage);
                UpdateIdOther = newIdContractStage;
            }
        }

        if (origDonepercent != ui->spinBox->value()) {
            lFields += "donepercent = ?, ";
            lValues.append(ui->spinBox->value());
        }

        if (origOrigDate != ui->dePod->date()) {
            lFields += "orig_date = ?, ";
            lValues.append(ui->dePod->date());
        }

        if (!ui->gbPayed->isChecked()) {
            if (ui->cbDeExpect->isChecked()) {
                if (origExpectDate != ui->deExpect->date()) {
                    lFields += "expect_date = ?, ";
                    lValues.append(ui->deExpect->date());
                }
            } else {
                lFields += "expect_date = null, ";
            }
        }

        if (origInvoice != ui->leInvoice->text()) {
            lFields += "invoice = ?, ";
            lValues.append(ui->leInvoice->text());
        }

        if (origOrigSumBrutto != gSettings->GetSumFromEdit(ui->leSum->text())) {
            lFields += "orig_sum_brutto = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSum->text()));
        }

        if (origOrigSumFull != gSettings->GetSumFromEdit(ui->leSumFull->text())) {
            lFields += "orig_sum_full = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSumFull->text()));
        }

        if (origOrigNdsPercent != gSettings->GetSumFromEdit(ui->leNdsPerCent->text())) {
            lFields += "orig_nds_percent = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leNdsPerCent->text()));
        }

        if (origOrigSumBruttoIndexed != gSettings->GetSumFromEdit(ui->leSumIndexed->text())) {
            lFields += "orig_sum_brutto_indexed = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSumIndexed->text()));
        }

        if (origOrigSumFullIndexed != gSettings->GetSumFromEdit(ui->leSumFullIndexed->text())) {
            lFields += "orig_sum_full_indexed = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSumFullIndexed->text()));
        }

        if (ui->gbSigned->isChecked()) {
            if (origSignDate != ui->deSignDate->date() || origSignDateIsNull) {
                lFields += "sign_date = ?, ";
                lValues.append(ui->deSignDate->date());
            }

            if (origSignSumBrutto != gSettings->GetSumFromEdit(ui->leSignSum->text()) || origSignDateIsNull) {
                lFields += "sign_sum_brutto = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->leSignSum->text()));
            }

            if (origSignSumFull != gSettings->GetSumFromEdit(ui->leSignSumFull->text()) || origSignDateIsNull) {
                lFields += "sign_sum_full = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->leSignSumFull->text()));
            }

            if (origSignNdsPercent != gSettings->GetSumFromEdit(ui->leSignNdsPerCent->text()) || origSignDateIsNull) {
                lFields += "sign_nds_percent = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->leSignNdsPerCent->text()));
            }

            if (origSignSumBruttoIndexed != gSettings->GetSumFromEdit(ui->leSignSumIndexed->text()) || origSignDateIsNull) {
                lFields += "sign_sum_brutto_indexed = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->leSignSumIndexed->text()));
            }

            if (origSignSumFullIndexed != gSettings->GetSumFromEdit(ui->leSignSumFullIndexed->text()) || origSignDateIsNull) {
                lFields += "sign_sum_full_indexed = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->leSignSumFullIndexed->text()));
            }
        } else if (!origSignDateIsNull) {
            lFields += "sign_date = null, sign_sum_brutto = null, sign_sum_full = null, sign_nds_percent = null, sign_sum_brutto_indexed = null, sign_sum_full_indexed = null, ";
        }

        if (ui->gbPayed->isChecked()) {
            if (origPayDate != ui->dePayDate->date() || origPayDateIsNull) {
                lFields += "pay_date = ?, ";
                lValues.append(ui->dePayDate->date());
            }

            if (origPayInvoice != ui->lePayInvoice->text() || origPayDateIsNull) {
                lFields += "pay_invoice = ?, ";
                lValues.append(ui->lePayInvoice->text());
            }

            if (origPaySumBrutto != gSettings->GetSumFromEdit(ui->lePaySum->text()) || origPayDateIsNull) {
                lFields += "pay_sum_brutto = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->lePaySum->text()));
            }

            if (origPaySumFull != gSettings->GetSumFromEdit(ui->lePaySumFull->text()) || origPayDateIsNull) {
                lFields += "pay_sum_full = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->lePaySumFull->text()));
            }

            if (origPayNdsPercent != gSettings->GetSumFromEdit(ui->lePayNdsPerCent->text()) || origPayDateIsNull) {
                lFields += "pay_nds_percent = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->lePayNdsPerCent->text()));
            }

            if (origPaySumBruttoIndexed != gSettings->GetSumFromEdit(ui->lePaySumIndexed->text()) || origPayDateIsNull) {
                lFields += "pay_sum_brutto_indexed = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->lePaySumIndexed->text()));
            }

            if (origPaySumFullIndexed != gSettings->GetSumFromEdit(ui->lePaySumFullIndexed->text()) || origPayDateIsNull) {
                lFields += "pay_sum_full_indexed = ?, ";
                lValues.append(gSettings->GetSumFromEdit(ui->lePaySumFullIndexed->text()));
            }
        } else if (!origPayDateIsNull) {
            lFields += "pay_date = null, pay_invoice = null, pay_sum_brutto = null, pay_sum_full = null, pay_nds_percent = null, pay_sum_brutto_indexed = null, pay_sum_full_indexed = null, ";
        }

        if (origComments != ui->ptComments->toPlainText()) {
            lFields += "comments = ?, ";
            lValues.append(ui->ptComments->toPlainText());
        }

        if (lFields.length()) {
            lFields.truncate(lFields.length() - 2);

            QSqlQuery qUpdate(db);

            qUpdate.prepare(QString("UPDATE V_PKZ_HASHBON SET ") + lFields + " WHERE ID = ?");
            for (int i = 0; i < lValues.length(); i++)
                qUpdate.addBindValue(lValues.at(i));
            qUpdate.addBindValue(mIdCheck);

            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(this, "חשבון", qUpdate);
            } else {
                UpdateId = mIdCheck;
                lDoAccept = true;
            }
        } else {
            // nothing changed, just close
            //lDoAccept = true;
        }
    }

    if (lDoAccept) accept();
}

void ContractCheck::on_leSum_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndFull(ui->leSum, ui->leNdsPerCent, ui->leSumNds, ui->leSumFull);

    ui->spinBox->setValue(((gSettings->GetSumFromEdit(ui->leSum->text()).toLongLong() * (qlonglong) 100)  + (mFullSum / 2)) / mFullSum);
}

void ContractCheck::on_leNdsPerCent_textEdited(const QString &arg1) {
    on_leSum_textEdited("");
    on_leSumIndexed_textEdited("");
}

void ContractCheck::on_leSumNds_textEdited(const QString &arg1) {
    gSettings->CalcFullWhenNdsSumChanged(ui->leSum, ui->leSumNds, ui->leSumFull);
}

void ContractCheck::on_gbSigned_toggled(bool arg1) {
    if (arg1) {
        ui->deSignDate->setDate(QDate::currentDate());

        ui->leSignSum->setText(ui->leSum->text());
        ui->leSignNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(ui->deSignDate->date())));
        ui->leSignSumNds->setText(ui->leSumNds->text());
        ui->leSignSumFull->setText(ui->leSumFull->text());

        ui->leSignSumIndexed->setText(ui->leSumIndexed->text());
        ui->leSignSumNdsIndexed->setText(ui->leSumNdsIndexed->text());
        ui->leSignSumFullIndexed->setText(ui->leSumFullIndexed->text());

        //        ui->deSignDate->setDate(QDate::currentDate());
//        ui->leSignSum->setText(ui->leSum->text());
//        ui->leSignNdsPerCent->setText(gSettings->FormatNumber(gSettings->NDS));
//        on_leSignSum_textEdited("");
//        ui->leSignSumIndexed->setText(ui->leSumIndexed->text());
//        on_leSignSumIndexed_textEdited("");
    }
}

void ContractCheck::on_leSumFull_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->leSumFull, ui->leNdsPerCent, ui->leSumNds, ui->leSum);
}

void ContractCheck::on_leSignSum_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndFull(ui->leSignSum, ui->leSignNdsPerCent, ui->leSignSumNds, ui->leSignSumFull);
}

void ContractCheck::on_leSignNdsPerCent_textEdited(const QString &arg1) {
    on_leSignSum_textEdited("");
    on_leSignSumIndexed_textEdited("");
}

void ContractCheck::on_leSignSumNds_textEdited(const QString &arg1) {
    gSettings->CalcFullWhenNdsSumChanged(ui->leSignSum, ui->leSignSumNds, ui->leSignSumFull);
}

void ContractCheck::on_leSignSumFull_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->leSignSumFull, ui->leSignNdsPerCent, ui->leSignSumNds, ui->leSignSum);
}

void ContractCheck::on_toolButton_clicked() {
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->dePod->date(), ui->leSum->text(), ui->leSumIndexed);
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->dePod->date(), ui->leSumNds->text(), ui->leSumNdsIndexed);
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->dePod->date(), ui->leSumFull->text(), ui->leSumFullIndexed);
}

void ContractCheck::on_leSum_customContextMenuRequested(const QPoint &pos) {
    QMenu *popupMenu = ui->leSum->createStandardContextMenu();
    popupMenu->insertAction(popupMenu->actions().at(0), ui->actionCopy_URL);
    popupMenu->insertAction(popupMenu->actions().at(1), ui->actionOpen_URL);
    popupMenu->insertSeparator(popupMenu->actions().at(2));
    popupMenu->exec(QCursor::pos());
    delete popupMenu;
}

void ContractCheck::on_actionCopy_URL_triggered() {
    QString lUrl;
    gSettings->CalcMadadFromSiteGetUrl(mIndexingType, mContractDate, ui->dePod->date(), ui->leSum->text(), lUrl);
    QApplication::clipboard()->setText(lUrl);
}

void ContractCheck::on_leSumIndexed_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndFull(ui->leSumIndexed, ui->leNdsPerCent, ui->leSumNdsIndexed, ui->leSumFullIndexed);
}

void ContractCheck::on_leSumNdsIndexed_textEdited(const QString &arg1) {
    gSettings->CalcFullWhenNdsSumChanged(ui->leSumIndexed, ui->leSumNdsIndexed, ui->leSumFullIndexed);
}

void ContractCheck::on_leSumFullIndexed_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->leSumFullIndexed, ui->leNdsPerCent, ui->leSumNdsIndexed, ui->leSumIndexed);
}

void ContractCheck::on_actionCopy_URLVat_triggered() {
    QString lUrl;
    gSettings->CalcMadadFromSiteGetUrl(mIndexingType, mContractDate, ui->dePod->date(), ui->leSumNds->text(), lUrl);
    QApplication::clipboard()->setText(lUrl);
}

void ContractCheck::on_actionCopy_URLFull_triggered() {
    QString lUrl;
    gSettings->CalcMadadFromSiteGetUrl(mIndexingType, mContractDate, ui->dePod->date(), ui->leSumFull->text(), lUrl);
    QApplication::clipboard()->setText(lUrl);
}

void ContractCheck::on_leSumNds_customContextMenuRequested(const QPoint &pos) {
    QMenu *popupMenu = ui->leSumNds->createStandardContextMenu();
    popupMenu->insertAction(popupMenu->actions().at(0), ui->actionCopy_URLVat);
    popupMenu->insertAction(popupMenu->actions().at(1), ui->actionOpen_URLVat);
    popupMenu->insertSeparator(popupMenu->actions().at(2));
    popupMenu->exec(QCursor::pos());
    delete popupMenu;
}

void ContractCheck::on_leSumFull_customContextMenuRequested(const QPoint &pos) {
    QMenu *popupMenu = ui->leSumFull->createStandardContextMenu();
    popupMenu->insertAction(popupMenu->actions().at(0), ui->actionCopy_URLFull);
    popupMenu->insertAction(popupMenu->actions().at(1), ui->actionOpen_URLFull);
    popupMenu->insertSeparator(popupMenu->actions().at(2));
    popupMenu->exec(QCursor::pos());
    delete popupMenu;
}

void ContractCheck::on_actionOpen_URL_triggered() {
    QString lUrl;
    gSettings->CalcMadadFromSiteGetUrl(mIndexingType, mContractDate, ui->dePod->date(), ui->leSum->text(), lUrl);
    QDesktopServices::openUrl(QUrl(lUrl));
}

void ContractCheck::on_actionOpen_URLVat_triggered() {
    QString lUrl;
    gSettings->CalcMadadFromSiteGetUrl(mIndexingType, mContractDate, ui->dePod->date(), ui->leSumNds->text(), lUrl);
    QDesktopServices::openUrl(QUrl(lUrl));
}

void ContractCheck::on_actionOpen_URLFull_triggered() {
    QString lUrl;
    gSettings->CalcMadadFromSiteGetUrl(mIndexingType, mContractDate, ui->dePod->date(), ui->leSumFull->text(), lUrl);
    QDesktopServices::openUrl(QUrl(lUrl));
}

void ContractCheck::on_toolButton_2_clicked() {
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->deSignDate->date(), ui->leSignSum->text(), ui->leSignSumIndexed);
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->deSignDate->date(), ui->leSignSumNds->text(), ui->leSignSumNdsIndexed);
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->deSignDate->date(), ui->leSignSumFull->text(), ui->leSignSumFullIndexed);
}

void ContractCheck::on_leSignSumIndexed_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndFull(ui->leSignSumIndexed, ui->leSignNdsPerCent, ui->leSignSumNdsIndexed, ui->leSignSumFullIndexed);
}

void ContractCheck::on_leSignSumNdsIndexed_textEdited(const QString &arg1)
{
    gSettings->CalcFullWhenNdsSumChanged(ui->leSignSumIndexed, ui->leSignSumNdsIndexed, ui->leSignSumFullIndexed);
}

void ContractCheck::on_leSignSumFullIndexed_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->leSignSumFullIndexed, ui->leSignNdsPerCent, ui->leSignSumNdsIndexed, ui->leSignSumIndexed);
}

void ContractCheck::on_lePaySum_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndFull(ui->lePaySum, ui->lePayNdsPerCent, ui->lePaySumNds, ui->lePaySumFull);
}

void ContractCheck::on_lePayNdsPerCent_textEdited(const QString &arg1) {
    on_lePaySum_textEdited("");
    on_lePaySumIndexed_textEdited("");

}

void ContractCheck::on_lePaySumNds_textEdited(const QString &arg1) {
    gSettings->CalcFullWhenNdsSumChanged(ui->lePaySum, ui->lePaySumNds, ui->lePaySumFull);
}

void ContractCheck::on_lePaySumFull_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->lePaySumFull, ui->lePayNdsPerCent, ui->lePaySumNds, ui->lePaySum);
}

void ContractCheck::on_lePaySumIndexed_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndFull(ui->lePaySumIndexed, ui->lePayNdsPerCent, ui->lePaySumNdsIndexed, ui->lePaySumFullIndexed);
}

void ContractCheck::on_lePaySumNdsIndexed_textEdited(const QString &arg1) {
    gSettings->CalcFullWhenNdsSumChanged(ui->lePaySumIndexed, ui->lePaySumNdsIndexed, ui->lePaySumFullIndexed);
}

void ContractCheck::on_lePaySumFullIndexed_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->lePaySumFullIndexed, ui->lePayNdsPerCent, ui->lePaySumNdsIndexed, ui->lePaySumIndexed);
}

void ContractCheck::on_toolButton_3_clicked() {
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->dePayDate->date(), ui->lePaySum->text(), ui->lePaySumIndexed);
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->dePayDate->date(), ui->lePaySumNds->text(), ui->lePaySumNdsIndexed);
    gSettings->CalcMadadFromSite(mIndexingType, mContractDate, ui->dePayDate->date(), ui->lePaySumFull->text(), ui->lePaySumFullIndexed);
}

void ContractCheck::on_gbPayed_toggled(bool arg1) {
    ui->lblDeExpect->setVisible(!arg1);
    ui->cbDeExpect->setVisible(!arg1);
    ui->deExpect->setVisible(!arg1);
    if (arg1) {
        ui->dePayDate->setDate(QDate::currentDate());
        ui->lePayNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(ui->dePayDate->date())));
        if (ui->leSignSum->text().length()) {
            ui->lePaySum->setText(ui->leSignSum->text());
            ui->lePaySumNds->setText(ui->leSignSumNds->text());
            ui->lePaySumFull->setText(ui->leSignSumFull->text());

            ui->lePaySumIndexed->setText(ui->leSignSumIndexed->text());
            ui->lePaySumNdsIndexed->setText(ui->leSignSumNdsIndexed->text());
            ui->lePaySumFullIndexed->setText(ui->leSignSumFullIndexed->text());
        } else {
            ui->lePaySum->setText(ui->leSum->text());
            ui->lePaySumNds->setText(ui->leSumNds->text());
            ui->lePaySumFull->setText(ui->leSumFull->text());

            ui->lePaySumIndexed->setText(ui->leSumIndexed->text());
            ui->lePaySumNdsIndexed->setText(ui->leSumNdsIndexed->text());
            ui->lePaySumFullIndexed->setText(ui->leSumFullIndexed->text());
        }
//        if (ui->leSignSum->text().length()) {
//            ui->lePaySum->setText(ui->leSignSum->text());
//            ui->lePaySumIndexed->setText(ui->leSignSumIndexed->text());
//        } else {
//            ui->lePaySumIndexed->setText(ui->leSumIndexed->text());
//        }
//        ui->lePayNdsPerCent->setText(gSettings->FormatNumber(gSettings->NDS));
//        on_lePaySum_textEdited("");
//        on_lePaySumIndexed_textEdited("");
    }
}

void ContractCheck::on_toolButton_4_clicked() {
    gSettings->CalcMadadAbort();
}

void ContractCheck::on_cbDeExpect_toggled(bool checked) {
    ui->deExpect->setEnabled(checked);
    ui->leNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(checked?ui->deExpect->date():ui->dePod->date())));
    on_leNdsPerCent_textEdited(""); // param unused in fact
}

void ContractCheck::on_dePod_userDateChanged(const QDate &date) {
    if (!ui->cbDeExpect->isVisible() || !ui->cbDeExpect->isChecked()) {
        ui->leNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(date)));
        on_leNdsPerCent_textEdited(""); // param unused in fact
    }
}

void ContractCheck::on_deExpect_userDateChanged(const QDate &date) {
    if (ui->cbDeExpect->isVisible() && ui->cbDeExpect->isChecked()) {
        ui->leNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(date)));
        on_leNdsPerCent_textEdited(""); // param unused in fact
    }
}

void ContractCheck::on_deSignDate_userDateChanged(const QDate &date) {
    ui->leSignNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(date)));
    on_leSignNdsPerCent_textEdited(""); // param unused in fact
}

void ContractCheck::on_dePayDate_userDateChanged(const QDate &date) {
    ui->lePayNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(date)));
    on_lePayNdsPerCent_textEdited(""); // param unused in fact
}
