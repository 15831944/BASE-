#ifndef PAYBYCUSTPARAMS_H
#define PAYBYCUSTPARAMS_H

#include "../VProject/qfcdialog.h"

namespace Ui {
class PayByCustParams;
}

class PayByCustParams : public QFCDialog
{
    Q_OBJECT

public:
    explicit PayByCustParams(QWidget *parent = 0);
    ~PayByCustParams();

    QStringList Selected() const;

private:
    Ui::PayByCustParams *ui;
};

#endif // PAYBYCUSTPARAMS_H
