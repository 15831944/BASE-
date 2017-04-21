#ifndef SENTPARAMSDGL_H
#define SENTPARAMSDGL_H

#include "qfcdialog.h"

namespace Ui {
class SentParamsDgl;
}

class SentParamsDgl : public QFCDialog
{
    Q_OBJECT

public:
    explicit SentParamsDgl(QWidget *parent = 0);
    ~SentParamsDgl();

    const QDate & SentDate() const;
    QString SentUser() const;

private:
    Ui::SentParamsDgl *ui;
};

#endif // SENTPARAMSDGL_H
