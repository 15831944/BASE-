#include "ContractPkz.h"
#include "ui_ContractPkz.h"

#include "contractprop.h"
#include "contractstageprop.h"
#include "contractcheck.h"
#include "ContractDefVAT.h"
#include "PayByCustParams.h"

#include "../VProject/SelectColumnsDlg.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/TreeData.h"
#include "../PlotLib/PlotData.h"
#include "../ProjectLib//ProjectData.h"
#include "../UsersDlg//UserRight.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QMessageBox>
#include <QThread>
#include <QFileDialog>
#include <QMenu>

ContractPkz::ContractPkz(QWidget *parent) :
    QFCDialog(parent, true/*false*/),
    ui(new Ui::ContractPkz),
    mJustStarted(true)
{
    CurrentVersion = 7;

    ui->setupUi(this);
    on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), NULL);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    // always selected in normal color (not gray)
    QPalette lPalette = ui->treeWidget->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->treeWidget->setPalette(lPalette);
    // ---------------------------------


    // ----------------------------------------------
    QSplitterHandle *handle = ui->splitter->handle(1);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    QFrame *line = new QFrame(handle);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    ui->splitter->setStretchFactor(1, 100);
    // ----------------------------------------------

}

ContractPkz::~ContractPkz()
{
    delete ui;
}

void ContractPkz::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        setLayoutDirection(Qt::RightToLeft);
        //setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        //setLayoutDirection(Qt::LeftToRight);

        if (ReadVersion < CurrentVersion) {
            QList<int> lSizes;
            lSizes.append(600);
            lSizes.append(200);
            ui->splitter->setSizes(lSizes);
        }

        ui->tbHashbonShow->setChecked(ui->dockHashbon->isVisible());
        ui->treeWidget->setSelectionMode(gSettings->Contract.MultiSelect?QAbstractItemView::ExtendedSelection:QAbstractItemView::SingleSelection);
        ui->treeWidget->header()->setStyleSheet("background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(200, 200, 200, 255), stop:1 rgba(255, 255, 255, 255));");
        ui->treeHashbon->header()->setStyleSheet("background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(200, 200, 200, 255), stop:1 rgba(255, 255, 255, 255));");

        ui->treeWidget->header()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->treeWidget->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumnsTop(const QPoint &)));

        ui->treeHashbon->header()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->treeHashbon->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumnsHashbon(const QPoint &)));

        if (ui->cbHideEmpty->isChecked()) {
            emit ui->cbHideEmpty->clicked();
        }
    }
    //QMessageBox::critical(NULL, "", ui->cbHideEmpty->isChecked()?"true":"false");
}

void ContractPkz::LoadAdditionalSettings(QSettings &aSettings) {
    ui->cbHideEmpty->setChecked(aSettings.value("HideNoDebts", false).toBool());
}

void ContractPkz::SaveAdditionalSettings(QSettings &aSettings) {
    aSettings.setValue("HideNoDebts", ui->cbHideEmpty->isChecked());
}

void ContractPkz::ShowProps(QContractTreeItem *item) {
    ContractProp w(this);
    if (item) {
        w.SetContractId(item->IdContract());
    } else {
        QContractTreeItem *curItem = (QContractTreeItem *) ui->treeWidget->currentItem();
        if (curItem) {
            while (curItem->parent()) curItem = (QContractTreeItem *) curItem->parent();
            w.SetProjectIdForNew(curItem->IdProject());
        }
    }
    if (w.exec() == QDialog::Accepted) {
        if (w.GetUpdateId()) {
            if (item) {
                item->ReloadContractFromBase(0);
            } else {
                ui->treeWidget->AddContract(w.GetUpdateId());
            }
        }
    }
}

void ContractPkz::ShowStageProps(QContractTreeItem *thisItem, QContractTreeItem *parentItem) {
    ContractStageProp w(this);
    int lIdContract, lIdContractStage = 0;
    if (!thisItem && parentItem) {
        // it is for new stage
        lIdContract = parentItem->IdContract();
        w.SetContractId(parentItem->IdContract());
        w.SetNum(parentItem->childCount() + 1);
    } else if (thisItem && !parentItem) {
        // it is for existing stage
        lIdContract = ((QContractTreeItem *) thisItem->parent())->IdContract();
        lIdContractStage = thisItem->IdContractStage();
        w.SetContractStageId(thisItem->IdContractStage());
    }

    QSqlQuery query(db);
    query.prepare(
                "select sum_brutto -"
                "  (select nvl(sum(sum_brutto), 0) from v_pkz_contract_stage"
                "    where id_pkz_contract = " + QString::number(lIdContract) +
                "    and id <> " + QString::number(lIdContractStage) + ")"
                " from v_pkz_contract_sum a where id_contract = " + QString::number(lIdContract) +
                " and order_num = (select max(order_num) from v_pkz_contract_sum where id_contract = a.id_contract)");
    if (!query.exec() || !query.next()) {
        gLogger->ShowSqlError(this, "חוזים", query);
        return;
    } else {
        if (thisItem && !parentItem || query.value(0).toLongLong() > 0)
            w.SetMaxSumBrutto(query.value(0).toLongLong());
        else {
            QMessageBox::critical(this, "חוזים", "Full sum in hashbones!");
            return;
        }
    }

    if (w.exec() == QDialog::Accepted) {
        if (w.GetUpdateId()) {
            if (thisItem) {
                ((QContractTreeItem *) thisItem->parent())->ReloadContractFromBase(0);
                thisItem->ReloadContractStageFromBase(0);
            } else {
                ui->treeWidget->AddContractStage(w.GetUpdateId(), parentItem);
            }
        }
    }
}

