#ifndef PLOTHISTORYTREE_H
#define PLOTHISTORYTREE_H

#include <QTreeWidget>

#if defined(VPROJECT_MAIN_IMPORT)
    #define PLOT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PLOT_LIBRARY)
        #define PLOT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PLOT_LIBRARY_EXP_IMP
    #endif
#endif

class PlotHistTreeItem;

class PLOT_LIBRARY_EXP_IMP PlotHistoryTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PlotHistoryTree(QWidget *parent);
    virtual ~PlotHistoryTree();

    PlotHistTreeItem *itemFromIndex(const QModelIndex & index) const;
};

#endif // PLOTHISTORYTREE_H
