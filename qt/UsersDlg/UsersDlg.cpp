#include "UsersDlg.h"
#include "ui_UsersDlg.h"

#include "CustomerData.h"
#include "DepartData.h"
#include "UserRight.h"

#include "UserPropDlg.h"
#include "UserRightsDlg.h"
#include "ChangePassDlg.h"
#include "NewUserDlg.h"

#include <QTimer>
#include <QScrollBar>
#include <QMenu>
#include <QMessageBox>
#include <QClipboard>

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/HomeData.h"
#include "../VProject/WaitDlg.h"
#include "../VProject/MainWindow.h"

UsersDlg::UsersDlg(DisplayTypeEnum aDisplayType, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::UsersDlg),
    mJustStarted(true), mInShowData(false),
    mCheckForInclude(NULL), mpDataForCheckForInclude(NULL),
    mDisplayType(aDisplayType), mListFlags(0),
    mSelectedUserId(0), mSelectedUser(NULL),
    mExcludedUsers(NULL), mExcludedLogins(NULL)
{
    InitInConstructor();
}

UsersDlg::UsersDlg(QSettings &aSettings, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::UsersDlg),
    mJustStarted(true), mInShowData(false),
    mCheckForInclude(NULL), mpDataForCheckForInclude(NULL),
    mDisplayType(DTEdit), mListFlags(0),
    mSelectedUserId(0), mSelectedUser(NULL),
    mExcludedUsers(NULL), mExcludedLogins(NULL)
{
    InitInConstructor();
}

UsersDlg::~UsersDlg() {
    delete ui;
}

void UsersDlg::done(int r) {
    if (gHomeData->Get("NEED_UPDATE_RIGHTS").toInt() == 1) {
        WaitDlg w(this);
        w.show();
        w.SetMessage(tr("Updating users permissions..."));
        gUserRight->UpdateRightsOnServer();
    }
    QFCDialog::done(r);
}

void UsersDlg::InitInConstructor() {
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->pbUpdateRights->setVisible(false);

    ui->pbSelect->setVisible(mDisplayType == DTSelectOne || mDisplayType == DTSelectMany);
    ui->pbCancel->setVisible(mDisplayType == DTSelectOne || mDisplayType == DTSelectMany);

    ui->tbPlus->setVisible(mDisplayType != DTSelectOne && mDisplayType != DTSelectMany && gUserRight->CanCreateUser());
    ui->tbRights->setVisible(gUserRight->CanManageUser());
    ui->tbMinus->setVisible(mDisplayType != DTSelectOne && mDisplayType != DTSelectMany
            && gUserRight->CanUpdate(gUsers->TableName(), "disabled"));


    connect(ui->twMain->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(MainScrolled(int)));
    connect(ui->twCommon->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(CommonScrolled(int)));

    connect(ui->twMain->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(MainSortChanged(int, Qt::SortOrder)));
    connect(ui->twCommon->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(CommonSortChanged(int, Qt::SortOrder)));

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

    ui->splitter->setStretchFactor(1, 100);
    // ----------------------------------------------

    // always selected in normal color (not gray)
    QPalette lPalette = ui->twMain->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->twMain->setPalette(lPalette);
    ui->twCommon->setPalette(lPalette);
    // ---------------------------------

    connect(gUsers, SIGNAL(UsersNeedUpdate()), this, SLOT(ShowData()));
}

