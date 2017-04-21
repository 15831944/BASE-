#include "contractstageprop.h"
#include "ui_contractstageprop.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/oracle.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

ContractStageProp::ContractStageProp(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ContractStageProp),
    mIdContract(0), mIdContractStage(0), mNum(0),
    isNewRecord(false), UpdateId(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    QPalette lPalette = ui->leNum->palette();
    // required
    lPalette.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    ui->leNum->setPalette(lPalette);
    ui->leSum->setPalette(lPalette);
    ui->leNdsPerCent->setPalette(lPalette);
    ui->leSumNds->setPalette(lPalette);
    ui->leSumFull->setPalette(lPalette);


    ui->leNum->setValidator(new QIntValidator(1, 99, this));

    QDoubleValidator *vForSum = new QDoubleValidator(0, 1e12, 2, this);
    vForSum->setNotation(QDoubleValidator::StandardNotation);
    ui->leSumNds->setValidator(vForSum);
    ui->leSumFull->setValidator(vForSum);

    vForSum = new QDoubleValidator(0, 30, 2, this);
    vForSum->setNotation(QDoubleValidator::StandardNotation);
    ui->leNdsPerCent->setValidator(vForSum);
}

ContractStageProp::~ContractStageProp()
{
    delete ui;
}

void ContractStageProp::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mIdContractStage) {
        QSqlQuery query(
                    "SELECT ORDER_NUM, START_DATE, NAME,"
                    " SUM_BRUTTO, NDS_PERCENT, SUM_FULL, COMMENTS FROM V_PKZ_CONTRACT_STAGE"
                    " WHERE ID = " + QString::number(mIdContractStage), db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, "שלב", query);
        } else {
            if (query.next()) {
                origNum = query.value("ORDER_NUM").toString();
                ui->leNum->setText(origNum);

                origDate = query.value("START_DATE").toDate();
                ui->dtStart->setDate(origDate);

                origName = query.value("NAME").toString();
                ui->leName->setText(origName);

                origSumBrutto = query.value("SUM_BRUTTO").toLongLong();
                origNdsPercent = query.value("NDS_PERCENT").toLongLong();
                origSumFull = query.value("SUM_FULL").toLongLong();

                ui->leSum->setText(gSettings->FormatSumForEdit(origSumBrutto));
                ui->leNdsPerCent->setText(gSettings->FormatNdsForEdit(origNdsPercent));
                ui->leSumNds->setText(gSettings->FormatSumForEdit(origSumFull - origSumBrutto));
                ui->leSumFull->setText(gSettings->FormatSumForEdit(origSumFull));

                origComments = query.value("COMMENTS").toString();
                ui->ptComments->setPlainText(origComments);
            } else {
                QMessageBox::critical(this, "שלב",
                                      QString("שלב id = ") + QString::number(mIdContractStage) + " לא קיימ!");
            }
        }
        isNewRecord = false;
        setWindowTitle("שלב");
    } else {
        ui->leNum->setText(QString::number(mNum));
        ui->dtStart->setDate(QDate::currentDate());
        ui->leSum->setText(gSettings->FormatSumForEdit(mMaxSumBrutto));
        ui->leNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(ui->dtStart->date())));
        on_leSum_textEdited("");

        ui->leName->setFocus();

        isNewRecord = true;
        setWindowTitle("שלב חדש");
    }

    QDoubleValidator *v = new QDoubleValidator(0, ((double) mMaxSumBrutto) / 100.0, 2, this);
    v->setNotation(QDoubleValidator::StandardNotation);
    ui->leSum->setValidator(v);
}

