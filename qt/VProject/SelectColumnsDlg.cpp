#include "SelectColumnsDlg.h"
#include "ui_SelectColumnsDlg.h"

#include <QMessageBox>

SelectColumnsDlg::SelectColumnsDlg(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::SelectColumnsDlg), mCount(-1), mHeaderView(NULL)
{
    ui->setupUi(this);

//    QMessageBox::critical(NULL, "", parent->objectName());

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
}

SelectColumnsDlg::SelectColumnsDlg(bool aLoadSettings, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::SelectColumnsDlg), mCount(-1), mHeaderView(NULL)
{
    mLoadSettings = aLoadSettings;
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
}

SelectColumnsDlg::~SelectColumnsDlg()
{
    delete ui;
}

void SelectColumnsDlg::showEvent(QShowEvent* event) {
    QPoint p = pos();
    QFCDialog::showEvent(event);

    move(p);

    if (!mHeaderView) return;

//    int lCount = (mCount == -1)?mHeaderView->count():mCount;

//    for (int i = 0; i < lCount; i++) {
//        QListWidgetItem *item = new QListWidgetItem(mHeaderView->model()->headerData(i, Qt::Horizontal).toString());
//        item->setCheckState(mHeaderView->isSectionHidden(i)?Qt::Unchecked:Qt::Checked);
//        if (mDisabledIndexes.contains(i))
//            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
//        ui->lwColumns->addItem(item);
//    }

    int lCount = (mCount == -1)?mHeaderView->count():mCount;

    for (int i = 0; i < mHeaderView->count(); i++) {
        int j = mHeaderView->logicalIndex(i);
        if (j == -1 || j >= lCount) continue;
        QListWidgetItem *item = new QListWidgetItem(mHeaderView->model()->headerData(j, Qt::Horizontal).toString().replace('\n', ' '));
        item->setCheckState(mHeaderView->isSectionHidden(j)?Qt::Unchecked:Qt::Checked);
        if (mDisabledIndexes.contains(j))
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, j);
        ui->lwColumns->addItem(item);
    }
}

void SelectColumnsDlg::on_buttonBox_accepted()
{
    bool lIsHidden;
    for (int i = 0; i < ui->lwColumns->count(); i++) {
        int j = ui->lwColumns->item(i)->data(Qt::UserRole).toInt();
        lIsHidden = ui->lwColumns->item(i)->checkState() == Qt::Unchecked;
        mHeaderView->setSectionHidden(j, lIsHidden);
        if (!lIsHidden && !mHeaderView->sectionSize(j)) {
            mHeaderView->resizeSection(j, 50);
        }

//        QListWidgetItem *item = new QListWidgetItem(mHeaderView->model()->headerData(i, Qt::Horizontal).toString());
//        item->setCheckState(mHeaderView->isSectionHidden(i)?Qt::Unchecked:Qt::Checked);
//        ui->lwColumns->addItem(item);
    }
}
