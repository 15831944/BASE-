#ifndef GEOBASE_H
#define GEOBASE_H

#include "../VProject/qfcdialog.h"
#include "geobase_global.h"

#include "qgeobasetree.h"

#include <QTreeWidget>

namespace Ui {
class Geobase;
}

class GEOBASESHARED_EXPORT Geobase : public QFCDialog
{
    Q_OBJECT
    
public:
    explicit Geobase(QWidget *parent = 0);
    ~Geobase();
    
private:
    Ui::Geobase *ui;

    bool mJustStarted;
    bool CanUpdateGeobase, CanDeleteGeobase;
    bool CanInsertGeobasePlot, CanUpdateGeobasePlot, CanDeleteGeobasePlot;

    bool DeleteGeobase(int aIdGeobase);

protected:
    virtual void showEvent(QShowEvent* event);

    void ShowProps(QGeobaseTreeItem *item);
    void ShowPropsForDrawing(QGeobaseTreeItem *item, int column);
public slots:
    void ShowData();
private slots:
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_treeWidget_customContextMenuRequested(const QPoint &pos);
    void on_actionAdd_geobase_triggered();
    void on_actionProp_geobase_triggered();
    void on_actionLoad_files_triggered();
    void on_tbPlus_2_clicked();
    void on_actionDel_geobase_triggered();
    void on_tbReload_clicked();
    void on_actionPlot_prop_triggered();
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_toolButton_clicked();
    void on_actionDel_files_triggered();
    void on_actionRecalc_coords_triggered();
    void on_actionView_triggered();
};

#endif // GEOBASE_H
