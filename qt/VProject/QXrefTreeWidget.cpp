#include "QXrefTreeWidget.h"
#include "common.h"
#include "TWIForSave.h"

#include <QMimeData>
#include <QDragMoveEvent>
#include <QDropEvent>

QXrefTreeWidget::QXrefTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void QXrefTreeWidget::dragMoveEvent(QDragMoveEvent * event) {
    if (event->source() == this
            && currentItem()
            && currentItem()->parent()) {
        TWIForSaveXrefTop * itemSrc = (TWIForSaveXrefTop *) currentItem();
        TWIForSaveXrefChild * itemSrcPar = (TWIForSaveXrefChild *) itemSrc->parent();

        QTreeWidgetItem *itemEvent = itemAt(event->pos());
        if (itemEvent) {
            TWIForSaveXrefTop * itemDest = static_cast<TWIForSaveXrefTop *>(itemEvent->parent()?itemEvent->parent():itemEvent);
            if (itemDest->XrefConst()->IdCommon() == itemSrcPar->XrefConst()->IdCommon()) {
                QTreeWidget::dragMoveEvent(event);
                return;
            }
        }
    }
    event->ignore();
}

void QXrefTreeWidget::dropEvent(QDropEvent * event) {
    //QMessageBox::critical(NULL, "BlobMemCache::AddToCache", QString::number(event->dropAction()));
    //QMessageBox::critical(NULL, "BlobMemCache::AddToCache", event->source()->metaObject()->className());
    if (event->source() == this
            && currentItem()
            && currentItem()->parent()) {
        TWIForSaveXrefTop *itemSrc = static_cast<TWIForSaveXrefTop *>(currentItem());
        TWIForSaveXrefChild *itemSrcPar = static_cast<TWIForSaveXrefChild *>(itemSrc->parent());

        QTreeWidgetItem *itemEvent = itemAt(event->pos());
        if (itemEvent) {
            TWIForSaveXrefTop * itemDest = static_cast<TWIForSaveXrefTop *>(itemEvent->parent()?itemEvent->parent():itemEvent);
            if (itemDest->XrefConst()->IdCommon() == itemSrcPar->XrefConst()->IdCommon()) {
                // take child from old parent
                itemSrcPar->takeChild(itemSrcPar->indexOfChild(itemSrc));
                // add it to new parent
                itemDest->addChild(itemSrc);

                emit TreeChanged(); // emit tvoyu mat'
            }
        }
    }
}
