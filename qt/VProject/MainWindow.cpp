#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "common.h"
#include "GlobalSettings.h"

#include "QMyMdiSubWindow.h"

#include "../contract-pkz/ContractPkz.h"
#include "../WorkByDateUser/workbydateuser.h"

#include "../ProjectLib/ProjectListDlg.h"
#include "../ProjectLib/ProjectTypeDlg.h"
#include "../ProjectLib//ProjectRightsDlg.h"

#include "PlotListDlg.h"
#include "../PlotLib/PlotHistDlg.h"
#include "../PlotLib/PlotAddFileDlg.h"
#include "../PlotLib/PlotAttrDlg.h"
#include "../PlotLib/PlotProp.h"
#include "../PlotLib/PlotNewDlg.h"
#include "../PlotLib/PlotSimpleListDlg.h"
#include "../PlotLib/PlotDeletedDlg.h"
#include "../PlotLib/DwgCmpSettingsDlg.h"

#include "../SaveLoadLib/LoadImagesDlg.h"
#include "../SaveLoadLib/LISettingsDlg.h"

#include "../geobase/geobase.h"

#include "CommonSettingsDlg.h"

#include "PublishReport.h"
#include "AuditReport.h"

#include "WaitDlg.h"
#include "FindDlg.h"

#include "ChooseAcadDlg.h"
#include "DocTreeSettings.h"
#include "LoadXrefsDlg.h"

// static libraries
#include "../UsersDlg/DepartDlg.h"
#include "../UsersDlg/UsersDlg.h"
#include "../UsersDlg/UserRight.h"
#include "../UsersDlg/ChangePassDlg.h"
#include "../UsersDlg//OrganizationsListDlg.h"

#include <QLabel>
#include <QCheckBox>
#include <QWindow>
#include <QInputDialog>
#include <QCloseEvent>

#include <QStyleFactory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mJustStarted(true), mAlertThread(NULL), mTimer(NULL),
    mInLoadWindows(false)
{
    ui->setupUi(this);

    if (!gSettings->BaseName.isEmpty())
        setWindowTitle(gSettings->BaseName);

    ui->actionProject_types->setVisible(false);

    // visible-invisible menus
    ui->actionContracts->setVisible(gHasModule("contract-pkz") && gUserRight->CanSelect("v_pkz_contract"));
    ui->actionContracts_2->setVisible(gUserRight->CanSelect("v_contract"));
    ui->actionSubcontracts->setVisible(gUserRight->CanSelect("v_contract"));

    ui->actionDocuments_tree->setVisible(gUserRight->CanInsert("v_treedata"));
    ui->actionGeobases->setVisible(gHasModule("geobase") && gUserRight->CanSelect("v_geobase") && gUserRight->CanSelect("v_geobase2plot"));

    ui->menuCorrespondence->menuAction()->setVisible(gUserRight->CanSelect("v_letters2"));

    ui->actionSaved_documents->setVisible(gUserRight->CanSelect("v_saved_docs"));
    ui->actionRestrictions_on_savings->setVisible(gUserRight->CanSelect("v_sr_send2base"));
    ui->menuServer_settings->menuAction()->setVisible(gUserRight->CanSelect("v_sr_send2base"));

    ui->mdiArea->setTabPosition(gSettings->Common.TabPos);
    ui->mdiArea->setViewMode(gSettings->Common.UseTabbedView?QMdiArea::TabbedView:QMdiArea::SubWindowView);

    connect(ui->menuWindow, SIGNAL(triggered(QAction *)), this, SLOT(WindowMenuTriggered(QAction *)));
    connect(ui->menuWindow, SIGNAL(aboutToShow()), this, SLOT(BeforeWindowMenu()));

    connect(gSettings, SIGNAL(StyleSheetChanged()), this, SLOT(StyleSheetChangedSlot()));

    mLabelOnSB = new QLabel(this);
    mLabelOnSB->setAlignment(Qt::AlignHCenter | Qt::AlignRight);
    mLabelOnSB->setVisible(false);
    mLabelOnSB->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mLabelOnSB, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(MenuOnBLList(const QPoint &)));

    ui->statusbar->addPermanentWidget(mLabelOnSB, 0);
}

MainWindow::~MainWindow() {
    if (mTimer) {
        mTimer->stop();
    }
    if (mAlertThread) {
        if (mAlertThread->isRunning()) {
            mAlertThread->AddIdProject(0);
        }
        // need not delete cos anyway all is done
        //delete mAlertThread;
        // just set to NULL - it is checked in some code
        mAlertThread = NULL;
    }
    delete ui;
}

MainWindow *MainWindow::GetInstance() {
    static MainWindow * lMainWindow = NULL;
    if (!lMainWindow) {
        lMainWindow = new MainWindow();
        qAddPostRoutine(MainWindow::clean);
    }
    return lMainWindow;
}

