#include "PlotListTree.h"

int PlotListTree::GetRowHeight(const QModelIndex & index) const {
    return rowHeight(index);
}

void PlotListTree::SetSelectedPlotIds(const QList<int> & aSelectedPlotIds) {
    mSelectedPlotIds = aSelectedPlotIds;
}

void PlotListTree::SetSelectedPlotIdsCommon(const QList<int> & aSelectedPlotIdsCommon) {
    mSelectedPlotIdsCommon = aSelectedPlotIdsCommon;
}

void PlotListTree::SetSelectedLayoutIds(const QList<int> & aSelectedLayoutIds) {
    mSelectedLayoutIds = aSelectedLayoutIds;
}

void PlotListTree::SetSelectedHistoryIds(const QList<int> & aSelectedHistoryIds) {
    mSelectedHistoryIds = aSelectedHistoryIds;
}

void PlotListTree::SetSelectedAddFilesIds(const QList<int> & aSelectedAddFilesIds) {
    mSelectedAddFilesIds = aSelectedAddFilesIds;
}

void PlotListTree::SetExpandedPlotIdsCommon(const QList<int> & aExpandedPlotIdsCommon) {
    mExpandedPlotIdsCommon = aExpandedPlotIdsCommon;
}

void PlotListTree::SetCurrentItem(int aType, int aId, int aColumn) {
    mCurrentItemType = aType; mCurrentItemId = aId; mCurrentColumn = aColumn;
}


void PlotListTree::SetProjectData(ProjectData *aProject) {
    mProject = aProject;
}

void PlotListTree::SetHideCancelled(bool aHideCancelled) {
    mHideCancelled = aHideCancelled;
}

void PlotListTree::SetComplect(const QString &aComplect) {
    mComplect = aComplect;
}

void PlotListTree::SetList(PlotNamedListData * aNamedList) {
    mNamedList = aNamedList;
}

QFCDialog *PlotListTree::ParentDlg() const {
    return mParentDlg;
}
const PlotListTree::ColsArrayType & PlotListTree::Cols() const {
    return mCols;
}

quint32 PlotListTree::PLTFlags() const {
    return mPLTFlags;
}

void PlotListTree::SetDialogButtonBox(QDialogButtonBox *aDialogButtonBox) {
    mDialogButtonBox = aDialogButtonBox;
}

void PlotListTree::SetEditType(int aType) {
    mEditType = aType;
}

void PlotListTree::SetEditItems(const QList <QTreeWidgetItem *> & aEditItems) {
    mEditItems = aEditItems;
}

void PlotListTree::SetEditColumn(int aColumn) {
    mEditColumn = aColumn;
}

void PlotListTree::SetEditOldValue(const QString & aOldValue) {
    mEditOldValue = aOldValue.trimmed();
}
