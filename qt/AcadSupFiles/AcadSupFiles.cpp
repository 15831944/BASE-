#include "AcadSupFiles.h"
#include "ui_AcadSupFiles.h"
#include "FileLoadData.h"
#include "AcadSupFileRight.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/UserRight.h"
#include "../VProject/FileUtils.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QFileDialog>
#include <QMenu>

AcadSupFiles::AcadSupFiles(QWidget *parent) :
    QFCDialog(parent),
    ui(new Ui::AcadSupFiles),
    mType(0), mUserRight(0),
    mCurDepartment(-1),
    mSelectedIdDepartment(-1),
    mInSetDepartment(false)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->twList->setItemDelegateForColumn(0, new QEditNewOnlyDelegate(ui->twList));
    ui->twList->setItemDelegateForColumn(1, new QNoEditDelegate(this));
    ui->twList->setItemDelegateForColumn(2, new QNoEditDelegate(this));
    ui->twList->setItemDelegateForColumn(3, new QNoEditDelegate(this));

    QStyledItemDelegate *commentsDelegate = new QStyledItemDelegate(this);
    ui->twList->setItemDelegateForColumn(4, commentsDelegate);
    connect(commentsDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(CommentsChanged(QWidget *)));

    ui->twList->setItemDelegateForColumn(5, new QNoEditDelegate(this));
}

AcadSupFiles::~AcadSupFiles()
{
    delete ui;
}

void AcadSupFiles::showEvent(QShowEvent* event)
{
    ui->twList->setColumnWidth(0, 170);
    ui->twList->setColumnWidth(1, 30);
    ui->twList->setColumnWidth(2, 145);
    ui->twList->setColumnWidth(3, 55);

    QFCDialog::showEvent(event);

    if (mSelectedIdDepartment != -1) {
        move(pos() + QPoint(25, 25));
        ui->pbUserRights->setVisible(false);
    }

    // "change users rights" button
    QSqlQuery query2(
                "select admin_option from v_as_file_right"
                " where login = user"
                " and admin_option = 1", db);

    bool b = false;
    if (query2.lastError().isValid()) {
        gLogger->ShowSqlError(this, "AutoCAD support files", query2.lastError());
    } else {
        if (query2.next()) {
            b = true;
        }
    }
    // ---

    ui->pbUserRights->setEnabled(b);

    ui->cbDepartment->clear();
    // first - for wuch user can change data
    // next - all other
    // then - 'All departments' before departments with other names
    QSqlQuery query(
                "select a.id id, a.name name, 1,"
                " case when exists (select 1 from v_as_file_right where login = user and id_department = a.id)"
                " or exists (select 1 from v_as_file_right where login = user and id_department is null)"
                " then 1 else 0 end"
                " from department a"
                " where deleted = 0"
                " union select 0 id , '" + tr("All departments") + "' name, 0,"
                " case when exists (select 1 from v_as_file_right where login = user and id_department is null)"
                " then 1 else 0 end"
                " from dual"
                " order by 4 desc, 3, 2", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "AutoCAD support files", query.lastError());
    } else {
        while (query.next()) {
            if (mSelectedIdDepartment == -1 || mSelectedIdDepartment == query.value("id").toInt())
                ui->cbDepartment->addItem(query.value("name").toString(), query.value("id").toInt());
        }

        mCurDepartment = ui->cbDepartment->currentData().toInt();
    }
}

