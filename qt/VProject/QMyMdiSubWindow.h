#ifndef QMYMDISUBWINDOW_H
#define QMYMDISUBWINDOW_H

#include <QMdiSubWindow>

class QMyMdiSubWindow : public QMdiSubWindow
{
    Q_OBJECT
public:
    QMyMdiSubWindow(QWidget * parent = 0, Qt::WindowFlags flags = 0);
public slots:
    void SwitchToNonMDI(bool aDummy);
};

#endif // QMYMDISUBWINDOW_H
