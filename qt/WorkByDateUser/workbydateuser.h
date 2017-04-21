#ifndef WORKBYDATEUSER_H
#define WORKBYDATEUSER_H

#include "../Login/Login.h"
#include "../VProject/qfcdialog.h"
#include "workbydateuser_global.h"

#include <QDate>
#include <QTreeWidgetItem>

namespace Ui {
class WorkByDateUser;
}

class WorkByDateUserItem;

class WORKBYDATEUSERSHARED_EXPORT WorkByDateUser : public QFCDialog
{
    friend WorkByDateUserItem;
    Q_OBJECT

protected:
    int mColIdProj, mColProjName, mColIdPlot, mColIdCommon, mColDeleted, mColWorking, mColHistIn, mColHistOut, mColHistMax;
    int mColElapsed, mColVerInt, mColVerExt, mColCode, mColSheet, mColNameTop, mColName, mColChangeDate;
    int mColStartTime, mColEndTime, mColSaveCount, mColUser, mColFilename;

    // report parameters
    bool mParamSetted;
    qlonglong mIdProject;
    bool mWithConstr;
    QString mLogin;
    QDate mStartDate, mEndDate;
    int mGrouping;

    void InitInConstructor();
    void FillItemInternal(const QSqlQuery &qSelect, WorkByDateUserItem *aItem, int aMode,
                          bool &aHasSheet, bool &aHasDeleted, bool &aHasNonWorking, bool &aHasFilename);
public:
    explicit WorkByDateUser(QWidget *parent = 0);
    explicit WorkByDateUser(QSettings &aSettings, QWidget *parent = 0);
    virtual ~WorkByDateUser();

    virtual void SaveState(QSettings &aSettings);
private slots:
    void on_cbShow_clicked();
    void DoSelectColumns(const QPoint &aPoint);

    void on_twList_customContextMenuRequested(const QPoint &pos);

    void on_twList_itemSelectionChanged();

    void on_toolButton_clicked();

    void on_leIdProject_editingFinished();

private:
    Ui::WorkByDateUser *ui;
};

class WorkByDateUserItem : public QTreeWidgetItem {
private:
    WorkByDateUser *mWorkByDateUser;

    bool LessByCodeSheet(const QTreeWidgetItem & other, int aSortCol) const;
public:
    void SetMainWidget(WorkByDateUser * aWorkByDateUser);
    virtual bool operator<(const QTreeWidgetItem & other) const;
};

#endif // WORKBYDATEUSER_H
