#include "contractprop.h"
#include "ui_contractprop.h"

#include "ContractPkz.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/oracle.h"
#include "../UsersDlg/UserRight.h"
#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectListDlg.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

ContractProp::ContractProp(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ContractProp),
    mFileToDo(0),
    mIdContract(0), mIdProjectForNew(0), mIdPlot(0), origProjectId(0), newProjectId(0),
    isNewRecord(false), UpdateId(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    QPalette lPalette = ui->leIdProject->palette();
    // required
    lPalette.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    ui->leIdProject->setPalette(lPalette);
    ui->leProjName->setPalette(lPalette);
    ui->leNum->setPalette(lPalette);
    ui->leSumOrig->setPalette(lPalette);
    ui->leNdsPerCentOrig->setPalette(lPalette);
    ui->leSumNdsOrig->setPalette(lPalette);
    ui->leSumFullOrig->setPalette(lPalette);

    // need NOT to destroy it
    QDoubleValidator *vForSum = new QDoubleValidator(0, 1e12, 2, this);
    ui->leSumOrig->setValidator(vForSum);
    ui->leSumNdsOrig->setValidator(vForSum);
    ui->leSumFullOrig->setValidator(vForSum);
    ui->leSumAct->setValidator(vForSum);
    ui->leSumNdsAct->setValidator(vForSum);
    ui->leSumFullAct->setValidator(vForSum);

    vForSum = new QDoubleValidator(0, 30, 2, this);
    ui->leNdsPerCentOrig->setValidator(vForSum);
    ui->leNdsPerCentAct->setValidator(vForSum);
}

ContractProp::~ContractProp()
{
    delete ui;
}

