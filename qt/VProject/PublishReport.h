#ifndef PUBLISHREPORT_H
#define PUBLISHREPORT_H

#include "qfcdialog.h"

void DoPublishReport(QWidget *aParent, int aId);

namespace Ui {
class PublishReport;
}

class PublishReport : public QFCDialog
{
    Q_OBJECT

public:
    explicit PublishReport(int aId, QWidget *parent = 0);
    ~PublishReport();

protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    void ShowData();
    void on_mGrid_customContextMenuRequested(const QPoint &pos);

    void on_actionView_file_triggered();

    void on_mGrid_doubleClicked(const QModelIndex &index);

    void on_actionSelect_directory_triggered();

    void on_mPublishList_currentIndexChanged(int index);

    void on_pbMakeXls_clicked();

    void on_actionView_triggered();

    void on_actionView_w_o_xrefs_last_hist_triggered();

    void on_actionProperties_triggered();

    void on_actionGo_to_documents_triggered();

private:
    bool mJustStarted;
    int mId;
    QString mDirName;

    bool DoSelectDirectory();
    void DoView(bool aWithoutXrefs);

    Ui::PublishReport *ui;
};

#endif // PUBLISHREPORT_H
