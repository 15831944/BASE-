#include "FindDlg.h"
#include "ui_FindDlg.h"

#include "common.h"

#include "../UsersDlg/UserData.h"
#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectListDlg.h"

FindDlg::FindDlg(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::FindDlg),
    qSelect(NULL), qSelectPrepared(false),
    mJustStarted(true), mIdProject(0)
{
    ui->setupUi(this);

    ui->leIdProject->setValidator(new QIntValidator(1, 1e9, this));
    ui->leId->setValidator(new QIntValidator(1, 1e9, this));

    ui->leStartWith->setValidator(new QIntValidator(1, 1e9, this));
    ui->leShowCount->setValidator(new QIntValidator(10, 10000, this));

    ui->twDocs->InitColumns(PLTWorking | PLTProject | PLTNoEditMenu | PLTFindMode);

    ui->cbCommentUser->addItem(tr("Any user"));
    ui->cbEditUser->addItem(tr("Any user"));
    for (int i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled()) {
            ui->cbCommentUser->addItem(gUsers->UsersConst().at(i)->NameConst(), gUsers->UsersConst().at(i)->LoginConst());
            ui->cbEditUser->addItem(gUsers->UsersConst().at(i)->NameConst(), gUsers->UsersConst().at(i)->LoginConst());
        }

    Clear();

    qSelect = new QSqlQuery(db);

//    ui->twDocs->header()->setContextMenuPolicy(Qt::CustomContextMenu);
//    connect(ui->twDocs->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(DoSelectColumns(const QPoint &)));
}

FindDlg::~FindDlg()
{
    delete ui;
    delete qSelect;
}

void FindDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        if (ReadVersion < CurrentVersion) {
            ui->twDocs->HideColumn(ui->twDocs->colIdCommon);
            ui->twDocs->HideColumn(ui->twDocs->colHistOrigin);
            ui->twDocs->HideColumn(ui->twDocs->colCreated);
            ui->twDocs->HideColumn(ui->twDocs->colCreatedBy);
        }

    }
}

bool FindDlg::ShowData(int aStart, int aCount) {

    if (!qSelectPrepared) return false;

    for (int i = 0; i < aCount; i++) {
        if (!i) {
            if (qSelect->seek(aStart)) {
                ui->twDocs->clear();
            } else {
                return false; // that is mean - no such number
            }
        } else {
            if (!qSelect->next()) break;
        }

        PlotData *lPlotData = new PlotData(qSelect->value("id").toInt(), qSelect->value("id_project").toInt(), qSelect->value("id_common").toInt(),
                                           qSelect->value("type_area").toInt(), qSelect->value("type").toInt(),
                                           qSelect->value("id_dwg").toInt(), qSelect->value("dwg_version").toInt(),
                                           qSelect->value("working").toInt(),
                                           qSelect->value("cancelled").toInt(), qSelect->value("cancdate").toDate(), qSelect->value("cancuser").toString(),
                                           qSelect->value("deleted").toInt(), qSelect->value("deldate").toDate(), qSelect->value("deluser").toString(),
                                           qSelect->value("version").toString(), qSelect->value("version_ext").toString(),
                                           qSelect->value("sentdate").toDate(), qSelect->value("sentuser").toString(),
                                           qSelect->value("section").toString(), qSelect->value("stage").toString(),
                                           qSelect->value("code").toString(), qSelect->value("sheet_number").toString(),
                                           qSelect->value("extension").toString(),
                                           qSelect->value("nametop").toString(), qSelect->value("name").toString(),
                                           qSelect->value("block_name").toString(),
                                           qSelect->value("crdate").toDateTime(), qSelect->value("cruser").toString(),
                                           qSelect->value("edit_date").toDateTime(), qSelect->value("edit_user").toString(),
                                           qSelect->value("data_length").toLongLong(),
                                           qSelect->value("xrefs_cnt").toInt(),
                                           qSelect->value("edit_na").toInt(), qSelect->value("load_na").toInt(), qSelect->value("editprop_na").toInt(),
                                           qSelect->value("delete_na").toInt(), qSelect->value("view_na").toInt(), qSelect->value("newver_na").toInt(),
                                           qSelect->value("comments").toString());
        PlotListTreeItem * lItem = new PlotListTreeItem(ui->twDocs, lPlotData, NULL, qSelect->value("dwg_version_max").toInt());
        lItem->SetIsOwner(true);
        lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable/* | Qt::ItemIsDragEnabled*/));
        ui->twDocs->addTopLevelItem(lItem);
    }

    if (gSettings->DocumentTree.AutoWidth)
        for (int i = 0; i < ui->twDocs->columnCount(); i++)
            ui->twDocs->resizeColumnToContents(i);
    return true;
}

