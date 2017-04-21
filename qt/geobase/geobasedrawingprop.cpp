#include "xreftypedata.h"
#include "geobasedrawingprop.h"
#include "ui_geobasedrawingprop.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QMessageBox>

extern QList <XrefTypeData> XrefTypeList;

GeobaseDrawingProp::GeobaseDrawingProp(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::GeobaseDrawingProp),
    mIdGeobase2Plot(0), UpdateId(0)
{
    ui->setupUi(this);
}

GeobaseDrawingProp::~GeobaseDrawingProp()
{
    delete ui;
}

void GeobaseDrawingProp::showEvent(QShowEvent* event) {
    int i;
    QFCDialog::showEvent(event);

    if (!XrefTypeList.count()) InitXrefTypeList(this);
    ui->cbType->addItem("", 0);
    for (i = 0; i < XrefTypeList.count(); i++) {
        ui->cbType->addItem(XrefTypeList.at(i).GetFilename(), XrefTypeList.at(i).GetId());
    }

    QSqlQuery query(
                "SELECT A.ID_PLOT, B.BLOCK_NAME, A.ID_XREFTYPE, A.COMMENTS FROM V_GEOBASE2PLOT A, V_PLOT_SIMPLE B "
                "WHERE A.ID = " + QString::number(mIdGeobase2Plot) +
                " AND A.ID_PLOT = B.ID", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
    } else {
        if (query.next()) {
            ui->leID->setText(query.value("ID_PLOT").toString());

            origFilename = query.value("BLOCK_NAME").toString();
            ui->leFilename->setText(origFilename);

            origType = query.value("ID_XREFTYPE").toInt();
            for (i = 0; i < ui->cbType->count(); i++)
                if (ui->cbType->itemData(i).toInt() == origType) {
                    ui->cbType->setCurrentIndex(i);
                    break;
                }

            origComments = query.value("COMMENTS").toString();
            ui->leComments->setText(origComments);
        } else {
            QMessageBox::critical(this, tr("Геоподосновы"),
                                  QString("Чертёж геоподосновы id = ") + QString::number(mIdGeobase2Plot) + " не существует!");
        }
    }
}

void GeobaseDrawingProp::Accept() {
    bool lDoAccept = false;
    QString lFields1, lFields2;
    QList <QVariant> lValues1, lValues2;

    //tr(QT_TR_NOOP("Error"));

    if (!ui->leFilename->text().length()) {
        QMessageBox::critical(this, tr("Геоподосновы"), "Имя файла должно быть задано!");
        ui->leFilename->setFocus();
        return;
    }

    // try to update existing record

    if (origFilename != ui->leFilename->text()) {
        lFields1 = "block_name = ?, name = ?";
        lValues1.append(ui->leFilename->text());
        lValues1.append(ui->leFilename->text());
    }

    if (origType != ui->cbType->itemData(ui->cbType->currentIndex()).toInt()) {
        lFields2 = "id_xreftype = ?, ";
        if (ui->cbType->itemData(ui->cbType->currentIndex()).toInt())
            lValues2.append(ui->cbType->itemData(ui->cbType->currentIndex()));
        else
            lValues2.append(QVariant()); // null
    }

    if (origComments != ui->leComments->text()) {
        lFields2 += "comments = ?, ";
        lValues2.append(ui->leComments->text());
    }

    if (lFields1.length() || lFields2.length()) {
        bool lIsOk1, lIsOk2;
        if (db.transaction()) {
            lIsOk1 = false;
            if (lFields1.length()) {
                QSqlQuery qUpdate(db);

                qUpdate.prepare(QString("UPDATE V_PLOT_SIMPLE SET ") + lFields1 + " WHERE ID = ?");
                for (int i = 0; i < lValues1.length(); i++)
                    qUpdate.addBindValue(lValues1.at(i));
                qUpdate.addBindValue(ui->leID->text().toInt());

                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(this, tr("Геоподосновы"), qUpdate.lastError());
                } else {
                    UpdateId = mIdGeobase2Plot;
                    lIsOk1 = true;
                }
            } else {
                lIsOk1 = true;
            }

            lIsOk2 = false;
            if (lFields2.length()) {
                lFields2.truncate(lFields2.length() - 2);
                QSqlQuery qUpdate(db);

                qUpdate.prepare(QString("UPDATE V_GEOBASE2PLOT SET ") + lFields2 + " WHERE ID = ?");
                for (int i = 0; i < lValues2.length(); i++)
                    qUpdate.addBindValue(lValues2.at(i));
                qUpdate.addBindValue(mIdGeobase2Plot);

                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(this, tr("Геоподосновы"), qUpdate.lastError());
                } else {
                    UpdateId = mIdGeobase2Plot;
                    lIsOk2 = true;
                }
            } else {
                lIsOk2 = true;
            }

            if (lIsOk1 && lIsOk2) {
                if (!db.commit()) {
                    gLogger->ShowSqlError(this, tr("Геоподосновы"), tr("Can't commit"), db.lastError());
                } else {
                    lDoAccept = true;
                }
            } else
                db.rollback();


        }
    } else {
        // nothing changed, just close
        //lDoAccept = true;
    }

    if (lDoAccept) accept();
}
