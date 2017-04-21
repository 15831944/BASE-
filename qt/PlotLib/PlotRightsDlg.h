#ifndef PLOTRIGHTSDLG_H
#define PLOTRIGHTSDLG_H

#define NO_EXPIMP
#include "../VProject/qfcdialog.h"
#include <qtablewidget.h>
namespace Ui {
class PlotRightsDlg;
}

class PlotRightsDlg : public QFCDialog
{
    Q_OBJECT

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void resizeEvent(QResizeEvent * event);

public:
    explicit PlotRightsDlg(int ID, QWidget *parent = 0);
    ~PlotRightsDlg();

private:
    int mIdPlot;
    bool mJustStarted;
    QString selText;
    Ui::PlotRightsDlg *ui;
    QTableWidgetItem *lItem, *rItem;
    bool get_Restrict;
    QPalette lPaletteDefault;
    QPalette lPaletteDis;
private slots:
    void ShowData();
    void on_cbUseRights_clicked();
    void on_buttonBox_accepted();
    void on_tbPlus_clicked();
    void on_tbMinus_clicked();
    void on_tableWidget_itemSelectionChanged();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_actionDelete_triggered();
    void on_actionProperties_triggered();
    void on_tableWidget_cellDoubleClicked(int row, int column);
};

#endif // PLOTRIGHTSDLG_H
