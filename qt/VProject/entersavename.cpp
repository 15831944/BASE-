#include "entersavename.h"
#include "ui_entersavename.h"
#include "common.h"

EnterSaveName::EnterSaveName(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnterSaveName)
{
    ui->setupUi(this);
}

EnterSaveName::~EnterSaveName()
{
    delete ui;
}

void EnterSaveName::SetFilename(QString aFilename) {
    ui->lFilename->setText(aFilename);
};

void EnterSaveName::SetCode(QString aCode) {
    ui->lCode->setText(aCode);
};

void EnterSaveName::SetNameTop(QString aNameTop) {
    ui->lNameTop->setPlainText(aNameTop);
};

void EnterSaveName::SetName(QString aName) {
    ui->lName->setPlainText(aName);
};

QString EnterSaveName::Filename() const {
    return ui->lFilename->text();
};

QString EnterSaveName::Code() const {
    return ui->lCode->text();
};

QString EnterSaveName::NameTop() const {
    return ui->lNameTop->toPlainText();
};

QString EnterSaveName::Name() const {
    return ui->lName->toPlainText();
};

void EnterSaveName::on_pbOK_clicked() {
    if (!ui->lNameTop->toPlainText().length()) {
        QMessageBox::critical(this, tr(QT_TR_NOOP("Error")), tr(QT_TR_NOOP("Top name must be specified")));
        ui->lNameTop->activateWindow();
        return;
    };
    if (!ui->lName->toPlainText().length()) {
        QMessageBox::critical(this, tr(QT_TR_NOOP("Error")), tr(QT_TR_NOOP("Bottom name must be specified")));
        ui->lName->activateWindow();
        return;
    };
    accept();
};

void EnterSaveName::on_pbCancel_clicked() {
    reject();
};
