#ifndef CONTRACTPKZ_H
#define CONTRACTPKZ_H

#include "../VProject/qfcdialog.h"
#include "contract-pkz_global.h"

#include "qcontracttree.h"

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class ContractPkz;
}

class CONTRACTPKZSHARED_EXPORT ContractPkz : public QFCDialog
{
    Q_OBJECT

public:
    explicit ContractPkz(QWidget *parent = 0);
    ~ContractPkz();

    static bool SelectFileForAttach(QString &aFileName);
    static bool AttachFile(int aIdContract, int aIdProject, const QString &aFileName);
    static bool RemoveFile(int aIdPlot, int aIdContract);
protected:
    virtual void showEvent(QShowEvent* event);
    virtual void LoadAdditionalSettings(QSettings &aSettings);
    virtual void SaveAdditionalSettings(QSettings &aSettings);

    void ShowProps(QContractTreeItem *item);
    void ShowStageProps(QContractTreeItem *thisItem, QContractTreeItem *parentItem);
    void ShowCheckProps(int aIdCheck);

    void ReportCommonOld(bool aAll);
    void ReportFull(bool aWithFeaturedPay, const QList<int> &aProjects);
    void ReportFullOld(bool aWithFeaturedPay, const QList<int> &aProjects);
    void SummaryByYear(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects);
    void SummaryByYearOld(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects);
    void ReportProjPayByYears(bool aWithFeaturedPay, const QList<int> &aProjects);
    void ReportProjPayByYearsOld(bool aWithFeaturedPay, const QList<int> &aProjects);
    void MonthlyPayments(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects);
    void MonthlyPaymentsOld(int aYear, bool aWithFeaturedPay, const QList<int> &aProjects);
    void ReportSignedNotPayed(bool aWithFeaturedPay, const QList<int> &aProjects);

    void ReportPayByCusts(bool aWithFeaturedPay, QStringList aCustomers);
    void ReportPayByCustsOld(bool aWithFeaturedPay, const QStringList &aCustomers);

    void HideEmptyChildren(QTreeWidgetItem *aParent);

    void GetSelectedProjects(QList<int> &aProjects);
private slots:
    void DoSelectColumnsTop(const QPoint &aPoint);
    void DoSelectColumnsHashbon(const QPoint &aPoint);
    void on_actionAdd_contract_triggered();

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_actionProp_contract_triggered();

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    void on_actionDel_contract_triggered();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_actionCollapse_all_triggered();

    void on_actionExpand_all_triggered();

    void on_actionExpand_projects_triggered();

    void on_tbStageProps_clicked();

    void on_tbHashbonShow_clicked();

    void on_dockWidget_2_topLevelChanged(bool topLevel);

    void on_dockHashbon_visibilityChanged(bool visible);

    void on_actionAdd_hashbon_triggered();

    void on_treeHashbon_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_actionProp_hashbon_triggered();

    void on_treeHashbon_customContextMenuRequested(const QPoint &pos);

    void on_actionAdd_stage_triggered();

    void on_actionProp_stage_triggered();

    void on_actionDel_stage_triggered();

    void on_tbSettings_clicked();

    void on_treeHashbon_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_actionDel_hashbon_triggered();

    void on_tbReports_clicked();

    void on_actionReport_Common_triggered();

    void on_actionSelect_all_triggered();

    void on_actionSelect_none_triggered();

    void on_actionInvert_selection_triggered();

    void on_actionReport_common_selected_triggered();

    void on_actionReport_ProjByYear_triggered();

    void on_actionPayments_by_customer_triggered();

    void on_actionCntrView_file_triggered();
    void on_actionCntrAttach_file_triggered();
    void on_actionCntrRemove_file_triggered();

    void on_cbHideEmpty_clicked();
    void on_actionFull_selected_triggered();

    void on_actionFull_all_triggered();

    void on_actionSigned_not_payed_triggered();

private:
    bool mJustStarted;
    Ui::ContractPkz *ui;

    void CollapseChilds(QTreeWidgetItem *aItem);
};

#endif // CONTRACTPKZ_H
