#include "TaskProp.h"
#include "ui_TaskProp.h"
#include "GlobalSettings.h"
#include "oracle.h"
#include "common.h"
#include "PlotListTree.h"
#include "PlotListDlg.h"

#include "../UsersDlg/UserData.h"

#include "../ProjectLib/ProjectListDlg.h"

#include <QMenu>

TaskProp::TaskProp(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::TaskProp),
    mNewRecord(true), mIdTask(0)/*, mIdProject(0)*/
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    // ----------------------------------------------
    QSplitterHandle *handle = ui->splitter->handle(1);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    QFrame *line = new QFrame(handle);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    // ----------------------------------------------
    handle = ui->splitter_2->handle(1);
    layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    line = new QFrame(handle);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    // ----------------------------------------------
    handle = ui->splitter_3->handle(1);
    layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    line = new QFrame(handle);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    // ----------------------------------------------

    ui->leIdProject->setValidator(new QIntValidator(1, 1e9, this));
    //ui->twUsers->setItemDelegateForColumn(1, new TaskUserDelegate(ui->twUsers));

    TaskUserDelegate *tud = new TaskUserDelegate(ui->twUsers);
    ui->twUsers->setItemDelegate(tud);
    connect(tud, SIGNAL(commitData(QWidget *)), this, SLOT(UserDataChanged(QWidget *)));
}

TaskProp::~TaskProp()
{
    delete ui;
}

