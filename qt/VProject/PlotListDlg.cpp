#include "PlotListDlg.h"
#include "ui_PlotListDlg.h"
#include "GlobalSettings.h"
#include "DocTreeSettings.h"
#include "SelectColumnsDlg.h"

#include "MainWindow.h"

#include "common.h"

#include "../PlotLib/DwgData.h"
#include "../PlotLib/DwgLayoutData.h"

#include "../ProjectLib/ProjectListDlg.h"

#include <QInputDialog>
#include <QMenuBar>
#include <QScrollBar>
#include <QPlainTextEdit>

PlotListDlg::PlotListDlg(DisplayType aDisplayType, PlotData *aSelectedPlotData, PlotHistoryData *aSelectedHistory, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotListDlg), mDisplayType(aDisplayType),
    mShowNonEmptyOnly(true),
    mJustStarted(true), mJustLoaded(false), mDocTreeScrollPos(0), mDocTreeScrollPosHoriz(0)
{
    ui->setupUi(this);

    ui->cbHideCancelled->blockSignals(true);
    ui->cbComplect->blockSignals(true);
    ui->cbList->blockSignals(true);
    ui->cbSecondLevel->blockSignals(true);
    ui->twType->blockSignals(true);

    // default value, can change if selected document (or history)
    GlobalSettings::DocumentTreeStruct::SLT lSLT = gSettings->DocumentTree.SecondLevelType;

//    if (aSelectedPlotData) {
//        gLogger->ShowError(this, tr("Documents list"), "Document " + QString::number(aSelectedPlotData->Id()));
//    }
//    if (aSelectedHistory) {
//        gLogger->ShowError(this, tr("Documents list"), "History " + QString::number(aSelectedHistory->Num()));
//    }

    if (aSelectedPlotData) {
        QList<int> lIds;
        if (!aSelectedPlotData->Working()) {
            // non working
            // show versions at second level
            lSLT = GlobalSettings::DocumentTreeStruct::SLTVersions;

//            lSelectedIds.clear();
//            foreach (PlotData *lPlotVersions, aSelectedPlotData->VersionsConst()) {
//                if (lPlotVersions->Working()
//                        && !lPlotVersions->Deleted()) {
//                    lSelectedIds.append(lPlotVersions->Id()); // expand working version
//                    break;
//                }
//            }
            // expanded
            lIds.append(aSelectedPlotData->IdCommon());
            ui->twDocs->SetExpandedPlotIdsCommon(lIds);

            // selected
            ui->twDocs->SetSelectedPlotId(aSelectedPlotData->Id());

            if (aSelectedHistory) {
                aSelectedPlotData->InitIdDwgMax();
                if (aSelectedPlotData->DwgVersionMax() != aSelectedHistory->Num()) {
                    // show history
                    gMainWindow->ShowPlotHist(aSelectedPlotData, aSelectedHistory->Num(), false /* auto update */, false /* no modal */);
                }
            }

        } else {
            // working
            bool lHistory = false;

            if (aSelectedHistory) {
                aSelectedPlotData->InitIdDwgMax();
                if (aSelectedPlotData->DwgVersionMax() != aSelectedHistory->Num()) {
                    // show history at second level
                    lSLT = GlobalSettings::DocumentTreeStruct::SLTHistory;

                    // expanded
                    lIds.append(aSelectedPlotData->IdCommon());
                    ui->twDocs->SetExpandedPlotIdsCommon(lIds);

                    // select history
                    lIds.append(aSelectedHistory->Id());
                    ui->twDocs->SetSelectedHistoryIds(lIds);

                    lHistory = true;
                }
            }

            if (!lHistory) {
                // selected
                ui->twDocs->SetSelectedPlotId(aSelectedPlotData->Id());
            }
        }
    }

    ui->cbSecondLevel->setCurrentIndex(lSLT);
    //ui->twDocs->SetSecondLevelType(lSLT);

    InitInConstructor();
}

PlotListDlg::PlotListDlg(QSettings &aSettings, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::PlotListDlg),
    mShowNonEmptyOnly(true),
    mJustStarted(true), mJustLoaded(true), mDocTreeScrollPos(0), mDocTreeScrollPosHoriz(0)
{
    ui->setupUi(this);

    //int i;
    ui->cbHideCancelled->blockSignals(true);
    ui->cbComplect->blockSignals(true);
    ui->cbList->blockSignals(true);
    ui->cbSecondLevel->blockSignals(true);
    ui->twType->blockSignals(true);

    SetProjectData(gProjects->FindByIdProject(aSettings.value("Project", 0).toInt()));
    //if (!ui->twType->ProjectConst()) return;

    aSettings.beginGroup("Parameters");
    LoadAdditionalSettings(aSettings); // show-non-empy and hide-cancelled
    aSettings.endGroup();
    //LoadSettings(aSettings);
    mDocTreeScrollPos = aSettings.value("ScrollPos").toInt();
    mDocTreeScrollPosHoriz = aSettings.value("ScrollPosHoriz").toInt();

    int lArea, lId;
    lArea = aSettings.value("TreeArea", 0).toInt();
    lId = aSettings.value("TreeId", 0).toInt();
    ui->twType->SetSelected(gTreeData->FindById(lArea, lId));

    mDisplayType = (DisplayType) aSettings.value("DisplayType", (int) DTShowFull).toInt();

    mComplectFromSettings = aSettings.value("Complect").toString();
    ui->twType->SetComplect(mComplectFromSettings);
    ui->twDocs->SetComplect(mComplectFromSettings);

    mListFromSettings = aSettings.value("List").toString();
/*    QString lComplect;
    lComplect = aSettings.value("Complect").toString();

    QString lList = aSettings.value("List").toString();
    if (!lList.isEmpty()) {
        ui->cbList->clear();
        ui->cbList->addItem(lList);
        ui->cbList->setCurrentText(lList);
    }*/

    ui->cbSecondLevel->setCurrentIndex(aSettings.value("SecondLevelType", gSettings->DocumentTree.SecondLevelType).toInt());
    ui->twDocs->SetSecondLevelType((GlobalSettings::DocumentTreeStruct::SLT) ui->cbSecondLevel->currentIndex());

    ui->twDocs->LoadFromSettings(aSettings);

    InitInConstructor();
/*    if (lComplect.isEmpty()) {
        ui->cbComplect->setCurrentIndex(0);
    } else {
        ui->cbComplect->setCurrentText(lComplect);
    }*/
}