void MainWindow::clean() {
    delete GetInstance();
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        mAlertThread = new UpdateThread(db, this);
        mAlertThread->start(QThread::LowestPriority);

        mTimer = new QTimer(this);
        connect(mTimer, SIGNAL(timeout()), this, SLOT(UpdateOnTimer()));
        mTimer->start(15000);

        QSettings settings;

        settings.beginGroup("Windows");
        settings.beginGroup(metaObject()->className());
        if (!isMaximized()) {
            move(settings.value("Pos").toPoint());
            resize(settings.value("Size").toSize());
        }
        settings.endGroup();
        settings.endGroup();

        //ui->menuServer_settings->menuAction()->setVisible(gUserRight->CanSelect("v_sr_send2base"));
        WaitDlg w(this, true);
        w.show();
        w.SetMessage(tr("Loading data..."));
        settings.beginGroup("WinState");
        LoadMDIWindows(settings);
        settings.endGroup();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (gSettings->Common.ConfirmQuit == 2) {
        event->accept();
    } else {
        QMessageBox mb(this);
        QCheckBox *cb;
        if (gSettings->Common.ConfirmQuit == 1) {
            mb.setWindowTitle(tr("Projects base"));
            mb.setIcon(QMessageBox::Question);
            mb.setText(tr("Quit?"));
            cb = new QCheckBox(tr("Save windows states"), &mb);
            cb->setChecked(gSettings->Common.SaveWinState);
            mb.setCheckBox(cb);
            mb.addButton(QMessageBox::Yes);
            mb.setDefaultButton(mb.addButton(QMessageBox::No));
        }

        QSettings settings;
        settings.beginGroup("Windows");
        settings.beginGroup(metaObject()->className());
        settings.setValue("Maximized", isMaximized());
        settings.setValue("Pos", pos());
        settings.setValue("Size", size());
        settings.endGroup();
        settings.endGroup();

        if (!gSettings->Common.ConfirmQuit
                || mb.exec() == QMessageBox::Yes) {
            QSettings settings2;

            mInLoadWindows = true;

            settings2.remove("WinState");
            if (gSettings->Common.ConfirmQuit == 1 && cb->isChecked()
                    || !gSettings->Common.ConfirmQuit && gSettings->Common.SaveWinState) {
                settings2.beginGroup("WinState");
                SaveMDIWindows(settings2);
                settings2.endGroup();
            }

            event->accept();
        } else {
            event->ignore();
        }
    }
}

void MainWindow::SaveMDIWindows(QSettings & aSettings) {
    int i, j, lActiveSubWindow = -1;
    QList<QMdiSubWindow *> lMdiList = ui->mdiArea->subWindowList(/*QMdiArea::StackingOrder*/);

    aSettings.beginGroup("MDI");
    aSettings.setValue("Count", lMdiList.length());
    aSettings.setValue("UseTabbedView", (ui->mdiArea->viewMode() == QMdiArea::TabbedView)?true:false);
    aSettings.setValue("TabPos", ui->mdiArea->tabPosition());
    for (i = 0; i < lMdiList.length(); i++) {
        if (lMdiList.at(i) == ui->mdiArea->activeSubWindow()) lActiveSubWindow = i;
        aSettings.beginGroup("Window" + QString::number(i));
        aSettings.setValue("Position", lMdiList.at(i)->pos());
        aSettings.setValue("Size", lMdiList.at(i)->size());
        for (int j = 0; j < lMdiList.at(i)->children().length(); j++) {
            if (qobject_cast<QFCDialog *>(lMdiList.at(i)->children().at(j))) {
                aSettings.setValue("Type", lMdiList.at(i)->children().at(j)->metaObject()->className());
                qobject_cast<QFCDialog *>(lMdiList.at(i)->children().at(j))->SaveState(aSettings);
                break;
            }
        }
        aSettings.endGroup();
    }
    aSettings.setValue("ActiveWindow", lActiveSubWindow);
    aSettings.endGroup();

    aSettings.beginGroup("Non-MDI");
    j = 0;
    for (i = 0; i < children().count(); i++) {
        if (qobject_cast<QFCDialog *>(children().at(i))) {
            aSettings.beginGroup("Window" + QString::number(j));

            aSettings.setValue("Position", qobject_cast<QFCDialog *>(children().at(i))->pos());
            aSettings.setValue("Size", qobject_cast<QFCDialog *>(children().at(i))->size());
            aSettings.setValue("Type", children().at(i)->metaObject()->className());
            qobject_cast<QFCDialog *>(children().at(i))->SaveState(aSettings);
            //gLogger->ShowSqlError(this, tr("Project list"), children().at(i)->metaObject()->className());
            aSettings.endGroup();

            j++;
        }
    }
    aSettings.setValue("Count", j);
    aSettings.endGroup();
}

