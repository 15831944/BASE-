#ifndef CODEFORMINGDLG_H
#define CODEFORMINGDLG_H

#include "../VProject/qfcdialog.h"

class ProjectData;

namespace Ui {
class CodeFormingDlg;
}

class CodeFormingDlg : public QFCDialog
{
    Q_OBJECT

protected:
    ProjectData *mProject;
public:
    explicit CodeFormingDlg(ProjectData *aProject, QWidget *parent = 0);
    ~CodeFormingDlg();

private slots:
    void on_cbUseSheet_toggled(bool checked);
    void Accept();

    void on_cbProjType_currentIndexChanged(int index);

private:
    Ui::CodeFormingDlg *ui;
};

#endif // CODEFORMINGDLG_H