PlotListDlg::~PlotListDlg() {
    delete ui;
}

void PlotListDlg::SaveState(QSettings &aSettings) {
    int i;

    if (ui->twType->ProjectConst()) {
        aSettings.setValue("Project", ui->twType->ProjectConst()->Id());
    } else {
        aSettings.setValue("Project", 0);
    }

    //SaveAdditionalSettings(aSettings);
    SaveSettings(aSettings);

    aSettings.setValue("ScrollPos", ui->twDocs->verticalScrollBar()->value());
    aSettings.setValue("ScrollPosHoriz", ui->twDocs->horizontalScrollBar()->value());

    TreeDataRecord * lTreeDataRecord = ui->twType->GetSelected();

    if (lTreeDataRecord) {
        aSettings.setValue("TreeArea", lTreeDataRecord->Area());
        aSettings.setValue("TreeId", lTreeDataRecord->Id());
    } else {
        aSettings.setValue("TreeArea", (int) 0);
        aSettings.setValue("TreeId", (int) 0);
    }

    aSettings.setValue("DisplayType", mDisplayType);

    if (!ui->cbComplect->currentIndex()) {
        aSettings.setValue("Complect", "");
    } else {
        aSettings.setValue("Complect", ui->cbComplect->currentText());
    }

    if (!ui->cbList->currentIndex()) {
        aSettings.setValue("List", "");
    } else {
        aSettings.setValue("List", ui->cbList->currentText());
    }

    aSettings.setValue("SecondLevelType", ui->cbSecondLevel->currentIndex());

    //QList<int> lSelectedPlotIds, lSelectedLayoutIds, lSelectedHistoryIds;
    QList<QTreeWidgetItem *> lSelected = ui->twDocs->selectedItems();
    QStringList lSelectedPlotIdsCommon, lSelectedLayoutIds, lSelectedHistoryIds, lSelectedAddFilesIds;
    QStringList lExpendedPlotIdsCommon;

    for (i = 0; i < lSelected.length(); i++) {
        if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()) {
            lSelectedPlotIdsCommon.append(QString::number(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()->IdCommon()));
        } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()) {
            lSelectedLayoutIds.append(QString::number(static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()->Id()));
        } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()) {
            lSelectedHistoryIds.append(QString::number(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()->Id()));
        } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotAddFileConst()) {
            lSelectedAddFilesIds.append(QString::number(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotAddFileConst()->Id()));
        }
    }

    for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
        if (ui->twDocs->topLevelItem(i)->isExpanded()) {
            lExpendedPlotIdsCommon.append(QString::number(static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i))->PlotConst()->IdCommon()));
        }
    }

    aSettings.setValue("SelectedDocs", lSelectedPlotIdsCommon.join(';'));
    aSettings.setValue("SelectedLayouts", lSelectedLayoutIds.join(';'));
    aSettings.setValue("SelectedHistory", lSelectedHistoryIds.join(';'));
    aSettings.setValue("SelectedAddFiles", lSelectedAddFilesIds.join(';'));

    aSettings.setValue("ExpandedDocs", lExpendedPlotIdsCommon.join(';'));

    // save current item
    if (ui->twDocs->currentItem()) {
        if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()) {
            aSettings.setValue("CurrentItemType", 0);
            aSettings.setValue("CurrentItemId", static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()->IdCommon());
        } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->DwgLayoutConst()) {
            aSettings.setValue("CurrentItemType", GlobalSettings::DocumentTreeStruct::SLTLayouts);
            aSettings.setValue("CurrentItemId", static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->DwgLayoutConst()->Id());
        } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryConst()) {
            aSettings.setValue("CurrentItemType", GlobalSettings::DocumentTreeStruct::SLTHistory);
            aSettings.setValue("CurrentItemId", static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryConst()->Id());
        } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotAddFileConst()) {
            aSettings.setValue("CurrentItemType", GlobalSettings::DocumentTreeStruct::SLTAddFiles);
            aSettings.setValue("CurrentItemId", static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotAddFileConst()->Id());
        }
        aSettings.setValue("CurrentColumn", ui->twDocs->currentColumn());
    }
}

void PlotListDlg::SetProjectData(ProjectData *aProjectData) {
    ui->twType->SetProjectData(aProjectData);
    ui->twDocs->SetProjectData(aProjectData);
    ShowWindowTitle();
    if (aProjectData) {
        ui->leIdProject->setText(QString::number(aProjectData->Id()));
        ui->leProjName->setText(aProjectData->FullShortName());
    } else {
        ui->leIdProject->setText("");
        ui->leProjName->setText("");
    }
}

const ProjectData *PlotListDlg::ProjectDataConst() const {
    return ui->twType->ProjectConst();
}