void MainWindow::LoadMDIWindows(QSettings & aSettings) {
    int lWinCnt, i, lActiveSubWindowIndex;
    QMdiSubWindow *lActiveSubWindow = NULL;

    mInLoadWindows = true;

    QList<QMdiSubWindow *> lMdiList = ui->mdiArea->subWindowList(QMdiArea::StackingOrder);
    for (i = 0; i < lMdiList.length(); i++) {
        lMdiList.at(i)->close();
    }

    aSettings.beginGroup("MDI");

    lWinCnt = aSettings.value("Count", 0).toInt();

    gSettings->Common.UseTabbedView = aSettings.value("UseTabbedView", false).toBool();
    gSettings->Common.TabPos = static_cast<QTabWidget::TabPosition>(aSettings.value("TabPos", 0).toInt());
    lActiveSubWindowIndex = aSettings.value("ActiveWindow", -1).toInt();

    for (i = 0; i < lWinCnt; i++) {
        aSettings.beginGroup("Window" + QString::number(i));

        QString lWinType = aSettings.value("Type").toString();
        QFCDialog *lDialog = NULL;
        if (lWinType == "ProjectListDlg") {
            lDialog = new ProjectListDlg(aSettings, this);
        } else if (lWinType == "PlotListDlg") {
            lDialog = new PlotListDlg(aSettings, this);
            if (!static_cast<PlotListDlg *>(lDialog)->ProjectDataConst()) {
                delete lDialog;
                lDialog = NULL;
            }
        } else if (lWinType == "PlotHistDlg") {
            lDialog = new PlotHistDlg(aSettings, this);
        } else if (lWinType == "PlotAddFileDlg") {
            lDialog = new PlotAddFileDlg(aSettings, this);
        } else if (lWinType == "DepartDlg") {
            lDialog = new DepartDlg(aSettings, this);
        } else if (lWinType == "UsersDlg") {
            lDialog = new UsersDlg(aSettings, this);
        } else if (lWinType == "WorkByDateUser") {
            lDialog = new WorkByDateUser(aSettings, this);
        }

        if (lDialog) {
            lDialog->setAttribute(Qt::WA_DeleteOnClose);

            QMdiSubWindow *msw = new QMyMdiSubWindow(ui->mdiArea);
            msw->setAttribute(Qt::WA_DeleteOnClose);
            msw->setWidget(lDialog);

            ui->mdiArea->addSubWindow(msw);

            if (i == lActiveSubWindowIndex) lActiveSubWindow = msw;

            QRect r;
            r.setTopLeft(aSettings.value("Position", lDialog->pos()).toPoint());
            r.setSize(aSettings.value("Size", lDialog->size()).toSize());
            msw->setGeometry(r);

            lDialog->LoadSettings(aSettings);
            msw->show();
        }

        aSettings.endGroup();
    }
    aSettings.endGroup();

    if (lActiveSubWindow) {
        //ui->mdiArea->setActiveSubWindow(lActiveSubWindow);
        lActiveSubWindow->setFocus();
    }

    ui->mdiArea->setViewMode(gSettings->Common.UseTabbedView?QMdiArea::TabbedView:QMdiArea::SubWindowView);
    ui->mdiArea->setTabPosition(gSettings->Common.TabPos);

    aSettings.beginGroup("Non-MDI");
    lWinCnt = aSettings.value("Count", 0).toInt();
    for (i = 0; i < lWinCnt; i++) {
        aSettings.beginGroup("Window" + QString::number(i));

        QString lWinType = aSettings.value("Type").toString();
        QFCDialog *lDialog = NULL;
        if (lWinType == "ProjectListDlg") {
            lDialog = new ProjectListDlg(aSettings, this);
        } else if (lWinType == "PlotListDlg") {
            lDialog = new PlotListDlg(aSettings, this);
        } else if (lWinType == "PlotHistDlg") {
            lDialog = new PlotHistDlg(aSettings, this);
        } else if (lWinType == "PlotAddFileDlg") {
            lDialog = new PlotAddFileDlg(aSettings, this);
        }

        if (lDialog) {
            lDialog->setAttribute(Qt::WA_DeleteOnClose);
            lDialog->show();
            lDialog->move(aSettings.value("Position", lDialog->pos()).toPoint());
            lDialog->resize(aSettings.value("Size", lDialog->size()).toSize());
        }


        aSettings.endGroup();
    }
    aSettings.endGroup();
    mInLoadWindows = false;
}

bool MainWindow::InLoadWindows() {
    return mInLoadWindows;
}

QMdiArea *MainWindow::MdiArea() {
    return ui->mdiArea;
}

UpdateThread *MainWindow::AlertThread() {
    return mAlertThread;
}

QMutex *MainWindow::UpdateMutex() {
    return &mUpdateMutex;
}

void MainWindow::SetPlotForCmp(int aIdPlot, int aHistNum) {
    mPlotForCmp.insert(0, qMakePair(aIdPlot, aHistNum));
    mLabelOnSB->setText(QString::number(aIdPlot) + "/" + QString::number(aHistNum));
    mLabelOnSB->setVisible(true);
}

bool MainWindow::GetPlotForCmp(int &aIdPlot, int &aHistNum) {
    if (mPlotForCmp.isEmpty()) {
        return false;
    } else {
        const tPairIntInt &lPair = mPlotForCmp.at(0);
        aIdPlot = lPair.first;
        aHistNum = lPair.second;
        return true;
    }
}

void MainWindow::ShowProjectRights(const ProjectData *aProject) {
    //gSettings->RunForm("project_rights", QString::number(aProject->Id()));
    ProjectRightsDlg dlg(ProjectRightsDlg::PRDRights, aProject, this);
    dlg.exec();
}

void MainWindow::ShowProjectEnvUser(const ProjectData * aProject) {
    //gSettings->RunForm("project_rights", QString::number(aProject->Id()));
    ProjectRightsDlg dlg(ProjectRightsDlg::PRDEnv, aProject, this);
    dlg.exec();
}