void AcadSupFiles::PopulateList()
{

    mListForDel.clear();

    ui->twList->clear();

    QSqlQuery query(
                "select id, filename, filedate, dbms_lob.getlength(data) filesize, sha256,"
                " case when computer = sys_context('userenv', 'host') and ip_addr = sys_context('userenv', 'ip_address')"
                " then fullname else null end fullname,"
                " comments"
                " from v_as_file"
                " where type = 0 and nvl(id_department, 0) = " + QString::number(ui->cbDepartment->currentData().toInt()) +
                " and nvl(deleted, 0) = 0"
                " order by filename", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "AutoCAD support files", query.lastError());
    } else {
        FileLoadData *twi;
        QList<int> lChangedIds;
        QStringList lChangedFiles;
        //bool lAnyFullName = false;

        while (query.next()) {
            twi = new FileLoadData(query.value("id").toInt(), query.value("filename").toString(),
                                   query.value("fullname").toString(), query.value("filedate").toDateTime(),
                                   query.value("filesize").toLongLong(), query.value("comments").toString(), query.value("sha256").toString());
            ui->twList->addTopLevelItem(twi);

            if (mSelectedFileName == query.value("filename").toString())
                ui->twList->setCurrentItem(twi, 0);

            // check also mUserRight because in query doesn't check, that user have rights to modify plot styles
            if (!query.value("fullname").isNull() && mUserRight) {
                //lAnyFullName = true;

                if (gFileUtils->IsFileChanged(query.value("fullname").toString(), query.value("filesize").toLongLong(), query.value("sha256").toString())) {
                    lChangedIds.append(query.value("id").toInt());
                    if (query.value("fullname").toString().right(query.value("filename").toString().length()).toUpper() == query.value("filename").toString().toUpper())
                        lChangedFiles.append(query.value("fullname").toString());
                    else
                        lChangedFiles.append(query.value("fullname").toString() + " (имя в Базе " + query.value("filename").toString() + ")");
                }
            }
        }

        //ui->twList->setColumnHidden(5, !lAnyFullName);

        if (!lChangedFiles.isEmpty()) {
            int i, j;
            QMessageBox mb(this);
            mb.setWindowTitle("AutoCAD support files");
            mb.setIcon(QMessageBox::Question);
            mb.setText("Ранее загруженные с этого компьютера файлы изменены на диске:\n\n"
                       + lChangedFiles.join('\n')
                       + "\n\nОбновить эти файлы в Базе?\n(Загрузка будет произведена только после нажатия ОК в следующем окне)");
            mb.addButton("Да", QMessageBox::YesRole);
            mb.setDefaultButton(mb.addButton("Нет", QMessageBox::NoRole));
            // 0 = yes, 1 = no
            if (!mb.exec()) {
                // rewrite to base
                for (i = 0 ; i < lChangedIds.length(); i++) {
                    for (j = 0; j < ui->twList->topLevelItemCount(); j++) {
                        twi = (FileLoadData *) ui->twList->topLevelItem(j);
                        if (twi->Id() == lChangedIds.at(i)) {
                            twi->ReloadFrom("");
                        }
                    }
                }
            }
        }
    }
}

void AcadSupFiles::Accept()
{
    bool lOk, lChanged;


//    for (i = 0; i < ui->twList->columnCount(); i++) {
//        QMessageBox::critical(this, "AutoCAD support files", QString::number(ui->twList->columnWidth(i)));
//    }

    SaveToDB(lChanged, lOk, false);

    if (!lChanged) reject();
    else if (lChanged && lOk) accept();
}

void AcadSupFiles::on_cbDepartment_currentIndexChanged(int index)
{

    if (mInSetDepartment) return;

    bool lOk, lChanged;
    int i, lNewIndex = index;

    // это ради эстэтства - когда пользователь переключил отдел, то там уже виден новый (и при этом модальный вопрос пользователю, ага)
    // здесь же возвращается предыдущий отдел
    for (i = 0; i < ui->cbDepartment->count(); i++)
        if (ui->cbDepartment->itemData(i) == mCurDepartment) {
            mInSetDepartment = true;
            ui->cbDepartment->setCurrentIndex(i);
            mInSetDepartment = false;
            break;
        }

    SaveToDB(lChanged, lOk, true);

    if (!(!lChanged || lChanged && lOk)) {
        // previously selected department already selected
        return; // was error in SaveToDB
    }

    // select new
    mInSetDepartment = true;
    ui->cbDepartment->setCurrentIndex(lNewIndex);
    mInSetDepartment = false;

    mCurDepartment = ui->cbDepartment->itemData(index).toInt();

    QSqlQuery query(
                "select admin_option from v_as_file_right"
                " where login = user"
                " and (nvl(id_department, 0) = " + QString::number(mCurDepartment) +
                "  or id_department is null)", db);

    mUserRight = 0;
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "AutoCAD support files", query.lastError());
    } else {
        if (query.next()) {
            if (query.value(0).toInt()) {
                mUserRight = 2;
            } else {
                mUserRight = 1;
            }
        } else {
            // no data = no rights
        }
    }

    if (mUserRight) {
        ui->lUserRight->setText(tr("change"));
        ui->tbPlus->setEnabled(true);
        ui->tbMinus->setEnabled(true);
    } else {
        ui->lUserRight->setText(tr("view"));
        ui->tbPlus->setEnabled(false);
        ui->tbMinus->setEnabled(false);
    }

    ui->twList->setColumnHidden(5, mUserRight == 0);

    PopulateList();
}

