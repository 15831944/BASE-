#include "PreloadParamsDlg.h"
#include "ui_PreloadParamsDlg.h"

#include "AcadExchange.h"

#include <QMessageBox>

PreloadParamsDlg::PreloadParamsDlg(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::PreloadParamsDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    FillLineWeights(ui->listLWBlocks);
    FillLineWeights(ui->listLWEntities);

    // 0 - By block
    // 256 - By layer
    ui->leColorBlocks->setValidator(new QIntValidator(0, 256, this));
    ui->leColorEntities->setValidator(new QIntValidator(1, 256, this));

    on_pbDefaults_clicked();

    QSettings settings;
    settings.beginGroup("GlobalSettings");
    settings.beginGroup("PreloadParams");

    ui->cbRemoveInvisible->setChecked(settings.value("cbRemoveInvisible", ui->cbRemoveInvisible->isChecked()).toBool());

    ui->cbColorBlocks->setChecked(settings.value("cbColorBlocks", ui->cbColorBlocks->isChecked()).toBool());
    ui->leColorBlocks->setEnabled(ui->cbColorBlocks->isChecked());
    ui->leColorBlocks->setText(settings.value("leColorBlocks", ui->leColorBlocks->text()).toString());

    ui->cbColorEntities->setChecked(settings.value("cbColorEntities", ui->cbColorEntities->isChecked()).toBool());
    ui->leColorEntities->setEnabled(ui->cbColorEntities->isChecked());
    ui->leColorEntities->setText(settings.value("leColorEntities", ui->leColorEntities->text()).toString());

    ui->cbLWBlocks->setChecked(settings.value("cbLWBlocks", ui->cbLWBlocks->isChecked()).toBool());
    ui->listLWBlocks->setEnabled(ui->cbLWBlocks->isChecked());
    ui->listLWBlocks->setCurrentText(settings.value("listLWBlocks", ui->listLWBlocks->currentText()).toString());

    ui->cbLWEntities->setChecked(settings.value("cbLWEntities", ui->cbLWEntities->isChecked()).toBool());
    ui->listLWEntities->setEnabled(ui->cbLWEntities->isChecked());
    ui->listLWEntities->setCurrentText(settings.value("listLWEntities", ui->listLWEntities->currentText()).toString());

    ui->cbRename0->setChecked(settings.value("cbRename0", ui->cbRename0->isChecked()).toBool());
    ui->leLayer0->setEnabled(ui->cbRename0->isChecked());
    ui->leLayer0->setText(settings.value("leLayer0", ui->leLayer0->text()).toString());

    ui->cbRemoveXrefs->setChecked(settings.value("cbRemoveXrefs", ui->cbRemoveXrefs->isChecked()).toBool());
    ui->cbClearAnnoScales->setChecked(settings.value("cbClearAnnoScales", ui->cbClearAnnoScales->isChecked()).toBool());

    ui->cbPurgeRegApps->setChecked(settings.value("cbPurgeRegApps", ui->cbPurgeRegApps->isChecked()).toBool());
    ui->cbPurgeAll->setChecked(settings.value("cbPurgeAll", ui->cbPurgeAll->isChecked()).toBool());
    ui->cbExplodeProxy->setChecked(settings.value("cbExplodeProxy", ui->cbExplodeProxy->isChecked()).toBool());
    ui->cbRemoveProxy->setChecked(settings.value("cbRemoveProxy", ui->cbRemoveProxy->isChecked()).toBool());
    ui->cbAudit->setChecked(settings.value("cbAudit", ui->cbAudit->isChecked()).toBool());
    ui->cbSwitchToModel->setChecked(settings.value("cbSwitchToModel", ui->cbSwitchToModel->isChecked()).toBool());
    ui->cbSetWCS->setChecked(settings.value("cbSetWCS", ui->cbSetWCS->isChecked()).toBool());
    ui->cbViewWCS->setChecked(settings.value("cbViewWCS", ui->cbViewWCS->isChecked()).toBool());
    ui->cbZoomE->setChecked(settings.value("cbZoomE", ui->cbZoomE->isChecked()).toBool());

    ui->cbUserCommands->setChecked(settings.value("cbUserCommands", ui->cbUserCommands->isChecked()).toBool());
    ui->ptUserCommands->setEnabled(ui->cbUserCommands->isChecked());
    ui->ptUserCommands->setPlainText(settings.value("ptUserCommands", ui->ptUserCommands->toPlainText()).toString());

    ui->cbManualClose->setChecked(settings.value("cbManualClose", ui->cbManualClose->isChecked()).toBool());
    ui->cbRestartAfterEach->setChecked(settings.value("cbRestartAfterEach", ui->cbRestartAfterEach->isChecked()).toBool());

    settings.endGroup();
    settings.endGroup();
}

