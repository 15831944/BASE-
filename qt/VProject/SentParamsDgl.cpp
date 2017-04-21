#include "SentParamsDgl.h"
#include "ui_SentParamsDgl.h"

#include "../UsersDlg/UserData.h"

#include "common.h"

SentParamsDgl::SentParamsDgl(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::SentParamsDgl)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    for (int i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled())
            ui->cbSentUser->addItem(gUsers->UsersConst().at(i)->NameConst(), gUsers->UsersConst().at(i)->LoginConst());
    ui->cbSentUser->setCurrentText(gUsers->GetName(db.userName()));

    ui->dteSentDate->setDate(QDate::currentDate());
}

SentParamsDgl::~SentParamsDgl()
{
    delete ui;
}

const QDate &SentParamsDgl::SentDate() const {
    return ui->dteSentDate->date();
}

QString SentParamsDgl::SentUser() const {
    // toString() making a local copy
    return ui->cbSentUser->currentData().toString();
}
