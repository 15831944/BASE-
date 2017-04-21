#include "geobase.h"
#include "geobaseprop.h"
#include "geobasedrawingprop.h"
#include "geobaseloadfiles.h"
#include "xreftypedata.h"
#include "ui_geobase.h"
#include "geobasesettings.h"

#include "../VProject/GlobalSettings.h"
#include "../UsersDlg/UserRight.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QProcess>
#include <QMenu>
#include <QMdiSubWindow>

extern QList <XrefTypeData> XrefTypeList;

Geobase::Geobase(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::Geobase),
    mJustStarted(true)
{
    CurrentVersion = 3;
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    if (!gSettings->Geobase.SelectBeh)
        ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    else
        ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    if (!gSettings->Geobase.SelectMode)
        ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    else
        ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

Geobase::~Geobase()
{
    delete ui;
}

void Geobase::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);
    if (mJustStarted) {
        /*if (!XrefTypeList.count()) */InitXrefTypeList(this);
        mJustStarted = false;

        if (!gUserRight->CanSelect("v_geobase")
                || !gUserRight->CanSelect("v_geobase2plot")) {
            if (qobject_cast<QMdiSubWindow *>(parent())) {
                QTimer::singleShot(0, parent(), SLOT(close()));
            } else {
                QTimer::singleShot(0, this, SLOT(close()));
            }
        } else {
            if (ReadVersion < CurrentVersion) {
                int nCol = 0;
                ui->treeWidget->setColumnWidth(nCol++, 180); // object
                ui->treeWidget->setColumnWidth(nCol++, 120); // GIP
                ui->treeWidget->setColumnWidth(nCol++, 20); // stage
                ui->treeWidget->setColumnWidth(nCol++, 80); // order num
                ui->treeWidget->setColumnWidth(nCol++, 20); // site
            }
            ui->tbPlus->setEnabled(gUserRight->CanInsert("v_geobase"));
            CanUpdateGeobase = gUserRight->CanUpdate("v_geobase");
            CanDeleteGeobase = gUserRight->CanDelete("v_geobase");
            ui->gbGeobase->setVisible(ui->tbPlus->isEnabled() || CanUpdateGeobase ||CanDeleteGeobase);

            CanInsertGeobasePlot = gUserRight->CanInsert("v_geobase2plot");
            CanUpdateGeobasePlot = gUserRight->CanUpdate("v_geobase2plot");
            CanDeleteGeobasePlot = gUserRight->CanDelete("v_geobase2plot");
            ui->gbDrawing->setVisible(CanInsertGeobasePlot || CanUpdateGeobasePlot ||CanDeleteGeobasePlot);

            QTimer::singleShot(0, this, SLOT(ShowData()));
        }
    }
}

void Geobase::ShowData() {
    bool lSignalsState = ui->treeWidget->blockSignals(true);
    ui->treeWidget->PopulateTree(0);
    ui->treeWidget->blockSignals(lSignalsState);
    emit ui->treeWidget->currentItemChanged(ui->treeWidget->currentItem(), NULL);
}

void Geobase::ShowProps(QGeobaseTreeItem *item) {
    GeobaseProp w(this);
    if (item) {
        // exist
        w.SetGeobaseId(item->IdGeobase());
    } else {
        // new geobase
        if (item = (QGeobaseTreeItem *) ui->treeWidget->currentItem()) {
            if (item->parent()) item = (QGeobaseTreeItem *) item->parent();
            w.SetProjectIdForNew(item->IdProject());
            w.SetOrderNumForNew(item->OrderNum());
        }
    }
    if (w.exec() == QDialog::Accepted) {
        if (w.GeobaseId()) {
            ui->treeWidget->PopulateTree(w.GeobaseId());
        }
    }
}

void Geobase::ShowPropsForDrawing(QGeobaseTreeItem *item, int column) {
    QList<int> list;
    list << 40 << 30 << 10 << 20 << 60 << 70 << 50 << 0;

    GeobaseDrawingProp w(this);
    w.SetGeobase2PlotId(item->IdGeobase2Plot(list.at(column - 7)));
    if (w.exec() == QDialog::Accepted) {
        if (w.GetUpdateId()) {
            ui->treeWidget->PopulateTree(0);
        }
    }
}

