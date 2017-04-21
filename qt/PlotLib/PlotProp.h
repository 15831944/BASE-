#ifndef PLOTPROP_H
#define PLOTPROP_H

#include "../VProject/qfcdialog.h"
#include "PlotData.h"
#include "../VProject/TreeData.h"

namespace Ui {
    class PlotProp;
}

class PlotProp : public QFCDialog
{
    Q_OBJECT
protected:
    PlotData * mPlotData;
    //PlotHistoryData * mPlotHistoryData;
    int mNumHistData;
    TreeDataRecord * mTreeData;

    QString mVerPrev, mStagePrev, mComplectPrev;
    bool mSheetSetted;

    void RegenCodePart(PlotData::PlotPropWithCode aPP, const QString &aOldVal, const QString &aNewVal);
public:
    explicit PlotProp(PlotData * aPlotData, PlotHistoryData * aPlotHistoryData, QWidget *parent = 0);
    virtual ~PlotProp();
protected:
    virtual void showEvent(QShowEvent* event);
    void ShowData();
    bool SaveData();
    void ShowComments();
private slots:
    void RegenCodeList();
    void on_leIdProject_editingFinished();

    void on_tbProjSel_clicked();

    void on_tbTreeSel_clicked();

    void on_pbOK_clicked();

    void on_twComments_customContextMenuRequested(const QPoint &pos);

    void on_cbSent_toggled(bool checked);

    void on_cbCancelled_toggled(bool checked);

    void on_cbDeleted_toggled(bool checked);

    void on_leVersionExt_textEdited(const QString &arg1);

    void on_cbStage_currentTextChanged(const QString &arg1);

    void on_cbSection_editTextChanged(const QString &arg1);

    void on_leCode_customContextMenuRequested(const QPoint &pos);

    void on_leVersionExt_editingFinished();

    void on_cbCode_customContextMenuRequested(const QPoint &pos);

//    void on_cbStage_editTextChanged(const QString &arg1);

private:
    bool mJustStarted;
    Ui::PlotProp *ui;
};

#endif // PLOTPROP_H
