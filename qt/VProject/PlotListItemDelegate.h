#ifndef PLOTLISTITEMDELEGATE_H
#define PLOTLISTITEMDELEGATE_H

#pragma warning(disable:4100)

#include <QStyledItemDelegate>
#include <QComboBox>

#include "def_expimp.h"

// for paint with grid
class EXP_IMP PlotListItemDelegate : public QStyledItemDelegate
{
public:
    explicit PlotListItemDelegate(QWidget *parent = 0);
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

// SD for Save Dialog
class SDDocItemDelegate : public PlotListItemDelegate
{
    Q_OBJECT
public:
    explicit SDDocItemDelegate(QWidget *parent = 0);
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class SDFilesItemDelegate : public PlotListItemDelegate
{
    Q_OBJECT
public:
    explicit SDFilesItemDelegate(QWidget *parent = 0);
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class SDXrefsItemDelegate : public PlotListItemDelegate
{
    Q_OBJECT
public:
    explicit SDXrefsItemDelegate(QWidget *parent = 0);
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const;
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
private slots:
    void indexChanged(int);
};

// RO - for user can select all and copy
class EXP_IMP ROListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ROListItemDelegate(QWidget *parent = 0);
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

// RO - for user can select all and copy
class EXP_IMP ROPlotListItemDelegate : public PlotListItemDelegate
{
    Q_OBJECT
public:
    explicit ROPlotListItemDelegate(QWidget *parent = 0);
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // PLOTLISTITEMDELEGATE_H
