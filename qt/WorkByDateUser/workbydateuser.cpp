#include "workbydateuser.h"
#include "ui_workbydateuser.h"


#include "../UsersDlg/UserData.h"
#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectListDlg.h"
#include "../PlotLib/PlotData.h"
#include "../PlotLib/DwgCmpSettingsDlg.h"
#include "../VProject/PlotListItemDelegate.h"
#include "../VProject/SelectColumnsDlg.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/MainWindow.h"
#include "../Logger/logger.h"

#include <QFileDialog>
#include <QMenu>

WorkByDateUser::WorkByDateUser(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::WorkByDateUser),
    mColIdProj(-1), mColProjName(-1), mColIdPlot(-1), mColIdCommon(-1), mColDeleted(-1), mColWorking(-1),
    mColHistIn(-1), mColHistOut(-1), mColHistMax(-1),
    mColElapsed(-1), mColVerInt(-1), mColVerExt(-1), mColCode(-1), mColSheet(-1), mColNameTop(-1), mColName(-1), mColChangeDate(-1),
    mColStartTime(-1), mColEndTime(-1), mColSaveCount(-1), mColUser(-1), mColFilename(-1),
    mParamSetted(false)
{
    InitInConstructor();

    ui->twList->setColumnHidden(3, true);
    ui->twList->setColumnHidden(11, true);
    ui->twList->setColumnHidden(17, true);
    ui->twList->setColumnHidden(18, true);
    ui->twList->setColumnHidden(19, true);
}

WorkByDateUser::WorkByDateUser(QSettings &aSettings, QWidget *parent)  :
    QFCDialog(parent, true),
    ui(new Ui::WorkByDateUser),
    mColIdProj(-1), mColProjName(-1), mColIdPlot(-1), mColIdCommon(-1), mColDeleted(-1), mColWorking(-1),
    mColHistIn(-1), mColHistOut(-1), mColHistMax(-1),
    mColElapsed(-1), mColVerInt(-1), mColVerExt(-1), mColCode(-1), mColSheet(-1), mColNameTop(-1), mColName(-1), mColChangeDate(-1),
    mColStartTime(-1), mColEndTime(-1), mColSaveCount(-1), mColUser(-1), mColFilename(-1)
{
    if (mParamSetted = aSettings.value("ParamSetted").toBool()) {
        mIdProject = aSettings.value("IdProject").toLongLong();
        mWithConstr = aSettings.value("WithConstr").toBool();
        mLogin = aSettings.value("Login").toString();
        mStartDate = aSettings.value("StartDate").toDate();
        mEndDate = aSettings.value("EndDate").toDate();
        mGrouping = aSettings.value("Grouping").toInt();
    }

    InitInConstructor();

    if (mParamSetted) {
        ProjectData *lProject = gProjects->FindByIdProject(mIdProject);
        if (lProject) {
            ui->leIdProject->setText(QString::number(lProject->Id()));
            ui->leProjName->setText(lProject->FullShortName());
        }
        ui->cbWithConstr->setChecked(mWithConstr);
        for (int i =0; i < ui->cbUser->count(); i++)
            if (ui->cbUser->itemData(i).toString() == mLogin) {
                ui->cbUser->setCurrentIndex(i);
                break;
            }
        ui->deStart->setDate(mStartDate);
        ui->deEnd->setDate(mEndDate);

        ui->cbGrouping->setCurrentIndex(mGrouping);
    } else {
        ui->cbWithConstr->setChecked(true);
    }
}

WorkByDateUser::~WorkByDateUser() {
    delete ui;
}

void WorkByDateUser::InitInConstructor() {
    ui->setupUi(this);

    ui->cbUser->addItem(tr("<Everyone>"));
    for (int i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled())
            ui->cbUser->addItem(gUsers->UsersConst().at(i)->NameConst(), gUsers->UsersConst().at(i)->LoginConst());

    ui->deStart->setDate(QDate::currentDate().addDays(-1));
    ui->deEnd->setDate(QDate::currentDate().addDays(-1));

    ui->deStart->setDateRange(QDate::fromString("01.01.2000", "dd.MM.yyyy"), QDate::currentDate());
    ui->deEnd->setDateRange(QDate::fromString("01.01.2000", "dd.MM.yyyy"), QDate::currentDate());

    ROPlotListItemDelegate *d = new ROPlotListItemDelegate(this);
    ui->twList->setItemDelegate(d);

    ui->lbMSLabel->setVisible(false);
    ui->lbMS->setVisible(false);

    ui->twList->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->twList->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumns(const QPoint &)));
}

