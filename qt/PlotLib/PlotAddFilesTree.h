#ifndef PLOTADDFILESTREE_H
#define PLOTADDFILESTREE_H

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

class PlotAddFilesTreeItem;

class PLOT_LIBRARY_EXP_IMP PlotAddFilesTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PlotAddFilesTree(QWidget *parent);
    virtual ~PlotAddFilesTree();

    PlotAddFilesTreeItem *itemFromIndex(const QModelIndex & index) const;
};

#endif // PLOTADDFILESTREE_H
