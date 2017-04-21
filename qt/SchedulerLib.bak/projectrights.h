#ifndef PROJECTRIGHTS_H
#define PROJECTRIGHTS_H

#include "../Login/Login.h"
#include "../VProject/qfcdialog.h"

#include <QMenu>
#include <QMenuBar>

class UserData;

namespace Ui {
class ProjectRights;
}


class ProjectRights : public QFCDialog
{
    Q_OBJECT

public:
    explicit ProjectRights(QWidget *parent = 0);
    virtual ~ProjectRights(){}

private slots:
    void on_le_IdProject_editingFinished();

    void on_tb_Projects_clicked();

    void on_tb_select_plus_clicked();

    void on_tb_select_minus_clicked();

    void on_btn_OK_clicked();

    void on_btn_Cancel_clicked();

    void on_tV_listUsers_customContextMenuRequested(const QPoint &pos);

private:
    Ui::ProjectRights *ui;
    bool mCancelled;
};

#endif // PROJECTRIGHTS_H