//void PlotListDlg::SetSelectedPlot(PlotData * aPlotData) {
//}

void PlotListDlg::GetSelectedPlots(QList<PlotData *> &lPlotList)
{
    int i;
    QList<QTreeWidgetItem *> selected = ui->twDocs->selectedItems();
    for (i = 0; i < selected.length(); i++) {
        lPlotList.append(static_cast<PlotListTreeItem *>(selected.at(i))->PlotRef());
    }
}

PlotData * PlotListDlg::SelectedPlot() {
    if (ui->twDocs->currentItem()) {
        return static_cast<PlotListTreeItem *> (ui->twDocs->currentItem())->PlotRef();
    } else {
        return NULL;
    }
}

void PlotListDlg::SetSecondLevelType(GlobalSettings::DocumentTreeStruct::SLT aSLT) {
    ui->cbSecondLevel->setCurrentIndex(aSLT);
}

void PlotListDlg::InitInConstructor() {
    if (mDisplayType != DTShowSelectMany
            && mDisplayType != DTShowSelectOne) {
        // OK-Cancle
        ui->buttonBox->setVisible(false);
    }

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->twDocs->setSelectionMode((mDisplayType == DTShowSelectOne)?QAbstractItemView::SingleSelection:QAbstractItemView::ExtendedSelection);
    //ui->twDocs->InitColumns((mDisplayType == DTShowSelectOne)?PLTSingleSelMode:0);
    if (mDisplayType == DTShowSelectOne) {
        // accept() owner
        ui->twDocs->SetDialogButtonBox(ui->buttonBox);
    }

    ui->leIdProject->setValidator(new QIntValidator(1, 1e9, this));

    //ui->twDocs->header()->setMaximumSectionSize(15);

    // it is line for splitter
    // ----------------------------------------------
    QSplitterHandle *handle = ui->splitter->handle(1);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    QFrame *line = new QFrame(handle);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);


    handle = ui->splitter->handle(2);
    layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    line = new QFrame(handle);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);


    //ui->splitter->setStretchFactor(1, 100);
    // ----------------------------------------------

    // ----------------------------------------------
//    QMenuBar * lMenuBar = new QMenuBar(this);
//    QMenu * lMenu;
//    if (mDisplayType != DTShowSelectMany
//            && mDisplayType != DTShowSelectOne) {

//        /*lMenu = lMenuBar->addMenu(QIcon(), tr("Documents"));

//        lMenu->addAction(ui->actionEdit);
//        lMenu->addAction(ui->actionView);
//        lMenu->addAction(ui->actionNew);*/
//    }

//    lMenu = lMenuBar->addMenu(QIcon(), tr("View"));
//    lMenu->addAction(ui->actionView_history);

//    ui->layoutForMenuBar->setMenuBar(lMenuBar);


    connect(gProjects, SIGNAL(PlotListBeforeUpdate(int)), this, SLOT(OnPlotListBeforeUpdate(int)));
    connect(gProjects, SIGNAL(PlotListNeedUpdate(int)), this, SLOT(OnPlotListNeedUpdate(int)));

    connect(gProjects, SIGNAL(PlotsNamedListNeedUpdate(PlotNamedListData *)), this, SLOT(OnPlotsNamedListNeedUpdate(PlotNamedListData *)));

    connect(gSettings, SIGNAL(DocTreeSettingsChanged()), this, SLOT(DoSettingsChanged()));
}

void PlotListDlg::ShowWindowTitle() {
    if (ui->twType->ProjectConst()) {
        if (qobject_cast<QMdiSubWindow *> (parent())) {
            switch (gSettings->DocumentTree.WindowTitleType) {
            case GlobalSettings::DocumentTreeStruct::WNDTLong:
                setWindowTitle(ui->twType->ProjectConst()->FullShortName());
                break;
            case GlobalSettings::DocumentTreeStruct::WNDTShort:
                setWindowTitle(ui->twType->ProjectConst()->ShortNameConst());
                break;
            case GlobalSettings::DocumentTreeStruct::WNDTNoGroup:
                setWindowTitle(ui->twType->ProjectConst()->FullShortName(true));
                break;
            }
        } else {
            switch (gSettings->DocumentTree.WindowTitleType) {
            case GlobalSettings::DocumentTreeStruct::WNDTLong:
                setWindowTitle(ui->twType->ProjectConst()->FullShortName() + " - " + gSettings->BaseName);
                break;
            case GlobalSettings::DocumentTreeStruct::WNDTShort:
                setWindowTitle(ui->twType->ProjectConst()->ShortNameConst() + " - " + gSettings->BaseName);
                break;
            case GlobalSettings::DocumentTreeStruct::WNDTNoGroup:
                setWindowTitle(ui->twType->ProjectConst()->FullShortName(true) + " - " + gSettings->BaseName);
                break;
            }
        }
    } else {
        if (qobject_cast<QMdiSubWindow *> (parent())) {
            setWindowTitle("");
        } else {
            setWindowTitle(gSettings->BaseName);
        }
    }
}

