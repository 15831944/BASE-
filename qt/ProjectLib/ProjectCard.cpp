#include "ProjectCard.h"
#include "ui_ProjectCard.h"

#include "ProjectData.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/oracle.h"

#include "../VProject/common.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"
#include "../UsersDlg/CustomerData.h"
#include "../UsersDlg/DepartData.h"

#include <QDateEdit>

ProjectCard::ProjectCard(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ProjectCard),
    mIdProject(0)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    QPCPropItemDelegate *idPCProp = new QPCPropItemDelegate(this);
    ui->twProps->setItemDelegateForColumn(2, idPCProp);
    connect(idPCProp, SIGNAL(commitData(QWidget *)), this, SLOT(PropDataChanged(QWidget *)));

    QPCDepCustItemDelegate *idPCDepCust = new QPCDepCustItemDelegate(this);
    ui->twSR->setItemDelegateForColumn(2, idPCDepCust);
    connect(idPCDepCust, SIGNAL(commitData(QWidget *)), this, SLOT(PropDeptCustChanged(QWidget *)));

    QPCPersonItemDelegate *idPCPerson = new QPCPersonItemDelegate(this);
    ui->twSR->setItemDelegateForColumn(3, idPCPerson);

//    mDoNotSave = true;
}

ProjectCard::~ProjectCard()
{
    delete ui;
}