void WorkByDateUser::SaveState(QSettings &aSettings) {
    aSettings.setValue("ParamSetted", mParamSetted);
    if (mParamSetted) {
        aSettings.setValue("IdProject", mIdProject);
        aSettings.setValue("WithConstr", mWithConstr);
        aSettings.setValue("Login", mLogin);
        aSettings.setValue("StartDate", mStartDate);
        aSettings.setValue("EndDate", mEndDate);
        aSettings.setValue("Grouping", mGrouping);
    }
}

// aMode = 0, 1, 2 - main item for Grouping
// 11 - subItem for Grouping = 1 (by id_plot)
// 12 - subItem for Grouping = 2 (by id_common)
void WorkByDateUser::FillItemInternal(const QSqlQuery &qSelect, WorkByDateUserItem *aItem, int aMode,
                      bool &aHasSheet, bool &aHasDeleted, bool &aHasNonWorking, bool &aHasFilename) {

    int lCol = 0;

    aItem->setFlags((aItem->flags() | Qt::ItemIsEditable) & ~Qt::ItemIsUserCheckable);
    aItem->SetMainWidget(this);

    ProjectData *lProject = NULL;

    if (aMode < 11) {
        lProject = gProjects->FindByIdProject(qSelect.value("id_project").toInt());
    }

    if (lProject)
        aItem->setText(lCol, QString::number(lProject->Id()));
    aItem->setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColIdProj = lCol;
    lCol++;

    if (lProject)
        aItem->setText(lCol, lProject->FullShortName());
    aItem->setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    mColProjName = lCol;
    lCol++;

    if (aMode != 11)
        aItem->setText(lCol, qSelect.value("id_plot").toString());
    aItem->setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColIdPlot = lCol;
    lCol++;

    if (aMode < 11)
        aItem->setText(lCol, qSelect.value("id_common").toString());
    aItem->setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColIdCommon = lCol;
    lCol++;

    if (aMode != 11) {
        aItem->setCheckState(lCol, (qSelect.value("deleted").toInt() == 1)?Qt::Checked:Qt::Unchecked);
        if (qSelect.value("deleted").toInt() == 1) aHasDeleted = true;
    }
    mColDeleted = lCol;
    lCol++;

    if (aMode != 11) {
        aItem->setCheckState(lCol, (qSelect.value("working").toInt() == 1)?Qt::Checked:Qt::Unchecked);
        if (qSelect.value("working").toInt() != 1) aHasNonWorking = true;
    }
    mColWorking = lCol;
    lCol++;

    if (qSelect.value("id_plot_from").toInt() == qSelect.value("id_plot").toInt()) {
        aItem->setText(lCol, qSelect.value("dwg_version_from").toString());
    } else {
        if (qSelect.value("id_plot_from").toInt() > 0)
            aItem->setText(lCol, qSelect.value("id_plot_from").toString() +"/" + qSelect.value("dwg_version_from").toString());
    }
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColHistIn = lCol;
    lCol++;

    aItem->setText(lCol, qSelect.value("dwg_version").toString());
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    if (qSelect.value("dwg_version").toInt() != qSelect.value("maxversion").toInt()) {
        aItem->setBackgroundColor(lCol, QColor(0x77, 0x77, 0x77));
   }
    mColHistOut = lCol;
    lCol++;

    if (aMode != 11)
        aItem->setText(lCol, qSelect.value("maxversion").toString());
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColHistMax = lCol;
    lCol++;

    if (!qSelect.value("elapsed").isNull()) {
        int lElapsed = qSelect.value("elapsed").toInt();

        QString lHours, lMinutes, lSeconds;
        lHours = QString::number(floor(lElapsed / 3600));
        if (lHours.length() < 2) lHours.prepend('0');

        lMinutes = QString::number(floor((lElapsed % 3600) / 60));
        if (lMinutes.length() < 2) lMinutes.prepend('0');

        lSeconds = QString::number(lElapsed % 60);
        if (lSeconds.length() < 2) lSeconds.prepend('0');

        //aItem->setText(lCol, QString::number(lElapsed) + " - " + lHours + ":" + lMinutes + ":" + lSeconds);
        aItem->setText(lCol, lHours + ":" + lMinutes + ":" + lSeconds);
    }
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColElapsed = lCol;
    lCol++;

    if (aMode != 11)
        aItem->setText(lCol, qSelect.value("plot_version").toString());
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColVerInt = lCol;
    lCol++;

    if (aMode != 11)
        aItem->setText(lCol, qSelect.value("plot_version_ext").toString());
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColVerExt = lCol;
    lCol++;


    if (aMode != 11)
        aItem->setText(lCol, qSelect.value("code").toString());
    aItem->setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    mColCode = lCol;
    lCol++;

    if (aMode != 11) {
        if (!qSelect.value("sheet_number").isNull()) aHasSheet = true;
        aItem->setText(lCol, qSelect.value("sheet_number").toString());
    }
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColSheet = lCol;
    lCol++;

    if (aMode != 11)
        aItem->setText(lCol, qSelect.value("nametop").toString());
    aItem->setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    mColNameTop = lCol;
    lCol++;

    if (aMode != 11)
        aItem->setText(lCol, qSelect.value("name").toString());
    aItem->setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    mColName = lCol;
    lCol++;

    if (aMode != 1 && aMode != 2)
        aItem->setText(lCol, qSelect.value("change_date_str").toString());
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColChangeDate = lCol;
    lCol++;

    if (aMode != 1 && aMode != 2)
        aItem->setText(lCol, qSelect.value("starttime").toDateTime().toString("dd.MM.yyyy HH:mm"));
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColStartTime = lCol;
    lCol++;

    if (aMode != 1 && aMode != 2)
        aItem->setText(lCol, qSelect.value("endtime").toDateTime().toString("dd.MM.yyyy HH:mm"));
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColEndTime = lCol;
    lCol++;

    if (qSelect.value("savecount").toInt() > 0)
        aItem->setText(lCol, qSelect.value("savecount").toString());
    aItem->setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColSaveCount = lCol;
    lCol++;

    if (aMode != 1 && aMode != 2)
        aItem->setText(lCol, gUsers->GetName(qSelect.value("username").toString()));
    aItem->setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    mColUser = lCol;
    lCol++;

    if (aMode != 1 && aMode != 2)
        if (!qSelect.value("filename").isNull()) aHasFilename = true;
    aItem->setText(lCol, gUsers->GetName(qSelect.value("filename").toString()));
    aItem->setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    mColFilename = lCol;
    lCol++;

    if (aMode < 10) {
        QFont lFont = aItem->font(0);
        lFont.setBold(true);
        for (int i = 0; i < aItem->columnCount(); i++)
            aItem->setFont(i, lFont);
    }
}

