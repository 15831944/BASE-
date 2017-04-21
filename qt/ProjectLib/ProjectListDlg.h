#ifndef PROJECTLIST_H
#define PROJECTLIST_H

#include <QTreeWidgetItem>
#include <QMdiSubWindow>
#include "../VProject/qfcdialog.h"

class ProjectData;

namespace Ui {
class ProjectListDlg;
}

#if defined(VPROJECT_MAIN_IMPORT)
    #define PROJECT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PROJECT_LIBRARY)
        #define PROJECT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PROJECT_LIBRARY_EXP_IMP
    #endif
#endif

class PROJECT_LIBRARY_EXP_IMP ProjectListDlg : public QFCDialog/*, public QMdiSubWindow*/
{
    Q_OBJECT
public:
    enum DisplayType { DTShowSelect, DTShowFull, DTShowView };
protected:
    ProjectData * mProjectData;
    DisplayType mDisplayType;

    virtual void showEvent(QShowEvent* event);
    void InitInConstructor();

    virtual void SaveAdditionalSettings(QSettings &aSettings);
    virtual void LoadAdditionalSettings(QSettings &aSettings);
public:
    explicit ProjectListDlg(DisplayType aDisplayType, QWidget *parent = 0);
    explicit ProjectListDlg(QSettings &aSettings, QWidget *parent = 0); // restore constructor
    virtual ~ProjectListDlg();

    virtual void SaveState(QSettings &aSettings);

    void ShowPlotList();
    void ShowProjectCard();

    void SetMode(int aMode);
    void SetSelectedProject(long aProject);

    ProjectData *GetProjectData() const { return mProjectData; }
private slots:
    void on_cbProjTreeMode_currentIndexChanged(int index);

    void on_buttonBox_accepted();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_treeWidget_doubleClicked(const QModelIndex &index);

    void on_tbReload_clicked();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    void on_pbDocs_clicked();

    void on_pbProjectCard_clicked();

    void on_pbIncoming_clicked();

    void on_pbContracts_clicked();

    void on_pbSubContracts_clicked();

private:
    bool mJustStarted;
    Ui::ProjectListDlg *ui;
};

#endif // PROJECTLIST_H