void ContractPkz::ShowCheckProps(int aIdCheck) {
    ContractCheck w(this);

    QContractTreeItem *item = (QContractTreeItem *) ui->treeWidget->currentItem();

    if (!item) return; // none
    if (!aIdCheck) {
        // new check
        if (!item->IdContractStage() && item->childCount()) return; // can't pay on staged contract

        if (item->IdContractStage()) {
            w.SetIdContractStage(item->IdContractStage());
        } else if (item->IdContract()) {
            w.SetIdContract(item->IdContract());
        }
    } else {
        // existing check
        w.SetIdCheck(aIdCheck);
    }

    QSqlQuery query(db);

//    if (item->IdContractStage())
//        query.prepare("select 100 - nvl(sum(donepercent), 0) from v_pkz_hashbon where id_contract_stage = " + QString::number(item->IdContractStage())
//                      + " and id <> " + QString::number(aIdCheck));
//    else
//        query.prepare("select 100 - nvl(sum(donepercent), 0) from v_pkz_hashbon where id_contract = " + QString::number(item->IdContract())
//                      + " and id <> " + QString::number(aIdCheck));
//    if (!query.exec() || !query.next()) {
//        QMessageBox::critical(this, "חוזים", query.lastError().text());
//        return;
//    } else {
//        if (aIdCheck || query.value(0).toInt() > 0)
//            w.SetMaxPercent(query.value(0).toInt());
//        else {
//            QMessageBox::critical(this, "חוזים",
//                                  "100 percent in hashbones!");
//            return;
//        }
//    }

    w.SetMaxPercent(100);

    if (item->IdContractStage()) {
        // rest for contract, not for stage only
        query.prepare(
                    "select sum_brutto -"
                    "  (select nvl(sum(nvl(nvl(b.pay_sum_brutto, b.sign_sum_brutto), b.orig_sum_brutto)), 0)"
                    "    from v_pkz_hashbon b, v_pkz_contract_stage c"
                    "      where c.id_pkz_contract = " + QString::number(((QContractTreeItem *) item->parent())->IdContract()) +
                    "      and b.id_contract_stage = c.id"
                    "      and b.id <> " + QString::number(aIdCheck) + ")"
                    " from v_pkz_contract_sum a where id_contract = " + QString::number(((QContractTreeItem *) item->parent())->IdContract()) +
                    " and order_num = (select max(order_num) from v_pkz_contract_sum where id_contract = a.id_contract)");

    } else {
        query.prepare(
                    "select sum_brutto -"
                    "  (select nvl(sum(nvl(nvl(pay_sum_brutto, sign_sum_brutto), orig_sum_brutto)), 0) from v_pkz_hashbon"
                    "    where id_contract = " + QString::number(item->IdContract()) +
                    "    and id <> " + QString::number(aIdCheck) + ")"
                    " from v_pkz_contract_sum a where id_contract = " + QString::number(item->IdContract()) +
                    " and order_num = (select max(order_num) from v_pkz_contract_sum where id_contract = a.id_contract)");
    }

    if (!query.exec() || !query.next()) {
        gLogger->ShowSqlError(this, "חוזים", query);
        return;
    } else {
        if (aIdCheck || query.value(0).toLongLong() > 0)
            w.SetMaxSumBrutto(query.value(0).toLongLong());
        else {
            QMessageBox::critical(this, "חוזים", "Full sum in hashbones!");
            return;
        }
    }

    w.SetFullSum(item->Sum());

    if (item->IdContractStage()) {
        w.SetContractDate(((QContractTreeItem *) item->parent())->StartDate());
        w.SetIndexingType(((QContractTreeItem *) item->parent())->IndexingType());
        w.SetContractNum(((QContractTreeItem *) item->parent())->Num());
    } else if (item->IdContract()) {
        w.SetContractDate(item->StartDate());
        w.SetIndexingType(item->IndexingType());
        w.SetContractNum(item->Num());
    }

    if (w.exec() == QDialog::Accepted) {
        if (w.GetUpdateId()) {
            if (item->IdContractStage()) {
                ((QContractTreeItem *) item->parent())->ReloadContractFromBase(0);
                item->ReloadContractStageFromBase(0);
                if (w.GetUpdateIdOther()) {
                    int i;
                    QContractTreeItem *itemParent = (QContractTreeItem *) item->parent(), *itemChild;
                    for (i = 0; i < itemParent->childCount(); i++) {
                        itemChild = (QContractTreeItem *) itemParent->child(i);
                        if (itemChild->IdContractStage() == w.GetUpdateIdOther()) {
                            itemChild->ReloadContractStageFromBase(0);
                            break;
                        }

                    }
                }
            } else if (item->IdContract()) {
                item->ReloadContractFromBase(0);
            }
            ui->treeHashbon->PopulateTree(item->IdContract(), item->IdContractStage(), item->Sum());
        }
    }
}

