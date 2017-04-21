#ifndef PROJECTTYPEDLG_H
#define PROJECTTYPEDLG_H

#include "../VProject/qfcdialog.h"


namespace Ui {
class ProjectTypeDlg;
}

class ProjectTypeDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit ProjectTypeDlg(QWidget *parent = 0);
    ~ProjectTypeDlg();

protected:
    //void InitInConstructor();
    virtual void showEvent(QShowEvent* event);

    void ShowData();

private:
    bool mJustStarted;
    Ui::ProjectTypeDlg *ui;
};

#endif // PROJECTTYPEDLG_H
