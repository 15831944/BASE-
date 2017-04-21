#include <QMenu>
#include "OrganizationsListDlg.h"
#include "ui_OrganizationsListDlg.h"

#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>
#include <QSignalMapper>
#include <QResizeEvent>

#include "CustomerData.h"
#include "OrganizationPropDlg.h"
#include "OrgPersonDlg.h"
#include "UserRight.h"

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/HomeData.h"
#include "../VProject/PlotListItemDelegate.h"

OrganizationsListDlg::OrganizationsListDlg(DisplayTypeEnum aDisplayType, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::OrganizationsListDlg),
    mCheckForInclude(NULL), mpDataForCheckForInclude(NULL),
    mSignalMapper(new QSignalMapper(this)),
    mDisplayType(aDisplayType),
    mFilterWithPersons(false), mFilterWithData(false),
    mShowDeleted(false), mInResise(false), mInResizeTimer(false),
    mSelectedCustomerId(0), mCustomerScrollPos(-1),
    mSelectedPersonId(0), mPersonScrollPos(-1),
    mJustStarted(true)
{
    InitInConstructor();
}

OrganizationsListDlg::OrganizationsListDlg(QSettings &aSettings, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::OrganizationsListDlg),
    mCheckForInclude(NULL), mpDataForCheckForInclude(NULL),
    mSignalMapper(new QSignalMapper(this)),
    mFilterWithPersons(false), mFilterWithData(false),
    mShowDeleted(false), mInResise(false), mInResizeTimer(false),
    mSelectedCustomerId(0), mCustomerScrollPos(-1),
    mSelectedPersonId(0), mPersonScrollPos(-1),
    mJustStarted(true)
{
    mDisplayType = static_cast<DisplayTypeEnum>(aSettings.value("DisplayType", 0).toInt());
    InitInConstructor();
}

OrganizationsListDlg::~OrganizationsListDlg() {
    delete mSignalMapper;
    delete ui;
}

void OrganizationsListDlg::SetCheckForInclude(int (*aCheckForInclude)(const CustomerData *, void *), void *apData) {
    mCheckForInclude = aCheckForInclude;
    mpDataForCheckForInclude = apData;
}

void OrganizationsListDlg::SetSelectedCustomerId(int aIdCustomer) {
    mSelectedCustomerId = aIdCustomer;
}

void OrganizationsListDlg::SetSelectedCustomers(const QList<CustomerData *> &aSelectedCustomers) {
    mSelectedCustomers = aSelectedCustomers;
}

int OrganizationsListDlg::GetSelectedCustomerId() const {
    if (mSelectedCustomer)
        return mSelectedCustomer->Id();
    else
        return 0;
}

CustomerData *OrganizationsListDlg::GetSelectedCustomer() const {
    return mSelectedCustomer;
}

const QList<CustomerData *> &OrganizationsListDlg::GetSelectedCustomers() const {
    return mSelectedCustomers;
}

void OrganizationsListDlg::SetSelectedPersonId(int aIdPerson) {
    CustomerPerson *lPerson;
    if (lPerson = gCustomers->GetPersonById(aIdPerson)) {
        mSelectedCustomerId = lPerson->IdCustomer();
        mSelectedPersonId = aIdPerson;
    } else {
        mSelectedCustomerId = 0;
        mSelectedPersonId = 0;
    }
}

void OrganizationsListDlg::SetSelectedPersons(const QList<CustomerPerson *> &aSelectedPersons) {
    mSelectedPersons = aSelectedPersons;
}

int OrganizationsListDlg::GetSelectedPersonId() const {
    if (mSelectedPerson)
        return mSelectedPerson->Id();
    else
        return 0;
}

CustomerPerson *OrganizationsListDlg::GetSelectedPerson() const {
    return mSelectedPerson;
}

const QList<CustomerPerson *> &OrganizationsListDlg::GetSelectedPersons() const {
    return mSelectedPersons;
}

void OrganizationsListDlg::InitInConstructor() {
    ui->setupUi(this);

    ui->wdSelected->setVisible(false);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->pbSelect->setVisible(mDisplayType == DTSelectOneFirm
                             || mDisplayType == DTSelectManyFirm
                             || mDisplayType == DTSelectOnePerson
                             || mDisplayType == DTSelectManyPerson
                             || mDisplayType == DTSelectMany);
    ui->pbCancel->setVisible(mDisplayType == DTSelectOneFirm
                             || mDisplayType == DTSelectManyFirm
                             || mDisplayType == DTSelectOnePerson
                             || mDisplayType == DTSelectManyPerson
                             || mDisplayType == DTSelectMany);

    ui->cbClient->setVisible(gCustomers->HasIsClient());
    ui->cbProvider->setVisible(gCustomers->HasIsSupplier());

    // always selected
    mPaletteAlwaysSelected = ui->listCustomers->palette();
    mPaletteAlwaysSelected.setColor(QPalette::Inactive, QPalette::Highlight, mPaletteAlwaysSelected.color(QPalette::Active, QPalette::Highlight));
    mPaletteAlwaysSelected.setColor(QPalette::Inactive, QPalette::HighlightedText, mPaletteAlwaysSelected.color(QPalette::Active, QPalette::HighlightedText));
    ui->listCustomers->setPalette(mPaletteAlwaysSelected);

    mPaletteAlwaysSelectedDis = mPaletteAlwaysSelected;
    mPaletteAlwaysSelectedDis.setColor(QPalette::Base, palette().color(QPalette::Window));

    mPaletteNorm = ui->leShortName->palette();
    mPaletteDis = mPaletteNorm;
    mPaletteDis.setColor(QPalette::Base, palette().color(QPalette::Window));

    // it is line for splitter
    // ----------------------------------------------
    QSplitterHandle *handle = ui->splitter->handle(1);
    QHBoxLayout *layout = new QHBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    QFrame *line = new QFrame(handle);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    QList<int> sizes;
    sizes << 200 << 200;
    ui->splitter->setSizes(sizes);
    ui->splitter->setStretchFactor(1, 100);
    // ----------------------------------------------

    // it is line for splitter
    // ----------------------------------------------
    handle = ui->splitter_2->handle(1);
    layout = new QHBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    line = new QFrame(handle);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    ui->splitter->setSizes(sizes);
    ui->splitter->setStretchFactor(1, 100);
    ui->splitter->setCollapsible(1, false);
    // ----------------------------------------------

    connect(ui->twMain->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(MainScrolled(int)));
    connect(ui->twCommon->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(CommonScrolled(int)));

    connect(gCustomers, SIGNAL(CustListBeforeUpdate(int)), this, SLOT(OnCustListBeforeUpdate(int)));
    connect(gCustomers, SIGNAL(CustListNeedUpdate(int)), this, SLOT(OnCustListNeedUpdate(int)));

    mCanAddCustomer = mDisplayType != DTEdit && gUserRight->CanInsert("v_customer") && gUserRight->CanInsert("v_custdata");
    mCanDelCustomer = mDisplayType != DTEdit && (gUserRight->CanDelete("v_customer") || gUserRight->CanUpdate("v_customer", "deluser") && gUserRight->CanUpdate("v_customer", "deldate"));
    mCanSeeCustProps = gUserRight->CanSelect("v_custdata") && gUserRight->CanSelect("v_custdata_t");
    mCanAddPerson = gUserRight->CanInsert("v_custperson");
    mCanDelPerson = gUserRight->CanDelete("v_custperson") || gUserRight->CanUpdate("v_custperson", "deluser") && gUserRight->CanUpdate("v_custperson", "deldate");

    ui->tbPlus->setVisible(mCanAddCustomer);
    ui->tbMinus->setVisible(mCanDelCustomer);
    ui->twCustProps->setVisible(mCanSeeCustProps);
    ui->tbAddPerson->setVisible(mCanAddPerson);
    ui->tbDelPerson->setVisible(mCanDelPerson);

    ui->twCustProps->setItemDelegateForColumn(1, new ROListItemDelegate(this));
    ui->twMain->setItemDelegate(new ROListItemDelegate(this));
    ui->twCommon->setItemDelegate(new ROListItemDelegate(this));

    connect(mSignalMapper, SIGNAL(mapped(QWidget *)), this, SLOT(OnPressed(QWidget *)));
}

void OrganizationsListDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;
        if (gHomeData->Get("CUST_RLO").toInt() == 1) {
            setLayoutDirection(Qt::RightToLeft);
        }
        QTimer::singleShot(0, this, SLOT(ShowData()));
    }
}

void OrganizationsListDlg::StyleSheetChangedInSescendant() {
    int i;
    if (ui->twCustProps->isVisible()) {
        ui->twCustProps->resizeColumnsToContents();
        ui->twCustProps->resizeRowsToContents();
        int lHeight = 0;
        for (i = 0; i < ui->twCustProps->rowCount(); i++) {
            ui->twCustProps->setRowHeight(i, ui->twCustProps->rowHeight(i) - gSettings->Common.SubRowHeight);
            lHeight += ui->twCustProps->rowHeight(i);
        }
        ui->twCustProps->setFixedHeight(lHeight);
        ui->twCustProps->setColumnWidth(0, ui->twCustProps->columnWidth(0) + gSettings->Common.AddColWidth);
    }

    ui->twCommon->resizeColumnsToContents();
    ui->twCommon->resizeRowsToContents();
    for (i = 0; i < ui->twCommon->rowCount(); i++) {
        ui->twCommon->setRowHeight(i, ui->twCommon->rowHeight(i) - gSettings->Common.SubRowHeight);
        ui->twMain->setRowHeight(i, ui->twCommon->rowHeight(i));
    }
    for (i = 0; i < ui->twCommon->columnCount(); i++) {
        ui->twCommon->setColumnWidth(i, ui->twCommon->columnWidth(i) + gSettings->Common.AddColWidth);
    }
}

void OrganizationsListDlg::resizeEvent(QResizeEvent * event) {
    QFCDialog::resizeEvent(event);

    if (event->size().width() != event->oldSize().width()) {
        if (!mInResizeTimer) {
            mInResizeTimer = true;
            QTimer::singleShot(100, this, SLOT(wdSelectedRebuild()));
        }
    }
}

void OrganizationsListDlg::ShowData() {
    ui->listCustomers->clear();
    int i, j;
    QString lFilter = ui->leFilter->text().toLower();
    QListWidgetItem *lItem, *lScrollToItem = NULL;

    for(j = 0; j < gCustomers->CustomerListConst().length(); j++) {
        CustomerData * lCustomer = gCustomers->CustomerListConst().at(j);
        int lIncRes = 1;
        if (mCheckForInclude) lIncRes = (*mCheckForInclude) (lCustomer, mpDataForCheckForInclude);
        if (!lIncRes) continue;
        if (lIncRes == 1) {
            bool lAdd = false;
            if (!lCustomer->DelUserConst().isEmpty() && !mShowDeleted) continue;

            if (lFilter.isEmpty()) {
                lAdd = true;
            } else {
                QString lSN = lCustomer->ShortNameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ "));
                lAdd = lSN.contains(lFilter);
                if (!lAdd) {
                    lSN = lCustomer->NameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ "));
                    lAdd = lSN.contains(lFilter);
                }
                if (!lAdd
                        && mFilterWithData
                        && mCanSeeCustProps) {
                    for (i = 0; i < lCustomer->PropsConst().length(); i++) {
                        if (lCustomer->PropsConst().at(i)->ValueConst().toLower().contains(lFilter)) {
                            lAdd = true;
                            break;
                        }
                    }
                }
                if (!lAdd
                        && mFilterWithPersons) {
                    for (i = 0; i < lCustomer->PersonsConst().length(); i++) {
                        if (lCustomer->PersonsConst().at(i)->LastNameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
                                || lCustomer->PersonsConst().at(i)->FirstNameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
                                /*|| lCustomer->PersonsConst().at(i)->MiddleNameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)*/
                                || lCustomer->PersonsConst().at(i)->Tel1Const().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
                                || lCustomer->PersonsConst().at(i)->Tel2Const().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
                                || lCustomer->PersonsConst().at(i)->TelMobConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
                                || lCustomer->PersonsConst().at(i)->EmailConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
                                ) {
                            lAdd = true;
                            break;
                        }
                    }
                }
            }

            if (!lAdd) continue;
        }

        lItem = new QListWidgetItem(lCustomer->ShortNameConst());
        lItem->setData(Qt::UserRole, QVariant::fromValue(lCustomer));

        if (!lCustomer->DelUserConst().isEmpty()) {
            lItem->setBackgroundColor(palette().color(QPalette::Window));
        }

        if (lCustomer->ShortNameConst().contains(QRegExp("[א-ת]"))
                && layoutDirection() == Qt::LeftToRight)
            lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        else
            lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        ui->listCustomers->addItem(lItem);

        if (lCustomer->Id() == mSelectedCustomerId) {
            lScrollToItem = lItem;
        }
    }

    if (mCustomerScrollPos == -1 && lScrollToItem) {
        ui->listCustomers->scrollToItem(lScrollToItem, QAbstractItemView::PositionAtCenter);
    } else {
        ui->listCustomers->verticalScrollBar()->setSliderPosition(mCustomerScrollPos);
    }
    mSelectedCustomerId = 0;
    mCustomerScrollPos = -1;

    ui->listCustomers->setCurrentItem(lScrollToItem);

    if (mDisplayType == DTSelectManyFirm
            || mDisplayType == DTSelectManyPerson
            || mDisplayType == DTSelectMany) {
        wdSelectedRebuild();
    }

//    ui->tbProps->setEnabled(false);
//    ui->tbMinus->setEnabled(false);

//    ui->tbAddPerson->setEnabled(false);
//    ui->tbPropPerson->setEnabled(false);
//    ui->tbDelPerson->setEnabled(false);
}

