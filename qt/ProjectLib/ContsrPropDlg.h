#ifndef CONTSRPROPDLG_H
#define CONTSRPROPDLG_H

#include "../VProject/qfcdialog.h"

class ProjectData;

namespace Ui {
class ContsrPropDlg;
}

class ConstrPropDlg : public QFCDialog
{
    Q_OBJECT

protected:
    ProjectData *mProject, *mParentProject;
    QPalette mPaletteNorm, mPaletteReq, mPaletteDis;
public:
    enum eNew { ConstrNew };
    enum eProps { ConstrProps };

    explicit ConstrPropDlg(eProps, ProjectData *aProject, QWidget *parent = 0);
    explicit ConstrPropDlg(eNew, ProjectData *aParentProject, QWidget *parent = 0);
    ~ConstrPropDlg();

    int ProjectId() const;
protected:
    void InitInConstructor();
    virtual void showEvent(QShowEvent* event);
    void ShowData();

private slots:
    void Accept();

private:
    bool mJustStarted;
    Ui::ContsrPropDlg *ui;
};

#endif // CONTSRPROPDLG_H