void WorkByDateUser::on_cbShow_clicked() {
    QSqlQuery qSelect(db), qSelect2(db);

    if (ui->deStart->date() > ui->deEnd->date()) {
        QMessageBox::critical(this, tr("Work report"), tr("Start date must be less than or equal to end date!"));
        return;
    }

    ui->leSelected->setText("");
    ui->leAll->setText("");

    mParamSetted = true;
    mIdProject = ui->leIdProject->text().toULongLong();
    mWithConstr = ui->cbWithConstr->isChecked();
    mLogin = ui->cbUser->currentData().toString();
    mStartDate = ui->deStart->date();
    mEndDate = ui->deEnd->date();
    mGrouping = ui->cbGrouping->currentIndex();

    ui->twList->clear();

    switch (mGrouping) {
    case 0:
        qSelect.prepare("select id_project, id_plot, id_common, to_char(change_date, 'dd.mm.yyyy hh24:mi') change_date_str, starttime, endtime,"
                        " savecount, username, elapsed, deleted, working, plot_version, plot_version_ext,"
                        " code, sheet_number, nametop, name, id_plot_from, dwg_version_from, id_dwg, dwg_version, maxversion, filename, computer"
                        " from v_plot_search_new a"
                        " where trunc(change_date) between to_date('" + mStartDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                                                                                                            "  and to_date('" + mEndDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                        + (mLogin.isEmpty()?"":" and username = :username")
                        + (!mIdProject?"":(QString(" and (id_project = :id_project")
                                                                 + QString(mWithConstr?" or exists (select id from v_project where id = a.id_project and id_project = :id_project))":")")))
                        + " order by change_date");
        break;
    case 1:
        qSelect.prepare("select id_project, id_plot, id_common,"
                        " sum(elapsed) elapsed, deleted, working, plot_version, plot_version_ext,"
                        " code, sheet_number, nametop, name,"
                        " min(id_plot_from) keep (dense_rank first order by change_date) id_plot_from,"
                        " min(dwg_version_from) keep (dense_rank first order by change_date) dwg_version_from,"
                        " /*id_dwg, */max(dwg_version) dwg_version, maxversion/*, filename, computer*/"
                        " from v_plot_search_new a"
                        " where trunc(change_date) between to_date('" + mStartDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                                                                                                            "  and to_date('" + mEndDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                        + (mLogin.isEmpty()?"":" and username = :username")
                        + (!mIdProject?"":(QString(" and (id_project = :id_project")
                                                                 + QString(mWithConstr?" or exists (select id from v_project where id = a.id_project and id_project = :id_project))":")")))
                        + " group by id_project, id_plot, id_common, deleted, working, plot_version, plot_version_ext, maxversion, code, sheet_number, nametop, name"
                        + " order by 4 desc");

        // it is a copy from case 0 with added param :id_plot
        qSelect2.prepare("select id_project, id_plot, id_common, to_char(change_date, 'dd.mm.yyyy hh24:mi') change_date_str, starttime, endtime,"
                         " savecount, username, elapsed, deleted, working, plot_version, plot_version_ext,"
                         " code, sheet_number, nametop, name, id_plot_from, dwg_version_from, id_dwg, dwg_version, maxversion, filename, computer"
                         " from v_plot_search_new a"
                         " where trunc(change_date) between to_date('" + mStartDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                                                                                                             "  and to_date('" + mEndDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                         + (mLogin.isEmpty()?"":" and username = :username")
                         + (!mIdProject?"":(QString(" and (id_project = :id_project")
                                                                  + QString(mWithConstr?" or exists (select id from v_project where id = a.id_project and id_project = :id_project))":")")))
                         + " and id_plot = :id_plot"
                         + " order by change_date");
        break;
    case 2:
        // max() and min() is formal in dense_rank queries, meaning has "first or "last" before "order by"
        qSelect.prepare("select id_project,"
                        " max(id_plot) keep (dense_rank last order by change_date) id_plot,"
                        " id_common,"
                        " sum(elapsed) elapsed,"
                        " max(deleted) keep (dense_rank last order by change_date) deleted,"
                        " max(working) keep (dense_rank last order by change_date) working,"
                        " max(plot_version) keep (dense_rank last order by change_date) plot_version,"
                        " max(plot_version_ext) keep (dense_rank last order by change_date) plot_version_ext,"
                        " max(code) keep (dense_rank last order by change_date) code,"
                        " max(sheet_number) keep (dense_rank last order by change_date) sheet_number,"
                        " max(nametop) keep (dense_rank last order by change_date) nametop,"
                        " max(name) keep (dense_rank last order by change_date) name,"
                        " min(id_plot_from) keep (dense_rank first order by change_date) id_plot_from,"
                        " min(dwg_version_from) keep (dense_rank first order by change_date) dwg_version_from,"
                        " /*max(dwg_version) dwg_version,*/"
                        " min(dwg_version) keep (dense_rank last order by change_date) dwg_version,"
                        " max(maxversion) keep (dense_rank last order by change_date) maxversion"
                        " from v_plot_search_new a"
                        " where trunc(change_date) between to_date('" + mStartDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                                                                                                            "  and to_date('" + mEndDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                        + (mLogin.isEmpty()?"":" and username = :username")
                        + (!mIdProject?"":(QString(" and (id_project = :id_project")
                                                                 + QString(mWithConstr?" or exists (select id from v_project where id = a.id_project and id_project = :id_project))":")")))
                        + " group by id_project, id_common"
                        + " order by 4 desc");

        // it is a copy from case 0 with added param :id_plot
        qSelect2.prepare("select id_project, id_plot, id_common, to_char(change_date, 'dd.mm.yyyy hh24:mi') change_date_str, starttime, endtime,"
                         " savecount, username, elapsed, deleted, working, plot_version, plot_version_ext,"
                         " code, sheet_number, nametop, name, id_plot_from, dwg_version_from, id_dwg, dwg_version, maxversion, filename, computer"
                         " from v_plot_search_new a"
                         " where trunc(change_date) between to_date('" + mStartDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                                                                                                             "  and to_date('" + mEndDate.toString("dd.MM.yyyy") + "', 'dd.mm.yyyy')"
                         + (mLogin.isEmpty()?"":" and username = :username")
                         + (!mIdProject?"":(QString(" and (id_project = :id_project")
                                                                  + QString(mWithConstr?" or exists (select id from v_project where id = a.id_project and id_project = :id_project))":")")))
                         + " and id_common = :id_common"
                         + " order by change_date");
        break;
    }

    if (qSelect.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Work report"), qSelect);
        return;
    }

    if (qSelect2.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Work report"), qSelect2);
        return;
    }

    qulonglong lAllElapsed =0;

    if (!mLogin.isEmpty()) {
        qSelect.bindValue(":username", mLogin);
    }
    if (mIdProject) {
        qSelect.bindValue(":id_project", mIdProject);
    }

    if (!qSelect.exec()) {
        gLogger->ShowSqlError(this, tr("Work report"), qSelect);
    } else {
        qint64 lStartMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();

        ui->twList->sortByColumn(-1);

        bool lHasSheet = false, lHasDeleted = false, lHasNonWorking = false,
                lHasFilename = false;

        ui->lbMSLabel->setVisible(false);
        ui->lbMS->setVisible(true);
        ui->lbMS->setText(tr("Working..."));
        QCoreApplication::processEvents();

        while (qSelect.next()) {
            WorkByDateUserItem *lItem = new WorkByDateUserItem();
            FillItemInternal(qSelect, lItem, mGrouping, lHasSheet, lHasDeleted, lHasNonWorking, lHasFilename);
            ui->twList->addTopLevelItem(lItem);
            lItem->setData(0, Qt::UserRole, qSelect.value("elapsed").toULongLong());

            lAllElapsed += qSelect.value("elapsed").toULongLong();

            if (mGrouping) {
                if (mGrouping == 1) {
                    qSelect2.bindValue(":id_plot", qSelect.value("id_plot").toInt());
                } else if (mGrouping == 2) {
                    qSelect2.bindValue(":id_common", qSelect.value("id_common").toInt());
                }
                if (!mLogin.isEmpty()) {
                    qSelect2.bindValue(":username", mLogin);
                }
                if (mIdProject) {
                    qSelect2.bindValue(":id_project", mIdProject);
                }

                if (!qSelect2.exec()) {
                    gLogger->ShowSqlError(this, "Work report", qSelect2);
                    break;
                } else {
                    while (qSelect2.next()) {
                        WorkByDateUserItem *lItem2 = new WorkByDateUserItem();
                        FillItemInternal(qSelect2, lItem2, 10 + mGrouping, lHasSheet, lHasDeleted, lHasNonWorking, lHasFilename);
                        lItem->addChild(lItem2);
                        lItem2->setData(0, Qt::UserRole, qSelect2.value("elapsed").toULongLong());
                    }
                }
            }
        }

        ui->lbMSLabel->setVisible(true);
        ui->lbMS->setText(QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStartMSecsSinceEpoch)) / 1000));

        // can't draw "+" when zeroth column is hidden
        //if (mColIdProj != -1) ui->twList->setColumnHidden(mColIdProj, mIdProject && !mWithConstr);
        if (mColProjName != -1) ui->twList->setColumnHidden(mColProjName, mIdProject && !mWithConstr);

        if (mColDeleted != -1) ui->twList->setColumnHidden(mColDeleted, !lHasDeleted);
        if (mColWorking != -1) ui->twList->setColumnHidden(mColWorking, !lHasNonWorking);
        if (mColSheet != -1) ui->twList->setColumnHidden(mColSheet, !lHasSheet);
        if (mColUser != -1) ui->twList->setColumnHidden(mColUser, !mLogin.isEmpty());
        if (mColFilename != -1) ui->twList->setColumnHidden(mColFilename, !lHasFilename);

        for (int i = 0; i < ui->twList->columnCount(); i++)
            ui->twList->resizeColumnToContents(i);

        QString lHours, lMinutes, lSeconds;
        lHours = QString::number(floor(lAllElapsed / 3600));
        if (lHours.length() < 2) lHours.prepend('0');

        lMinutes = QString::number(floor((lAllElapsed % 3600) / 60));
        if (lMinutes.length() < 2) lMinutes.prepend('0');

        lSeconds = QString::number(lAllElapsed % 60);
        if (lSeconds.length() < 2) lSeconds.prepend('0');

        ui->leAll->setText(lHours + ":" + lMinutes + ":" + lSeconds);
    }
}

