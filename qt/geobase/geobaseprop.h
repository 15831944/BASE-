#ifndef GEOBASEPROP_H
#define GEOBASEPROP_H

#include "../VProject/qfcdialog.h"

#include <QDateTime>

namespace Ui {
class GeobaseProp;
}

class GeobaseProp : public QFCDialog
{
    Q_OBJECT
    
public:
    explicit GeobaseProp(QWidget *parent = 0);
    ~GeobaseProp();

    void SetGeobaseId(int aIdGeobase) { mIdGeobase = aIdGeobase; }
    int GeobaseId() { return mIdGeobase; }

    void SetProjectIdForNew(int aIdProject) { newProjectId = aIdProject; }
    void SetOrderNumForNew(QString aOrderNum) { origOrderNum = aOrderNum; }

protected:
    virtual void showEvent(QShowEvent* event);

private slots:
    void on_toolButton_clicked();
    void IdProjectChanged();
    void Accept();

private:
    Ui::GeobaseProp *ui;

    int mIdGeobase;
    int origCustomerId;

    QString origOrderNum;

    int origProjectId, newProjectId;
    QDate origDateRcv, origDateExp;

    bool mIsNewRecord;
};

#endif // GEOBASEPROP_H