void FindDlg::Clear() {
    ui->leStartWith->setText("1");
    ui->leShowCount->setText("1000");

    ui->leIdProject->setText("");
    ui->leProjName->setText("");

    ui->leId->setText("");
    ui->leComplect->setText("");
    ui->leBlockName->setText("");
    ui->leCode->setText("");
    ui->leTopName->setText("");
    ui->leBottomName->setText("");

    ui->leNote->setText("");
    ui->cbCommentUser->setCurrentIndex(0);
    ui->leComment->setText("");

    ui->cbEditUser->setCurrentIndex(0);
    ui->cbEdited->setChecked(false);
    ui->deStart->setDate(QDate::currentDate());
    ui->deEnd->setDate(QDate::currentDate());
    on_cbEdited_toggled(false);

    ui->cbWithoutHistory->setChecked(true);
    ui->cbOnlyWorking->setChecked(false);
}


void FindDlg::on_pbFind_clicked() {
    QStringList lWheres;
    QList<QVariant> lParams;
    QString lWhere2;
    bool lFindEdited = false;

    qSelectPrepared = false;

    if (!ui->leIdProject->text().isEmpty()) {
        lWheres.append("id_project = ?");
        lParams.append(ui->leIdProject->text().toInt());
    }

    if (!ui->leId->text().isEmpty()) {
        lWheres.append("id = ?");
        lParams.append(ui->leId->text().toInt());
    }

    if (!ui->leComplect->text().isEmpty()) {
        lWheres.append("upper(section) like '%' || upper(?) || '%'");
        lParams.append(ui->leComplect->text());
    }

    if (!ui->leBlockName->text().isEmpty()) {
        lWheres.append("upper(block_name) like '%' || upper(?) || '%'");
        lParams.append(ui->leBlockName->text());
    }

    if (!ui->leCode->text().isEmpty()) {
        lWheres.append("upper(code) like '%' || upper(?) || '%'");
        lParams.append(ui->leCode->text());
    }

    if (!ui->leTopName->text().isEmpty()) {
        lWheres.append("upper(nametop) like '%' || upper(?) || '%'");
        lParams.append(ui->leTopName->text());
    }

    if (!ui->leBottomName->text().isEmpty()) {
        lWheres.append("upper(name) like '%' || upper(?) || '%'");
        lParams.append(ui->leBottomName->text());
    }

    if (!ui->leNote->text().isEmpty()) {
        lWheres.append("upper(comments) like '%' || upper(?) || '%'");
        lParams.append(ui->leNote->text());
    }

    if (ui->cbCommentUser->currentIndex()) {
        // comment user specofied
        if (ui->leComment->text().isEmpty()) {
            lWheres.append("exists (select 1 from v_plot_comments where id_plot = z.id and insert_user = ?)");
            lParams.append(ui->cbCommentUser->currentData().toString());
        } else {
            // text also specified
            lWheres.append("exists (select 1 from v_plot_comments where id_plot = z.id and insert_user = ? and upper(comments) like '%' || upper(?) || '%')");
            lParams.append(ui->cbCommentUser->currentData().toString());
            lParams.append(ui->leComment->text());
        }
    } else {
        if (!ui->leComment->text().isEmpty()) {
            // only comment text specified
            lWheres.append("exists (select 1 from v_plot_comments where id_plot = z.id and upper(comments) like '%' || upper(?) || '%')");
            lParams.append(ui->leComment->text());
        }
    }

    if (ui->cbEditUser->currentIndex()) {
        lWheres.append("edit_user = ?");
        lParams.append(ui->cbEditUser->currentData().toString());
    }

    if (ui->cbEdited->isChecked()) {
        if (ui->cbWithoutHistory->isChecked()) {
            // only last dwg
            lWheres.append("trunc(edit_date) >= ? and trunc(edit_date) <= ?");
            lParams.append(ui->deStart->date());
            lParams.append(ui->deEnd->date());
        } else {
            // all dwgs
            lFindEdited = true;
            lWhere2 =
                    " and (exists (select 1 from dwg_edit where (trunc(starttime) >= trunc(?) and trunc(starttime) <= trunc(?)"
                        " or trunc(endtime) >= trunc(?) and trunc(endtime) <= trunc(?)"
                        " or trunc(starttime) <= trunc(?) and trunc(endtime) >= trunc(?))"
                      " and id_dwgout = b.id)"
                    " or exists (select 1 from dwg_file where inout = 0 and  trunc(\"WHEN\") >= trunc(?) and trunc(\"WHEN\") <= trunc(?) and id_dwg = b.id))";
            lParams.append(ui->deStart->date());
            lParams.append(ui->deEnd->date());
            lParams.append(ui->deStart->date());
            lParams.append(ui->deEnd->date());
            lParams.append(ui->deStart->date());
            lParams.append(ui->deEnd->date());
            lParams.append(ui->deStart->date());
            lParams.append(ui->deEnd->date());
        }
    }

    if (!lParams.isEmpty()) {
        if (ui->cbOnlyWorking->isChecked()) {
            lWheres.append("working = 1");
        }
        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colWorking], ui->cbOnlyWorking->isChecked());

        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colIdProject], !ui->leIdProject->text().isEmpty());
        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colProject], !ui->leIdProject->text().isEmpty());

        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colIdHist], ui->cbWithoutHistory->isChecked());
        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colHist], ui->cbWithoutHistory->isChecked());

        QString lWhereFull;

        lWhereFull = " where id_project > 0";

        foreach (const QString & lStr1, lWheres) {
            lWhereFull += " and " + lStr1;
        }
        //QMessageBox::information(this, tr("About"), lWhereFull);

        if (!lFindEdited) {
            qSelect->prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id id_dwg, b.version dwg_version, 0 dwg_version_max,"
                         " a.working,"
                         " a.cancelled, a.cancdate, a.cancuser,"
                         " a.deleted, a.deldate, a.deluser,"
                         " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.stage, a.code, a.sheet_number, a.extension,"
                         " a.nametop, a.name, a.block_name,"
                         " a.crdate, a.cruser, a.edit_date, a.edit_user,"
                         " dbms_lob.getlength(b.data) data_length,"
                         " (select count(1) from v_xref2dwg where id_dwg_main = b.id) xrefs_cnt,"
                         " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
                         " a.comments"
                         " from (select * from v_plot_simple z " + lWhereFull + ") a"
                         " left outer join v_dwg b on b.id_plot = a.id"
                         " where (b.version = (select max(version) from v_dwg where id_plot = a.id) or b.version is null)"
                         " order by a.edit_date desc");
        } else {
            qSelect->prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id id_dwg, b.version dwg_version, c.version dwg_version_max,"
                         " a.working,"
                         " a.cancelled, a.cancdate, a.cancuser,"
                         " a.deleted, a.deldate, a.deluser,"
                         " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.stage, a.code, a.sheet_number, a.extension,"
                         " a.nametop, a.name, a.block_name,"
                         " a.crdate, a.cruser, a.edit_date, a.edit_user,"
                         " dbms_lob.getlength(b.data) data_length,"
                         " (select count(1) from v_xref2dwg where id_dwg_main = b.id) xrefs_cnt,"
                         " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
                         " a.comments"
                         " from (select * from v_plot_simple z " + lWhereFull + ") a, v_dwg b, v_dwg c"
                         " where b.id_plot = a.id and c.id_plot = a.id and c.version = (select max(version) from v_dwg where id_plot = a.id)"
                         + lWhere2 +
                         " order by a.edit_date desc, b.version desc");
        }

        if (qSelect->lastError().isValid()) {
            gLogger->ShowSqlError(this, tr("Finding documents"), *qSelect);
        } else {
            foreach (QVariant lValue, lParams) qSelect->addBindValue(lValue);
            if (!qSelect->exec()) {
                gLogger->ShowSqlError(this, tr("Finding documents"), *qSelect);
            } else {
                qSelectPrepared = true;

                ShowData(ui->leStartWith->text().toInt() - 1, ui->leShowCount->text().toInt());
            }
        }

    }
}

