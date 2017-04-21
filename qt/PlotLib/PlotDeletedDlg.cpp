#include "PlotDeletedDlg.h"
#include "ui_PlotDeletedDlg.h"

#include "../ProjectLib/ProjectData.h"

#include "../VProject/SelectColumnsDlg.h"
#include "../VProject/WaitDlg.h"

PlotDeletedDlg::PlotDeletedDlg(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotDeletedDlg),
    mJustStarted(true)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    mQuery = new QSqlQuery(db);

    ui->twDocs->InitColumns(PLTWorking | PLTProject | PLTDeleted | PLTNoEditMenu | PLTNoColors);
    ui->twDocs->setSortingEnabled(false);
}

PlotDeletedDlg::~PlotDeletedDlg()
{
    qDeleteAll(mDeletedPlot);
    delete ui;
    delete mQuery;
}

void PlotDeletedDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        if (ReadVersion < CurrentVersion) {
            ui->twDocs->HideColumn(ui->twDocs->colIdCommon);
            ui->twDocs->HideColumn(ui->twDocs->colCancelDate);
            ui->twDocs->HideColumn(ui->twDocs->colCancelUser);
            ui->twDocs->HideColumn(ui->twDocs->colSentDate);
            ui->twDocs->HideColumn(ui->twDocs->colSentBy);
            ui->twDocs->HideColumn(ui->twDocs->colCreated);
            ui->twDocs->HideColumn(ui->twDocs->colCreatedBy);
        }

        mQuery->prepare(QString() + "select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id as id_dwg, b.version as dwg_version,"
                        " a.working,"
                        " a.cancelled, a.cancdate, a.cancuser,"
                        " a.deldate, a.deluser,"
                        " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.stage, a.code, a.sheet_number, a.extension,"
                        " a.nametop, a.name, a.block_name,"
                        " a.crdate, a.cruser, a.edit_date, a.edit_user,"
                        + ((db.driverName()== "QPSQL")?" length(b.data) as data_length,":" dbms_lob.getlength(b.data) data_length,")
                        + " (select count(1) from v_xref2dwg where id_dwg_main = b.id) as xrefs_cnt,"
                        " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
                        " a.comments"
                        " from (select * from v_plot_simple where deleted = 1) a"
                        " left outer join v_dwg b on b.id_plot = a.id"
                        " where (b.version = (select max(version) from v_dwg where id_plot = a.id) or b.version is null)"
                        " order by deldate desc");

        if (mQuery->lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Deleted documents"), *mQuery);
            QTimer::singleShot(0, this, SLOT(close()));
        } else {
            if (!mQuery->exec()) {
                gLogger->ShowSqlError(QObject::tr("Deleted documents"), *mQuery);
                QTimer::singleShot(0, this, SLOT(close()));
            } else {
                QTimer::singleShot(0, this, SLOT(ShowData()));
                //ShowData();
                //if (mQuery->next()) {
                //}
            }
        }
    }
}

void PlotDeletedDlg::ShowData() {
    WaitDlg w(this);
    w.show();
    w.SetMessage(tr("Loading data..."));

    ui->twDocs->clear();

    qDeleteAll(mDeletedPlot);
    mDeletedPlot.clear();

    if (mQuery->seek(ui->sbStartWith->value() - 1)) {
        int lCur = 0, lCnt = ui->sbShowCount->value();
        do {
            PlotData *lPlotData = new PlotData(mQuery->value("id").toInt(), mQuery->value("id_project").toInt(), mQuery->value("id_common").toInt(),
                                               mQuery->value("type_area").toInt(), mQuery->value("type").toInt(),
                                               mQuery->value("id_dwg").toInt(), mQuery->value("dwg_version").toInt(),
                                               mQuery->value("working").toInt(),
                                               mQuery->value("cancelled").toInt(), mQuery->value("cancdate").toDate(), mQuery->value("cancuser").toString(),
                                               1, mQuery->value("deldate").toDate(), mQuery->value("deluser").toString(),
                                               mQuery->value("version").toString(), mQuery->value("version_ext").toString(),
                                               mQuery->value("sentdate").toDate(), mQuery->value("sentuser").toString(),
                                               mQuery->value("section").toString(), mQuery->value("stage").toString(),
                                               mQuery->value("code").toString(), mQuery->value("sheet_number").toString(),
                                               mQuery->value("extension").toString(),
                                               mQuery->value("nametop").toString(), mQuery->value("name").toString(),
                                               mQuery->value("block_name").toString(),
                                               mQuery->value("crdate").toDateTime(), mQuery->value("cruser").toString(),
                                               mQuery->value("edit_date").toDateTime(), mQuery->value("edit_user").toString(),
                                               mQuery->value("data_length").toLongLong(),
                                               mQuery->value("xrefs_cnt").toInt(),
                                               mQuery->value("edit_na").toInt(), mQuery->value("load_na").toInt(), mQuery->value("editprop_na").toInt(),
                                               mQuery->value("delete_na").toInt(), mQuery->value("view_na").toInt(), mQuery->value("newver_na").toInt(),
                                               mQuery->value("comments").toString());
            mDeletedPlot.append(lPlotData);

            PlotListTreeItem * pd = new PlotListTreeItem(ui->twDocs, lPlotData, NULL, 0);
            //pd->setFlags((pd->flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
            //pd->setFlags(pd->flags() | Qt::ItemIsUserCheckable);
            ui->twDocs->addTopLevelItem(pd);

            lCur++;
        } while (mQuery->next() && lCur < lCnt);

        for (lCur = 0; lCur < ui->twDocs->topLevelItemCount(); lCur++) {
            ui->twDocs->resizeColumnToContents(lCur);
        }
    }
    emit setFocus();
}

void PlotDeletedDlg::RequeryData() {
    ui->twDocs->clear();

    qDeleteAll(mDeletedPlot);
    mDeletedPlot.clear();

    if (!mQuery->exec()) {
        gLogger->ShowSqlError(QObject::tr("Deleted documents"), *mQuery);
    } else {
        ShowData();
    }
}

void PlotDeletedDlg::on_pbPrev_clicked() {
    if (ui->sbStartWith->value() > 1) {
        if (ui->sbStartWith->value() > ui->sbShowCount->value() + 1) {
            ui->sbStartWith->setValue(ui->sbStartWith->value() - ui->sbShowCount->value());
        } else {
            ui->sbStartWith->setValue(1);
        }
        ShowData();
    }
}

void PlotDeletedDlg::on_pbNext_clicked() {
    if (mQuery->next()) {
        ui->sbStartWith->setValue(ui->sbStartWith->value() + ui->sbShowCount->value());
        ShowData();
    }
}

void PlotDeletedDlg::on_twDocs_itemSelectionChanged() {
    if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())
            && static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()) {
        emit gProjects->PlotBecameSelected(static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotRef());
    }
}

void PlotDeletedDlg::on_sbShowCount_valueChanged(int arg1) {
    ui->sbStartWith->setSingleStep(arg1);
}