void MainWindow::ShowPlotLists(const QList<tPairIntIntString> &aIds) {
    foreach (tPairIntIntString lExistingId, aIds) {
        PlotData * lPlotGoto = gProjects->FindByIdPlot(lExistingId.first.first);
        if (lPlotGoto) {
            PlotHistoryData * lHistoryGoto = NULL;
            lPlotGoto->ReinitHistory(); // reinit, ya
            foreach (PlotHistoryData * lHistoryFound, lPlotGoto->HistoryConst()) {
                if (lHistoryFound->Num() == lExistingId.first.second) {
                    lHistoryGoto = lHistoryFound;
                    break;
                }
            }
            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, lHistoryGoto);
        } else {
            QMessageBox::critical(this, tr("Project data"), tr("Cant find document with ID = ") + QString::number(lExistingId.first.first));
        }
    }

}

void MainWindow::AddWindowToMDI(QDialog *aDlg) {
    QMdiSubWindow * msw = new QMyMdiSubWindow(ui->mdiArea);
    msw->setAttribute(Qt::WA_DeleteOnClose);
    msw->setWidget(aDlg);
    ui->mdiArea->addSubWindow(msw);
    if (ui->mdiArea->viewMode() == QMdiArea::TabbedView
            || ui->mdiArea->activeSubWindow()
                && ui->mdiArea->activeSubWindow()->isMaximized()) {
        msw->showMaximized();
    } else {
        msw->showNormal();
        QRect lGeom = msw->geometry();
        lGeom.setSize(aDlg->size());
        if (lGeom.height() < 200) lGeom.setHeight(600);
        msw->setGeometry(lGeom);
    }
}

void MainWindow::ShowPlotList(ProjectData * aProjectData, PlotData * aSelectedPlotData, PlotHistoryData *aSelectedHistory) {
    PlotListDlg *wPL = new PlotListDlg(PlotListDlg::DTShowFull, aSelectedPlotData, aSelectedHistory, this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    wPL->SetProjectData(aProjectData);
    AddWindowToMDI(wPL);
}

void MainWindow::ShowPlotVersions(PlotData * aPlotData, bool aUpdateAuto) {
    PlotSimpleListDlg *wPH = new PlotSimpleListDlg(PlotSimpleListDlg::NDTVersions, aPlotData, NULL, false, this);
    wPH->setAttribute(Qt::WA_DeleteOnClose);
    if (gSettings->DocumentHistory.MDI) {
        AddWindowToMDI(wPH);
    } else {
        wPH->show();
    }
}

void MainWindow::ShowPlotHist(PlotData * aPlotData, int aHistoryNum, bool aUpdateAuto, bool aModal) {
    PlotHistDlg *wPH = new PlotHistDlg(aPlotData, aHistoryNum, aUpdateAuto, this);
    wPH->setAttribute(Qt::WA_DeleteOnClose);
    if (aModal) {
        wPH->exec();
    } else {
        if (gSettings->DocumentHistory.MDI) {
            AddWindowToMDI(wPH);
        } else {
            wPH->show();
        }
    }
}

void MainWindow::ShowPlotAddFiles(PlotData * aPlot, PlotHistoryData * aHistory, bool aModal) {
    PlotAddFileDlg *wPH = new PlotAddFileDlg(aPlot, aHistory, this);
    wPH->setAttribute(Qt::WA_DeleteOnClose);
    if (aModal) {
        wPH->exec();
    } else {
        if (gSettings->DocumentHistory.MDI) {
            AddWindowToMDI(wPH);
        } else {
            wPH->show();
        }
    }
}

void MainWindow::ShowPlotAttrs() {
    PlotAttrDlg *wPH = new PlotAttrDlg(this);
    wPH->setAttribute(Qt::WA_DeleteOnClose);
    if (gSettings->DocumentHistory.MDI) {
        AddWindowToMDI(wPH);
    } else {
        wPH->show();
    }
}

void MainWindow::ShowPlotProp(PlotData * aPlotData, PlotHistoryData * aPlotHistoryData) {
    PlotProp w(aPlotData, aPlotHistoryData, this);
    w.exec();
}

void MainWindow::ShowPlotRights(PlotData * aPlotData) {
    gSettings->RunForm("plot_rights", QString::number(aPlotData->Id()));
}

void MainWindow::ShowPlotXrefs(PlotData * aPlotData, PlotHistoryData *aPlotHistory, bool aUpdateAuto, bool aModal) {
    PlotSimpleListDlg *wPH = new PlotSimpleListDlg(PlotSimpleListDlg::NDTXrefs, aPlotData, aPlotHistory, aUpdateAuto, this);
    wPH->setAttribute(Qt::WA_DeleteOnClose);
    if (aModal) {
        wPH->exec();
    } else {
        if (gSettings->DocumentHistory.MDI) {
            AddWindowToMDI(wPH);
        } else {
            wPH->show();
        }
    }
}

void MainWindow::ShowPlotXrefsAllOLD(PlotData * aPlotData) {
    aPlotData->InitIdDwgMax();
    gSettings->RunForm("plot_xref", QString::number(aPlotData->IdDwgMax()) + " 1");
}

void MainWindow::ShowPlotXrefFor(PlotData * aPlotData) {
    PlotSimpleListDlg *wPH = new PlotSimpleListDlg(PlotSimpleListDlg::NDTXrefFor, aPlotData, /*aPlotHistory*/NULL, /*aUpdateAuto*/true, this);
    wPH->setAttribute(Qt::WA_DeleteOnClose);
    if (/*aModal*/false) {
        wPH->exec();
    } else {
        if (gSettings->DocumentHistory.MDI) {
            AddWindowToMDI(wPH);
        } else {
            wPH->show();
        }
    }
/*    aPlotData->InitIdDwgMax();
    gSettings->RunForm("plot_xref_for", QString::number(aPlotData->IdDwgMax()));*/
}

void MainWindow::ShowLoadImages(int aIdProject) {
    LoadImagesDlg *wPL = new LoadImagesDlg(aIdProject, this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPL);
}

void MainWindow::ShowPublishReport(int aId) {
    PublishReport *wPL = new PublishReport(aId, this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPL);
}

void MainWindow::RecoverPlot(PlotData * aPlotData) {
    aPlotData->InitIdDwgMax();

    MainDataForCopyToAcad lDataForAcad(6);
    lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eAP, aPlotData->IdDwgMax()));
    gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
}