//void FindDlg::on_pbFind_clicked() {
//    QStringList lWheres;
//    QString lWhere2;
//    QList<QVariant> lParams;

//    qSelectPrepared = false;

//    if (!ui->leIdProject->text().isEmpty()) {
//        lWheres.append("id_project = ?");
//        lParams.append(ui->leIdProject->text().toInt());
//    }

//    if (!ui->leId->text().isEmpty()) {
//        lWheres.append("id = ?");
//        lParams.append(ui->leId->text().toInt());
//    }

//    if (!ui->leComplect->text().isEmpty()) {
//        lWheres.append("upper(section) like '%' || upper(?) || '%'");
//        lParams.append(ui->leComplect->text());
//    }

//    if (!ui->leBlockName->text().isEmpty()) {
//        lWheres.append("upper(block_name) like '%' || upper(?) || '%'");
//        lParams.append(ui->leBlockName->text());
//    }

//    if (!ui->leCode->text().isEmpty()) {
//        lWheres.append("upper(code) like '%' || upper(?) || '%'");
//        lParams.append(ui->leCode->text());
//    }

//    if (!ui->leTopName->text().isEmpty()) {
//        lWheres.append("upper(nametop) like '%' || upper(?) || '%'");
//        lParams.append(ui->leTopName->text());
//    }