void ContractPkz::DoSelectColumnsTop(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    //w.setLayoutDirection(layoutDirection());
    w.SetHeaderView(ui->treeWidget->header(), ui->treeWidget->header()->count() - 2);
    w.exec();
}

void ContractPkz::DoSelectColumnsHashbon(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QPoint(QCursor::pos().x(), QCursor::pos().y() - w.height()));
    //w.setLayoutDirection(layoutDirection());
    w.SetHeaderView(ui->treeHashbon->header(), ui->treeHashbon->header()->count() - 4);
    w.exec();
}

void ContractPkz::on_actionAdd_contract_triggered() {
    ShowProps(NULL);
}

void ContractPkz::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (((QContractTreeItem *) item)->IdContract()) {
        ShowProps((QContractTreeItem *) item);
    } else if (((QContractTreeItem *) item)->IdContractStage()) {
        ShowStageProps((QContractTreeItem *) item, NULL);
    }
}

void ContractPkz::on_actionProp_contract_triggered() {
    QContractTreeItem *item = (QContractTreeItem *) ui->treeWidget->currentItem();
    if (item) {
        if (item->IdContract()) ShowProps(item);
    }
}

void ContractPkz::on_treeWidget_customContextMenuRequested(const QPoint &pos) {
    //if (pos.x() > ui->treeWidget->width() - ui->treeWidget->indentation()) {
    if (ui->treeWidget->columnAt(pos.x()) == 0) {
        QMenu popMenu(this);
        popMenu.setLayoutDirection(this->layoutDirection());
        popMenu.addAction(ui->actionExpand_all);
        popMenu.addAction(ui->actionExpand_projects);
        popMenu.addAction(ui->actionCollapse_all);
        if (!gSettings->Contract.MultiSelect) {
            popMenu.addSeparator();
            popMenu.addAction(ui->actionSelect_all);
            popMenu.addAction(ui->actionSelect_none);
            popMenu.addAction(ui->actionInvert_selection);
        }
        popMenu.exec(QCursor::pos());
    } else {
        QContractTreeItem *item;
        item = (QContractTreeItem *) ui->treeWidget->itemAt(pos);
        if (item) {
            QMenu popMenu(this);
            if (item->IdContract()) {
                popMenu.addAction(ui->actionProp_contract);
                popMenu.setDefaultAction(ui->actionProp_contract);
                popMenu.addSeparator();
                popMenu.addAction(ui->actionDel_contract);

                if (!item->IdPlot()) {
                    if (gUserRight->CanInsert("v_plot2pkz_contract")) {
                        popMenu.addSeparator();
                        popMenu.addAction(ui->actionCntrAttach_file);
                    }
                } else {
                    bool lSepAdded = false;
                    if (gUserRight->CanSelect("v_plot2pkz_contract")) {
                        popMenu.addSeparator();
                        popMenu.addAction(ui->actionCntrView_file);
                        lSepAdded = true;
                    }
                    if (gUserRight->CanUpdate("v_plot2pkz_contract", "deleted")) {
                        if (!lSepAdded) {
                            popMenu.addSeparator();
                        }
                        popMenu.addAction(ui->actionCntrRemove_file);

                    }
                }
            }
            if (item->IdContractStage()) {
                popMenu.addAction(ui->actionProp_stage);
                popMenu.addSeparator();
                popMenu.addAction(ui->actionDel_stage);
            }
            if (!popMenu.actions().isEmpty()) {
                popMenu.exec(QCursor::pos());
            }
        }
    }
}

void ContractPkz::on_actionDel_contract_triggered() {
    QContractTreeItem *item = (QContractTreeItem *) ui->treeWidget->currentItem();
    if (item && item->IdContract()) {
        if (QMessageBox::question(this, "חוזים", QString("Remove contract ") + item->text(2) + "?") == QMessageBox::Yes) {
            QSqlQuery qDelete("delete from v_pkz_contract where id = " + QString::number(item->IdContract()), db);
            if (qDelete.lastError().isValid()) {
                gLogger->ShowSqlError(this, "חוזים", qDelete);
            } else {
                if (!qDelete.exec()) {
                    gLogger->ShowSqlError(this, "חוזים", qDelete);
                } else {
                    QContractTreeItem *parentItem = (QContractTreeItem *) item->parent();
                    // we can do it
                    delete item;
                    if (!parentItem->childCount()) delete parentItem;
                }
            }
        }
    }
}

