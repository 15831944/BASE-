#ifndef SELECTCOLUMNSDLG_H
#define SELECTCOLUMNSDLG_H

#include "qfcdialog.h"
#include <QHeaderView>

#include "def_expimp.h"

namespace Ui {
class SelectColumnsDlg;
}

class EXP_IMP SelectColumnsDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit SelectColumnsDlg(QWidget *parent = 0);
    explicit SelectColumnsDlg(bool aLoadSettings, QWidget *parent = 0);
    ~SelectColumnsDlg();

    void SetHeaderView(QHeaderView *aHeaderView, int aCount = -1) { mHeaderView = aHeaderView; mCount = aCount; }
    void SetDisabledIndexes(const QList<int> & aDisabledIndexes) { mDisabledIndexes = aDisabledIndexes; }

protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    void on_buttonBox_accepted();

private:
    Ui::SelectColumnsDlg *ui;

    int mCount;
    QHeaderView *mHeaderView;
    QList<int> mDisabledIndexes;
};

#endif // SELECTCOLUMNSDLG_H
