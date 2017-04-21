#ifndef PLOTADDFILESTREEITEM_H
#define PLOTADDFILESTREEITEM_H

#include <QTreeWidgetItem>

#if defined(VPROJECT_MAIN_IMPORT)
    #define PLOT_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(PLOT_LIBRARY)
        #define PLOT_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define PLOT_LIBRARY_EXP_IMP
    #endif
#endif

class PlotAddFileData;

class PLOT_LIBRARY_EXP_IMP PlotAddFilesTreeItem : public QTreeWidgetItem
{
private:
    PlotAddFileData * mAddFile;
public:
    explicit PlotAddFilesTreeItem(PlotAddFileData * aAddFile);
    void ShowData();
    virtual bool operator<(const QTreeWidgetItem & other) const;

    const PlotAddFileData * AddFileConst() const;
};


#endif // PLOTADDFILESTREEITEM_H