void UsersDlg::ShowData() {
    if (mInShowData) return;
    mInShowData = true;
    if (mJustStarted) {
        //ui->twCommon->setColumnHidden(4, !gUsers->HasCompany()); // company
        ui->twCommon->setColumnHidden(5, !gUsers->HasDepartment()); // department

        if (mDisplayType == DTEdit) {
            ui->twMain->setSelectionMode(QAbstractItemView::SingleSelection);
            ui->twCommon->setSelectionMode(QAbstractItemView::SingleSelection);
        } else if (mDisplayType == DTSelectMany) {
            ui->twMain->setSelectionMode(QAbstractItemView::ExtendedSelection);
            ui->twCommon->setSelectionMode(QAbstractItemView::ExtendedSelection);
        }
    }

    int i, j, cnt = 0, lIdCustomer, lFirstFired;

    bool lAnyCustomer = false, lHideFired = true;

    const QList<UserData *> &lUsers = gUsers->UsersConst();

    ui->twMain->setRowCount(0);
    ui->twCommon->setRowCount(0);

    int lMainSortInd, lCommonSortInd;
    Qt::SortOrder lMainSortOrder, lCommonSortOrder;

    lMainSortInd = ui->twMain->horizontalHeader()->sortIndicatorSection();
    lMainSortOrder = ui->twMain->horizontalHeader()->sortIndicatorOrder();

    lCommonSortInd = ui->twCommon->horizontalHeader()->sortIndicatorSection();
    lCommonSortOrder = ui->twCommon->horizontalHeader()->sortIndicatorOrder();

    ui->twMain->horizontalHeader()->blockSignals(true);
    ui->twMain->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    ui->twMain->horizontalHeader()->blockSignals(false);

    ui->twCommon->horizontalHeader()->blockSignals(true);
    ui->twCommon->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    ui->twCommon->horizontalHeader()->blockSignals(false);

    for (i = 0; i < lUsers.length(); i++) {
        UserData *lUser = lUsers.at(i);

        if (mExcludedUsers && mExcludedUsers->contains(lUser)) continue;
        if (mExcludedLogins && mExcludedLogins->contains(lUser->LoginConst())) continue;

        int lIncRes = 1;
        if (mCheckForInclude) lIncRes = (*mCheckForInclude) (lUser, mpDataForCheckForInclude);
        if (!lIncRes) continue;
        if (lIncRes == 1) {

            if (!ui->cbType->currentIndex() && lUser->Disabled()
                    || ui->cbType->currentIndex() == 1 && !lUser->Disabled()) continue;

            if (mListFlags
                    && !((mListFlags & UsersDlgShowGip) && lUser->IsGIP()
                         || (mListFlags & UsersDlgShowBoss) && lUser->IsBoss())) continue;

            if (!ui->leFilter->text().isEmpty()) {
                QString lFilter = ui->leFilter->text().toLower();

                if (!(lUser->NameConst().toLower().contains(lFilter)
                      || lUser->LoginConst().toLower().contains(lFilter)
                      || lUser->PlotNameConst().toLower().contains(lFilter)
                      || lUser->LongNameConst().toLower().contains(lFilter)
                      || lUser->EMailConst().toLower().contains(lFilter)
                      || lUser->Phone1Const().toLower().remove(QChar('-')).remove(QChar('(')).remove(QChar(')')).contains(lFilter)
                      || lUser->Phone2Const().toLower().remove(QChar('-')).remove(QChar('(')).remove(QChar(')')).contains(lFilter)
                      || lUser->Phone3Const().toLower().remove(QChar('-')).remove(QChar('(')).remove(QChar(')')).contains(lFilter))) continue;
            }
        }

        //--- add row
        ui->twMain->setRowCount(cnt + 1);
        ui->twCommon->setRowCount(cnt + 1);

        // left
        QTableWidgetItem *lItem = new QTableWidgetItem(lUser->NameConst());
        if (lUser->NameConst().contains(QRegExp("[א-ת]")))
            lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        else
            lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lItem->setData(Qt::UserRole, QVariant::fromValue(lUser));
        ui->twMain->setItem(cnt, 0, lItem);
        // right
        FillRightPart(cnt, lUser);

        if (!cnt) {
            lIdCustomer = lUser->IdCustomer();
            lFirstFired = lUser->Disabled();
        } else {
            if (lIdCustomer != lUser->IdCustomer()) lAnyCustomer = true;
            if (lFirstFired != lUser->Disabled()) lHideFired = false;
        }

        cnt++;
    }

    if (lMainSortInd != -1) {
        ui->twMain->horizontalHeader()->setSortIndicator(lMainSortInd, lMainSortOrder);
    } else if (lCommonSortInd != -1) {
        ui->twCommon->horizontalHeader()->setSortIndicator(lCommonSortInd, lCommonSortOrder);
    }

    for (i = 0; i < ui->twCommon->columnCount(); i++) {
        bool b = false;
        for (j = 0; j < ui->twCommon->rowCount(); j++) {
            if (ui->twCommon->item(j, i)
                    && !ui->twCommon->item(j, i)->text().isEmpty()) {
                b = true;
                break;
            }
        }
        ui->twCommon->setColumnHidden(i, !b);
    }
    ui->twCommon->setColumnHidden(3, lHideFired); // fired flag
    ui->twCommon->setColumnHidden(4, !lAnyCustomer); // different company name (usually it is the same)

    StyleSheetChangedInSescendant();

    if (mJustStarted) {
        mJustStarted = false;
        if (mDisplayType == DTSelectOne) {
            for (j = 0; j < ui->twMain->rowCount(); j++) {
                if (mSelectedUserId == ui->twMain->item(j, 0)->data(Qt::UserRole).value<UserData *>()->Id()) {
                    ui->twMain->item(j, 0)->setSelected(true);
                    break;
                }
            }
        } else if (mDisplayType == DTSelectMany) {
            for (i = 0; i < mSelectedUsers.length(); i++) {
                for (j = 0; j < ui->twMain->rowCount(); j++) {
                    if (mSelectedUsers.at(i)->NameConst() == ui->twMain->item(j, 0)->text()) {
                        ui->twMain->item(j, 0)->setSelected(true);
                        break;
                    }
                }
            }
        }
    }
    mInShowData = false;
}

void UsersDlg::StyleSheetChangedInSescendant() {
    ui->twMain->resizeRowsToContents();
    ui->twCommon->resizeColumnsToContents();
    ui->twCommon->resizeRowsToContents();

    int i;
    for (i = 0; i < ui->twCommon->rowCount(); i++) {
        ui->twCommon->setRowHeight(i, ui->twCommon->rowHeight(i) - gSettings->Common.SubRowHeight);
        ui->twMain->setRowHeight(i, ui->twCommon->rowHeight(i));
    }

    for (i = 0; i < ui->twCommon->columnCount(); i++) {
        ui->twCommon->setColumnWidth(i, ui->twCommon->columnWidth(i) + gSettings->Common.AddColWidth);
    }
}

void UsersDlg::SetCheckForInclude(int (*aCheckForInclude)(const UserData *, void *), void *apData) {
    mCheckForInclude = aCheckForInclude;
    mpDataForCheckForInclude = apData;
}

void UsersDlg::SetListFlags(uint aListFlags) {
    mListFlags = aListFlags;
}

void UsersDlg::SetExcludedUsers(const QList<UserData *> *aExcludedUsers) {
    mExcludedUsers = aExcludedUsers;
}

