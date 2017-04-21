#include "wdSettingsImage.h"
#include "ui_wdSettingsImage.h"

#include "../VProject/GlobalSettings.h"

#include <QFileDialog>

wdSettingsImage::wdSettingsImage(QWidget *parent) :
    wdSettings(parent),
    ui(new Ui::wdSettingsImage),
    mJustStarted(true)
{
    ui->setupUi(this);
}

wdSettingsImage::~wdSettingsImage()
{
    delete ui;
}

void wdSettingsImage::showEvent(QShowEvent* event) {
    wdSettings::showEvent(event);

    if (mJustStarted) {
        ui->rbXY->blockSignals(true);
        ui->sbPercent->blockSignals(true);
        ui->rbExtSpecified->blockSignals(true);
        ui->rbInternal->blockSignals(true);
        ui->rbExtDefault->blockSignals(true);
        ui->rbExtSpecified->blockSignals(true);
        ui->cbExtViewer->blockSignals(true);
        ui->rbMSPaint->blockSignals(true);
        ui->rbEditorSpecified->blockSignals(true);

        ui->cbFNExists->setCurrentIndex(gSettings->Image.LoadWhenFNExists);

        ui->gbResizeForPreview->setChecked(gSettings->Image.ResizeForPreview == 1);
        ui->leMaxPreviewWidth->setText(QString::number(gSettings->Image.MaxPreviewWidth));
        ui->leMaxPreviewHeight->setText(QString::number(gSettings->Image.MaxPreviewHeight));
        ui->cbDblClickShow->setCurrentIndex(gSettings->Image.OnPreviewDblClick);

        bool lIsXY = !gSettings->Image.ConvertType;
        if (lIsXY) {
            ui->rbXY->setChecked(true);
        } else {
            ui->rbPercent->setChecked(true);
        }

        ui->leMaxWidth->setEnabled(lIsXY);
        ui->leMaxHeight->setEnabled(lIsXY);
        ui->sbPercent->setEnabled(!lIsXY);

        ui->leMaxWidth->setText(QString::number(gSettings->Image.MaxConvertWidth));
        ui->leMaxHeight->setText(QString::number(gSettings->Image.MaxConvertHeight));
        ui->sbPercent->setValue(gSettings->Image.ConvertPercent);

        ui->leMaxFileSize->setText(QString::number(gSettings->Image.MaxFileSize));

        // viewer
        switch (gSettings->Image.ViewerType) {
        case 0:
            ui->rbInternal->setChecked(true);
            ui->cbExtViewer->setEnabled(false);
            ui->tbSelectViewer->setEnabled(false);
            ui->cbSaveAll->setEnabled(false);
            ui->cbConfirmClosed->setEnabled(false);
            break;
        case 1:
            ui->rbExtDefault->setChecked(true);
            ui->cbExtViewer->setEnabled(false);
            ui->tbSelectViewer->setEnabled(false);
            ui->cbSaveAll->setEnabled(true);
            ui->cbConfirmClosed->setEnabled(true);
            break;
        case 2:
            ui->rbExtSpecified->setChecked(true);
            ui->cbExtViewer->setEnabled(true);
            ui->tbSelectViewer->setEnabled(true);
            ui->cbSaveAll->setEnabled(true);
            ui->cbConfirmClosed->setEnabled(true);
            break;
        }

        ui->cbExtViewer->setCurrentText(gSettings->Image.ViewerPath);
        ui->cbSaveAll->setChecked(gSettings->Image.SaveAll);
        ui->cbConfirmClosed->setChecked(gSettings->Image.ConfirmViewerClosed);

        // editor
        switch (gSettings->Image.EditorType) {
        case 0:
            ui->rbMSPaint->setChecked(true);
            ui->leExtEditor->setEnabled(false);
            ui->tbSelectEditor->setEnabled(false);
            ui->cbSaveAllForEdit->setEnabled(false);
            ui->cbConfirmEditorClosed->setEnabled(false);
            break;
        case 1:
            ui->rbEditorSpecified->setChecked(true);
            ui->leExtEditor->setEnabled(true);
            ui->tbSelectEditor->setEnabled(true);
            ui->cbSaveAllForEdit->setEnabled(true);
            ui->cbConfirmEditorClosed->setEnabled(true);
            break;
        }

        ui->leExtEditor->setText(gSettings->Image.EditorPath);
        ui->cbSaveAllForEdit->setChecked(gSettings->Image.SaveAllForEditor);
        ui->cbConfirmEditorClosed->setChecked(gSettings->Image.ConfirmEditorClosed);

        ui->rbXY->blockSignals(false);
        ui->sbPercent->blockSignals(false);
        ui->rbExtSpecified->blockSignals(false);
        ui->rbInternal->blockSignals(false);
        ui->rbExtDefault->blockSignals(false);
        ui->rbExtSpecified->blockSignals(false);
        ui->cbExtViewer->blockSignals(false);
        ui->rbMSPaint->blockSignals(false);
        ui->rbEditorSpecified->blockSignals(false);

        mJustStarted = false;
    }
}

