#include "PlotAttrDlg.h"
#include "ui_PlotAttrDlg.h"

#include "PlotData.h"
#include "DwgLayoutData.h"

#include "../ProjectLib/ProjectData.h"

#include "../VProject/SelectColumnsDlg.h"

#include <QPainter>
#include <QLineEdit>
#include <QTimer>

PlotAttrDlg::PlotAttrDlg(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotAttrDlg)
{
    ui->setupUi(this);

    ui->cbAutoUpdate->setChecked(true);
    InitInConstructor();
}

PlotAttrDlg::~PlotAttrDlg()
{
    delete ui;
}


void PlotAttrDlg::InitInConstructor() {
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    PlotAttrItemDelegate *lDelegate = new PlotAttrItemDelegate(this);
    ui->twAttrs->setItemDelegate(lDelegate);
    connect(lDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(onCommitData(QWidget *)));

    ui->twAttrs->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->twAttrs->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumns(const QPoint &)));
}

void PlotAttrDlg::SaveItemsForEdit() {
    mItemsForEdit = ui->twAttrs->selectedItems();
}

void PlotAttrDlg::ShowData() {
    ui->twAttrs->clear();
    int lIdPlot = -1;
    bool lDiffDocs = false, lAnyHistory = false;
    bool lFirstInDoc, lFirstInLayout, lFirstInBlock;
    bool lWhichColorDoc = false, lWhichColorLayout = false, lWhichColorBlock = false;
    QColor lFirstColor(212, 208, 220), lSecondColor(212, 228, 200);
    QBrush lBrush;
    QString lCurTag;
    QStringList lTags;

    if (ui->cbTag->currentIndex() > 0)
        lCurTag = ui->cbTag->currentText();

    ui->cbTag->clear();

    foreach (PlotAndHistoryData lPair, mData) {
        lFirstInDoc = true;
        if (lIdPlot == -1) {
            lIdPlot = lPair.first->Id();
        } else {
            if (lIdPlot != lPair.first->Id()) lDiffDocs = true;
        }
        foreach (const DwgLayoutData *lLayout, lPair.first->LayoutsConst()) {
            lFirstInLayout = true;
            foreach (const DwgLayoutBlockData *lBlock, lLayout->BlocksConst()) {
                lFirstInBlock = true;
                foreach (DwgLBAttrData *lAttr, lBlock->AttrsConst()) {
                    if (lFirstInDoc) {
                        lWhichColorDoc = !lWhichColorDoc;
                        lFirstInDoc = false;
                    }
                    if (lFirstInLayout) {
                        lWhichColorLayout = !lWhichColorLayout;
                        lFirstInLayout = false;
                    }
                    if (lFirstInBlock) {
                        lWhichColorBlock = !lWhichColorBlock;
                        lFirstInBlock = false;
                    }

                    if (!lTags.contains(lAttr->TagConst()))
                        lTags.append(lAttr->TagConst());

                    QTreeWidgetItem *lItem = new QTreeWidgetItem();

                    lItem->setData(0, Qt::UserRole + 0, QVariant::fromValue(lPair.first));
                    lItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(lPair.second));
                    lItem->setData(0, Qt::UserRole + 2, QVariant::fromValue(lAttr));

                    lItem->setFlags(lItem->flags() | Qt::ItemIsEditable);

                    lBrush = palette().brush(QPalette::Background);
                    if (lWhichColorDoc) {
                        lBrush.setColor(lFirstColor);
                    } else {
                        lBrush.setColor(lSecondColor);
                    }


                    lItem->setText(0, QString::number(lPair.first->Id()));
                    lItem->setTextAlignment(0, Qt::AlignRight | Qt::AlignVCenter);
                    lItem->setBackground(0, lBrush);

                    lItem->setText(1, lPair.first->CodeConst());
                    lItem->setTextAlignment(1, Qt::AlignLeft | Qt::AlignVCenter);
                    lItem->setBackground(1, lBrush);


                    lItem->setText(2, lPair.first->SheetConst());
                    lItem->setTextAlignment(2, Qt::AlignCenter);
                    lItem->setBackground(2, palette().brush(QPalette::Background));
                    lItem->setBackground(2, lBrush);

                    if (lPair.second) {
                        lItem->setText(3, QString::number(lPair.second->Num()) + "/" + QString::number(lPair.first->DwgVersionMax()));
                        lItem->setTextAlignment(3, Qt::AlignCenter);
                        lAnyHistory = true;
                    }
                    lItem->setBackground(3, lBrush);

                    lItem->setText(4, lLayout->NameConst());
                    lItem->setTextAlignment(4, Qt::AlignLeft | Qt::AlignVCenter);
                    lBrush = palette().brush(QPalette::Background);
                    if (lWhichColorLayout) {
                        lBrush.setColor(lFirstColor);
                    } else {
                        lBrush.setColor(lSecondColor);
                    }
                    lItem->setBackground(4, lBrush);
                    //lItem->setText(4, QString::number(lBrush.color().red()) + ":" + QString::number(lBrush.color().green()) + ":" + QString::number(lBrush.color().blue()));


                    lItem->setText(5, lBlock->NameConst());
                    lItem->setTextAlignment(5, Qt::AlignLeft | Qt::AlignVCenter);
                    lBrush = palette().brush(QPalette::Background);
                    if (lWhichColorBlock) {
                        lBrush.setColor(lFirstColor);
                    } else {
                        lBrush.setColor(lSecondColor);
                    }
                    lItem->setBackground(5, lBrush);

                    lItem->setText(6, lAttr->TagConst());
                    lItem->setTextAlignment(6, Qt::AlignLeft | Qt::AlignVCenter);
                    lItem->setBackground(6, palette().brush(QPalette::Background));

                    lItem->setText(7, lAttr->PromptConst());
                    lItem->setTextAlignment(7, Qt::AlignLeft | Qt::AlignVCenter);
                    lItem->setBackground(7, palette().brush(QPalette::Background));

                    lItem->setText(8, lAttr->GetValue());
                    lItem->setTextAlignment(8, Qt::AlignLeft | Qt::AlignVCenter);
                    lItem->setBackground(8, palette().brush(QPalette::Background));


                    lItem->setText(9, lAttr->EncValueConst());
                    lItem->setTextAlignment(9, Qt::AlignLeft | Qt::AlignVCenter);
                    lItem->setBackground(9, palette().brush(QPalette::Background));

                    ui->twAttrs->addTopLevelItem(lItem);
                }
            }
        }
    }

    lTags.sort(Qt::CaseInsensitive);
    lTags.push_front("<All>");

    ui->cbTag->addItems(lTags);
    if (!lCurTag.isEmpty() && lTags.contains(lCurTag))
        ui->cbTag->setCurrentText(lCurTag);

    ui->twAttrs->setColumnHidden(0, !lDiffDocs);
    ui->twAttrs->setColumnHidden(1, !lDiffDocs);
    ui->twAttrs->setColumnHidden(2, !lDiffDocs);

    ui->twAttrs->setColumnHidden(3, !lDiffDocs && !lAnyHistory);


    for (int i = 0; i < ui->twAttrs->columnCount(); i++) {
        ui->twAttrs->resizeColumnToContents(i);
    }
}