void OrganizationsListDlg::on_actionAdd_triggered() {
    OrganizationPropDlg dlg(NULL, this);

    if (dlg.exec() == QDialog::Accepted) {
        gCustomers->EmitCustListBeforeUpdate(dlg.IdCustomer());
        gCustomers->EmitCustListNeedUpdate(0);
    }
}

void OrganizationsListDlg::on_listCustomers_customContextMenuRequested(const QPoint &pos) {
    QMenu lPopup;
    QAction *lAShowDeleted = NULL, *lARes = NULL;

    if(ui->listCustomers->currentItem()) {
        if (mDisplayType == DTSelectOneFirm) {
            lPopup.addAction(ui->actionSelect);
            lPopup.setDefaultAction(ui->actionSelect);
        } else if (mDisplayType == DTSelectManyFirm
                   || mDisplayType == DTSelectMany) {
            lPopup.addAction(ui->actionAdd_to_selection);
            lPopup.setDefaultAction(ui->actionAdd_to_selection);
        }
        lPopup.addAction(ui->actionProperties);
        if (mDisplayType != DTSelectOneFirm
                && mDisplayType != DTSelectManyFirm
                && mDisplayType != DTSelectMany) lPopup.setDefaultAction(ui->actionProperties);
        lPopup.addSeparator();
    }

    if (mCanAddCustomer) lPopup.addAction(ui->actionAdd);
    if(ui->listCustomers->currentItem()) {
        if (ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>()->DelUserConst().isEmpty()) {
            if (mCanDelCustomer) {
                lPopup.addAction(ui->actionDelete);
            }
        }
    }
    lPopup.addSeparator();
    lAShowDeleted = lPopup.addAction(tr("Show deleted"));
    lAShowDeleted->setCheckable(true);
    lAShowDeleted->setChecked(mShowDeleted);

    lARes = lPopup.exec(QCursor::pos());
    if (lARes == lAShowDeleted) {
        mShowDeleted = !mShowDeleted;
        if (ui->listCustomers->currentItem()) {
            mSelectedCustomerId = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>()->Id();
            if (ui->twMain->currentItem()) {
                mSelectedPersonId = ui->twMain->currentItem()->data(Qt::UserRole).value<CustomerPerson *>()->Id();
            } else {
                mSelectedPersonId = 0;
            }
        } else {
            mSelectedCustomerId = 0;
        }
        ShowData();
    }
}

void OrganizationsListDlg::on_listCustomers_doubleClicked(const QModelIndex &index) {
    if (mDisplayType == DTSelectOneFirm) {
        emit ui->actionSelect->trigger();
    } else if (mDisplayType == DTSelectManyFirm
               || mDisplayType == DTSelectMany) {
        emit ui->actionAdd_to_selection->trigger();
    } else {
        emit ui->actionProperties->triggered();
    }
}

void OrganizationsListDlg::on_actionDelete_triggered() {
    if (!ui->listCustomers->currentItem()) return;
    CustomerData * lCustomer = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>();

    if (QMessageBox::question(this, tr("Company"), tr("Delete company") + " '" + lCustomer->ShortNameConst() + "'?") == QMessageBox::Yes
            && lCustomer->RemoveFromDB()) {
        gCustomers->InitCustomerDataList();
    }
}

void OrganizationsListDlg::on_actionProperties_triggered() {
    if (!ui->listCustomers->currentItem()) return;
    OrganizationPropDlg dlg(ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>(), this);

    if (dlg.exec() == QDialog::Accepted) {
        gCustomers->InitCustomerDataList();
    }
}

void OrganizationsListDlg::MainScrolled(int aValue) {
    ui->twCommon->verticalScrollBar()->setSliderPosition(aValue);
}

void OrganizationsListDlg::CommonScrolled(int aValue) {
    ui->twMain->verticalScrollBar()->setSliderPosition(aValue);
}

void TempOutLayout(int aLevel, const QLayout * const aLayout, bool aNoNested = false) {
    QString lPrefix(aLevel * 4, ' ');
    gLogger->ShowErrorInList(NULL, lPrefix + "count", QString::number(aLayout->count()), false);
    for (int i = 0; i < aLayout->count(); i++) {
        if (aLayout->itemAt(i)->spacerItem()) {
            gLogger->ShowErrorInList(NULL, lPrefix + QString::number(i), "spacer " + QString::number(aLayout->itemAt(i)->spacerItem()->geometry().width()), false);
        } else if (aLayout->itemAt(i)->widget()) {
            gLogger->ShowErrorInList(NULL, lPrefix + QString::number(i), aLayout->itemAt(i)->widget()->metaObject()->className(), false);
        } else if (aLayout->itemAt(i)->layout()) {
            if (!aNoNested) {
                gLogger->ShowErrorInList(NULL, lPrefix + QString::number(i), "layout", false);
                TempOutLayout(aLevel + 1, aLayout->itemAt(i)->layout());
            }
        } else if (aLayout->itemAt(i)->isEmpty()) {
            gLogger->ShowErrorInList(NULL, lPrefix + QString::number(i), "isEmpty", false);
        } else {
            gLogger->ShowErrorInList(NULL, lPrefix + QString::number(i), "UNKNOWN", false);
        }
    }
}

