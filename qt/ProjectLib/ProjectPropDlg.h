#ifndef PROJECTPROPDLG_H
#define PROJECTPROPDLG_H

#include "../VProject/qfcdialog.h"

class ProjectData;

namespace Ui {
class ProjectPropDlg;
}

class ProjectPropDlg : public QFCDialog
{
    Q_OBJECT

protected:
    ProjectData *mProject, *mParentGroup;
    QPalette mPaletteNorm, mPaletteReq, mPaletteDis;
public:
    enum eNew { ProjectNew };
    enum eProps { ProjectProps };

    explicit ProjectPropDlg(eProps, ProjectData *aProject, QWidget *parent = 0);
    explicit ProjectPropDlg(eNew, ProjectData *aParentGroup, QWidget *parent = 0);
    ~ProjectPropDlg();

    int ProjectId() const;
protected:
    void InitInConstructor();
    virtual void showEvent(QShowEvent* event);
    void ShowData();

private slots:
    void Accept();

    void on_cbContractDate_clicked(bool checked);

    void on_tbSelManager_clicked();

    void on_tbSelCustomer_clicked();

private:
    Ui::ProjectPropDlg *ui;
};

#endif // PROJECTPROPDLG_H
