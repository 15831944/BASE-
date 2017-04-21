#include "PublishDlg.h"
#include "ui_PublishDlg.h"

#include "common.h"

#include <QFileDialog>
#include <QPushButton>

#define NOMINMAX
#include <windows.h>
#include <winspool.h>

#include "GlobalSettings.h"

PublishDlg::PublishDlg(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::PublishDlg)
{
    void *lBuf;
    PRINTER_INFO_2 lPI1, *lpPI1;
    int j;
    DWORD lNeeded, lCount, i;

    ui->setupUi(this);

    setWindowFlags(Qt::WindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint));

    ui->cbUseVersion->addItem("Always ask in AutoCAD", -1);
    ui->cbUseVersion->addItem("Old versions (recommended)", 0);
    ui->cbUseVersion->addItem("New versions", 1);
    ui->cbUseVersion->addItem("Without xrefs (FOR TESTING ONLY)", 2);

    for (j = 0; j < ui->cbUseVersion->count(); j++) {
        if (ui->cbUseVersion->itemData(j).toInt() == gSettings->Publish.UseVersion) {
            ui->cbUseVersion->setCurrentIndex(j);
            break;
        }
    }

    QStringList lPlotStyles;
    gSettings->EnumPlotStyles(lPlotStyles);

    ui->cbPlotStyles->addItem("<Auto>", (int) 0);
    ui->cbPlotStyles->addItem("<As saved>", (int) 1);
    for (int i = 0; i < lPlotStyles.length(); i++) {
        ui->cbPlotStyles->addItem(lPlotStyles.at(i), lPlotStyles.at(i));
    }

    if (gSettings->Publish.CTBType < 2) {
        ui->cbPlotStyles->setCurrentIndex(gSettings->Publish.CTBType);
    } else {
        ui->cbPlotStyles->setCurrentText(gSettings->Publish.CTBName);
    }

    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, (LPBYTE) &lPI1, sizeof(lPI1), &lNeeded, &lCount);
    lBuf = malloc(lNeeded);
    if (EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, (LPBYTE) lBuf, lNeeded, &lNeeded, &lCount)) {
        lpPI1 = (PRINTER_INFO_2 *) lBuf;
        for (i = 0; i < lCount; i++) {
            QString lPrinterName = QString::fromWCharArray(lpPI1->pPrinterName);
            if (!_wcsicmp(lpPI1->pPrinterName, lpPI1->pDriverName)) {
                ui->cbDevices->addItem(lPrinterName, lPrinterName);
            } else {
                ui->cbDevices->addItem(lPrinterName + ": " + QString::fromWCharArray(lpPI1->pDriverName), lPrinterName);
            }
            lpPI1++;
        }
    }

    ui->cbPDF->setChecked(gSettings->Publish.PDF);
    ui->cbDWF->setChecked(gSettings->Publish.DWF);
    ui->cbPLT->setChecked(gSettings->Publish.PLT);
    ui->cbNotScale->setChecked(gSettings->Publish.DontScale);
    for (j = 0; j < ui->cbDevices->count(); j++) {
        if (ui->cbDevices->itemData(j).toString() == gSettings->Publish.PlotterName) {
            ui->cbDevices->setCurrentIndex(j);
            break;
        }
    }
    ui->cbDevices->setEnabled(gSettings->Publish.PLT);

    ui->leDirName->setText(gSettings->SaveFiles.LastDir);
}

PublishDlg::~PublishDlg()
{
    delete ui;
}

void PublishDlg::Accept() {
    if (ui->leDirName->text().isEmpty()) {
        on_tbTreeSel_2_clicked();
    }

    while (!ui->leDirName->text().isEmpty()
           && ui->leDirName->text().endsWith('/'))
        ui->leDirName->setText(ui->leDirName->text().left(ui->leDirName->text().length() - 1));


    if (ui->leDirName->text().isEmpty()) {
        QMessageBox::critical(this, tr("Publish"), tr("Output directory name must be specified!"));
        return;
    }

    gSettings->SaveFiles.LastDir = ui->leDirName->text();

    QDir lDir(ui->leDirName->text());

    if (!lDir.exists()) {
        if (!lDir.mkpath(ui->leDirName->text())) {
            QMessageBox::critical(this, tr("Publish"), tr("Can't create directory") + "\n" + ui->leDirName->text());
            return;
        }
    }

    switch (ui->cbPlotStyles->currentData().type()) {
    case QMetaType::Int:
        gSettings->Publish.CTBType = ui->cbPlotStyles->currentData().toInt();
        break;
    case QMetaType::QString:
        gSettings->Publish.CTBType = 2;
        gSettings->Publish.CTBName = ui->cbPlotStyles->currentText();
        break;
    }

    gSettings->Publish.PDF = ui->cbPDF->isChecked();
    gSettings->Publish.DWF = ui->cbDWF->isChecked();
    gSettings->Publish.PLT = ui->cbPLT->isChecked();
    gSettings->Publish.DontScale = ui->cbNotScale->isChecked();
    gSettings->Publish.UseVersion = ui->cbUseVersion->currentData().toInt();
    gSettings->Publish.PlotterName = ui->cbDevices->currentData().toString();
    gSettings->Publish.OutDir = ui->leDirName->text();

    accept();
}

void PublishDlg::CheckAnySelected(int aDummy) {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->cbPDF->isChecked()
                                               || ui->cbDWF->isChecked()
                                               || ui->cbPLT->isChecked());
}

void PublishDlg::on_cbPLT_toggled(bool checked) {
    ui->cbDevices->setEnabled(checked);
}

void PublishDlg::on_tbTreeSel_2_clicked() {
    QFileDialog dlg;

    dlg.setAcceptMode(QFileDialog::AcceptOpen);

    dlg.setFileMode(QFileDialog::DirectoryOnly);

    // recommended - not work as usual
    //dlg.setFileMode(QFileDialog::Directory);
    //dlg.setOption(QFileDialog::ShowDirsOnly, true);

    dlg.setDirectory(ui->leDirName->text());
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        gSettings->SaveFiles.LastDir = files.at(0);
        ui->leDirName->setText(files.at(0));
    }
}