void PlotAttrDlg::PlotsSelectionChanged(QList<PlotAndHistoryData> aData) {
    mData = aData;
    ShowData();
}

void PlotAttrDlg::DoSelectColumns(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    QList<int> lDis;
    lDis << 0 << 1 << 2;

    w.SetHeaderView(ui->twAttrs->header());
    w.SetDisabledIndexes(lDis);
    if (w.exec() == QDialog::Accepted) {
        for (int i = 0; i < ui->twAttrs->columnCount(); i++) {
            ui->twAttrs->resizeColumnToContents(i);
        }
    }
}

void PlotAttrDlg::on_cbAutoUpdate_toggled(bool checked) {
    if (checked) {
        connect(gProjects, SIGNAL(PlotsBecameSelected(QList<PlotAndHistoryData>)), this, SLOT(PlotsSelectionChanged(QList<PlotAndHistoryData>)));
    } else {
        disconnect(gProjects, SIGNAL(PlotsBecameSelected(QList<PlotAndHistoryData>)), this, SLOT(PlotsSelectionChanged(QList<PlotAndHistoryData>)));
    }
}

void PlotAttrDlg::onCommitDataTimer() {
    int i;
    DwgLBAttrData *lAttr;

    for (i = 0; i < mItemsForEdit.count(); i++) {
        lAttr = mItemsForEdit.at(i)->data(0, Qt::UserRole + 2).value<DwgLBAttrData *>();
        lAttr->SetValue(mNewString);
    }
    if (db.transaction()) {
        bool lIsOk = true;
        for (i = 0; i < mItemsForEdit.count(); i++) {
            if (!mItemsForEdit.at(i)->data(0, Qt::UserRole + 2).value<DwgLBAttrData *>()->SaveData()) {
                lIsOk = false;
                break;
            }
        }

        if (lIsOk) {
            if (db.commit()) {
                for (i = 0; i < mItemsForEdit.count(); i++) {
                    mItemsForEdit.at(i)->data(0, Qt::UserRole + 2).value<DwgLBAttrData *>()->CommitEdit();
                }
            } else {
                gLogger->ShowSqlError(this, "Dwg layout block attribute data", tr("Can't commit"), db);
                lIsOk = false;
            }

        }
        if (!lIsOk) {
            db.rollback();
            for (i = 0; i < mItemsForEdit.count(); i++) {
                mItemsForEdit.at(i)->data(0, Qt::UserRole + 2).value<DwgLBAttrData *>()->RollbackEdit();
            }
        }

    } else {
        gLogger->ShowSqlError(this, "Dwg layout block attribute data", tr("Can't start transaction"), db);
    }

    for (i = 0; i < mItemsForEdit.count(); i++) {
        QTreeWidgetItem *lItem = mItemsForEdit.at(i);
        lAttr = lItem->data(0, Qt::UserRole + 2).value<DwgLBAttrData *>();
        lItem->setText(8, lAttr->GetValue());
        lItem->setText(9, lAttr->EncValueConst());
    }
}

