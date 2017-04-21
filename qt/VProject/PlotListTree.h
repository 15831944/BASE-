#ifndef PLOTLISTTREE_H
#define PLOTLISTTREE_H

#include <QTreeWidget>
#include <QDialogButtonBox>

#include "PlotTree.h"
#include "GlobalSettings.h"
#include "PlotListItemDelegate.h"

#include "AcadXchgDialog.h"

#include "../PlotLib/DwgLayoutData.h"

#include "def_expimp.h"

// what to show in tree
#define PLTWorking          0x0001
#define PLTProject          0x0002
#define PLTEditStatus       0x0004
#define PLTDeleted          0x0008
#define PLTNoEditMenu       0x0010
#define PLTNewVersion       0x0020
#define PLTFindMode         0x0040
#define PLTVersions         0x0080
#define PLTNoColors         0x0100
#define PLTForXrefs         0x0200
#define PLTSingleSelMode    0x0400


class PlotListTreeItem;
class PlotListTreeItemDelegate;

class VPImageViewer;

class ReplaceTextDlg;


class EXP_IMP PlotListTree : public QTreeWidget, public AcadXchgDialog
{
    Q_OBJECT
public:
    enum PLTCols { colXREFBlockName = 0, colXREFUseWorking, colID, colWorking,
                   colXREFHist,
                   colIdProject, colProject, colIdCommon, colIdHist, colHist, colHistOrigin,
                   colVersionInt, colVersionExt, colVersionIntNew, colVersionExtNew, colVersionDate,
                   colDeleteDate, colDeleteUser,
                   colCancelDate, colCancelUser,
                   colSentDate, colSentBy, colSection, colLayoutName, colCode, colSheet, colStage, colPurpose,
                   colBlockName, colNameTop, colNameBottom,
                   colCreated, colCreatedBy, colEdited, colEditedBy,
                   colStatus, colExt, colSize, colXrefs, colComments, colLAST
                 };
typedef int ColsArrayType[colLAST];

    explicit PlotListTree(QWidget *parent);
    virtual ~PlotListTree();

    void InitColumns(qint32 aPLTFlags);
    void HideColumn(PLTCols aColName, bool aHide = true);

    int GetRowHeight(const QModelIndex & index) const;

    void SetSecondLevelType(GlobalSettings::DocumentTreeStruct::SLT aSLT); // look at GlobalSettings::DocumentTree enum SLT
    GlobalSettings::DocumentTreeStruct::SLT SLT() const;

    void SetSelectedPlotId(int aSelectedPlotId);

    void LoadFromSettings(QSettings &aSettings);

    PlotListTreeItem *itemFromIndex(const QModelIndex & index) const;
    // ----------------------------------------------------------
    void SetSelectedPlotIds(const QList<int> & aSelectedPlotIds);
    void SetSelectedPlotIdsCommon(const QList<int> & aSelectedPlotIdsCommon);
    void SetSelectedLayoutIds(const QList<int> & aSelectedLayoutIds);
    void SetSelectedHistoryIds(const QList<int> & aSelectedHistoryIds);
    void SetSelectedAddFilesIds(const QList<int> & aSelectedAddFilesIds);
    void SetExpandedPlotIdsCommon(const QList<int> & aExpandedPlotIdsCommon);
    void SetCurrentItem(int aType, int aId, int aColumn);

    void SetProjectData(ProjectData *aProject);
    void SetHideCancelled(bool aHideCancelled);
    void SetComplect(const QString &aComplect);
    void SetList(PlotNamedListData * aNamedList);


    QFCDialog *ParentDlg() const;

    const ColsArrayType & Cols() const;
    quint32 PLTFlags() const;
    void SetDialogButtonBox(QDialogButtonBox *aDialogButtonBox);

    void SetEditType(int aType);
    void SetEditItems(const QList <QTreeWidgetItem *> & aEditItems);
    void SetEditColumn(int aColumn);
    void SetEditOldValue(const QString & aOldValue);
    // ----------------------------------------------------------


    void Populate(const PlotTreeItem * aPlotTreeItem); // populating list from tree item

    void ViewEdit(bool aTrueForEdit, bool aNoXrefs);
    void DoAuditPurge();
    void MakeXLS();
    void DoPublish(bool aJustListMaatz);
    void DoReplaceText(const ReplaceTextDlg *aReplaceTextDlg);

    void NewPlot(ProjectData * aProjectData, const TreeDataRecord * aTreeData, const QString & aComplect,
                 PlotData * aPlotDataFrom, PlotHistoryData * aPlotHistoryDataFrom); // create new document; add new item if need

    void SaveDocuments();
    void SaveAddFiles();
    void LoadOneDocument(PlotData * aPlot);

protected:
    int mVersionSaved, mVersionCurrent;