void WorkByDateUser::DoSelectColumns(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    QList<int> lDis;
    lDis << 2 << 4 << 5;

    w.SetHeaderView(ui->twList->header());
    w.SetDisabledIndexes(lDis);

    if (w.exec() == QDialog::Accepted) {
        for (int i = 0; i < ui->twList->columnCount(); i++)
            ui->twList->resizeColumnToContents(i);
    }
}

void WorkByDateUser::on_twList_customContextMenuRequested(const QPoint &pos) {
    QMutexLocker lLocker(gMainWindow->UpdateMutex());
    QMenu lMenu(this);

    QAction *lARes, *lAExpandAll = NULL, *lACollapseAll = NULL,
            *lAView = NULL, *lAViewNoXrefs = NULL, *lAGotoDocument = NULL, *lAHistory = NULL,
            *lACompareVisual = NULL, *lACompareVer2 = NULL, *lACompareToPdf = NULL,
            *lACopyToClipboard = NULL;

    PlotData *lPlotNew /*for "history" and "compare"*/, *lPlotOld /*for "compare"*/, *lPlot/*for "vew" and "goto"*/;
    PlotHistoryData *lHistoryOld, *lHistoryNew, *lHistory/*for "vew" and "goto"*/;
    QString lExt;

    QList <QTreeWidgetItem *> lSelected = ui->twList->selectedItems();;

    if (mGrouping
            && pos.x() < ui->twList->indentation()) {
        lAExpandAll = lMenu.addAction(tr("Expand all"));
        lACollapseAll = lMenu.addAction(tr("Collapse all"));
    } else {
        if (lSelected.isEmpty()) return;

        int lHistNumOld = 0, lHistNumNew = 0 /*both for "compare"*/;

        if (lSelected.length() == 1) {
//            QMessageBox::critical(NULL, "", QString::number(ui->twList->currentColumn()) + ": "
//                                  + QString::number(mColHistIn) + " - "
//                                  + QString::number(mColHistOut) + " - "
//                                  + QString::number(mColHistMax) + " - ");
            if (ui->twList->currentColumn() == mColHistIn && !lSelected.at(0)->text(mColHistIn).isEmpty()
                    || ui->twList->currentColumn() == mColHistOut
                    || ui->twList->currentColumn() == mColHistMax) {

                QString lText = lSelected.at(0)->text(ui->twList->currentColumn());

                QTreeWidgetItem *lItem;
                if (lSelected.at(0)->parent()
                        && lSelected.at(0)->text(mColIdPlot).isEmpty()) {
                    lItem = lSelected.at(0)->parent();
                } else {
                    lItem = lSelected.at(0);
                }

                int lIdPlot, lHistNum = 0;

                if (lText.contains('/')) {
                    lIdPlot = lText.left(lText.indexOf('/')).toInt();
                    lHistNum = lText.mid(lText.indexOf('/') + 1).toInt();
                } else {
                    lIdPlot = lItem->text(mColIdPlot).toInt();
                    lHistNum = lText.toInt();
                }
                if (!(lPlot = gProjects->FindByIdPlot(lIdPlot))) return;
                if (!(lHistory = lPlot->GetHistoryByNum(lHistNum))) return;

                // get extension
                lExt = lHistory->ExtConst().toLower();

                if (lHistNum == lItem->text(mColHistMax).toInt()) {
                    lHistory = NULL;
                }

                lAView = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View"));
                lAGotoDocument = lMenu.addAction(tr("Go to document"));
            }
            lAHistory = lMenu.addAction(QIcon(":/some/ico/ico/File-History.png"), tr("History"));

            //QMessageBox::critical(NULL, "", QString::number(mColIdPlot));

            // get datas (for compare and not only for it)
            int lIdPlotNew;
            if (lSelected.at(0)->parent()
                    && lSelected.at(0)->text(mColIdPlot).isEmpty()) { // or could check grouping type here
                lIdPlotNew = lSelected.at(0)->parent()->text(mColIdPlot).toInt();
                //QMessageBox::critical(NULL, "parent", lSelected.at(0)->parent()->text(mColIdPlot));
            } else {
                lIdPlotNew = lSelected.at(0)->text(mColIdPlot).toInt();
                //QMessageBox::critical(NULL, "self", lSelected.at(0)->text(mColIdPlot));
            }

            // it is used for "history" and "compare"
            if (!(lPlotNew = gProjects->FindByIdPlot(lIdPlotNew))) return;
            lHistNumNew = lSelected.at(0)->text(mColHistOut).toInt();

            QString lTextOld = lSelected.at(0)->text(mColHistIn);
            if (lTextOld.isEmpty()) {
                // loaded from file
                if (lHistNumNew > 1) {
                    lPlotOld  = lPlotNew;
                    lHistNumOld = lHistNumNew - 1;
                }
            } else if (lTextOld.contains('/')) {
                // from other document
                if (!(lPlotOld = gProjects->FindByIdPlot(lTextOld.left(lTextOld.indexOf('/')).toInt()))) return;
                lHistNumOld = lTextOld.mid(lTextOld.indexOf('/') + 1).toInt();
            } else {
                lPlotOld = lPlotNew;
                lHistNumOld = lTextOld.toInt();
            }

            //
        } else if (lSelected.length() == 2) {
            if (lSelected.at(0)->parent()
                    && lSelected.at(0)->parent() == lSelected.at(1)->parent()) {
                if (!(lPlotNew = gProjects->FindByIdPlot(lSelected.at(0)->parent()->text(mColIdPlot).toInt()))) return;

                int lMaxIndex;
                if (lSelected.at(0)->text(mColHistOut).toInt() > lSelected.at(1)->text(mColHistOut).toInt())
                    lMaxIndex = 0;
                else
                    lMaxIndex = 1;

                // it is max
                lHistNumNew = lSelected.at(lMaxIndex)->text(mColHistOut).toInt();

                // and it is - min
                QString lTextOld = lSelected.at(1 - lMaxIndex)->text(mColHistIn);
                if (lTextOld.isEmpty()) {
                    if (lHistNumNew > 1) {
                        lPlotOld  = lPlotNew;
                        lHistNumOld = lHistNumNew - 1;
                    }
                } else if (lTextOld.contains('/')) {
                    if (!(lPlotOld = gProjects->FindByIdPlot(lTextOld.left(lTextOld.indexOf('/')).toInt()))) return;
                    lHistNumOld = lTextOld.mid(lTextOld.indexOf('/') + 1).toInt();
                } else {
                    lPlotOld  = lPlotNew;
                    lHistNumOld = lTextOld.toInt();
                }
            }
        }

        if (lHistNumOld) {
            if (!(lHistoryOld = lPlotOld->GetHistoryByNum(lHistNumOld))) return;
            if (!(lHistoryNew = lPlotNew->GetHistoryByNum(lHistNumNew))) return;
            if (lHistoryOld->ExtConst().toLower() == "dwg"
                    && lHistoryNew->ExtConst().toLower() == "dwg") {
                lACompareVisual = lMenu.addAction(tr("Compare in AutoCAD (visual)"));
                //lACompareVer2 = lMenu.addAction(tr("Compare in AutoCAD (by elements)"));
            }
        }
        lACompareToPdf = lMenu.addAction(tr("Compare to PDF"));

        lMenu.addSeparator();
        lACopyToClipboard = lMenu.addAction(QIcon(":/some/ico/ico/copy.png"), tr("Copy to clipboard"));
    }

    if (!lMenu.actions().isEmpty()
            && (lARes = lMenu.exec(QCursor::pos()))) {
        if (lARes == lAExpandAll) {
            ui->twList->expandAll();
        } else if (lARes == lACollapseAll) {
            ui->twList->collapseAll();
        } else if (lARes == lAView
                   || lARes == lAViewNoXrefs) {
            if ((lPlot->FileType() < 20 || lPlot->FileType() > 29) && lExt == "dwg") {
                // AutoCAD drawing
                MainDataForCopyToAcad lDataForAcad(1, lARes == lAViewNoXrefs);
                lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lPlot->Id(), lHistory?lHistory->Id():0, 0, false));
                gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
            } else {
                // non-AutoCAD document
                if (lHistory) {
                    gSettings->DoOpenNonDwg(lHistory->Id(), 2 /*id_dwg*/, 0 /*view*/, "");
                } else {
                    gSettings->DoOpenNonDwg(lPlot->Id(), 1 /*id_plot*/, 0 /*view*/, "");
                }
            }
        } else if (lARes == lAGotoDocument) {
            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlot->IdProject()), lPlot, lHistory);
        } else if (lARes == lAHistory) {
            gMainWindow->ShowPlotHist(lPlotNew, lSelected.at(0)->text(mColHistOut).toInt(), true, false);
        } else if (lARes == lACompareVisual
                   || lARes == lACompareVer2) {
            ULONG lCmpVersion = (lARes == lACompareVisual)?1:((lARes == lACompareVer2)?2:3);
            MainDataForCopyToAcad lDataForAcad(lCmpVersion/*version*/, cmpWithXrefs, "");
            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eCMP, lHistoryOld, lHistoryNew));
            gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
        } else if (lARes == lACompareToPdf) {
            // parameters at first
            bool lParamsIsOk;
            if (!gSettings->Compare.Readed
                    || gSettings->Compare.OldColor == gSettings->Compare.NewColor
                    || gSettings->Compare.AlwaysAskAll) {
                DwgCmpSettingsDlg w(this);
                lParamsIsOk = w.exec() == QDialog::Accepted;
            } else if (gSettings->Compare.AlwaysAskOutputDir
                       || gSettings->Compare.OutputDir.isEmpty()) {
                QFileDialog dlg(this);
                dlg.setFileMode(QFileDialog::DirectoryOnly);

                dlg.setDirectory(gSettings->Compare.OutputDir);
                if (lParamsIsOk = (dlg.exec() == QDialog::Accepted)) {
                    gSettings->Compare.OutputDir = dlg.selectedFiles().at(0);
                }
            } else {
                lParamsIsOk = true;
            }

            if (!lParamsIsOk) return;

            QDir lDir(gSettings->Compare.OutputDir);

            if (!lDir.exists()) {
                if (!lDir.mkpath(gSettings->Compare.OutputDir)) {
                    QMessageBox::critical(this, tr("Work report"), tr("Can't create directory") + "\n" + gSettings->Compare.OutputDir);
                    return;
                }
            }

            int lHistNumOld, lHistNumNew;

            MainDataForCopyToAcad lDataForAcad(3/*version*/, cmpWithXrefs, gSettings->Compare.OutputDir);
            for (int i = 0; i < lSelected.length(); i++) {
                QTreeWidgetItem *lItem = lSelected.at(i);

                // finding new version
                int lIdPlotNew;
                if (lItem->parent()
                        && lItem->text(mColIdPlot).isEmpty()) {
                    lIdPlotNew = lItem->parent()->text(mColIdPlot).toInt();
                } else {
                    lIdPlotNew = lItem->text(mColIdPlot).toInt();
                }
                // it is used for "history" and "compare"
                if (!(lPlotNew = gProjects->FindByIdPlot(lIdPlotNew))) {
                    QMessageBox::critical(this, tr("Work report"), "Document not found, ID = " + lItem->text(mColIdPlot));
                    continue;
                }
                lHistNumNew = lItem->text(mColHistOut).toInt();
                // new is defined

                // finding old version
                QString lTextOld = lSelected.at(i)->text(mColHistIn);
                if (lTextOld.isEmpty()) {
                    // loaded from file
                    if (lHistNumNew > 1) {
                        lPlotOld  = lPlotNew;
                        lHistNumOld = lHistNumNew - 1;
                    } else {
                        continue;
                    }
                } else if (lTextOld.contains('/')) {
                    // from other document
                    if (!(lPlotOld = gProjects->FindByIdPlot(lTextOld.left(lTextOld.indexOf('/')).toInt()))) {
                        QMessageBox::critical(this, tr("Work report"), "Document not found, ID = " + lTextOld);
                        return;
                    }
                    lHistNumOld = lTextOld.mid(lTextOld.indexOf('/') + 1).toInt();
                } else {
                    lPlotOld = lPlotNew;
                    lHistNumOld = lTextOld.toInt();
                }
                // old is defined

                // get PlotHistoryData objects
                if (!(lHistoryOld = lPlotOld->GetHistoryByNum(lHistNumOld))) {
                    QMessageBox::critical(this, tr("Work report"), "History " + QString::number(lHistNumOld) + " for document ID = "
                                          + QString::number(lPlotOld->Id()) + " not found");
                    return;
                }
                if (!(lHistoryNew = lPlotNew->GetHistoryByNum(lHistNumNew))) {
                    QMessageBox::critical(this, tr("Work report"), "History " + QString::number(lHistNumNew) + " for document ID = "
                                          + QString::number(lPlotNew->Id()) + " not found");
                    return;
                }
                if (lHistoryOld->ExtConst().toLower() == "dwg"
                        && lHistoryNew->ExtConst().toLower() == "dwg") {
                    lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eCMP, lHistoryOld, lHistoryNew));
                }
            }
            if (!lDataForAcad.ListRef().isEmpty()) {
                gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
            }
        } else if (lARes == lACopyToClipboard) {
            gSettings->CopyToClipboard(ui->twList);
        }
    }
}

