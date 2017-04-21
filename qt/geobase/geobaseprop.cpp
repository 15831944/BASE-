#include "geobaseprop.h"
#include "ui_geobaseprop.h"

#include "../VProject/GlobalSettings.h"

#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectListDlg.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

// TODO: "isModified" must be replaced on orig with new values comparing

GeobaseProp::GeobaseProp(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::GeobaseProp),
    mIdGeobase(0), origCustomerId(0),
    origProjectId(0), newProjectId(0), mIsNewRecord(false)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    mDoNotSave = true;

    QPalette lPalette = ui->leIdProject->palette();
    // required
    lPalette.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    ui->leIdProject->setPalette(lPalette);
    ui->leProjName->setPalette(lPalette);

    // read only
    lPalette.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);
    ui->leGip->setPalette(lPalette);
    ui->leStage->setPalette(lPalette);

    connect(ui->leIdProject, SIGNAL(editingFinished()), this, SLOT(IdProjectChanged()));

    ui->leIdProject->setValidator(new QIntValidator(1, 1e9, this));

    QSqlQuery query("SELECT ID, SHORTNAME, (SELECT COUNT(*) FROM V_GEOBASE WHERE ID_CUSTOMER = A.ID) FROM V_CUSTOMER A ORDER BY 3 DESC, LOWER(SHORTNAME)", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
    } else {
        while (query.next()) {
            ui->cbMaker->addItem(query.value("SHORTNAME").toString(), query.value("ID"));
        }
    }
}

GeobaseProp::~GeobaseProp()
{
    delete ui;
}

void GeobaseProp::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mIdGeobase) {
        mIsNewRecord = false;
        QSqlQuery query(
                    "SELECT ID_CUSTOMER, ORDERNUM, SITE_NUM, ID_PROJECT, RECEIVE_DATE, EXPIRE_DATE, COMMENTS FROM V_GEOBASE"
                    " WHERE ID = " + QString::number(mIdGeobase), db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
        } else {
            if (query.next()) {
                origCustomerId = query.value("ID_CUSTOMER").toInt();
                ui->cbMaker->setCurrentIndex(ui->cbMaker->findData(origCustomerId));

                ui->leOrderNum->setText(query.value("ORDERNUM").toString());
                ui->leSiteNum->setText(query.value("SITE_NUM").toString());

                origProjectId = query.value("ID_PROJECT").toInt();
                newProjectId = origProjectId;
                ui->leIdProject->setText(query.value("ID_PROJECT").toString());
                IdProjectChanged();

                origDateRcv = query.value("RECEIVE_DATE").toDate();
                ui->deDateRcv->setDate(origDateRcv);
                origDateExp = query.value("EXPIRE_DATE").toDate();
                ui->deDateExp->setDate(origDateExp);

                ui->leComments->setText(query.value("COMMENTS").toString());
            } else {
                QMessageBox::critical(this, tr("Геоподосновы"),
                                      QString("Геоподоснова id = ") + QString::number(mIdGeobase) + " не найдена!");
            }
        }
        setWindowTitle("Свойства геоподосновы");
    } else {
        mIsNewRecord = true;
        ui->cbMaker->setCurrentIndex(0);

        if (newProjectId) {
            ui->leIdProject->setText(QString::number(newProjectId));
            IdProjectChanged();
        }

        ui->leOrderNum->setText(origOrderNum);

        ui->deDateRcv->setDate(QDate::currentDate());
        ui->deDateExp->setDate(QDate::currentDate().addYears(3));
        setWindowTitle("Новая геоподоснова");
    }
}

void GeobaseProp::on_toolButton_clicked()
{
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(ui->leIdProject->text().toInt());
    if (dSel.exec() == QDialog::Accepted) {
        ui->leIdProject->setText(QString::number(dSel.GetProjectData()->Id()));
        IdProjectChanged();
    }
}

void GeobaseProp::IdProjectChanged() {
    QSqlQuery query(
                "SELECT PP.GETPROJECTSHORTNAME(ID) PROJNAME,"
                " PP.GETUSERNAMEDISP(GIP) GIP, STAGE"
                " FROM V_PROJECT WHERE ID = " + ui->leIdProject->text(), db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
    } else {
        if (query.next()) {
            ui->leIdProject->setModified(true);

            ui->leProjName->setText(query.value("PROJNAME").toString());
            ui->leGip->setText(query.value("GIP").toString());
            ui->leStage->setText(query.value("STAGE").toString());

            newProjectId = ui->leIdProject->text().toInt();
            return;
        };
    };
    ui->leIdProject->setText(QString::number(newProjectId));
};

