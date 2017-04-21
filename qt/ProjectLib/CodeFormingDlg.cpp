#include "CodeFormingDlg.h"
#include "ui_CodeFormingDlg.h"

#include <QPushButton>

#include "ProjectData.h"
#include "ProjectTypeData.h"

#include "../VProject/common.h"
#include "../UsersDlg/UserRight.h"

CodeFormingDlg::CodeFormingDlg(ProjectData *aProject, QWidget *parent) :
    QFCDialog(parent, false),
    mProject(aProject),
    ui(new Ui::CodeFormingDlg)
{
    int i;

    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    ui->cbProjType->addItem("", 0);

    const QList<ProjectTypeData *> &lProjTypes = gProjectTypes->ProjTypeListConst();

    for (i = 0; i < lProjTypes.length(); i++) {
        if (lProjTypes.at(i)->Id()) {
            ui->cbProjType->addItem(lProjTypes.at(i)->TypeNameConst(), lProjTypes.at(i)->Id());
        }
    }

    for (i = 0; i < ui->cbProjType->count(); i++) {
        if (mProject->IdProjType() == ui->cbProjType->itemData(i).toInt()) {
            ui->cbProjType->setCurrentIndex(i);
            break;
        }
    }

    if (mProject->SheetDigits() == -1) {
        ui->cbUseSheet->setChecked(false);
        ui->sbSheetDigits->setEnabled(false);
    } else {
        ui->cbUseSheet->setChecked(true);
        ui->sbSheetDigits->setEnabled(true);
        ui->sbSheetDigits->setValue(mProject->SheetDigits());
    }
    ui->leCodeTemplate->setText(mProject->CodeTemplateConst());

    bool b1 = gUserRight->CanUpdate("v_project", "sheet_digits");

    ui->cbUseSheet->setReadOnly(!b1);
    ui->sbSheetDigits->setReadOnly(!b1);

    bool b2 = gUserRight->CanUpdate("v_project", "code_template");
    ui->leCodeTemplate->setReadOnly(!b2);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(b1 || b2);
}

CodeFormingDlg::~CodeFormingDlg()
{
    delete ui;
}

void CodeFormingDlg::on_cbUseSheet_toggled(bool checked) {
    ui->sbSheetDigits->setEnabled(checked);

    QString lCodeTempl = ui->leCodeTemplate->text();
    if (!lCodeTempl.isEmpty()) {
        if (checked) {
            // replace %N..N% to %SHEET%
            QString lN = lCodeTempl;
            lN.replace(QRegExp("^.*%N"), "%N").replace(QRegExp("N%.*$"), "N%");
            ui->sbSheetDigits->setValue(lN.length() - 2);

            lCodeTempl.replace(QRegExp("%N*%"), "%SHEET%");
        } else {
            // replace %SHEET% to %N..N%
            QString lN = "N";
            while (lN.length() < ui->sbSheetDigits->value()) lN += "N";

            lCodeTempl.replace("%SHEET%", "%" + lN + "%");
        }
        ui->leCodeTemplate->setText(lCodeTempl);
    }
}

void CodeFormingDlg::Accept() {
    bool lIsChanged = false;
    int lSheetDigits;

    if (ui->cbUseSheet->isChecked()) {
        lSheetDigits = ui->sbSheetDigits->value();
    } else {
        lSheetDigits = -1;
    }

    if (mProject->IdProjType() != ui->cbProjType->currentData().toInt()) {
        mProject->setIdProjType(ui->cbProjType->currentData().toInt());
        lIsChanged = true;
    }
    if (mProject->SheetDigits() != lSheetDigits) {
        mProject->setSheetDigits(lSheetDigits);
        lIsChanged = true;
    }
    if (mProject->CodeTemplateConst() != ui->leCodeTemplate->text().trimmed()) {
        mProject->CodeTemplateRef() = ui->leCodeTemplate->text().trimmed();
        lIsChanged = true;
    }

    if (lIsChanged) {
        if (mProject->SaveData()) {
            mProject->CommitEdit();
            accept();
        } else {
            mProject->RollbackEdit();
        }
    } else {
        accept();
    }
}

void CodeFormingDlg::on_cbProjType_currentIndexChanged(int index) {
    const ProjectTypeData *lType = gProjectTypes->GetById(ui->cbProjType->currentData().toInt());

    if (lType && !lType->DefTemplConst().isEmpty()) {
        ui->leCodeTemplate->setText(lType->DefTemplConst());

        emit ui->cbUseSheet->toggled(ui->cbUseSheet->isChecked());
    }
}
