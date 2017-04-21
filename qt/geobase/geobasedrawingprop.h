#ifndef GEOBASEDRAWINGPROP_H
#define GEOBASEDRAWINGPROP_H

#include "../VProject/qfcdialog.h"

namespace Ui {
class GeobaseDrawingProp;
}

class GeobaseDrawingProp : public QFCDialog
{
    Q_OBJECT

public:
    explicit GeobaseDrawingProp(QWidget *parent = 0);
    ~GeobaseDrawingProp();

    void SetGeobase2PlotId(int aIdGeobase2Plot)  { mIdGeobase2Plot = aIdGeobase2Plot; };
    int GetUpdateId() { return UpdateId; };
protected:
    virtual void showEvent(QShowEvent* event);
private:
    Ui::GeobaseDrawingProp *ui;

    int mIdGeobase2Plot;
    int UpdateId;

    int origType;
    QString origFilename, origComments;

private slots:
    void Accept();
};

#endif // GEOBASEDRAWINGPROP_H