void MainWindow::LoadXrefs(int aIdProject) {
    return;
    LoadXrefsDlg *wPL = new LoadXrefsDlg(aIdProject, this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPL);
}

PlotData * MainWindow::NewPlot(ProjectData *aProjectData, const TreeDataRecord *aTreeData, const QString &aComplect, PlotData *aPlotDataFrom,
                               PlotHistoryData *aPlotHistoryDataFrom) {
    PlotNewDlg w(aProjectData, aTreeData, aComplect, aPlotDataFrom, aPlotHistoryDataFrom, this);
    if (w.exec() == QDialog::Accepted) {
        PlotData * lPlotData = new PlotData(w.IdPlot());
        return lPlotData;
    }
    return NULL;
}

void MainWindow::on_actionProjects_triggered() {
    ProjectListDlg * wPL = new ProjectListDlg(ProjectListDlg::DTShowFull, this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPL);
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox::information(this, tr("About"), tr("Projects Base\n\nVersion 2.0"));
}

void MainWindow::on_actionContracts_triggered()
{
    if (!gHasModule("contract-pkz")) return;
    gSettings->InitNDS();
    ContractPkz *w = new ContractPkz(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void MainWindow::on_actionCommonSettings_triggered() {
    CommonSettingsDlg w(this);

    w.SetVisualStyle(gSettings->Common.VisualStyle);
    w.SetConfirmQuit(gSettings->Common.ConfirmQuit);
    w.SetUseTabbedView(gSettings->Common.UseTabbedView);
    w.SetTabPos(gSettings->Common.TabPos);
    w.SetSaveWinState(gSettings->Common.SaveWinState);

    w.SetFont(QApplication::font());
    w.SetAddRowHeight(gSettings->Common.AddRowHeight);
    //w.SetAddRowHeight(gSettings->Common.SubRowHeight);

    w.SetShowAfterCopy(gSettings->Common.ShowAfterCopy);

    if (w.exec() == QDialog::Accepted) {
        QString lNewVisualStyle = w.VisualStyle();
        if (lNewVisualStyle.toLower() != gSettings->Common.VisualStyle.toLower()) {
            QApplication::setStyle(QStyleFactory::create(lNewVisualStyle));
            gSettings->Common.VisualStyle = lNewVisualStyle;
        }

        gSettings->Common.ConfirmQuit = w.ConfirmQuit();
        gSettings->Common.SaveWinState = w.SaveWinState();

        gSettings->Common.TabPos = w.TabPos();
        ui->mdiArea->setTabPosition(gSettings->Common.TabPos);
        if (gSettings->Common.UseTabbedView != w.UseTabbedView()) {
            gSettings->Common.UseTabbedView = w.UseTabbedView();
            ui->mdiArea->setViewMode(gSettings->Common.UseTabbedView?QMdiArea::TabbedView:QMdiArea::SubWindowView);
        }

        QApplication::setFont(w.Font());
        gSettings->Common.AddRowHeight = w.AddRowHeight();
        emit gSettings->StyleSheetChanged();

        gSettings->Common.ShowAfterCopy = w.ShowAfterCopy();
    }
}

void MainWindow::on_actionQuick_find_by_id_triggered() {
    QInputDialog lInput(this);

    lInput.setWindowFlags(lInput.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //lInput.setOptions();
    lInput.setWindowTitle(tr("Quick find"));
    lInput.setLabelText(tr("Enter document ID:"));
    lInput.setInputMode(QInputDialog::IntInput);
    lInput.setIntMinimum(1);
    lInput.setIntMaximum(1000000);

a1:
    if (lInput.exec() == QDialog::Accepted) {
        PlotData * lPlotData = gProjects->FindByIdPlot(lInput.intValue());
        if (lPlotData) {
            ShowPlotList(gProjects->FindByIdProject(lPlotData->IdProject()), lPlotData, NULL);
        } else {
            QMessageBox::critical(this, tr("Find document by id"), tr("Document not found"));
            goto a1; // fuck you
        }
    }
}

void MainWindow::on_actionDrawings_audit_triggered() {
    AuditReport *wPL = new AuditReport(this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPL);
}

void MainWindow::on_actionDrawings_publish_triggered() {
    ShowPublishReport(0);
}

void MainWindow::StyleSheetChangedSlot() {
    gProjects->EmitPlotListBeforeUpdate(0); // update all lists
    gProjects->EmitPlotListNeedUpdate(0); // update all lists
}

void MainWindow::WindowMenuTriggered(QAction * action) {
    QVariant lParam1;
    QSettings lSettings;
    lSettings.beginGroup("Windows sets");

    lParam1 = action->property("MyWindowPointer");
    if (!lParam1.isNull()) {
        lParam1.value<QMdiSubWindow *>()->setFocus();
    } else {
        lParam1 = action->property("NewWindowSet");
        if (!lParam1.isNull()) {
            QInputDialog lInput(this);

            lInput.setWindowFlags(lInput.windowFlags() & ~Qt::WindowContextHelpButtonHint);

            lInput.setWindowTitle(tr("Create windows set"));
            lInput.setLabelText(tr("Enter new windows set name"));
            lInput.setTextValue(lParam1.toString());
            if (lInput.exec() == QDialog::Accepted) {
                //QStringList lSets;
                //lSets = lSettings.allKeys();
                if (lSettings.allKeys().contains(lInput.textValue())) {
                    if (QMessageBox::question(this, tr("Create windows set"), tr("Windows set") + " " + lInput.textValue() + " "
                                              + tr("already exists!") + "\n" + tr("Overwrite?")) != QMessageBox::Yes) {
                        return;
                    }

                }
                lSettings.beginGroup(lInput.textValue());
                SaveMDIWindows(lSettings);
                lSettings.endGroup();
            }
        } else {
            lParam1 = action->property("SaveWindowSet");
            if (!lParam1.isNull()) {
                if (QMessageBox::question(this, tr("Save windows set"), tr("Windows set") + " " + lParam1.toString() + " "
                                          + tr("already exists!") + "\n" + tr("Overwrite?")) != QMessageBox::Yes) {
                    return;
                }
                lSettings.beginGroup(lParam1.toString());
                SaveMDIWindows(lSettings);
                lSettings.endGroup();
            } else {
                lParam1 = action->property("LoadWindowSet");
                if (!lParam1.isNull()) {
                    lSettings.beginGroup(lParam1.toString());
                    LoadMDIWindows(lSettings);
                    lSettings.endGroup();
                } else {
                    lParam1 = action->property("DeleteWindowSet");
                    if (!lParam1.isNull()) {
                        if (QMessageBox::question(this, tr("Delete windows set"), tr("Delete windows set") + " " + lParam1.toString() + "?") == QMessageBox::Yes) {
                            lSettings.remove(lParam1.toString());
                        }
                    }
                }
            }
        }
    }
    lSettings.endGroup();
}

void MainWindow::BeforeWindowMenu() {
    int i, j, sepCnt = 0;
    QAction *lNewAction;
    QList<QMdiSubWindow *> lMdiList = ui->mdiArea->subWindowList(/*QMdiArea::StackingOrder*/);

    for (i = ui->menuWindow->actions().length() - 1; i >= 0; i--) {
        if (ui->menuWindow->actions().at(i)->isSeparator()) sepCnt++;
        if (sepCnt == 2) break;
        delete ui->menuWindow->actions().at(i);
    }

    // collect windows set (as strings)
    QStringList lSets;
    QSettings lSettings;
    lSettings.beginGroup("Windows sets");
    lSets = lSettings.childGroups();
    lSettings.endGroup();
    // end collectioning

    QMenu *lSubMenu;
    lSubMenu = ui->menuWindow->addMenu(tr("Save set"));
    if (!lSets.isEmpty()) {
        for (i = 0; i < lSets.length(); i++) {
            lNewAction = lSubMenu->addAction(lSets.at(i));
            lNewAction->setProperty("SaveWindowSet", lSets.at(i));
        }
        lSubMenu->addSeparator();
    }
    lNewAction = lSubMenu->addAction(tr("New..."));
    lNewAction->setProperty("NewWindowSet", tr("Windows set") + " " + QString::number(lSets.length() + 1));

    if (!lSets.isEmpty()) {
        lSubMenu = ui->menuWindow->addMenu(tr("Restore set"));
        for (i = 0; i < lSets.length(); i++) {
            lNewAction = lSubMenu->addAction(lSets.at(i));
            lNewAction->setProperty("LoadWindowSet", lSets.at(i));
        }
        lSubMenu = ui->menuWindow->addMenu(tr("Delete set"));
        for (i = 0; i < lSets.length(); i++) {
            lNewAction = lSubMenu->addAction(lSets.at(i));
            lNewAction->setProperty("DeleteWindowSet", lSets.at(i));
        }
    }

    ui->menuWindow->addSeparator();

    for (i = 0; i < lMdiList.length(); i++) {
        QString lWT = lMdiList.at(i)->windowTitle();
        lWT = lWT.left(lWT.lastIndexOf(" - " + gSettings->BaseName));

        for (j = 0; j < lMdiList.at(i)->children().length(); j++) {
            if (qobject_cast<ProjectListDlg *>(lMdiList.at(i)->children().at(j))
                    && qobject_cast<ProjectListDlg *>(lMdiList.at(i)->children().at(j))->GetProjectData()) {
                lWT += " - " + qobject_cast<ProjectListDlg *>(lMdiList.at(i)->children().at(j))->GetProjectData()->FullShortName();
                break;
            }
        }

        lNewAction = ui->menuWindow->addAction(lWT);

        if (i < 9) {
            lNewAction->setShortcut(QKeySequence("Ctrl+" + QString::number(i + 1)));
        } else if (i == 9) {
            lNewAction->setShortcut(QKeySequence("Ctrl+0"));
        }

        bool lIsCurrent = lMdiList.at(i) == ui->mdiArea->activeSubWindow();
        lNewAction->setCheckable(lIsCurrent);
        lNewAction->setChecked(lIsCurrent);

        lNewAction->setProperty("MyWindowPointer", QVariant::fromValue(lMdiList.at(i)));
    }
}

void MainWindow::UpdateOnTimer() {
    if (!mAlertThread) return;

    MyMutexLocker lLocker(&mUpdateMutex, 0);
    if (!lLocker.IsLocked()) return; // skip to next timer cycle

    if (QApplication::modalWindow()) return; // skip to next timer cycle

    tPairIntInt lPair;
    while (mAlertThread->GetUpdateData(lPair)) {
        switch (lPair.first) {
        case 0:
            gProjects->UpdateProjectInList();
            break;
        case 1:
            gProjects->UpdateGroupInList();
            break;
        case 2:
            gProjects->RemoveProjectFromList();
            break;
        case 3:
            gProjects->RemoveGroupFromList();
            break;
        case 4:
            gProjects->UpdatePlotList(lPair.second);
            break;
        case 5:
            // disconnect, thread connection failed
            db.close();
            if (QMessageBox::question(this, tr("Connection to Projects Base"),
                                      tr("Connection closed\nQuit program?")) == QMessageBox::Yes) {
                QApplication::quit();
            }
            break;
        }
    }
}

void MainWindow::MenuOnBLList(const QPoint &aPoint) {
    QMenu lMenu(this);

    QAction *lARes;

    foreach(tPairIntInt lPair, mPlotForCmp) {
        lMenu.addAction(QString::number(lPair.first) + "/" + QString::number(lPair.second));
    }

    if (lARes = lMenu.exec(QCursor::pos())) {

    }
}

void MainWindow::on_actionSelect_AutoCAD_triggered() {
    ChooseAcadDlg w(NULL, this);
    w.exec();
//    if (w.exec() == QDialog::Accepted) {
//    }
}

void MainWindow::on_actionScheduler_triggered() {
    gSettings->RunForm("scheduler", "");
}

void MainWindow::on_actionPlanner_day_triggered() {
    gSettings->RunForm("adm_scheduler", "");
}

void MainWindow::on_actionSend_message_triggered() {
    gSettings->RunOldMessageData("/3");
}

void MainWindow::on_actionMessages_to_me_triggered() {
    gSettings->RunOldMessageData("/1");

}

void MainWindow::on_actionMessages_from_me_triggered() {
    gSettings->RunOldMessageData("/2");
}

void MainWindow::on_actionDocuments_list_triggered() {
    DocTreeSettings w(this);

    w.SetTTExpandLevel(gSettings->TypeTree.ExpandLevel);
    w.SetTTFontPlusOne(gSettings->TypeTree.FontPlusOne);
    w.SetTTFontBold(gSettings->TypeTree.FontBold);

    w.SetWindowTitleType(gSettings->DocumentTree.WindowTitleType);

    w.SetOpenSingleDocument(gSettings->DocumentTree.OpenSingleDocument);
    w.SetShowGridLines(gSettings->DocumentTree.ShowGridLines);
    w.SetAutoWidth(gSettings->DocumentTree.AutoWidth);
    w.SetDocFontPlusOne(gSettings->DocumentTree.DocFontPlusOne);
    w.SetDocFontBold(gSettings->DocumentTree.DocFontBold);
    w.SetAddRowHeight(gSettings->DocumentTree.AddRowHeight);

    w.SetDragDrop(gSettings->DocumentTree.DragDrop);

    w.SetOnDocDblClick(gSettings->DocumentTree.OnDocDblClick);
    w.SetSecondLevel(gSettings->DocumentTree.SecondLevelType);
    w.SetExpandOnShow(gSettings->DocumentTree.ExpandOnShow);

    w.SetColors(gSettings->DocumentTree.UseDocColor, gSettings->DocumentTree.DocColor,
                gSettings->DocumentTree.UseSecondColor, gSettings->DocumentTree.SecondColor);

    if (w.exec() == QDialog::Accepted) {
        gSettings->TypeTree.ExpandLevel = w.TTExpandLevel();
        gSettings->TypeTree.FontPlusOne = w.TTFontPlusOne();
        gSettings->TypeTree.FontBold = w.TTFontBold();

        gSettings->DocumentTree.WindowTitleType = static_cast<GlobalSettings::DocumentTreeStruct::WNDTitleType>(w.WindowTitleType());

        gSettings->DocumentTree.OpenSingleDocument = w.OpenSingleDocument();
        gSettings->DocumentTree.ShowGridLines = w.ShowGridLines();
        gSettings->DocumentTree.AutoWidth = w.AutoWidth();
        gSettings->DocumentTree.DocFontPlusOne = w.DocFontPlusOne();
        gSettings->DocumentTree.DocFontBold = w.DocFontBold();
        gSettings->DocumentTree.AddRowHeight = w.AddRowHeight();

        gSettings->DocumentTree.DragDrop = w.DragDrop();

        gSettings->DocumentTree.OnDocDblClick = w.OnDocDblClick();
        gSettings->DocumentTree.SecondLevelType = w.SecondLevel();
        gSettings->DocumentTree.ExpandOnShow = w.ExpandOnShow();

        w.GetColors(gSettings->DocumentTree.UseDocColor, gSettings->DocumentTree.DocColor,
                    gSettings->DocumentTree.UseSecondColor, gSettings->DocumentTree.SecondColor);

        emit gSettings->DocTreeSettingsChanged();

    }
}

void MainWindow::on_actionQuit_triggered() {
    close();
}

void MainWindow::on_actionQuit_and_reset_triggered() {
    if (QMessageBox::question(this, tr("Projects base"), tr("Quit and restore all settings to defaults?")) == QMessageBox::Yes) {
        gSettings->Common.ConfirmQuit = 2;
        close();
    }
}

void MainWindow::on_actionFind_triggered() {
    FindDlg *wPL = new FindDlg(this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPL);
}

void MainWindow::on_actionDepartments_triggered() {
//    gSettings->RunForm("department", "");
    DepartDlg *wPL = new DepartDlg(DepartDlg::DTEdit, this);
    wPL->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPL);
}

void MainWindow::on_actionOrganizations_triggered() {
//    gSettings->RunForm("customers2", "");
    OrganizationsListDlg *wPO = new OrganizationsListDlg(OrganizationsListDlg::DTEdit, this);
    wPO->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wPO);
}


void MainWindow::on_actionUsers_work_triggered() {
    if (gHasModule("WorkByDateUser")) {
        WorkByDateUser *w = new WorkByDateUser(this);
        w->setAttribute(Qt::WA_DeleteOnClose);
        AddWindowToMDI(w);
    }
}

void MainWindow::on_actionComparing_in_AutoCAD_triggered() {
    DwgCmpSettingsDlg w(this);
    w.exec();
}

void MainWindow::on_actionEmployees_triggered() {
    UsersDlg *wU = new UsersDlg(UsersDlg::DTEdit, this);
    wU->setAttribute(Qt::WA_DeleteOnClose);
    AddWindowToMDI(wU);
}

void MainWindow::on_actionDeleted_documents_triggered() {
    PlotDeletedDlg *wPH = new PlotDeletedDlg(this);
    wPH->setAttribute(Qt::WA_DeleteOnClose);
    if (/*gSettings->DocumentHistory.MDI*/true) {
        AddWindowToMDI(wPH);
    } else {
        wPH->show();
    }
}

void MainWindow::on_actionChange_password_triggered() {
    UserData *lUser = gUsers->FindByLogin(db.userName());
    if (lUser) {
        ChangePassDlg w(lUser, this);
        w.exec();
    }
}

void MainWindow::on_actionDocuments_tree_triggered() {
    gSettings->RunForm("tree_admin", "");
}

void MainWindow::on_actionSaved_documents_triggered() {
    gSettings->RunForm("report_save", "");
}

void MainWindow::on_actionRestrictions_on_savings_triggered() {
    gSettings->RunForm("save_restrict", "");
}

void MainWindow::on_actionGeobases_triggered() {
    if (gHasModule("geobase")) {
        Geobase *wPH = new Geobase(this);
        wPH->setAttribute(Qt::WA_DeleteOnClose);
        if (/*gSettings->DocumentHistory.MDI*/true) {
            AddWindowToMDI(wPH);
        } else {
            wPH->show();
        }
    }
}

void MainWindow::on_actionIncoming_triggered() {
    gSettings->RunForm("letters", "0");
}

void MainWindow::on_actionOutgouing_triggered() {
    gSettings->RunForm("letters", "1");
}

void MainWindow::on_actionTemplates_for_outgoing_triggered() {
    gSettings->RunForm("templ_letter_out", "");
}

void MainWindow::on_actionWeekly_report_old_triggered() {
    gSettings->RunForm("letterRepByWeek", "");
}

void MainWindow::on_actionLetters_list_triggered() {
    gSettings->RunForm("lett_reestr", "");
}

void MainWindow::on_actionProject_types_triggered() {
    ProjectTypeDlg w(this);

    w.exec();
}

void MainWindow::on_actionContracts_2_triggered() {
    gSettings->RunForm("contracts", "byAll");
}

void MainWindow::on_actionImage_settings_triggered() {
    const int lResizeForPreview = gSettings->Image.ResizeForPreview;
    const int lW = gSettings->Image.MaxPreviewWidth, lH = gSettings->Image.MaxPreviewHeight;

    LISettingsDlg w(this);
    if (w.exec() == QDialog::Accepted) {
        if (lResizeForPreview != gSettings->Image.ResizeForPreview
                || lW != gSettings->Image.MaxPreviewWidth
                || lH != gSettings->Image.MaxPreviewHeight/*
                || lMFS != gSettings->Image.MaxFileSize*/) {
            emit ImageSettingsChanged();
        }
    }
}

void MainWindow::on_actionSubcontracts_triggered() {
    gSettings->RunForm("contrSm", "byAll");
}