void Geobase::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (column < 7) {
        // geobase properties
        if (item->parent())
            ShowProps((QGeobaseTreeItem *) item->parent());
        else
            ShowProps((QGeobaseTreeItem *) item);
    } else if (column < 15) {
        if (gSettings->Geobase.OnDblClick == 0) {
            // drawing property
            if (item->text(column).length()) {
                ShowPropsForDrawing((QGeobaseTreeItem *) item, column);
            }
        } else if (gSettings->Geobase.OnDblClick == 1) {
            ui->actionView->trigger();
        }
    }
}

void Geobase::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    QGeobaseTreeItem *item;
    item = (QGeobaseTreeItem *) ui->treeWidget->itemAt(pos);

    if (item) {
        bool lHasAnyDrawing = false;
        if (ui->treeWidget->selectionMode() == QAbstractItemView::ExtendedSelection
                && ui->treeWidget->selectionBehavior() == QAbstractItemView::SelectItems) {
            QModelIndexList SIL = ui->treeWidget->selectionModel()->selectedIndexes();

            foreach (QModelIndex SI, SIL) {
                QGeobaseTreeItem * item = (QGeobaseTreeItem *) ui->treeWidget->itemFromModelIndex(SI);
                if (SI.column() > 6 && SI.column() < 15
                        && !item->text(SI.column()).isEmpty()) {
                    lHasAnyDrawing = true;
                }
            }
        }

        bool lDefSetted = false;
        QMenu popMenu(this);
        if (ui->tbPlus->isEnabled())
            popMenu.addAction(ui->actionAdd_geobase);
        if (CanUpdateGeobase)
            popMenu.addAction(ui->actionProp_geobase);
        if (CanDeleteGeobase)
            popMenu.addAction(ui->actionDel_geobase);
        if (CanUpdateGeobase)
            popMenu.addAction(ui->actionRecalc_coords);

        popMenu.addSeparator();
        if (CanInsertGeobasePlot)
            popMenu.addAction(ui->actionLoad_files);
        if (ui->treeWidget->currentColumn() > 6 && ui->treeWidget->currentColumn() < 15
                && item->text(ui->treeWidget->currentColumn()).length()) {
            //if (CanUpdateGeobasePlot) {
                popMenu.addAction(ui->actionPlot_prop);
                if (gSettings->Geobase.OnDblClick == 0) {
                    popMenu.setDefaultAction(ui->actionPlot_prop);
                    lDefSetted = true;
                }
            //}
        }

        if (ui->treeWidget->currentColumn() > 6 && ui->treeWidget->currentColumn() < 15
                && item->text(ui->treeWidget->currentColumn()).length()
                || lHasAnyDrawing) {
            popMenu.addAction(ui->actionView);
            if (gSettings->Geobase.OnDblClick == 1) {
                popMenu.setDefaultAction(ui->actionView);
                lDefSetted = true;
            }
        }

        if (ui->treeWidget->currentColumn() > 6 && ui->treeWidget->currentColumn() < 15
                && item->text(ui->treeWidget->currentColumn()).length()
                || lHasAnyDrawing) {
            if (CanDeleteGeobasePlot)
                popMenu.addAction(ui->actionDel_files);
        }

        popMenu.addSeparator();
        if (CanUpdateGeobase)
            popMenu.addAction(ui->actionRecalc_coords);

        if (CanUpdateGeobase && !lDefSetted) {
            popMenu.setDefaultAction(ui->actionProp_geobase);
        }

        // 1 element is always added - it is separator; it is not shown if it need not but it exists
        if (popMenu.actions().count() > 2)
            popMenu.exec(QCursor::pos());

    }
}

void Geobase::on_actionAdd_geobase_triggered()
{
    ShowProps(NULL);
}

void Geobase::on_actionProp_geobase_triggered()
{
    QGeobaseTreeItem *item;
    item = (QGeobaseTreeItem *) ui->treeWidget->currentItem();
    if (item) {
        if (item->parent())
            ShowProps((QGeobaseTreeItem *) item->parent());
        else
            ShowProps(item);
    }
}

void Geobase::on_actionLoad_files_triggered()
{
    GeobaseLoadFiles w(this);

    QGeobaseTreeItem *item;
    item = (QGeobaseTreeItem *) ui->treeWidget->currentItem();
    if (item) {
        if (item->parent())
            item = (QGeobaseTreeItem *) item->parent();
        w.SetGeobaseData(item->IdGeobase(), item->IdProject(), item->Maker(), item->OrderNum(), item->SiteNum());
        if (w.exec() == QDialog::Accepted) {
            ui->treeWidget->PopulateTree(item->IdGeobase());
        }
    }
}

void Geobase::on_tbPlus_2_clicked()
{
    on_actionLoad_files_triggered();
}