void ContractPkz::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    QContractTreeItem *item = (QContractTreeItem *) current;
    if (item) {
        ui->tbMinus->setEnabled(item->IdContract() != 0);
        ui->tbProps->setEnabled(item->IdContract() != 0);
        ui->tbStagePlus->setEnabled(item->IdContract() != 0 || item->IdContractStage() != 0);
        ui->tbStageMinus->setEnabled(item->IdContractStage() != 0);
        ui->tbStageProps->setEnabled(item->IdContractStage() != 0);

        ui->tbHashbonPlus->setEnabled(item->IdContractStage() || item->IdContract() && !item->childCount());

        if (item->IdContractStage())
            ui->treeHashbon->PopulateTree(0, item->IdContractStage(), item->Sum());
        else if (item->IdContract() && !item->childCount())
            ui->treeHashbon->PopulateTree(item->IdContract(), 0, item->Sum());
        else
            ui->treeHashbon->ClearTree();
    } else {
        ui->tbMinus->setEnabled(false);
        ui->tbProps->setEnabled(false);
        ui->tbStagePlus->setEnabled(false);
        ui->tbStageMinus->setEnabled(false);
        ui->tbStageProps->setEnabled(false);

        ui->treeHashbon->ClearTree();

        ui->tbHashbonPlus->setEnabled(false);
        ui->tbHashbonProps->setEnabled(false);
        ui->tbHashbonMinus->setEnabled(false);
    }
    on_treeHashbon_currentItemChanged(ui->treeHashbon->currentItem(), NULL);
}

void ContractPkz::on_actionCollapse_all_triggered() {
    ui->treeWidget->collapseAll();
}

void ContractPkz::on_actionExpand_all_triggered() {
    ui->treeWidget->expandAll();
}

void ContractPkz::CollapseChilds(QTreeWidgetItem *aItem) {
    for (int i = 0; i < aItem->childCount(); i++) {
        aItem->child(i)->setExpanded(false);
    }
}

void ContractPkz::on_actionExpand_projects_triggered() {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        ui->treeWidget->topLevelItem(i)->setExpanded(true);
        CollapseChilds(ui->treeWidget->topLevelItem(i));
    }
}

void ContractPkz::on_tbStageProps_clicked() {
    QContractTreeItem *item = (QContractTreeItem *) ui->treeWidget->currentItem();
    if (item && item->IdContractStage()) {
        ShowStageProps(item, NULL);
    }
}

void ContractPkz::on_tbHashbonShow_clicked() {
    ui->dockHashbon->setVisible(ui->tbHashbonShow->isChecked());
}

void ContractPkz::on_dockWidget_2_topLevelChanged(bool topLevel) {
    if (topLevel) {
        //QMessageBox::critical(this, "חוזים", QString::number(ui->dockWidget_2->width()));
        QRect r = ui->dockWidget_2->geometry();
        r.setWidth(10);
        ui->dockWidget_2->setGeometry(r);
        //ui->dockWidget_2->setWindowFlags(ui->dockWidget_2->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
        //ui->dockWidget_2->setFixedSize(ui->dockWidget_2->width(), ui->dockWidget_2->height());
        ui->dockWidget_2->setFixedHeight(ui->dockWidget_2->height());
        //ui->dockWidget_2->setMaximumHeight(ui->dockWidget_2->minimumHeight());
    }
}

void ContractPkz::on_dockHashbon_visibilityChanged(bool visible) {
    ui->tbHashbonShow->setChecked(visible);
}

void ContractPkz::on_actionAdd_hashbon_triggered() {
    ShowCheckProps(0);
}

void ContractPkz::on_treeHashbon_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    QTreeWidgetItem *item = current;
    if (item) {
        ui->tbHashbonProps->setEnabled(true);
        ui->tbHashbonMinus->setEnabled(true);
    } else {
        ui->tbHashbonProps->setEnabled(false);
        ui->tbHashbonMinus->setEnabled(false);
    }
}

void ContractPkz::on_actionProp_hashbon_triggered() {
    QHashbonTreeItem *item = (QHashbonTreeItem *) ui->treeHashbon->currentItem();
    if (item && item->IdCheck()) {
        ShowCheckProps(item->IdCheck());
    }
}

void ContractPkz::on_treeHashbon_customContextMenuRequested(const QPoint &pos) {
    QMenu popMenu(this);
    if (ui->tbHashbonPlus->isEnabled()) popMenu.addAction(ui->actionAdd_hashbon);
    if (ui->tbHashbonProps->isEnabled()) popMenu.addAction(ui->actionProp_hashbon);
    if (ui->tbHashbonMinus->isEnabled()) {
        popMenu.addSeparator();
        popMenu.addAction(ui->actionDel_hashbon);
    }
    if (!popMenu.actions().isEmpty()) popMenu.exec(QCursor::pos());
}