bool wdSettingsImage::DoSave() {
    if (ui->rbExtSpecified->isChecked()) {
        bool lIsOk = false;
        for (int i = 1; i < ui->cbExtViewer->count(); i++) {
            if (ui->cbExtViewer->currentText() == ui->cbExtViewer->itemText(i)) {
                lIsOk = true;
                break;
            }
        }

        if (!lIsOk) {
            if (!QFile::exists(ui->cbExtViewer->currentText())) {
                if (QMessageBox::question(this, tr("Images settings"), tr("Viewer doesn''t exist!") + "\n" + tr("Continue anyway?")) != QMessageBox::Yes) {
                    ui->cbExtViewer->setFocus();
                    return false;
                }
            }
        }
    }

    if (ui->rbEditorSpecified->isChecked()) {
        if (!QFile::exists(ui->leExtEditor->text())) {
            if (QMessageBox::question(this, tr("Images settings"), tr("Editor doesn''t exist!") + "\n" + tr("Continue anyway?")) != QMessageBox::Yes) {
                ui->leExtEditor->setFocus();
                return false;
            }
        }
    }

    gSettings->Image.LoadWhenFNExists = ui->cbFNExists->currentIndex();

    gSettings->Image.ResizeForPreview = ui->gbResizeForPreview->isChecked()?1:0;
    gSettings->Image.MaxPreviewWidth = ui->leMaxPreviewWidth->text().toInt();
    gSettings->Image.MaxPreviewHeight = ui->leMaxPreviewHeight->text().toInt();
    gSettings->Image.OnPreviewDblClick = ui->cbDblClickShow->currentIndex();

    gSettings->Image.ConvertType = ui->rbXY->isChecked()?0:1;

    gSettings->Image.MaxConvertWidth = ui->leMaxWidth->text().toInt();
    gSettings->Image.MaxConvertHeight = ui->leMaxHeight->text().toInt();
    gSettings->Image.ConvertPercent = ui->sbPercent->value();

    gSettings->Image.MaxFileSize = ui->leMaxFileSize->text().toUInt();

    if (ui->rbInternal->isChecked()) {
        gSettings->Image.ViewerType = 0;
    } else if (ui->rbExtDefault->isChecked()) {
        gSettings->Image.ViewerType = 1;
    } else if (ui->rbExtSpecified->isChecked()) {
        gSettings->Image.ViewerType = 2;
    }

    gSettings->Image.ViewerPath = ui->cbExtViewer->currentText();
    gSettings->Image.SaveAll = ui->cbSaveAll->isChecked();
    gSettings->Image.ConfirmViewerClosed = ui->cbConfirmClosed->isChecked();


    if (ui->rbMSPaint->isChecked()) {
        gSettings->Image.EditorType = 0;
    } else if (ui->rbEditorSpecified->isChecked()) {
        gSettings->Image.EditorType = 1;
    }

    gSettings->Image.EditorPath = ui->leExtEditor->text();
    gSettings->Image.SaveAllForEditor = ui->cbSaveAllForEdit->isChecked();
    gSettings->Image.ConfirmEditorClosed = ui->cbConfirmEditorClosed->isChecked();

    gSettings->SaveSettingsImage();

    return true;
}