//    if (!ui->leBottomName->text().isEmpty()) {
//        lWheres.append("upper(name) like '%' || upper(?) || '%'");
//        lParams.append(ui->leBottomName->text());
//    }

//    if (!ui->leNote->text().isEmpty()) {
//        lWheres.append("upper(comments) like '%' || upper(?) || '%'");
//        lParams.append(ui->leNote->text());
//    }

//    if (ui->cbCommentUser->currentIndex()) {
//        // comment user specofied
//        if (ui->leComment->text().isEmpty()) {
//            lWheres.append("exists (select 1 from v_plot_comments where id_plot = z.id and insert_user = ?)");
//            lParams.append(ui->cbCommentUser->currentData().toString());
//        } else {
//            // text also specified
//            lWheres.append("exists (select 1 from v_plot_comments where id_plot = z.id and insert_user = ? and upper(comments) like '%' || upper(?) || '%')");
//            lParams.append(ui->cbCommentUser->currentData().toString());
//            lParams.append(ui->leComment->text());
//        }
//    } else {
//        if (!ui->leComment->text().isEmpty()) {
//            // only comment text specified
//            lWheres.append("exists (select 1 from v_plot_comments where id_plot = z.id and upper(comments) like '%' || upper(?) || '%')");
//            lParams.append(ui->leComment->text());
//        }
//    }

//    if (ui->cbEditUser->currentIndex()) {
//        lWheres.append("edit_user = ?");
//        lParams.append(ui->cbEditUser->currentData().toString());
//    }

//    lWhere2 =  " where (b.version = (select max(version) from v_dwg where id_plot = a.id) or b.version is null)";
//    if (ui->cbEdited->isChecked()) {
//        if (ui->cbWithoutHistory->isChecked()) {
//            // only last dwg
//            lWheres.append("trunc(edit_date) >= ? and trunc(edit_date) <= ?");
//            lParams.append(ui->deStart->date());
//            lParams.append(ui->deEnd->date());
//        } else {
//            // all dwgs
//            lWhere2 =
//                    " where (exists (select 1 from dwg_edit where (trunc(starttime) >= trunc(?) and trunc(starttime) <= trunc(?)"
//                        " or trunc(endtime) >= trunc(?) and trunc(endtime) <= trunc(?)"
//                        " or trunc(starttime) <= trunc(?) and trunc(endtime) >= trunc(?))"
//                      " and id_dwgout = b.id)"
//                    " or exists (select 1 from dwg_file where inout = 0 and  trunc(\"WHEN\") >= trunc(?) and trunc(\"WHEN\") <= trunc(?) and id_dwg = b.id))";
//            lParams.append(ui->deStart->date());
//            lParams.append(ui->deEnd->date());
//            lParams.append(ui->deStart->date());
//            lParams.append(ui->deEnd->date());
//            lParams.append(ui->deStart->date());
//            lParams.append(ui->deEnd->date());
//            lParams.append(ui->deStart->date());
//            lParams.append(ui->deEnd->date());
//        }
//    }

//    if (!lParams.isEmpty()) {
//        if (ui->cbOnlyWorking->isChecked()) {
//            lWheres.append("working = 1");
//        }
//        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colWorking], ui->cbOnlyWorking->isChecked());

//        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colIdProject], !ui->leIdProject->text().isEmpty());
//        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colProject], !ui->leIdProject->text().isEmpty());