void UsersDlg::SetExcludedLogins(const QStringList *aExcludedLogins) {
    mExcludedLogins = aExcludedLogins;
}

void UsersDlg::SetSelectedUserId(int aSelectedUserId) {
    mSelectedUserId = aSelectedUserId;
}

UserData *UsersDlg::GetSelectedUser() const {
    return mSelectedUser;
}

const QList<UserData *> &UsersDlg::GetSelectedUsers() const {
    return mSelectedUsers;
}

void UsersDlg::FillRightPart(int aRow, const UserData *aUser) {
    int j = 0;
    QTableWidgetItem *lItem;

    lItem = new QTableWidgetItem(aUser->LoginConst());
    //lItem->setData(Qt::UserRole, QVariant::fromValue(aUser));
    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (!aUser->Disabled() && !aUser->HasLogin()) {
        lItem->setBackgroundColor(gSettings->Common.DisabledFieldColor);
        //lItem->setBackgroundColor(palette().color(QPalette::Window));
    }
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    if (gUsers->HasPlotname()) {
        lItem = new QTableWidgetItem(aUser->PlotNameConst());
        if (aUser->PlotNameConst().contains(QRegExp("[א-ת]")))
            lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        else
            lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->twCommon->setItem(aRow, j, lItem);
    }
    j++;

    lItem = new QTableWidgetItem(aUser->LongNameConst());
    if (aUser->LongNameConst().contains(QRegExp("[א-ת]")))
        lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    else
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem();
    if (aUser->Disabled())
        lItem->setCheckState(Qt::Checked);
    else
        lItem->setCheckState(Qt::Unchecked);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    // company
    if (aUser->IdCustomer()) {
        QString lCustomerName;
        CustomerData *lCustomer;
        if (lCustomer = gCustomers->GetCustomerById(aUser->IdCustomer())) lCustomerName = lCustomer->ShortNameConst();
        lItem = new QTableWidgetItem(lCustomerName);
    } else {
        lItem = new QTableWidgetItem(gHomeData->Get("NAME"));
    }
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    DepartData *lDepartment = gDeparts->FindById(aUser->IdDepartment());
    lItem = new QTableWidgetItem(lDepartment?lDepartment->NameConst():"");
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->JobTitleConst());
    if (aUser->JobTitleConst().contains(QRegExp("[א-ת]")))
        lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    else
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->EMailConst());
    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->Phone1Const());
    lItem->setTextAlignment(Qt::AlignCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->Phone2Const());
    lItem->setTextAlignment(Qt::AlignCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->Phone3Const());
    lItem->setTextAlignment(Qt::AlignCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->AddrConst());
    if (aUser->AddrConst().contains(QRegExp("[א-ת]")))
        lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    else
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->RoomConst());
    lItem->setTextAlignment(Qt::AlignCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    lItem = new QTableWidgetItem(aUser->BirthDateConst().toString("dd.MM.yyyy"));
    lItem->setTextAlignment(Qt::AlignCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;

    if (gUsers->HasHireDate()) {
        lItem = new QTableWidgetItem(aUser->HireDateConst().toString("dd.MM.yyyy"));
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->twCommon->setItem(aRow, j, lItem);
    }
    j++;

    if (gUsers->HasTrialPeriod()) {
        lItem = new QTableWidgetItem(aUser->TrialPeriod()?QString::number(aUser->TrialPeriod()):"");
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->twCommon->setItem(aRow, j, lItem);
    }
    j++;

    if (gUsers->HasEPH()) {
        lItem = new QTableWidgetItem(aUser->EPH()?QString::number(aUser->EPH()):"");
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->twCommon->setItem(aRow, j, lItem);
    }
    j++;

    lItem = new QTableWidgetItem(aUser->CommentsConst());
    if (aUser->CommentsConst().contains(QRegExp("[א-ת]")))
        lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    else
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->twCommon->setItem(aRow, j, lItem);
    j++;
}

//void UsersDlg::AfterFirstShowData() {
//    ui->twCommon->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
//}

void UsersDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        if (gHomeData->Get("NEED_UPDATE_RIGHTS").toInt() == 1) {
            WaitDlg w(this);
            w.show();
            w.SetMessage(tr("Updating users permissions..."));
            gUserRight->UpdateRightsOnServer();
            setFocus();
        }
        QTimer::singleShot(0, this, SLOT(ShowData()));
    }
}

void UsersDlg::SaveState(QSettings &aSettings) {
//    aSettings.setValue("ParamSetted", mParamSetted);
//    if (mParamSetted) {
//        aSettings.setValue("IdProject", mIdProject);
//        aSettings.setValue("WithConstr", mWithConstr);
//        aSettings.setValue("Login", mLogin);
//        aSettings.setValue("StartDate", mStartDate);
//        aSettings.setValue("EndDate", mEndDate);
//        aSettings.setValue("Grouping", mGrouping);
//    }
}

void UsersDlg::MainScrolled(int aValue) {
    ui->twCommon->verticalScrollBar()->setSliderPosition(aValue);
}

void UsersDlg::CommonScrolled(int aValue) {
    ui->twMain->verticalScrollBar()->setSliderPosition(aValue);
}

