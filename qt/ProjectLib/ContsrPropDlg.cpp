#include "ContsrPropDlg.h"
#include "ui_ContsrPropDlg.h"

#include "ProjectData.h"

#include "../VProject/GlobalSettings.h"

//#include "../VProject/CustomerData.h"
//#include "../VProject/UpdateThread.h"
//#include "../VProject/MainWindow.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"

ConstrPropDlg::ConstrPropDlg(eProps, ProjectData *aProject, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ContsrPropDlg),
    mJustStarted(true),
    mProject(aProject), mParentProject(NULL)
{
    InitInConstructor();
}

ConstrPropDlg::ConstrPropDlg(eNew aDummy, ProjectData *aParentProject, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ContsrPropDlg),
    mJustStarted(true),
    mProject(NULL), mParentProject(aParentProject)
{
    while (mParentProject->Parent() && mParentProject->Parent()->Type() == ProjectData::PDProject) mParentProject = mParentProject->Parent();
    InitInConstructor();
}


ConstrPropDlg::~ConstrPropDlg()
{
    delete ui;
}

int ConstrPropDlg::ProjectId() const {
    if (mProject)
        return mProject->Id();
    else
        return 0;
}

void ConstrPropDlg::InitInConstructor() {
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    mPaletteNorm = ui->leId->palette();
    // required
    mPaletteReq = mPaletteNorm;
    mPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);

    // read only
    mPaletteDis = mPaletteNorm;
    mPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);
    ui->leId->setPalette(mPaletteDis);

    ui->leCrDate->setPalette(mPaletteDis);
    ui->leCrUser->setPalette(mPaletteDis);
    ui->leEndDate->setPalette(mPaletteDis);
    ui->leEndUser->setPalette(mPaletteDis);
}

void ConstrPropDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        ShowData();
        mJustStarted = false;
    }
}

void ConstrPropDlg::ShowData() {
    if (mProject) {
        ui->wdId->setVisible(true);
        ui->wdCreated->setVisible(true);

        ui->leId->setText(QString::number(mProject->Id()));

        ui->leShortName->setText(mProject->ShortNameConst());
        if (gUserRight->CanUpdate("v_project", "shortname") && !mProject->Archived()) {
            ui->leShortName->setReadOnly(false);
            ui->leShortName->setPalette(mPaletteReq);
        } else {
            ui->leShortName->setReadOnly(true);
            ui->leShortName->setPalette(mPaletteDis);
        }

        ui->leNumber->setText(mProject->ShortNumConst());
        if (gUserRight->CanUpdate("v_project", "shortname") && !mProject->Archived()) {
            ui->leNumber->setReadOnly(false);
            ui->leNumber->setPalette(mPaletteNorm);
        } else {
            ui->leNumber->setReadOnly(true);
            ui->leNumber->setPalette(mPaletteDis);
        }

        //QTextCharFormat lFormat = ui->ptFullName->currentCharFormat();
        //lFormat.setLayoutDirection(Qt::RightToLeft);
        //ui->ptFullName->setCurrentCharFormat(lFormat);
        //ui->ptFullName->setLayoutDirection(Qt::RightToLeft);
        //ui->ptFullName->setStyleSheet("text-align: right;");

        ui->ptFullName->setPlainText(mProject->NameConst());
        if (gUserRight->CanUpdate("v_project", "name") && !mProject->Archived()) {
            ui->ptFullName->setReadOnly(false);
            ui->ptFullName->setPalette(mPaletteReq);
        } else {
            ui->ptFullName->setReadOnly(true);
            ui->ptFullName->setPalette(mPaletteDis);
        }

        ui->ptComments->setPlainText(mProject->CommentsConst());
        if (gUserRight->CanUpdate("v_project", "comments") && !mProject->Archived()) {
            ui->ptComments->setReadOnly(false);
            ui->ptComments->setPalette(mPaletteNorm);
        } else {
            ui->ptComments->setReadOnly(true);
            ui->ptComments->setPalette(mPaletteDis);
        }

        ui->leCrDate->setText(mProject->StartDateConst().toString("dd.MM.yy"));
        ui->leCrUser->setText(gUsers->GetName(mProject->CrUserConst()));

        if (!mProject->EndDateConst().isNull()) {
            ui->leEndDate->setText(mProject->EndDateConst().toString("dd.MM.yy"));
            ui->leEndUser->setText(gUsers->GetName(mProject->EndUserConst()));
            ui->wdArchive->setVisible(true);
        } else {
            ui->wdArchive->setVisible(false);
        }
    } else {
        if (gUserRight->CanInsertAnyColumn("v_project")) {
            ui->wdId->setVisible(false);
            ui->wdCreated->setVisible(false);
            ui->wdArchive->setVisible(false);

            ui->leShortName->setReadOnly(false);
            ui->leShortName->setPalette(mPaletteReq);

            ui->ptFullName->setReadOnly(false);
            ui->ptFullName->setPalette(mPaletteReq);
        } else {
            QTimer::singleShot(0, this, SLOT(close()));
        }
    }
}

void ConstrPropDlg::Accept() {
    if (ui->leShortName->text().trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("Construction"), tr("Short name must be specified!"));
        ui->leShortName->setFocus();
        return;
    }

    if (ui->ptFullName->toPlainText().trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("Construction"), tr("Full name must be specified!"));
        ui->ptFullName->setFocus();
        return;
    }

    bool lIsNew = !mProject;
    if (lIsNew) {
        int lNewId;
        if (gOracle->GetSeqNextVal("project_id_seq", lNewId)) {
            mProject = new ProjectData(lNewId, ProjectData::PDProject);
            mProject->setIdParentProject(mParentProject->Id());
        } else {
            return;
        }
    }

    mProject->ShortNameRef() = ui->leShortName->text().trimmed();
    mProject->ShortNumRef() = ui->leNumber->text().trimmed();

    mProject->NameRef() = ui->ptFullName->toPlainText().trimmed();
    mProject->CommentsRef() = ui->ptComments->toPlainText().trimmed();

    if (mProject->SaveData()) {
        mProject->CommitEdit();
        if (lIsNew) {
            mProject->RefreshData();
            mProject->setParent(mParentProject);
        }
        // update list after return - so need not update twice, byt the caller already knows ID before update
        QTimer::singleShot(0, gProjects, SLOT(ProjectListNeedUpdate()));
        accept();
    } else {
        if (!lIsNew) {
            mProject->RollbackEdit();
        } else {
            delete mProject;
            mProject = NULL;
        }
    }
}
