#ifndef QXREFTREEWIDGET_H
#define QXREFTREEWIDGET_H

#include <QTreeWidget>

class QXrefTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QXrefTreeWidget(QWidget *parent = 0);

protected:
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);
signals:
    void TreeChanged(); // call when drag$drop ended
public slots:

};

#endif // QXREFTREEWIDGET_H
