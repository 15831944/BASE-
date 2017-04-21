#ifndef PLOTHISTTREEITEM_H
#define PLOTHISTTREEITEM_H

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

class PlotData;
class PlotHistoryData;

class PLOT_LIBRARY_EXP_IMP PlotHistTreeItem : public QTreeWidgetItem
{
private:
    int mIdPlot;
    PlotData * mPlot;
    PlotHistoryData * mHistory;
    PlotHistTreeItem *mItemPrev;
public:
    explicit PlotHistTreeItem(PlotData * aPlot, PlotHistoryData * aHistory, PlotHistTreeItem *aItemPrev);
    void ShowData();
    //void ShowTempFileName(bool aShow);
    //void ShowSavedFromFileName(bool aShow);

    int IdPlot() const;

    const PlotData * PlotConst() const;
    PlotData * PlotRef();

    const PlotHistoryData * HistoryConst() const;
    PlotHistoryData * HistoryRef();
};


#endif // PLOTHISTTREEITEM_H

