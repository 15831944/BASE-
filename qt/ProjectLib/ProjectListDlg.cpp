#include "ProjectListDlg.h"
#include "ui_ProjectListDlg.h"
#include "ProjectTree.h"

#include "ProjectData.h"
#include "CodeFormingDlg.h"
#include "ProjectCard.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"
#include "../UsersDlg//CustomerData.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/HomeData.h"
#include "../VProject/PlotListDlg.h"

#include "../VProject/MainWindow.h"

#include "../VProject/common.h"
#include "../VProject/oracle.h"

#include <QMenu>
#include <QMenuBar>

ProjectListDlg::ProjectListDlg(DisplayType aDisplayType, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::ProjectListDlg),
    mProjectData(NULL), mDisplayType(aDisplayType),
    mJustStarted(true)
{
    ui->setupUi(this);

    InitInConstructor();
}

ProjectListDlg::ProjectListDlg(QSettings &aSettings, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::ProjectListDlg),
    mProjectData(NULL),
    mJustStarted(true)
{
    ui->setupUi(this);

    ui->cbProjTreeMode->setCurrentIndex(aSettings.value("Mode", 0).toInt());

    ui->lineEdit->setText(aSettings.value("Filter").toString());
    mDisplayType = (DisplayType) aSettings.value("DisplayType", (int) DTShowFull).toInt();

    int lSelectedType, lSelectedId;

    lSelectedType = aSettings.value("SelectedType", 0).toInt();
    lSelectedId = aSettings.value("SelectedId", 0).toInt();

    if (lSelectedId) {
        mProjectData = gProjects->FindByIdProject(lSelectedId);
        if (lSelectedType == ProjectData::PDProject) {
            ui->treeWidget->SetSelectedProject(lSelectedId);
        } else {
            ui->treeWidget->SetSelectedGroup(lSelectedId);
        }
    }

    ui->treeWidget->setLayoutDirection((Qt::LayoutDirection) aSettings.value("TreeRLO", Qt::LeftToRight).toInt());

    InitInConstructor();
}

void ProjectListDlg::InitInConstructor() {
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    ui->treeWidget->SetCanDragDrop(mDisplayType == DTShowFull);

    // ----------------------------------------------
    QSplitterHandle *handle = ui->splitter->handle(1);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    QFrame *line = new QFrame(handle);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    ui->splitter->setStretchFactor(1, 100);
    // ----------------------------------------------

    QPalette lPalette = ui->treeWidget->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->treeWidget->setPalette(lPalette);

    bool lHasOldContracts = QFile::exists(QApplication::applicationDirPath() + "/../../forms/contracts.fmx");
    ui->pbContracts->setVisible(lHasOldContracts);
    ui->pbSubContracts->setVisible(lHasOldContracts);

    ui->wdButtons->setVisible((gHomeData->Get("BUTTONS_ON_PROJLIST").toInt() == 1)
                              && (mDisplayType == DTShowFull));
}

ProjectListDlg::~ProjectListDlg() {
    delete ui;
}

void ProjectListDlg::SaveAdditionalSettings(QSettings &aSettings) {
    if (mDisplayType == DTShowSelect) {
        aSettings.setValue("ProjTreeMode", ui->cbProjTreeMode->currentIndex());
    }
    aSettings.setValue("TreeRLO", ui->treeWidget->layoutDirection());
}

void ProjectListDlg::LoadAdditionalSettings(QSettings &aSettings) {
    if (mDisplayType == DTShowSelect) {
        ui->cbProjTreeMode->setCurrentIndex(aSettings.value("ProjTreeMode", 4).toInt());
    }
    ui->treeWidget->setLayoutDirection((Qt::LayoutDirection) aSettings.value("TreeRLO", Qt::LeftToRight).toInt());
}

