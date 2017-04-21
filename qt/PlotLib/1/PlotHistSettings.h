#ifndef PLOTHISTSETTINGS_H
#define PLOTHISTSETTINGS_H

#include "../VProject/qfcdialog.h"

namespace Ui {
class PlotHistSettings;
}

class PlotHistSettings : public QFCDialog
{
    Q_OBJECT

public:
    explicit PlotHistSettings(QWidget *parent = 0);
    ~PlotHistSettings();

    bool MDI() const;
    void SetMDI(bool aMDI);

    bool AutoWidth() const;
    void SetAutoWidth(bool aAutoWidth);

private:
    Ui::PlotHistSettings *ui;
};

#endif // PLOTHISTSETTINGS_H
