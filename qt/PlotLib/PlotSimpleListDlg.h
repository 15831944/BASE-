#ifndef PLOTSIMPLELISTDLG_H
#define PLOTSIMPLELISTDLG_H

#include <QTreeWidgetItem>

#include "../VProject/AcadXchgDialog.h"
#include "../VProject/qfcdialog.h"
#include "../VProject/PlotListItemDelegate.h"

class FileType;

class ProjectData;

class PlotData;
class PlotHistoryData;
typedef QPair<PlotData *, PlotHistoryData *> PlotAndHistoryData;

typedef QPair<int, int> tPairIntInt;
typedef QPair<tPairIntInt, QString> tPairIntIntString;

namespace Ui {
class PlotSimpleListDlg;
}

class PlotSimpleListDlg : public QFCDialog, public AcadXchgDialog
{
    Q_OBJECT

public:
    enum NewDisplayType { NDTNewVersion = 0, NDTVersions, NDTXrefs, NDTXrefFor };
protected:
    class PlotAddData {
    protected:
        QString mBlockName;
        bool mUseWorking;
        int mHistory;
    public:
        explicit PlotAddData(const QString &aBlockName, bool aUseWorking, int aHistory) :
            mBlockName(aBlockName),
            mUseWorking(aUseWorking),
            mHistory(aHistory) {}
        const QString &BlockName() const { return mBlockName; }
        bool UseWorking() const { return mUseWorking; }
        int History() const { return mHistory; }
    };

    PlotData *mPlot;
    PlotHistoryData *mPlotHistory;
    QList <PlotAddData *> mPlotAddData;

    NewDisplayType mDisplayType;
    QList <PlotAndHistoryData> mPlots;
    QList <int> mProjectIds;
    QList <int> mPlotIds, mHistoryIds;

    QList <int> mSelectedPlotIds;
    int mCurrentItemId, mCurrentColumn;

    ProjectData * mProjectForTimer;

    // file data
    qint64 mOrigFileSize; // sum size of directory calced after select; for single file - size of file, which also refrereshed on pressed OK

    QList<tPairIntIntString> mExistingIds;
    QMenu *mExistsListMenu;
    const FileType * mFileType;
public:
    explicit PlotSimpleListDlg(NewDisplayType aDisplayType, const QList<PlotAndHistoryData> &aPlots, QWidget *parent = 0);
    explicit PlotSimpleListDlg(NewDisplayType aDisplayType, PlotData *aPlot, PlotHistoryData *aHistory, bool aAutoUpdate, QWidget *parent = 0);
    //explicit PlotSimpleListDlg(DisplayType aDisplayType, QWidget *parent = 0);
    ~PlotSimpleListDlg();

    virtual QString AddToClassName() const;

    const QList <PlotAndHistoryData> & PlotsConst() const;

    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);
protected:
    void InitInConstructor();
    virtual void showEvent(QShowEvent* event);

    void CollectXrefs();
    void CollectXrefFor();

private slots:
    void ShowData();
    void Accept();
    void ListItemChanged(QTreeWidgetItem * item, int column);
    void DoSettingsChanged();

    void TimerForUpdate();

    void OnPlotListBeforeUpdate(int aIdProject);
    void OnPlotListNeedUpdate(int aIdProject);

    void PlotSelectionChanged(PlotData * aPlot);
    void PlotHistorySelectionChanged(PlotData * aPlot, PlotHistoryData * aHistory);

    void on_twDocs_itemSelectionChanged();

    void on_cbAutoUpdate_toggled(bool checked);

    void DoSelectColumns(const QPoint &aPoint);

    void on_tbSelFile_clicked();

    void on_cbWhatToDo_currentIndexChanged(int index);

    void on_pbAlreadyInBase_clicked();

private:
    bool mInListItemCahnged;
    bool mJustStarted;
    Ui::PlotSimpleListDlg *ui;

};

#endif // PLOTSIMPLELISTDLG_H