//void OrganizationsListDlg::wdSelectedCleanupInternal(QLayout *aLayout) {
//    if (aLayout->count() == 1
//            && aLayout->itemAt(0)->spacerItem()) {
//        QLayoutItem *lChild = aLayout->takeAt(0);
//        delete lChild->spacerItem();
//        delete lChild;
//        delete aLayout;
//    } else {
//        for (int i = aLayout->count() - 1; i >= 0; i--) {
//            if (aLayout->itemAt(i)->layout()) {
//                if (!aLayout->itemAt(i)->layout()->count()) {
//                    // delete empty layout
//                    QLayoutItem *lChild = aLayout->takeAt(i);
//                    delete lChild;
//                } else {
//                    // process nested layuot
//                    wdSelectedCleanupInternal(aLayout->itemAt(i)->layout());
//                }
//            }
//        }
//    }
//}

//void OrganizationsListDlg::wdSelectedCleanup() {
//    wdSelectedCleanupInternal(ui->wdSelected->layout());
//    if (ui->wdSelected->layout()->count()) {
//        lLastHBoxLayout = static_cast<QHBoxLayout *>(ui->wdSelected->layout()->itemAt(ui->wdSelected->layout()->count() - 1)->layout());
//    } else {
//        lLastHBoxLayout = NULL;
//        ui->wdSelected->setVisible(false);
//    }
//}

void OrganizationsListDlg::wdSelectedRebuildCleanLayout(QLayout *aLayout) {
    QLayoutItem *lChild;
    while (lChild = aLayout->takeAt(0)) {
        if (lChild->widget()) {
            delete lChild->widget();
            delete lChild;
        } else if (lChild->spacerItem()) {
            delete lChild->spacerItem();
            delete lChild;
        } else if (lChild->layout()) {
            wdSelectedRebuildCleanLayout(lChild->layout());
            delete lChild->layout();
            // need not delete this child???
        }
    }
}

void OrganizationsListDlg::wdSelectedAdd(bool aIsFirst, QHBoxLayout *&aLastHBoxLayout, int &aCurWidth,
                                         CustomerData *aCustomer, CustomerPerson *aPerson) {
    const QString &lCustomerName = aCustomer->ShortNameConst();
    const QString &lPersonName = aPerson?aPerson->FullName():"";

    if (aIsFirst) {
        aLastHBoxLayout = new QHBoxLayout();
        static_cast<QVBoxLayout *>(ui->wdSelected->layout())->addLayout(aLastHBoxLayout);
        aLastHBoxLayout->addStretch(); // last stretch
        aLastHBoxLayout->setSpacing(8);
    }

    QLineEdit *lCustomerLineEdit = new QLineEdit();
    QFontMetrics lCustomerFontMetrics(lCustomerLineEdit->font());
    int lCustomerLEWidth = lCustomerFontMetrics.width(lCustomerName + "XX");
    lCustomerLineEdit->setReadOnly(true);
    lCustomerLineEdit->setText(lCustomerName);
    lCustomerLineEdit->setFixedWidth(lCustomerLEWidth);

    QLineEdit *lPersonLineEdit = NULL;
    int lPersonLEWidth = 0;
    if (aPerson) {
        lPersonLineEdit = new QLineEdit();
        QFontMetrics lPersonFontMetrics(lPersonLineEdit->font());
        lPersonLEWidth = lPersonFontMetrics.width(lPersonName + "XX");
        lPersonLineEdit->setReadOnly(true);
        lPersonLineEdit->setText(lPersonName);
        lPersonLineEdit->setFixedWidth(lPersonLEWidth);
    }

    //if (i && lSpacerWidth < lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 8) {
    if (ui->wdSelected->width() < aCurWidth + lCustomerLEWidth + lPersonLEWidth + ui->leFilter->height() + 16) {
        //QMessageBox::critical(this, QString::number(lSpacerWidth), QString::number(lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 8));
        aLastHBoxLayout = new QHBoxLayout();
        static_cast<QVBoxLayout *>(ui->wdSelected->layout())->addLayout(aLastHBoxLayout);
        aLastHBoxLayout->addStretch(); // last stretch
        aLastHBoxLayout->setSpacing(8);
        aCurWidth = lCustomerLEWidth + lPersonLEWidth + ui->leFilter->height() + 16;
    } else {
        aCurWidth += lCustomerLEWidth + lPersonLEWidth + ui->leFilter->height() + 16;
    }

    QHBoxLayout *lLayout = new QHBoxLayout();
    lLayout->setSpacing(0);

    lLayout->addWidget(lCustomerLineEdit);
    if (lPersonLineEdit) {
        lLayout->addWidget(lPersonLineEdit);
    }
    QToolButton *lTB = new QToolButton();
    lTB->setText("-");
    lTB->setFixedSize(ui->leFilter->height() + 2, ui->leFilter->height() + 2);
    lTB->setIcon(QIcon(":/some/ico/ico/exit.png"));
    lLayout->addWidget(lTB);

    connect(lTB, SIGNAL(pressed()), mSignalMapper, SLOT(map()));
    if (!aPerson) {
        lTB->setProperty("customer", QVariant::fromValue(aCustomer));
    } else {
        lTB->setProperty("person", QVariant::fromValue(aPerson));
    }
    lTB->setProperty("layout", QVariant::fromValue(lLayout));
    mSignalMapper->setMapping(lTB, lTB);

    aLastHBoxLayout->insertLayout(aLastHBoxLayout->count() - 1, lLayout);
}