bool Geobase::DeleteGeobase(int aIdGeobase)
{
    bool res = false;
    if (db.transaction()) {
        bool lIsErr = false;
        QSqlQuery querySelPlot(db);
        querySelPlot.prepare("select id from v_geobase2plot where id_geobase = :id");

        if (querySelPlot.lastError().isValid()) {
            gLogger->ShowSqlError(this, "Удаление геоподосновы", querySelPlot.lastError());
            lIsErr = true;
        } else {
            querySelPlot.bindValue(":id", aIdGeobase);
            if (!querySelPlot.exec()) {
                gLogger->ShowSqlError(this, "Удаление геоподосновы", querySelPlot.lastError());
                lIsErr = true;
            } else {
                QSqlQuery queryDelPlot(db);
                queryDelPlot.prepare("begin pgeobase.deletegeobaseplot(:id); end;");
                if (queryDelPlot.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "Удаление геоподосновы", queryDelPlot.lastError());
                    lIsErr = true;
                } else {
                    while (querySelPlot.next()) {
                        queryDelPlot.bindValue(":id", querySelPlot.value(0).toInt());
                        if (!queryDelPlot.exec()) {
                            gLogger->ShowSqlError(this, "Удаление геоподосновы", queryDelPlot.lastError());
                            lIsErr = true;
                            break;
                        }
                    }
                    if (!lIsErr) {
                        // delete v_geobase
                        QSqlQuery queryDelGB(db);
                        queryDelGB.prepare("delete from v_geobase where id = :id");

                        if (queryDelGB.lastError().isValid()) {
                            gLogger->ShowSqlError(this, "Удаление геоподосновы", queryDelGB.lastError());
                            lIsErr = true;
                        } else {
                            queryDelGB.bindValue(":id", aIdGeobase);
                            if (!queryDelGB.exec()) {
                                gLogger->ShowSqlError(this, "Удаление геоподосновы", queryDelGB.lastError());
                                lIsErr = true;
                            }
                        }
                    }
                }
            }
        }
        if (!lIsErr) {
            if (!db.commit()) {
                gLogger->ShowSqlError(this, "Удаление геоподосновы", tr("Can't commit"), db.lastError());
            } else {
                res = true;
            }
        } else {
            db.rollback();
        }
    } else {
        gLogger->ShowSqlError(this, "Удаление геоподосновы", tr("Can't start transaction"), db.lastError());
    }
    return res;
}

void Geobase::on_actionDel_geobase_triggered()
{
    if (ui->treeWidget->selectionMode() == QAbstractItemView::ExtendedSelection) {
        QList <int> listForDel;
        QList <QTreeWidgetItem *> SWIL = ui->treeWidget->selectedItems();
        QString lAskStr;


        foreach (QTreeWidgetItem *WI, SWIL) {
            QGeobaseTreeItem * item = (QGeobaseTreeItem *) WI;
            if (!listForDel.contains(item->IdGeobase())) {
                listForDel.append(item->IdGeobase());

                if (!lAskStr.isEmpty()) lAskStr += "\n";
                if (!item->text(4).isEmpty())
                    lAskStr += item->text(3) + " (уч. " + item->text(4) + ")";
                else
                    lAskStr += item->text(3);
            }

        }
        QMessageBox qb;

        qb.setWindowTitle(tr("Геоподосновы"));
        qb.setDetailedText(lAskStr);
        qb.setText("Удалить выбранные геоподосновы?");
        qb.addButton(QMessageBox::Yes);
        qb.setDefaultButton(qb.addButton(QMessageBox::No));

        if (qb.exec() == QMessageBox::Yes) {
            foreach(int lId, listForDel)
                if (!DeleteGeobase(lId))
                    break;
            ui->treeWidget->PopulateTree(0);
        }
    } else {
        // delete single geobase

        QGeobaseTreeItem *item;
        item = (QGeobaseTreeItem *) ui->treeWidget->currentItem();
        if (item) {
            if (item->parent())
                item = (QGeobaseTreeItem *) item->parent();
            QString question;
            if (item->text(4).length())
                question = QString("Удалить геоподоснову ") + item->text(3) + " (уч. " + item->text(4) + ")?";
            else
                question = QString("Удалить геоподоснову ") + item->text(3) + "?";
            if (QMessageBox::question(this, tr("Геоподосновы"), question) == QMessageBox::Yes) {
                if (DeleteGeobase(item->IdGeobase()))
                    ui->treeWidget->PopulateTree(0);
            }
        }
    }
}