void AcadSupFiles::on_actionAddFiles_triggered()
{
    QFileDialog dlg(this);

    QStringList filters;
    filters << tr("Plot styles") + " (*.ctb)";

    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilters(filters);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        //QString str;
        FileLoadData *twi;

        //bool lAnyExisting = false;

        for (int i = 0; i < files.length(); i++) {
            twi = new FileLoadData(files.at(i));
            ui->twList->addTopLevelItem(twi);
        }
    }

}

void AcadSupFiles::SaveToDB(bool &aChanges, bool &aOk, bool aAskForSave) {
    int i, j;
    FileLoadData *twi, *twi2;

    aChanges = false;
    aOk = true;

    bool lAnyInsert = false, lAnyLoad = false, lAnyUpdate = false;
    QSqlQuery queryChk1(db);

    if (ui->twList->topLevelItemCount())
        ui->twList->setCurrentItem(ui->twList->topLevelItem(0), 1);


    for (i = 0; i < ui->twList->topLevelItemCount(); i++) {
        twi = (FileLoadData *) ui->twList->topLevelItem(i);

        if (!twi->Id()) {
            lAnyInsert = true;
        } else if (twi->Load()) {
            lAnyLoad = true;
        } else if (twi->FileNameShort() != twi->text(0)
                   || twi->Comments() != twi->text(4)) {
            lAnyUpdate = true;
        }
    }

    aChanges = !mListForDel.isEmpty() || lAnyInsert || lAnyLoad || lAnyUpdate;
    if (!aChanges) return;
    if (aAskForSave) {
        int res = QMessageBox::critical(this, "AutoCAD support files", "Сохранить изменения?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (res == QMessageBox::No) return;
        if (res == QMessageBox::Cancel) {
            // some kind of quiet "error"
            aChanges = true;
            aOk = false;
            return;
        }
    }

    if (!mCurDepartment) {
        // current is "all"; cheak in all other
        queryChk1.prepare("select b.id id, b.name name from v_as_file a, department b where a.id_department  = b.id and lower(filename) = :filename");
    } else {
        // current is not "all"; check in "all department"
        queryChk1.prepare("select id from v_as_file where id_department is null and lower(filename) = :filename");
    }
    if (queryChk1.lastError().isValid()) {
        gLogger->ShowSqlError(this, "AutoCAD support files", queryChk1.lastError());
        aOk = false;
        return;
    } else {
        bool lDoRetry;
        do {
            lDoRetry = false;
            for (i = 0; i < ui->twList->topLevelItemCount(); i++) {
                twi = (FileLoadData *) ui->twList->topLevelItem(i);

                // extension is type depending (field type in table, which the only value is 0 now), don't forget about it
                if (twi->text(0).right(4).toLower() != ".ctb") {
                    QMessageBox::critical(this, "AutoCAD support files", "Недопустимое расширение файла " + twi->text(0) + "!");;
                    ui->twList->setCurrentItem(twi, 0);
                    aOk = false;
                    return;
                }

                for (j = i + 1; j < ui->twList->topLevelItemCount(); j++) {
                    twi2 = (FileLoadData *) ui->twList->topLevelItem(j);
                    if (twi2->text(0).toLower() == twi->text(0).toLower()) {
                        QMessageBox::critical(this, "AutoCAD support files", "Имя файла " + twi->text(0) + " повторяется!");;
                        ui->twList->setCurrentItem(twi, 0);
                        ui->twList->setCurrentItem(twi2, 0, QItemSelectionModel::Select);
                        aOk = false;
                        return;
                    }
                }

                queryChk1.bindValue(":filename", twi->text(0).toLower());
                if (!queryChk1.exec()) {
                    gLogger->ShowSqlError(this, "AutoCAD support files", queryChk1.lastError());
                    aOk = false;
                    return;
                } else {
                    if (queryChk1.next()) {
                        QMessageBox mb(this);
                        mb.setWindowTitle("AutoCAD support files");
                        if (!mCurDepartment) {
                            mb.setText("Имя файла " + twi->text(0) + " уже используется для отделa '" + queryChk1.value("name").toString() + "'!\nЗадайте другое имя файла!");
                        } else {
                            mb.setText("Имя файла " + twi->text(0) + " уже используется для всех отделов!\nЗадайте другое имя файла!");
                        }
                        mb.setInformativeText("\nНажмите 'Посмотреть', чтобы посмотреть/изменить список, содержащий такое же имя (откроется в новом окне).\n\nНажмите 'Продолжить'', чтобы продолжить редактирование текущего списка.");
                        mb.addButton("Посмотреть", QMessageBox::AcceptRole);
                        mb.addButton("Продолжить", QMessageBox::RejectRole);
                        if (!mb.exec()) {
                            AcadSupFiles w(this);
                            if (!mCurDepartment) {
                                // "all departments" selected
                                w.SetSelectedIdDepartment(queryChk1.value("id").toInt());
                            } else {
                                w.SetSelectedIdDepartment(0); // list for "all departments"
                            }
                            w.SetSelectedFileName(twi->text(0));
                            if (w.exec() == 1) {
                                lDoRetry = true;
                                break;
                            } else {
                                ui->twList->setCurrentItem(twi, 0);
                                aOk = false;
                                return; // user doesn't change anything in window - so let him continue edit current
                            }
                        } else {
                            ui->twList->setCurrentItem(twi, 0);
                            aOk = false;
                            return; // 'continue' pressed by user
                        }
                    }
                }
            }
        } while (lDoRetry);
    }

    if (aChanges && aOk) {
        if (db.transaction()) {
            QSqlQuery qDel(db), qInsert(db), qLoad(db), qUpdate(db);

            if (!mListForDel.isEmpty()) {
                qDel.prepare("update v_as_file set deleted = 1 where id = :id");
                if (qDel.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "AutoCAD support files", qDel.lastError());
                    aOk = false;
                }
            }

            if (lAnyInsert && aOk) {
                qInsert.prepare(
                            "insert into v_as_file (type, id_department, filename, fullname, filedate, sha256, comments, data)"
                            " values (:type, :id_department, :filename, :fullname, :filedate, :sha256, :comments, :data)");
                if (qInsert.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "AutoCAD support files", qInsert.lastError());
                    aOk = false;
                }
            }

            if (lAnyLoad && aOk) {
                qLoad.prepare("update v_as_file set filename = :filename, fullname = :fullname, filedate = :filedate, sha256 = :sha256, comments = :comments, data = :data where id = :id");
                if (qLoad.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "AutoCAD support files", qLoad.lastError());
                    aOk = false;
                }
            }

            if (lAnyUpdate && aOk) {
                qUpdate.prepare("update v_as_file set filename = :filename, comments = :comments where id = :id");
                if (qUpdate.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "AutoCAD support files", qUpdate.lastError());
                    aOk = false;
                }
            }

            if (aOk) {

                for (i = 0; i < mListForDel.length(); i++) {
                    qDel.bindValue(":id", mListForDel.at(i));
                    if (!qDel.exec()) {
                        gLogger->ShowSqlError(this, "AutoCAD support files", qDel.lastError());
                        aOk = false;
                        break;
                    }
                }

                if (aOk) {
                    qInsert.bindValue(":type", 0); // for future use
                    if (mCurDepartment) {
                        qInsert.bindValue(":id_department", mCurDepartment);
                    } else {
                        qInsert.bindValue(":id_department", QVariant());
                    }

                    for (i = 0; i < ui->twList->topLevelItemCount(); i++) {
                        twi = (FileLoadData *) ui->twList->topLevelItem(i);
                        if (!twi->Id()) {
                            // insert record
                            QFile data(twi->FileNameFull());
                            if (data.open(QFile::ReadOnly)) {
                                QByteArray buffer(data.readAll());
                                data.close();

                                qInsert.bindValue(":filename", twi->text(0));
                                qInsert.bindValue(":fullname", twi->FileNameFull());
                                qInsert.bindValue(":filedate", twi->FileDate());
                                qInsert.bindValue(":sha256", twi->Sha256());
                                qInsert.bindValue(":comments", twi->text(4));
                                qInsert.bindValue(":data", buffer);
                                if (!qInsert.exec()) {
                                    ui->twList->setCurrentItem(twi, 0);
                                    gLogger->ShowSqlError(this, "AutoCAD support files", qInsert.lastError());
                                    aOk = false;
                                    break;
                                }
                            } else {
                                ui->twList->setCurrentItem(twi, 0);
                                gLogger->ShowError(this, "AutoCAD support files",
                                                  tr("Error opening file") + ":\n" + twi->FileNameFull() + "\n" + tr("Error") +": " + data.errorString());
                                aOk = false;
                                break;
                            }
                        } else if (twi->Load()) {
                            // load file
                            QFile data(twi->FileNameFull());
                            if (data.open(QFile::ReadOnly)) {
                                QByteArray buffer(data.readAll());
                                data.close();
                                qLoad.bindValue(":id", twi->Id());
                                qLoad.bindValue(":filename", twi->text(0));
                                qLoad.bindValue(":fullname", twi->FileNameFull());
                                qLoad.bindValue(":filedate", twi->FileDate());
                                qLoad.bindValue(":sha256", twi->Sha256());
                                qLoad.bindValue(":comments", twi->text(4));
                                qLoad.bindValue(":data", buffer);
                                if (!qLoad.exec()) {
                                    ui->twList->setCurrentItem(twi, 0);
                                    gLogger->ShowSqlError(this, "AutoCAD support files", qLoad.lastError());
                                    aOk = false;
                                    break;
                                }
                            } else {
                                ui->twList->setCurrentItem(twi, 0);
                                gLogger->ShowError(this, "AutoCAD support files",
                                                  tr("Error opening file") + ":\n" + twi->FileNameFull() + "\n" + tr("Error") +": " + data.errorString());
                                aOk = false;
                                break;
                            }
                        } else if (twi->FileNameShort() != twi->text(0)
                                   || twi->Comments() != twi->text(4)) {
                            // update record
                            qUpdate.bindValue(":id", twi->Id());
                            qUpdate.bindValue(":filename", twi->text(0));
                            qUpdate.bindValue(":comments", twi->text(4));
                            if (!qUpdate.exec()) {
                                ui->twList->setCurrentItem(twi, 0);
                                gLogger->ShowSqlError(this, "AutoCAD support files", qUpdate.lastError());
                                aOk = false;
                                break;
                            }
                        }
                    }
                }
            }
            if (aOk) {
                if (!db.commit()) {
                    gLogger->ShowSqlError(this, "AutoCAD support files", tr("Can't commit"), db.lastError());
                    aOk = false;
                }
            } else {
                db.rollback();
            }
        } else {
            gLogger->ShowSqlError(this, "AutoCAD support files", tr("Can't start transaction"), db.lastError());
            aOk = false;
        }
    }
}

