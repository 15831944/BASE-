#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#pragma warning(disable:4100)

#include "ProjectData.h"

#include "../VProject/TreeData.h"

#include <QTreeWidget>

class ProjectTreeItem : public QTreeWidgetItem
{
protected:
    ProjectData *mProjectData;
public:
    explicit ProjectTreeItem(ProjectData *aProjectData);

    const ProjectData * ProjectConst() const { return mProjectData; }
    ProjectData * ProjectRef() { return mProjectData; }

    int InUserList() {
        if (mProjectData->InUserList() == 1) return 1;
        // empty group appear in all lists
        if (mProjectData->Type() == ProjectData::PDGroup && !childCount()) return 1;
        for (int i = 0; i < childCount(); i++) {
            ProjectTreeItem *item = static_cast<ProjectTreeItem *>(child(i));
            if (item->InUserList() == 1) return 1;
        };
        return 0;
    }

    int Recently() {
        if (mProjectData->Recently() == 1) return 1;
        // empty group appear in all lists
        if (mProjectData->Type() == ProjectData::PDGroup && !childCount()) return 1;
        for (int i = 0; i < childCount(); i++) {
            ProjectTreeItem *item = static_cast<ProjectTreeItem *>(child(i));
            if (item->Recently() == 1) return 1;
        };
        return 0;
    }

    int NonArchive() {
        if (mProjectData->Archived() == 0) return 1;
        // empty group appear in all lists
        if (mProjectData->Type() == ProjectData::PDGroup && !childCount()) return 1;
        for (int i = 0; i < childCount(); i++) {
            ProjectTreeItem *item = static_cast<ProjectTreeItem *>(child(i));
            if (item->NonArchive() == 1) return 1;
        };
        return 0;
    }

    int Archive() {
        if (mProjectData->Archived() == 1) return 1;
        // empty group appear in all lists
        if (mProjectData->Type() == ProjectData::PDGroup && !childCount()) return 1;
        for (int i = 0; i < childCount(); i++) {
            ProjectTreeItem *item = static_cast<ProjectTreeItem *>(child(i));
            if (item->Archive() == 1) return 1;
        };
        return 0;
    }

    int Contains(const QString &str) {
        if (text(0).contains(str, Qt::CaseInsensitive)) return 1;
        if (mProjectData->Id() == str.toInt()) return 1;
        for (int i = 0; i < childCount(); i++) {
            ProjectTreeItem *item = static_cast<ProjectTreeItem *>(child(i));
            if (item->Contains(str) == 1) return 1;
        };
        return 0;
    }

};

class ProjectTree : public QTreeWidget
{
    Q_OBJECT
public:
    enum PTShowMode { PTMyList, PTWorking, PTNonArchive, PTArchive, PTAll };

    explicit ProjectTree(QWidget *parent = 0);

    ProjectData * GetSelectedProject();
    PTShowMode Mode();
    void SetMode(PTShowMode aMode); // 0 - my list, 1 - working, 2 - non-archive, 3 - archive, 4 - all, 5 - filter
    void SetFilter(QString aFilter); // filter for short project names

    void SetSelectedProject(long aIdProject);
    void SetSelectedGroup(long aIdGroup);
    void SetCanDragDrop(bool aCanDragDrop) { mCanDragDrop = aCanDragDrop; }

    void ShowTree();
protected:
    PTShowMode mMode;
    long mSelectedId;
    ProjectData::PDType mSelectedType;
    QString mFilter;
    bool mCanDragDrop;

    void PopulateTreeInternal(ProjectTreeItem *aParentItem = NULL, int aLavel = 0);

    void ShowItemInUserList(ProjectTreeItem *aItem);
    void ShowItemRecently(ProjectTreeItem *aItem);
    void ShowItemNonArchive(ProjectTreeItem *aItem);
    void ShowItemArchive(ProjectTreeItem *aItem);
    void ShowItemAll(ProjectTreeItem *aItem);

    void ShowItemFilter(ProjectTreeItem *aItem);

    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
signals:
    
public slots:
    void PopulateTree();
protected slots:
    void OnCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous);
    
};

#endif // PROJECTTREE_H
