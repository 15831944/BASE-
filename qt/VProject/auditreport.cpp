#include "auditreport.h"
#include "ui_auditreport.h"

#include "common.h"

#include "../UsersDlg/UserData.h"

#include <QMessageBox>
#include <QMenu>

AuditReport::AuditReport(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::AuditReport)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
}

AuditReport::~AuditReport()
{
    delete ui;
}

void AuditReport::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);
    QIcon icon;

//    QSqlQuery query("SELECT ID, DECODE(ROWNUM, 1, '(last) ' || AUDIT_NAME, AUDIT_NAME) FROM"
//                        " (SELECT ID, START_DATE, TO_CHAR(START_DATE, 'DD.MM.YY HH24:MI:SS') || ' - ' || USERNAME AUDIT_NAME"
//                        " FROM V_AUDIT_STATUS ORDER BY START_DATE DESC)"
//                    /*" WHERE ROWNUM < 201"*/, db);
    QSqlQuery query("SELECT ID, AUDIT_DATE, USERNAME FROM"
                        " (SELECT ID, START_DATE, TO_CHAR(START_DATE, 'DD.MM.YY HH24:MI:SS') AUDIT_DATE, USERNAME"
                        " FROM V_AUDIT_STATUS ORDER BY START_DATE DESC) A"
                    /*" WHERE ROWNUM < 201"*/, db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Drawings audit report"), query);
    } else {
        while (query.next()) {
            ui->mAuditList->addItem(icon, query.value(1).toString() + " - " + gUsers->GetName(query.value(2).toString()), query.value(0));
        }
    }
}

void AuditReport::on_actionPlotView_triggered() {
    //QMessageBox::critical(this, "Info", "a");
}

void AuditReport::on_mGrid_customContextMenuRequested(const QPoint &) {
    return;
    QMenu popMenu(this);
    popMenu.addAction(ui->actionPlotView);
    //popMenu.exec(/*ui->mGrid->mapToGlobal(pos) + QPoint(20, 20)*/QCursor::pos());
}

