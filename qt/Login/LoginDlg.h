#ifndef LOGINDLG_H
#define LOGINDLG_H

//#include "../VProject/qfcdialog.h"

#include <QDialog>

#define QFCDialog QDialog

#include <QList>
#include <QMap>

namespace Ui {
class LoginDlg;
}

class LoginDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit LoginDlg(QWidget *parent = 0);
    ~LoginDlg();

    QString Lang();
    QString SchemaName();
    QString BaseName();
protected:
    QString mUser, mLogin, mLang;
    QString mSchemaName, mBaseName;

    QMap<QString, QMap<QString, QString>> mUserList, mLangList;

    bool InitByHost();
    bool InitDB(const QString &aSchemaName);

    virtual void showEvent(QShowEvent* event);
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void on_cbBase_currentIndexChanged(int index);

    void on_buttonBox_accepted();

    void on_leHost_editingFinished();

private:
    Ui::LoginDlg *ui;
};

#endif // LOGINDLG_H
