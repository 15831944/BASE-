#ifndef PLOTLISTDLG_H
#define PLOTLISTDLG_H

#include "AcadXchgDialog.h"
#include "qfcdialog.h"

#include <QTreeWidget>
#include <QMdiSubWindow>

#include "GlobalSettings.h"
#include "../PlotLib/PlotData.h"

#include "../UsersDlg/UserData.h"
#include "../ProjectLib/ProjectData.h"

class PlotTreeItem;

namespace Ui {
class PlotListDlg;
}

#include "def_expimp.h"

class EXP_IMP PlotListDlg : public QFCDialog
{
    Q_OBJECT
public:
    enum DisplayType { DTShowSelectOne, DTShowSelectMany, DTShowFull, DTShowView };
public:
    explicit PlotListDlg(DisplayType aDisplayType, PlotData * aSelectedPlotData, PlotHistoryData * aSelectedHistory, QWidget *parent = 0);
    explicit PlotListDlg(QSettings &aSettings, QWidget *parent = 0); // restore constructor
    virtual ~PlotListDlg();

    virtual void SaveState(QSettings &aSettings);
    //virtual void LoadState(QSettings &aSettings);

    void SetProjectData(ProjectData *aProjectData);
    const ProjectData *ProjectDataConst() const;

    // debug222
    //void SetSelectedPlot(PlotData * aPlotData);
    void GetSelectedPlots(QList<PlotData *> &lPlotList);
    PlotData * SelectedPlot();

    // for call from PlotListTree, visua changes only
    void SetSecondLevelType(GlobalSettings::DocumentTreeStruct::SLT aSLT); // look at GlobalSettings::DocumentTree enum SLT

    //void ShowStatus(const QString & aStatus);

private:
    bool mJustStarted, mJustLoaded;
    int mDocTreeScrollPos, mDocTreeScrollPosHoriz; // for loading existing window
protected:
    DisplayType mDisplayType;
    bool mShowNonEmptyOnly;
    QString mComplectFromSettings, mListFromSettings;

    void InitInConstructor();
    void ShowWindowTitle();
    void ShowData();
    virtual void showEvent(QShowEvent* event);

    virtual void SaveAdditionalSettings(QSettings &aSettings);
    virtual void LoadAdditionalSettings(QSettings &aSettings);

private slots:
    void OnPlotListBeforeUpdate(int aIdProject);
    void OnPlotListNeedUpdate(int aIdProject);
    void OnPlotsNamedListNeedUpdate(PlotNamedListData *aNamedList);
    void DoSettingsChanged();
    void Accept();
    void on_twType_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_toolButton_clicked();
    void on_leIdProject_editingFinished();

    void on_cbSecondLevel_currentIndexChanged(int index);

    void on_actionNew_triggered();

    void on_actionRefresh_triggered();

    void on_twType_customContextMenuRequested(const QPoint &pos);

    void on_twDocs_itemSelectionChanged();

    void on_cbComplect_currentIndexChanged(int index);

    void on_cbList_customContextMenuRequested(const QPoint &pos);

    void on_cbList_currentIndexChanged(int index);

    void on_cbHideCancelled_toggled(bool checked);

    void on_actionShow_non_empty_only_triggered();

    void on_splitter_splitterMoved(int pos, int index);

    void on_cbLayout_currentIndexChanged(int index);

    void on_cbBlock_currentIndexChanged(int index);

private:
    Ui::PlotListDlg *ui;
};

void PlotSelect(int aIdProject, QList<PlotData *> &aPlotList);
#endif // PLOTLISTDLG_H