void UsersDlg::MainSortChanged(int logicalIndex, Qt::SortOrder order) {
    ui->twCommon->horizontalHeader()->blockSignals(true);
    ui->twCommon->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    ui->twCommon->horizontalHeader()->blockSignals(false);

    for (int i = 0; i < ui->twMain->rowCount(); i++) {
        UserData *lUser = ui->twMain->item(i, 0)->data(Qt::UserRole).value<UserData *>();
        if (lUser) {
            FillRightPart(i, lUser);
        } else {
            gUsers->InitUserList();
            break;
        }
    }
    emit ui->twMain->itemSelectionChanged();
}

void UsersDlg::CommonSortChanged(int logicalIndex, Qt::SortOrder order) {
    //QMessageBox::critical(NULL, "", QString::number(logicalIndex));
    ui->twMain->horizontalHeader()->blockSignals(true);
    ui->twMain->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    ui->twMain->horizontalHeader()->blockSignals(false);

    for (int i = 0; i < ui->twMain->rowCount(); i++) {
        UserData *lUser = gUsers->FindByLogin(ui->twCommon->item(i, 0)->text());
        ui->twMain->item(i, 0)->setText(lUser->NameConst());
        ui->twMain->item(i, 0)->setData(Qt::UserRole, QVariant::fromValue(lUser));
    }
    emit ui->twCommon->itemSelectionChanged();
}

void UsersDlg::on_twMain_itemSelectionChanged() {
    ui->twMain->blockSignals(true);
    ui->twCommon->blockSignals(true);

    ui->twCommon->verticalScrollBar()->setSliderPosition(ui->twMain->verticalScrollBar()->sliderPosition());

    int i, j;
    QList<QTableWidgetItem *> lSelected = ui->twCommon->selectedItems();

    for (i = 0; i < lSelected.length(); i++) {
        ui->twCommon->setItemSelected(lSelected.at(i), false);
    }

    lSelected = ui->twMain->selectedItems();

    for (i = 0; i < lSelected.length(); i++) {
        for (j = 0; j < ui->twCommon->model()->columnCount(); j++) {
            QTableWidgetItem *lItem = ui->twCommon->item(lSelected.at(i)->row(), j);
            ui->twCommon->setItemSelected(lItem, true);
        }
    }

    ui->tbProps->setEnabled(ui->twMain->currentItem());
    if (ui->tbRights->isVisible()) ui->tbRights->setEnabled(ui->twMain->currentItem());
    if (ui->tbMinus->isVisible()) ui->tbMinus->setEnabled(!lSelected.isEmpty());

    ui->twMain->blockSignals(false);
    ui->twCommon->blockSignals(false);
}

void UsersDlg::on_twCommon_itemSelectionChanged() {
    ui->twMain->blockSignals(true);
    ui->twCommon->blockSignals(true);

    ui->twMain->verticalScrollBar()->setSliderPosition(ui->twCommon->verticalScrollBar()->sliderPosition());

    int i;
    QList<QTableWidgetItem *> lSelected = ui->twMain->selectedItems();

    for (i = 0; i < lSelected.length(); i++) {
        ui->twMain->setItemSelected(lSelected.at(i), false);
    }

    lSelected = ui->twCommon->selectedItems();

    for (i = 0; i < lSelected.length(); i++) {
        QTableWidgetItem *lItem = ui->twMain->item(lSelected.at(i)->row(), 0);
        ui->twMain->setItemSelected(lItem, true);
    }

    ui->tbProps->setEnabled(ui->twCommon->currentItem());
    if (ui->tbRights->isVisible()) ui->tbRights->setEnabled(ui->twCommon->currentItem());
    if (ui->tbMinus->isVisible()) ui->tbMinus->setEnabled(!lSelected.isEmpty());

    ui->twMain->blockSignals(false);
    ui->twCommon->blockSignals(false);
}

void UsersDlg::on_twCommon_customContextMenuRequested(const QPoint &pos) {
    ContextMenuForUsers();
}

void UsersDlg::on_cbType_currentIndexChanged(int index) {
    ShowData();
}

void UsersDlg::on_leFilter_textEdited(const QString &arg1) {
    ShowData();
}

void UsersDlg::on_tbReload_clicked() {
    gUsers->InitUserList();
}

void UsersDlg::on_actionProperties_triggered() {
    QList<QTableWidgetItem *> lSelected;
    if (ui->twMain->hasFocus()) {
        lSelected = ui->twMain->selectedItems();
    } else {
        lSelected = ui->twCommon->selectedItems();
    }

    if (!lSelected.isEmpty()) {
        UserData *lUser = ui->twMain->item(lSelected.at(0)->row(), 0)->data(Qt::UserRole).value<UserData *>();

        int lHasLogin = lUser->HasLogin(), lDisabled = lUser->Disabled();
        UserPropDlg w(lUser, this);
        if (w.exec() == QDialog::Accepted) {
            if (lHasLogin != lUser->HasLogin()
                    || lDisabled != lUser->Disabled()) {
                ui->pbUpdateRights->setVisible(true);
            }
            //renew
            gUsers->InitUserList();
        }
    }
}

