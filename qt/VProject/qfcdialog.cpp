#include <QTreeView>
#include <QTableWidget>
#include <QDockWidget>
#include <QApplication>
#include <QWindowList>
#include <QWindow>
#include <QMdiSubWindow>
#include <QSplitter>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>

#include "qfcdialog.h"
#include "GlobalSettings.h"
#include "QMyMdiSubWindow.h"
#include "MainWindow.h"

#include "PlotListTree.h"

QFCDialog::QFCDialog(QWidget *parent, bool aAutoSaveState) :
    QDialog(parent), mJustStarted(true), mLoadSettings(true), mAutoSaveState(aAutoSaveState),
    mDoNotSave(false), ReadVersion(0),
    CurrentVersion(2) // it is version when i split it for different windows; before it was just one global variable
{
    connect(gSettings, SIGNAL(StyleSheetChanged()), this, SLOT(StyleSheetChangedSlot()));
}

QFCDialog::~QFCDialog() {
    // close parent if we in mdi
    if (qobject_cast<QMdiSubWindow *>(parent())) {
        qobject_cast<QMdiSubWindow *>(parent())->close();
    }

    if (mAutoSaveState && !gMainWindow->InLoadWindows()) {
        QSettings settings2;
        settings2.remove("WinState");
        settings2.beginGroup("WinState");
        gMainWindow->SaveMDIWindows(settings2);
        settings2.endGroup();
    }
}

QString QFCDialog::AddToClassName() const {
    return "";
}

void QFCDialog::ReadChildren(QSettings &settings, const QObjectList &c) {
    for (int i = 0; i < c.length(); i++) {
        c.at(i)->installEventFilter(this);
        if (ReadVersion == CurrentVersion) {
            if (qobject_cast<QTreeView *> (c.at(i))) {
                if (!qobject_cast<PlotListTree *> (c.at(i))) {
                    (qobject_cast<QTreeView *> (c.at(i)))->header()->restoreState(settings.value(c.at(i)->objectName()).toByteArray());
                }
            } else if (qobject_cast<QTableView *> (c.at(i))) {
                (qobject_cast<QTableView *> (c.at(i)))->horizontalHeader()->restoreState(settings.value(c.at(i)->objectName() + ".hor").toByteArray());
                //(qobject_cast<QTableView *> (c.at(i)))->verticalHeader()->restoreState(settings.value(c.at(i)->objectName() + ".vert").toByteArray());
            } else if (qobject_cast<QSplitter *> (c.at(i))) {
                (qobject_cast<QSplitter *> (c.at(i)))->restoreState(settings.value(c.at(i)->objectName()).toByteArray());
            } else if (qobject_cast<QDockWidget *> (c.at(i))) {
                QDockWidget *dw = qobject_cast<QDockWidget *> (c.at(i));
                bool lIsFloating, lIsVisible;

                lIsFloating = settings.value(c.at(i)->objectName() + ".Floating", dw->isFloating()).toBool();
                lIsVisible = settings.value(c.at(i)->objectName() + ".Visible", dw->isVisible()).toBool();

                dw->setFloating(lIsFloating);
                dw->setVisible(lIsVisible || (dw->features() & QDockWidget::DockWidgetClosable) == 0);
                dw->restoreGeometry(settings.value(c.at(i)->objectName()).toByteArray());
            } else if (qobject_cast<QTabWidget *> (c.at(i))) {
                (qobject_cast<QTabWidget *> (c.at(i)))->setCurrentIndex(
                            settings.value((qobject_cast<QTabWidget *> (c.at(i)))->objectName() + ".Curr").toInt());
                QObjectList tabChildren;
                for (int j = 0; j < (qobject_cast<QTabWidget *> (c.at(i)))->count(); j++) {
                    tabChildren.append((qobject_cast<QTabWidget *> (c.at(i)))->widget(j));
//                    ((qobject_cast<QTabWidget *> (c.at(i)))->widget(j))->restoreGeometry(
//                                settings.value(((qobject_cast<QTabWidget *> (c.at(i)))->widget(j))->objectName() + ".Geometry").toByteArray());
                }
                ReadChildren(settings, tabChildren);
            }
        }
        if (c.at(i)->children().length()) {
            ReadChildren(settings, c.at(i)->children());
        }
    }
}