void TaskProp::showEvent(QShowEvent* event)
{
    QFCDialog::showEvent(event);

    if (ReadVersion < CurrentVersion) {
        ui->twUsers->setColumnWidth(0, 30);
        ui->twUsers->setColumnWidth(1, 130); // executor
        ui->twUsers->setColumnWidth(4, 130); // virified by

        ui->twDocs->setColumnWidth(0, 80); // id
        ui->twDocs->setColumnWidth(1, 60); // version
        ui->twDocs->setColumnWidth(2, 80); // sent date
        ui->twDocs->setColumnWidth(3, 100); // code
        ui->twDocs->setColumnWidth(4, 40); // sheet
        ui->twDocs->setColumnWidth(5, 180); // name - top
        ui->twDocs->setColumnWidth(6, 180); // name - bottom
        ui->twDocs->setColumnWidth(7, 80); // ch. date
        ui->twDocs->setColumnWidth(8, 100); // ch. user
    }

    ui->cbVerifyUser->setEditable(true);
    ui->cbVerifyUser->addItem("");

    for (int i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled()) {
            ui->cbVerifyUser->addItem(gUsers->UsersConst().at(i)->NameConst()); // fill verify list
            if (!mIdTask) {
                // for new task - set "from" field
                if (gUsers->UsersConst().at(i)->LoginConst().toUpper() == db.userName().toUpper()) {
                    mFromUser = gUsers->UsersConst().at(i)->LoginConst();
                    ui->leFromUser->setText(gUsers->UsersConst().at(i)->NameConst());
                }
            }
        }

    if (mIdTask) {
        // properties of existing task
        mNewRecord = false;
        if (qobject_cast<QMdiSubWindow *> (parent())) {
            setWindowTitle(tr("Task properties"));
        } else {
            setWindowTitle(tr("Task properties") + " - " + gSettings->BaseName);
        }

        // we need show this fields cos it state saved in registry
        ui->twUsers->setColumnHidden(2, false);
        ui->twUsers->setColumnHidden(3, false);
        ui->twUsers->setColumnHidden(4, false);


        QSqlQuery query(
                    "select id_project, pp.GetProjectShortName(id_project) proj_name,"
                    " pp.GetUserNameDisp(from_user) from_user, pp.GetUserNameDisp(verify_user) verify_user,"
                    " start_date, finish_date, text,"
                    " case when user = from_user then 1 else 0 end IsCreator,"
                    " case when user = nvl(verify_user, from_user) then 1 else 0 end IsVerifier"
                    " from v_task where id = " + QString::number(mIdTask), db);
        if (query.lastError().isValid()) {
            QMessageBox::critical(this, tr("Tasks"), query.lastError().text());
            QTimer::singleShot(0, this, SLOT(close()));
        } else {
            if (query.next()) {
                int lIsCreator = 0, lIsVerifier = 0;

                if (query.value("IsCreator").toInt() == 1) {
                    lIsCreator = 1;
                }
                if (query.value("IsVerifier").toInt() == 1) {
                    lIsVerifier = 4;
                }

                ui->leFromUser->setText(query.value("from_user").toString());
                ui->dteStartDate->setDate(query.value("start_date").toDate());
                if (!query.value("id_project").isNull()) {
                    ui->leIdProject->setText(query.value("id_project").toString());
                    ui->leProjName->setText(query.value("proj_name").toString());
                }
                ui->dteFinishDate->setDate(query.value("finish_date").toDate());
                ui->cbVerifyUser->setCurrentText(query.value("verify_user").toString());
                ui->pteText->setPlainText(query.value("text").toString());

                QSqlQuery query2(
                            "select id, pp.GetUserNameDisp(to_user) to_user, complete_date, verify_date,"
                            " pp.GetUserNameDisp(verified_by) verified_by,"
                            " case when user = to_user then 2 else 0 end IsExecutor" /* it is bit flag for delegation*/
                            " from v_task_user where id_task = " + QString::number(mIdTask), db);
                if (query2.lastError().isValid()) {
                    QMessageBox::critical(this, tr("Tasks"), query2.lastError().text());
                    QTimer::singleShot(0, this, SLOT(close()));
                } else {
                    int cnt = 1;
                    while (query2.next()) {
                        ui->twUsers->addTopLevelItem(new TaskUserData(cnt, query2.value("id").toInt(), query2.value("to_user").toString(),
                                                                      query2.value("complete_date").toDate(), query2.value("verify_date").toDate(),
                                                                      query2.value("verified_by").toString(),
                                                                      lIsCreator
                                                                      | (query2.value("complete_date").isNull()?query2.value("IsExecutor").toInt():0)
                                                                      | ((!query2.value("complete_date").isNull() && query2.value("verify_date").isNull())?lIsVerifier:0)));
                    }
                }


            } else {
                QMessageBox::critical(this, tr("Tasks"), tr("Task id = ") + QString::number(mIdTask) + tr("not found!"));
                QTimer::singleShot(0, this, SLOT(close()));
            }
        }
    } else {
        // new task
        mNewRecord = true;
        if (qobject_cast<QMdiSubWindow *> (parent())) {
            setWindowTitle(tr("New task"));
        } else {
            setWindowTitle(tr("New task") + " - " + gSettings->BaseName);
        }

        ui->dteStartDate->setDate(QDate::currentDate());
        ui->dteFinishDate->setDate(QDate::currentDate().addDays(1)); // tomorrow

        ui->twUsers->addTopLevelItem(new TaskUserData(1, 1));

        // this fileds can't be filled for new task
        ui->twUsers->setColumnHidden(2, true); // done
        ui->twUsers->setColumnHidden(3, true); // verified
        ui->twUsers->setColumnHidden(4, true); // verified by
    }

}

