#include "escuneater.h"

#include <QDialog>
#include <QEvent>
#include <QKeyEvent>

EscUneater::EscUneater(QObject *parent) :
    QObject(parent)
{
}

bool EscUneater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress
            && qobject_cast<QDialog *> (obj->parent())) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            qobject_cast<QDialog *> (obj->parent())->reject();
            return true;
        };
    }
    return QObject::eventFilter(obj, event);
}