void QFCDialog::SaveState(QSettings &aSettings) {
}

void QFCDialog::LoadSettings(QSettings &settings) {
    ReadVersion = settings.value("Version", 0).toInt();
    if (ReadVersion == CurrentVersion) {
        restoreGeometry(settings.value("Geometry").toByteArray());
    }
    // it is also set "eventfilter"; think i should plotpro, i need not event-filter
    ReadChildren(settings, children());

    settings.beginGroup("Parameters");
    LoadAdditionalSettings(settings);
    settings.endGroup();
}

void QFCDialog::showEvent(QShowEvent* event) {
    if (mJustStarted) {
        if (mLoadSettings) {
            LoadWindowDefaults();
        }
    }

    QDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        if (!isModal()) {
            HMENU lMenu = ::GetSystemMenu((HWND) winId(), FALSE);
            ::AppendMenu(lMenu, MF_SEPARATOR, 0, NULL);
            ::AppendMenu(lMenu, MF_STRING | (qobject_cast<QMdiSubWindow *>(parent())?MF_CHECKED:0), 1, L"MDI");
        }

        //QMessageBox::critical(NULL, "", QString(parentWidget()->metaObject()->className()));
        if (parentWidget()) {
            setLayoutDirection(parentWidget()->layoutDirection());
        }
//        if (!qobject_cast<QMdiSubWindow *> (parent())) {
//            if (!gSettings->BaseName.isEmpty())
//                setWindowTitle(windowTitle() + " - " + gSettings->BaseName);
//        }
        if (!parent()
                && !gSettings->BaseName.isEmpty()) {
            setWindowTitle(windowTitle() + " - " + gSettings->BaseName);
        }
    }

    if (mAutoSaveState && !gMainWindow->InLoadWindows()) {
        QSettings settings2;
        settings2.remove("WinState");
        settings2.beginGroup("WinState");
        gMainWindow->SaveMDIWindows(settings2);
        settings2.endGroup();
    }
}

//void QFCDialog::closeEvent(QCloseEvent* event) {
//    QMdiSubWindow::closeEvent(event);

//    QSettings settings;
//    settings.setValue(QString(this->metaObject()->className()) + "/Geometry", saveGeometry());
//    settings.setValue(QString(this->metaObject()->className()) + "/StyleSheet", styleSheet());
//};

void QFCDialog::SaveChildren(QSettings &settings, const QObjectList &c) {
    for (int i = 0; i < c.length(); i++) {
        if (qobject_cast<QTreeView *> (c.at(i))) {
            if (!qobject_cast<PlotListTree *> (c.at(i))) {
                settings.setValue(c.at(i)->objectName(), (qobject_cast<QTreeView *> (c.at(i)))->header()->saveState());
            }
        } else if (qobject_cast<QTableView *> (c.at(i))) {
            settings.setValue(c.at(i)->objectName() + ".hor", (qobject_cast<QTableView *> (c.at(i)))->horizontalHeader()->saveState());
            //settings.setValue(c.at(i)->objectName() + ".vert", (qobject_cast<QTableView *> (c.at(i)))->verticalHeader()->saveState());
        } else if (qobject_cast<QSplitter *> (c.at(i))) {
            settings.setValue(c.at(i)->objectName(), (qobject_cast<QSplitter *> (c.at(i)))->saveState());
        } else if (qobject_cast<QDockWidget *> (c.at(i))) {
            settings.setValue(c.at(i)->objectName(), (qobject_cast<QDockWidget *> (c.at(i)))->saveGeometry());
            settings.setValue(c.at(i)->objectName() + ".Visible", (qobject_cast<QDockWidget *> (c.at(i)))->isVisible());
            settings.setValue(c.at(i)->objectName() + ".Floating", (qobject_cast<QDockWidget *> (c.at(i)))->isFloating());
        } else if (qobject_cast<QTabWidget *> (c.at(i))) {
            settings.setValue((qobject_cast<QTabWidget *> (c.at(i)))->objectName() + ".Curr",
                              (qobject_cast<QTabWidget *> (c.at(i)))->currentIndex());
            QObjectList tabChildren;
            for (int j = 0; j < (qobject_cast<QTabWidget *> (c.at(i)))->count(); j++) {
                tabChildren.append((qobject_cast<QTabWidget *> (c.at(i)))->widget(j));
                //                settings.setValue(((qobject_cast<QTabWidget *> (c.at(i)))->widget(j))->objectName() + ".Geometry",
//                                  ((qobject_cast<QTabWidget *> (c.at(i)))->widget(j))->saveGeometry());
            }
            SaveChildren(settings, tabChildren);
        }
        if (c.at(i)->children().length()) {
            SaveChildren(settings, c.at(i)->children());
        }
    }
}