PreloadParamsDlg::~PreloadParamsDlg()
{
    delete ui;
}

ULONG PreloadParamsDlg::ProcessType() {
    return
            (ui->cbRemoveInvisible->isChecked()?lpRemoveInvisible:0)
            | (ui->cbColorBlocks->isChecked()?lpColorBlocks:0)
            | (ui->cbColorEntities->isChecked()?lpColorEntities:0)
            | (ui->cbLWBlocks->isChecked()?lpLWBlocks:0)
            | (ui->cbLWEntities->isChecked()?lpLWEntities:0)
            | (ui->cbRename0->isChecked()?lpRenameLayer0:0)
            | (ui->cbRemoveXrefs->isChecked()?lpRemoveAllXrefs:0)

            | (ui->cbClearAnnoScales->isChecked()?lpClearAnnoScales:0)
            | (ui->cbPurgeRegApps->isChecked()?lpPurgeRegapps:0)
            | (ui->cbPurgeAll->isChecked()?lpPurgeAll:0)
            | (ui->cbExplodeProxy->isChecked()?lpExplodeAllProxies:0)
            | (ui->cbRemoveProxy->isChecked()?lpRemoveAllProxies:0)
            | (ui->cbAudit->isChecked()?lpAudit:0)

            | (ui->cbSwitchToModel->isChecked()?lpSwitchToModel:0)
            | (ui->cbSetWCS->isChecked()?lpSetWCS:0)
            | (ui->cbViewWCS->isChecked()?lpViewWCS:0)
            | (ui->cbZoomE->isChecked()?lpZoomE:0)

            | (ui->cbUserCommands->isChecked()?lpUserCommands:0)
            | (ui->cbManualClose->isChecked()?lpManualClose:0)
            | (ui->cbRestartAfterEach->isChecked()?lpRestartAfterEach:0)
            ;
}

ULONG PreloadParamsDlg::ColorBlocks() {
    return ui->leColorBlocks->text().toULong();
}

ULONG PreloadParamsDlg::ColorEntities() {
    return ui->leColorEntities->text().toULong();
}

ULONG PreloadParamsDlg::LWBlocks() {
    return ui->listLWBlocks->currentData().toUInt();
}

ULONG PreloadParamsDlg::LWEntities() {
    return ui->listLWEntities->currentData().toUInt();
}

QString PreloadParamsDlg::Layer0Name() {
    if (ui->cbRename0->isChecked())
        return ui->leLayer0->text();
    else
        return "";
}

QString PreloadParamsDlg::UserCommands() {
    if (ui->cbUserCommands->isChecked()) {
        QString s = ui->ptUserCommands->toPlainText();
        if (s.at(s.length() - 1).toLatin1() != 10) s += QChar(10);
        return s;
    } else
        return "";
}


void PreloadParamsDlg::on_pbOK_clicked() {
    if (ui->cbColorBlocks->isChecked()
            && ui->leColorBlocks->text().isEmpty()) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("Color must be specified!"));
        ui->leColorBlocks->setFocus();
        return;
    }

    if (ui->cbColorBlocks->isChecked()
            && (ui->leColorBlocks->text().toInt() < 0 || ui->leColorBlocks->text().toInt() > 256)) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("Color must be between 0 and 256!"));
        ui->leColorBlocks->setFocus();
        return;
    }

    if (ui->cbColorEntities->isChecked()
            && ui->leColorEntities->text().isEmpty()) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("Color must be specified!"));
        ui->leColorEntities->setFocus();
        return;
    }

    if (ui->cbColorEntities->isChecked()
            && (ui->leColorEntities->text().toInt() < 1 || ui->leColorEntities->text().toInt() > 256)) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("Color must be between 1 and 256!"));
        ui->leColorEntities->setFocus();
        return;
    }

    if (ui->cbRename0->isChecked()
            && ui->leLayer0->text().isEmpty()) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("Layer name must be specified!"));
        ui->leLayer0->setFocus();
        return;
    }

    if (ui->cbRename0->isChecked()
            && ui->leLayer0->text().length() > 36) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("Layer name too long!"));
        ui->leLayer0->setFocus();
        return;
    }

    if (ui->cbUserCommands->isChecked()
            && ui->ptUserCommands->toPlainText().isEmpty()) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("User commands must be specified!"));
        ui->ptUserCommands->setFocus();
        return;
    }

    if (ui->cbUserCommands->isChecked()
            && ui->ptUserCommands->toPlainText().length() > 1024) {
        QMessageBox::critical(this, tr("Preload parameters"), tr("User commands too long!"));
        ui->ptUserCommands->setFocus();
        return;
    }

    accept();
}