void PlotListDlg::ShowData() {
    ProjectData * lProject = ui->twType->ProjectRef();

    // complect
    bool lNeedUnblock = false;
    if (!ui->cbComplect->signalsBlocked()) {
        ui->cbComplect->blockSignals(true);
        lNeedUnblock = true;
    }

    QString lSelected;
    if (!mComplectFromSettings.isEmpty()) {
        lSelected = mComplectFromSettings;
        mComplectFromSettings.clear();
    } else {
        lSelected = ui->cbComplect->currentText();
    }

    ui->cbComplect->clear();
    if (!lProject || lProject->ComplectListConst().isEmpty()) {
        ui->lblComplect->setVisible(false);
        ui->cbComplect->setVisible(false);
    } else {
        ui->lblComplect->setVisible(true);
        ui->cbComplect->setVisible(true);
        ui->cbComplect->addItem(tr("All"));
        ui->cbComplect->addItems(lProject->ComplectListConst());
        ui->cbComplect->setCurrentText(lSelected);
    }

    // list
    bool lNeedUnblock2 = false;
    if (!ui->cbList->signalsBlocked()) {
        ui->cbList->blockSignals(true);
        lNeedUnblock2 = true;
    }

    if (!mListFromSettings.isEmpty()) {
        lSelected = mListFromSettings;
        mListFromSettings.clear();
    } else {
        lSelected = ui->cbList->currentText();
    }

    ui->cbList->clear();
    ui->cbList->addItem(tr("(no list)"));
    if (lProject) {
        foreach (PlotNamedListData * lNamedList, lProject->NamedListsConst()) {
            ui->cbList->addItem(lNamedList->NameConst(), QVariant::fromValue(lNamedList));
        }
        ui->cbList->setCurrentText(lSelected);
    }

    // populate
    ui->twType->SetList(ui->cbList->currentData().value<PlotNamedListData *>());
    ui->twDocs->SetList(ui->cbList->currentData().value<PlotNamedListData *>());
    ui->twType->PopulateTree();

    //-------------------
    if (lNeedUnblock)
        ui->cbComplect->blockSignals(false);

    if (lNeedUnblock2)
        ui->cbList->blockSignals(false);
}

void PlotListDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {

        ShowWindowTitle();

        if (ReadVersion < CurrentVersion) {
            // default sizes and columns visibility
            QList<int> lSizes;
            int i;
            lSizes = ui->splitter->sizes();
            i = lSizes.at(0) - 200;
            lSizes.replace(0, lSizes.at(0) - i);
            lSizes.replace(1, lSizes.at(1) + i);
            lSizes.replace(2, 0);
            ui->splitter->setSizes(lSizes);

//            ui->twDocs->HideColumn(ui->twDocs->colIdCommon);
//            ui->twDocs->HideColumn(ui->twDocs->colIdHist);
//            ui->twDocs->HideColumn(ui->twDocs->colHist);
//            ui->twDocs->HideColumn(ui->twDocs->colHistOrigin);
//            ui->twDocs->HideColumn(ui->twDocs->colCancelDate);
//            ui->twDocs->HideColumn(ui->twDocs->colCancelUser);
//            ui->twDocs->HideColumn(ui->twDocs->colSentBy);
//            ui->twDocs->HideColumn(ui->twDocs->colBlockName);
//            ui->twDocs->HideColumn(ui->twDocs->colCreated);
//            ui->twDocs->HideColumn(ui->twDocs->colCreatedBy);
        }


        ui->twDocs->SetSecondLevelType((GlobalSettings::DocumentTreeStruct::SLT) ui->cbSecondLevel->currentIndex());

        // loaded in LoadAdditionalSettings
        ui->twType->SetHideEmpty(mShowNonEmptyOnly);
        ui->twType->SetHideCancelled(ui->cbHideCancelled->isChecked());
        ui->twDocs->SetHideCancelled(ui->cbHideCancelled->isChecked());
        //
        if (mJustLoaded) {
            ShowData();
        } else {
            if (ui->twType->ProjectConst()) ui->twType->ProjectRef()->ReinitLists();
        }

        if (!ui->twType->currentItem()
                && ui->twType->topLevelItemCount()) {
            // make first item current
            ui->twType->topLevelItem(0)->setSelected(true);
            ui->twType->setCurrentItem(ui->twType->topLevelItem(0));
        }

        if (ui->cbHideCancelled->blockSignals(false)
                && ui->cbComplect->blockSignals(false)
                && ui->cbList->blockSignals(false)
                && ui->cbSecondLevel->blockSignals(false)
                && ui->twType->blockSignals(false)) { // was blocked; it is must be true
            if (ui->twType->currentItem()) {
                ui->twDocs->Populate(static_cast<PlotTreeItem *> (ui->twType->currentItem()));
                if (mDocTreeScrollPos) {
                    ui->twDocs->verticalScrollBar()->setValue(mDocTreeScrollPos);
                    mDocTreeScrollPos = 0;
                }
                if (mDocTreeScrollPosHoriz) {
                    ui->twDocs->horizontalScrollBar()->setValue(mDocTreeScrollPosHoriz);
                    mDocTreeScrollPosHoriz = 0;
                }
            }
        } else {
            gLogger->ShowError(this, tr("Documents list"), "Some signals was not blocked!\r\nINTERNAL ERROR");
        }

        if (mJustLoaded) {
            mJustLoaded = false;
        }

        ui->twDocs->setFocus();

        mJustStarted = false;
    }
}

void PlotListDlg::on_twType_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    ui->twDocs->clear();
    if (current) {
        ui->twDocs->Populate(static_cast<PlotTreeItem *>(current));
    }
}

void PlotListDlg::on_toolButton_clicked() {
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    if (ui->twType->ProjectConst()) dSel.SetSelectedProject(ui->twType->ProjectConst()->Id());

    if (dSel.exec() == QDialog::Accepted && ui->twType->ProjectConst() != dSel.GetProjectData()) {
        ProjectData *lProject = dSel.GetProjectData();
        SetProjectData(lProject);
        lProject->ReinitLists();
//        ui->twType->PopulateTree();
//        if (!ui->twType->currentItem() && ui->twType->topLevelItemCount()) {
//            ui->twType->topLevelItem(0)->setSelected(true);
//            ui->twType->setCurrentItem(ui->twType->topLevelItem(0));
//        }
    }
}