void ProjectListDlg::SaveState(QSettings &aSettings) {
    SaveSettings(aSettings);
    aSettings.setValue("Mode", ui->cbProjTreeMode->currentIndex());
    aSettings.setValue("Filter", ui->lineEdit->text());
    aSettings.setValue("DisplayType", mDisplayType);
    aSettings.setValue("TreeRLO", ui->treeWidget->layoutDirection());

    mProjectData = ui->treeWidget->GetSelectedProject();
    if (mProjectData) {
        aSettings.setValue("SelectedType", mProjectData->Type());
        aSettings.setValue("SelectedId", mProjectData->Id());
    } else {
        aSettings.setValue("SelectedType", 0);
        aSettings.setValue("SelectedId", 0);
    }
}

void ProjectListDlg::ShowPlotList() {
    if (!ui->treeWidget->GetSelectedProject()) return;
    if (ui->treeWidget->GetSelectedProject()->Type() == ProjectData::PDGroup) return;

    if (qobject_cast<QMdiSubWindow *>(parent())) {
        gMainWindow->ShowPlotList(ui->treeWidget->GetSelectedProject(), NULL, NULL);
    } else {
//        if (gSettings->DocumentTree.RefreshOnWindowOpen && ui->treeWidget->GetSelectedProject()) {
//            ui->treeWidget->GetSelectedProject()->ReinitLists();
//        }
        PlotListDlg *w = new PlotListDlg(PlotListDlg::DTShowFull, NULL, NULL, parentWidget());
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->SetProjectData(ui->treeWidget->GetSelectedProject());
        w->show();
    }
}

void ProjectListDlg::ShowProjectCard() {
    if (!ui->treeWidget->GetSelectedProject()) return;
    if (ui->treeWidget->GetSelectedProject()->Type() == ProjectData::PDGroup) return;

    if (!gUserRight->CanSelect("v_proj_prop")) return;
    if (!QFile::exists(QApplication::applicationDirPath() + "/../../common/templates/ProjectCard.doc")) return;

    ProjectCard w;
    w.SetIdProject(ui->treeWidget->GetSelectedProject()->Id());
    w.exec();
}


void ProjectListDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        ui->treeWidget->PopulateTree();

        // it will be showed on (user) select in tree
        if (mDisplayType == DTShowSelect) {
            if (qobject_cast<QMdiSubWindow *> (parent())) {
                setWindowTitle(tr("Select project"));
            } else {
                setWindowTitle(tr("Select project") + " - " + gSettings->BaseName);
            }
            ui->stProps->setCurrentWidget(ui->page_sel);
        } else {
            if (qobject_cast<QMdiSubWindow *> (parent())) {
                setWindowTitle(tr("Projects"));
            } else {
                setWindowTitle(tr("Projects") + " - " + gSettings->BaseName);
            }
        }

        on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), NULL);

        mJustStarted = false;
    }


//    int i, j;
//    bool lMoved;
//    if (parentWidget()->asdfsdfsdfinherits("QMdiSubWindow")) {
//        QList<QMdiSubWindow *> lMdiList = qobject_cast<QMdiSubWindow *>(parentWidget())->mdiArea()->subWindowList();
//        do {
//            lMoved = false;
//            int lFound = -1;
//            for (i = 0; i < lMdiList.length(); i++) {
//                for (j = 0; j < lMdiList.at(i)->children().length(); j++) {
//                    if (lMdiList.at(i)->children().at(j) == this) {
//                        lFound = i;
//                        gLogger->ShowError(this, tr("Project list"), "aaaa");
//                        break;
//                    }
//                }
//            }

//            if (lFound != -1) {
//                for (i = 0; i < lMdiList.length(); i++) {
//                    if (i != lFound
//                            && lMdiList.at(i)->pos() == lMdiList.at(lFound)->pos()) {
//                        lMdiList.at(lFound)->move(lMdiList.at(lFound)->pos().x() + 20, lMdiList.at(lFound)->pos().y() + 20);
//                        lMoved = true;
//                    }
//                    for (j = 0; j < lMdiList.at(i)->children().length(); j++) {
//                        if (lMdiList.at(i)->children().at(j) != this
//                                && lMdiList.at(i)->children().at(j)->sdfsdfsdfinherits("ProjectListDlg")
//                                && qobject_cast<ProjectListDlg *>(lMdiList.at(i)->children().at(j))->pos() == pos()) {
//                        }
//                    }
//                }
//            }
//        } while (lMoved);

