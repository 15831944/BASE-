#include "ProjectTree.h"

#include "../VProject/GlobalSettings.h"

#include "../UsersDlg/UserRight.h"

#include <QStandardItemModel>
#include <QMimeData>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QtWidgets/QHeaderView>

ProjectTree::ProjectTree(QWidget *parent) :
    QTreeWidget(parent),
    mMode(PTMyList), mSelectedId(-1), mSelectedType(ProjectData::PDProject), mCanDragDrop(true)
{
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(OnCurrentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(gProjects, SIGNAL(ProjectListNeedUpdate()), this, SLOT(PopulateTree()));
}

ProjectData * ProjectTree::GetSelectedProject() {
    if (currentItem())
        return static_cast<ProjectTreeItem *>(currentItem())->ProjectRef();
    return NULL;
}

ProjectTree::PTShowMode ProjectTree::Mode() {
    return mMode;
}

void ProjectTree::SetMode(PTShowMode aMode) {
    mMode = aMode;
}

void ProjectTree::SetFilter(QString aFilter) {
    mFilter = aFilter;
}

void ProjectTree::SetSelectedProject(long aIdProject) {
    mSelectedId = aIdProject;
    mSelectedType = ProjectData::PDProject;
}

void ProjectTree::SetSelectedGroup(long aIdGroup) {
    mSelectedId = aIdGroup;
    mSelectedType = ProjectData::PDGroup;
}


void ProjectTree::PopulateTree() {
    //setUpdatesEnabled(false); seem doesn't work
    clear();
    PopulateTreeInternal();
    ShowTree();
    //setUpdatesEnabled(true);
}

void ProjectTree::PopulateTreeInternal(ProjectTreeItem *aParentItem, int aLavel) {
    int i;

    ProjectTreeItem *item, *itemScrollTo = NULL;

    for (i = 0; i < (aParentItem?aParentItem->ProjectRef()->ProjListConst().length():gProjects->ProjListConst().length()); i++) {
        if (aParentItem) {
            item = new ProjectTreeItem(aParentItem->ProjectRef()->ProjListConst().at(i));
            aParentItem->addChild(item);
        } else {
            item = new ProjectTreeItem(gProjects->ProjListConst().at(i));
            addTopLevelItem(item);
        }

        if (mMode == PTMyList && !aParentItem && item->ProjectConst()->Type() == ProjectData::PDGroup)
            item->setFlags(item->flags() & ~(Qt::ItemIsDragEnabled));

        if(!mCanDragDrop)
            item->setFlags(item->flags() & ~(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));

        if (aParentItem
                && aParentItem->ProjectRef()->ProjListConst().at(i)->Id() == mSelectedId
                && aParentItem->ProjectRef()->ProjListConst().at(i)->Type() == mSelectedType
                ||
                !aParentItem
                && gProjects->ProjListConst().at(i)->Id() == mSelectedId
                && gProjects->ProjListConst().at(i)->Type() == mSelectedType) {

            item->setSelected(true);
            setCurrentItem(item);
            itemScrollTo = item;
        }

        PopulateTreeInternal(item, aLavel + 1);
    }

    if (itemScrollTo) scrollToItem(itemScrollTo, QAbstractItemView::PositionAtCenter);

}

void ProjectTree::ShowItemInUserList(ProjectTreeItem *aItem) {
    aItem->setHidden(aItem->InUserList() == 0);
    for (int i = 0; i < aItem->childCount(); i++) {
        ShowItemInUserList(static_cast<ProjectTreeItem *>(aItem->child(i)));
    }
}

void ProjectTree::ShowItemRecently(ProjectTreeItem *aItem) {
    aItem->setHidden(aItem->Recently() == 0);
    for (int i = 0; i < aItem->childCount(); i++) {
        ShowItemRecently(static_cast<ProjectTreeItem *>(aItem->child(i)));
    }
}

void ProjectTree::ShowItemNonArchive(ProjectTreeItem *aItem) {
    aItem->setHidden(aItem->NonArchive() == 0);
    for (int i = 0; i < aItem->childCount(); i++) {
        ShowItemNonArchive(static_cast<ProjectTreeItem *>(aItem->child(i)));
    }
}

void ProjectTree::ShowItemArchive(ProjectTreeItem *aItem) {
    aItem->setHidden(aItem->Archive() == 0);
    for (int i = 0; i < aItem->childCount(); i++) {
        ShowItemArchive(static_cast<ProjectTreeItem *>(aItem->child(i)));
    }
}

void ProjectTree::ShowItemAll(ProjectTreeItem *aItem) {
    aItem->setHidden(false);
    for (int i = 0; i < aItem->childCount(); i++) {
        ShowItemAll(static_cast<ProjectTreeItem *>(aItem->child(i)));
    }
}

void ProjectTree::ShowItemFilter(ProjectTreeItem *aItem) {
    aItem->setHidden(aItem->Contains(mFilter) == 0);
    if (aItem->text(0).contains(mFilter, Qt::CaseInsensitive) == 1
            || aItem->ProjectConst()->Id() == mFilter.toInt()) {
        // all childrens
        ShowItemAll(aItem);
    } else {
        for (int i = 0; i < aItem->childCount(); i++) {
            ShowItemFilter(static_cast<ProjectTreeItem *>(aItem->child(i)));
        }
    }
}

void ProjectTree::ShowTree() {
    for (int i = 0; i < topLevelItemCount(); i++) {
        ProjectTreeItem *item = static_cast<ProjectTreeItem *>(topLevelItem(i));

        if (mFilter.isEmpty()) {
            switch (mMode) {
            case PTMyList:
                ShowItemInUserList(item);
                break;
            case PTWorking:
                ShowItemRecently(item);
                break;
            case PTNonArchive:
                ShowItemNonArchive(item);
                break;
            case PTArchive:
                ShowItemArchive(item);
                break;
            default:
                ShowItemAll(item);
                break;
            }
        } else {
            ShowItemFilter(item);
        }
    }
}

void ProjectTree::dragMoveEvent(QDragMoveEvent * event) {
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        // it is destination item
        ProjectTreeItem * lItem = static_cast<ProjectTreeItem *>(this->itemAt(event->pos()));

        if ((lItem
                && lItem->ProjectConst()
                && (lItem->ProjectConst()->Type() == ProjectData::PDGroup
                    || lItem->ProjectConst()->Type() == ProjectData::PDProject
                    && lItem->ProjectConst()->IdParentProject() == 0))
                || (!lItem && mMode == PTMyList)) {

            QDataStream stream(&event->mimeData()->data("application/x-qabstractitemmodeldatalist"), QIODevice::ReadOnly);

            bool res = false;

            while (!stream.atEnd()) {
                int row, col;
                QMap<int,  QVariant> roleDataMap;
                stream >> row >> col >> roleDataMap;

                if (!col
                        && roleDataMap[Qt::UserRole + 0].toString() == gSettings->BaseNameOnly + "/" + gSettings->CurrentSchema) {
                    if (roleDataMap[Qt::UserRole + 1].toString() == "PROJECT") {
                        ProjectData * lProjectData = gProjects->FindByIdProject(roleDataMap[Qt::UserRole + 2].toInt());
                        if (lProjectData) {
                            if (!lItem
                                    && mMode == PTMyList) {
                                if (!lProjectData->InUserList()) {
                                    // add to my list
                                    res = true;
                                    break;
                                }
                            } else {
                                // move to group, to sub, etc
                                if (!lProjectData->Archived()
                                        && ((lItem->ProjectConst()->Type() == ProjectData::PDGroup && lProjectData->IdParentProject() == 0
                                             && lItem->ProjectConst()->Id() != lProjectData->IdGroup()) /*to group*/
                                            || (lItem->ProjectConst()->Type() == ProjectData::PDProject && lProjectData->IdParentProject() == 0
                                                && lItem->ProjectRef()->IdGroup() == 0 && lProjectData->IdGroup() != 0) /*out of group*/
                                            || (lItem->ProjectConst()->Type() == ProjectData::PDProject && lProjectData->IdParentProject() != 0
                                                && lItem->ProjectConst()->Id() != lProjectData->IdParentProject()
                                                && !lItem->ProjectConst()->Archived()) /*suproject to other project*/)) {
                                    res = true;
                                    break;
                                }
                            }
                        }
                    } else if (roleDataMap[Qt::UserRole + 1].toString() == "GROUP") {
                        // group to my list
                        if (!lItem && mMode == PTMyList)
                            res = true;
                    }
                }
            }

            if (res) {
                QTreeWidget::dragMoveEvent(event);
                return;
            }
        }
    }
    event->ignore();
}