void ContractStageProp::Accept() {
    bool lDoAccept = false;
    QString lFields;
    QList <QVariant> lValues;

    //tr(QT_TR_NOOP("Error"));

    if (ui->leNum->text().isEmpty()) {
        QMessageBox::critical(this, "שלב", "Number must be specified!");
        ui->leNum->setFocus();
        return;
    }

    if (ui->leSum->text().isEmpty()
            || ui->leNdsPerCent->text().isEmpty()
            || ui->leSumNds->text().isEmpty()
            || ui->leSumFull->text().isEmpty()) {
        QMessageBox::critical(this, "חוזה", "Amount must be specified!");
        ui->leSum->setFocus();
        return;
    }

    if (gSettings->GetSumFromEdit(ui->leSum->text()).toLongLong() > mMaxSumBrutto) {
        QMessageBox::critical(this, "חוזה", "Maximum amount is " + gSettings->FormatSumForEdit(mMaxSumBrutto) + "!");
        ui->leSum->setFocus();
        return;
    }

    if (isNewRecord) {
        // try to insert new record
        QString lPH;

        lFields = "id_pkz_contract";
        lPH = "?";
        lValues.append(mIdContract);

        lFields += ", order_num";
        lPH += ", ?";
        lValues.append(ui->leNum->text());

        lFields += ", start_date";
        lPH += ", ?";
        lValues.append(ui->dtStart->date());

        lFields += ", name";
        lPH += ", ?";
        lValues.append(ui->leName->text());

        lFields += ", sum_brutto";
        lPH += ", ?";
        lValues.append(gSettings->GetSumFromEdit(ui->leSum->text()));

        lFields += ", nds_percent";
        lPH += ", ?";
        lValues.append(gSettings->GetSumFromEdit(ui->leNdsPerCent->text()));

        lFields += ", sum_full";
        lPH += ", ?";
        lValues.append(gSettings->GetSumFromEdit(ui->leSumFull->text()));

        lFields += ", comments";
        lPH += ", ?";
        lValues.append(ui->ptComments->toPlainText());

        if (db.transaction()) {
            bool lIsOk = false;
            QSqlQuery qInsert(db);

            qInsert.prepare(QString("INSERT INTO V_PKZ_CONTRACT_STAGE(") + lFields + ") VALUES (" + lPH + ")");
            for (int i = 0; i < lValues.length(); i++)
                qInsert.addBindValue(lValues.at(i));

            if (!qInsert.exec()) {
                gLogger->ShowSqlError(this, "שלב", qInsert);
            } else {
                qulonglong lId;
                if (gOracle->GetSeqCurVal("SEQ_PKZ_CONTRACT_STAGE_ID", lId)) {
                    QSqlQuery qUpdate(db);

                    qUpdate.prepare(QString("UPDATE V_PKZ_HASHBON SET ID_CONTRACT = NULL, ID_CONTRACT_STAGE = ")
                                    + QString::number(lId) + " WHERE ID_CONTRACT = " + QString::number(mIdContract));
                    if (!qUpdate.exec()) {
                        gLogger->ShowSqlError(this, "שלב", qUpdate);
                    } else {
                        if (!db.commit()) {
                            gLogger->ShowSqlError(this, "שלב", db);
                        } else {
                            lDoAccept = true;
                            UpdateId = lId;
                            lIsOk = true;
                        }
                    }
                }
            }
            if (!lIsOk) {
                db.rollback();
            }
        } else {
            gLogger->ShowSqlError(this, "שלב", db);
        }
    } else {
        // try to update existing record

        if (origNum != ui->leNum->text()) {
            lFields += "order_num = ?, ";
            lValues.append(ui->leNum->text());
        }

        if (origDate != ui->dtStart->date()) {
            lFields += "start_date = ?, ";
            lValues.append(ui->dtStart->date());
        }

        if (origName != ui->leName->text()) {
            lFields += "name = ?, ";
            lValues.append(ui->leName->text());
        }

        if (origSumBrutto != gSettings->GetSumFromEdit(ui->leSum->text())) {
            lFields += "sum_brutto = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSum->text()));
        }

        if (origNdsPercent != gSettings->GetSumFromEdit(ui->leNdsPerCent->text())) {
            lFields += "nds_percent = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leNdsPerCent->text()));
        }

        if (origSumFull != gSettings->GetSumFromEdit(ui->leSumFull->text())) {
            lFields += "sum_full = ?, ";
            lValues.append(gSettings->GetSumFromEdit(ui->leSumFull->text()));
        }

        if (origComments != ui->ptComments->toPlainText()) {
            lFields += "comments = ?, ";
            lValues.append(ui->ptComments->toPlainText());
        }

        if (lFields.length()) {
            lFields.truncate(lFields.length() - 2);

            QSqlQuery qUpdate(db);

            qUpdate.prepare(QString("UPDATE V_PKZ_CONTRACT_STAGE SET ") + lFields + " WHERE ID = ?");
            for (int i = 0; i < lValues.length(); i++)
                qUpdate.addBindValue(lValues.at(i));
            qUpdate.addBindValue(mIdContractStage);

            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(this, "שלב", qUpdate);
            } else {
                UpdateId = mIdContractStage;
                lDoAccept = true;
            }
        } else {
            // nothing changed, just close
            //lDoAccept = true;
        }
    }

    if (lDoAccept) accept();
}

void ContractStageProp::on_leSum_textEdited(const QString &arg1)
{
    gSettings->CalcNdsAndFull(ui->leSum, ui->leNdsPerCent, ui->leSumNds, ui->leSumFull);
}

void ContractStageProp::on_leNdsPerCent_textEdited(const QString &arg1)
{
    on_leSum_textEdited("");
}

void ContractStageProp::on_leSumNds_textEdited(const QString &arg1)
{
    gSettings->CalcFullWhenNdsSumChanged(ui->leSum, ui->leSumNds, ui->leSumFull);
}

void ContractStageProp::on_leSumFull_textEdited(const QString &arg1)
{
    gSettings->CalcNdsAndBrutto(ui->leSumFull, ui->leNdsPerCent, ui->leSumNds, ui->leSum);
}

void ContractStageProp::on_dtStart_userDateChanged(const QDate &date) {
    ui->leNdsPerCent->setText(gSettings->FormatNumber(gSettings->GetNDS(date)));
    on_leSum_textEdited("");
}
