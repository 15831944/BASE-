#ifndef PLOTADDFILEDLG_H
#define PLOTADDFILEDLG_H

#include "../VProject/qfcdialog.h"

class VPImageViewer;
class PlotData;
class PlotHistoryData;
class PlotAddFileData;
class QTreeWidgetItem;

namespace Ui {
class PlotAddFileDlg;
}

class PlotAddFileDlg : public QFCDialog
{
    Q_OBJECT

protected:
    VPImageViewer *mVPImageViewer;
    PlotData * mPlot;
    PlotHistoryData * mHistory;
    int mIdPlot, mIdProject, mNumHist;

    int mPrevWidth;

    int mCurrentItemId, mCurrentColumn;
    QList<int> mSelectedIds;
    int mScrollPosVert, mScrollPosHoriz;
public:
    explicit PlotAddFileDlg(PlotData * aPlot, PlotHistoryData * aHistory, QWidget *parent = 0);
    explicit PlotAddFileDlg(QSettings &aSettings, QWidget *parent = 0);
    virtual ~PlotAddFileDlg();

    virtual void SaveState(QSettings &aSettings);
protected:
    void InitInConstructor();
    bool CreateDwgCopy();
    void View();
    void Save();
    void Load();
    void Delete();

    virtual void showEvent(QShowEvent* event);
    virtual void resizeEvent(QResizeEvent * event);

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
    void OnPlotHistoryBecameSelected(PlotData * aPlot, PlotHistoryData * aHistory);


    void DoSelectColumns(const QPoint &aPoint);

    void on_cbAutoUpdate_toggled(bool checked);

    void on_twAddFile_customContextMenuRequested(const QPoint &pos);

    void on_twAddFile_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_twAddFile_doubleClicked(const QModelIndex &index);

private:
    bool mJustStarted;
    static bool mBlockUpdateSignals;

    Ui::PlotAddFileDlg *ui;
};

#endif // PLOTADDFILEDLG_H