void ProjectTree::dropEvent(QDropEvent * event) {
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        bool lNeedPopulateTree = false;
        // it is destination item
        ProjectTreeItem * lItem = static_cast<ProjectTreeItem *>(this->itemAt(event->pos()));

        if ((lItem
                && lItem->ProjectConst()
                && (lItem->ProjectConst()->Type() == ProjectData::PDGroup
                    || lItem->ProjectConst()->Type() == ProjectData::PDProject
                    && lItem->ProjectConst()->IdParentProject() == 0))
                || (!lItem && mMode == PTMyList)) {

            QDataStream stream(&event->mimeData()->data("application/x-qabstractitemmodeldatalist"), QIODevice::ReadOnly);

            while (!stream.atEnd()) {
                int row, col;
                QMap<int,  QVariant> roleDataMap;
                stream >> row >> col >> roleDataMap;

                QMessageBox mb(this);
                mb.setTextFormat(Qt::RichText);
                mb.setWindowTitle(tr("Projects list"));
                mb.setIcon(QMessageBox::Question);
                mb.addButton(QMessageBox::Yes);
                mb.setDefaultButton(mb.addButton(QMessageBox::No));

                if (!col
                        && roleDataMap[Qt::UserRole + 0].toString() == gSettings->BaseNameOnly + "/" + gSettings->CurrentSchema) {
                    if (roleDataMap[Qt::UserRole + 1].toString() == "PROJECT") {
                        ProjectData * lProjectData = gProjects->FindByIdProject(roleDataMap[Qt::UserRole + 2].toInt());
                        if (lProjectData) {
                            if (!lItem && mMode == PTMyList) {
                                if (/*!lProjectData->InUserList()*/true) {
                                    // add to my list
                                    mb.setText(tr("Add constructions?"));
                                    if (lProjectData->AddToMyList((!lProjectData->ProjListConst().length())?false:(mb.exec() == QMessageBox::Yes)?true:false)) {
                                        mSelectedId = lProjectData->Id();
                                        mSelectedType = ProjectData::PDProject;
                                        lNeedPopulateTree = true;
                                    }
                                }
                            } else {
                                if (!lProjectData->Archived()) {
                                    if (lItem->ProjectConst()->Type() == ProjectData::PDGroup && lProjectData->IdParentProject() == 0
                                            && lItem->ProjectConst()->Id() != lProjectData->IdGroup()) /*to group*/ {
                                        mb.setText(tr("Move project") + "<br><b>" + lProjectData->ShortNameConst() + "</b><br>" + tr("to group") + "<br><b>" + lItem->ProjectConst()->ShortNameConst() + "</b>?");
                                        if (mb.exec() == QMessageBox::Yes) {
                                            bool lRes1 = true;
                                            lProjectData->setIdGroup(lItem->ProjectConst()->Id());

                                            if (mMode == PTMyList
                                                    && !lProjectData->InUserList()) {
                                                lRes1 = lProjectData->AddToMyList(true);
                                            }

                                            if (lRes1
                                                    && lProjectData->SaveData()) {
                                                lProjectData->CommitEdit();
                                                lProjectData->setParent(lItem->ProjectRef());
                                                mSelectedId = lProjectData->Id();
                                                mSelectedType = ProjectData::PDProject;
                                                lNeedPopulateTree = true;
                                            } else {
                                                lProjectData->RollbackEdit();
                                            }
                                        }

                                    } else if ((lItem->ProjectRef()->Type() == ProjectData::PDProject && lProjectData->IdParentProject() == 0
                                                && lItem->ProjectRef()->IdGroup() == 0 && lProjectData->IdGroup() != 0) /*out of group*/) {
                                        mb.setText(tr("Move project") + "<br>" + lProjectData->ShortNameConst() + "<br>" + tr("out of group") + "?");
                                        if (mb.exec() == QMessageBox::Yes) {
                                            lProjectData->setIdGroup(0);
                                            if (lProjectData->SaveData()) {
                                                lProjectData->CommitEdit();
                                                lProjectData->setParent(NULL);
                                                mSelectedId = lProjectData->Id();
                                                mSelectedType = ProjectData::PDProject;
                                                lNeedPopulateTree = true;
                                            } else {
                                                lProjectData->RollbackEdit();
                                            }
                                        }
                                    } else if (lItem->ProjectConst()->Type() == ProjectData::PDProject && lProjectData->IdParentProject() != 0
                                               && lItem->ProjectConst()->Id() != lProjectData->IdParentProject() /*suproject to other project*/) {
                                        mb.setText(tr("Move construction") + "<br><b>" + lProjectData->ShortNameConst() + "</b><br>" + tr("from project")
                                                   + " <b>" + lProjectData->Parent()->FullShortName() + "</b><br>" + tr("to project") + " <b>"
                                                   + lItem->ProjectConst()->FullShortName() + "</b>?");
                                        if (mb.exec() == QMessageBox::Yes) {
                                            lProjectData->setIdParentProject(lItem->ProjectConst()->Id());
                                            if (lProjectData->SaveData()) {
                                                lProjectData->CommitEdit();
                                                lProjectData->setParent(lItem->ProjectRef());
                                                mSelectedId = lProjectData->Id();
                                                mSelectedType = ProjectData::PDProject;
                                                lNeedPopulateTree = true;
                                            } else {
                                                lProjectData->RollbackEdit();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } else if (roleDataMap[Qt::UserRole + 1].toString() == "GROUP") {
                        // add group to my list
                        if (!lItem && mMode == PTMyList) {
                            ProjectData * lGroup = gProjects->FindByIdGroup(roleDataMap[Qt::UserRole + 2].toInt());
                            mb.setText(tr("Add group") + " <b>" + lGroup->ShortNameConst() + "</b> " + tr("to my list") + "?");
                            if (mb.exec() == QMessageBox::Yes) {
                                for (int i = 0; i < lGroup->ProjListConst().length(); i++) {
                                    lGroup->ProjListConst().at(i)->AddToMyList(true);
                                    lNeedPopulateTree = true;
                                }
                                mSelectedId = lGroup->Id();
                                mSelectedType = ProjectData::PDGroup;
                            }
                        }
                    }
                }
            }
        }

        if (lNeedPopulateTree) {
            emit gProjects->ProjectListNeedUpdate();
        }
    }
    event->ignore();
}

void ProjectTree::keyPressEvent(QKeyEvent * event) {
    if (event->nativeVirtualKey() == 13) {
        emit this->doubleClicked(currentIndex());
    } else {
        QTreeWidget::keyPressEvent(event);
    }
}

void ProjectTree::OnCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous) {
    // it is used for correct update on signel "list reloaded"
    if (current) {
        mSelectedId = static_cast<ProjectTreeItem *>(current)->ProjectConst()->Id();
        mSelectedType = static_cast<ProjectTreeItem *>(current)->ProjectConst()->Type();
    } else {
        //mSelectedId = -1;
    }
}

//-------------------------------------------------------------------------------------------------------
ProjectTreeItem::ProjectTreeItem(ProjectData *aProjectData) :
    QTreeWidgetItem(), mProjectData(aProjectData)
{
    setText(0, aProjectData->ShortNameConst());

    // it is data for drag&drop
    setData(0, Qt::UserRole + 0, gSettings->BaseNameOnly + "/" + gSettings->CurrentSchema); // database/schema id; for drag & dropping in one database
    switch (aProjectData->Type()) {
    case ProjectData::PDProject:
        setData(0, Qt::UserRole + 1, "PROJECT"); // data type
        break;
    case ProjectData::PDGroup:
        setData(0, Qt::UserRole + 1, "GROUP"); // data type
        break;
    }
    setData(0, Qt::UserRole + 2, aProjectData->Id()); // id of data

    if (aProjectData->Type() == ProjectData::PDGroup) {
        QFont lFont = font(0);
        lFont.setBold(true);
        setFont(0, lFont);
        //setFlags(flags() & ~Qt::ItemIsDragEnabled); // can't drag group
    } else {
        if (aProjectData->IdParentProject()) {
            setFlags(flags() & ~Qt::ItemIsDropEnabled); // can't drop on construction
            if (!gUserRight->CanUpdate("v_project", "id_project"))
                setFlags(flags() & ~Qt::ItemIsDragEnabled);
        } else {
            if (!gUserRight->CanUpdate("v_project", "id_group"))
                setFlags(flags() & ~Qt::ItemIsDragEnabled);
        }
    }
    //setText(1, QString::number(aArchived));
}
