#include "WaitDlg.h"
#include "ui_WaitDlg.h"

#include "MainWindow.h"

WaitDlg::WaitDlg(QWidget *parent, bool aDoNotDisable) :
    QFCDialog(parent, false),
    ui(new Ui::WaitDlg),
    mWidgetForDisable(parent),
    mCancelled(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint));
    ui->pbCancel->setVisible(false);

    if (aDoNotDisable) {
        mWidgetForDisable = NULL;
    } else {
        if (!mWidgetForDisable) {
            mWidgetForDisable = gMainWindow;
        } else {
            while (mWidgetForDisable->parentWidget()) mWidgetForDisable = mWidgetForDisable->parentWidget();
        }
        if (mWidgetForDisable) mWidgetForDisable->setDisabled(true);
    }

    QPalette lPalette = ui->label->palette();
    lPalette.setColor(QPalette::Disabled, QPalette::WindowText, lPalette.color(QPalette::Inactive, QPalette::WindowText));
    ui->label->setPalette(lPalette);
    ui->lblMessage->setPalette(lPalette);
}

WaitDlg::~WaitDlg() {
    if (mWidgetForDisable) mWidgetForDisable->setDisabled(false);
    delete ui;
}

void WaitDlg::SetCanCancelled(bool aCanCancelled) {
    mCancelled = false;
    ui->pbCancel->setVisible(aCanCancelled);
}

bool WaitDlg::CancelRequested() {
    return mCancelled;
}

void WaitDlg::SetMessage(const QString & aMessage) {
    ui->lblMessage->setText(aMessage);
    QCoreApplication::processEvents();
}

void WaitDlg::on_pbCancel_clicked() {
    mCancelled = true;
}