void ContractPkz::on_actionAdd_stage_triggered() {
    QContractTreeItem *item = (QContractTreeItem *) ui->treeWidget->currentItem();
    if (item) {
        if (item->IdContract())
            ShowStageProps(NULL, item);
        else if (item->parent() && ((QContractTreeItem *) item->parent())->IdContract())
            ShowStageProps(NULL, (QContractTreeItem *) item->parent());
    }
}

void ContractPkz::on_actionProp_stage_triggered() {
    QContractTreeItem * item = (QContractTreeItem *) ui->treeWidget->currentItem();
    if (item->IdContractStage()) {
        ShowStageProps(item, NULL);
    }
}

void ContractPkz::on_actionDel_stage_triggered() {
    QContractTreeItem * item = (QContractTreeItem *) ui->treeWidget->currentItem();
    if (item && item->IdContractStage()) {
        if (QMessageBox::question(this, "חוזים", QString("Remove stage ") + item->text(2) + "?") == QMessageBox::Yes) {
            QSqlQuery qDelete("delete from v_pkz_contract_stage where id = " + QString::number(item->IdContractStage()), db);
            if (qDelete.lastError().isValid()) {
                gLogger->ShowSqlError(this, "חוזים", qDelete);
            } else {
                if (!qDelete.exec()) {
                    gLogger->ShowSqlError(this, "חוזים", qDelete);
                } else {
                    // we can do it
                    delete item;
                }
            }
        }
    }
}

void ContractPkz::on_tbSettings_clicked() {
    ContractDefVAT w(this);
    if (w.exec() == QDialog::Accepted) {
        ui->treeWidget->setSelectionMode(gSettings->Contract.MultiSelect?QAbstractItemView::ExtendedSelection:QAbstractItemView::SingleSelection);
        if (w.AnyChanged)
            ui->treeWidget->PopulateTree();
    }
}

void ContractPkz::on_treeHashbon_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (((QHashbonTreeItem *) item)->IdCheck()) {
        ShowCheckProps(((QHashbonTreeItem *) item)->IdCheck());
    }
}

//http://www1.cbs.gov.il/reader/?MIval=%2Fprices_db%2FMachshevon_Results_E.html&MD=d&MySubject=53&MyCode=11240010&MultMin=19830216&MultMax=20131201&DateMin=16%2F02%2F1983&DateMax=01%2F12%2F2013&ssum=5000&koeff=1&Days_1=16&Months_1=6&Years_1=2009&Days_2=1&Months_2=12&Years_2=2013

//MIval=%2Fprices_db%2FMachshevon_Results_E.html
//MD=d
//MySubject=53
//MyCode=11240010
//MultMin=19830216
//MultMax=20131201
//DateMin=16%2F02%2F1983
//DateMax=01%2F12%2F2013
//ssum=5000
//koeff=1
//Days_1=16
//Months_1=6
//Years_1=2009
//Days_2=1
//Months_2=12
//Years_2=2013

void ContractPkz::on_actionDel_hashbon_triggered() {
    QHashbonTreeItem * item = (QHashbonTreeItem *) ui->treeHashbon->currentItem();
    if (item) {
        if (QMessageBox::question(this, "חוזים", QString("Remove hashbon ") + item->text(1) + "?") == QMessageBox::Yes) {
            QSqlQuery qDelete("delete from v_pkz_hashbon where id = " + QString::number(item->IdCheck()), db);
            if (qDelete.lastError().isValid()) {
                gLogger->ShowSqlError(this, "חוזים", qDelete);
            } else {
                if (!qDelete.exec()) {
                    gLogger->ShowSqlError(this, "חוזים", qDelete);
                } else {
                    // we can do it
                    delete item;
                    on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), NULL);
                }
            }
        }
    }

}