void ProjectCard::showEvent(QShowEvent* event) {
    int i;
    QFCDialog::showEvent(event);

    QTableWidgetItem *twi;
    QFont f;

    //ui->pbWord->setVisible(false);

    // properties part ---------------------------------------------------------------------------------------------------------------
    ui->twProps->setRowCount(0);
    ui->twProps->verticalHeader()->setSectionsClickable(false);
    ui->twProps->horizontalHeader()->setSectionsClickable(false);
    ui->twProps->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    ui->twProps->setColumnHidden(1, true); // label in doc
    ui->twProps->setColumnHidden(3, true);// prop. original value (for compare in Accept)
    ui->twProps->setColumnHidden(4, true); // id for existing record in v_proj_prop (null for new record)
    ui->twProps->setColumnHidden(5, true); // prop_type
    ui->twProps->setColumnHidden(6, true); // order_num

    if (ReadVersion < CurrentVersion) {
        ui->twProps->setColumnWidth(0, 300); // russian prop. name
        ui->twProps->setColumnWidth(2, 800); // prop. value
    }

    // all users
    ui->cbCreator->clear();
    ui->cbCreator->addItem("");
    // bosses only
    ui->cbSigner->clear();
    ui->cbSigner->addItem("");

    for (i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled()) {
            ui->cbCreator->addItem(gUsers->UsersConst().at(i)->NameConst());
            if (gUsers->UsersConst().at(i)->IsBoss())
                ui->cbSigner->addItem(gUsers->UsersConst().at(i)->NameConst());
        }

    int cnt = 0;

    QSqlQuery query(
                "select prop_name, doc_label, prop_value, id, order_num, coalesce(prop_type, 0) prop_type"
                " from v_proj_prop where id_project = " + QString::number(mIdProject) +
                " order by order_num", db);

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "Карточка проекта - свойства", query.lastError().text());
    } else {
        while (query.next()) {
            if (query.value("order_num").toInt() < 1000) {
                ui->twProps->insertRow(cnt);

                twi = new QTableWidgetItem(query.value("prop_name").toString());
                twi->setFlags(Qt::ItemIsEnabled);
                ui->twProps->setItem(cnt, 0, twi);

                twi = new QTableWidgetItem(query.value("doc_label").toString());
                ui->twProps->setItem(cnt, 1, twi);

                twi = new QTableWidgetItem(query.value("prop_value").toString());
                ui->twProps->setItem(cnt, 2, twi);

                twi = new QTableWidgetItem(query.value("prop_value").toString());
                ui->twProps->setItem(cnt, 3, twi);

                twi = new QTableWidgetItem(query.value("id").toString());
                ui->twProps->setItem(cnt, 4, twi);

                twi = new QTableWidgetItem(query.value("prop_type").toString());
                ui->twProps->setItem(cnt, 5, twi);

                twi = new QTableWidgetItem(query.value("order_num").toString());
                ui->twProps->setItem(cnt, 6, twi);
            } else {
                if (query.value(1).toString().toLower() == "created_user") {
                    creatorId = query.value(3).toInt();
                    origCreator = query.value(2).toString();
                    ui->cbCreator->setCurrentText(origCreator);
                } else if (query.value(1).toString().toLower() == "sign_user") {
                    signerId = query.value(3).toInt();
                    origSigner = query.value(2).toString();
                    ui->cbSigner->setCurrentText(origSigner);
                } else if (query.value(1).toString().toLower() == "created_post") {
                    creatorPostId = query.value(3).toInt();
                } else if (query.value(1).toString().toLower() == "sign_post") {
                    signerPostId = query.value(3).toInt();
                }
            }
            cnt++;
        }
    }

    if (!cnt) {
        mNewRecordProp = true;
        QSqlQuery queryProj(
                    "select a.name name,"
                    " a.shortname shortname, a.stage stage, (select name from v_customer where id = a.id_customer) tech_client,"
                    " (select thevalue from homedata where thename = 'NAME') our_name,"
                    " (select num from v_contract_simple where id_project = a.id_project"
                    "   and id_contract is null"
                    "   and coalesce(signdate, trunc(current_date)) ="
                    "    (select coalesce(max(coalesce(signdate, trunc(current_date))), trunc(current_date)) from v_contract_simple where id_project = a.id_project)) contract_num,"
                    " (select signdate from v_contract_simple where id_project = a.id_project"
                    "   and id_contract is null"
                    "   and coalesce(signdate, trunc(current_date)) ="
                    "    (select coalesce(max(coalesce(signdate, trunc(current_date))), trunc(current_date)) from v_contract_simple where id_project = a.id_project)) contract_date,"
                    " pp.GetUserNameDisp(a.gip) gip, a.comments comments"
                    " from v_project a where a.id = " + QString::number(mIdProject), db);

        if (queryProj.lastError().isValid()) {
            QMessageBox::critical(this, "Карточка проекта - проект", queryProj.lastError().text());
        } else {
            if (queryProj.next()) {
                QSqlQuery query2(
                            "select prop_name, doc_label, coalesce(prop_type, 0) prop_type, order_num"
                            " from v_proj_prop_t where order_num < 1000 order by order_num", db);

                if (query2.lastError().isValid()) {
                    QMessageBox::critical(this, "Карточка проекта - шаблон свойств", query2.lastError().text());
                } else {
                    while (query2.next()) {
                        ui->twProps->insertRow(cnt);

                        twi = new QTableWidgetItem(query2.value("prop_name").toString());
                        twi->setFlags(/*Qt::ItemIsSelectable |*/ Qt::ItemIsEnabled);
                        ui->twProps->setItem(cnt, 0, twi);

                        twi = new QTableWidgetItem(query2.value("doc_label").toString());
                        ui->twProps->setItem(cnt, 1, twi);


                        if (twi->text() == "project") {
                            twi = new QTableWidgetItem(queryProj.value("name").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "contract") {
                            twi = new QTableWidgetItem(queryProj.value("contract_num").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "proj_short") {
                            twi = new QTableWidgetItem(queryProj.value("shortname").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "stages") {
                            twi = new QTableWidgetItem(queryProj.value("stage").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "tech_client") {
                            twi = new QTableWidgetItem(queryProj.value("tech_client").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "gen_developer") {
                            twi = new QTableWidgetItem(queryProj.value("our_name").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "contract_date") {
                            twi = new QTableWidgetItem(queryProj.value("contract_date").toDate().toString("dd.MM.yyyy"));
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "kgip") {
                            twi = new QTableWidgetItem(queryProj.value("gip").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        } else if (twi->text() == "comments") {
                            twi = new QTableWidgetItem(queryProj.value("comments").toString());
                            ui->twProps->setItem(cnt, 2, twi);
                        }

                        twi = new QTableWidgetItem(query2.value("prop_type").toString());
                        ui->twProps->setItem(cnt, 5, twi);


                        twi = new QTableWidgetItem(query2.value("order_num").toString());
                        ui->twProps->setItem(cnt, 6, twi);

                        cnt++;
                    }
                }
            } else {
                QMessageBox::critical(this, "Карточка проекта", "Проект " + QString::number(mIdProject) + " не существует!");
            }
        }
    } else {
        mNewRecordProp = false;
    }

    QStringList sl;
    for (i = 0 ; i < ui->twProps->rowCount(); i++) {
        sl.append("    ");
    }
    ui->twProps->setVerticalHeaderLabels(sl);

    if (ReadVersion < CurrentVersion) {
        ui->twProps->resizeColumnToContents(0);
        ui->twProps->setColumnWidth(0, ui->twProps->columnWidth(0) + 40);
    }
    ui->twProps->resizeRowsToContents();


    // works part ---------------------------------------------------------------------------------------------------------------
    ui->twSR->setRowCount(0);
    ui->twSR->verticalHeader()->setSectionsClickable(false);
    ui->twSR->horizontalHeader()->setSectionsClickable(false);

    if (ReadVersion < CurrentVersion) {
        ui->twSR->setColumnWidth(0, 100); // number for main
        ui->twSR->setColumnWidth(1, 300); // work name
        ui->twSR->setColumnWidth(2, 300); // deprtment/customer
        ui->twSR->setColumnWidth(3, 300); // person
        ui->twSR->setColumnWidth(4, 300); // comments
    }
    ui->twSR->setColumnHidden(5, true);
    ui->twSR->setColumnHidden(6, true);
    ui->twSR->setColumnHidden(7, true);
    ui->twSR->setColumnHidden(8, true);
    ui->twSR->setColumnHidden(9, true);

    if (!query.exec(
                "select case when id_parent is null"
                "  then order_num * 10000"
                "  else (select order_num * 10000 + a.order_num from v_proj_sr where id = a.id_parent) end,"
                " id, id_parent, order_num, name, department, person, comments"
                "  from v_proj_sr a where id_project = " + QString::number(mIdProject) +
                " order by 1")) {
        QMessageBox::critical(this, "Карточка проекта - состав работ", query.lastError().text());
    } else {
        int mainCnt = 1;
        cnt = 0;
        while (query.next()) {
            ui->twSR->insertRow(cnt);

            twi = new QTableWidgetItem();
            if (query.value("id_parent").isNull()) {
                twi->setText(QString::number(mainCnt));
                mainCnt++;
            }
            twi->setFlags(Qt::ItemIsEnabled);
            twi->setTextAlignment(Qt::AlignCenter);
            f = twi->font();
            f.setBold(true);
            twi->setFont(f);
            ui->twSR->setItem(cnt, 0, twi);

            twi = new QTableWidgetItem(query.value("name").toString());
            twi->setFlags(Qt::ItemIsEnabled);
            if (query.value("id_parent").isNull()) {
                f = twi->font();
                f.setBold(true);
                twi->setFont(f);
            }
            ui->twSR->setItem(cnt, 1, twi);

            twi = new QTableWidgetItem(query.value("department").toString());
            ui->twSR->setItem(cnt, 2, twi);

            twi = new QTableWidgetItem(query.value("person").toString());
            ui->twSR->setItem(cnt, 3, twi);

            twi = new QTableWidgetItem(query.value("comments").toString());
            ui->twSR->setItem(cnt, 4, twi);

            twi = new QTableWidgetItem(query.value("order_num").toString());
            ui->twSR->setItem(cnt, 5, twi);

            twi = new QTableWidgetItem(query.value("id").toString());
            ui->twSR->setItem(cnt, 6, twi);

            // original values for compare
            twi = new QTableWidgetItem(query.value("department").toString());
            ui->twSR->setItem(cnt, 7, twi);

            twi = new QTableWidgetItem(query.value("person").toString());
            ui->twSR->setItem(cnt, 8, twi);

            twi = new QTableWidgetItem(query.value("comments").toString());
            ui->twSR->setItem(cnt, 9, twi);


            cnt++;
        }

        if (!cnt) {
            mNewRecordSR = true;

            QSqlQuery query2(
                        "select id, order_num, name"
                        " from v_proj_sr_t where id_parent is null order by order_num", db);


            if (query2.lastError().isValid()) {
                QMessageBox::critical(this, "Карточка проекта - шаблон состава работ", query2.lastError().text());
            } else {
                while (query2.next()) {
                    ui->twSR->insertRow(cnt);

                    twi = new QTableWidgetItem(QString::number(mainCnt));
                    twi->setFlags(Qt::ItemIsEnabled);
                    twi->setTextAlignment(Qt::AlignCenter);
                    f = twi->font();
                    f.setBold(true);
                    twi->setFont(f);
                    ui->twSR->setItem(cnt, 0, twi);

                    twi = new QTableWidgetItem(query2.value("name").toString());
                    twi->setFlags(Qt::ItemIsEnabled);
                    f = twi->font();
                    f.setBold(true);
                    twi->setFont(f);
                    ui->twSR->setItem(cnt, 1, twi);

                    twi = new QTableWidgetItem(query2.value("order_num").toString());
                    ui->twSR->setItem(cnt, 5, twi);

                    cnt++;

                    QSqlQuery query3(
                                "select id, order_num, name"
                                " from v_proj_sr_t where id_parent = " + query2.value("id").toString() + "order by order_num", db);

                    if (query3.lastError().isValid()) {
                        QMessageBox::critical(this, "Карточка проекта - шаблон состава работ", query3.lastError().text());
                    } else {
                        while (query3.next()) {
                            ui->twSR->insertRow(cnt);

                            twi = new QTableWidgetItem();
                            twi->setFlags(Qt::ItemIsEnabled);
                            ui->twSR->setItem(cnt, 0, twi);

                            twi = new QTableWidgetItem(query3.value("name").toString());
                            twi->setFlags(Qt::ItemIsEnabled);
                            ui->twSR->setItem(cnt, 1, twi);

                            twi = new QTableWidgetItem(query3.value("order_num").toString());
                            ui->twSR->setItem(cnt, 5, twi);

                            cnt++;
                        }
                    }

                    mainCnt++;
                }
            }
        } else {
            mNewRecordSR = false;
        }
        if (ReadVersion < CurrentVersion) {
            ui->twSR->resizeColumnToContents(0);
            ui->twSR->resizeColumnToContents(1);
            ui->twSR->setColumnWidth(1, ui->twSR->columnWidth(1) + 20);
            ui->twSR->resizeRowsToContents();
            if (ui->twSR->columnWidth(2) < 350)
                ui->twSR->setColumnWidth(2, 350);
            if (ui->twSR->columnWidth(3) < 200)
                ui->twSR->setColumnWidth(3, 200);
            if (ui->twSR->columnWidth(4) < 250)
                ui->twSR->setColumnWidth(4, 250);
        }
        ui->twSR->resizeRowsToContents();
    }
}

bool ProjectCard::DoSave() {
    bool lDoAccept = false;
    int i;
    QString PropValue;

    // it causes end editing with post data
    ui->twProps->setCurrentCell(0, 0);
    ui->twSR->setCurrentCell(0, 0);

    if (db.transaction()) {
        bool lChanged = false;
        bool lErr = false;
        if (mNewRecordProp) {
            QSqlQuery qInsert(db);

            lChanged = true;
            // cerate & sign
            for (i = 0; i < 2; i++) {
                QString userName, DocLabel, DocLabel2;
                if (!i) {
                    userName = ui->cbCreator->currentText();
                    DocLabel = "created_user";
                    DocLabel2 = "created_post";
                } else {
                    userName = ui->cbSigner->currentText();
                    DocLabel = "sign_user";
                    DocLabel2 = "sign_post";
                }
                qInsert.prepare("insert into v_proj_prop(id_project, order_num, prop_name, doc_label, prop_type, prop_value)"
                                "  (select :id_project, order_num, prop_name, doc_label, prop_type, :prop_value"
                                "     from v_proj_prop_t where lower(doc_label) = lower(:doc_label))");
                if (qInsert.lastError().isValid()) {
                    QMessageBox::critical(this, "Карточка проекта", qInsert.lastError().text());
                    lErr = true;
                    break;
                } else {
                    qInsert.bindValue(":id_project", mIdProject);
                    qInsert.bindValue(":prop_value", userName);
                    qInsert.bindValue(":doc_label", DocLabel);
                    if (!qInsert.exec()) {
                        QMessageBox::critical(this, "Карточка проекта", qInsert.lastError().text());
                        lErr = true;
                        break;
                    } else {
                        UserData *lUser = NULL;
                        if (userName.isEmpty() || (lUser = gUsers->FindByName(userName))) {
                            qInsert.bindValue(":prop_value", lUser?lUser->JobTitleConst():"");
                            qInsert.bindValue(":doc_label", DocLabel2);
                            if (!qInsert.exec()) {
                                QMessageBox::critical(this, "Карточка проекта", qInsert.lastError().text());
                                lErr = true;
                                break;
                            }
                        }
                    }
                }
            }

            // properties
            if (!lErr) {
                qInsert.prepare("insert into v_proj_prop(id_project, order_num, prop_name, doc_label, prop_type, prop_value)"
                                " values (:id_project, :order_num, :prop_name, :doc_label, :prop_type, :prop_value)");
                if (qInsert.lastError().isValid()) {
                    QMessageBox::critical(this, "Карточка проекта", qInsert.lastError().text());
                    lErr = true;
                } else {
                    qInsert.bindValue(":id_project", mIdProject);

                    for (i = 0; i < ui->twProps->rowCount(); i++) {
                        QTableWidgetItem *twi = ui->twProps->item(i, 2);
                        if (twi)
                            PropValue = twi->text();
                        else
                            PropValue.clear();

                        qInsert.bindValue(":order_num", ui->twProps->item(i, 6)->text());
                        qInsert.bindValue(":prop_name", ui->twProps->item(i, 0)->text());
                        qInsert.bindValue(":doc_label", ui->twProps->item(i, 1)->text());
                        qInsert.bindValue(":prop_type", ui->twProps->item(i, 5)->text());
                        qInsert.bindValue(":prop_value", PropValue);

                        if (!qInsert.exec()) {
                            QMessageBox::critical(this, "Карточка проекта", qInsert.lastError().text());
                            lErr = true;
                            break;
                        }
                    }
                }
            }
        } else {
            // update existing records
            QSqlQuery qUpdate(db);
            qUpdate.prepare("update v_proj_prop set prop_value = :prop_value where id = :id");
            if (qUpdate.lastError().isValid()) {
                QMessageBox::critical(this, "Карточка проекта", qUpdate.lastError().text());
                lErr = true;
            } else {
                // created && sign
                for (i = 0; i < 2; i++) {
                    QString userName, userNameOrig;
                    int updIdUser, updIdPost;
                    if (!i) {
                        userName = ui->cbCreator->currentText();
                        userNameOrig = origCreator;
                        updIdUser = creatorId;
                        updIdPost = creatorPostId;
                    } else {
                        userName = ui->cbSigner->currentText();
                        userNameOrig = origSigner;
                        updIdUser = signerId;
                        updIdPost = signerPostId;
                    }
                    if (userName != userNameOrig) {
                        lChanged = true;

                        qUpdate.bindValue(":id", updIdUser);
                        qUpdate.bindValue(":prop_value", userName);
                        if (!qUpdate.exec()) {
                            QMessageBox::critical(this, "Карточка проекта", qUpdate.lastError().text());
                            lErr = true;
                            break;
                        } else {
                            UserData *lUser;
                            if (lUser = gUsers->FindByName(userName)) {
                                qUpdate.bindValue(":id", updIdPost);
                                qUpdate.bindValue(":prop_value", lUser->JobTitleConst());
                                if (!qUpdate.exec()) {
                                    QMessageBox::critical(this, "Карточка проекта", qUpdate.lastError().text());
                                    lErr = true;
                                    break;
                                }
                            }
                        }
                    }
                }

                // properties
                if (!lErr) {
                    for (i = 0; i < ui->twProps->rowCount(); i++) {
                        QTableWidgetItem *twi = ui->twProps->item(i, 2);
                        if (twi)
                            PropValue = twi->text();
                        else
                            PropValue.clear();
                        if (PropValue != ui->twProps->item(i, 3)->text()) {
                            lChanged = true;


                            qUpdate.bindValue(":id", ui->twProps->item(i, 4)->text());
                            qUpdate.bindValue(":prop_value", PropValue);
                            if (!qUpdate.exec()) {
                                QMessageBox::critical(this, "Карточка проекта", qUpdate.lastError().text());
                                lErr = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (!lErr) {
            // works
            if (mNewRecordSR) {
                QSqlQuery qInsert(db);

                lChanged = true;

                qInsert.prepare(
                            "insert into v_proj_sr (id, id_project, id_parent, order_num, name, department, person, comments)"
                            " values (:id, :id_project, :id_parent, :order_num, :name, :department, :person, :comments)");
                if (qInsert.lastError().isValid()) {
                    QMessageBox::critical(this, "Карточка проекта", qInsert.lastError().text());
                    lErr = true;
                } else {
                    qInsert.bindValue(":id_project", mIdProject);

                    int idMain = 0, idChild;
                    QString sDepartment, sPerson, sComments;
                    QTableWidgetItem *twi;

                    for (i = 0; i < ui->twSR->rowCount(); i++) {
                        if (!ui->twSR->item(i, 0)->text().isEmpty()) {
                            // main item
                            if (gOracle->GetSeqNextVal("seq_proj_sr_id", idMain)) {
                                qInsert.bindValue(":id", idMain);
                                qInsert.bindValue(":id_parent", QVariant());
                            } else {
                                lErr = true;
                                break;
                            }
                        } else {
                            if (gOracle->GetSeqNextVal("seq_proj_sr_id", idChild)) {
                                qInsert.bindValue(":id", idChild);
                                qInsert.bindValue(":id_parent", idMain);

                            } else {
                                lErr = true;
                                break;
                            }
                        }
                        qInsert.bindValue(":order_num", ui->twSR->item(i, 5)->text());
                        qInsert.bindValue(":name", ui->twSR->item(i, 1)->text());

                        twi = ui->twSR->item(i, 2);
                        if (twi)
                            sDepartment = twi->text();
                        else
                            sDepartment.clear();
                        qInsert.bindValue(":department", sDepartment);

                        twi = ui->twSR->item(i, 3);
                        if (twi)
                            sPerson = twi->text();
                        else
                            sPerson.clear();
                        qInsert.bindValue(":person", sPerson);

                        twi = ui->twSR->item(i, 4);
                        if (twi)
                            sComments = twi->text();
                        else
                            sComments.clear();
                        qInsert.bindValue(":comments", sComments);

                        if (!qInsert.exec()) {
                            QMessageBox::critical(this, "Карточка проекта", qInsert.lastError().text());
                            lErr = true;
                            break;
                        }
                    }
                }
            } else {
                // sr - existing record
                QSqlQuery qUpdate(db);

                qUpdate.prepare("update v_proj_sr set department = :department, person = :person, comments = :comments where id = :id");
                if (qUpdate.lastError().isValid()) {
                    QMessageBox::critical(this, "Карточка проекта", qUpdate.lastError().text());
                    lErr = true;
                } else {
                    QString sDepartment, sDepartmentOrig, sPerson, sPersonOrig, sComments, sCommentsOrig;

                    for (i = 0; i < ui->twSR->rowCount(); i++) {
                        if (ui->twSR->item(i, 2))
                            sDepartment = ui->twSR->item(i, 2)->text();
                        else
                            sDepartment.clear();
                        if (ui->twSR->item(i, 7))
                            sDepartmentOrig = ui->twSR->item(i, 7)->text();
                        else
                            sDepartmentOrig.clear();

                        if (ui->twSR->item(i, 3))
                            sPerson = ui->twSR->item(i, 3)->text();
                        else
                            sPerson.clear();
                        if (ui->twSR->item(i, 8))
                            sPersonOrig = ui->twSR->item(i, 8)->text();
                        else
                            sPersonOrig.clear();

                        if (ui->twSR->item(i, 4))
                            sComments = ui->twSR->item(i, 4)->text();
                        else
                            sComments.clear();
                        if (ui->twSR->item(i, 9))
                            sCommentsOrig = ui->twSR->item(i, 9)->text();
                        else
                            sCommentsOrig.clear();

                        if (sDepartment != sDepartmentOrig
                                || sPerson != sPersonOrig
                                || sComments != sCommentsOrig) {
                            // any changes
                            lChanged = true;
                            if (ui->twSR->item(i, 6)
                                    && !ui->twSR->item(i, 6)->text().isEmpty()) {
                                // id exists
                                qUpdate.bindValue(":id", ui->twSR->item(i, 6)->text().toInt());
                                qUpdate.bindValue(":department", sDepartment);
                                qUpdate.bindValue(":person", sPerson);
                                qUpdate.bindValue(":comments", sComments);
                                if (!qUpdate.exec()) {
                                    QMessageBox::critical(this, "Карточка проекта", qUpdate.lastError().text());
                                    lErr = true;
                                    break;
                                }
                            } else {
                                // "rare" part - user added string (not realized yet)
                            }
                        }
                    }
                }
            }
        }

        if (lChanged && !lErr) {
            if (!db.commit()) {
                QMessageBox::critical(this, "Карточка проекта", tr("Can't commit") + "\n" + db.lastError().text());
            } else {
                lDoAccept = true;
            }
        } else {
            db.rollback();
            if (!lChanged) lDoAccept = true;
        }
    } else {
        QMessageBox::critical(this, "Карточка проекта", db.lastError().text());
    }
    return lDoAccept;
}

void ProjectCard::Accept() {
    if (DoSave()) accept();
}

void ProjectCard::PropDataChanged(QWidget *editor)
{
    int lDT;

    lDT = ui->twProps->model()->data(ui->twProps->model()->index(ui->twProps->currentRow(), 5)).toInt();
    if (lDT == 1) {
        if (qobject_cast<QDateEdit *> (editor)) {
            if (qobject_cast<QDateEdit *> (editor)->date().toString("dd.MM.yyyy") == "01.01.2000") {
                ui->twProps->currentItem()->setData(Qt::DisplayRole, "");
            } else {
                ui->twProps->currentItem()->setData(Qt::DisplayRole, qobject_cast<QDateEdit *> (editor)->date().toString("dd.MM.yyyy"));
            }
        }
    } else if (lDT > 100 && lDT < 200) {
        if (qobject_cast<QComboBox *> (editor))
            if (!qobject_cast<QComboBox *>(editor)->currentText().isEmpty()
                    && !gUsers->FindByName(qobject_cast<QComboBox *>(editor)->currentText())) {
                QMessageBox::critical(this, "Карточка проекта", "Пользователь " + (qobject_cast<QComboBox *> (editor))->currentText() + " не существует!");
                ui->twProps->currentItem()->setData(Qt::DisplayRole, "");
            }
    }
}

void ProjectCard::PropDeptCustChanged(QWidget *editor)
{
    if (qobject_cast<QComboBox *> (editor)) {
        QComboBox *cb = qobject_cast<QComboBox *> (editor);
        int i;
        //QMessageBox::critical(this, "Карточка проекта", cb->currentText());
        //i = cb->findText(cb->currentText(), Qt::MatchExactly);
        //QMessageBox::critical(this, "Карточка проекта", QString::number(i));
        i = -1;
        if (i == -1) {
            i = cb->findText(cb->currentText(), Qt::MatchContains);
            //QMessageBox::critical(this, "Карточка проекта", cb->itemText(i));
            if (i != -1) {
                ui->twSR->currentItem()->setData(Qt::DisplayRole, cb->itemText(i));
            }
        } else {
            QMessageBox::critical(this, "Карточка проекта", QString::number(cb->count()));
            //QMessageBox::critical(this, "Карточка проекта", QString::number(i));
            cb->setCurrentText(cb->itemText(i));
        }
    }
}

QPCPropItemDelegate::QPCPropItemDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

void QPCPropItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    if (qobject_cast<QDateEdit *> (editor)) {
        QDate d;
        d = d.fromString(index.data().toString(), "dd.MM.yyyy");
        qobject_cast<QDateEdit *> (editor)->setDate(d);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

QWidget * QPCPropItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    int lDT;

//    if (index.data().canConvert(QVariant::Date)) {
//    }

    lDT = index.model()->data(index.model()->index(index.row(), 5)).toInt();
    if (lDT == 1) {
        QDateEdit *de = new QDateEdit(parent);
        de->setCalendarPopup(true);
        return de;
    } else if (lDT > 100 && lDT < 200) {
        QComboBox *ul = new QComboBox(parent);
        ul->setEditable(true);
        ul->addItem("");
        for (int i = 0; i < gUsers->UsersConst().length(); i++)
            if (!gUsers->UsersConst().at(i)->Disabled())
                ul->addItem(gUsers->UsersConst().at(i)->NameConst());
        return ul;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

QPCDepCustItemDelegate::QPCDepCustItemDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * QPCDepCustItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    int i;
    QComboBox *ul = new QComboBox(parent);
    ul->setEditable(true);
    ul->addItem("");

    for (i = 0; i < gDeparts->DepartListConst().length(); i++)
        ul->addItem(gDeparts->DepartListConst().at(i)->NameConst());
    foreach (const CustomerData * lCustomerData, gCustomers->CustomerListConst()) ul->addItem(lCustomerData->ShortNameConst());

    return ul;
}

QPCPersonItemDelegate::QPCPersonItemDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * QPCPersonItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QComboBox *ul = new QComboBox(parent);
    ul->setEditable(true);
    ul->addItem("");
    for (int i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled())
            ul->addItem(gUsers->UsersConst().at(i)->NameConst());

    return ul;
}