void PreloadParamsDlg::FillLineWeights(QComboBox *aListLW) {
    aListLW->clear();

    // just a predifined list from ObjectARX AcDb::LineWeight enum

    //aCC.SetItemData(aCC.AddString(VARIES), -200);
    //aCC.SetItemData(aCC.AddString(L"Not specified"), -100);
    aListLW->addItem("By layer", -1);
    aListLW->addItem("By block", -2);
    //aCC.SetItemData(aCC.AddString(L"By lw default"), -3);

    aListLW->addItem("0.00", 0);
    aListLW->addItem("0.05", 5);
    aListLW->addItem("0.09", 9);
    aListLW->addItem("0.13", 13);
    aListLW->addItem("0.15", 15);
    aListLW->addItem("0.18", 18);
    aListLW->addItem("0.20", 20);
    aListLW->addItem("0.25", 25);
    aListLW->addItem("0.30", 30);
    aListLW->addItem("0.35", 35);
    aListLW->addItem("0.40", 40);
    aListLW->addItem("0.50", 50);
    aListLW->addItem("0.53", 53);
    aListLW->addItem("0.60", 60);
    aListLW->addItem("0.70", 70);
    aListLW->addItem("0.80", 80);
    aListLW->addItem("0.90", 90);
    aListLW->addItem("1.00", 100);
    aListLW->addItem("1.06", 106);
    aListLW->addItem("1.20", 120);
    aListLW->addItem("1.40", 140);
    aListLW->addItem("1.58", 158);
    aListLW->addItem("2.00", 200);
    aListLW->addItem("2.11", 211);
}

void PreloadParamsDlg::on_pbDefaults_clicked() {
    ui->cbPurgeRegApps->setEnabled(false);
    ui->cbAudit->setEnabled(false);

    ui->cbRemoveInvisible->setChecked(true);
    ui->cbColorBlocks->setChecked(false);
    ui->leColorBlocks->setEnabled(false);
    ui->leColorBlocks->setText("0");

    ui->cbColorEntities->setChecked(false);
    ui->leColorEntities->setEnabled(false);
    ui->leColorEntities->setText("256");

    ui->cbLWBlocks->setChecked(false);
    ui->listLWBlocks->setEnabled(false);
    ui->listLWBlocks->setCurrentText("By block");

    ui->cbLWEntities->setChecked(false);
    ui->listLWEntities->setEnabled(false);
    ui->listLWEntities->setCurrentText("By layer");

    ui->cbRename0->setChecked(true);
    ui->leLayer0->setEnabled(true);
    ui->leLayer0->setText("0Layer");

    ui->cbRemoveXrefs->setChecked(false);
    ui->cbClearAnnoScales->setChecked(true);

    ui->cbPurgeRegApps->setChecked(true);
    ui->cbPurgeAll->setChecked(true);
    ui->cbExplodeProxy->setChecked(true);
    ui->cbRemoveProxy->setChecked(true);
    ui->cbAudit->setChecked(true);
    ui->cbSwitchToModel->setChecked(false);
    ui->cbSetWCS->setChecked(false);
    ui->cbViewWCS->setChecked(false);
    ui->cbZoomE->setChecked(false);

    ui->cbUserCommands->setChecked(false);
    ui->ptUserCommands->setEnabled(false);
    ui->cbManualClose->setChecked(false);
    ui->cbRestartAfterEach->setChecked(false);
}

void PreloadParamsDlg::on_cbColorBlocks_toggled(bool checked) {
    ui->leColorBlocks->setEnabled(checked);
}

void PreloadParamsDlg::on_cbColorEntities_toggled(bool checked) {
    ui->leColorEntities->setEnabled(checked);
}

void PreloadParamsDlg::on_cbLWBlocks_toggled(bool checked) {
    ui->listLWBlocks->setEnabled(checked);
}

void PreloadParamsDlg::on_cbLWEntities_toggled(bool checked) {
    ui->listLWEntities->setEnabled(checked);
}

void PreloadParamsDlg::on_cbRename0_toggled(bool checked)
{
    ui->leLayer0->setEnabled(checked);
}


void PreloadParamsDlg::on_cbUserCommands_toggled(bool checked)
{
    ui->ptUserCommands->setEnabled(checked);
}