void OrganizationsListDlg::wdSelectedRebuild() {
    if (mInResise) return;
    mInResise = true;
    if (!mSelectedCustomers.isEmpty()
            || !mSelectedPersons.isEmpty()) {
        wdSelectedRebuildCleanLayout(ui->wdSelected->layout());
        if (!ui->wdSelected->isVisible())
            ui->wdSelected->setVisible(true);
    } else {
        if (ui->wdSelected->isVisible())
            ui->wdSelected->setVisible(false);
    }
    //TempOutLayout(0, ui->wdSelected->layout());
    int i;
    QHBoxLayout *lLastHBoxLayout;
    int lCurWidth = 0;
    bool lIsFirst = true;
    for (i = 0; i < mSelectedCustomers.length(); i++) {
        wdSelectedAdd(lIsFirst, lLastHBoxLayout, lCurWidth, mSelectedCustomers.at(i), NULL);
        lIsFirst = false;
//        QString lName = mSelectedCustomers.at(i)->ShortNameConst();
//        if (!i) {
//            lLastHBoxLayout = new QHBoxLayout();
//            static_cast<QVBoxLayout *>(ui->wdSelected->layout())->addLayout(lLastHBoxLayout);
//            lLastHBoxLayout->addStretch(); // last stretch
//            lLastHBoxLayout->setSpacing(8);
//        }

//        QLineEdit *lLineEdit = new QLineEdit();
//        QFontMetrics lFontMetrics(lLineEdit->font());
//        lLineEdit->setReadOnly(true);
//        lLineEdit->setText(lName);
//        lLineEdit->setFixedWidth(lFontMetrics.width(lName + "XX"));
//        //int lSpacerWidth = lLastHBoxLayout->itemAt(lLastHBoxLayout->count() - 1)->spacerItem()->geometry().width();

//        //if (i && lSpacerWidth < lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 8) {
//        if (ui->wdSelected->width() < lCurWidth + lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 16) {
//            //QMessageBox::critical(this, QString::number(lSpacerWidth), QString::number(lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 8));
//            lLastHBoxLayout = new QHBoxLayout();
//            static_cast<QVBoxLayout *>(ui->wdSelected->layout())->addLayout(lLastHBoxLayout);
//            lLastHBoxLayout->addStretch(); // last stretch
//            lLastHBoxLayout->setSpacing(8);
//            lCurWidth = lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 16;
//        } else {
//            lCurWidth += lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 16;
//        }

//        QHBoxLayout *lLayout = new QHBoxLayout();
//        lLayout->setSpacing(0);

//        lLayout->addWidget(lLineEdit);
//        QToolButton *lTB = new QToolButton();
//        lTB->setText("-");
//        lTB->setFixedSize(ui->leFilter->height() + 2, ui->leFilter->height() + 2);
//        lTB->setIcon(QIcon(":/some/ico/ico/exit.png"));
//        lLayout->addWidget(lTB);

//        connect(lTB, SIGNAL(pressed()), mSignalMapper, SLOT(map()));
//        lTB->setProperty("customer", QVariant::fromValue(mSelectedCustomers.at(i)));
//        lTB->setProperty("layout", QVariant::fromValue(lLayout));
//        mSignalMapper->setMapping(lTB, lTB);

//        lLastHBoxLayout->insertLayout(lLastHBoxLayout->count() - 1, lLayout);
    }
    for (i = 0; i < mSelectedPersons.length(); i++) {
        wdSelectedAdd(lIsFirst, lLastHBoxLayout, lCurWidth,
                      gCustomers->GetCustomerById(mSelectedPersons.at(i)->IdCustomer()), mSelectedPersons.at(i));
        lIsFirst = false;
    }
    mInResise = false;
    mInResizeTimer = false;
}

void OrganizationsListDlg::OnPressed(QWidget *aWidget) {
    if (qobject_cast<QToolButton *>(aWidget)) {
        QToolButton *lTB = qobject_cast<QToolButton *>(aWidget);
        CustomerData *lCustomerData;
        if (lCustomerData = lTB->property("customer").value<CustomerData *>()) {
            mSelectedCustomers.removeAll(lCustomerData);
        } else {
            CustomerPerson *lCustomerPerson;
            if (lCustomerPerson = lTB->property("person").value<CustomerPerson *>())
                mSelectedPersons.removeAll(lCustomerPerson);
        }
        wdSelectedRebuild();
//        QHBoxLayout *lLayout = lTB->property("layout").value<QHBoxLayout *>();

//        QLayoutItem *lChild;
//        while (lChild = lLayout->takeAt(0)) {
//            if (lChild->widget()) {
//                delete lChild->widget();
//            }
//            delete lChild;
//        }
//        delete lLayout;

//        wdSelectedCleanup();
//        //TempOutLayout(0, ui->wdSelected->layout());
    }
}

void OrganizationsListDlg::OnCustListBeforeUpdate(int aIdCustomer) {
    if (aIdCustomer) {
        mSelectedCustomerId = aIdCustomer;
        mCustomerScrollPos = -1;
    } else if (ui->listCustomers->currentItem()) {
        mSelectedCustomerId = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>()->Id();
        mCustomerScrollPos = ui->listCustomers->verticalScrollBar()->sliderPosition();
        if (ui->twMain->currentItem()) {
            mSelectedPersonId = ui->twMain->currentItem()->data(Qt::UserRole).value<CustomerPerson *>()->Id();
            mPersonScrollPos = ui->twMain->verticalScrollBar()->sliderPosition();
        }
    } else {
        mSelectedCustomerId = 0;
        mCustomerScrollPos = ui->listCustomers->verticalScrollBar()->sliderPosition();
        mPersonScrollPos = ui->twMain->verticalScrollBar()->sliderPosition();
    }
}

void OrganizationsListDlg::OnCustListNeedUpdate(int aIdCustomer) {
    ShowData();
}

void OrganizationsListDlg::on_twMain_itemSelectionChanged() {
    ui->twMain->blockSignals(true);
    ui->twCommon->blockSignals(true);

    ui->twCommon->setCurrentIndex(ui->twMain->currentIndex());

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

    CustomerData *lCustomer = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>();
    ui->tbPropPerson->setEnabled(!lSelected.isEmpty() && lCustomer->DelUserConst().isEmpty());
    ui->tbDelPerson->setEnabled(!lSelected.isEmpty() && lCustomer->DelUserConst().isEmpty());

    ui->twMain->blockSignals(false);
    ui->twCommon->blockSignals(false);

}