void WorkByDateUser::on_twList_itemSelectionChanged() {
    QList<QTreeWidgetItem *> lSelected = ui->twList->selectedItems();

    ui->leSelected->setText("");

    if (!lSelected.isEmpty()) {
        qulonglong lElapsed =0;
        for (int i = 0; i <lSelected.length(); i++) {
            // skip child if parent selected
            if (lSelected.at(i)->parent()
                    && lSelected.contains(lSelected.at(i)->parent())) continue;
            lElapsed += static_cast<WorkByDateUserItem *> (lSelected.at(i))->data(0, Qt::UserRole).toULongLong();
        }

        QString lHours, lMinutes, lSeconds;
        lHours = QString::number(floor(lElapsed / 3600));
        if (lHours.length() < 2) lHours.prepend('0');

        lMinutes = QString::number(floor((lElapsed % 3600) / 60));
        if (lMinutes.length() < 2) lMinutes.prepend('0');

        lSeconds = QString::number(lElapsed % 60);
        if (lSeconds.length() < 2) lSeconds.prepend('0');

        ui->leSelected->setText(lHours + ":" + lMinutes + ":" + lSeconds);
    }
}

void WorkByDateUser::on_toolButton_clicked() {
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(ui->leIdProject->text().toLong());

    if (dSel.exec() == QDialog::Accepted/* && ui->twType->ProjectConst() != dSel.GetProjectData()*/) {
        ProjectData *lProject = dSel.GetProjectData();
        ui->leIdProject->setText(QString::number(lProject->Id()));
        ui->leProjName->setText(lProject->FullShortName());
    }
}

void WorkByDateUser::on_leIdProject_editingFinished() {
    ProjectData *lProject = gProjects->FindByIdProject(ui->leIdProject->text().toLong());
    if (lProject) {
        ui->leIdProject->setText(QString::number(lProject->Id()));
        ui->leProjName->setText(lProject->FullShortName());
    } else {
        ui->leIdProject->setText("");
        ui->leProjName->setText("");
    }
}