void ContractProp::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    bool lDoClose = false;

    if (!gSettings->mCustListForContract.length()) gSettings->PopulateCustListForContract();

    ui->cbCustomer->addItems(gSettings->mCustListForContract);

    if (mIdContract) {
        QSqlQuery query(
                    "SELECT A.ID_PROJECT, d.id_plot, A.CUSTOMER, A.NUM, A.START_DATE, A.END_DATE, A.NAME,"
                    " B.SUM_BRUTTO SUM_BRUTTO_ORIG, B.NDS_PERCENT NDS_PERCENT_ORIG, B.SUM_FULL SUM_FULL_ORIG,"
                    " C.SUM_BRUTTO SUM_BRUTTO_ACT, C.NDS_PERCENT NDS_PERCENT_ACT, C.SUM_FULL SUM_FULL_ACT,"
                    " CASE WHEN B.ORDER_NUM <> C.ORDER_NUM THEN 1 ELSE 0 END HAS_ACT,"
                    " A.INDEXING_TYPE, A.COMMENTS FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B, V_PKZ_CONTRACT_SUM C,"
                    " (select * from v_plot2pkz_contract where nvl(deleted, 0) = 0) d"
                    " WHERE A.ID = " + QString::number(mIdContract) +
                    " AND B.ID_CONTRACT = A.ID"
                    " AND B.ORDER_NUM = (SELECT MIN(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                    " AND C.ID_CONTRACT = A.ID"
                    " and d.id_pkz_contract(+) = a.id"
                    " AND C.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)", db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, "חוזה", query);
            lDoClose = true;
        } else {
            if (query.next()) {
                mIdPlot = query.value("id_plot").toInt();

                ui->pbView->setVisible(mIdPlot != 0);
                ui->pbRemove->setVisible(mIdPlot != 0 && gUserRight->CanUpdate("v_plot2pkz_contract", "deleted"));

                origProjectId = query.value("ID_PROJECT").toInt();
                newProjectId = origProjectId;
                ui->leIdProject->setText(QString::number(newProjectId));

                origCustomer = query.value("CUSTOMER").toString();
                ui->cbCustomer->setCurrentText(origCustomer);

                origNum = query.value("NUM").toString();
                ui->leNum->setText(origNum);

                origStartDate = query.value("START_DATE").toDate();
                ui->dtStart->setDate(origStartDate);

                origEndDate = query.value("END_DATE").toDate();
                ui->dtEnd->setDate(origEndDate);

                origName = query.value("NAME").toString();
                ui->ptName->setPlainText(origName);

                origSumBruttoOrig = query.value("SUM_BRUTTO_ORIG").toLongLong();
                origNdsPercentOrig = query.value("NDS_PERCENT_ORIG").toLongLong();
                origSumFullOrig = query.value("SUM_FULL_ORIG").toLongLong();

                ui->leSumOrig->setText(gSettings->FormatSumForEdit(origSumBruttoOrig));
                ui->leNdsPerCentOrig->setText(gSettings->FormatNdsForEdit(origNdsPercentOrig));
                ui->leSumNdsOrig->setText(gSettings->FormatSumForEdit(origSumFullOrig - origSumBruttoOrig));
                ui->leSumFullOrig->setText(gSettings->FormatSumForEdit(origSumFullOrig));

                if (query.value("HAS_ACT").toInt() == 1) {
                    origSumBruttoAct = query.value("SUM_BRUTTO_ACT").toLongLong();
                    origNdsPercentAct = query.value("NDS_PERCENT_ACT").toLongLong();
                    origSumFullAct = query.value("SUM_FULL_ACT").toLongLong();

                    ui->leSumAct->setText(gSettings->FormatSumForEdit(origSumBruttoAct));
                    ui->leNdsPerCentAct->setText(gSettings->FormatNdsForEdit(origNdsPercentAct));
                    ui->leSumNdsAct->setText(gSettings->FormatSumForEdit(origSumFullAct - origSumBruttoAct));
                    ui->leSumFullAct->setText(gSettings->FormatSumForEdit(origSumFullAct));
                } else {
                    origSumBruttoAct = -1;
                    origNdsPercentAct = -1;
                    origSumFullAct = -1;
                }

                origIndexingType = query.value("INDEXING_TYPE").toInt();
                ui->cbIndexingType->setCurrentIndex(origIndexingType);

                origComments = query.value("COMMENTS").toString();
                ui->ptComments->setPlainText(origComments);

                IdProjectChanged();
            } else {
                QMessageBox::critical(this, "חוזה",
                                      QString("חוזה id = ") + QString::number(mIdContract) + " לא קיימ!");
                lDoClose = true;
            }
        }
        isNewRecord = false;
        setWindowTitle("חוזה");
    } else {
        ui->pbView->setVisible(false);
        ui->pbRemove->setVisible(false);

        if (mIdProjectForNew) {
            ui->leIdProject->setText(QString::number(mIdProjectForNew));
            IdProjectChanged();
        }

        ui->cbCustomer->setCurrentText("");

        ui->dtStart->setDate(QDate::currentDate());
        ui->dtEnd->setDate(QDate::currentDate().addYears(1));
        ui->leNdsPerCentOrig->setText(gSettings->FormatNumber(gSettings->GetNDS(ui->dtStart->date())));

        ui->cbIndexingType->setCurrentIndex(0);

        isNewRecord = true;
        setWindowTitle("חוזה חדש");
    }

    ui->pbAttach->setVisible(gUserRight->CanInsert("v_plot2pkz_contract"));

    if (lDoClose) {
        QTimer::singleShot(0, this, SLOT(hide()));
    }
}

void ContractProp::on_toolButton_clicked()
{
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(ui->leIdProject->text().toInt());
    if (dSel.exec() == QDialog::Accepted) {
        ui->leIdProject->setText(QString::number(dSel.GetProjectData()->Id()));
        IdProjectChanged();
    }
}

void ContractProp::IdProjectChanged() {
    if (!ui->leIdProject->text().length()) {
        ui->leProjName->setText("");
        return;
    }

    QSqlQuery query(
                "SELECT PP.GETPROJECTSHORTNAME(ID) PROJNAME"
                " FROM V_PROJECT WHERE ID = " + ui->leIdProject->text(), db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזה", query);
    } else {
        if (query.next()) {
            ui->leIdProject->setModified(true);
            ui->leProjName->setText(query.value("PROJNAME").toString());

            newProjectId = ui->leIdProject->text().toInt();
            return;
        }
    }
    ui->leIdProject->setText(QString::number(newProjectId));
}

