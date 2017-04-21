#ifndef AUDITREPORT_H
#define AUDITREPORT_H

#include "qfcdialog.h"

namespace Ui {
class AuditReport;
}

class AuditReport : public QFCDialog
{
    Q_OBJECT
    
public:
    explicit AuditReport(QWidget *parent = 0);
    virtual ~AuditReport();
    
protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    void on_actionPlotView_triggered();

    void on_mGrid_customContextMenuRequested(const QPoint &pos);

    void on_mAuditList_currentIndexChanged(int index);

private:
    Ui::AuditReport *ui;
};

#endif // AUDITREPORT_H