void PlotListDlg::SaveAdditionalSettings(QSettings &aSettings) {
    aSettings.setValue("ShowNonEmpty", mShowNonEmptyOnly);
    aSettings.setValue("HideCancelled", ui->cbHideCancelled->isChecked());
}

void PlotListDlg::LoadAdditionalSettings(QSettings &aSettings) {
    mShowNonEmptyOnly = aSettings.value("ShowNonEmpty", true).toBool();
    ui->cbHideCancelled->setChecked(aSettings.value("HideCancelled", false).toBool());
}

void PlotListDlg::OnPlotListBeforeUpdate(int aIdProject) {
    if (mJustLoaded) return;
    if (mJustStarted) return;
    if (!aIdProject || aIdProject == ui->twType->IdProject()) {
        int i;
        QList<QTreeWidgetItem *> lSelected = ui->twDocs->selectedItems();
        QList<int> lSelectedPlotIds, lSelectedLayoutIds, lSelectedHistoryIds, lSelectedAddFilesIds;
        QList<int> lExpendedPlotIdsCommon;

        // null;
        ui->twDocs->SetSelectedPlotIdsCommon(lSelectedPlotIds);

        for (i = 0; i < lSelected.length(); i++) {
            if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()) {
                lSelectedPlotIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()->Id());
            } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()) {
                lSelectedLayoutIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()->Id());
            } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()) {
                lSelectedHistoryIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()->Id());
            } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotAddFileConst()) {
                lSelectedAddFilesIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotAddFileConst()->Id());
            }
        }

        for (i = 0; i < ui->twDocs->topLevelItemCount(); i++) {
            if (ui->twDocs->topLevelItem(i)->isExpanded()) {
                lExpendedPlotIdsCommon.append(static_cast<PlotListTreeItem *>(ui->twDocs->topLevelItem(i))->PlotConst()->IdCommon());
            }
        }

        ui->twDocs->SetSelectedPlotIds(lSelectedPlotIds);
        ui->twDocs->SetSelectedLayoutIds(lSelectedLayoutIds);
        ui->twDocs->SetSelectedHistoryIds(lSelectedHistoryIds);
        ui->twDocs->SetSelectedAddFilesIds(lSelectedAddFilesIds);
        ui->twDocs->SetExpandedPlotIdsCommon(lExpendedPlotIdsCommon);

        if (ui->twDocs->currentItem()) {
            int lCurrentItemType = 0, lCurrentItemId = 0;
            if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()) {
                lCurrentItemType = 0;
                lCurrentItemId = static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()->Id();
            } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->DwgLayoutConst()) {
                lCurrentItemType = GlobalSettings::DocumentTreeStruct::SLTLayouts;
                lCurrentItemId = static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->DwgLayoutConst()->Id();
            } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryConst()) {
                lCurrentItemType = GlobalSettings::DocumentTreeStruct::SLTHistory;
                lCurrentItemId = static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryConst()->Id();
            } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotAddFileConst()) {
                lCurrentItemType = GlobalSettings::DocumentTreeStruct::SLTAddFiles;
                lCurrentItemId = static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotAddFileConst()->Id();
            }
            ui->twDocs->SetCurrentItem(lCurrentItemType, lCurrentItemId, ui->twDocs->currentColumn());
        }

        mDocTreeScrollPos = ui->twDocs->verticalScrollBar()->value();
        mDocTreeScrollPosHoriz = ui->twDocs->horizontalScrollBar()->value();
    }
}

void PlotListDlg::OnPlotListNeedUpdate(int aIdProject) {
    if (mJustLoaded) return;
    if (!aIdProject || aIdProject == ui->twType->IdProject()) {
        ProjectData *lProjectData = gProjects->FindByIdProject(ui->twType->IdProject());
        SetProjectData(lProjectData);
        ShowData();
        if (mDocTreeScrollPos) {
            ui->twDocs->verticalScrollBar()->setValue(mDocTreeScrollPos);
            mDocTreeScrollPos = 0;
        }
        if (mDocTreeScrollPosHoriz) {
            ui->twDocs->horizontalScrollBar()->setValue(mDocTreeScrollPosHoriz);
            mDocTreeScrollPosHoriz = 0;
        }
    }
}

void PlotListDlg::OnPlotsNamedListNeedUpdate(PlotNamedListData *aNamedList) {
    if (ui->cbList->currentData().value<PlotNamedListData *>() == aNamedList) {
        ui->twType->PopulateTree();
    }
}

void PlotListDlg::DoSettingsChanged() {
    ShowWindowTitle();

    // save all and reshow - that's it
    OnPlotListBeforeUpdate(0);
    OnPlotListNeedUpdate(0);
}

void PlotListDlg::Accept() {
    if (mDisplayType == DTShowSelectMany
            || mDisplayType == DTShowSelectOne && ui->twDocs->currentItem()) accept();
}

void PlotListDlg::on_leIdProject_editingFinished() {
    int lNewIdProject = ui->leIdProject->text().toInt();
    if (!ui->twType->ProjectConst() || ui->twType->ProjectConst()->Id() != lNewIdProject) {
        SetProjectData(gProjects->FindByIdProject(lNewIdProject));
        ui->twType->PopulateTree();
        if (!ui->twType->currentItem() && ui->twType->topLevelItemCount()) {
            ui->twType->topLevelItem(0)->setSelected(true);
            ui->twType->setCurrentItem(ui->twType->topLevelItem(0));
        }
    }
}

