#ifndef WDSETTINGS_H
#define WDSETTINGS_H

#include <QWidget>

#include "def_expimp.h"

class EXP_IMP wdSettings : public QWidget
{
    Q_OBJECT
public:
    explicit wdSettings(QWidget *parent = 0);

    virtual bool DoSave();
signals:

public slots:
};

#endif // WDSETTINGS_H