void UsersDlg::ContextMenuForUsers() {
    int i;

    QList<int> lSelectedRows;

    if (ui->twMain->selectionMode() == QAbstractItemView::SingleSelection) {
        if (ui->twMain->hasFocus()) {
            if (ui->twMain->currentItem()) lSelectedRows.append(ui->twMain->currentRow());
        } else {
            if (ui->twCommon->currentItem()) lSelectedRows.append(ui->twCommon->currentRow());
        }
    } else {
        QList<QTableWidgetItem *> lSelectedItems;

        if (ui->twMain->hasFocus()) {
            lSelectedItems = ui->twMain->selectedItems();
        } else {
            lSelectedItems = ui->twCommon->selectedItems();
        }

        for (i = 0; i < lSelectedItems.length(); i++)
            if (!lSelectedRows.contains(lSelectedItems.at(i)->row()))
                lSelectedRows.append(lSelectedItems.at(i)->row());
    }

    bool lHasExisting = false, lHasNoExisting = false;
    bool lHasWorking = false, lHasFired = false;
    for (i = 0; i < lSelectedRows.length(); i++) {
        UserData *lUser = ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>();
        if (gUserRight->CanCreateUser()
                || gUserRight->CanManageUser()
                || gUserRight->CanDropUser()) {
            if (lUser->HasLogin()) {
                lHasExisting = true;
            } else {
                lHasNoExisting = true;
            }
        }
        if (gUserRight->CanUpdate(gUsers->TableName(), "disabled")) {
            if (!lUser->Disabled()) {
                lHasWorking = true;
            } else {
                lHasFired = true;
            }
        }
    }

    QMenu lPopup;

    if (mDisplayType == DTSelectOne) {
        lPopup.addAction(ui->actionSelect);
        lPopup.setDefaultAction(ui->actionSelect);
        lPopup.addSeparator();
    } else if (mDisplayType == DTSelectMany) {
        lPopup.addAction(ui->actionSelect);
        lPopup.setDefaultAction(ui->actionSelect);
        lPopup.addSeparator();
    }

    lPopup.addAction(ui->actionProperties);
    if (mDisplayType != DTSelectOne && mDisplayType != DTSelectMany) lPopup.setDefaultAction(ui->actionProperties);

    if (ui->tbRights->isVisible() && lHasExisting) {
        lPopup.addAction(ui->actionRights);
        lPopup.addAction(ui->actionChange_password);
    }
    lPopup.addSeparator();


    if (ui->tbPlus->isVisible()) {
        lPopup.addAction(ui->actionAdd);
    }

    if (lHasWorking) {
        lPopup.addAction(ui->actionFire_block);
    }

    if (lHasFired) {
        lPopup.addAction(ui->actionRestore);
    }

    QAction *lARes = NULL, *lACreate = NULL, *lALock = NULL, *lAUnlock = NULL, *lADrop = NULL;

    if (!lSelectedRows.isEmpty()) {
        QMenu *lMenuUtils = NULL;

        if (lHasNoExisting
                && gUserRight->CanCreateUser()) {
            lPopup.addSeparator();
            lMenuUtils = lPopup.addMenu(tr("Utilities"));
            lACreate = lMenuUtils->addAction(tr("Create"));
        }

        if (lHasExisting) {
            if (gUserRight->CanManageUser()) {
                if (!lMenuUtils) {
                    lPopup.addSeparator();
                    lMenuUtils = lPopup.addMenu(tr("Utilities"));
                }
                lALock = lMenuUtils->addAction(tr("Lock"));
                lAUnlock = lMenuUtils->addAction(tr("Unlock"));
            }
            if (gUserRight->CanDropUser()) {
                if (!lMenuUtils) {
                    lPopup.addSeparator();
                    lMenuUtils = lPopup.addMenu(tr("Utilities"));
                }
                lADrop = lMenuUtils->addAction(tr("Drop"));
            }
        }

        QTableWidgetItem *lItem;
        if (ui->twMain->hasFocus()) {
            lItem = ui->twMain->currentItem();
        } else {
            lItem = ui->twCommon->currentItem();
        }

        if (lItem && !lItem->text().isEmpty()) {
            QTableWidgetItem *lItemHeader;
            if (ui->twMain->hasFocus()) {
                lItemHeader = ui->twMain->horizontalHeaderItem(ui->twMain->currentColumn());
            } else {
                lItemHeader = ui->twCommon->horizontalHeaderItem(ui->twCommon->currentColumn());
            }

            lPopup.addSeparator();
            ui->actionCopy->setText(tr("Copy") + " " + (lItemHeader->toolTip().isEmpty()?lItemHeader->text():lItemHeader->toolTip()).toLower());
            lPopup.addAction(ui->actionCopy);
        }
    }

    if (lARes = lPopup.exec(QCursor::pos())) {
        if (lARes == lACreate) {
            for (i = 0; i < lSelectedRows.length(); i++) {
                if (QMessageBox::question(this, tr("Create user"), tr("Create user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst() + "'?") == QMessageBox::Yes) {
                    if (gUsers->CreateUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst(), "a1")) {
                        QMessageBox::information(this, tr("Create user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst()
                                                 + "' " + tr("created with password") + " a1\n" + tr("You need to set permissions for this user"));
                        UserRightsDlg w(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>(), this);
                        w.exec();
                        gUsers->InitUserList();
                        ui->pbUpdateRights->setVisible(true);
                    }
                }
            }
        } else if (lARes == lALock) {
            for (i = 0; i < lSelectedRows.length(); i++) {
                if (QMessageBox::question(this, tr("Lock user"), tr("Lock user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst() + "'?") == QMessageBox::Yes) {
                    if (gUsers->LockUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst(), true)) {
                        QMessageBox::information(this, tr("Lock user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst()
                                                 + "' " + tr("locked successfully"));
                    }
                }
            }
        } else if (lARes == lAUnlock) {
            for (i = 0; i < lSelectedRows.length(); i++) {
                if (QMessageBox::question(this, tr("Unlock user"), tr("Unlock user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst() + "'?") == QMessageBox::Yes) {
                    if (gUsers->UnlockUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst())) {
                        QMessageBox::information(this, tr("Unlock user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst()
                                                 + "' " + tr("unlocked successfully"));
                    }
                }
            }
        } else if (lARes == lADrop) {
            for (i = 0; i < lSelectedRows.length(); i++) {
                if (QMessageBox::question(this, tr("Drop user"), tr("Drop user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst() + "'?") == QMessageBox::Yes) {
                    if (gUsers->DropUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst(), true)) {
                        QMessageBox::information(this, tr("Drop user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst()
                                                 + "' " + tr ("dropped successfully"));
                        gUsers->InitUserList();
                    }
                }
            }
        }
    }
//    QList<QTableWidgetItem *> lSelectedItems;
//    if (ui->twMain->hasFocus()) {
//        lSelectedItems = ui->twMain->selectedItems();
//    } else {
//        lSelectedItems = ui->twCommon->selectedItems();
//    }

//    QList<int> lSelectedRows;

//    bool lHasExisting = false, lHasNoExisting = false;
//    bool lHasWorking = false, lHasFired = false;
//    if (!lSelectedItems.isEmpty()) {
//        for (int i = 0; i < lSelectedItems.length(); i++) {
//            if (!lSelectedRows.contains(lSelectedItems.at(i)->row()))
//                lSelectedRows.append(lSelectedItems.at(i)->row());
//        }


//        for (int i = 0; i < lSelectedRows.length(); i++) {
//            UserData *lUser = ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>();
//            if (gUserRight->CanCreateUser()
//                    || gUserRight->CanManageUser()
//                    || gUserRight->CanDropUser()) {
//                if (lUser->HasLogin()) {
//                    lHasExisting = true;
//                } else {
//                    lHasNoExisting = true;
//                }
//            }
//            if (gUserRight->CanUpdate(gUsers->TableName(), "disabled")) {
//                if (!lUser->Disabled()) {
//                    lHasWorking = true;
//                } else {
//                    lHasFired = true;
//                }
//            }
//        }
//    }

//    QMenu lPopup;

//    if (ui->tbPlus->isVisible()) {
//        lPopup.addAction(ui->actionAdd);
//    }
//    lPopup.addAction(ui->actionProperties);
//    lPopup.setDefaultAction(ui->actionProperties);

//    if (ui->tbRights->isVisible() && lHasExisting) {
//        lPopup.addAction(ui->actionRights);
//        lPopup.addAction(ui->actionChange_password);
//    }

//    if (lHasWorking) {
//        lPopup.addAction(ui->actionFire_block);
//    }

//    if (lHasFired) {
//        lPopup.addAction(ui->actionRestore);
//    }

//    QAction *lARes = NULL, *lACreate = NULL, *lALock = NULL, *lAUnlock = NULL, *lADrop = NULL;

//    if (!lSelectedRows.isEmpty()) {
//        QMenu *lMenuUtils = NULL;

//        if (lHasNoExisting
//                && gUserRight->CanCreateUser()) {
//            lPopup.addSeparator();
//            lMenuUtils = lPopup.addMenu(tr("Utilities"));
//            lACreate = lMenuUtils->addAction(tr("Create"));
//        }

//        if (lHasExisting) {
//            if (gUserRight->CanManageUser()) {
//                if (!lMenuUtils) {
//                    lPopup.addSeparator();
//                    lMenuUtils = lPopup.addMenu(tr("Utilities"));
//                }
//                lALock = lMenuUtils->addAction(tr("Lock"));
//                lAUnlock = lMenuUtils->addAction(tr("Unlock"));
//            }
//            if (gUserRight->CanDropUser()) {
//                if (!lMenuUtils) {
//                    lPopup.addSeparator();
//                    lMenuUtils = lPopup.addMenu(tr("Utilities"));
//                }
//                lADrop = lMenuUtils->addAction(tr("Drop"));
//            }
//        }

//        QTableWidgetItem *lItem;
//        if (ui->twMain->hasFocus()) {
//            lItem = ui->twMain->item(lSelectedRows.at(0), ui->twMain->currentColumn());
//        } else {
//            lItem = ui->twCommon->item(lSelectedRows.at(0), ui->twCommon->currentColumn());
//        }

//        if (lItem && !lItem->text().isEmpty()) {
//            QTableWidgetItem *lItemHeader;
//            if (ui->twMain->hasFocus()) {
//                lItemHeader = ui->twMain->horizontalHeaderItem(ui->twMain->currentColumn());
//            } else {
//                lItemHeader = ui->twCommon->horizontalHeaderItem(ui->twCommon->currentColumn());
//            }

//            lPopup.addSeparator();
//            ui->actionCopy->setText(tr("Copy") + " " + (lItemHeader->toolTip().isEmpty()?lItemHeader->text():lItemHeader->toolTip()).toLower());
//            lPopup.addAction(ui->actionCopy);
//        }
//    }

//    if (lARes = lPopup.exec(QCursor::pos())) {
//        if (lARes == lACreate) {
//            for (int i = 0; i < lSelectedRows.length(); i++) {
//                if (QMessageBox::question(this, tr("Create user"), tr("Create user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst() + "'?") == QMessageBox::Yes) {
//                    if (gUsers->CreateUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst(), "a1")) {
//                        QMessageBox::information(this, tr("Create user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst()
//                                                 + "' " + tr("created with password") + " a1\n" + tr("You need to set permissions for this user"));
//                        UserRightsDlg w(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>(), this);
//                        w.exec();
//                        gUsers->InitUserList();
//                        ui->pbUpdateRights->setVisible(true);
//                    }
//                }
//            }
//        } else if (lARes == lALock) {
//            for (int i = 0; i < lSelectedRows.length(); i++) {
//                if (QMessageBox::question(this, tr("Lock user"), tr("Lock user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst() + "'?") == QMessageBox::Yes) {
//                    if (gUsers->LockUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst(), true)) {
//                        QMessageBox::information(this, tr("Lock user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst()
//                                                 + "' " + tr("locked successfully"));
//                    }
//                }
//            }
//        } else if (lARes == lAUnlock) {
//            for (int i = 0; i < lSelectedRows.length(); i++) {
//                if (QMessageBox::question(this, tr("Unlock user"), tr("Unlock user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst() + "'?") == QMessageBox::Yes) {
//                    if (gUsers->UnlockUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst())) {
//                        QMessageBox::information(this, tr("Unlock user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->NameConst()
//                                                 + "' " + tr("unlocked successfully"));
//                    }
//                }
//            }
//        } else if (lARes == lADrop) {
//            for (int i = 0; i < lSelectedRows.length(); i++) {
//                if (QMessageBox::question(this, tr("Drop user"), tr("Drop user") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst() + "'?") == QMessageBox::Yes) {
//                    if (gUsers->DropUser(ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst(), true)) {
//                        QMessageBox::information(this, tr("Drop user"), tr("User") + " '" + ui->twMain->item(lSelectedRows.at(i), 0)->data(Qt::UserRole).value<UserData *>()->LoginConst()
//                                                 + "' " + tr ("dropped successfully"));
//                        gUsers->InitUserList();
//                    }
//                }
//            }
//        }
//    }
}

void UsersDlg::on_twMain_customContextMenuRequested(const QPoint &pos) {
    ContextMenuForUsers();
}

void UsersDlg::on_twMain_cellDoubleClicked(int row, int column) {
    if (mDisplayType == DTEdit) {
        emit ui->actionProperties->trigger();
    } else if (mDisplayType == DTSelectOne) {
        emit Accept();
    }
}

void UsersDlg::on_twCommon_cellDoubleClicked(int row, int column) {
    emit ui->actionProperties->trigger();
}

void UsersDlg::on_actionFire_block_triggered() {
    QList<QTableWidgetItem *> lSelected;
    if (ui->twMain->hasFocus()) {
        lSelected = ui->twMain->selectedItems();
    } else {
        lSelected = ui->twCommon->selectedItems();
    }

    if (!lSelected.isEmpty()) {
        UserData *lUser = ui->twMain->item(lSelected.at(0)->row(), 0)->data(Qt::UserRole).value<UserData *>();

        if (QMessageBox::question(this, tr("User data"), tr("Fire user") + " '" + lUser->NameConst() + "'?") == QMessageBox::Yes) {
            lUser->setDisabled(1);
            bool lDummyNotChanged;
            if (lUser->SaveData(lDummyNotChanged)) {
                lUser->CommitEdit();
                // we apopogize that it is enough for user blocking;
                // although, we try to drop/lock account on server

                QSqlQuery qUserLock(db);
                if (gSettings->ServerSecure.DropOnFire == -1) {
                    gSettings->ServerSecure.DropOnFire = gHomeData->Get("DROP_ON_FIRE").toInt();
                }

                bool lTryDrop = gSettings->ServerSecure.DropOnFire;
retryForLock:
                if (lTryDrop) {
                    // NB: no cascade clause, no fucking manager can drop my or other significant account
                    qUserLock.prepare("drop user " + lUser->LoginConst());
                } else {
                    qUserLock.prepare("alter user " + lUser->LoginConst() + " account lock");
                }
                if (qUserLock.lastError().isValid()) {
                    gLogger->ShowSqlError(tr("User permissions"), qUserLock);
                } else {
                    if (!qUserLock.exec()) {
                        //gLogger->ShowSqlError(tr("User permissions"), qUserLock);
                        if (lTryDrop) {
                            gLogger->ShowError(this, tr("User permissions"), tr("Security warning: can't drop user") + " " + lUser->LoginConst() + "\n" + qUserLock.lastError().text());
                            lTryDrop = false;
                            goto retryForLock;
                        } else {
                            gLogger->ShowError(this, tr("User permissions"), tr("Security warning: can't lock user") + " " + lUser->LoginConst() + "\n" + qUserLock.lastError().text());
                            // ну и хуй с ним
                        }
                    }
                }

                //renew
                gUsers->InitUserList();
            } else {
                lUser->RollbackEdit();
            }
        }
    }
}

void UsersDlg::Accept() {
    if (mDisplayType == DTSelectOne) {
        if (ui->twMain->currentItem()
                && (mSelectedUser = ui->twMain->currentItem()->data(Qt::UserRole).value<UserData *>()))
            accept();
    } else {
        mSelectedUsers.clear(); //???
        QList<QTableWidgetItem *> lSelected = ui->twMain->selectedItems();
        for (int i = 0; i < lSelected.length(); i++) {
            UserData *lUser = ui->twMain->item(lSelected.at(i)->row(), 0)->data(Qt::UserRole).value<UserData *>();
            if (lUser && !mSelectedUsers.contains(lUser)) mSelectedUsers.append(lUser);
        }
        accept();
    }
}

void UsersDlg::on_actionCopy_triggered() {
    QList<QTableWidgetItem *> lSelected;
    QTableWidget *lTableWidget;

    if (ui->twMain->hasFocus()) {
        lTableWidget = ui->twMain;
    } else {
        lTableWidget = ui->twCommon;
    }
    lSelected = lTableWidget->selectedItems();

    if (!lSelected.isEmpty()) {
        QTableWidgetItem *lItem = lTableWidget->item(lSelected.at(0)->row(), lTableWidget->currentColumn());
        if (lItem && !lItem->text().isEmpty()) {
            QApplication::clipboard()->setText(lItem->text());
        }
    }
}

void UsersDlg::on_actionRights_triggered() {
    QList<QTableWidgetItem *> lSelected;
    if (ui->twMain->hasFocus()) {
        lSelected = ui->twMain->selectedItems();
    } else {
        lSelected = ui->twCommon->selectedItems();
    }

    if (!lSelected.isEmpty()) {
        UserData *lUser = ui->twMain->item(lSelected.at(0)->row(), 0)->data(Qt::UserRole).value<UserData *>();

        UserRightsDlg w(lUser, this);
        if (w.exec() == QDialog::Accepted) {
            ui->pbUpdateRights->setVisible(true);
        }
    }
}

void UsersDlg::on_actionChange_password_triggered() {
    if (mDisplayType == DTSelectOne
            || mDisplayType == DTSelectMany) {
        return;
    }

    QList<QTableWidgetItem *> lSelected;
    if (ui->twMain->hasFocus()) {
        lSelected = ui->twMain->selectedItems();
    } else {
        lSelected = ui->twCommon->selectedItems();
    }

    if (!lSelected.isEmpty()) {
        UserData *lUser = ui->twMain->item(lSelected.at(0)->row(), 0)->data(Qt::UserRole).value<UserData *>();

        ChangePassDlg w(lUser, this);
        w.exec();
        /*if (w.exec() == QDialog::Accepted) {
            //renew
            gUsers->InitUserList();
        }*/
    }

}

void UsersDlg::on_actionAdd_triggered() {
    if (mDisplayType == DTSelectOne
            || mDisplayType == DTSelectMany) {
        return;
    }

    NewUserDlg w(this);
    switch (w.exec()) {
    case QDialog::Accepted:
        gUsers->InitUserList();
        ui->pbUpdateRights->setVisible(true);
        break;
    case 200:
        int lHasLogin = w.User()->HasLogin(), lDisabled = w.User()->Disabled();
        UserPropDlg w2(w.User(), this);
        if (w2.exec() == QDialog::Accepted) {
            if (lHasLogin != w.User()->HasLogin()
                    || lDisabled != w.User()->Disabled()) {
                ui->pbUpdateRights->setVisible(true);
            }
            //renew
            gUsers->InitUserList();
        }
    }

/*    if (w.exec() == QDialog::Accepted) {
        //renew
        gUsers->InitUserList();
        ui->pbUpdateRights->setVisible(true);
    }*/
}

void UsersDlg::on_pbUpdateRights_clicked() {
    if (gHomeData->Get("NEED_UPDATE_RIGHTS").toInt() == 1) {
        WaitDlg w(this);
        w.show();
        w.SetMessage(tr("Updating users permissions..."));
        gUserRight->UpdateRightsOnServer();
        ui->pbUpdateRights->setVisible(gHomeData->Get("NEED_UPDATE_RIGHTS").toInt() == 1);
        setFocus();
    }
}

void UsersDlg::on_actionRestore_triggered() {
    QList<QTableWidgetItem *> lSelected;
    if (ui->twMain->hasFocus()) {
        lSelected = ui->twMain->selectedItems();
    } else {
        lSelected = ui->twCommon->selectedItems();
    }

    if (!lSelected.isEmpty()) {
        UserData *lUser = ui->twMain->item(lSelected.at(0)->row(), 0)->data(Qt::UserRole).value<UserData *>();
        if (lUser->Disabled() == 0) return;

        if (QMessageBox::question(this, tr("User data"), tr("Restore user") + " '" + lUser->NameConst() + "'?") == QMessageBox::Yes) {
            bool lCreated = false;
            if (!lUser->HasLogin()) {
                if (!gUsers->CreateUser(lUser->LoginConst(), "a1")) {
                    return;
                }
                lCreated = true;
            }

            if (gUsers->UnlockUser(lUser->LoginConst())) {
                lUser->setDisabled(0);
                bool lDummyNotChanged;
                if (lUser->SaveData(lDummyNotChanged)) {
                    lUser->CommitEdit();
                    QMessageBox::information(this, tr("Restore user"), tr("User") + " '" + lUser->NameConst()
                                             + "' " + tr("restored successfully") + (lCreated?(" " + tr("with password") + " a1"):""));

                    UserRightsDlg w1(lUser, this);
                    w1.exec();

                    UserPropDlg w2(lUser, this);
                    w2.exec();

                    ui->pbUpdateRights->setVisible(true);
                    //renew
                    gUsers->InitUserList();
                } else {
                    lUser->RollbackEdit();
                    gUsers->LockUser(lUser->LoginConst(), false);
                    if (lCreated) {
                        gUsers->DropUser(lUser->LoginConst(), false);
                    }
                }
            }
        }
    }
}
