#include "PlotListTreeChange.h"
#include "ui_PlotListTreeChange.h"

#include "common.h"

PlotListTreeChange::PlotListTreeChange(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::PlotListTreeChange),
    mJustStarted(true)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->twRes->setItemDelegate(new ROPlotListItemDelegate(ui->twRes));
}

PlotListTreeChange::~PlotListTreeChange()
{
    delete ui;
}

void PlotListTreeChange::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;
        if (ReadVersion < CurrentVersion) {
            ui->twRes->setColumnWidth(0, 100);
            ui->twRes->setColumnWidth(1, (ui->twRes->width() - 80) / 2);
        }
    }
}

bool PlotListTreeChange::SetData(PlotListTree::PLTCols aColumn, const QList <QTreeWidgetItem *> & aItems) {
    bool res = false;
    mColumn = aColumn;
    switch (mColumn) {
    case PlotListTree::colCode:
        ui->lblFieldName->setText(tr("Code"));
        break;
    case PlotListTree::colSheet:
        ui->lblFieldName->setText(tr("Sheet"));
        break;
    case PlotListTree::colNameTop:
        ui->lblFieldName->setText(tr("Name top"));
        break;
    case PlotListTree::colNameBottom:
        ui->lblFieldName->setText(tr("Name bottom"));
        break;
    case PlotListTree::colComments:
        ui->lblFieldName->setText(tr("Notes"));
        break;
    case PlotListTree::colSection:
        ui->lblFieldName->setText(tr("Complect"));
        break;
    }

    for (int i = 0; i < aItems.length(); i++) {
        if (static_cast<PlotListTreeItem *>(aItems.at(i))->PlotConst()
                && !static_cast<PlotListTreeItem *>(aItems.at(i))->PlotRef()->IsSent()) {
            PlotData * lPlot = static_cast<PlotListTreeItem *>(aItems.at(i))->PlotRef();

            if (!mProjectIds.contains(lPlot->IdProject())) mProjectIds.append(lPlot->IdProject());

            QTreeWidgetItem *lItem = new QTreeWidgetItem();

            lItem->setFlags(lItem->flags() | Qt::ItemIsEditable);
            lItem->setData(0, Qt::UserRole, QVariant::fromValue(lPlot));

            lItem->setText(0, QString::number(lPlot->Id()));
            lItem->setTextAlignment(0, Qt::AlignRight | Qt::AlignTop);

            switch (mColumn) {
            case PlotListTree::colCode:
                lItem->setText(1, lPlot->CodeConst());
                break;
            case PlotListTree::colSheet:
                lItem->setText(1, lPlot->SheetConst());
                break;
            case PlotListTree::colNameTop:
                lItem->setText(1, lPlot->NameTopConst());
                break;
            case PlotListTree::colNameBottom:
                lItem->setText(1, lPlot->NameConst());
                break;
            case PlotListTree::colComments:
                lItem->setText(1, lPlot->NotesConst());
                break;
            case PlotListTree::colSection:
                lItem->setText(1, lPlot->SectionConst());
                break;
            }

            ui->twRes->addTopLevelItem(lItem);

            res = true; // at least one document's data exists
        }
    }
    OnChange();

    return res;
}

void PlotListTreeChange::OnChange() {
    for (int i = 0; i < ui->twRes->topLevelItemCount(); i++) {
        ui->twRes->topLevelItem(i)->setText(2, ui->twRes->topLevelItem(i)->text(1).replace(ui->leFrom->text(), ui->leTo->text()));
    }
}

void PlotListTreeChange::on_leFrom_textEdited(const QString &arg1) {
    OnChange();
}

void PlotListTreeChange::on_leTo_textEdited(const QString &arg1) {
    OnChange();
}

void PlotListTreeChange::Accept() {
    if (!db.transaction()) {
        gLogger->ShowSqlError(this, tr("Replace in properties"), tr("Can't start transaction"), db);
        return;
    }

    bool lIsOk = true;

    for (int i = 0; i < ui->twRes->topLevelItemCount(); i++) {
        bool lNeedSave = false;
        QTreeWidgetItem * lItem = ui->twRes->topLevelItem(i);
        PlotData * lPlot = lItem->data(0, Qt::UserRole).value<PlotData *>();

        switch (mColumn) {
        case PlotListTree::colCode:
            if (lPlot->CodeConst() != lItem->text(2)) {
                lPlot->CodeRef() = lItem->text(2);
                lNeedSave = true;
            }
            break;
        case PlotListTree::colSheet:
            if (lPlot->SheetConst() != lItem->text(2)) {
                lPlot->SheetRef() = lItem->text(2);
                lNeedSave = true;
            }
            break;
        case PlotListTree::colNameTop:
            if (lPlot->NameTopConst() != lItem->text(2)) {
                lPlot->NameTopRef() = lItem->text(2);
                lNeedSave = true;
            }
            break;
        case PlotListTree::colNameBottom:
            if (lPlot->NameConst() != lItem->text(2)) {
                lPlot->NameRef() = lItem->text(2);
                lNeedSave = true;
            }
            break;
        case PlotListTree::colComments:
            if (lPlot->NotesConst() != lItem->text(2)) {
                lPlot->NotesRef() = lItem->text(2);
                lNeedSave = true;
            }
            break;
        case PlotListTree::colSection:
            if (lPlot->SectionConst() != lItem->text(2)) {
                lPlot->SectionRef() = lItem->text(2);
                lNeedSave = true;
            }
            break;
        }

        if (lNeedSave) {
            if (!(lIsOk = lPlot->SaveData())) {
                break;
            }
        }
    }

    if (lIsOk) {
        if (!db.commit()) {
            gLogger->ShowSqlError(this, tr("Replace in properties"), tr("Can't commit"), db);
            lIsOk = false;
        }
    }
    if (lIsOk) {
        for (int i = 0; i < ui->twRes->topLevelItemCount(); i++) {
            ui->twRes->topLevelItem(i)->data(0, Qt::UserRole).value<PlotData *>()->CommitEdit();
        }

        accept();
    } else {
        for (int i = 0; i < ui->twRes->topLevelItemCount(); i++) {
            ui->twRes->topLevelItem(i)->data(0, Qt::UserRole).value<PlotData *>()->RollbackEdit();
        }
        db.rollback();
    }
}
