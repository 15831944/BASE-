#include "DepartDlg.h"
#include "ui_DepartDlg.h"

#include "DepartData.h"

#define USE_AS_STATIC
#include "../VProject/PlotListItemDelegate.h"
#include "../VProject/common.h"
#include "../VProject/SelectColumnsDlg.h"

#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>

#include <QTimer>
#include <QPainter>

DepartDlg::DepartDlg(DisplayTypeEnum aDisplayType, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::DepartDlg),
    mDisplayType(aDisplayType), mCurrentId(0), mJustStarted(true)
{
    InitInConstructor();
}

DepartDlg::DepartDlg(QSettings &aSettings, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::DepartDlg), mCurrentId(0), mJustStarted(true)
{
    mDisplayType = static_cast<DisplayTypeEnum>(aSettings.value("DisplayType", 0).toInt());
    InitInConstructor();
}


DepartDlg::~DepartDlg()
{
    delete ui;
}

void DepartDlg::InitInConstructor() {
    ui->setupUi(this);

    // always selected in normal color (not gray)
    QPalette lPalette = ui->twDepart->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->twDepart->setPalette(lPalette);
    // ---------------------------------

    DepartListItemDelegate *lItemDelegate = new DepartListItemDelegate(this);
    ui->twDepart->setItemDelegate(lItemDelegate);
    connect(lItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));

    ui->twDepart->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->twDepart->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumns(const QPoint &)));
    ui->twDepart->header()->setSectionsMovable(false);

    connect(gDeparts, SIGNAL(DepartsNeedUpdate()), this, SLOT(OnNeedUpdate()));

    gDeparts->InitDepartList();
}

void DepartDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;
        ui->wdEdit->setVisible(mDisplayType == DTEdit);
        if (ReadVersion < CurrentVersion) {
            ui->twDepart->setColumnHidden(0, true);
        }
    }
}

void DepartDlg::SaveState(QSettings &aSettings) {
    SaveSettings(aSettings);
    aSettings.setValue("DisplayType", mDisplayType);
}

void DepartDlg::OnNeedUpdate() {
    ShowData();
}

void DepartDlg::DoSelectColumns(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    QList<int> lDis;
    lDis << 1;

    w.SetHeaderView(ui->twDepart->header());
    w.SetDisabledIndexes(lDis);

    if (w.exec() == QDialog::Accepted) {
        if (!ui->twDepart->isColumnHidden(0)) ui->twDepart->resizeColumnToContents(0);
    }
}

void DepartDlg::OnCommitDataTimer() {
    if (!ui->twDepart->currentItem()) return;
    DepartData * lDepart = gDeparts->FindById(ui->twDepart->currentItem()->text(0).toInt());
    if (!lDepart) {
        gLogger->ShowError(this, tr("Department data"), tr("Can't find department with id") + " = " + ui->twDepart->currentItem()->text(0));
        return;
    }

    if (lDepart->NameConst() != mStrFromEdit) {
        lDepart->setName(mStrFromEdit);
        if (lDepart->SaveData()) {
            gDeparts->InitDepartList();
        } else {
            lDepart->RollbackEdit();
        }
    }
}

void DepartDlg::OnCommitData(QWidget *editor) {
    if (qobject_cast<QLineEdit *> (editor)) {
        mStrFromEdit = qobject_cast<QLineEdit *> (editor)->text().trimmed();
        QTimer::singleShot(0, this, SLOT(OnCommitDataTimer()));
    }
}

void DepartDlg::ShowData() {
    if (!mCurrentId
            && ui->twDepart->currentItem()) {
        mCurrentId = ui->twDepart->currentItem()->text(0).toInt();
    }

    ui->twDepart->clear();

    foreach (const DepartData * lDepart, gDeparts->DepartListConst()) {
        QTreeWidgetItem *lItem = new QTreeWidgetItem();
        if (mDisplayType == DTEdit) lItem->setFlags(lItem->flags() | Qt::ItemIsEditable);
        lItem->setText(0, QString::number(lDepart->Id()));
        lItem->setTextAlignment(0, Qt::AlignRight | Qt::AlignVCenter);
        lItem->setText(1, lDepart->NameConst());
        ui->twDepart->addTopLevelItem(lItem);
        if (lDepart->Id() == mCurrentId) {
            ui->twDepart->setCurrentItem(lItem, 1);
        }
    }

    if (!ui->twDepart->currentItem()) on_twDepart_currentItemChanged(NULL, NULL);

    mCurrentId = 0;
    if (!ui->twDepart->isColumnHidden(0)) ui->twDepart->resizeColumnToContents(0);
}

