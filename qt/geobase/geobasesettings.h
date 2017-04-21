#ifndef GEOBASESETTINGS_H
#define GEOBASESETTINGS_H

#include "../VProject/qfcdialog.h"

namespace Ui {
class GeobaseSettings;
}

class GeobaseSettings : public QFCDialog
{
    Q_OBJECT

public:
    explicit GeobaseSettings(QWidget *parent = 0);
    ~GeobaseSettings();

    void SetOnDblClick(int aValue);
    void SetSelectBeh(int aValue);
    void SetSelectMode(int aValue);
    void SetDrawingShowMode(int aValue);

    int OnDblClick() const;
    int GetSelectBeh() const;
    int GetSelectMode() const;
    int GetDrawingShowMode() const;
private:
    Ui::GeobaseSettings *ui;
};

#endif // GEOBASESETTINGS_H
