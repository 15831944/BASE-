#include "LoggerWnd.h"
#include "ui_LoggerWnd.h"

#include "Logger.h"

#include <QDateTime>

LoggerWnd::LoggerWnd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoggerWnd)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint));
}

LoggerWnd::~LoggerWnd()
{
    delete ui;
    gLogger->WindowDeleted();
}

void LoggerWnd::AddLineToLog(const QString &aError) {
    ui->teLog->setPlainText(ui->teLog->toPlainText()
                            + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss ")
                            + aError + "\r\n");
    QTextCursor c = ui->teLog->textCursor();
    c.movePosition(QTextCursor::End);
    ui->teLog->setTextCursor(c);
}

void LoggerWnd::on_pbClearClose_clicked() {
    delete this;
}