void TaskProp::Accept() {
    bool lDoAccept = false;
    int i;
    bool lIsFound;
    QString lVerifyUser;

    ui->twUsers->setCurrentItem(ui->twUsers->topLevelItem(0), 0);

    if (ui->pteText->toPlainText().isEmpty()) {
        QMessageBox::critical(this, tr("Tasks"), tr("Text must be specified!"));
        ui->pteText->setFocus();
        return;
    }

    lIsFound = false;
    for (i = 0; i < ui->twUsers->topLevelItemCount(); i++) {
        if (!ui->twUsers->topLevelItem(i)->text(1).isEmpty()) {
            lIsFound = true;
            break;
        }
    }

    if (!lIsFound) {
        QMessageBox::critical(this, tr("Tasks"), tr("At least one user must be specified!"));
        ui->twUsers->setCurrentItem(ui->twUsers->topLevelItem(0), 1);
        ui->twUsers->setFocus();
        return;
    }

    if (ui->dteFinishDate->date() < ui->dteStartDate->date()) {
        QMessageBox::critical(this, tr("Tasks"), tr("Finish date must be not earlier than start date!"));
        ui->dteFinishDate->setFocus();
        return;
    }

    if (mNewRecord) {
        if (ui->dteFinishDate->date() < QDate::currentDate()) {
            QMessageBox::critical(this, tr("Tasks"), tr("Finish date must be not earlier than today!"));
            ui->dteFinishDate->setFocus();
            return;
        }

        if (!ui->cbVerifyUser->currentText().isEmpty()) {
            UserData * lUserData = gUsers->FindByName(ui->cbVerifyUser->currentText());
            if (lUserData) {
                lVerifyUser = lUserData->LoginConst();
            } else {
                QMessageBox::critical(this, tr("Tasks"),
                                      tr("User") + " " + ui->cbVerifyUser->currentText() + " " + tr("doesn't exist") + "!");
                ui->cbVerifyUser->setFocus();
                return;
            }
        }
    } else {
    }

    if (db.transaction()) {
        bool lChanged = false;
        bool lErr = false;
        if (mNewRecord) {
            int lId;
            QSqlQuery qInsert(db);

            lChanged = true;
            if (!gOracle->GetSeqNextVal("seq_task_id", lId)) {
                lErr = true;
            } else {
                qInsert.prepare(
                            "insert into v_task (id, id_project, from_user, start_date, finish_date, text, comments, verify_user)"
                            " values (:id, :id_project, :from_user, :start_date, :finish_date, :text, :comments, :verify_user)");
                if (qInsert.lastError().isValid()) {
                    QMessageBox::critical(this, tr("Tasks"), qInsert.lastError().text());
                    lErr = true;
                } else {
                    qInsert.bindValue(":id", lId);
                    qInsert.bindValue(":id_project", ui->leIdProject->text());
                    qInsert.bindValue(":from_user", mFromUser);
                    qInsert.bindValue(":start_date", ui->dteStartDate->date());
                    qInsert.bindValue(":finish_date", ui->dteFinishDate->date());
                    qInsert.bindValue(":text", ui->pteText->toPlainText());
                    qInsert.bindValue(":comments", ui->pteComments->toPlainText());
                    qInsert.bindValue(":verify_user", lVerifyUser);
                    if (!qInsert.exec()) {
                        QMessageBox::critical(this, tr("Tasks"), qInsert.lastError().text());
                        lErr = true;
                    } else {
                        qInsert.prepare(
                                    "insert into v_task_user (id_task, to_user)"
                                    " values (:id_task, :to_user)");
                        if (qInsert.lastError().isValid()) {
                            QMessageBox::critical(this, tr("Tasks"), qInsert.lastError().text());
                            lErr = true;
                        } else {
                            qInsert.bindValue(":id_task", lId);
                            for (i = 0; i < ui->twUsers->topLevelItemCount(); i++) {
                                if (!ui->twUsers->topLevelItem(i)->text(1).isEmpty()) {
                                    UserData * lUserData = gUsers->FindByName(ui->twUsers->topLevelItem(i)->text(1));
                                    if (lUserData) {
                                        qInsert.bindValue(":to_user", lUserData->LoginConst());
                                        if (!qInsert.exec()) {
                                            QMessageBox::critical(this, tr("Tasks"), qInsert.lastError().text());
                                            lErr = true;
                                            break;
                                        }
                                    } else {
                                        QMessageBox::critical(this, tr("Tasks"),
                                                              tr("User") + " " + ui->twUsers->topLevelItem(i)->text(1) + " " + tr("doesn't exist") + "!");
                                        lErr = true;
                                        break;
                                    }
                                }
                            }
                            if (!lErr) {
                                qInsert.prepare(
                                            "insert into v_task_plot (id_task, id_plot)"
                                            " values (:id_task, :id_plot)");
                                if (qInsert.lastError().isValid()) {
                                    QMessageBox::critical(this, tr("Tasks"), qInsert.lastError().text());
                                    lErr = true;
                                } else {
                                    qInsert.bindValue(":id_task", lId);
                                    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
                                        qInsert.bindValue(":id_plot", ui->twDocs->topLevelItem(i)->text(0));
                                        if (!qInsert.exec()) {
                                            QMessageBox::critical(this, tr("Tasks"), qInsert.lastError().text());
                                            lErr = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        } else {
            // update existsing record


            // execute users
            QDate d1;
            QSqlQuery qExecutorUpdate(db);

            for (i = 0; i < ui->twUsers->topLevelItemCount(); i++) {
                TaskUserData *tud = (TaskUserData *) ui->twUsers->topLevelItem(i);
                if (tud->IsChanged()) {
                    lChanged = true;
                    //---------------------------------------------------------------------------------------------------------
                    if (tud->CompleteDateChanged()) {
                        qExecutorUpdate.prepare("update v_task_user set complete_date = ? where id = ?");
                        if (qExecutorUpdate.lastError().isValid()) {
                            QMessageBox::critical(this, tr("Tasks"), qExecutorUpdate.lastError().text());
                            lErr = true;
                            break;
                        }
                        d1 = d1.fromString(tud->text(2), "dd.MM.yyyy");
                        qExecutorUpdate.addBindValue(d1);
                        qExecutorUpdate.addBindValue(tud->Id());
                        if (!qExecutorUpdate.exec()) {
                            QMessageBox::critical(this, tr("Tasks"), qExecutorUpdate.lastError().text());
                            lErr = true;
                            break;
                        }
                    }

                    //---------------------------------------------------------------------------------------------------------
                    if (tud->VerifyDateChanged()) {
                        d1 = d1.fromString(tud->text(3), "dd.MM.yyyy");
                        if (d1.isNull()) {
                            qExecutorUpdate.prepare("update v_task_user set verify_date = null, verified_by = null where id = ?");
                        } else {
                            qExecutorUpdate.prepare("update v_task_user set verify_date = ?, verified_by = user where id = ?");
                        }

                        if (qExecutorUpdate.lastError().isValid()) {
                            QMessageBox::critical(this, tr("Tasks"), qExecutorUpdate.lastError().text());
                            lErr = true;
                            break;
                        }
                        if (!d1.isNull()) {
                            qExecutorUpdate.addBindValue(d1);
                        }

                        qExecutorUpdate.addBindValue(tud->Id());
                        if (!qExecutorUpdate.exec()) {
                            QMessageBox::critical(this, tr("Tasks"), qExecutorUpdate.lastError().text());
                            lErr = true;
                            break;
                        }
                    }
                }
            }
        }

        if (lChanged && !lErr) {
            if (!db.commit()) {
                QMessageBox::critical(this, tr("Tasks"), tr("Can't commit") + "\n" + db.lastError().text());
            } else {
                lDoAccept = true;
            }
        } else {
            db.rollback();
            if (!lChanged) lDoAccept = true;
        }

    } else {
        QMessageBox::critical(this, tr("Tasks"), db.lastError().text());
    }
    if (lDoAccept) accept();
}

void TaskProp::UserDataChanged(QWidget *editor)
{
    if (qobject_cast<QDateEdit *> (editor)) {
        ui->twUsers->currentItem()->setText(ui->twUsers->currentColumn(), qobject_cast<QDateEdit *> (editor)->date().toString("dd.MM.yyyy"));
    }
}

void TaskProp::IdProjectChanged()
{
    QSqlQuery query("select pp.GetProjectShortName(" + ui->leIdProject->text() + ") from dual");

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, tr("Tasks"), query.lastError().text());
    } else {
        if (query.next()) {
            ui->leProjName->setText(query.value(0).toString());
        }
    }
}

void TaskProp::on_twUsers_customContextMenuRequested(const QPoint &pos)
{
    QMenu popup(this);
    QAction *actAdd = NULL, *actDel = NULL, *actClear1 = NULL, *actClear2 = NULL, *actRes;

    if (ui->twUsers->currentItem()) {
        if (((TaskUserData *) ui->twUsers->currentItem())->Rights() & 1) {
            if (!ui->twUsers->currentItem()->text(0).isEmpty())
                actAdd = popup.addAction(tr("Add executor"));
            if (ui->twUsers->topLevelItemCount() > 1)
                actDel = popup.addAction(tr("Remove executor"));
        }

        if ((((TaskUserData *) ui->twUsers->currentItem())->Rights() & 2
                || ((TaskUserData *) ui->twUsers->currentItem())->Rights() & 4)
                && ui->twUsers->currentColumn() == 2) {
            actClear1 = popup.addAction(tr("Clear date 'done'"));
        }

        if (((TaskUserData *) ui->twUsers->currentItem())->Rights() & 4
                && ui->twUsers->currentColumn() == 3) {
            actClear2 = popup.addAction(tr("Clear date 'verified'"));
        }
    }

    if (!popup.actions().isEmpty()
            && (actRes = popup.exec(QCursor::pos()))) {
        if (actRes == actAdd) {
            TaskUserData *tud = new TaskUserData(ui->twUsers->topLevelItemCount() + 1, 1);
            ui->twUsers->addTopLevelItem(tud);
        } else if (actRes == actDel) {
            delete ui->twUsers->currentItem();
        } else if (actRes == actClear1) {
            ui->twUsers->currentItem()->setText(2, "");
        } else if (actRes == actClear2) {
            ui->twUsers->currentItem()->setText(3, "");
        }
    }
}

void TaskProp::on_twDocs_customContextMenuRequested(const QPoint &pos)
{
    int i, j;
    QMenu popup(this);
    bool lIsFound;
    QAction *actAdd = NULL, *actDel = NULL, *actRes;

    actAdd = popup.addAction(tr("Add"));
    actDel = popup.addAction(tr("Remove"));

    if (actRes = popup.exec(QCursor::pos())) {
        if (actRes == actAdd) {
            QList<PlotData *> lPlotList;
            PlotSelect(ui->leIdProject->text().toInt(), lPlotList);

            for (i = 0; i < lPlotList.length(); i++) {
                // check for already exist
                lIsFound = false;

                for (j = 0; j < ui->twDocs->topLevelItemCount(); j++) {
                    if (static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(j))->PlotConst()->Id() == lPlotList.at(i)->Id()) {
                        lIsFound = true;
                        break;
                    }
                }

                //if (!lIsFound) ui->twDocs->addTopLevelItem(new PlotListTreeItem(ui->twDocs, lPlotList.at(i)));
            }
        } else if (actRes == actDel) {
            //delete ui->twDocs->currentItem();
        }
    }
}

void TaskProp::on_toolButton_clicked()
{
    int mPrevProject = ui->leIdProject->text().toInt();

    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(mPrevProject);
    dSel.SetMode(2);
    if (dSel.exec() == QDialog::Accepted
            && mPrevProject != dSel.GetProjectData()->Id()) {
        ui->leIdProject->setText(QString::number(dSel.GetProjectData()->Id()));
        IdProjectChanged();
    }

}

void TaskProp::on_leIdProject_editingFinished()
{
    IdProjectChanged();
}

//-------------------------------------------------------------------------------------------------------------------------------------
TaskUserData::TaskUserData(int aNumber, int aRights) :
    QTreeWidgetItem(), mRights(aRights), mId(0)
{
    setFlags(flags() | Qt::ItemIsEditable);

    setSizeHint(1, QSize(160, 20));

    setText(0, QString::number(aNumber));
    setTextAlignment(0, Qt::AlignCenter);

}

TaskUserData::TaskUserData(int aNumber, int aId, const QString &aToUser, const QDate &aCompleteDate, const QDate &aVerifyDate,
                      const QString &aVerifyUser, int aRights) :
    QTreeWidgetItem(), mRights(aRights), mId(aId),
    origToUser(aToUser), origCompleteDate(aCompleteDate), origVerifyDate(aVerifyDate),
    origVerifyUser(aVerifyUser)
{
    setFlags(flags() | Qt::ItemIsEditable);

    setSizeHint(1, QSize(160, 20));

    setText(0, QString::number(aNumber));
    setTextAlignment(0, Qt::AlignCenter);

    setText(1, aToUser);

    setText(2, aCompleteDate.toString("dd.MM.yyyy"));
    setTextAlignment(2, Qt::AlignCenter);
    if (aRights & 2) setBackgroundColor(2, QColor(0x88, 0xff, 0x88));

    setText(3, aVerifyDate.toString("dd.MM.yyyy"));
    setTextAlignment(3, Qt::AlignCenter);
    if (aRights & 4) setBackgroundColor(3, QColor(0x88, 0xff, 0x88));
}

bool TaskUserData::IsChanged() const
{
    QDate dDone, dVerified;
    dDone = dDone.fromString(text(2), "dd.MM.yyyy");
    dVerified = dVerified.fromString(text(3), "dd.MM.yyyy");

    return (origToUser != text(1)
            || origCompleteDate != dDone
            || origVerifyDate != dVerified);

}

bool TaskUserData::CompleteDateChanged() const
{
    QDate dDone;
    dDone = dDone.fromString(text(2), "dd.MM.yyyy");

    return origCompleteDate != dDone;
}

bool TaskUserData::VerifyDateChanged() const
{
    QDate dDone;
    dDone = dDone.fromString(text(3), "dd.MM.yyyy");

    return origVerifyDate != dDone;
}

//-------------------------------------------------------------------------------------------------------------------------------------
TaskUserDelegate::TaskUserDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * TaskUserDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {

    if (qobject_cast<QTreeWidget *> (this->parent())) {
        QTreeWidget *tw = qobject_cast<QTreeWidget *> (this->parent());

        switch (index.column()) {
        case 1: // executor
            if (((TaskUserData *) tw->topLevelItem(index.row()))->Rights() & 1) {
                QComboBox *ul = new QComboBox(parent);
                ul->setEditable(true);
                ul->addItem("");
                for (int i = 0; i < gUsers->UsersConst().length(); i++)
                    if (!gUsers->UsersConst().at(i)->Disabled())
                        ul->addItem(gUsers->UsersConst().at(i)->NameConst());
                return ul;
            }
            break;
        case 2:
            if (((TaskUserData *) tw->topLevelItem(index.row()))->Rights() & 2) {
                QDateTimeEdit *de = new QDateEdit(parent);
                de->setCalendarPopup(true);
                de->setDate(QDate::currentDate());
                return de;
            }
            break;
        case 3:
            if (((TaskUserData *) tw->topLevelItem(index.row()))->Rights() & 4) {
                QDateTimeEdit *de = new QDateEdit(parent);
                de->setCalendarPopup(true);
                de->setDate(QDate::currentDate());
                return de;
            }
            break;
        case 5: // comments (for executor!)
            if (((TaskUserData *) tw->topLevelItem(index.row()))->Rights() & 1) {
                return QStyledItemDelegate::createEditor(parent, option, index);
            }
            break;
        }
    }

    return NULL;
}

void TaskUserDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    if (qobject_cast<QDateEdit *> (editor)) {
        QDate d;
        d = d.fromString(index.data().toString(), "dd.MM.yyyy");
        qobject_cast<QDateEdit *> (editor)->setDate(d);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