    bool mVerExtVisible, mVerDateVisible, mSentDateVisible, mSentUserVisible, mComplectVisible;
    bool mBlockNameVisible;

    bool mIgnoreSectionResize;
    int mInSaveTimer;
    QFCDialog *mParentDlg; // owner is PlotListDlg - for switching second level type
    QDialogButtonBox *mDialogButtonBox;
    ColsArrayType mCols;
    quint32 mPLTFlags;
    GlobalSettings::DocumentTreeStruct::SLT mSLT;

    //QList<PlotData *> mSelectedPlots;
    QList<int> mSelectedPlotIds, mSelectedPlotIdsCommon, mSelectedLayoutIds, mSelectedHistoryIds, mSelectedAddFilesIds;
    QList<int> mExpandedPlotIdsCommon;
    int mCurrentItemType, mCurrentItemId, mCurrentColumn;
    //int mDocTreeScrollPos, mDocTreeScrollPosHoriz; // for loading existing window

    int mEditType;
    QList <QTreeWidgetItem *>  mEditItems;
    int mEditColumn;
    QString mEditOldValue;
    QString mStrFromEdit;

    ProjectData * mProject;
    bool mHideCancelled;
    QString mComplect;
    PlotNamedListData * mNamedList;

    VPImageViewer *mVPImageViewer;

    void PopulateLayoutsInternal(PlotListTreeItem * aParentItem, PlotData *aPlot);
    void PopulateInternal(const PlotTreeItem * aPlotTreeItem);

    void SaveCurrentStateInternal() const;

    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);
signals:
    void AskMakeNewDocument();
    void WasUndeleted();
public slots:

private slots:
    void OnDeleteVPImageViewer(int);
    void OnCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void ShowContextMenu(const QPoint & pos);
    void OnItemDoubleClicked(QTreeWidgetItem * item, int column);

    void StringChangedTimer();
    void StringChanged(QWidget *editor);

//    void OnPlotListBeforeUpdate(int aIdProject);
//    void OnPlotListNeedUpdate(int aIdProject);

    void OnPlotBeforeUpdate(PlotData *aPlot, int aType); // the main need of it is collect data - selected, expaneded, scrolled pos., etc
    void OnPlotNeedUpdate(PlotData *aPlot, int aType);

    //void OnHeaderChanged();
    void OnSectionResizedTimer();
    void OnSectionResized(int logicalIndex, int oldSize, int newSize);

    void OnSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void OnSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);

    void OnSelectColumns(const QPoint &aPoint);
};

class EXP_IMP PlotListTreeItem : public QTreeWidgetItem
{
private:
    PlotListTree * mPlotListTree; // it is duplicate of function treeWidget(), but is primary used in construcros

    bool mIsOwner;
    PlotData * mPlotData;
    int mMaxDwgVersion;
    DwgLayoutData * mDwgLayout;
    DwgLayoutBlockData * mDwgLayoutBlock;

    PlotHistoryData * mPlotHistoryData;
    PlotAddFileData * mPlotAddFileData;

    bool LessByCodeSheet(const PlotListTreeItem & other, int aSortCol) const;
public:
//    explicit PlotTreeWidgetItem(QTreeWidget * parent = 0);
    explicit PlotListTreeItem(PlotListTree * aPlotListTree, PlotData * aPlotData, PlotHistoryData * aPlotHistoryData, int aMaxDwgVersion, bool aIsSecondLevel = false);
    explicit PlotListTreeItem(PlotListTree * aPlotListTree, DwgLayoutData * aDwgLayout, DwgLayoutBlockData * aDwgLayoutBlock);
    explicit PlotListTreeItem(PlotListTree * aPlotListTree, PlotHistoryData * aPlotHistoryData);
    explicit PlotListTreeItem(PlotListTree * aPlotListTree, PlotAddFileData * aPlotAddFileData);
    virtual ~PlotListTreeItem();

    void SetIsOwner(bool aIsOwner);

    virtual bool operator<(const QTreeWidgetItem & other) const;

    PlotData * PlotRef() const;
    const PlotData * PlotConst() const;

    DwgLayoutData * DwgLayoutRef() const;
    const DwgLayoutData * DwgLayoutConst() const;

    PlotHistoryData * PlotHistoryRef() const;
    const PlotHistoryData * PlotHistoryConst() const;

    PlotAddFileData * PlotAddFileRef() const;
    const PlotAddFileData * PlotAddFileConst() const;

    void ShowEditStatus();
    void ShowData();
signals:

public slots:

};

class PlotListTreeItemDelegate : public PlotListItemDelegate
{
    Q_OBJECT
public:
    explicit PlotListTreeItemDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // PLOTLISTTREE_H