void AuditReport::on_mAuditList_currentIndexChanged(int index) {
    long i, j;
    QColor redColor(176, 0, 0);
    QPalette palette;

    QTableWidgetItem *twi;
    QSqlQuery query(
                "SELECT TO_CHAR(START_DATE, 'DD.MM.YY HH24:MI:SS'), TO_CHAR(END_DATE, 'DD.MM.YY HH24:MI:SS') ENDTIME,"
                " PURGE_R, PURGE_A, EXPLODE_PROXY, REMOVE_PROXY, AUDIT_FLAG, USERNAME, COMPUTER, 2000 + ACAD_VERSION ACAD_VERSION"
                " FROM V_AUDIT_STATUS WHERE ID = " + ui->mAuditList->itemData(index).toString(), db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Drawings audit report"), query);
        return;
    }

    if (query.next()) {
        palette = ui->eAutocad->palette();
        if (query.value("ENDTIME").isNull()) {
            palette.setColor(QPalette::Base, redColor);
        };
        ui->eEndTime->setPalette(palette);

        ui->eEndTime->setText(query.value("ENDTIME").toString());

        ui->cbPurgeR->setChecked(query.value("PURGE_R").toInt());
        ui->cbPurgeA->setChecked(query.value("PURGE_A").toInt());
        ui->cbExplodeProxy->setChecked(query.value("EXPLODE_PROXY").toInt());
        ui->cbRemoveProxy->setChecked(query.value("REMOVE_PROXY").toInt());
        ui->cbAudit->setChecked(query.value("AUDIT_FLAG").toInt());

        ui->eUser->setText(gUsers->GetName(query.value("USERNAME").toString()));
        ui->eComputer->setText(query.value("COMPUTER").toString());
        ui->eAutocad->setText(query.value("ACAD_VERSION").toString());

        ui->mGrid->setRowCount(0);

//        QSqlQuery query2(
//                "SELECT C.ID, C.CODE,"
//                    " DECODE(A.ID_DWG, NULL, '',"
//                    " (SELECT VERSION FROM V_DWG WHERE ID = A.ID_DWG) || '/' || (SELECT TO_CHAR(MAX(VERSION)) FROM V_DWG WHERE ID_PLOT = C.ID)),"
//                    " DECODE(A.STATUS, 0, 'Can''t open', 1, 'No changes', 2, 'Saved', 3, 'Acad version', 4, 'Can''t save to Projects Base',"
//                    " 5, 'Size is 0', 6, 'Can''t save to file', TO_CHAR(A.STATUS)"
//                    " FROM V_AUDIT_DWG A, V_PLOT_SIMPLE C"
//                    " WHERE A.ID_AUDIT = " + ui->mAuditList->itemData(index).toString() +
//                    " AND (A.ID_DWG IS NOT NULL AND C.ID = (SELECT ID_PLOT FROM V_DWG WHERE ID = A.ID_DWG) OR A.ID_DWG IS NULL AND A.ID_PLOT = C.ID)"
//                    " ORDER BY ORDER_NUM", db);

        QSqlQuery query2(
                QString("SELECT C.ID, C.CODE,"
                    " CASE WHEN A.ID_DWG IS NULL THEN ''"
                    " ELSE (SELECT VERSION FROM V_DWG WHERE ID = A.ID_DWG) || '/' || (SELECT ")
                    + ((db.driverName() == "QPSQL")?"CAST (MAX(VERSION) AS VARCHAR)":"TO_CHAR(MAX(VERSION))")
                    + " FROM V_DWG WHERE ID_PLOT = C.ID) END,"
                    " A.STATUS"
                    " FROM V_AUDIT_DWG A, V_PLOT_SIMPLE C"
                    " WHERE A.ID_AUDIT = " + ui->mAuditList->itemData(index).toString() +
                    " AND (A.ID_DWG IS NOT NULL AND C.ID = (SELECT ID_PLOT FROM V_DWG WHERE ID = A.ID_DWG) OR A.ID_DWG IS NULL AND A.ID_PLOT = C.ID)"
                    " ORDER BY ORDER_NUM", db);

        if (query2.lastError().isValid()) {
            gLogger->ShowSqlError(this, tr("Drawings audit report"), query2);
        } else {
            //bool lIsColor;

            i = 0;
            while (query2.next()) {
                j = 0;
                ui->mGrid->insertRow(i);

                //lIsColor = (query2.value(mStatusField + 1).isNull() || query2.value(mStatusField + 1).toInt() != 0);

                // id, code, history (done/current), status text
                for (j = 0; j < 4; j++) {
                    QString str;
                    if (j == 3) {
                        switch (query2.value(j).toInt()) {
                        case 0:
                            str = tr("Can't open");
                            break;
                        case 1:
                            str = tr("No changes");
                            break;
                        case 2:
                            str = tr("Saved");
                            break;
                        case 3:
                            str = tr("Acad version");
                            break;
                        case 4:
                            str = tr("Can't save to server");
                            break;
                        case 5:
                            str = tr("Size is 0");
                            break;
                        case 6:
                            str = tr("Can't save to file");
                            break;
                        default:
                            str = query2.value(j).toString();
                            break;
                        }
                    } else {
                        str = query2.value(j).toString();
                    }
                    twi = new QTableWidgetItem(str);
                    twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                    //if (lIsColor && !j) {
                    //    twi->setBackgroundColor(redColor);
                    //    twi->setToolTip(query2.value(mStatusField + 2).toString());
                    //};
                    if (!j) twi->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    else if (j == 2) twi->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                    ui->mGrid->setItem(i, j, twi);
                }

                i++;
            }
            ui->mGrid->resizeColumnsToContents();
            ui->mGrid->resizeRowsToContents();
        }
    }
}