void wdSettingsImage::on_gbResizeForPreview_toggled(bool arg1) {
    ui->leMaxPreviewWidth->setEnabled(arg1);
    ui->leMaxPreviewHeight->setEnabled(arg1);
    ui->cbDblClickShow->setEnabled(arg1);
}

void wdSettingsImage::on_rbXY_toggled(bool checked) {
    ui->leMaxWidth->setEnabled(checked);
    ui->leMaxHeight->setEnabled(checked);
    ui->sbPercent->setEnabled(!checked);
}

void wdSettingsImage::on_rbInternal_toggled(bool checked) {
    if (checked) {
        ui->cbExtViewer->setEnabled(false);
        ui->tbSelectViewer->setEnabled(false);
        ui->cbSaveAll->setEnabled(false);
        ui->cbConfirmClosed->setEnabled(false);
    }
}

void wdSettingsImage::on_rbExtDefault_toggled(bool checked) {
    if (checked) {
        ui->cbExtViewer->setEnabled(false);
        ui->tbSelectViewer->setEnabled(false);
        ui->cbSaveAll->setEnabled(true);
        ui->cbConfirmClosed->setEnabled(true);
    }
}

void wdSettingsImage::on_rbExtSpecified_toggled(bool checked) {
    if (checked) {
        ui->cbExtViewer->setEnabled(true);
        ui->tbSelectViewer->setEnabled(true);
        ui->cbSaveAll->setEnabled(true);
        ui->cbConfirmClosed->setEnabled(true);
    }
}

void wdSettingsImage::on_tbSelectViewer_clicked() {
    QFileDialog dlg;

    QStringList lFilters;
    lFilters.append("Executables (*.exe)");
    lFilters.append("All files (*)");
    dlg.setNameFilters(lFilters);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.fileSelected(ui->cbExtViewer->currentText());
    if (dlg.exec() == QDialog::Accepted) {
        ui->cbExtViewer->setCurrentText("\"" + dlg.selectedFiles().at(0) + "\"");
    }
}

void wdSettingsImage::on_cbExtViewer_currentIndexChanged(int index) {
    switch (index) {
    case 1:
        ui->cbSaveAll->setChecked(true);
        ui->cbConfirmClosed->setChecked(false);
        break;
    case 2:
        ui->cbSaveAll->setChecked(false);
        ui->cbConfirmClosed->setChecked(true);
        break;
    }
}

void wdSettingsImage::on_rbMSPaint_toggled(bool checked) {
    if (checked) {
        ui->leExtEditor->setEnabled(false);
        ui->tbSelectEditor->setEnabled(false);
        ui->cbSaveAllForEdit->setEnabled(false);
        ui->cbConfirmEditorClosed->setEnabled(false);
    }
}

void wdSettingsImage::on_rbEditorSpecified_toggled(bool checked) {
    if (checked) {
        ui->leExtEditor->setEnabled(true);
        ui->tbSelectEditor->setEnabled(true);
        ui->cbSaveAllForEdit->setEnabled(true);
        ui->cbConfirmEditorClosed->setEnabled(true);
    }

}

void wdSettingsImage::on_tbSelectEditor_clicked() {
    QFileDialog dlg;

    QStringList lFilters;
    lFilters.append("Executables (*.exe)");
    lFilters.append("All files (*)");
    dlg.setNameFilters(lFilters);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.fileSelected(ui->leExtEditor->text());
    if (dlg.exec() == QDialog::Accepted) {
        ui->leExtEditor->setText("\"" + dlg.selectedFiles().at(0) + "\"");
    }
}