void PlotSelect(int aIdProject, QList<PlotData *> &aPlotList) {
    PlotListDlg w(PlotListDlg::DTShowSelectMany, NULL, NULL);
    if (!aIdProject) {
        ProjectListDlg dSel(ProjectListDlg::DTShowSelect);

        dSel.SetSelectedProject(aIdProject);
        //dSel.SetMode(4);
        if (dSel.exec() != QDialog::Accepted) return;
        aIdProject = dSel.GetProjectData()->Id();
    }
    w.SetProjectData(gProjects->FindByIdProject(aIdProject));
    if (w.exec() == QDialog::Accepted) {
        w.GetSelectedPlots(aPlotList);
    }
}

void PlotListDlg::on_cbSecondLevel_currentIndexChanged(int index)
{
    ui->twDocs->SetSecondLevelType((GlobalSettings::DocumentTreeStruct::SLT) index);
    if (ui->twType->currentItem()) {
        ui->twDocs->Populate(static_cast<PlotTreeItem *>(ui->twType->currentItem()));
    }
}

void PlotListDlg::on_actionNew_triggered() {
    QList <QTreeWidgetItem *> selected = ui->twDocs->selectedItems();
    PlotListTreeItem * lSingleItem = NULL;

    PlotData * lPlotData = NULL;
    PlotHistoryData * lPlotHistoryData = NULL;

    if (gSettings->DocumentTree.SingleSelect == gSettings->DocumentTreeStruct::SSCurrent) {
        lSingleItem = static_cast<PlotListTreeItem *>(ui->twDocs->currentItem());
    } else if (selected.count() == 1) {
        lSingleItem = static_cast<PlotListTreeItem *>(selected.at(0));
    }

    if (lSingleItem) {
        if (lSingleItem->PlotRef()) {
            lPlotData = lSingleItem->PlotRef();
        } else if (lSingleItem->parent()
                   && static_cast<PlotListTreeItem *>(lSingleItem->parent())->PlotRef()) {
            lPlotData = static_cast<PlotListTreeItem *>(lSingleItem->parent())->PlotRef();
        }

        if (lSingleItem->PlotHistoryRef()) {
            lPlotHistoryData = lSingleItem->PlotHistoryRef();
        }
    }
    ui->twDocs->NewPlot(ui->twType->ProjectRef(), ui->twType->GetSelected(),
                        (!ui->cbComplect->currentIndex())?"":ui->cbComplect->currentText(),
                        lPlotData, lPlotHistoryData);
}

void PlotListDlg::on_actionRefresh_triggered() {
    if (ui->twType->ProjectConst()) ui->twType->ProjectRef()->ReinitLists();
}

void PlotListDlg::on_twType_customContextMenuRequested(const QPoint &pos)
{
    QMenu lMenu(this);

    lMenu.addAction(ui->actionNew);
    lMenu.addSeparator();
    lMenu.addAction(ui->actionShow_non_empty_only);
    ui->actionShow_non_empty_only->setChecked(mShowNonEmptyOnly);
    lMenu.exec(QCursor::pos());
}

