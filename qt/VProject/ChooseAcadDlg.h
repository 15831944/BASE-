#ifndef CHOOSEACADDLG_H
#define CHOOSEACADDLG_H

#include "qfcdialog.h"

class AcadParamData;
class RunningAcadData;
class RunningAcadList;

namespace Ui {
class ChooseAcadDlg;
}

class ChooseAcadDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit ChooseAcadDlg(RunningAcadList *aRunningAcadList, QWidget *parent);
    virtual ~ChooseAcadDlg();

    const RunningAcadData *GetRunningAcadData() const;

protected:
    void ShowList();
    virtual void showEvent(QShowEvent* event);
private:
    Ui::ChooseAcadDlg *ui;

    int mPreviousRow;
    RunningAcadList *mRunningAcadList;
    RunningAcadData *mRunningAcadData;
    QList<AcadParamData *> mParams;
    void SavePreviousRow();
    void RestoreCurrentRow();
    void ShowFullCommandLine();
private slots:
    void Accept();
    void on_lwAutocads_doubleClicked(const QModelIndex &index);
    void on_lwAutocads_currentRowChanged(int currentRow);
    void on_cbKeepAfterEdit_toggled(bool checked);
    void on_leCommandLine_textEdited(const QString &arg1);
    void on_cbNoLogo_toggled(bool checked);
    void on_cbProfile_currentIndexChanged(int index);
    void on_pbCopy_clicked();
    void on_pbRename_clicked();
    void on_pbRemove_clicked();
    void on_pbResetAll_clicked();
};

#endif // CHOOSEACADDLG_H
