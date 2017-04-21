#include "common.h"
#include "typetreeselect.h"
#include "ui_typetreeselect.h"
#include "PlotTree.h"

TypeTreeSelect::TypeTreeSelect(const TreeDataRecord *aSelected, QWidget *parent) :
    QDialog(parent),
    mArea(-1), mType(-1),
    ui(new Ui::TypeTreeSelect)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    if (aSelected) ui->treeWidget->SetSelected(aSelected);
    ui->treeWidget->PopulateTree();
}

TypeTreeSelect::~TypeTreeSelect() {
    delete ui;
}

//void TypeTreeSelect::SetSelected(const TreeDataRecord * aTreeDataRecord) {
//    ui->treeWidget->SetSelected(aTreeDataRecord);
//}

TreeDataRecord * TypeTreeSelect::GetSelected() {
    return ui->treeWidget->GetSelected();
}

void TypeTreeSelect::on_buttonBox_accepted() {
    if (ui->treeWidget->GetSelected()) accept();
    //if (mArea != -1 && mType != -1) accept();
}

void TypeTreeSelect::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *, int) {
    if (ui->treeWidget->GetSelected()) accept();
    //ui->treeWidget->GetSelected(mArea, mType);
    //if (mArea != -1 && mType != -1) accept();
}
