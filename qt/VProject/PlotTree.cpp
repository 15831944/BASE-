#include "common.h"
#include "GlobalSettings.h"
#include "PlotTree.h"
#include "TreeData.h"
#include "PlotListTree.h"

#include <QMimeData>
#include <QDragMoveEvent>
#include <QDropEvent>

#include "../ProjectLib/ProjectData.h"

PlotTree::PlotTree(QWidget *parent) :
    QTreeWidget(parent),
    mTreeDataSelected(NULL), mIdProject(0), mProjectData(NULL), mHideEmpty(true), mHideCancelled(false), mNamedList(NULL)
{
//    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(DoubleClicked(QTreeWidgetItem *, int)));
}

TreeDataRecord * PlotTree::GetSelected() const {
    if (currentItem()) {
        return static_cast<PlotTreeItem *>(currentItem())->TreeDataRef();
    } else {
        return NULL;
    }
}

void PlotTree::SetSelected(const TreeDataRecord * aTreeDataSelected) {
    mTreeDataSelected = aTreeDataSelected;
}

int PlotTree::IdProject() const {
    return mIdProject;
}

void PlotTree::SetIdProject(int aIdProject) {
    mIdProject = aIdProject;
    mProjectData = gProjects->FindByIdProject(aIdProject);
}

void PlotTree::SetProjectData(ProjectData *aProjectData) {
    mProjectData = aProjectData;
    if (mProjectData)
        mIdProject = mProjectData->Id();
    else
        mIdProject = 0;
}

void PlotTree::SetHideEmpty(bool aHideEmpty) {
    mHideEmpty = aHideEmpty;
    ShowVisibility();
}

void PlotTree::SetHideCancelled(bool aHideCancelled) {
    mHideCancelled = aHideCancelled;
    ShowVisibility();
}

void PlotTree::SetComplect(const QString &aComplect) {
    mComplect = aComplect;
    ShowVisibility();
}

void PlotTree::SetList(const PlotNamedListData * aNamedList) {
    mNamedList = aNamedList;
    ShowVisibility();
}

ProjectData * PlotTree::ProjectRef() {
    return mProjectData;
}

const ProjectData * PlotTree::ProjectConst() const {
    return mProjectData;
}

bool PlotTree::ShowVisibilityItem(PlotTreeItem *aItem) {
    int i;
    int res = false;
    for (i = 0; i < aItem->childCount(); i++) {
        if (ShowVisibilityItem((PlotTreeItem *) aItem->child(i))) res = true;
    }
    if (mProjectData) {
        bool lIsEmpty = true;
        if (mComplect.isEmpty() && !mNamedList) {
            lIsEmpty = aItem->PlotsConst().isEmpty();
        } else {
            foreach (const PlotData * lPlot, aItem->PlotsConst()) {
                if (!mComplect.isEmpty()) {
                    if (lPlot->SectionConst() == mComplect) {
                        lIsEmpty = false;
                        break;
                    }
                }
                if (mNamedList) {
                    if (mNamedList->IdsCommonConst().contains(lPlot->IdCommon())) {
                        lIsEmpty = false;
                        break;
                    }
                }
            }
        }
        if (!res) {
            res = (!mHideEmpty || mHideEmpty && !lIsEmpty);
        }
    } else
        res = true;
    aItem->setHidden(!res);
    return res;
}

void PlotTree::ShowVisibility() {
    int i;
    for (i = 0; i < topLevelItemCount(); i++) {
        ShowVisibilityItem((PlotTreeItem *) topLevelItem(i));
    }
}

void PlotTree::PopulateTree() {
    //GetSelected(mArea, mId);
    //model()->removeRows(0, this->model()->rowCount());

    if (!mTreeDataSelected && currentItem()) mTreeDataSelected = static_cast<PlotTreeItem *>(currentItem())->TreeDataRef();
    bool lNeedUnblock = false;
    if (!signalsBlocked()) {
        blockSignals(true);
        lNeedUnblock = true;
    }
    clear();

    // anyway there is nothing to update in fact
    // if data is loaded - it is not updated, just populated in PopulateTreeInternal()
    // if not loaded - so it is unneeded in other windows
    bool lSignalsBlocked = gProjects->blockSignals(true);
    PopulateTreeInternal();
    gProjects->blockSignals(lSignalsBlocked);

    QTreeWidgetItem * lParentItem = currentItem();
    if (lParentItem)
        while (lParentItem = lParentItem->parent()) lParentItem->setExpanded(true);

    if (mProjectData) {
        ShowVisibility();
    }
    if (lNeedUnblock)
        blockSignals(false);
    if (mProjectData) {
        emit currentItemChanged(currentItem(), currentItem());
    }
}

