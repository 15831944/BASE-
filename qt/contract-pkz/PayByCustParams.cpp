#include "PayByCustParams.h"
#include "ui_PayByCustParams.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QMessageBox>

PayByCustParams::PayByCustParams(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::PayByCustParams)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    //ui->rbByProjects->setChecked(true);

    QSqlQuery query(db);
    query.prepare("select distinct customer from v_pkz_contract order by 1");
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, QObject::tr("Payments by customers"), query);
    } else {
        if (!query.exec()) {
            gLogger->ShowSqlError(this, QObject::tr("Payments by customers"), query);
        } else {
            while (query.next()) {
                ui->lvCustomers->addItem(query.value(0).toString());
            }
        }
    }
}

PayByCustParams::~PayByCustParams()
{
    delete ui;
}

QStringList PayByCustParams::Selected() const {
    QStringList lStrs;
    QList<QListWidgetItem *> lItems = ui->lvCustomers->selectedItems();
    foreach (QListWidgetItem *lItem, lItems) {
        lStrs.append(lItem->text());
    }
    return lStrs;
}