void AcadSupFiles::on_pbUserRights_clicked()
{
    AcadSupFileRight asfr(this);

    asfr.exec();
}

QNoEditDelegate::QNoEditDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * QNoEditDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    return NULL;
}

QEditNewOnlyDelegate::QEditNewOnlyDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * QEditNewOnlyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    if (qobject_cast<QTreeWidget *> (this->parent())) {
        QTreeWidget *tw = qobject_cast<QTreeWidget *> (this->parent());

        if (!((FileLoadData *) tw->topLevelItem(index.row()))->Id()) {
            return QStyledItemDelegate::createEditor(parent, option, index);
        }
    }

    return NULL;
}

void AcadSupFiles::on_twList_customContextMenuRequested(const QPoint &pos)
{
    int i;
    QList<QTreeWidgetItem *> selected = ui->twList->selectedItems();
    FileLoadData *twi, *twi2;
    QMenu popup(this);
    QAction *actReload, *actLoad, *actSave, *actRes;

    if (mUserRight) {
        bool lAnyReload = false;
        for (i = 0; i < selected.count(); i++) {
            twi = (FileLoadData *) selected.at(i);
            if (!twi->FileNameFull().isEmpty()
                    && gFileUtils->IsFileChanged(twi->FileNameFull(), twi->FileSize(), twi->Sha256())) {
                lAnyReload = true;
            }
        }

        if (lAnyReload) actReload = popup.addAction("Загрузить из того же файла");
        //popup.addAction("Загрузить...", this, SLOT(MenuLoad()));
        actLoad = popup.addAction("Загрузить...");
        if (!selected.isEmpty()) popup.addSeparator();
    }

    if (!selected.isEmpty()) actSave = popup.addAction("Сохранить...");

    if (actRes = popup.exec(QCursor::pos())) {
        if (actRes == actReload) {
        } else if (actRes == actLoad) {
            if (selected.length() == 1) {
                // load insteed
                QFileDialog dlg(this);
                QStringList filters;
                filters << tr("Plot styles") + " (*.ctb)"
                        //<< "Any files (*)"
                           ;
                dlg.setAcceptMode(QFileDialog::AcceptOpen);
                dlg.setFileMode(QFileDialog::ExistingFile);
                dlg.setNameFilters(filters);
                if (dlg.exec() == QDialog::Accepted) {
                    QString lName = dlg.selectedFiles().at(0);
                    twi = (FileLoadData *) selected.at(0);
                    if (twi->FileNameShort().toUpper() == lName.right(twi->FileNameShort().length()).toUpper()
                            || QMessageBox::question(this, "AutoCAD support files",
                                                     "Загрузить файл \n" + lName + "\nвместо файла\n" + twi->FileNameShort() + "?") == QMessageBox::Yes) {
                        QFile file(lName);

                        if (file.open(QFile::ReadOnly)) {
                            QCryptographicHash hash1(QCryptographicHash::Sha256);
                            hash1.addData(&file);
                            file.close();

                            QString Sha256New = QString(hash1.result().toHex()).toUpper();
                            bool DoReload = true;

                            for (i = 0; i < ui->twList->topLevelItemCount(); i++) {
                                if ((twi2 = (FileLoadData *) ui->twList->topLevelItem(i))/* != twi*/) {
                                    if (twi2->Sha256().toUpper() == Sha256New) {
                                        if (QMessageBox::question(this, "AutoCAD support files",
                                                                  "Загружаемый файл\n" + lName + "\nсовпадает с файлом\n" + twi2->FileNameShort() + "\n\nВсё равно загрузить?") == QMessageBox::Yes) {
                                            DoReload = true;
                                            break;
                                        } else {
                                            DoReload = false;
                                            break;
                                        }
                                    }
                                }

                            }
                            if (DoReload) twi->ReloadFrom(lName);


                        } else {
                            gLogger->ShowError(this, "AutoCAD support files",
                                              tr("Error opening file") + ":\n" + lName + "\n" + tr("Error") +": " + file.errorString());
                        }
                    }
                }

            } else {
                // like "add" button
                on_actionAddFiles_triggered();
            }
        } else if (actRes == actSave) {
            QFileDialog dlg(this);
            QString lName;
            dlg.setFileMode(QFileDialog::DirectoryOnly);


            lName = QCoreApplication::applicationDirPath();
            lName.resize(lName.lastIndexOf('/'));
            lName.resize(lName.lastIndexOf('/'));
            lName += "/temp/Plot Styles";

            QDir lDir(lName);
            if (lDir.exists()) dlg.setDirectory(lName);
            if (dlg.exec() == QDialog::Accepted) {
                for (i = 0; i < selected.count(); i++) {
                    twi = (FileLoadData *) selected.at(i);
                    if (twi->Id()) {
                        lName = dlg.selectedFiles().at(0) + "/" + twi->FileNameShort();
                        QSqlQuery query("select data, filedate from v_as_file where id = " + QString::number(twi->Id()), db);

                        if (query.lastError().isValid()) {
                            gLogger->ShowSqlError(this, "AutoCAD support files", query.lastError());
                        } else {
                            if (query.next()) {
                                QFile file(lName);
                                if (file.open(QFile::WriteOnly)) {
                                    file.write(query.value("data").toByteArray());
                                    file.close();
                                    gFileUtils->SetFileTime(lName, query.value("filedate").toDateTime());
                                } else {
                                    gLogger->ShowError(this, "AutoCAD support files",
                                                      tr("Error saving file") + ":\n" + lName + "\n" + tr("Error") +": " + file.errorString());
                                }
                            }
                        }
                    } else {
                        QMessageBox::critical(this, "AutoCAD support files", twi->FileNameShort() + " - пропущен");
                    }
                }
            }
        }
    }
}

