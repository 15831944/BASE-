#include "PlotListItemDelegate.h"
#include "GlobalSettings.h"
#include "TWIForSave.h"

#include <QPainter>
#include <QComboBox>

#include "../PlotLib/DwgData.h"

PlotListItemDelegate::PlotListItemDelegate(QWidget *parent) :
    QStyledItemDelegate(parent)
{
}

void PlotListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QStyledItemDelegate::paint(painter, option, index);

    if (gSettings->DocumentTree.ShowGridLines) {

        QRect r = option.rect;

        painter->save();
        // line at right
        if (index.column() < ((QTreeWidget *) option.widget)->columnCount()) {
            if (option.state & QStyle::State_Selected) {
                // it is default white for selected row
                QPen lPen(painter->pen());
                lPen.setColor(option.palette.highlightedText().color());
                painter->setPen(lPen);
            }
            painter->drawLine(r.right(), r.top(), r.right(), r.bottom());
        }

        // line at top
        if (/*(index.row() || index.parent() != QModelIndex())
                && */(index.parent() == QModelIndex() || index.column() > 0)) {
            if (option.state & QStyle::State_Selected) {
                QPen lPen(painter->pen());
                lPen.setColor(option.palette.highlightedText().color());
                painter->setPen(lPen);
            }
            painter->drawLine(r.left(), r.top(), r.right(), r.top());
        }
        painter->restore();
    }
}

SDDocItemDelegate::SDDocItemDelegate(QWidget *parent) :
    PlotListItemDelegate(parent)
{
}

QWidget *SDDocItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    if (index.column() == 1/* && qobject_cast<QTreeWidget *> (this->parent())*/) {
        //QTreeWidget *tw = qobject_cast<QTreeWidget *> (this->parent());
        QLineEdit *l = new QLineEdit(parent);

        return l;
    }

    return NULL;
}

SDFilesItemDelegate::SDFilesItemDelegate(QWidget *parent) :
    PlotListItemDelegate(parent)
{
}

QWidget *SDFilesItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    if (index.column() == 2 && qobject_cast<QTreeWidget *> (this->parent())) {
        QTreeWidget *tw = qobject_cast<QTreeWidget *> (this->parent());
        const QList<int> & lIds = ((TWIForSaveAddFile *) tw->topLevelItem(index.row()))->Ids();
        QComboBox *l = new QComboBox(parent);

        for (int i = 0; i < lIds.length(); i++)
            l->addItem(QString::number(lIds.at(i)));
        return l;
    }

    return NULL;
}

SDXrefsItemDelegate::SDXrefsItemDelegate(QWidget *parent) :
    PlotListItemDelegate(parent)
{
}

void SDXrefsItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
    PlotListItemDelegate::setEditorData(editor, index);
    if (index.column() == 1 && qobject_cast<QLineEdit *> (editor)) {
        // if new name is empty - get block name (original name)
        if (qobject_cast<QLineEdit *> (editor)->text().isEmpty()) {
            qobject_cast<QLineEdit *> (editor)->setText(index.sibling(index.row(), 0).data().toString());
        }
    }
}

void SDXrefsItemDelegate::indexChanged(int) {
    emit commitData(qobject_cast<QWidget *>(parent())->focusWidget());
}

QWidget *SDXrefsItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    if (index.column() == 1
            && index.parent() == QModelIndex()) {
        return PlotListItemDelegate::createEditor(parent, option, index);
    } else if (index.column() == 3
               && qobject_cast<QTreeWidget *> (this->parent())) {
        QTreeWidget *tw = qobject_cast<QTreeWidget *> (this->parent());

        TWIForSaveXrefTop * item = static_cast<TWIForSaveXrefTop *>(tw->topLevelItem(index.row()));

        if (item->VarList().count() > 1) {
            QComboBox * lCB = new QComboBox(parent);

            foreach (QString s1, item->VarList().keys()) lCB->addItem(s1);

            lCB->setCurrentText(QString::number(item->XrefConst()->Id()) + " - " +
                                QString::number(item->XrefConst()->DwgConst()->Version()) + "/" +
                                QString::number(item->XrefConst()->DwgVersionMax()));

            // commit on indexChanged
            connect(lCB, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));

            return lCB;
        }
    }

    return NULL;
}

//----------------
ROListItemDelegate::ROListItemDelegate(QWidget *parent) :
    QStyledItemDelegate(parent)
{
}

QWidget *ROListItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    if (!index.data().toString().isEmpty()) {
        QLineEdit *l = new QLineEdit(parent);
        l->setReadOnly(true);
        l->setFrame(false);
        QPalette lPalette = l->palette();
        lPalette.setColor(QPalette::Base, parent->palette().color(QPalette::Window));
        l->setPalette(lPalette);
        return l;
    } else {
        return NULL;
    }
}

//----------------
ROPlotListItemDelegate::ROPlotListItemDelegate(QWidget *parent) :
    PlotListItemDelegate(parent)
{
}

QWidget *ROPlotListItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    if (!index.data().toString().isEmpty()) {
        QLineEdit *l = new QLineEdit(parent);
        l->setReadOnly(true);
        //l->setFrame(false);
        return l;
    } else {
        return NULL;
    }
}