//    } else {
//        do {
//            lMoved = false;
//            for (i = 0; i < QApplication::topLevelWidgets().length(); i++) {
//                if (qobject_cast<ProjectListDlg *>(QApplication::topLevelWidgets().at(i))
//                        && QApplication::topLevelWidgets().at(i) != this) {
//                    if (QApplication::topLevelWidgets().at(i)->pos() == pos()) {
//                        move(pos().x() + 20, pos().y() + 20);
//                        lMoved = true;
//                    }
//                }
//            }
//        } while (lMoved);
//    }
}

void ProjectListDlg::SetMode(int aMode) {
    ui->cbProjTreeMode->setCurrentIndex(aMode);
}

void ProjectListDlg::SetSelectedProject(long aProject) {
    ui->treeWidget->SetSelectedProject(aProject);
}


void ProjectListDlg::on_cbProjTreeMode_currentIndexChanged(int index) {
    if (index == 5
            && ui->lineEdit->text().isEmpty()) {
        // can't switch to filter mode manually
        ui->cbProjTreeMode->blockSignals(true);
        ui->cbProjTreeMode->setCurrentIndex(ui->treeWidget->Mode());
        ui->cbProjTreeMode->blockSignals(false);
    } else {
        // non filter mode
        ui->lineEdit->blockSignals(true);
        ui->lineEdit->setText("");
        ui->lineEdit->blockSignals(false);

        ui->treeWidget->SetMode(static_cast<ProjectTree::PTShowMode>(index));
        ui->treeWidget->SetFilter("");
        ui->treeWidget->ShowTree();
    }
}

void ProjectListDlg::on_buttonBox_accepted() {
    mProjectData = ui->treeWidget->GetSelectedProject();
    if (mProjectData && mProjectData->Type() == ProjectData::PDProject) accept();
}

void ProjectListDlg::on_lineEdit_textChanged(const QString &arg1) {
    ui->cbProjTreeMode->blockSignals(true);
    if (!arg1.isEmpty()) {
        ui->cbProjTreeMode->setCurrentIndex(5);
    } else {
        ui->cbProjTreeMode->blockSignals(true);
        ui->cbProjTreeMode->setCurrentIndex(ui->treeWidget->Mode());
        ui->cbProjTreeMode->blockSignals(false);
    }
    ui->cbProjTreeMode->blockSignals(false);
    // need not set mode to treeWidget
    ui->treeWidget->SetFilter(arg1);
    ui->treeWidget->ShowTree();
}

void ProjectListDlg::on_treeWidget_doubleClicked(const QModelIndex &) {
    if (mDisplayType == DTShowSelect) {
        mProjectData = ui->treeWidget->GetSelectedProject();
        if (mProjectData && mProjectData->Type() == ProjectData::PDProject) accept();
    } else {
        ShowPlotList();
    }
}

void ProjectListDlg::on_tbReload_clicked() {
    //QMutexLocker lLocker(gMainWindow->UpdateMutex());
    MyMutexLocker lLocker(gMainWindow->UpdateMutex(), 0);
    if (!lLocker.IsLocked()) return; // something wrong
    gProjects->InitProjectList(true); // refresh
}