void PlotTree::PopulateTreeInternal(PlotTreeItem *aParent, int aLevel) {
    for (int i = 0; i < (aParent?aParent->TreeDataConst()->LeafsConst().length():gTreeData->LeafsConst().length()); i++) {
        // skip hidden
        if ((aParent?aParent->TreeDataConst()->LeafsConst().at(i):gTreeData->LeafsConst().at(i))->Hidden()) continue;

        PlotTreeItem *item = new PlotTreeItem(aParent?aParent->TreeDataConst()->LeafsConst().at(i):gTreeData->LeafsConst().at(i));

        if (!aParent) {
            addTopLevelItem(item);
        } else {
            aParent->addChild(item);
        }

        if (item->TreeDataConst() == mTreeDataSelected) {
            item->setSelected(true);
            setCurrentItem(item);
            mTreeDataSelected = NULL;
        }

        PopulateTreeInternal(item, aLevel + 1);

        item->setExpanded(aLevel < gSettings->TypeTree.ExpandLevel);

        if (mProjectData) {
            item->Plots() = mProjectData->GetPlotsByTreeData(item->TreeDataConst()->Area(), item->TreeDataConst()->Id());
        }
    }
}

PlotTreeItem * PlotTree::GetDocTreeItemInternal(PlotTreeItem * aItem, int aTreeDataArea, int aTreeDataId) {
    PlotTreeItem * res = NULL;

    if (aItem->TreeDataConst()->Area() == aTreeDataArea
            && aItem->TreeDataConst()->Id() == aTreeDataId)
        return aItem;

    for (int i = 0; i < aItem->childCount(); i++) {
        if (static_cast<PlotTreeItem *>(aItem->child(i))->TreeDataConst()->Area() == aTreeDataArea
                && static_cast<PlotTreeItem *>(aItem->child(i))->TreeDataConst()->Id() == aTreeDataId)
            return static_cast<PlotTreeItem *>(aItem->child(i));
        res = GetDocTreeItemInternal(static_cast<PlotTreeItem *>(aItem->child(i)), aTreeDataArea, aTreeDataId);
        if (res) return res;
    }
    return NULL;
}

PlotTreeItem * PlotTree::GetDocTreeItem(int aTreeDataArea, int aTreeDataId) {
    PlotTreeItem * res = NULL;
    for (int i = 0; i < topLevelItemCount(); i++) {
        res = GetDocTreeItemInternal(static_cast<PlotTreeItem *>(topLevelItem(i)), aTreeDataArea, aTreeDataId);
        if (res) return res;
    }
    return NULL;

}