void OrganizationsListDlg::on_twCommon_itemSelectionChanged() {
    ui->twMain->blockSignals(true);
    ui->twCommon->blockSignals(true);

    ui->twMain->setCurrentCell(ui->twCommon->currentIndex().row(), 0);

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

    CustomerData *lCustomer = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>();
    ui->tbPropPerson->setEnabled(!lSelected.isEmpty() && lCustomer->DelUserConst().isEmpty());
    ui->tbDelPerson->setEnabled(!lSelected.isEmpty() && lCustomer->DelUserConst().isEmpty());

    if (ui->pbSelect->isVisible()) ui->pbSelect->setEnabled(!lSelected.isEmpty());

    ui->twMain->blockSignals(false);
    ui->twCommon->blockSignals(false);

}

void OrganizationsListDlg::on_leFilter_textEdited(const QString &arg1) {
    if (ui->listCustomers->currentItem())
        mSelectedCustomerId = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>()->Id();
    ShowData();
}

void OrganizationsListDlg::on_twMain_customContextMenuRequested(const QPoint &pos) {
    bool lAny = false;
    QMenu lPopup;
    if(ui->twMain->currentItem()) {
        if (mDisplayType == DTSelectOnePerson) {
            lPopup.addAction(ui->actionSelectPerson);
            lPopup.setDefaultAction(ui->actionSelectPerson);
        } else if (mDisplayType == DTSelectManyPerson
                   || mDisplayType == DTSelectMany) {
            lPopup.addAction(ui->actionAddPerson_to_selection);
            lPopup.setDefaultAction(ui->actionAddPerson_to_selection);
        }
        lPopup.addAction(ui->actionProperties_of_person);
        if (mDisplayType != DTSelectOnePerson
                && mDisplayType != DTSelectManyPerson
                && mDisplayType != DTSelectMany) lPopup.setDefaultAction(ui->actionProperties_of_person);
        lPopup.addSeparator();
        lAny = true;
    }
    if (mCanAddPerson) {
        lPopup.addAction(ui->actionAdd_person);
        lAny = true;
    }
    if(ui->twMain->currentItem()
            && mCanDelPerson) {
        lPopup.addAction(ui->actionDelete_person);
        lAny = true;
    }
    if (lAny) lPopup.exec(QCursor::pos());
}

void OrganizationsListDlg::on_twCommon_customContextMenuRequested(const QPoint &pos) {
    emit ui->twMain->customContextMenuRequested(pos);
}

void OrganizationsListDlg::on_twMain_doubleClicked(const QModelIndex &index) {
    if (mDisplayType == DTSelectOnePerson) {
        emit ui->actionSelectPerson->trigger();
    } else if (mDisplayType == DTSelectManyPerson
               || mDisplayType == DTSelectMany) {
        emit ui->actionAddPerson_to_selection->trigger();
    } else {
        emit ui->actionProperties_of_person->triggered();
    }
}

void OrganizationsListDlg::on_twCommon_doubleClicked(const QModelIndex &index) {
    if (mDisplayType == DTSelectOnePerson) {
        emit ui->actionSelectPerson->trigger();
    } else if (mDisplayType == DTSelectManyPerson
               || mDisplayType == DTSelectMany) {
        emit ui->actionAddPerson_to_selection->trigger();
    } else {
        emit ui->actionProperties_of_person->triggered();
    }
}

void OrganizationsListDlg::on_actionRefresh_triggered() {
    gCustomers->InitCustomerDataList();
}

void OrganizationsListDlg::on_leFilter_customContextMenuRequested(const QPoint &pos) {
    QMenu *lMenu = ui->leFilter->createStandardContextMenu();
    QAction *lWithPersons = NULL, *lWithData = NULL, *lARes = NULL;
    lMenu->addSeparator();

    lWithPersons = lMenu->addAction(tr("Include employees"));
    lWithPersons->setCheckable(true);
    lWithPersons->setChecked(mFilterWithPersons);

    if (mCanSeeCustProps) {
        lWithData = lMenu->addAction(tr("Include data"));
        lWithData->setCheckable(true);
        lWithData->setChecked(mFilterWithData);
    }

    lARes = lMenu->exec(QCursor::pos());
    if (lARes == lWithPersons) {
        mFilterWithPersons = !mFilterWithPersons;
    } else if (lARes == lWithData) {
        mFilterWithData = !mFilterWithData;
    }
}

void OrganizationsListDlg::on_actionAdd_person_triggered() {
    if (!ui->listCustomers->currentItem()) return;
    CustomerData *lCustomer = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>();
    OrgPersonDlg dlg(lCustomer, NULL, this);
    if (dlg.exec() == QDialog::Accepted) {
        gCustomers->EmitCustListBeforeUpdate(lCustomer->Id());
        gCustomers->EmitCustListNeedUpdate(lCustomer->Id());
    }
}

void OrganizationsListDlg::on_actionProperties_of_person_triggered() {
    if (!ui->listCustomers->currentItem()) return;
    if (!ui->twMain->currentItem()) return;
    CustomerPerson *lPerson = ui->twMain->currentItem()->data(Qt::UserRole).value<CustomerPerson *>();
    OrgPersonDlg dlg(ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>(), lPerson, this);
    if (dlg.exec() == QDialog::Accepted) {
        gCustomers->EmitCustListBeforeUpdate(lPerson->IdCustomer());
        gCustomers->EmitCustListNeedUpdate(lPerson->IdCustomer());
    }
}

void OrganizationsListDlg::on_listCustomers_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    if (!ui->listCustomers->currentItem()) {
        ui->swData->setCurrentIndex(1);
        return;
    }

    int i, j;
    CustomerData *lCustomer = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>();

    ui->swData->setCurrentIndex(0);

    if (!lCustomer->DelUserConst().isEmpty()) {
        ui->leShortName->setPalette(mPaletteDis);
        ui->leFullName->setPalette(mPaletteDis);

        ui->twMain->setPalette(mPaletteAlwaysSelectedDis);
        ui->twCommon->setPalette(mPaletteAlwaysSelectedDis);
    } else {
        ui->leShortName->setPalette(mPaletteNorm);
        ui->leFullName->setPalette(mPaletteNorm);

        ui->twMain->setPalette(mPaletteAlwaysSelected);
        ui->twCommon->setPalette(mPaletteAlwaysSelected);
    }

    ui->tbMinus->setEnabled(lCustomer->DelUserConst().isEmpty());
    ui->tbProps->setEnabled(true);

    ui->tbAddPerson->setEnabled(lCustomer->DelUserConst().isEmpty());
    ui->tbPropPerson->setEnabled(false);
    ui->tbDelPerson->setEnabled(false);

    if (ui->cbClient->isVisible()) {
        ui->cbClient->setCheckState((lCustomer->IsClient() == 1)?Qt::Checked:Qt::Unchecked);
    }
    if (ui->cbProvider->isVisible()) {
        ui->cbProvider->setCheckState((lCustomer->IsProvider() == 1)?Qt::Checked:Qt::Unchecked);
    }

    ui->leShortName->setText(lCustomer->ShortNameConst());
    ui->leFullName->setText(lCustomer->NameConst());

    //QString lFilter = ui->leFilter->text().toLower();
    QTableWidgetItem *lItem;