//        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colIdHist], ui->cbWithoutHistory->isChecked());
//        ui->twDocs->setColumnHidden(ui->twDocs->Cols()[PlotListTree::colHist], ui->cbWithoutHistory->isChecked());

//        QString lWhereFull;

//        lWhereFull = " where id_project > 0";

//        foreach (const QString & lStr1, lWheres) {
//            lWhereFull += " and " + lStr1;
//        }
//        //QMessageBox::information(this, tr("About"), lWhereFull);

//        qSelect->prepare("select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id id_dwg, b.version dwg_version,"
//                         " a.working,"
//                         " a.cancelled, a.cancdate, a.cancuser,"
//                         " a.deleted, a.deldate, a.deluser,"
//                         " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.code, a.sheet_number, a.extension,"
//                         " a.nametop, a.name, a.block_name,"
//                         " a.crdate, a.cruser, a.edit_date, a.edit_user,"
//                         " dbms_lob.getlength(b.data) data_length,"
//                         " (select count(1) from v_xref2dwg where id_dwg_main = b.id) xrefs_cnt,"
//                         " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
//                         " a.comments"
//                         " from (select * from v_plot_simple z " + lWhereFull + ") a"
//                         " left outer join v_dwg b on b.id_plot = a.id"
//                         + lWhere2 +
//                         " order by a.edit_date desc");

//        if (qSelect->lastError().isValid()) {
//            gLogger->ShowSqlError(this, tr("Finding documents"), qSelect->lastError().text());
//        } else {
//            foreach (QVariant lValue, lParams)
//            qSelect->addBindValue(lValue);
//            if (!qSelect->exec()) {
//                gLogger->ShowSqlError(this, tr("Finding documents"), qSelect->lastError().text());
//            } else {
//                qSelectPrepared = true;

//                ShowData(ui->leStartWith->text().toInt() - 1, ui->leShowCount->text().toInt());
//            }
//        }

//    }
//}

void FindDlg::on_cbEdited_toggled(bool checked) {
    ui->deStart->setEnabled(checked);
    ui->deEnd->setEnabled(checked);
    ui->cbWithoutHistory->setEnabled(checked);
}

void FindDlg::on_pbPrev_clicked() {
    if (ui->leStartWith->text().toInt() > ui->leShowCount->text().toInt()) {
        ui->leStartWith->setText(QString::number(ui->leStartWith->text().toInt() - ui->leShowCount->text().toInt()));
    } else {
        ui->leStartWith->setText("1");
    }

    ShowData(ui->leStartWith->text().toInt() - 1, ui->leShowCount->text().toInt());
}

void FindDlg::on_pbNext_clicked(bool checked) {
    if (ShowData(ui->leStartWith->text().toInt() - 1 + ui->leShowCount->text().toInt(), ui->leShowCount->text().toInt())) {
        ui->leStartWith->setText(QString::number(ui->leShowCount->text().toInt() + ui->leStartWith->text().toInt()));
    }
}

void FindDlg::on_pbClear_clicked() {
    Clear();
}

void FindDlg::on_leIdProject_editingFinished() {
    if (ui->leIdProject->text().isEmpty()) {
        mIdProject = 0;
        ui->leProjName->setText("");
        return;
    }

    ProjectData * lProject = gProjects->FindByIdProject(ui->leIdProject->text().toInt());
    if (lProject) {
        ui->leProjName->setText(lProject->FullShortName());
        mIdProject = lProject->Id();
    } else {
        if (mIdProject) {
            lProject = gProjects->FindByIdProject(mIdProject);
        }
        if (lProject) {
            ui->leIdProject->setText(QString::number(lProject->Id()));
            ui->leProjName->setText(lProject->FullShortName());
        } else {
            mIdProject = 0;
            ui->leIdProject->setText("");
            ui->leProjName->setText("");
        }

    }
}

void FindDlg::on_toolButton_clicked() {
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    if (!ui->leIdProject->text().isEmpty()) dSel.SetSelectedProject(ui->leIdProject->text().toInt());

    if (dSel.exec() == QDialog::Accepted) {
        ProjectData * lProject = dSel.GetProjectData();
        if (lProject) {
            ui->leIdProject->setText(QString::number(lProject->Id()));
            ui->leProjName->setText(lProject->FullShortName());
        } else {
            ui->leIdProject->setText("");
            ui->leProjName->setText("");
        }
    }
}