void ProjectListDlg::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (mDisplayType == DTShowSelect) return;

    if (current) {
        mProjectData = static_cast<ProjectTreeItem *>(current)->ProjectRef();
    } else {
        mProjectData = NULL;
    }

    if (mProjectData) {
        if (mProjectData->Type() == ProjectData::PDProject) {
            ui->leID->setText(QString::number(mProjectData->Id()));
            ui->leShortName->setText(mProjectData->ShortNameConst());

            ui->leStage->setText(mProjectData->StageConst());

            if (mProjectData->IdParentProject()) {
                ui->leNumber->setText(mProjectData->ShortNumConst());
                ui->leNumber->setVisible(true);
                ui->lblNumber->setVisible(true);
            } else {
                ui->leNumber->setText("");
                ui->leNumber->setVisible(false);
                ui->lblNumber->setVisible(false);
            }

            const QString & lGipName = gUsers->GetName(mProjectData->GipConst());
            ui->leGIP->setText(lGipName);

            CustomerData *lCustomerData;
            if (mProjectData->IdCustomer()
                    && (lCustomerData = gCustomers->GetCustomerById(mProjectData->IdCustomer()))) {
                ui->leCustomer->setText(lCustomerData->ShortNameConst());
            } else {
                ui->leCustomer->setText("");
            }

            // contract
            if (gSettings->ContractMode == 1) {
                ui->leContract->setText(mProjectData->ContractConst());
                if (!mProjectData->ContractDateConst().isNull()) {
                    ui->leContractDate->setText(mProjectData->ContractDateConst().toString("dd.MM.yy"));
                } else {
                    // no contract date
                    ui->leContractDate->setText("");
                }
            } else {

            }

            ui->ptFullName->setPlainText(mProjectData->NameConst());
            ui->ptComments->setPlainText(mProjectData->CommentsConst());

            ui->leCrDate->setText(mProjectData->StartDateConst().toString("dd.MM.yy"));
            ui->leCrUser->setText(gUsers->GetName(mProjectData->CrUserConst()));

            if (!mProjectData->EndDateConst().isNull()) {
                ui->leEndDate->setText(mProjectData->EndDateConst().toString("dd.MM.yy"));
                ui->leEndUser->setText(gUsers->GetName(mProjectData->EndUserConst()));
                ui->wdArchive->setVisible(true);
            } else {
                ui->wdArchive->setVisible(false);
            }

            ui->stProps->setCurrentWidget(ui->page_proj);
        } else {
            ui->leGrpName->setText(mProjectData->ShortNameConst());

            ui->stProps->setCurrentWidget(ui->page_group);
        }
    } else {
        ui->stProps->setCurrentWidget(ui->page_null);
    }
}