void AcadSupFiles::on_actionDelFIles_triggered()
{
    QList<QTreeWidgetItem *> selected = ui->twList->selectedItems();
    if (selected.isEmpty()) return;
    if (QMessageBox::question(this, "AutoCAD support files", "Удалить выбранные строки?") != QMessageBox::Yes) return;

    int i;
    FileLoadData *twi;
    for (i = 0; i < selected.count(); i++) {
        twi = (FileLoadData *) selected.at(i);
        if (twi->Id()) mListForDel.append(twi->Id());
        delete twi;
    }
}

void AcadSupFiles::CommentsChanged(QWidget *editor)
{
    if (qobject_cast<QLineEdit *> (editor)) {
        QLineEdit *lLE = qobject_cast<QLineEdit *> (editor);
        QList<QTreeWidgetItem *> selected = ui->twList->selectedItems();
        for (int i = 0; i < selected.count(); i++) {
            selected.at(i)->setText(4, lLE->text());
        }
    }
}

void ASF_Sync() {
    QString lASFDirName;
    QDir lASFDir;
    QFile file;
    bool lDirExists;

    if (!gUserRight->CanSelect("v_as_file")) return;
    if (gSettings->GetHomeData("PLOT_STYLES").toInt() != 1) return;

    lASFDirName = QCoreApplication::applicationDirPath();
    lASFDirName.resize(lASFDirName.lastIndexOf('/'));
    lASFDirName.resize(lASFDirName.lastIndexOf('/'));

    lASFDirName += "/temp/Plot Styles";

    //gLogger->ShowError("AutoCAD support files", lASFDirName);

    lASFDir.setPath(lASFDirName);
    if (!(lDirExists = lASFDir.exists())) lASFDir.mkdir(lASFDirName);

    int i, j, k, l;

    // check all AutoCAD all profiles for correct Plot Styles path and copy files from existing if needed
    QSettings Acads("HKEY_CURRENT_USER\\Software\\Autodesk\\AutoCAD", QSettings::NativeFormat);

    QStringList AcadListR, AcadListVer;

    // through R17.0, etc
    AcadListR = Acads.childGroups();
    AcadListR.sort();
    for (i = AcadListR.length() - 1; i >= 0; i--) {
        QSettings AcadsR("HKEY_CURRENT_USER\\Software\\Autodesk\\AutoCAD\\" + AcadListR.at(i), QSettings::NativeFormat);
        // throw ACAD-5001:409, etc
        AcadListVer = AcadsR.childGroups();
        for (j = 0; j < AcadListVer.length(); j++) {
            QSettings AcadsProfiles("HKEY_CURRENT_USER\\Software\\Autodesk\\AutoCAD\\" + AcadListR.at(i) + "\\" + AcadListVer.at(j) + "\\Profiles", QSettings::NativeFormat);
            //gLogger->ShowError("AutoCAD support files", AcadListR.at(i));
            // throw profiles
            for (k = 0; k < AcadsProfiles.childGroups().length(); k++) {
                QString lOrigName = AcadsProfiles.value(AcadsProfiles.childGroups().at(k) + "/General/PrinterStyleSheetDir").toString();
                if (lOrigName.toLower() != lASFDirName.replace('/', '\\').toLower()) {
                    //if (!lDirExists) {
                        //// copy files if directory doesn't exist
                        QDir lOrig(lOrigName);
                        QStringList Files = lOrig.entryList();
                        for (l = 0; l < Files.length(); l++) {
                            if (Files.at(l)[0] != '.') {
                                file.setFileName(lASFDirName + "/" + Files.at(l));
                                if (!file.exists()) {
                                    file.setFileName(lOrigName + "/" + Files.at(l));
                                    file.copy(lASFDirName + "/" + Files.at(l));
                                }
                            }
                        }
                    //}
                    AcadsProfiles.setValue(AcadsProfiles.childGroups().at(k) + "/General/PrinterStyleSheetDir", lASFDirName.replace('/', '\\'));
                }
            }
        }
    }

    // download files from database
    // select deleted first and delete them
    QSqlQuery query("select a.id, a.filename, a.filedate, a.sha256, nvl(a.deleted, 0) deleted, dbms_lob.getlength(a.data) datasize, a.data,"
                    " case when exists (select 1 from v_as_file_right where login = user and (id_department = a.id_department or id_department is null))"
                    " then 1 else 0 end can_change"
                    " from v_as_file a"
                    " where a.id_department is null or a.id_department = (select id_department from users where login = user)"
                    " order by 4 desc", db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("AutoCAD support files", "Select from V_AS_FILE", query.lastError());
    } else {
        while (query.next()) {
            bool lFileEqual = false, lFileExists;
            QDateTime lDTOnDisk;
            QString lSha256;
            file.setFileName(lASFDirName + "/" + query.value("filename").toString());

            if (lFileExists = file.exists()) {
                QFileInfo fi(file);
                lDTOnDisk = fi.lastModified();

                // там где-то была функция, но не буду менять - и так хорошо
                if (file.open(QFile::ReadOnly)) {
                    // das ist fantastish!
                    QCryptographicHash hash1(QCryptographicHash::Sha256);
                    hash1.addData(&file);
                    file.close();

                    lSha256 = QString(hash1.result().toHex()).toUpper();
                    lFileEqual = lSha256 == query.value("sha256").toString().toUpper();
                }
            }
            if (lFileEqual) {
                // same file like in database but need to delete
                if (query.value("deleted").toInt()) {
                    file.remove();
                }
                // nothing to do if not deleted
                //gLogger->ShowError("AutoCAD support files", "equal " + query.value("filename").toString());
            } else {
                // not deleted
                if (!query.value("deleted").toInt()) {
                    bool lDoRewrite = false;

                    // уже проверено - не равен, не на удаление
                    // проверяется - уже существует и пользователь может его менять в Базе
                    if (lFileExists && query.value("can_change").toInt() == 1) {
                        if (lDTOnDisk < query.value("filedate").toDateTime()) {
                            // newer in base, rewrite on disk

                            lDoRewrite = true;
                        } else {
                            // newer file on disk, ask user what to do

                            QMessageBox mb;
                            mb.setWindowTitle("AutoCAD support files");
                            mb.setText("Файл " + query.value("filename").toString() + " новее на диске, чем в Базе.");
                            mb.setIcon(QMessageBox::Question);
                            mb.addButton("Перезаписать файл на диске", QMessageBox::YesRole);
                            mb.addButton("Загрузить файл в Базу", QMessageBox::NoRole);
                            mb.setDefaultButton(mb.addButton("Продолжить как есть", QMessageBox::RejectRole));
                            int re = mb.exec();
                            //gLogger->ShowError("AutoCAD support files", QString::number(re));
                            if (!re) {
                                // rewrite on disk
                                lDoRewrite = true;
                            } if (re == 1) {
                                // load file to database
                                if (file.open(QFile::ReadOnly)) {
                                    QByteArray buffer(file.readAll());
                                    file.close();

                                    QSqlQuery qLoad(db);
                                    qLoad.prepare("update v_as_file set fullname = :fullname, filedate = :filedate, sha256 = :sha256, data = :data where id = :id");
                                    if (qLoad.lastError().isValid()) {
                                        gLogger->ShowSqlError("AutoCAD support files - prepare", QObject::tr("Error loading file to Projects Base"), qLoad.lastError());
                                    } else {
                                        qLoad.bindValue(":id", query.value("id").toInt());
                                        qLoad.bindValue(":fullname", file.fileName());
                                        qLoad.bindValue(":filedate", lDTOnDisk);
                                        qLoad.bindValue(":sha256", lSha256);
                                        qLoad.bindValue(":data", buffer);
                                        if (!qLoad.exec()) {
                                            gLogger->ShowSqlError("AutoCAD support files - execute", QObject::tr("Error loading file to Projects Base"), qLoad.lastError());
                                        }
                                    }
                                } else {
                                    gLogger->ShowError("AutoCAD support files",
                                                          QObject::tr("Can't open file") + ":\n" + file.fileName() +
                                                          "\n" + QObject::tr("Error") + ": " + file.errorString());
                                }
                            } // do nothing in other cases
                        }

                    } else {
                        lDoRewrite = true;
                    }

                    // (re)write on disk
                    if (lDoRewrite) {
                        if (file.open(QFile::WriteOnly)) {
                            file.write(query.value("data").toByteArray());
                            file.close();

                            gFileUtils->SetFileTime(file.fileName(), query.value("filedate").toDateTime());
                        }
                    }
                }
                //gLogger->ShowError("AutoCAD support files", "NO equal " + query.value("filename").toString());
            }
        }
    }
}
