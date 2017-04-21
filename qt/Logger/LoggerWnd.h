#ifndef LOGGERWND_H
#define LOGGERWND_H

#include <QDialog>

namespace Ui {
class LoggerWnd;
}

class LoggerWnd : public QDialog
{
    Q_OBJECT

public:
    explicit LoggerWnd(QWidget *parent = 0);
    ~LoggerWnd();

    void AddLineToLog(const QString &aError);

private slots:
    void on_pbClearClose_clicked();

private:
    Ui::LoggerWnd *ui;
};

#endif // LOGGERWND_H
