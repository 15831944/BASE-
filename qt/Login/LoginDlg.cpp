#include "LoginDlg.h"
#include "ui_LoginDlg.h"

#include "Login.h"

//#include "../VProject/GlobalSettings.h"

#include "../Logger/logger.h"

#include <QTimer>
#include <QFile>
#include <QKeyEvent>
#include <QProcessEnvironment>
#include <QSettings>
#include <QMessageBox>

//extern QSqlDatabase db;
//extern QString gCurrentSchema;

bool Login(QString &aSelectedLang, QString &aSchemaName, QString &aBaseName) {
    LoginDlg w;
    if (w.exec() == QDialog::Accepted) {
        aSelectedLang = w.Lang();
        aSchemaName = w.SchemaName();
        aBaseName = w.BaseName();
        return true;
    }
    return false;
}

LoginDlg::LoginDlg(QWidget *parent) :
    QFCDialog(parent),
    ui(new Ui::LoginDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    for (int i = 0; i < children().length(); i++) {
        children().at(i)->installEventFilter(this);
    }

    ui->lblHost->setVisible(false);
    ui->leHost->setVisible(false);

    ui->lePass->setFocus();

    const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment();
    db.setDatabaseName(env.value("PROJ_HOSTSTRING"));

    QSettings lSettings;
    QString lCurSchema;

    lSettings.beginGroup("LastLogin");
    mUser = lSettings.value("Username").toString();
    mLang = lSettings.value("Lang").toString();
    lCurSchema = lSettings.value("Schema").toString();
    lSettings.endGroup();

    // TEMPORARY PART
    //------------------------------------------------------------------------
    QString lLastUser = QCoreApplication::applicationDirPath();

    lLastUser.resize(lLastUser.lastIndexOf(QChar('/')));
    lLastUser.resize(lLastUser.lastIndexOf(QChar('/')));
    lLastUser += "/lastuser";
    if (mUser.isEmpty()) {

        QFile fLastUser(lLastUser);
        if (fLastUser.open(QFile::ReadOnly)) {
            QByteArray buffer(fLastUser.readAll());
            fLastUser.close();
            QList<QByteArray> buffer2 = buffer.split('\n');
            if (buffer2.length() > 1) {
                mUser = buffer2.at(0);
                mUser = mUser.trimmed();
                mLang = buffer2.at(1);
                mLang = mLang.trimmed();
            }
        }
    } else {
        QFile::remove(lLastUser);
    }
    //------------------------------------------------------------------------

    if (lCurSchema.isEmpty()) {
        lCurSchema = env.value("PROJ_LOGIN");
    }

    if (InitByHost()) {
        if (!lCurSchema.isEmpty()) {
            for (int i = 0; i < ui->cbBase->count(); i++) {
                if (ui->cbBase->itemData(i).toString() == lCurSchema) {
                    ui->cbBase->setCurrentIndex(i);
                    break;
                }
            }
        }
    } else {
        QTimer::singleShot(0, this, SLOT(hide()));
    }
}

LoginDlg::~LoginDlg()
{
    delete ui;
}