void ContractPkz::on_tbReports_clicked() {
    QMenu popMenu(this);

    QSqlQuery query("select distinct to_char(nvl(pay_date, expect_date), 'yyyy') from v_pkz_hashbon"
                    " where pay_date is not null or expect_date is not null order by 1 desc", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", query);
    } else {
        QMenu *popYearsAll, *popYearsSelected = NULL, *popMonthlyAll, *popMonthlySelected = NULL;

        QList<int> lProjects;
        GetSelectedProjects(lProjects);

        QSqlQuery query2(db);
        bool lIsQ2 = false;
        if (!lProjects.isEmpty()) {
            // select years for selected projects
            QStringList lProjectsStr;
            for (int i = 0; i < lProjects.length(); i++) {
                lProjectsStr.append(QString::number(lProjects.at(i)));
            }
            query2.prepare("select distinct to_char(nvl(pay_date, expect_date), 'yyyy')"
                    " from v_pkz_hashbon a, v_pkz_contract b"
                    " where (a.pay_date is not null or a.expect_date is not null)"
                    " and a.id_contract = b.id"
                    " and b.id_project in (" + lProjectsStr.join(" ,") + ")"
                    " union"
                    " select distinct to_char(nvl(pay_date, expect_date), 'yyyy')"
                    " from v_pkz_hashbon a, v_pkz_contract_stage c, v_pkz_contract b"
                    " where (a.pay_date is not null or a.expect_date is not null)"
                    " and a.id_contract_stage = c.id"
                    " and c.id_pkz_contract = b.id"
                    " and b.id_project in (" + lProjectsStr.join(" ,") + ")"
                    "order by 1 desc");
            if (query2.lastError().isValid()) {
                gLogger->ShowSqlError(this, "חוזים", query2);
            } else {
                if (!query2.exec()) {
                    gLogger->ShowSqlError(this, "חוזים", query2);
                } else {
                    lIsQ2 = true;
                }
            }
        }

        popMenu.addAction(ui->actionFull_all);
        if (!lProjects.isEmpty()) {
            popMenu.addAction(ui->actionFull_selected);
        }

//        popMenu.addAction(ui->actionReport_Common);
//        if (!lProjects.isEmpty()) {
//            popMenu.addAction(ui->actionReport_common_selected);
//        }

        popYearsAll = popMenu.addMenu("Summary by year - all");
        if (!lProjects.isEmpty()) {
            popYearsSelected = popMenu.addMenu("Summary by year - selected");
        }
        popMenu.addAction(ui->actionReport_ProjByYear);
        popMonthlyAll = popMenu.addMenu("Monthly payments - all");
        if (!lProjects.isEmpty()) {
            popMonthlySelected = popMenu.addMenu("Monthly payments - selected");
        }

        while (query.next()) {
            popYearsAll->addAction(new QAction(query.value(0).toString(), popYearsAll));
            popMonthlyAll->addAction(new QAction(query.value(0).toString(), popMonthlyAll));
        }

        if (lIsQ2) {
            while (query2.next()) {
                popYearsSelected->addAction(new QAction(query2.value(0).toString(), popYearsSelected));
                popMonthlySelected->addAction(new QAction(query2.value(0).toString(), popMonthlySelected));
            }
        }

        popMenu.addAction(ui->actionPayments_by_customer);
        popMenu.addAction(ui->actionSigned_not_payed);

        QAction *a;
        if (a = popMenu.exec(ui->tbReports->mapToGlobal(ui->tbReports->rect().bottomLeft()))) {
            bool lOk;
            int lYear = a->text().toInt(&lOk);
            if (lOk && lYear) {
                if (a->parent() == popYearsAll) {
                    lProjects.clear();
                    SummaryByYear(lYear, true, lProjects);
                } else if (a->parent() == popYearsSelected) {
                    SummaryByYear(lYear, true, lProjects);
                } else if (a->parent() == popMonthlyAll) {
                    lProjects.clear();
                    MonthlyPayments(lYear, true, lProjects);
                } else if (a->parent() == popMonthlySelected) {
                    MonthlyPayments(lYear, true, lProjects);
                }
            }
        }
    }
}

void ContractPkz::on_actionReport_Common_triggered() {
    ReportCommonOld(true);
}

void ContractPkz::on_actionSelect_all_triggered() {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
        ui->treeWidget->topLevelItem(i)->setCheckState(0, Qt::Checked);
}

void ContractPkz::on_actionSelect_none_triggered() {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
        ui->treeWidget->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
}

void ContractPkz::on_actionInvert_selection_triggered() {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
        if (ui->treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked)
            ui->treeWidget->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
        else
            ui->treeWidget->topLevelItem(i)->setCheckState(0, Qt::Checked);
}

void ContractPkz::on_actionReport_common_selected_triggered() {
    ReportCommonOld(false);
}

void ContractPkz::on_actionReport_ProjByYear_triggered() {
    QList<int> lProjects;
    GetSelectedProjects(lProjects);
    ReportProjPayByYears(true, lProjects);
}

void ContractPkz::on_actionPayments_by_customer_triggered() {
    PayByCustParams w(this);
    if (w.exec() == QDialog::Accepted) {
        QStringList lCusts = w.Selected();
        if (lCusts.length())
            ReportPayByCusts(true, w.Selected());
        else
            QMessageBox::critical(this, "Contracts - report", "Nothing selected!");
    }
}

void ContractPkz::on_actionCntrView_file_triggered() {
    QContractTreeItem * item = (QContractTreeItem *) ui->treeWidget->currentItem();
    if (item
            && item->IdContract()
            && item->IdPlot()) {
        gSettings->DoOpenNonDwg(item->IdPlot(), 1, 0, "");
    }
}

