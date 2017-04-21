#include "QMyMdiSubWindow.h"

#include <QMenu>

#include "../Logger/logger.h"

#include "GlobalSettings.h"
#include "qfcdialog.h"

QMyMdiSubWindow::QMyMdiSubWindow(QWidget *parent, Qt::WindowFlags flags) :
    QMdiSubWindow(parent, flags)
{
    QMenu *lMenu = systemMenu();
    lMenu->addSeparator();
    QAction * lSwitchMDI = lMenu->addAction("MDI");
    lSwitchMDI->setCheckable(true);
    lSwitchMDI->setChecked(true); // always true - we an in MdiSubWindow
    setSystemMenu(lMenu);

    connect(lSwitchMDI, SIGNAL(toggled(bool)), this, SLOT(SwitchToNonMDI(bool)));
}

void QMyMdiSubWindow::SwitchToNonMDI(bool aDummy) {
    if (qobject_cast<QFCDialog *>(widget())) {
        qobject_cast<QFCDialog *>(widget())->MakeNonMDI();
    }
}