void PlotListDlg::on_twDocs_itemSelectionChanged() {
    if (mJustLoaded) return;
    if (ui->twType->ProjectConst()
            && gProjects->IsPlotListInUpdate(ui->twType->ProjectConst()->Id())) return;

    int i, j, k;

    // item selection changed
    if (ui->twDocs->currentItem()) {
        // emit "new selected plot"
        if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryConst()) {
            emit gProjects->PlotHistoryBecameSelected(static_cast<PlotListTreeItem *>(ui->twDocs->currentItem()->parent())->PlotRef(),
                                                      static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotHistoryRef());
        } else if (static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotConst()) {
            emit gProjects->PlotBecameSelected(static_cast<PlotListTreeItem *>(ui->twDocs->currentItem())->PlotRef());
        }
    }

    QList<QTreeWidgetItem *> lSelected = ui->twDocs->selectedItems();
    QList<PlotAndHistoryData> lData;

    for (i = 0; i < lSelected.length(); i++) {
        if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()) {
            // static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryRef() IS NULL IN FACT
            lData.append(qMakePair(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotRef(), static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryRef()));
        } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()
                   && lSelected.at(i)->parent()) {
            lData.append(qMakePair(static_cast<PlotListTreeItem *>(lSelected.at(i)->parent())->PlotRef(), static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryRef()));
        }
    }

    emit gProjects->PlotsBecameSelected(lData);

    if (ui->splitter->sizes().at(2)) {
        switch (ui->twAddInfo->currentIndex()) {
        case 0:
            {
                QList <PlotAddFileData *> lAddFiles;
                bool lFound;

                for (i = 0; i < lData.length(); i++) {
                    if (lData.at(i).second) {
                        for (j = 0; j < lData.at(i).second->AddFilesConst().length(); j++) {
                            lFound = false;
                            for (k = 0; k < lAddFiles.length(); k++) {
                                if (lAddFiles.at(k)->IdLob() == lData.at(i).second->AddFilesConst().at(j)->IdLob()) {
                                    lFound = true;
                                    break;
                                }
                            }
                            if (!lFound) lAddFiles.append(lData.at(i).second->AddFilesConst().at(j));
                        }
                    } else {
                        for (j = 0; j < lData.at(i).first->AddFilesConst().length(); j++) {
                            lFound = false;
                            for (k = 0; k < lAddFiles.length(); k++) {
                                if (lAddFiles.at(k)->IdLob() == lData.at(i).first->AddFilesConst().at(j)->IdLob()) {
                                    lFound = true;
                                    break;
                                }
                            }
                            if (!lFound) lAddFiles.append(lData.at(i).first->AddFilesConst().at(j));
                        }
                    }
                }

                std::sort(lAddFiles.begin(), lAddFiles.end(),
                          [] (const PlotAddFileData * d1, const PlotAddFileData * d2) { return d1->NameConst().toLower() < d2->NameConst().toLower(); });

                ui->twFilesList->clear();
                for (i = 0; i < lAddFiles.length(); i++) {
                    QTreeWidgetItem *lItem = new QTreeWidgetItem();
                    lItem->setText(0, lAddFiles.at(i)->NameConst());
                    ui->twFilesList->addTopLevelItem(lItem);
                }
            }
            break;
        case 1:
            {
                QList<DwgLayoutData *> lLayouts;
//                QList<DwgLayoutBlockData *> lBlocks;
//                QList<DwgLBAttrData *> lAttrs;

                for (i = 0; i < lSelected.length(); i++) {
                    if (static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()) {
                        if (static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()->HasAnyProp()) {
                            lLayouts.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutRef());
                        }
                    } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()) {
                        for (j = 0; j < static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotRef()->LayoutsConst().length(); j++) {
                            if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotRef()->LayoutsConst().at(j)->HasAnyProp()) {
                                lLayouts.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotRef()->LayoutsConst().at(j));
                            }
                        }
                    }
                }

                int lOldCnt = ui->cbLayout->count();

                bool lOldSignalsState = ui->cbLayout->blockSignals(true);

                ui->cbLayout->clear();

                if (lLayouts.length() > 1) ui->cbLayout->addItem(tr("*ALL*"));
                for (i = 0; i < lLayouts.length(); i++) {
                    ui->cbLayout->addItem(lLayouts.at(i)->NameConst(), QVariant::fromValue(lLayouts.at(i)));
                }
                ui->wdLayout->setVisible(ui->cbLayout->count() > 1);
                if (ui->cbLayout->count()) ui->cbLayout->setCurrentIndex(0);

                ui->cbLayout->blockSignals(lOldSignalsState);

                if (lOldCnt || ui->cbLayout->count()) {
                    emit ui->cbLayout->currentIndexChanged(ui->cbLayout->currentIndex());
                }
            }
            break;
        }
    }
}

void PlotListDlg::on_cbComplect_currentIndexChanged(int index) {
    if (!index) {
        ui->twType->SetComplect("");
        ui->twDocs->SetComplect("");
    } else {
        ui->twType->SetComplect(ui->cbComplect->currentText());
        ui->twDocs->SetComplect(ui->cbComplect->currentText());
    }

    ui->twDocs->clear();
    if (!ui->twType->currentItem()
            || ui->twType->currentItem()->isHidden()) {
        if (ui->twType->topLevelItemCount()) {
            // it means - select first item
            ui->twType->topLevelItem(0)->setSelected(true);
            ui->twType->setCurrentItem(ui->twType->topLevelItem(0));
        }
    } else {
        ui->twDocs->Populate(static_cast<PlotTreeItem *>(ui->twType->currentItem()));
    }
}

void PlotListDlg::on_cbList_customContextMenuRequested(const QPoint &pos) {
    QMenu popup(this);
    QAction *aAdd = NULL, *aRename = NULL, *aRemove = NULL, *actRes;

    aAdd = popup.addAction(tr("New..."));

    if (ui->cbList->currentIndex() > 0) {
        aRename = popup.addAction(tr("Rename..."));
        aRemove = popup.addAction(tr("Delete..."));
    }

    if (actRes = popup.exec(QCursor::pos())) {
        if (actRes == aAdd) {
            QInputDialog lDlg(this);
            lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
            lDlg.setWindowTitle(tr("New list"));
            lDlg.setLabelText(tr("Enter name for new list"));
            if (lDlg.exec() == QDialog::Accepted) {
                int lId = 0;
                PlotNamedListData * lNamedList;
                if (lNamedList = PlotNamedListData::INSERT(lId, ui->twType->IdProject(), lDlg.textValue())) {
                    ui->twType->ProjectRef()->NamedListsRef().append(lNamedList);
                    ui->cbList->addItem(lNamedList->NameConst(), QVariant::fromValue(lNamedList));
                    ui->cbList->setCurrentText(lNamedList->NameConst());
                }
            }
        } else if (actRes == aRename) {
            QInputDialog lDlg(this);
            lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
            lDlg.setWindowTitle(tr("Rename list"));
            lDlg.setLabelText(tr("Enter new name for list") + "\" " + ui->cbList->currentText() + "\"");
            lDlg.setTextValue(ui->cbList->currentText());
            if (lDlg.exec() == QDialog::Accepted) {
                PlotNamedListData * lPlotNamedList = ui->cbList->currentData().value<PlotNamedListData *>();
                lPlotNamedList->setName(lDlg.textValue());
                if (lPlotNamedList->SaveData()) {
                    lPlotNamedList->CommitEdit();
                    ui->cbList->setItemData(ui->cbList->currentIndex(), lDlg.textValue(), Qt::DisplayRole);
                } else {
                    lPlotNamedList->RollbackEdit();
                }
            }
        } else if (actRes == aRemove) {
            if (QMessageBox::question(this, tr("Delete list"), tr("Do you want to delete list ") + "\"" + ui->cbList->currentText() + "\"?") == QMessageBox::Yes) {
                if (PlotNamedListData::DODELETE(ui->cbList->currentData().value<PlotNamedListData *>()->Id())) {
                    ui->twType->ProjectRef()->NamedListsRef().removeAll(ui->cbList->currentData().value<PlotNamedListData *>());
                    ui->cbList->removeItem(ui->cbList->currentIndex());
                }
            }
        }
    }


}