void ContractPkz::on_actionCntrAttach_file_triggered() {
    QContractTreeItem * item = static_cast<QContractTreeItem *>(ui->treeWidget->currentItem());
    if (item
            && item->IdContract()
            && item->parent()) {
        QString lFileName;
        if (SelectFileForAttach(lFileName)) {
            if (db.transaction()) {
                bool b;
                if (b = AttachFile(item->IdContract(), static_cast<QContractTreeItem *>(item->parent())->IdProject(), lFileName)) {
                    if ((b = db.commit())) {
                        item->ReloadContractFromBase(0);
                    } else {
                        gLogger->ShowSqlError(this, "חוזה", tr("Can't commit"), db);
                    }
                }
                if (!b) {
                    db.rollback();
                }
            } else {
                gLogger->ShowSqlError(this, "חוזה", tr("Can't start transaction"), db);
            }
        }
    }
}

void ContractPkz::on_actionCntrRemove_file_triggered() {
    QContractTreeItem * item = static_cast<QContractTreeItem *>(ui->treeWidget->currentItem());
    if (item
            && item->IdContract()
            && item->IdPlot()) {
        if (QMessageBox::question(this, "חוזים", "Remove attached file?") == QMessageBox::Yes) {
            if (db.transaction()) {
                bool b;
                if (b = RemoveFile(item->IdPlot(), item->IdContract())) {
                    if ((b = db.commit())) {
                        item->ReloadContractFromBase(0);
                    } else {
                        gLogger->ShowSqlError(this, "חוזה", tr("Can't commit"), db);
                    }
                }
                if (!b) {
                    db.rollback();
                }
            } else {
                gLogger->ShowSqlError(this, "חוזה", tr("Can't start transaction"), db);
            }
        }
    }
}

bool ContractPkz::SelectFileForAttach(QString &aFileName) {
    bool res = false;

    QFileDialog dlg;
    QStringList filters;
    filters << tr("Documents") + " (*.doc *.docx *.xls *.xlsx *.jpg *.jpeg *.png)"
            << tr("Any files") + " (*)";

    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setNameFilters(filters);

    dlg.setDirectory(gSettings->LoadFiles.LastDir);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        if (files.length() == 1) {
            aFileName = files.at(0);
            gSettings->LoadFiles.LastDir = aFileName.left(aFileName.lastIndexOf('/'));
            res = true;
        }
    }

    return res;
}

bool ContractPkz::AttachFile(int aIdContract, int aIdProject, const QString &aFileName) {
    if (!aIdContract) return false;

    TreeDataRecord *lTreeData = gTreeData->FindByGroupId(18);
    if (!lTreeData) {
        gLogger->ShowError("Tree data", "Cant find TREEDATA with GROUP_ID = 18");
        return false;
    }

    QSqlQuery qSelect(db);
    qSelect.prepare("select id_plot from v_plot2pkz_contract where id_pkz_contract = :id_contract and nvl(deleted, 0) = 0");

    if (qSelect.lastError().isValid()) {
        gLogger->ShowSqlError("Attach file", qSelect);
    } else {
        qSelect.bindValue(":id_contract", aIdContract);
        if (!qSelect.exec()) {
            gLogger->ShowSqlError("Attach file", qSelect);
        } else {
            int lIdPlot = 0, lDWGMaxVersion = 0;

            if (qSelect.next()) {
                lIdPlot = qSelect.value("id_plot").toInt();

                // update existing plot
                QSqlQuery qUpdate(db);
                qUpdate.prepare("update v_plot_simple set code = :code, nametop = :nametop, name = :name where id = :id");

                if (qUpdate.lastError().isValid()) {
                    gLogger->ShowSqlError("Attach file", qUpdate);
                    return false;
                } else {
                    qUpdate.bindValue(":code", QDate::currentDate().toString("dd.MM.yy"));
                    qUpdate.bindValue(":nametop", "Loaded " + QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"));
                    qUpdate.bindValue(":name", aFileName.mid(aFileName.lastIndexOf('/') + 1));
                    qUpdate.bindValue(":id", lIdPlot);
                    if (!qUpdate.exec()) {
                        gLogger->ShowSqlError("Attach file", qUpdate);
                        return false;
                    }
                }

                PlotData *lPlot = gProjects->FindByIdPlot(lIdPlot);
                if (!lPlot) {
                    return false;
                } else {
                    lPlot->InitIdDwgMax();
                    lDWGMaxVersion = lPlot->DwgVersionMax();
                }
            } else {
                // create new PLOT record
                int lIdCommonDummy = 0;
                if (!PlotData::INSERT(lIdPlot, lIdCommonDummy, aIdProject, lTreeData->Area(), lTreeData->Id(), "1" /*ver.int*/, "1"/*ver.ext*/,
                                             ""/*complect*/, ""/*stage*/, QDate::currentDate().toString("dd.MM.yy")/*code*/, ""/*sheet*/,
                                             "Loaded " + QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"), aFileName.mid(aFileName.lastIndexOf('/') + 1),
                                             aFileName.mid(aFileName.lastIndexOf('/') + 1), ""/*notes*/)) {
                    return false;
                }

                QSqlQuery qInsert(db);
                qInsert.prepare("insert into v_plot2pkz_contract (id_plot, id_pkz_contract) values (:id_plot, :id_pkz_contract)");

                if (qInsert.lastError().isValid()) {
                    gLogger->ShowSqlError("Attach file", qInsert);
                    return false;
                } else {
                    qInsert.bindValue(":id_plot", lIdPlot);
                    qInsert.bindValue(":id_pkz_contract", aIdContract);
                    if (!qInsert.exec()) {
                        gLogger->ShowSqlError("Attach file", qInsert);
                        return false;
                    }
                }
                // insert v_plot2pkz_contract
            }
            // insert new DWG record (from file)
            if (PlotData::LOADFROMFILESIMPLE(lIdPlot, lDWGMaxVersion, aFileName)) {
                return true;
            }
        }
    }

    return false;
}

bool ContractPkz::RemoveFile(int aIdPlot, int aIdContract) {
    bool res = false;

    QSqlQuery qUpdate(db);

    qUpdate.prepare("update v_plot2pkz_contract set deleted = 1 where id_plot = :id_plot and id_pkz_contract = :id_pkz_contract");
    if (qUpdate.lastError().isValid()) {
        gLogger->ShowSqlError(NULL, "חוזה", qUpdate);
    } else {
        qUpdate.bindValue(":id_plot", aIdPlot);
        qUpdate.bindValue(":id_pkz_contract", aIdContract);
        if (!qUpdate.exec()) {
            gLogger->ShowSqlError(NULL, "חוזה", qUpdate);
        } else {
            qUpdate.prepare("update v_plot_simple set deleted = 1 where id = :id_plot");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(NULL, "חוזה", qUpdate);
            } else {
                qUpdate.bindValue(":id_plot", aIdPlot);
                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(NULL, "חוזה", qUpdate);
                } else {
                    res = true;
                }
            }
        }
    }

    return res;
}

