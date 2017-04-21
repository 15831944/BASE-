#ifndef DOCLISTSETTINGSDLG_H
#define DOCLISTSETTINGSDLG_H

#include "qfcdialog.h"

class ProjectData;
class TreeDataRecord;

namespace Ui {
class DocListSettingsDlg;
}

class DocListSettingsDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit DocListSettingsDlg(ProjectData * aProject, QWidget *parent = 0);
    ~DocListSettingsDlg();

    int IdTemplate();

    bool NeedSave();
    int IdProject();
    int TypeArea();
    int TypeId();
    QString Code();
    QString NameTop();
    QString NameBottom();

protected:
    TreeDataRecord *mTreeData;
    ProjectData *mProject;
private slots:
    void on_cbSave_toggled(bool checked);

    void on_leIdProject_editingFinished();

private:
    Ui::DocListSettingsDlg *ui;
};

#endif // DOCLISTSETTINGSDLG_H
