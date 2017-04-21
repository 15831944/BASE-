#include "PlotAddFilesTree.h"
#include "PlotAddFilesTreeItem.h"

PlotAddFilesTree::PlotAddFilesTree(QWidget *parent) :
    QTreeWidget(parent)
{

}

PlotAddFilesTree::~PlotAddFilesTree() {
}

PlotAddFilesTreeItem *PlotAddFilesTree::itemFromIndex(const QModelIndex & index) const {
    return static_cast<PlotAddFilesTreeItem *>(QTreeWidget::itemFromIndex(index));
}
