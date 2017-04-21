#include "ReplaceTextDlg.h"
#include "ui_ReplaceTextDlg.h"

#include <QMessageBox>

ReplaceTextDlg::ReplaceTextDlg(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ReplaceTextDlg)
{
    ui->setupUi(this);

//    ui->leFindText->setText("UNCLASSIFIED FOUO");
//    ui->leReplaceWith->setText("UNCLASSIFIED//with IMOD Handling Instructions");

    emit ui->cbMoveType->currentIndexChanged(ui->cbMoveType->currentIndex());
}

ReplaceTextDlg::~ReplaceTextDlg()
{
    delete ui;
}

QString ReplaceTextDlg::FindTextCopy() const {
    return ui->leFindText->text();
}

QString ReplaceTextDlg::ReplaceWithCopy() const {
    return ui->leReplaceWith->text();
}

long ReplaceTextDlg::MoveType() const {
    return ui->cbMoveType->currentIndex();
}

double ReplaceTextDlg::DX() const {
    return ui->leDX->text().toDouble();
}

double ReplaceTextDlg::DY() const {
    return ui->leDY->text().toDouble();
}

void ReplaceTextDlg::Accept() {
    if (ui->leFindText->text().isEmpty()) {
        QMessageBox::critical(this, windowTitle(), tr("Find text must be specified!"));
        ui->leFindText->setFocus();
        return;
    }

    accept();
}

void ReplaceTextDlg::on_cbMoveType_currentIndexChanged(int index) {
    ui->lblDX->setVisible(index);
    ui->leDX->setVisible(index);
    ui->lblDY->setVisible(index);
    ui->leDY->setVisible(index);
}