void QFCDialog::SaveSettings(QSettings &settings) {
    settings.setValue("Version", CurrentVersion);
    if (parentWidget() && qobject_cast<QMdiSubWindow *>(parentWidget())) {
        settings.setValue("Geometry", parentWidget()->saveGeometry());
    } else {
        settings.setValue("Geometry", saveGeometry());
    }
    SaveChildren(settings, children());

    settings.beginGroup("Parameters");
    SaveAdditionalSettings(settings);
    settings.endGroup();
}

void QFCDialog::LoadWindowDefaults() {
    QSettings settings;
    mLoadSettings = false;
    settings.beginGroup("Windows");
    settings.beginGroup(metaObject()->className() + AddToClassName());
    LoadSettings(settings);
    settings.endGroup();
    settings.endGroup();
}

void QFCDialog::SaveWindowDefaults() {
    QSettings settings;
    settings.beginGroup("Windows");
    settings.beginGroup(metaObject()->className() + AddToClassName());
    SaveSettings(settings);
    settings.endGroup();
    settings.endGroup();
}

void QFCDialog::MakeNonMDI() {
    if (!qobject_cast<QMdiSubWindow *>(parent())) return;

    QMdiSubWindow * lOldParent = qobject_cast<QMdiSubWindow *>(parent());
    gMainWindow->MdiArea()->removeSubWindow(lOldParent);
    lOldParent->setWidget(NULL);
    setParent(gMainWindow);
    setWindowFlags(lOldParent->windowFlags());

    QRect lGeom = geometry();
    lGeom.setTopLeft((lOldParent->pos() == QPoint(0, 0))?mapToParent(lOldParent->pos()):lOldParent->pos());
    lGeom.setSize(lOldParent->size());
    setGeometry(lGeom);

    lOldParent->close();
    show();

    ::GetSystemMenu((HWND) winId(), TRUE); // it means reset to default
    HMENU lMenu = ::GetSystemMenu((HWND) winId(), FALSE);
    ::AppendMenu(lMenu, MF_SEPARATOR, 0, NULL);
    ::AppendMenu(lMenu, MF_STRING | (qobject_cast<QMdiSubWindow *>(parent())?MF_CHECKED:0), 1, L"MDI");

}

void QFCDialog::MakeMDI() {
    if (qobject_cast<QMdiSubWindow *>(parent())) return;

    QMdiSubWindow * msw = new QMyMdiSubWindow(gMainWindow->MdiArea());
    msw->setAttribute(Qt::WA_DeleteOnClose);
    msw->setWidget(this);
    gMainWindow->MdiArea()->addSubWindow(msw);
    msw->show();
    QRect lGeom = msw->geometry();
    lGeom.setSize(size());
    msw->setGeometry(lGeom);
}