void GeobaseProp::Accept() {
    bool lDoAccept = false;
    QString lFields;
    QList <QVariant> lValues;

    if (ui->cbMaker->currentIndex() == -1) {
        QMessageBox::critical(this, tr("Геоподосновы"), "Изготовитель должен быть задан!");
        ui->leIdProject->setFocus();
        return;
    }

    //tr(QT_TR_NOOP("Error"));
    if (!newProjectId) {
        QMessageBox::critical(this, tr("Геоподосновы"), "Проект должен быть задан!");
        ui->leIdProject->setFocus();
        return;
    }

    if (mIsNewRecord) {
        // try to insert new record
        if (!mIdGeobase
                && !gOracle->GetSeqNextVal("seq_geobase_id", mIdGeobase)) return;

        QString lPH;

        lFields = "id";
        lPH = "?";
        lValues.append(mIdGeobase);

        lFields += ", id_customer";
        lPH += ", ?";
        lValues.append(ui->cbMaker->itemData(ui->cbMaker->currentIndex()));

        lFields += ", id_project";
        lPH += ", ?";
        lValues.append(newProjectId);

        if (ui->leOrderNum->text().length()) {
            lFields += ", ordernum";
            lPH += ", ?";
            lValues.append(ui->leOrderNum->text());
        };
        if (ui->leSiteNum->text().length()) {
            lFields += ", site_num";
            lPH += ", ?";
            lValues.append(ui->leSiteNum->text());
        };

        lFields += ", receive_date";
        lPH += ", ?";
        lValues.append(ui->deDateRcv->date());

        lFields += ", expire_date";
        lPH += ", ?";
        lValues.append(ui->deDateExp->date());

        if (ui->leComments->text().length()) {
            lFields += ", comments";
            lPH += ", ?";
            lValues.append(ui->leComments->text());
        }

        QSqlQuery qInsert(db);

        qInsert.prepare(QString("INSERT INTO V_GEOBASE(") + lFields + ") VALUES (" + lPH + ")");
        for (int i = 0; i < lValues.length(); i++) {
            qInsert.addBindValue(lValues.at(i));
        }

        if (!qInsert.exec()) {
            gLogger->ShowSqlError(this, tr("Геоподосновы"), qInsert.lastError());
        } else {
            lDoAccept = true;
        }
    } else {
        // try to update existing record
        if (origCustomerId != ui->cbMaker->itemData(ui->cbMaker->currentIndex())) {
            lFields += "id_customer = ?, ";
            lValues.append(ui->cbMaker->itemData(ui->cbMaker->currentIndex()));
        }

        if (ui->leOrderNum->isModified()) {
            lFields += "ordernum = ?, ";
            lValues.append(ui->leOrderNum->text());
        }

        if (ui->leSiteNum->isModified()) {
            lFields += "site_num = ?, ";
            lValues.append(ui->leSiteNum->text());
        }

        if (origProjectId != newProjectId) {
            lFields += "id_project = ?, ";
            lValues.append(newProjectId);
        }

        if (origDateRcv != ui->deDateRcv->date()) {
            lFields += "receive_date = ?, ";
            lValues.append(ui->deDateRcv->date());
        }

        if (origDateExp != ui->deDateExp->date()) {
            lFields += "expire_date = ?, ";
            lValues.append(ui->deDateExp->date());
        }

        if (ui->leComments->isModified()) {
            lFields += "comments = ?, ";
            lValues.append(ui->leComments->text());
        }

        if (lFields.length()) {
            lFields.truncate(lFields.length() - 2);

            QSqlQuery qUpdate(db);

            qUpdate.prepare("UPDATE V_GEOBASE SET " + lFields + " WHERE ID = ?");
            for (int i = 0; i < lValues.length(); i++)
                qUpdate.addBindValue(lValues.at(i));
            qUpdate.addBindValue(mIdGeobase);

            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(tr("Геоподосновы"), qUpdate.lastError());
            } else {
                lDoAccept = true;
            }
        }
    }

    if (lDoAccept) accept();
}