//    if (ui->twCustProps->isVisible()) {
//        ui->twCustProps->setRowCount(0);
    ui->twCustProps->setRowCount(0);

    // ---------------------------------------------------------------------------
    ui->twCustProps->insertRow(ui->twCustProps->rowCount());
    lItem = new QTableWidgetItem(tr("Short name"));
    lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lItem->setBackgroundColor(palette().color(QPalette::Window));
    lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
    ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 0, lItem);

    lItem = new QTableWidgetItem(lCustomer->ShortNameConst());
    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (!lCustomer->DelUserConst().isEmpty()) {
        lItem->setBackgroundColor(palette().color(QPalette::Window));
    }
    //lItem->setFlags(lItem->flags() & ~Qt::ItemIsEditable);
    ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 1, lItem);

    // ---------------------------------------------------------------------------
    ui->twCustProps->insertRow(ui->twCustProps->rowCount());
    lItem = new QTableWidgetItem(tr("Full name"));
    lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lItem->setBackgroundColor(palette().color(QPalette::Window));
    lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
    ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 0, lItem);

    lItem = new QTableWidgetItem(lCustomer->NameConst());
    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (!lCustomer->DelUserConst().isEmpty()) {
        lItem->setBackgroundColor(palette().color(QPalette::Window));
    }
    //lItem->setFlags(lItem->flags() & ~Qt::ItemIsEditable);
    ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 1, lItem);

    if (mCanSeeCustProps) {
        for (i = 0; i < lCustomer->PropsConst().length(); i++) {
            ui->twCustProps->insertRow(ui->twCustProps->rowCount());

            lItem = new QTableWidgetItem(lCustomer->PropsConst().at(i)->TitleConst());
            //        if (lCustomer->PropsConst().at(i)->TitleConst().contains(QRegExp("[א-ת]"))
            //                && layoutDirection() == Qt::LeftToRight)
            //            lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            //        else
            //            lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lItem->setBackgroundColor(palette().color(QPalette::Window));
            lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
            ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 0, lItem);

            lItem = new QTableWidgetItem(lCustomer->PropsConst().at(i)->ValueConst());
            //            if (lCustomer->PropsConst().at(i)->ValueConst().contains(QRegExp("[א-ת]"))
            //                    && layoutDirection() == Qt::LeftToRight)
            //                lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            //            else
            //                lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            if (!lCustomer->DelUserConst().isEmpty()) {
                lItem->setBackgroundColor(palette().color(QPalette::Window));
            }
            //lItem->setFlags(lItem->flags() & ~Qt::ItemIsEditable);
            ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 1, lItem);
        }
    }

    ui->twMain->setRowCount(0);
    ui->twCommon->setRowCount(0);
    ui->twCommon->setColumnHidden(0, false);

    QString lFullName;
    int lRowForSelect = -1;
    QTableWidgetItem *lScrollToItemMain = NULL, *lScrollToItemCommon = NULL;
    for (i = 0; i < lCustomer->PersonsConst().length();i++) {
        CustomerPerson *lPerson = lCustomer->PersonsConst().at(i);
        lFullName = lPerson->FullName();

        ui->twMain->insertRow(ui->twMain->rowCount());
        ui->twCommon->insertRow(ui->twCommon->rowCount());

        lItem = new QTableWidgetItem(lFullName);
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lItem->setData(Qt::UserRole, QVariant::fromValue(lPerson));
        ui->twMain->setItem(ui->twMain->rowCount() - 1, 0, lItem);

        if (lPerson->Id() == mSelectedPersonId) {
            lRowForSelect = ui->twMain->rowCount() - 1;
            lScrollToItemMain = lItem;
        }

        j = 0;

        lItem = new QTableWidgetItem(lFullName);
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->twCommon->setItem(ui->twCommon->rowCount() - 1, j, lItem);
        j++;

        lItem = new QTableWidgetItem(lPerson->PostConst());
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->twCommon->setItem(ui->twCommon->rowCount() - 1, j, lItem);
        j++;

        if (lPerson->Id() == mSelectedPersonId) {
            lScrollToItemCommon = lItem;
        }

        lItem = new QTableWidgetItem(lPerson->TelMobConst());
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->twCommon->setItem(ui->twCommon->rowCount() - 1, j, lItem);
        j++;

        lItem = new QTableWidgetItem(lPerson->EmailConst());
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->twCommon->setItem(ui->twCommon->rowCount() - 1, j, lItem);
        j++;

        lItem = new QTableWidgetItem(lPerson->Tel1Const());
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->twCommon->setItem(ui->twCommon->rowCount() - 1, j, lItem);
        j++;

        lItem = new QTableWidgetItem(lPerson->Tel2Const());
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->twCommon->setItem(ui->twCommon->rowCount() - 1, j, lItem);
        j++;

//        if (!lFilter.isEmpty()) {
//            if (lPerson->LastNameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
//                    || lPerson->FirstNameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
//                    /*|| lPerson->MiddleNameConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)*/
//                    || lPerson->Tel1Const().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
//                    || lPerson->Tel2Const().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
//                    || lPerson->TelMobConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
//                    || lPerson->EmailConst().toLower().remove(QRegExp("()-'\"=_\\/.,@ ")).contains(lFilter)
//                    ) {
//                lItem->setSelected(true);
//            }
//        }
    }

