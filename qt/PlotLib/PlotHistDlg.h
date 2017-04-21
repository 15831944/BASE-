#ifndef PLOTHISTDLG_H
#define PLOTHISTDLG_H

#include <QTreeWidgetItem>

#include "../VProject/qfcdialog.h"

#include "PlotData.h"

#if defined(VPROJECT_MAIN_IMPORT)
    #define PLOT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PLOT_LIBRARY)
        #define PLOT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PLOT_LIBRARY_EXP_IMP
    #endif
#endif

class VPImageViewer;
class PlotHistTreeItem;

namespace Ui {
class PlotHistDlg;
}

class PlotHistDlg : public QFCDialog
{
    Q_OBJECT
protected:
    VPImageViewer *mVPImageViewer;
    int mCurNum, mCurColumn;
    QList<int> mSelectedIds;
    int mScrollPosVert, mScrollPosHoriz;
    int mIdProject, mIdPlot, mDeleted;
    PlotData * mPlot;
public:
    explicit PlotHistDlg(PlotData * aPlot, int aHistotyNum, bool aAutoUpdate, QWidget *parent = 0);
    explicit PlotHistDlg(QSettings &aSettings, QWidget *parent = 0); // restore constructor
    virtual ~PlotHistDlg();

    virtual void SaveState(QSettings &aSettings);
protected:
    void InitInConstructor();
    virtual void showEvent(QShowEvent* event);

    void ViewSelected(bool aWithoutXrefs);
    virtual void StyleSheetChangedInSescendant();
    void SaveCurrentStateForRefresh();
private slots:
    void ShowData();
    void OnDeleteVPImageViewer(int);

    void OnPlotListBeforeUpdate(int aIdProject);
    void OnPlotListNeedUpdate(int aIdProject);

    void OnPlotBeforeUpdate(PlotData *aPlot, int aType);
    void OnPlotNeedUpdate(PlotData *aPlot, int aType);

    void OnPlotBecameSelected(PlotData * aPlot);
    void DoSelectColumns(const QPoint &aPoint);

    void on_twHist_customContextMenuRequested(const QPoint &pos);

    void on_cbAutoUpdate_toggled(bool checked);

    void on_twHist_itemSelectionChanged();

    void on_twHist_doubleClicked(const QModelIndex &index);

    void on_tbReload_clicked();

    void on_cbFullMode_toggled(bool checked);

    void on_twHist_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    bool mJustStarted;
    static bool mBlockUpdateSignals;

    Ui::PlotHistDlg *ui;
};

#endif // PLOTHISTDLG_H