void QFCDialog::done(int r) {
    if (!mDoNotSave) {
        SaveWindowDefaults();
    }

    QDialog::done(r);
}

int QFCDialog::exec() {
    // no save for modal dialogs
    mAutoSaveState = false;
    return QDialog::exec();
}

void QFCDialog::StyleSheetChangedSlot() {
    StyleSheetChangedInSescendant(); // for auto width in lists/trees
}

bool QFCDialog::nativeEvent(const QByteArray & eventType, void * message, long * result) {
    MSG *lMSG = (MSG *) message;

    if (lMSG->message == WM_SYSCOMMAND
            && lMSG->wParam == 1) {
        MakeMDI();
        return true;
    } else if (lMSG->message == WM_INITMENUPOPUP
               && !isModal()) {
//        //::GetSystemMenu((HWND) winId(), TRUE);
//        HMENU lMenu = ::GetSystemMenu((HWND) winId(), FALSE);
//        ::AppendMenu(lMenu, MF_SEPARATOR, 0, NULL);
//        ::AppendMenu(lMenu, MF_STRING | (qobject_cast<QMdiSubWindow *>(parent())?MF_CHECKED:0), 1, L"MDI");
    }

    return QDialog::nativeEvent(eventType, message, result);
}

void QFCDialog::StyleSheetChangedInSescendant() {
}

void QFCDialog::LoadAdditionalSettings(QSettings &aSettings) {
}

void QFCDialog::SaveAdditionalSettings(QSettings &aSettings) {
}

bool QFCDialog::eventFilter(QObject *obj, QEvent *event) {
    if (qobject_cast<QMdiSubWindow *>(obj)) {
        //gLogger->ShowErrorInList(NULL, "QFCDialog::eventFilter", QString::number(event->type(), 10), false);
    } else if (event->type() == QEvent::KeyPress
            && qobject_cast<QDialog *> (obj->parent())) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

//        if ((keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
//            QMessageBox::critical(this, tr("Project list"), QString::number(keyEvent->key()));
//        }
        if ((keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier
                && (keyEvent->key() == 43 || keyEvent->key() == 45 || keyEvent->key() == 48)) {
            QFont lFont = QApplication::font();
            int lPrevFontSize = lFont.pointSize();
            int lNewFontSize = lPrevFontSize;

            if (keyEvent->key() == 43 && lPrevFontSize < 18) lNewFontSize = lPrevFontSize + 1;
            else if (keyEvent->key() == 45 && lPrevFontSize > 6) lNewFontSize = lPrevFontSize - 1;
            else if (keyEvent->key() == 48) lNewFontSize = 8;

            if (lNewFontSize != lPrevFontSize) {
                lFont.setPointSize(lNewFontSize);
                QApplication::setFont(lFont);
                emit gSettings->StyleSheetChanged();
            }
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

//void QFCDialog::changeEvent(QEvent * event) {
//    if (!mJustStarted
//            && event->type() == QEvent::WindowStateChange
//            && isVisible()
//            && qobject_cast<QMdiSubWindow *>(parent())
//            && qobject_cast<QMdiSubWindow *>(parent())->mdiArea()->activeSubWindow() == qobject_cast<QMdiSubWindow *>(parent())
//            && qobject_cast<QMdiSubWindow *>(parent())->windowState() == 0) {
//        //QMessageBox::critical(this, metaObject()->className(), QString::number((int) windowState()));
//        //QMessageBox::critical(this, metaObject()->className(), QString::number((int) qobject_cast<QMdiSubWindow *>(parent())->windowState()));
//        //QMessageBox::critical(this, metaObject()->className(), QString::number((int) static_cast<QWindowStateChangeEvent *>(event)->oldState()));
//        if (!mDoNotSave) {
//            QSettings settings;
//            settings.beginGroup("Windows");
//            settings.beginGroup(metaObject()->className());
//            SaveSettings(settings);
//            settings.endGroup();
//            settings.endGroup();
//        }
//    }
//    QDialog::changeEvent(event);
//}