void ContractPkz::GetSelectedProjects(QList<int> &aProjects) {
//    QList<QContractTreeItem *> items;
    int i;

    aProjects.clear();
    if (!gSettings->Contract.MultiSelect) {
        for (i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
            if (ui->treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked) {
                aProjects.append(static_cast<QContractTreeItem *>(ui->treeWidget->topLevelItem(i))->IdProject());
            }
        }
    } else {
        for (i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
            if (ui->treeWidget->topLevelItem(i)->isSelected()) {
                aProjects.append(static_cast<QContractTreeItem *>(ui->treeWidget->topLevelItem(i))->IdProject());
            }
        }
    }

//    if (!items.length()) {
//        if (ui->treeWidget->currentItem()
//                && ((QContractTreeItem *) ui->treeWidget->currentItem())->IdProject())
//            items.append((QContractTreeItem *) ui->treeWidget->currentItem());
//    }

//    if (!items.length()) {
//        QMessageBox::critical(this, "Contracts - report", "Nothing selected!");
//        return;
//    }
}

void ContractPkz::HideEmptyChildren(QTreeWidgetItem *aParent) {
    for (int i = 0; i < aParent->childCount(); i++) {
        if (ui->cbHideEmpty->isChecked()) {
            aParent->child(i)->setSelected(false);
            aParent->child(i)->setHidden(static_cast<QContractTreeItem *>(aParent->child(i))->RestThisYear()
                                         + static_cast<QContractTreeItem *>(aParent->child(i))->RestFuture() <= 10000);
        } else {
            aParent->child(i)->setHidden(false);
        }
        HideEmptyChildren(aParent->child(i));
    }
}

void ContractPkz::on_cbHideEmpty_clicked() {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        if (ui->cbHideEmpty->isChecked()) {
            ui->treeWidget->topLevelItem(i)->setSelected(false);
            ui->treeWidget->topLevelItem(i)->setHidden(static_cast<QContractTreeItem *>(ui->treeWidget->topLevelItem(i))->RestThisYear()
                                                       + static_cast<QContractTreeItem *>(ui->treeWidget->topLevelItem(i))->RestFuture() <= 10000);
        } else {
            ui->treeWidget->topLevelItem(i)->setHidden(false);
        }
        HideEmptyChildren(ui->treeWidget->topLevelItem(i));
    }
}

void ContractPkz::on_actionFull_selected_triggered() {
    QList<int> lProjects;
    GetSelectedProjects(lProjects);
    ReportFull(true, lProjects);
}

void ContractPkz::on_actionFull_all_triggered() {
    QList<int> lProjects;
    ReportFull(true, lProjects);
}

void ContractPkz::on_actionSigned_not_payed_triggered() {
    QList<int> lProjects;
    GetSelectedProjects(lProjects);
    ReportSignedNotPayed(true, lProjects);
}