void PlotTree::dragMoveEvent(QDragMoveEvent * event) {
    PlotTreeItem * lItem = (PlotTreeItem *) this->itemAt(event->pos());

    if (mProjectData
            && !mProjectData->Archived()
            && event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {

        QDataStream stream(&event->mimeData()->data("application/x-qabstractitemmodeldatalist"), QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;

            if (!col
                    && roleDataMap[Qt::UserRole + 0].toString() == gSettings->BaseNameOnly + "/" + gSettings->CurrentSchema
                    && roleDataMap[Qt::UserRole + 1].toString() == "PLOT") {

                if (roleDataMap[Qt::UserRole + 2].toInt() == mProjectData->Id()
                        && lItem
                        && lItem->TreeDataConst()->CanExists()
                        && lItem->TreeDataConst()->ActualFileType() != -1) {
                    // same project, change document type
                    PlotData *lPlotData = mProjectData->GetPlotById(roleDataMap[Qt::UserRole + 3].toInt());

                    if (lPlotData
                            && (lPlotData->TDArea() != lItem->TreeDataConst()->Area()
                                || lPlotData->TDId() != lItem->TreeDataConst()->Id())) {
                        QTreeWidget::dragMoveEvent(event);
                        return;
                    }
                } else if (roleDataMap[Qt::UserRole + 2].toInt() != mProjectData->Id()) {
                    // move to other project
                    ProjectData * lProjectData = gProjects->FindByIdProject(roleDataMap[Qt::UserRole + 2].toInt());
                    if (lProjectData
                            && !lProjectData->Archived()) {
                        QTreeWidget::dragMoveEvent(event);
                        return;
                    }
                }
            }
        }

    }

    event->ignore();
}

void PlotTree::dropEvent(QDropEvent * event) {
    PlotTreeItem * lItem = (PlotTreeItem *) this->itemAt(event->pos());

    if (mProjectData
            && !mProjectData->Archived()
            && event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {

        QDataStream stream(&event->mimeData()->data("application/x-qabstractitemmodeldatalist"), QIODevice::ReadOnly);

        int lDropType = 0;
        QList<PlotData *> lMoveList;
        QStringList lMoveStr;
        ProjectData * lOldProject = NULL;

        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;

            if (!col
                    && roleDataMap[Qt::UserRole + 0].toString() == gSettings->BaseNameOnly + "/" + gSettings->CurrentSchema
                    && roleDataMap[Qt::UserRole + 1].toString() == "PLOT") {
                if (roleDataMap[Qt::UserRole + 2].toInt() == mProjectData->Id()
                        && lItem
                        && lItem
                        && lItem->TreeDataConst()->CanExists()
                        && lItem->TreeDataConst()->ActualFileType() != -1) {
                    // same project, change document type
                    lDropType = 1;

                    PlotData *lPlot = mProjectData->GetPlotById(roleDataMap[Qt::UserRole + 3].toInt());

                    if (lPlot
                            && (lPlot->TDArea() != lItem->TreeDataConst()->Area()
                                || lPlot->TDId() != lItem->TreeDataConst()->Id())) {
                        lMoveList.append(lPlot);
                        lMoveStr.append(QString::number(lPlot->Id()) + " - " + lPlot->CodeSheetConst());
                    }
                } else if (roleDataMap[Qt::UserRole + 2].toInt() != mProjectData->Id()) {
                    // move to other project
                    lDropType = 2;

                    if (!lOldProject) lOldProject = gProjects->FindByIdProject(roleDataMap[Qt::UserRole + 2].toInt());
                    if (lOldProject
                            && !lOldProject->Archived()) {
                        PlotData *lPlot = lOldProject->GetPlotById(roleDataMap[Qt::UserRole + 3].toInt());

                        if (lPlot) {
                            lMoveList.append(lPlot);
                            lMoveStr.append(QString::number(lPlot->Id()) + " - " + lPlot->CodeSheetConst());
                        }
                    }
                }
            }
        }

        if (!lMoveList.isEmpty()) {
            QMessageBox mb(this);
            mb.setWindowTitle(tr("Documents list"));
            mb.setIcon(QMessageBox::Question);
            switch (lDropType) {
            case 1:
                mb.setText(tr("Move selected documents to") + "\n" + lItem->TreeDataConst()->FullName() + "?");
                break;
            case 2:
                mb.setText(tr("Move selected documents to project") + "\n" + mProjectData->FullShortName() + "?");
                break;
            default:
                return;
            }

            mb.setDetailedText(lMoveStr.join("\n"));
            mb.addButton(QMessageBox::Yes);
            mb.setDefaultButton(mb.addButton(QMessageBox::No));
            if (mb.exec() == QMessageBox::Yes) {
                if (!db.transaction()) {
                    gLogger->ShowSqlError(this, tr("Documents list"), tr("Can't start transaction"), db);
                } else {
                    bool lIsOk = false;
                    for (int i = 0; i < lMoveList.length(); i++) {
                        if (!i) lIsOk = true;
                        PlotData * lPlot = lMoveList.at(i);
                        switch (lDropType) {
                        case 1:
                            lPlot->setTDArea(lItem->TreeDataConst()->Area());
                            lPlot->setTDId(lItem->TreeDataConst()->Id());
                            break;
                        case 2:
                            lPlot->setIdProject(mProjectData->Id());
                            break;
                        }
                        if (!lPlot->SaveData()) {
                            lIsOk = false;
                            break;
                        } else {
                            lPlot->LoadVersions();
                            for (int j = 0; j < lPlot->VersionsConst().length(); j++) {
                                PlotData * lPlot2 = lPlot->VersionsConst().at(j);
                                switch (lDropType) {
                                case 1:
                                    lPlot2->setTDArea(lItem->TreeDataConst()->Area());
                                    lPlot2->setTDId(lItem->TreeDataConst()->Id());
                                    break;
                                case 2:
                                    lPlot2->setIdProject(mProjectData->Id());
                                    break;
                                }
                                if (!lPlot2->SaveData()) {
                                    lIsOk = false;
                                    break;
                                }
                            }
                        }
                        if (!lIsOk) break;
                    }
                    if (lIsOk) {
                        if (!db.commit()) {
                            gLogger->ShowSqlError(this, tr("Documents list"), tr("Can't commit"), db);
                            lIsOk = false;
                        }
                    }
                    if (!lIsOk) {
                        db.rollback();
                    }
                    mProjectData->ReinitLists();
                    if (lDropType == 2
                            && lOldProject) {
                        lOldProject->ReinitLists();
                    }
                }
            }
        }
    }

    event->ignore(); // all is done
}

//-------------------------------------------------------------------------------------------------------------
PlotTreeItem::PlotTreeItem(TreeDataRecord * aTreeDataRecord) :
    QTreeWidgetItem(), mTreeDataRecord(aTreeDataRecord)
{
    setText(0, mTreeDataRecord->TextConst());
    if (gSettings->TypeTree.FontPlusOne || gSettings->TypeTree.FontBold) {
        QFont lFont = font(0);
        if (gSettings->TypeTree.FontPlusOne) {
            lFont.setPointSize(lFont.pointSize() + 1);
            /*if (lFont.pointSize() != -1)
                lFont.setPointSize(lFont.pointSize() + 1);
            else if (lFont.pixelSize() != -1)
                lFont.setPixelSize(lFont.pixelSize() + 1);*/
        }
        if (gSettings->TypeTree.FontBold) lFont.setBold(true);
        setFont(0, lFont);
    }
}

TreeDataRecord * PlotTreeItem::TreeDataRef() {
    return mTreeDataRecord;
}

const TreeDataRecord * PlotTreeItem::TreeDataConst() const {
    return mTreeDataRecord;
}

QList<PlotData *> & PlotTreeItem::Plots() {
    return mPlots;
}

const QList<PlotData *> & PlotTreeItem::PlotsConst() const {
    return mPlots;
}