void PlotListDlg::on_cbList_currentIndexChanged(int index) {
    ui->twType->SetList(ui->cbList->currentData().value<PlotNamedListData *>());
    ui->twDocs->SetList(ui->cbList->currentData().value<PlotNamedListData *>());

    ui->twDocs->clear();
    if (!ui->twType->currentItem()
            || ui->twType->currentItem()->isHidden()) {
        if (ui->twType->topLevelItemCount()) {
            // it means - select first item
            ui->twType->topLevelItem(0)->setSelected(true);
            ui->twType->setCurrentItem(ui->twType->topLevelItem(0));
        }
    } else {
        ui->twDocs->Populate(static_cast<PlotTreeItem *>(ui->twType->currentItem()));
    }
}

void PlotListDlg::on_cbHideCancelled_toggled(bool checked) {
    ui->twType->SetHideCancelled(checked);
    ui->twDocs->SetHideCancelled(checked);

    ui->twDocs->clear();
    if (!ui->twType->currentItem()
            || ui->twType->currentItem()->isHidden()) {
        if (ui->twType->topLevelItemCount()) {
            // it means - select first item
            ui->twType->topLevelItem(0)->setSelected(true);
            ui->twType->setCurrentItem(ui->twType->topLevelItem(0));
        }
    } else {
        ui->twDocs->Populate(static_cast<PlotTreeItem *>(ui->twType->currentItem()));
    }
}

void PlotListDlg::on_actionShow_non_empty_only_triggered() {
    mShowNonEmptyOnly = !mShowNonEmptyOnly;
    ui->twType->SetHideEmpty(mShowNonEmptyOnly);
}

void PlotListDlg::on_splitter_splitterMoved(int pos, int index) {
    emit ui->twDocs->itemSelectionChanged();
}

void PlotListDlg::on_cbLayout_currentIndexChanged(int index) {
    int lOldCnt = ui->cbBlock->count();
    bool lOldSignalState = ui->cbBlock->blockSignals(true);
    ui->cbBlock->clear();
    if (index == -1) {
        ui->wdBlock->setVisible(false);
    } else {
        int i, j;
        DwgLayoutData *lLayout = ui->cbLayout->itemData(index).value<DwgLayoutData *>();
        QList <DwgLayoutBlockData *> lBlocks;
        if (lLayout) {
            // single layout
            for (j = 0; j < lLayout->BlocksConst().length(); j++) {
                if (lLayout->BlocksConst().at(j)->HasAnyProp()) {
                    lBlocks.append(lLayout->BlocksConst().at(j));
                }
            }
        } else {
            // TO DO: all layouts
        }
        if (lBlocks.length() > 1) ui->cbBlock->addItem(tr("*ALL*"));
        for (i = 0; i < lBlocks.length(); i++) {
            ui->cbBlock->addItem(lBlocks.at(i)->NameConst(), QVariant::fromValue(lBlocks.at(i)));
        }


    }
    ui->cbBlock->setVisible(ui->cbBlock->count() > 1);
    if (ui->cbBlock->count()) ui->cbBlock->setCurrentIndex(0);

    ui->cbBlock->blockSignals(lOldSignalState);

    if (lOldCnt || ui->cbBlock->count()) {
        emit ui->cbBlock->currentIndexChanged(ui->cbBlock->currentIndex());
    }
}

void PlotListDlg::on_cbBlock_currentIndexChanged(int index) {
    ui->twAttrs->setRowCount(0);
    if (index == -1) {
        ui->twAttrs->setVisible(false);
    } else {
        int i, j;

        ui->twAttrs->setVisible(true);

        DwgLayoutBlockData *lBlock = ui->cbBlock->itemData(index).value<DwgLayoutBlockData *>();
        QTableWidgetItem *lItem;
        if (lBlock) {
            // single block
            ui->twAttrs->setColumnHidden(0, true);
            for (j = 0; j < lBlock->AttrsConst().length(); j++) {
                ui->twAttrs->insertRow(ui->twAttrs->rowCount());
                lItem = new QTableWidgetItem(lBlock->AttrsConst().at(j)->PromptConst());
                lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                lItem->setBackgroundColor(palette().color(QPalette::Window));
                lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
                ui->twAttrs->setItem(ui->twAttrs->rowCount() - 1, 1, lItem);

                lItem = new QTableWidgetItem(lBlock->AttrsConst().at(j)->EncValueConst());
                lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                ui->twAttrs->setItem(ui->twAttrs->rowCount() - 1, 2, lItem);
            }
        } else {
            // TO DO: all blocks
        }

        ui->twAttrs->resizeColumnsToContents();
        ui->twAttrs->resizeRowsToContents();
        for (i = 0; i < ui->twAttrs->rowCount(); i++) {
            ui->twAttrs->setRowHeight(i, ui->twAttrs->rowHeight(i) - gSettings->Common.SubRowHeight);
        }
        for (i = 0; i < ui->twAttrs->columnCount() - 1; i++) {
            ui->twAttrs->setColumnWidth(i, ui->twAttrs->columnWidth(i) + gSettings->Common.AddColWidth);
        }
    }
}
