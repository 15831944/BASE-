#ifndef PLOTTREE_H
#define PLOTTREE_H

#include <QTreeWidget>
//#include "../PlotLib/PlotData.h"
#include "TreeData.h"

class PlotTreeItem;
class ProjectData;
class PlotNamedListData;
class PlotData;

class PlotTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PlotTree(QWidget *parent = 0);

    TreeDataRecord * GetSelected() const;
    //void SetSelected(int aArea, int aId) { mArea = aArea; mId = aId; }
    void SetSelected(const TreeDataRecord * aTreeDataSelected);

    int IdProject() const;
    void SetIdProject(int aIdProject);
    void SetProjectData(ProjectData *aProjectData);
    void SetHideEmpty(bool aHideEmpty);
    void SetHideCancelled(bool aHideCancelled);
    void SetComplect(const QString &aComplect);
    void SetList(const PlotNamedListData * aNamedList);

    ProjectData * ProjectRef();
    const ProjectData * ProjectConst() const;

    void ShowVisibility();
    void PopulateTree();

protected:
    const TreeDataRecord * mTreeDataSelected;
    int mIdProject;
    ProjectData * mProjectData;

    bool mHideEmpty, mHideCancelled;
    QString mComplect;
    const PlotNamedListData * mNamedList;

    //void ShowAllVisible(QDocTreeItem *aItem);
    bool ShowVisibilityItem(PlotTreeItem *aItem);
    void PopulateTreeInternal(PlotTreeItem *aParent = NULL, int aLevel = 0);

    PlotTreeItem * GetDocTreeItemInternal(PlotTreeItem * aItem, int aTreeDataArea, int aTreeDataId);
    PlotTreeItem * GetDocTreeItem(int aTreeDataArea, int aTreeDataId);

    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);
signals:
    
public slots:

};

class PlotTreeItem : public QTreeWidgetItem
{
protected:
    TreeDataRecord * mTreeDataRecord; // need not delete, it's not an owner
    QList<PlotData *> mPlots; // need not delete, it's not an owner
public:
    explicit PlotTreeItem(TreeDataRecord * aTreeDataRecord);

    TreeDataRecord * TreeDataRef();
    const TreeDataRecord * TreeDataConst() const;

    QList<PlotData *> & Plots();
    const QList<PlotData *> & PlotsConst() const;
};

#endif // PLOTTREE_H