void Geobase::on_tbReload_clicked()
{
    QGeobaseTreeItem *item = (QGeobaseTreeItem *) ui->treeWidget->currentItem();
    if (item) {
        if (item->parent())
            item = (QGeobaseTreeItem *) item->parent();
        ui->treeWidget->PopulateTree(item->IdGeobase());
    } else {
        ui->treeWidget->PopulateTree(0);
    }
}

void Geobase::on_actionPlot_prop_triggered()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (item
            && ui->treeWidget->currentColumn() > 6
            &&  ui->treeWidget->currentColumn() < 15
            && item->text(ui->treeWidget->currentColumn()).length())
        ShowPropsForDrawing((QGeobaseTreeItem *) item, ui->treeWidget->currentColumn());
}

void Geobase::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (!current) {
        ui->tbProps->setEnabled(false);
        ui->tbMinus->setEnabled(false);

        ui->tbPlus_2->setEnabled(false);
        ui->tbProps_2->setEnabled(false);
        ui->tbMinus_2->setEnabled(false);
    } else {

        ui->tbProps->setEnabled(CanUpdateGeobase);
        ui->tbMinus->setEnabled(CanDeleteGeobase);

        ui->tbPlus_2->setEnabled(CanInsertGeobasePlot);

        bool drwEnabled = ui->treeWidget->currentColumn() > 6
                && ui->treeWidget->currentColumn() < 15
                && current->text(ui->treeWidget->currentColumn()).length();
        ui->tbProps_2->setEnabled(CanUpdateGeobasePlot && drwEnabled);
        ui->tbMinus_2->setEnabled(CanDeleteGeobasePlot
                                  && (drwEnabled ||
                                      (ui->treeWidget->selectionMode() == QAbstractItemView::ExtendedSelection
                                       && ui->treeWidget->selectionBehavior() == QAbstractItemView::SelectItems)));
    }
}

void Geobase::on_toolButton_clicked() {
    GeobaseSettings w(this);
    w.SetOnDblClick(gSettings->Geobase.OnDblClick);
    w.SetSelectBeh(gSettings->Geobase.SelectBeh);
    w.SetSelectMode(gSettings->Geobase.SelectMode);
    w.SetDrawingShowMode(gSettings->Geobase.DrawingShowMode);
    if (w.exec() == QDialog::Accepted) {
        gSettings->Geobase.OnDblClick = w.OnDblClick();
        gSettings->Geobase.DrawingShowMode = w.GetDrawingShowMode();

        if (!(gSettings->Geobase.SelectBeh = w.GetSelectBeh()))
            ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
        else
            ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

        if (!(gSettings->Geobase.SelectMode = w.GetSelectMode()))
            ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        else
            ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        on_tbReload_clicked();
    }

}

void Geobase::on_actionDel_files_triggered()
{
    QList<int> list;
    list << 40 << 30 << 10 << 20 << 60 << 70 << 50 << 0;

    // multidelete
    if (ui->treeWidget->selectionMode() == QAbstractItemView::ExtendedSelection
            && ui->treeWidget->selectionBehavior() == QAbstractItemView::SelectItems) {
        QModelIndexList SIL = ui->treeWidget->selectionModel()->selectedIndexes();

        QSqlQuery query(db);
        query.prepare("begin pgeobase.deletegeobaseplot(:id); end;");
        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, "Геоподосновы - удаление чертежа", query.lastError());
        } else {
            QString lAskStr;
            foreach (QModelIndex SI, SIL) {
                QGeobaseTreeItem * item = (QGeobaseTreeItem *) ui->treeWidget->itemFromModelIndex(SI);
                if (SI.column() > 6 && SI.column() < 15
                        && !item->text(SI.column()).isEmpty()) {
                    //
                    if (!lAskStr.isEmpty()) lAskStr += "\n";
                    lAskStr += QString::number(item->IdPlot(list.at(SI.column() - 7))) + "  -  " +
                            item->PlotName(list.at(SI.column() - 7));
                }
            }
            QMessageBox qb;

            qb.setWindowTitle(tr("Геоподосновы"));
            qb.setDetailedText(lAskStr);
            qb.setText("Удалить выбранные чертежи?");
            qb.addButton(QMessageBox::Yes);
            qb.setDefaultButton(qb.addButton(QMessageBox::No));
            if (qb.exec() == QMessageBox::Yes) {
                foreach (QModelIndex SI, SIL) {
                    QGeobaseTreeItem * item = (QGeobaseTreeItem *) ui->treeWidget->itemFromModelIndex(SI);
                    if (SI.column() > 6 && SI.column() < 15
                            && !item->text(SI.column()).isEmpty()) {
                        // delete it baby
                        query.bindValue(":id", item->IdGeobase2Plot(list.at(SI.column() - 7)));
                        if (!query.exec()) {
                            gLogger->ShowSqlError(this, "Геоподосновы - удаление чертежа", query.lastError());
                        } else {
                            ui->treeWidget->PopulateTree(0);
                        }

                    }
                }
                ui->treeWidget->PopulateTree(0);
            }
        }
    } else {
        // delete single drawing
        QGeobaseTreeItem * item = (QGeobaseTreeItem *) ui->treeWidget->currentItem();
        if (item && ui->treeWidget->currentColumn() > 6 && ui->treeWidget->currentColumn() < 15
                && item->text(ui->treeWidget->currentColumn()).length()
                && QMessageBox::question(this, "Геоподосновы - удаление чертежа",
                                         "Удалить чертёж " + item->text(ui->treeWidget->currentColumn()) + "?") == QMessageBox::Yes) {
            QSqlQuery query(db);
            query.prepare("begin pgeobase.deletegeobaseplot(:id); end;");
            if (query.lastError().isValid()) {
                gLogger->ShowSqlError(this, "Геоподосновы - удаление чертежа", query.lastError());
            } else {
                query.bindValue(":id", item->IdGeobase2Plot(list.at(ui->treeWidget->currentColumn() - 7)));
                if (!query.exec()) {
                    gLogger->ShowSqlError(this, "Геоподосновы - удаление чертежа", query.lastError());
                } else {
                    ui->treeWidget->PopulateTree(0);
                }
            }
        }
    }
}