void PlotAttrDlg::onCommitData(QWidget *editor) {
    return;
    if (qobject_cast<QLineEdit *>(editor)) {
        mNewString = qobject_cast<QLineEdit *>(editor)->text();
        QTimer::singleShot(0, this, SLOT(onCommitDataTimer()));
    }
}

//---------------------------------------------------------------
PlotAttrItemDelegate::PlotAttrItemDelegate(QWidget *parent) : ROPlotListItemDelegate(parent)
{

}

QWidget *PlotAttrItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    PlotData * lPlot = index.sibling(index.row(), 0).data(Qt::UserRole).value<PlotData *>();
    PlotHistoryData * lHistory = index.sibling(index.row(), 0).data(Qt::UserRole + 1).value<PlotHistoryData *>();
    //DwgLBAttrData * lAttr = index.sibling(index.row(), 0).data(Qt::UserRole + 2).value<DwgLBAttrData *>();

    if ((index.column() == 8 || index.column() == 9)
            && (!lHistory
                || lHistory->Num() == lPlot->DwgVersionMax())) {
        QLineEdit *l = new QLineEdit(parent);
        if (qobject_cast<PlotAttrDlg *>(this->parent()))
            qobject_cast<PlotAttrDlg *>(this->parent())->SaveItemsForEdit();

        return l;
    }
    return ROPlotListItemDelegate::createEditor(parent, option, index);
}

void PlotAttrItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    ROPlotListItemDelegate::paint(painter, option, index);

    QRect r = option.rect;
    //DwgLBAttrData * lAttr = index.sibling(index.row(), 0).data(Qt::UserRole + 2).value<DwgLBAttrData *>();

    painter->save();

    r.adjust(1, 1, -1, -1);

    //QBrush lBrush(QColor(255, 0, 0));
    //painter->fillRect(r, lBrush);

    if (index.column() > 5) {
        QPixmap lImage(r.width(), r.height());
        QLineEdit lLineEdit(index.data().toString());

        QPalette lPalette = lLineEdit.palette();
        if (option.state & QStyle::State_Selected) {
            lPalette.setColor(QPalette::Base, lPalette.color(QPalette::Highlight));
            lPalette.setColor(QPalette::Text, lPalette.color(QPalette::HighlightedText));
        } else {
            if (index.column() != 8)
                lPalette.setColor(QPalette::Base, option.palette.color(QPalette::Window));
        }
        lLineEdit.setPalette(lPalette);

        lLineEdit.resize(r.width(), r.height());
        lLineEdit.setFrame(false);
        lLineEdit.render(&lImage);
        painter->drawPixmap(r, lImage);
    }

//    if (index.column() != 8
//            || lAttr->UFValueConst().trimmed() != lAttr->EncValueConst().trimmed()) {

//        painter->setCompositionMode(QPainter::CompositionMode_Multiply);
//        painter->fillRect(r.left() - 1, r.top(), r.width() + 2, r.height() + 1, option.widget->palette().brush(QPalette::Background));
//    }
    painter->restore();
}

QSize PlotAttrItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QSize lSize = ROPlotListItemDelegate::sizeHint(option, index);
    lSize.setHeight(lSize.height() + 5);
    return lSize;
}

void PlotAttrDlg::on_cbTag_currentIndexChanged(int index) {
    QString lTag;
    if (index > 0) {
        lTag = ui->cbTag->currentText();
    }
    for (int i= 0; i < ui->twAttrs->topLevelItemCount(); i++) {
        if (!lTag.isEmpty()) {
            ui->twAttrs->topLevelItem(i)->setHidden(ui->twAttrs->topLevelItem(i)->text(6) != lTag);
        } else {
            ui->twAttrs->topLevelItem(i)->setHidden(false);
        }
    }
}