void ContractProp::Accept() {
    bool lDoAccept = false;
    QString lFields;
    QList <QVariant> lValues;

    //tr(QT_TR_NOOP("Error"));

    if (!newProjectId) {
        QMessageBox::critical(this, "חוזה", "Project must be specified!");
        ui->leIdProject->setFocus();
        return;
    }

    if (!ui->leNum->text().length()) {
        QMessageBox::critical(this, "חוזה", "Number must be specified!");
        ui->leNum->setFocus();
        return;
    }

    if (ui->leSumOrig->text().isEmpty()
            || ui->leNdsPerCentOrig->text().isEmpty()
            || ui->leSumNdsOrig->text().isEmpty()
            || ui->leSumFullOrig->text().isEmpty()) {
        QMessageBox::critical(this, "חוזה", "Amount must be specified!");
        ui->leSumOrig->setFocus();
        return;
    }

    UpdateId = 0;

    if (db.transaction()) {
        bool lIsOk = false, lIsErr = false;
        if (isNewRecord) {
            // try to insert new record
            QString lPH;

            // look some kinf of foolish (cos no any checking now, so easy way was to write direct bind variables)
            lFields = "id_project";
            lPH = "?";
            lValues.append(newProjectId);

            lFields += ", customer";
            lPH += ", ?";
            lValues.append(ui->cbCustomer->currentText());

            lFields += ", num";
            lPH += ", ?";
            lValues.append(ui->leNum->text());

            lFields += ", start_date";
            lPH += ", ?";
            lValues.append(ui->dtStart->date());

            lFields += ", end_date";
            lPH += ", ?";
            lValues.append(ui->dtEnd->date());

            lFields += ", name";
            lPH += ", ?";
            lValues.append(ui->ptName->toPlainText());

            lFields += ", indexing_type";
            lPH += ", ?";
            lValues.append(ui->cbIndexingType->currentIndex());

            lFields += ", comments";
            lPH += ", ?";
            lValues.append(ui->ptComments->toPlainText());

            QSqlQuery qInsert(db);

            qInsert.prepare(QString("INSERT INTO V_PKZ_CONTRACT(") + lFields + ") VALUES (" + lPH + ")");
            if (qInsert.lastError().isValid()) {
                gLogger->ShowSqlError(this, "חוזה", qInsert);
                lIsErr = true;
            } else {
                for (int i = 0; i < lValues.length(); i++)
                    qInsert.addBindValue(lValues.at(i));
                if (!qInsert.exec()) {
                    gLogger->ShowSqlError(this, "חוזה", qInsert);
                    lIsErr = true;
                } else {
                    if (!gOracle->GetSeqCurVal("seq_pkz_contract_id", UpdateId)) {
                        lIsErr = true;
                    } else {
                        // original cost
                        qInsert.prepare(
                                    "INSERT INTO V_PKZ_CONTRACT_SUM(ID_CONTRACT, ORDER_NUM, SUM_BRUTTO, NDS_PERCENT, SUM_FULL)"
                                    " VALUES (:ID_CONTRACT, :ORDER_NUM, :SUM_BRUTTO, :NDS_PERCENT, :SUM_FULL)");
                        if (qInsert.lastError().isValid()) {
                            gLogger->ShowSqlError(this, "חוזה", qInsert);
                            lIsErr = true;
                        } else {
                            qInsert.bindValue(":ID_CONTRACT", UpdateId);
                            qInsert.bindValue(":ORDER_NUM", 0);
                            qInsert.bindValue(":SUM_BRUTTO", gSettings->GetSumFromEdit(ui->leSumOrig->text()));
                            qInsert.bindValue(":NDS_PERCENT", gSettings->GetSumFromEdit(ui->leNdsPerCentOrig->text()));
                            qInsert.bindValue(":SUM_FULL", gSettings->GetSumFromEdit(ui->leSumFullOrig->text()));

                            if (!qInsert.exec()) {
                                gLogger->ShowSqlError(this, "חוזה", qInsert);
                                lIsErr = true;
                            } else {
                                if (!ui->leSumAct->text().isEmpty()
                                        && !ui->leNdsPerCentAct->text().isEmpty()
                                        && !ui->leSumFullAct->text().isEmpty()) {

                                    // actual cost
                                    qInsert.bindValue(":ORDER_NUM", 1);
                                    qInsert.bindValue(":SUM_BRUTTO", gSettings->GetSumFromEdit(ui->leSumAct->text()));
                                    qInsert.bindValue(":NDS_PERCENT", gSettings->GetSumFromEdit(ui->leNdsPerCentAct->text()));
                                    qInsert.bindValue(":SUM_FULL", gSettings->GetSumFromEdit(ui->leSumFullAct->text()));

                                    if (!qInsert.exec()) {
                                        gLogger->ShowSqlError(this, "חוזה", qInsert);
                                        lIsErr = true;
                                    } else {
                                        lIsOk = true;
                                        mIdContract = UpdateId;
                                    }
                                } else {
                                    lIsOk = true;
                                    mIdContract = UpdateId;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // try to update existing record

            if (newProjectId != origProjectId) {
                lFields += "id_project = ?, ";
                lValues.append(newProjectId);
            }

            if (origCustomer != ui->cbCustomer->currentText()) {
                lFields += "customer = ?, ";
                lValues.append(ui->cbCustomer->currentText());
            }

            if (origNum != ui->leNum->text()) {
                lFields += "num = ?, ";
                lValues.append(ui->leNum->text());
            }

            if (origStartDate != ui->dtStart->date()) {
                lFields += "start_date = ?, ";
                lValues.append(ui->dtStart->date());
            }

            if (origEndDate != ui->dtEnd->date()) {
                lFields += "end_date = ?, ";
                lValues.append(ui->dtEnd->date());
            }

            if (origName != ui->ptName->toPlainText()) {
                lFields += "name = ?, ";
                lValues.append(ui->ptName->toPlainText());
            }

            if (origIndexingType != ui->cbIndexingType->currentIndex()) {
                lFields += "indexing_type = ?, ";
                lValues.append(ui->cbIndexingType->currentIndex());
            }

            if (origComments != ui->ptComments->toPlainText()) {
                lFields += "comments = ?, ";
                lValues.append(ui->ptComments->toPlainText());
            }

            if (lFields.length()) {
                lFields.truncate(lFields.length() - 2);

                QSqlQuery qUpdate(db);

                qUpdate.prepare(QString("UPDATE V_PKZ_CONTRACT SET ") + lFields + " WHERE ID = ?");
                if (qUpdate.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "חוזה", qUpdate);
                    lIsErr = true;
                } else {
                    for (int i = 0; i < lValues.length(); i++)
                        qUpdate.addBindValue(lValues.at(i));
                    qUpdate.addBindValue(mIdContract);

                    if (!qUpdate.exec()) {
                        gLogger->ShowSqlError(this, "חוזה", qUpdate);
                        lIsErr = true;
                    } else {
                        UpdateId = mIdContract;
                        lIsOk = true;
                    }
                }
            }

            if (!lFields.length() || lIsOk) {
                bool lCont = false;
                if (origSumBruttoOrig != gSettings->GetSumFromEdit(ui->leSumOrig->text())
                        || origNdsPercentOrig != gSettings->GetSumFromEdit(ui->leNdsPerCentOrig->text())
                        || origSumFullOrig != gSettings->GetSumFromEdit(ui->leSumFullOrig->text())) {

                    lIsOk = false;
                    QSqlQuery qUpdate(db);
                    qUpdate.prepare(
                                "update v_pkz_contract_sum a set sum_brutto = :sum_brutto, nds_percent = :nds_percent, sum_full = :sum_full"
                                "  where id_contract = :id_contract and order_num ="
                                "   (select min(order_num) from v_pkz_contract_sum where id_contract = a.id_contract)");
                    if (qUpdate.lastError().isValid()) {
                        gLogger->ShowSqlError(this, "חוזה", qUpdate);
                        lIsErr = true;
                    } else {
                        qUpdate.bindValue(":sum_brutto", gSettings->GetSumFromEdit(ui->leSumOrig->text()));
                        qUpdate.bindValue(":nds_percent", gSettings->GetSumFromEdit(ui->leNdsPerCentOrig->text()));
                        qUpdate.bindValue(":sum_full", gSettings->GetSumFromEdit(ui->leSumFullOrig->text()));
                        qUpdate.bindValue(":id_contract", mIdContract);
                        if (!qUpdate.exec()) {
                            gLogger->ShowSqlError(this, "חוזה", qUpdate);
                            lIsErr = true;
                        } else {
                            UpdateId = mIdContract;
                            lIsOk = true;
                            lCont = true;
                        }
                    }
                } else {
                    lCont = true;
                }

                if (lCont
                        && !ui->leSumAct->text().isEmpty()
                        && !ui->leNdsPerCentAct->text().isEmpty()
                        && !ui->leSumFullAct->text().isEmpty()
                        && (origSumBruttoAct != gSettings->GetSumFromEdit(ui->leSumAct->text())
                            || origNdsPercentAct != gSettings->GetSumFromEdit(ui->leNdsPerCentAct->text())
                            || origSumFullAct != gSettings->GetSumFromEdit(ui->leSumFullAct->text()))) {

                    lIsOk = false;
                    QSqlQuery query("select nvl(max(order_num) + 1, 0) from v_pkz_contract_sum where id_contract = " + QString::number(mIdContract), db);

                    if (query.lastError().isValid()) {
                        gLogger->ShowSqlError(this, "חוזה", query);
                        lIsErr = true;
                    } else {
                        int lNewOrderNum;
                        if (query.next()) {
                            lNewOrderNum = query.value(0).toInt();
                        } else {
                            lNewOrderNum = 0;
                        }

                        QSqlQuery qInsert(db);

                        qInsert.prepare(
                                    "INSERT INTO V_PKZ_CONTRACT_SUM(ID_CONTRACT, ORDER_NUM, SUM_BRUTTO, NDS_PERCENT, SUM_FULL)"
                                    " VALUES (:ID_CONTRACT, :ORDER_NUM, :SUM_BRUTTO, :NDS_PERCENT, :SUM_FULL)");
                        if (qInsert.lastError().isValid()) {
                            gLogger->ShowSqlError(this, "חוזה", qInsert);
                            lIsErr = true;
                        } else {
                            qInsert.bindValue(":ID_CONTRACT", mIdContract);
                            qInsert.bindValue(":ORDER_NUM", lNewOrderNum);
                            qInsert.bindValue(":SUM_BRUTTO", gSettings->GetSumFromEdit(ui->leSumAct->text()));
                            qInsert.bindValue(":NDS_PERCENT", gSettings->GetSumFromEdit(ui->leNdsPerCentAct->text()));
                            qInsert.bindValue(":SUM_FULL", gSettings->GetSumFromEdit(ui->leSumFullAct->text()));

                            if (!qInsert.exec()) {
                                gLogger->ShowSqlError(this, "חוזה", qInsert);
                                lIsErr = true;
                            } else {
                                UpdateId = mIdContract;
                                lIsOk = true;
                            }
                        }
                    }
                }
            }
        }

        if (!lIsErr && mFileToDo) {
            // operation with document (attach or delete)
            switch (mFileToDo) {
            case 1:
                // attach
                if (lIsOk = ContractPkz::AttachFile(mIdContract, newProjectId, mAttachFileName)) {
                    UpdateId = mIdContract;
                }
                break;
            case 2:
                // remove
                if (lIsOk = ContractPkz::RemoveFile(mIdPlot, mIdContract)) {
                    UpdateId = mIdContract;
                }
                break;
            }
        }

        if (lIsOk) {
            if (!(lIsOk = db.commit())) {
                gLogger->ShowSqlError(this, "חוזה", tr("Can't commit"), db);
            } else {
                lDoAccept = true;
            }
        }
        if (!lIsOk) {
            db.rollback();
        }

    } else {
        gLogger->ShowSqlError(this, "חוזה", tr("Can't start transaction"), db);
    }


    if (!gSettings->mCustListForContract.contains(ui->cbCustomer->currentText())) {
        gSettings->mCustListForContract.append(ui->cbCustomer->currentText());
        gSettings->mCustListForContract.sort(Qt::CaseInsensitive);
    }

    if (lDoAccept) accept();
}

void ContractProp::on_leSumOrig_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndFull(ui->leSumOrig, ui->leNdsPerCentOrig, ui->leSumNdsOrig, ui->leSumFullOrig);
}

void ContractProp::on_leNdsPerCentOrig_textEdited(const QString &arg1) {
    on_leSumOrig_textEdited("");
}

void ContractProp::on_leSumNdsOrig_textEdited(const QString &arg1) {
    gSettings->CalcFullWhenNdsSumChanged(ui->leSumOrig, ui->leSumNdsOrig, ui->leSumFullOrig);
}

void ContractProp::on_leSumFullOrig_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->leSumFullOrig, ui->leNdsPerCentOrig, ui->leSumNdsOrig, ui->leSumOrig);
}

void ContractProp::on_leSumAct_textEdited(const QString &arg1) {
    if (!arg1.isEmpty() && ui->leNdsPerCentAct->text().isEmpty()) {
        ui->leNdsPerCentAct->setText(gSettings->FormatNumber(gSettings->GetNDS(QDate::currentDate())));
    }
    gSettings->CalcNdsAndFull(ui->leSumAct, ui->leNdsPerCentAct, ui->leSumNdsAct, ui->leSumFullAct);
}

void ContractProp::on_leNdsPerCentAct_textEdited(const QString &arg1) {
    on_leSumAct_textEdited("");
}

void ContractProp::on_leSumNdsAct_textEdited(const QString &arg1) {
    gSettings->CalcFullWhenNdsSumChanged(ui->leSumAct, ui->leSumNdsAct, ui->leSumFullAct);
}

void ContractProp::on_leSumFullAct_textEdited(const QString &arg1) {
    gSettings->CalcNdsAndBrutto(ui->leSumFullAct, ui->leNdsPerCentAct, ui->leSumNdsAct, ui->leSumAct);
}

void ContractProp::on_pbAttach_clicked() {
    if (ContractPkz::SelectFileForAttach(mAttachFileName)) {
        mFileToDo = 1;
        ui->pbAttach->setText(tr("Attach: ") + mAttachFileName.mid(mAttachFileName.lastIndexOf('/') + 1));
        ui->pbAttach->setToolTip(mAttachFileName);

        ui->pbRemove->setVisible(true);
    }
}

void ContractProp::on_pbRemove_clicked() {
    if (mFileToDo == 1) {
        mFileToDo = 0;
        ui->pbAttach->setText(tr("Attach"));
        ui->pbAttach->setToolTip("");
    } else {
        ui->pbView->setVisible(false);
        mFileToDo = 2;
    }
    ui->pbRemove->setVisible(false);
}

void ContractProp::on_pbView_clicked() {
    gSettings->DoOpenNonDwg(mIdPlot, 1, 0, "");
}

void ContractProp::on_dtStart_userDateChanged(const QDate &date) {
    ui->leNdsPerCentOrig->setText(gSettings->FormatNumber(gSettings->GetNDS(date)));
    on_leSumOrig_textEdited("");
    ui->leNdsPerCentAct->setText(gSettings->FormatNumber(gSettings->GetNDS(date)));
    on_leSumAct_textEdited("");
}