void ProjectListDlg::on_treeWidget_customContextMenuRequested(const QPoint &pos) {
    QMenu lMenu(this);
    ProjectTreeItem * lCurItem = static_cast<ProjectTreeItem *>(ui->treeWidget->currentItem());
    QAction *lARes;
    QAction *lAExpandAll = NULL, *lACollapseAll = NULL,
            *lADocuments = NULL,
            *lAAddProject = NULL, *lAAddConstruction = NULL, *lAAddGroup = NULL, *lADelGroup = NULL, *lADelProject = NULL,
            *lARemFromList = NULL, *lAAddToList = NULL,
            *lAProps = NULL, *lARights = NULL, *lACodeForming = NULL, *lARLO = NULL;

    if (ui->treeWidget->layoutDirection() == Qt::LeftToRight
            && pos.x() < ui->treeWidget->indentation()
            || ui->treeWidget->layoutDirection() == Qt::RightToLeft
            && pos.x() > ui->treeWidget->width() - ui->treeWidget->indentation()) {
        lAExpandAll = lMenu.addAction(tr("Expand all"));
        lACollapseAll = lMenu.addAction(tr("Collapse all"));
    } else {
        if (mDisplayType != DTShowSelect) {
            lADocuments = lMenu.addAction(tr("Documents"));
            lMenu.setDefaultAction(lADocuments);
            lMenu.addSeparator();

            if (gUserRight->CanInsert("proj_group", "name")) {
                lAAddGroup = lMenu.addAction(tr("New group..."));
            }
            if (gUserRight->CanInsertAnyColumn("v_project")) {
                lAAddProject = lMenu.addAction(tr("New project..."));
                if (lCurItem
                        && lCurItem->ProjectConst()->Type() == ProjectData::PDProject) {
                    lAAddConstruction = lMenu.addAction(tr("New construction..."));
                }
            }

            if (lCurItem) {
                if (lCurItem->ProjectRef()->ProjListConst().isEmpty()) {
                    if (lCurItem->ProjectConst()->Type() == ProjectData::PDGroup) {
                        if (gUserRight->CanDelete("proj_group")) {
                            lADelGroup = lMenu.addAction(tr("Delete group"));
                        }
                    } else if (lCurItem->ProjectConst()->Type() == ProjectData::PDProject) {
                        if (/* it can be too long static_cast<ProjectTreeItem *>(ui->treeWidget->currentItem())->GetProjectData()->PlotListConst().isEmpty()*/
                                gUserRight->CanDelete("v_project")) {
                            if (!lCurItem->ProjectConst()->IdParentProject()) {
                                lADelProject = lMenu.addAction(tr("Delete project"));
                            } else {
                                lADelProject = lMenu.addAction(tr("Delete construction"));
                            }
                        }
                    }
                }

                if (static_cast<ProjectTree::PTShowMode>(ui->cbProjTreeMode->currentIndex()) == ProjectTree::PTMyList
                        && ui->lineEdit->text().isEmpty()
                        || !ui->lineEdit->text().isEmpty()
                        && lCurItem->ProjectConst()->Type() == ProjectData::PDProject
                        && lCurItem->ProjectConst()->InUserList()) {
                    lMenu.addSeparator();
                    lARemFromList = lMenu.addAction(tr("Remove from list"));
                }
                if (static_cast<ProjectTree::PTShowMode>(ui->cbProjTreeMode->currentIndex()) != ProjectTree::PTMyList
                        && (lCurItem->ProjectConst()->Type() == ProjectData::PDGroup
                            || lCurItem->ProjectConst()->Type() == ProjectData::PDProject
                                && !lCurItem->ProjectConst()->InUserList())) {
                    lMenu.addSeparator();
                    lAAddToList = lMenu.addAction(tr("Add to my list"));
                }
            }

            bool lSeparatorAdded = false;

            if (lCurItem) {
                if (lCurItem->ProjectConst()->Type() == ProjectData::PDProject) {
                    if (!lSeparatorAdded) {
                        lMenu.addSeparator();
                        lSeparatorAdded = true;
                    }
                    lAProps = lMenu.addAction(/*QIcon(":/some/ico/ico/security.png"), */tr("Properties..."));
                    if (gUserRight->CanSelect("v_project_right")) {
                        lARights = lMenu.addAction(QIcon(":/some/ico/ico/security.png"), tr("Rights..."));
                    }
                } else if (lCurItem->ProjectConst()->Type() == ProjectData::PDGroup
                           && gUserRight->CanUpdate("proj_group", "name")) {
                    if (!lSeparatorAdded) {
                        lMenu.addSeparator();
                        lSeparatorAdded = true;
                    }
                    lAProps = lMenu.addAction(/*QIcon(":/some/ico/ico/security.png"), */tr("Rename group..."));
                }

                if (lCurItem->ProjectConst()->Type() == ProjectData::PDProject
                        && !lCurItem->ProjectConst()->IdParentProject()) {
                    if (!lSeparatorAdded) {
                        lMenu.addSeparator();
                        lSeparatorAdded = true;
                    }
                    lACodeForming = lMenu.addAction(tr("Code forming..."));
                }
            }
        }

        if (gSettings->Features.CanRLO) {
            lMenu.addSeparator();
            lARLO = lMenu.addAction(tr("Right to left"));
            lARLO->setCheckable(true);
            lARLO->setChecked(ui->treeWidget->layoutDirection() == Qt::RightToLeft);
        }
    }

    if (lARes = lMenu.exec(QCursor::pos())) {
        if (lARes == lAExpandAll) {
            ui->treeWidget->expandAll();
        } else if (lARes == lACollapseAll) {
            ui->treeWidget->collapseAll();
        } else if (lARes == lADocuments) {
            ShowPlotList();
        } else if (lARes == lAAddProject) {
            ProjectData *lParentGroup = NULL;
            if (lCurItem)
                lParentGroup = lCurItem->ProjectRef();
            while (lParentGroup && lParentGroup->Type() == ProjectData::PDProject)
                lParentGroup = lParentGroup->Parent();
            int lNewIdProj = ProjectData::NewProject(lParentGroup, this);
            if (lNewIdProj) {
                if (static_cast<ProjectTree::PTShowMode>(ui->cbProjTreeMode->currentIndex()) == ProjectTree::PTMyList) {
                    ProjectData *lProject = gProjects->FindByIdProject(lNewIdProj);
                    if (lProject) lProject->AddToMyList(false);
                }
                ui->treeWidget->SetSelectedProject(lNewIdProj); // make selected in this window
            }
        } else if (lARes == lAAddConstruction) {
            int lNewIdConstr = lCurItem->ProjectRef()->NewConstruction(this);
            if (lNewIdConstr) {
                ui->treeWidget->SetSelectedProject(lNewIdConstr); // make selected in this window
            }
        } else if (lARes == lAAddGroup) {
            int lNewIdGroup = ProjectData::NewGroup(this);
            if (lNewIdGroup) {
                ui->treeWidget->SetSelectedGroup(lNewIdGroup); // make selected in this window
            }
        } else if (lARes == lADelGroup
                   || lARes == lADelProject) {
            //QMutexLocker lLocker(gMainWindow->UpdateMutex());
            MyMutexLocker lLocker(gMainWindow->UpdateMutex(), 0);
            //if (!lLocker.IsLocked()) return; // something wrong
            if (lLocker.IsLocked() && mProjectData->RemoveFromDB()) {
                if (mProjectData->Parent()) {
                    mProjectData->Parent()->ProjListRef().removeAll(mProjectData);
                } else {
                    gProjects->ProjListRef().removeAll(mProjectData);
                }
                delete mProjectData;
                mProjectData = NULL;
                // do not re-read database, just update lists
                emit gProjects->ProjectListNeedUpdate();
            }
        } else if (lARes == lARemFromList) {
            if (lCurItem->ProjectRef()->RemoveFromMyList()) {
                emit gProjects->ProjectListNeedUpdate();
            }

        } else if (lARes == lAAddToList) {
            if (lCurItem->ProjectRef()->AddToMyList(true)) {
                emit gProjects->ProjectListNeedUpdate();
            }
        } else if (lARes == lAProps) {
            lCurItem->ProjectRef()->ShowProps(this);
        } else if (lARes == lARights) {
            gMainWindow->ShowProjectRights(lCurItem->ProjectConst());
        } else if (lARes == lACodeForming) {
            CodeFormingDlg w(lCurItem->ProjectRef(), this);
            if (w.exec() == QDialog::Accepted) {
                // the best what we can do - do nothing
            }
        } else if (lARes == lARLO) {
            if (ui->treeWidget->layoutDirection() == Qt::RightToLeft) {
                ui->treeWidget->setLayoutDirection(Qt::LeftToRight);
            } else {
                ui->treeWidget->setLayoutDirection(Qt::RightToLeft);
            }
            SaveWindowDefaults();
        }
    }
}

void ProjectListDlg::on_pbDocs_clicked() {
    ShowPlotList();
}

void ProjectListDlg::on_pbProjectCard_clicked() {
    ShowProjectCard();
}

void ProjectListDlg::on_pbIncoming_clicked() {
    mProjectData = ui->treeWidget->GetSelectedProject();
    if (mProjectData) {
        gSettings->RunForm("letters", QString::number(mProjectData->Id()));
    }
}

void ProjectListDlg::on_pbContracts_clicked() {
    mProjectData = ui->treeWidget->GetSelectedProject();
    if (mProjectData) {
        gSettings->RunForm("contracts", "byproject " + QString::number(mProjectData->Id()));
    }
}

void ProjectListDlg::on_pbSubContracts_clicked() {
    mProjectData = ui->treeWidget->GetSelectedProject();
    if (mProjectData) {
        gSettings->RunForm("contrSm", "byproject " + QString::number(mProjectData->Id()));
    }
}