void Geobase::on_actionRecalc_coords_triggered()
{
    QGeobaseTreeItem *item = (QGeobaseTreeItem *) ui->treeWidget->currentItem();
    if (item) {
        if (item->parent())
            item = (QGeobaseTreeItem *) item->parent();
    }

    if (!item) return;

    QString cmdLine;
    cmdLine = QCoreApplication::applicationDirPath();
    cmdLine.resize(cmdLine.lastIndexOf(QChar('/')));
    cmdLine += "/oraint.exe \"" + db.databaseName() + "\" \""
            + db.userName() + "\" \"" + db.password() + "\" \""
            + gSettings->CurrentSchema + "\" calcgeo " + QString::number(item->IdGeobase());

    QProcess proc1;
    proc1.start(cmdLine);
    if (!proc1.waitForStarted(-1)) {
        gLogger->ShowError(this, "AutoCAD wait for started", proc1.errorString());
    } else {
        if (!proc1.waitForFinished(-1)) {
            gLogger->ShowError(this, "AutoCAD wait for finished", proc1.errorString());
        } else {
            // everything is ok, baby
        }
    }
}

void Geobase::on_actionView_triggered() {
    QList<int> list;
    list << 40 << 30 << 10 << 20 << 60 << 70 << 50 << 0;
    MainDataForCopyToAcad lDataForAcad(1, false);

    if (ui->treeWidget->selectionMode() == QAbstractItemView::ExtendedSelection
            && ui->treeWidget->selectionBehavior() == QAbstractItemView::SelectItems) {
        QModelIndexList SIL = ui->treeWidget->selectionModel()->selectedIndexes();
        QList <int> lAddedList;

        foreach (QModelIndex SI, SIL) {
            QGeobaseTreeItem * item = (QGeobaseTreeItem *) ui->treeWidget->itemFromModelIndex(SI);
            if (SI.column() > 6 && SI.column() < 15
                    && !item->text(SI.column()).isEmpty()
                    && !lAddedList.contains(item->IdPlot(list.at(SI.column() - 7)))) {
                QMessageBox::critical(NULL, "", QString::number(item->IdPlot(list.at(SI.column() - 7))));
                lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(item->IdPlot(list.at(SI.column() - 7)), 0, 0, false));
                lAddedList.append(item->IdPlot(list.at(SI.column() - 7)));
            }
        }
    } else {
        QGeobaseTreeItem *item = (QGeobaseTreeItem *) ui->treeWidget->currentItem();
        if (item) {
            if (item->parent())
                item = (QGeobaseTreeItem *) item->parent();
            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(item->IdPlot(list.at(ui->treeWidget->currentColumn() - 7)), 0, 0, false));
        }
    }

    if (!lDataForAcad.ListConst().isEmpty()) {
        gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
    }
}