void DepartDlg::on_twDepart_customContextMenuRequested(const QPoint &pos) {
    if (mDisplayType == DTEdit) {
        QMenu lMenu(this);

        lMenu.addAction(ui->actionNew);

        if (ui->twDepart->currentItem()) {
            lMenu.addAction(ui->actionRename);
            lMenu.addAction(ui->actionDelete);
        }
        lMenu.exec(QCursor::pos());
    }
}

void DepartDlg::on_actionNew_triggered() {
    QInputDialog lDlg(this);
    lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    lDlg.setWindowTitle(tr("New department"));
    lDlg.setLabelText(tr("Enter name for new department"));
    if (lDlg.exec() == QDialog::Accepted
            && !lDlg.textValue().isEmpty()) {
        DepartData lDepart(0, lDlg.textValue());
        if (lDepart.SaveData()) {
            mCurrentId = lDepart.Id();
            gDeparts->InitDepartList();
        }
    }
}

void DepartDlg::on_actionRename_triggered() {
    if (!ui->twDepart->currentItem()) return;
    DepartData * lDepart = gDeparts->FindById(ui->twDepart->currentItem()->text(0).toInt());
    if (!lDepart) {
        gLogger->ShowError(this, tr("Department data"), tr("Can't find department with id") + " = " + ui->twDepart->currentItem()->text(0));
        return;
    }

    QInputDialog lDlg(this);
    lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    lDlg.setWindowTitle(tr("Renaming department"));
    lDlg.setLabelText(tr("Enter new name for department") + " '" + lDepart->NameConst() + "'");
    lDlg.setTextValue(lDepart->NameConst());
    if (lDlg.exec() == QDialog::Accepted
            && !lDlg.textValue().isEmpty()) {
        lDepart->setName(lDlg.textValue());
        if (lDepart->SaveData()) {
            gDeparts->InitDepartList();
        } else {
            lDepart->RollbackEdit();
        }
    }
}

void DepartDlg::on_actionDelete_triggered() {
    if (!ui->twDepart->currentItem()) return;
    DepartData * lDepart = gDeparts->FindById(ui->twDepart->currentItem()->text(0).toInt());
    if (!lDepart) {
        gLogger->ShowError(this, tr("Department data"), tr("Can't find department with id") + " = " + ui->twDepart->currentItem()->text(0));
        return;
    }
    if (QMessageBox::question(this, tr("Department data"), tr("Delete department") + " '" + lDepart->NameConst() + "'?") == QMessageBox::Yes
            && lDepart->RemoveFromDB()) {
        gDeparts->InitDepartList();
    }
}

void DepartDlg::on_twDepart_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    ui->tbProps->setEnabled(current);
    ui->tbMinus->setEnabled(current);
}

//----------------------------------------------------------------------------------------------------------------------------------------
DepartListItemDelegate::DepartListItemDelegate(QWidget *parent) :
    QStyledItemDelegate(parent)
{
}

void DepartListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QStyledItemDelegate::paint(painter, option, index);

    QRect r = option.rect;

    if (/*index.column() > 0*/index.column() < ((QTreeWidget *) option.widget)->columnCount() - 1) {
        painter->save();
        if ((option.state & QStyle::State_Selected)/* && index.column() > 1*/) {
            QPen lPen(painter->pen());
            lPen.setColor(option.palette.highlightedText().color());
            painter->setPen(lPen);
        }
        painter->drawLine(r.right(), r.top(), r.right(), r.bottom());
        painter->restore();
    }

    if ((index.row() || index.parent() != QModelIndex())
            && (index.parent() == QModelIndex() || index.column() > 0))
    {
        painter->save();
        if ((option.state & QStyle::State_Selected)/* && index.column() > 1*/) {
            QPen lPen(painter->pen());
            lPen.setColor(option.palette.highlightedText().color());
            painter->setPen(lPen);
        }
        painter->drawLine(r.left(), r.top(), r.right(), r.top());
        painter->restore();
    }
}

QWidget *DepartListItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    DepartDlg *lDlg = qobject_cast<DepartDlg *> (this->parent());

    if (lDlg) {
        QLineEdit *l = new QLineEdit(parent);
        if (!index.column()
                || lDlg->DisplayType() != DepartDlg::DTEdit) l->setReadOnly(true);
        return l;
    }

    return NULL;
}