bool LoginDlg::InitByHost() {
    bool lIsOk = true;

    const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment();
    db.setUserName(env.value("PROJ_LOGIN"));
    db.setPassword(env.value("PROJ_PASS"));

    if (db.driverName() == "QPSQL") {
        db.setHostName(env.value("PROJ_HOST"));
        db.setPort(env.value("PROJ_PORT").toInt());
    }

    mUserList.clear();
    mLangList.clear();

    ui->cbBase->clear();

    if (!db.open()) {
        gLogger->ShowSqlError(this, "Login", "Initial connect", db);
        lIsOk = false;
    } else {
        if (db.driverName() == "QPSQL") {
            QSqlQuery qUsers("select login, name from v_users_login order by name", db);
            if (qUsers.lastError().isValid()) {
                gLogger->ShowSqlError(this, "Login", "Select from V_USERS", qUsers);
                lIsOk = false;
            } else {
                QString lProjSchema = env.value("PROJ_SCHEMA");
                QMap<QString, QString> lUsers;
                while (qUsers.next()) {
                    lUsers[qUsers.value("name").toString()] = qUsers.value("login").toString();
                }
                mUserList[lProjSchema] = lUsers;

                QSqlQuery qLangs("select name || ' - ' || name_en as name, id from langs where coalesce(disabled, 0) <> 1", db);
                if (qLangs.lastError().isValid()) {
                    gLogger->ShowSqlError(this, "Login", "Select from LANGS", qLangs);
                    lIsOk = false;
                } else {
                    QMap<QString, QString> lLangs;
                    while (qLangs.next()) {
                        lLangs[qLangs.value("name").toString()] = qLangs.value("id").toString();
                    }
                    mLangList[lProjSchema] = lLangs;

                    ui->cbBase->addItem("PostgreSQL", lProjSchema);
                }
            }
        } else {
            QSqlQuery query("select owner from all_tab_privs_recd where table_name = 'V_BASENAME'", db);
            if (query.lastError().isValid()) {
                gLogger->ShowSqlError(this, "Login", "Select from ALL_TAB_PRIVS_RECD", query);
                lIsOk = false;
            } else {
                while (query.next() && lIsOk) {
                    QSqlQuery qSession("alter session set current_schema = " + query.value("owner").toString(), db);
                    if (qSession.lastError().isValid() || !qSession.exec()) {
                        gLogger->ShowSqlError(this, "Login", "Alter session", qSession);
                        lIsOk = false;
                    } else {
                        QSqlQuery qBaseName("select base_name from v_basename", db);
                        if (qBaseName.lastError().isValid()) {
                            gLogger->ShowSqlError(this, "Login", "Select from V_BASENAME", qBaseName);
                            lIsOk = false;
                        } else {
                            if (!qBaseName.next()) {
                                gLogger->ShowError(this, "Login", "Select from V_BASENAME - no records");
                                lIsOk = false;
                            } else {
                                QSqlQuery qUsers("select login, name from v_users_login order by name", db);
                                if (qUsers.lastError().isValid()) {
                                    gLogger->ShowSqlError(this, "Login", "Select from V_USERS", qUsers);
                                    lIsOk = false;
                                } else {
                                    QMap<QString, QString> lUsers;
                                    while (qUsers.next()) {
                                        lUsers[qUsers.value("name").toString()] = qUsers.value("login").toString();
                                    }
                                    mUserList[query.value("owner").toString()] = lUsers;

                                    QSqlQuery qLangs("select name || ' - ' || name_en name, id from langs where coalesce(disabled, 0) <> 1", db);
                                    if (qLangs.lastError().isValid()) {
                                        gLogger->ShowSqlError(this, "Login", "Select from LANGS", qLangs);
                                        lIsOk = false;
                                    } else {
                                        QMap<QString, QString> lLangs;
                                        while (qLangs.next()) {
                                            lLangs[qLangs.value("name").toString()] = qLangs.value("id").toString();
                                        }
                                        mLangList[query.value("owner").toString()] = lLangs;

                                        ui->cbBase->addItem(qBaseName.value("base_name").toString(), query.value("owner").toString());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        db.close();
    }
    ui->cbBase->setVisible(ui->cbBase->count() > 1);
    ui->lblBase->setVisible(ui->cbBase->count() > 1);

    return lIsOk;
}

bool LoginDlg::InitDB(const QString &aSchemaName) {
    bool res = false;

    if (db.driverName() == "QPSQL") {
        res = true;
    } else {
        QSqlQuery query("alter session set current_schema = " + aSchemaName, db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError("InitDB", query);
        } else {
            if (!query.exec()) {
                gLogger->ShowSqlError("InitDB", query);
            } else {
                res = true;
            }
        }
    }
    if (res) {
        res = false;
        QSqlQuery query2("with b as (select base_name from settings_local),"
                         " c as (select name from v_users_login where login = user)"
                         " select a.base_name"
                           " || case when b.base_name is null then '' else '(' || b.base_name || ')' end"
                           " || '/' || c.name"
                           " from settings a, b, c", db);
        if (query2.lastError().isValid()) {
            gLogger->ShowSqlError("InitDB", query2);
        } else {
            if (query2.next()) {
                mBaseName = query2.value(0).toString();
                mSchemaName = aSchemaName;
                res = true;
            } else {
                gLogger->ShowError("Settings", "No data found");
            }
        }
    }
    return res;
}

void LoginDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    adjustSize();
}

bool LoginDlg::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress
            && qobject_cast<QDialog *> (obj->parent())) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if ((keyEvent->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) == (Qt::ControlModifier | Qt::ShiftModifier)
                && keyEvent->key() == 72) { // Ctrl+Shift+H

            ui->lblHost->setVisible(true);
            ui->leHost->setVisible(true);
            ui->leHost->setText(db.databaseName());

            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void LoginDlg::on_cbBase_currentIndexChanged(int index) {
    int i;

    if (!ui->cbName->currentText().isEmpty()) {
        mUser = ui->cbName->currentText();
        mLogin = ui->cbName->currentData().toString();
    }
    if (!ui->cbLang->currentText().isEmpty()) {
        mLang = ui->cbLang->currentData().toString();
    }

    ui->cbName->clear();
    QMap<QString, QMap<QString, QString>>::const_iterator itrUserList = mUserList.find(ui->cbBase->currentData().toString());
    if (itrUserList != mUserList.end() && itrUserList.key() == ui->cbBase->currentData().toString()) {
        QMap<QString, QString> lUsers = itrUserList.value();

        QMap<QString, QString>::const_iterator itr2 = lUsers.constBegin();
        while (itr2 != lUsers.constEnd()) {
            ui->cbName->addItem(itr2.key(), itr2.value());
            itr2++;
        }

        if (!mUser.isEmpty()) {
            for (i = 0; i < ui->cbName->count(); i++) {
                if (ui->cbName->itemText(i) == mUser
                        || ui->cbName->itemData(i) == mLogin) {
                    ui->cbName->setCurrentIndex(i);
                    break;
                }
            }
        }
    }

    ui->cbLang->clear();
    QMap<QString, QMap<QString, QString>>::const_iterator itrLangList = mLangList.find(ui->cbBase->currentData().toString());
    if (itrLangList != mLangList.end() && itrLangList.key() == ui->cbBase->currentData().toString()) {
        QMap<QString, QString> lLangs = itrLangList.value();

        QMap<QString, QString>::const_iterator itr2 = lLangs.constBegin();
        while (itr2 != lLangs.constEnd()) {
            ui->cbLang->addItem(itr2.key(), itr2.value());
            itr2++;
        }

        if (!mLang.isEmpty()) {
            for (i = 0; i < ui->cbLang->count(); i++) {
                if (ui->cbLang->itemData(i) == mLang) {
                    ui->cbLang->setCurrentIndex(i);
                    break;
                }
            }
        }
    }

    if (!ui->cbLang->count()) {
        ui->lblLang->setVisible(false);
        ui->cbLang->setVisible(false);
    } else if (ui->cbLang->count() == 1) {
        ui->cbLang->setCurrentIndex(0);
        ui->lblLang->setVisible(false);
        ui->cbLang->setVisible(false);
    } else {
        ui->lblLang->setVisible(true);
        ui->cbLang->setVisible(true);
    }
}

void LoginDlg::on_buttonBox_accepted() {
    db.setUserName(ui->cbName->currentData().toString());
    db.setPassword(ui->lePass->text());

    if (!db.open()) {
        gLogger->ShowSqlError(this, "Login", db);
        ui->lePass->setFocus();
        ui->lePass->selectAll();
    } else {
        if (InitDB(ui->cbBase->currentData().toString())) {
            QSettings lSettings;

            lSettings.beginGroup("LastLogin");
            lSettings.setValue("Username", ui->cbName->currentText());
            lSettings.setValue("Schema", ui->cbBase->currentData().toString());
            lSettings.setValue("Lang", ui->cbLang->currentData().toString());

            const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment();

            lSettings.setValue("ProjLogin", env.value("PROJ_LOGIN"));
            lSettings.setValue("ProjPass", env.value("PROJ_PASS"));
            lSettings.setValue("ProjHost", env.value("PROJ_HOSTSTRING"));
            lSettings.endGroup();

            accept();
        }
    }
}

void LoginDlg::on_leHost_editingFinished() {
    db.setDatabaseName(ui->leHost->text());

    if (InitByHost()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString lCurSchema = env.value("PROJ_LOGIN");
        if (!lCurSchema.isEmpty()) {
            for (int i = 0; i < ui->cbBase->count(); i++) {
                if (ui->cbBase->itemData(i).toString() == lCurSchema) {
                    ui->cbBase->setCurrentIndex(i);
                    break;
                }
            }
        }
    } else {
    }
}

QString LoginDlg::Lang() {
    return ui->cbLang->currentData().toString();
}

QString LoginDlg::SchemaName() {
    return mSchemaName;
}

QString LoginDlg::BaseName() {
    return mBaseName;
}

