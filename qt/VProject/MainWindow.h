#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSettings>
#include <QApplication>

#include "TreeData.h"
#include "oracle.h"

#include "UpdateThread.h"

#include "def_expimp.h"

class ProjectData;
class PlotData;
class PlotHistoryData;

namespace Ui {
class MainWindow;
}

class EXP_IMP MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    bool mJustStarted;
    UpdateThread *mAlertThread;
    QTimer *mTimer;
    QMutex mUpdateMutex;

    QLabel *mLabelOnSB;

    bool mInLoadWindows;

    QList<tPairIntInt> mPlotForCmp;
protected:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void clean();

    virtual void showEvent(QShowEvent* event);
    virtual void closeEvent(QCloseEvent* event);
    void LoadMDIWindows(QSettings & aSettings);

    void AddWindowToMDI(QDialog *aDlg);
public:
    static MainWindow *GetInstance();

    void SaveMDIWindows(QSettings & aSettings);
    bool InLoadWindows();

    QMdiArea *MdiArea();

    UpdateThread *AlertThread();
    QMutex *UpdateMutex();

    void SetPlotForCmp(int aIdPlot, int aHistNum);
    bool GetPlotForCmp(int &aIdPlot, int &aHistNum);

    void ShowProjectRights(const ProjectData * aProject);
    void ShowProjectEnvUser(const ProjectData * aProject);

    void ShowPlotLists(const QList<tPairIntIntString> &aIds);
    void ShowPlotList(ProjectData * aProjectData, PlotData * aSelectedPlotData, PlotHistoryData * aSelectedHistory);
    void ShowPlotVersions(PlotData * aPlotData, bool aUpdateAuto);
    void ShowPlotHist(PlotData * aPlotData, int aHistoryNum, bool aUpdateAuto, bool aModal);
    void ShowPlotAddFiles(PlotData * aPlot, PlotHistoryData * aHistory, bool aModal);
    void ShowPlotAttrs();
    void ShowPlotProp(PlotData * aPlotData, PlotHistoryData * aPlotHistoryData);
    void ShowPlotRights(PlotData * aPlotData);
    void ShowPlotXrefs(PlotData * aPlotData, PlotHistoryData *aPlotHistory, bool aUpdateAuto, bool aModal);
    void ShowPlotXrefsAllOLD(PlotData * aPlotData);
    void ShowPlotXrefFor(PlotData * aPlotData);
    void ShowLoadImages(int aIdProject);

    void ShowPublishReport(int aId);

    void RecoverPlot(PlotData * aPlotData);

    void LoadXrefs(int aIdProject);

    PlotData * NewPlot(ProjectData * aProjectData, const TreeDataRecord * aTreeData, const QString & aComplect,
                       PlotData * aPlotDataFrom, PlotHistoryData * aPlotHistoryDataFrom);
signals:
    void ImageSettingsChanged();
private slots:
    void StyleSheetChangedSlot(); // dummy??? TODO
    void WindowMenuTriggered(QAction * action);
    void BeforeWindowMenu();

    void UpdateOnTimer();
    void MenuOnBLList(const QPoint &aPoint);

    void on_actionProjects_triggered();

    void on_actionAbout_triggered();

    void on_actionContracts_triggered();

    void on_actionCommonSettings_triggered();

    void on_actionQuick_find_by_id_triggered();

    void on_actionDrawings_audit_triggered();

    void on_actionDrawings_publish_triggered();

    void on_actionSelect_AutoCAD_triggered();

    void on_actionScheduler_triggered();

    void on_actionPlanner_day_triggered();

    void on_actionSend_message_triggered();

    void on_actionMessages_to_me_triggered();

    void on_actionMessages_from_me_triggered();

    void on_actionDocuments_list_triggered();

    void on_actionQuit_triggered();

    void on_actionQuit_and_reset_triggered();

    void on_actionFind_triggered();

    void on_actionDepartments_triggered();

    void on_actionOrganizations_triggered();

    void on_actionUsers_work_triggered();

    void on_actionComparing_in_AutoCAD_triggered();

    void on_actionEmployees_triggered();

    void on_actionDeleted_documents_triggered();

    void on_actionChange_password_triggered();

    void on_actionDocuments_tree_triggered();

    void on_actionSaved_documents_triggered();

    void on_actionRestrictions_on_savings_triggered();

    void on_actionGeobases_triggered();

    void on_actionIncoming_triggered();

    void on_actionOutgouing_triggered();

    void on_actionTemplates_for_outgoing_triggered();

    void on_actionWeekly_report_old_triggered();

    void on_actionLetters_list_triggered();

    void on_actionProject_types_triggered();

    void on_actionContracts_2_triggered();

    void on_actionImage_settings_triggered();

    void on_actionSubcontracts_triggered();

private:
    Ui::MainWindow *ui;
};

#define gMainWindow MainWindow::GetInstance()
#endif // MAINWINDOW_H
