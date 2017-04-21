#include "PlotHistoryTree.h"
#include "PlotHistTreeItem.h"

PlotHistoryTree::PlotHistoryTree(QWidget *parent) :
    QTreeWidget(parent)
{

}

PlotHistoryTree::~PlotHistoryTree() {

}

PlotHistTreeItem *PlotHistoryTree::itemFromIndex(const QModelIndex & index) const {
    return static_cast<PlotHistTreeItem *>(QTreeWidget::itemFromIndex(index));
}
