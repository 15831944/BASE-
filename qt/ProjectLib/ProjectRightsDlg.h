#ifndef PROJECTRIGHTSDLG_H
#define PROJECTRIGHTSDLG_H

#define NO_EXPIMP
#include "../VProject/qfcdialog.h"
#include <qtablewidget.h>

class ProjectData;

namespace Ui {
class ProjectRightsDlg;
}

class ProjectRightsDlg : public QFCDialog
{
    Q_OBJECT

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void resizeEvent(QResizeEvent * event);

public:
    enum UserListType { PRDRights, PRDEnv };
    explicit ProjectRightsDlg(UserListType aULT, const ProjectData *PrjData, QWidget *parent = 0);
    ~ProjectRightsDlg();

private:
    UserListType mULT;
    int mIdProj;
    bool mJustStarted;
    Ui::ProjectRightsDlg *ui;
    //QPalette lPaletteReq;
    QPalette lPaletteDis;
    QTableWidgetItem *lItem;
    bool get_Restrict;

private slots:
    void ShowData();
    void on_buttonBox_accepted();
    void on_tableWidget_itemSelectionChanged();
    void on_tbPlus_clicked();
    void on_actionDelete_triggered();
    void on_actionProperties_triggered();
    void on_tbMinus_clicked();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_tableWidget_cellDoubleClicked(int row, int column);
};

#endif // PROJECTRIGHTSDLG_H