//    QMessageBox::critical(NULL, QString::number(lRowForSelect), QString::number(mSelectedPersonId));

    StyleSheetChangedInSescendant();

    ui->twCommon->setColumnHidden(0, ui->splitter->sizes().at(0));

    if (lRowForSelect != -1) {
        ui->twMain->setCurrentItem(lScrollToItemMain);
        ui->twMain->selectRow(lRowForSelect);
        if (mPersonScrollPos == -1) {
            ui->twMain->scrollToItem(lScrollToItemMain, QAbstractItemView::PositionAtCenter);
        }

        ui->twCommon->setCurrentItem(lScrollToItemCommon);
        ui->twCommon->selectRow(lRowForSelect);
        if (mPersonScrollPos == -1) {
            ui->twCommon->scrollToItem(lScrollToItemCommon, QAbstractItemView::PositionAtCenter);
        }
    }

    if (mPersonScrollPos != -1) {
        ui->twMain->verticalScrollBar()->setSliderPosition(mPersonScrollPos);
        ui->twCommon->verticalScrollBar()->setSliderPosition(mPersonScrollPos);
    }

    mSelectedPersonId = 0;
    mPersonScrollPos = -1;
}

void OrganizationsListDlg::on_splitter_splitterMoved(int pos, int index) {
    //gLogger->ShowErrorInList(NULL, "", QString::number(pos) + ":" + QString::number(index));
    if (index == 1) {
        ui->twCommon->setColumnHidden(0, pos);
    }
}

void OrganizationsListDlg::on_twCustProps_customContextMenuRequested(const QPoint &pos) {
    QList <QTableWidgetItem *> lSelected = ui->twCustProps->selectedItems();

    bool lAny = false;

    for (int i = 0; i < lSelected.length(); i++) {
        if (!lSelected.at(i)->text().isEmpty()) {
            lAny = true;
            break;
        }
    }

    if (lAny) {
        QMenu lPopup;
        QAction *lARes = NULL, *lACopyToClipboard = NULL;
        lACopyToClipboard = lPopup.addAction(QIcon(":/some/ico/ico/copy.png"), tr("Copy to clipboard"));

        if (lARes = lPopup.exec(QCursor::pos())) {
            if (lARes == lACopyToClipboard) {
                gSettings->CopyToClipboard(ui->twCustProps);
            }
        }
    }
}

void OrganizationsListDlg::on_actionSelect_triggered() {
    emit ui->pbSelect->click();
}

void OrganizationsListDlg::on_actionAdd_to_selection_triggered() {
    if (mDisplayType == DTSelectManyFirm
            || mDisplayType == DTSelectMany) {
        if (ui->listCustomers->currentItem()) {
            CustomerData *lSelectedCustomer = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>();
            if (!mSelectedCustomers.contains(lSelectedCustomer)) {
//                QString lName = lSelectedCustomer->ShortNameConst();

//                bool lJustAdded = false;

//                if (!ui->wdSelected->isVisible()) {
//                    ui->wdSelected->setVisible(true);
//                    lLastHBoxLayout = new QHBoxLayout();
//                    static_cast<QVBoxLayout *>(ui->wdSelected->layout())->addLayout(lLastHBoxLayout);
//                    lLastHBoxLayout->addStretch(); // last stretch
//                    lJustAdded = true;
//                }

//                QLineEdit *lLineEdit = new QLineEdit();
//                QFontMetrics lFontMetrics(lLineEdit->font());
//                lLineEdit->setReadOnly(true);
//                lLineEdit->setText(lName);
//                lLineEdit->setFixedWidth(lFontMetrics.width(lName + "XX"));
//                int lSpacerWidth = lLastHBoxLayout->itemAt(lLastHBoxLayout->count() - 1)->spacerItem()->geometry().width();

//                //QMessageBox::critical(NULL, "", QString::number(lSpacerWidth));
//                if (!lJustAdded && lSpacerWidth < lFontMetrics.width(lName + "XX") + ui->leFilter->height() + 8) {
//                    lLastHBoxLayout = new QHBoxLayout();
//                    static_cast<QVBoxLayout *>(ui->wdSelected->layout())->addLayout(lLastHBoxLayout);
//                    lLastHBoxLayout->addStretch(); // last stretch
//                }

//                QHBoxLayout *lLayout = new QHBoxLayout();
//                lLayout->setSpacing(0);

//                lLayout->addWidget(lLineEdit);
//                QToolButton *lTB = new QToolButton();
//                lTB->setText("-");
//                lTB->setFixedSize(ui->leFilter->height() + 2, ui->leFilter->height() + 2);
//                lTB->setIcon(QIcon(":/some/ico/ico/exit.png"));
//                lLayout->addWidget(lTB);

//                connect(lTB, SIGNAL(pressed()), mSignalMapper, SLOT(map()));
//                lTB->setProperty("customer", QVariant::fromValue(lSelectedCustomer));
//                lTB->setProperty("layout", QVariant::fromValue(lLayout));
//                mSignalMapper->setMapping(lTB, lTB);

//                lLastHBoxLayout->insertLayout(lLastHBoxLayout->count() - 1, lLayout);

                mSelectedCustomers.append(lSelectedCustomer);
                wdSelectedRebuild();
            }
        }
    }
}

void OrganizationsListDlg::on_pbSelect_clicked() {
    if (mDisplayType == DTSelectOneFirm) {
        if (ui->listCustomers->currentItem()) {
            //mCurrentId = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>()->Id();
            mSelectedCustomer = ui->listCustomers->currentItem()->data(Qt::UserRole).value<CustomerData *>();
            accept();
        }
    } else if (mDisplayType == DTSelectOnePerson) {
        if (ui->twMain->currentItem()) {
            mSelectedPerson = ui->twMain->currentItem()->data(Qt::UserRole).value<CustomerPerson *>();
            accept();
        }
    } else if (mDisplayType == DTSelectManyFirm
               || mDisplayType == DTSelectManyPerson
               || mDisplayType == DTSelectMany) {
        accept();
    }

}

void OrganizationsListDlg::on_actionSelectPerson_triggered(){
    emit ui->pbSelect->click();
}

void OrganizationsListDlg::on_actionAddPerson_to_selection_triggered() {
    if (mDisplayType == DTSelectManyPerson
            || mDisplayType == DTSelectMany) {
        if (ui->twMain->currentItem()) {
            CustomerPerson *lSelectedPerson = ui->twMain->currentItem()->data(Qt::UserRole).value<CustomerPerson *>();
            if (!mSelectedPersons.contains(lSelectedPerson)) {
                mSelectedPersons.append(lSelectedPerson);
                wdSelectedRebuild();
            }
        }
    }
}
