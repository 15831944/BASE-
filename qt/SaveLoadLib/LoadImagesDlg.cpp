#include "LoadImagesDlg.h"
#include "ui_LoadImagesDlg.h"

#include "LISettingsDlg.h"

#include <QFileDialog>
#include <QTimer>
#include <QMenu>
#include <QBuffer>
#include <QMdiSubWindow>
#include <QProcess>
#include <QCloseEvent>
#include <QScrollBar>

#include "../VProject/FileUtils.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/typetreeselect.h"
#include "../VProject/PlotListDlg.h"
#include "../VProject/WaitDlg.h"
#include "../VProject/MainWindow.h"

#include "../Login/Login.h"

#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectListDlg.h"

LoadImagesDlg::LoadImagesDlg(int aIdProject, QWidget *parent) :
    mDlgType(MainDoc),
    QFCDialog(parent, true), AcadXchgDialog(),
    ui(new Ui::LoadImagesDlg),
    mJustStarted(true), mIgnoreResize(false),
    mIdProject(aIdProject),
    mInMouseDragImage(false),
    mOutOfMemory(false)
{
    ui->setupUi(this);

    setWindowTitle(tr("Loading images"));

    for (int i = 0; i < ui->twMain->columnCount(); i++)
        ui->twMain->setColumnHidden(i, false);
    ui->twMain->header()->setStretchLastSection(true);

    ui->wdPlotData->setVisible(true);

    TreeWidgetItemLoadDelegate *lItemDelegate = new TreeWidgetItemLoadDelegate(this);
    ui->twMain->setItemDelegate(lItemDelegate);
    connect(lItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));

    InitInConstructor();
}

LoadImagesDlg::LoadImagesDlg(const QStringList &aFilenames, QWidget *parent) :
    mDlgType(AddFilesForNew),
    QFCDialog(parent, true), AcadXchgDialog(),
    ui(new Ui::LoadImagesDlg),
    mJustStarted(true), mIgnoreResize(false),
    mIdProject(0),
    mInMouseDragImage(false),
    mOutOfMemory(false)
{
    ui->setupUi(this);

    setWindowTitle(tr("Loading add. files"));

    for (int i = 3; i < ui->twMain->columnCount(); i++)
        ui->twMain->setColumnHidden(i, true);
    ui->twMain->header()->setStretchLastSection(false);

    ui->wdPlotData->setVisible(false);

    ROPlotListItemDelegate *lItemDelegate = new ROPlotListItemDelegate(this);
    ui->twMain->setItemDelegate(lItemDelegate);

    InitInConstructor();

    ProcessStringList(aFilenames);
}

//LoadImagesDlg::LoadImagesDlg(QSettings &aSettings, QWidget *parent) :
//    QFCDialog(parent, true), AcadXchgDialog(),
//    ui(new Ui::LoadImagesDlg),
//    mJustStarted(true),
//    mIdProject(0),
//    mInMouseDragImage(false)
//{
//    ui->setupUi(this);
//}


void LoadImagesDlg::InitInConstructor() {
//    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::CustomizeWindowHint);

    connect(gMainWindow, SIGNAL(ImageSettingsChanged()), this, SLOT(SettingsChanged()));

    // always selected in normal color (not gray)
    QPalette lPalette = ui->twMain->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->twMain->setPalette(lPalette);
    // ---------------------------------

    // it is line for splitter
    // ----------------------------------------------
    QSplitterHandle *handle = ui->VSplitter->handle(1);
    QBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    QFrame *line = new QFrame(handle);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    //

    // it is line for splitter
    // ----------------------------------------------
    handle = ui->HSplitter->handle(1);
    layout = new QHBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    line = new QFrame(handle);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    //

    ui->lblImage->setBackgroundRole(QPalette::Base);
//    QPalette lPalette = ui->lblImage->palette();
//    lPalette.setColor(QPalette::Base, ui->leDirectory->palette().color(QPalette::Base));
//    ui->lblImage->setPalette(lPalette);

    //ui->lblImage->installEventFilter(this);
    //gMainWindow->installEventFilter(this);
    //QApplication::instance()->installEventFilter(this);

    mLoadImagesDlgAppEventFilter = new LoadImagesDlgAppEventFilter(this);
}

LoadImagesDlg::~LoadImagesDlg()
{
    delete ui;
}

QString LoadImagesDlg::AddToClassName() const {
    return QString::number(mDlgType);
}

//void LoadImagesDlg::SaveState(QSettings &aSettings) {
//    SaveSettings(aSettings);

//    aSettings.setValue("mIdProject", mIdProject);
//    aSettings.setValue("TreeArea", mTreeData->Area());
//    aSettings.setValue("TreeId", mTreeData->Id());

//    int i, lIndex;

//    lIndex = -1;
//    for (i = 0; i < ui->cbVersion->count(); i++) {
//        if (ui->cbVersion->currentText() == ui->cbVersion->itemText(i)) {
//            lIndex = i;
//            break;
//        }
//    }
//    aSettings.setValue("VersionIndex", lIndex);

//    lIndex = -1;
//    for (i = 1; i < ui->cbNameTop->count(); i++) {
//        if (ui->cbNameTop->currentText() == ui->cbNameTop->itemText(i)) {
//            lIndex = i;
//            break;
//        }
//    }
//    aSettings.setValue("NameTopIndex", lIndex);

//    for (i = 0; i < ui->cbNameBottom->count(); i++) {
//        if (ui->cbNameBottom->currentText() == ui->cbNameBottom->itemText(i)) {
//            lIndex = i;
//            break;
//        }
//    }
//    aSettings.setValue("NameBottomIndex", lIndex);

//    aSettings.setValue("CodeWOE", ui->cbCodeWOE->isChecked());
//    aSettings.setValue("NameTopWOE", ui->cbNameTopWOE->isChecked());
//    aSettings.setValue("NameBottomWOE", ui->cbNameBottomWOE->isChecked());

//    aSettings.setValue("ResizeType", ui->rbXY->isChecked()?0:1);

//    aSettings.setValue("MaxWidth", ui->leMaxWidth->text());
//    aSettings.setValue("MaxHeight", ui->leMaxHeight->text());

//    aSettings.setValue("Percent", ui->sbPercent->value());

//    aSettings.setValue("ZoomType", ui->pbZoomToFit->isChecked()?0:1);
//}

const QList<QTreeWidgetItem *> &LoadImagesDlg::SaveSelectedItems() {
    mEditedColumn = ui->twMain->currentColumn();
    mEditedItems = ui->twMain->selectedItems();
    if (ui->twMain->currentItem()
            && !mEditedItems.contains(ui->twMain->currentItem())) {
        mEditedItems.append(ui->twMain->currentItem());
    }
    return mEditedItems;
}

void LoadImagesDlg::RescanForChanges() {
    int i;
    TreeWidgetItemLoad *lItem;
    XchgFileData *lXchgFileData;
    QFileInfoList lChangedFileInfo;
    QList <TreeWidgetItemLoad *> lChangedItems;

    for (int i = 0; i < ui->twMain->topLevelItemCount(); i++) {
        lItem = static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i));
        lXchgFileData = lItem->XchgFileDataRef();
        QFileInfo lFileInfo(lXchgFileData->FileInfoOrigConst().filePath());

        qint64 lPrevFileSize;
        QDateTime lPrevFileDateTime;

        if (lItem->data(0, Qt::UserRole).isNull()) {
            lPrevFileSize = lXchgFileData->FileInfoOrigConst().size();
        } else {
            lPrevFileSize = lItem->data(0, Qt::UserRole).toULongLong();
        }

        if (lItem->data(0, Qt::UserRole + 1).isNull()) {
            lPrevFileDateTime = lXchgFileData->FileInfoOrigConst().lastModified();
        } else {
            lPrevFileDateTime = lItem->data(0, Qt::UserRole + 1).toDateTime();
        }

        if (lFileInfo.size() != lPrevFileSize
                || lFileInfo.lastModified() != lPrevFileDateTime) {
            lChangedFileInfo.append(lFileInfo);
            lChangedItems.append(lItem);
        }
    }

    if (!lChangedFileInfo.isEmpty()) {
        QMessageBox mb(this);
        mb.setWindowFlags(mb.windowFlags() | Qt::WindowMinMaxButtonsHint);
        mb.setIcon(QMessageBox::Question);
        mb.setWindowTitle(windowTitle());
        mb.setText(tr("Some files are changed. Reread them from disk?"));
        QStringList lFileNames;
        for (i = 0; i < lChangedFileInfo.length(); i++)
            lFileNames.append(lChangedFileInfo.at(i).filePath());
        mb.setDetailedText(lFileNames.join('\n'));
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        if (static_cast<QMessageBox::StandardButton>(mb.exec()) == QMessageBox::Yes) {
            for (i = 0; i < lChangedItems.length(); i++) {
                lItem = lChangedItems.at(i);

                lItem->ImageRef() = QImage();

                lItem->setText(1, "");
                lItem->setText(2, "");

                XchgFileData *lXchgFileData = lItem->XchgFileDataRef();
                lXchgFileData->FileInfoOrigRef() = lChangedFileInfo.at(i);

                qint64 lOrigFileSize; // it is dummy now; it is used as sum for directory
                if (!gFileUtils->InitDataForLoad(true, *lXchgFileData, lOrigFileSize)) {
                    //delete lXchgFileData;
                    continue;
                }

                QImage lImage;
                lImage.loadFromData(*lXchgFileData->BinaryDataConst());

                lItem->SetImageWidth(-1);
                lItem->SetImageHeight(-1);
                lItem->SetImageLoadError(false);

                if (!lImage.isNull()) {
                    lItem->SetImageWidth(lImage.width());
                    lItem->SetImageHeight(lImage.height());

                    if (!mOutOfMemory) {
                        if (gSettings->Image.ResizeForPreview == 1
                                /*&& lXchgFileData->BinaryDataConst()->length() > gSettings->Image.MaxFileSize*/
                                && (lImage.width() > gSettings->Image.MaxPreviewWidth
                                    || lImage.height() > gSettings->Image.MaxPreviewHeight)) {
                            lItem->ImageRef() = lImage.scaled(gSettings->Image.MaxPreviewWidth, gSettings->Image.MaxPreviewHeight, Qt::KeepAspectRatio);
                        } else {
                            lItem->ImageRef() = lImage;
                        }
                    } else  {
                        lItem->ImageRef() = QImage();
                    }

                    lItem->setText(1, gSettings->FormatNumber(lXchgFileData->FileInfoOrigConst().size()) + " / " + gSettings->FormatNumber((qint64) lXchgFileData->BinaryDataConst()->length()));
                    lItem->setText(2, QString::number(lItem->ImageWidth()) + " x " + QString::number(lItem->ImageHeight()));

                    if (mDlgType == MainDoc) {
                        QList<tPairIntIntString> lExistingIds;
                        if (!gOracle->CollectAlreadyLoaded(lXchgFileData->HashOrigConst(), lExistingIds)) {
                            // it was error
                            break;
                        }
                        lItem->LoadedPlotsDataEqualRef() = lExistingIds;

                        if (!lItem->LoadedPlotsDataEqualRef().isEmpty()) {
                            lItem->setText(3, tr("Don't load"));
                            lItem->setData(3, Qt::UserRole, 4);

                            lItem->setText(4, QString::number(lItem->LoadedPlotsDataEqualRef().at(0).first.first));
                            ExistingIdChanged(lItem);
                        } else {
                            lItem->LoadedPlotsFNEqualRef().clear();
                            ProjectData *lProject = NULL, *lProjectMain = NULL;
                            if (mIdProject
                                    && (lProject = gProjects->FindByIdProject(mIdProject))) {
                                lProjectMain = lProject;
                                while (lProjectMain->Parent()
                                       && lProjectMain->Parent()->Type() == ProjectData::PDProject) {
                                    lProjectMain = lProjectMain->Parent();
                                }
                            }

                            if (lProject) {
                                for (int j = 0; j < lProject->PlotListConst().length(); j++) {
                                    if (!lProject->PlotListConst().at(j)->BlockNameConst().compare(lItem->text(0), Qt::CaseInsensitive)) {
                                        lItem->LoadedPlotsFNEqualRef().append(qMakePair(qMakePair(lProject->PlotListConst().at(j)->Id(), 0), QString()));
                                        ui->twMain->setColumnHidden(4, false);
                                    }
                                }
                            }

                            if (!lItem->LoadedPlotsFNEqualRef().isEmpty()) {
                                switch (gSettings->Image.LoadWhenFNExists) {
                                case 0:
                                    lItem->setText(3, tr("Instead of"));
                                    lItem->setData(3, Qt::UserRole, 2);
                                    break;
                                case 1:
                                    lItem->setText(3, tr("New version of"));
                                    lItem->setData(3, Qt::UserRole, 3);
                                    break;
                                }

                                lItem->setText(4, QString::number(lItem->LoadedPlotsFNEqualRef().at(0).first.first));
                                ExistingIdChanged(lItem);
                            } else {
                                lItem->setText(3, tr("New"));
                                lItem->setData(3, Qt::UserRole, 1);

                                lItem->setText(4, "");
                            }
                        }
                    }
                } else {
                    // QImage is null, can't load image;
                    gLogger->ShowError(this, windowTitle(), tr("Can't load image") + "\r\n\r\n"
                                       + lXchgFileData->FileInfoOrigConst().filePath());

                }

                lItem->setData(0, Qt::UserRole, QVariant());
                lItem->setData(0, Qt::UserRole + 1, QVariant());
            }

            emit ui->twMain->currentItemChanged(ui->twMain->currentItem(), NULL);
        } else {
            for (i = 0; i < lChangedItems.length(); i++) {
                lItem = lChangedItems.at(i);
                lItem->setData(0, Qt::UserRole, lChangedFileInfo.at(i).size());
                lItem->setData(0, Qt::UserRole + 1, lChangedFileInfo.at(i).lastModified());
            }
        }
    }
}

void LoadImagesDlg::ShowSettings() {
    ui->rbXY->blockSignals(true);
    ui->sbPercent->blockSignals(true);

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

    ui->rbXY->blockSignals(false);
    ui->sbPercent->blockSignals(false);
}

void LoadImagesDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        QFont lFont = ui->lblLowMemory->font();
        lFont.setBold(true);
        if (lFont.pointSize()) {
            lFont.setPointSize(lFont.pointSize() + 2);
        } else if (lFont.pixelSize()) {
            lFont.setPixelSize(lFont.pixelSize() + 2);
        }
        ui->lblLowMemory->setFont(lFont);
        ui->lblLowMemory->setVisible(false);

        lFont = ui->pbLoad->font();
        lFont.setBold(true);
//        if (lFont.pointSize()) {
//            lFont.setPointSize(lFont.pointSize() + 2);
//        } else if (lFont.pixelSize()) {
//            lFont.setPixelSize(lFont.pixelSize() + 2);
//        }
        ui->pbLoad->setFont(lFont);

        ShowSettings();
        QTimer::singleShot(0, this, SLOT(FirstInit()));
        mJustStarted = false;
    }
}

void LoadImagesDlg::closeEvent(QCloseEvent* event) {
    if (mDlgType != MainDoc
            || !ui->twMain->topLevelItemCount()
            || QMessageBox::question(this, windowTitle(), tr("Close this window?")) == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool LoadImagesDlg::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->lblImage) {
        if (ui->saSelf->horizontalScrollBar()->isVisible()
                || ui->saSelf->verticalScrollBar()->isVisible()) {
            if (event->type() == QEvent::MouseButtonPress) {
                QMouseEvent *lEvent = static_cast<QMouseEvent *>(event);
                if (lEvent->button() == Qt::LeftButton) {
                    ui->lblImage->setCursor(Qt::ClosedHandCursor);
                    mMouseX = QCursor::pos().x();
                    mMouseY = QCursor::pos().y();
                    mInMouseDragImage = true;
                    return false;
                }
            } else if (event->type() == QEvent::MouseButtonRelease) {
                QMouseEvent *lEvent = static_cast<QMouseEvent *>(event);
                if (lEvent->button() == Qt::LeftButton) {
                    ui->lblImage->setCursor(Qt::ArrowCursor);
                    mInMouseDragImage = false;
                    return false;
                }
            } else if (mInMouseDragImage && event->type() == QEvent::MouseMove) {

                ui->saSelf->horizontalScrollBar()->setValue(ui->saSelf->horizontalScrollBar()->value() - (QCursor::pos().x() - mMouseX));
                ui->saSelf->verticalScrollBar()->setValue(ui->saSelf->verticalScrollBar()->value() - (QCursor::pos().y() - mMouseY));

                mMouseX = QCursor::pos().x();
                mMouseY = QCursor::pos().y();
                return false;
            }
        } else {
            if (event->type() == QEvent::Wheel) {
                QWheelEvent *lEvent = static_cast<QWheelEvent *>(event);
                QModelIndex lModelIndex = ui->twMain->currentIndex(), lModelIndexNew;
                if (lEvent->angleDelta().y() > 0) {
                    if (lModelIndex.row()) {
                        lModelIndexNew = lModelIndex.sibling(lModelIndex.row() - 1, ui->twMain->currentColumn());
                    } else {
                        lModelIndexNew = lModelIndex.sibling(ui->twMain->topLevelItemCount() - 1, ui->twMain->currentColumn());
                    }
                } else if (lEvent->angleDelta().y() < 0) {
                    if (lModelIndex.row() < ui->twMain->topLevelItemCount() - 1) {
                        lModelIndexNew = lModelIndex.sibling(lModelIndex.row() + 1, ui->twMain->currentColumn());
                    } else {
                        lModelIndexNew = lModelIndex.sibling(0, ui->twMain->currentColumn());
                    }
                }
                ui->twMain->setCurrentIndex(lModelIndexNew);
                ui->twMain->scrollTo(lModelIndexNew, QAbstractItemView::PositionAtCenter);
            }
        }

        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *lEvent = static_cast<QMouseEvent *>(event);

            if (lEvent->button() == Qt::LeftButton) {
                if (ui->pbZoom100->isChecked()) {
                    emit ui->pbZoomToFit->toggle();
                } else {
                    int lWMult = lEvent->x() - (ui->lblImage->width() - ui->lblImage->pixmap()->width()) / 2, lWDiv = ui->lblImage->pixmap()->width();
                    int lHMult = lEvent->y() - (ui->lblImage->height() - ui->lblImage->pixmap()->height()) / 2, lHDiv = ui->lblImage->pixmap()->height();

                    emit ui->pbZoom100->toggle();

                    int lXCent = ui->lblImage->pixmap()->width() * lWMult / lWDiv, lYCent = ui->lblImage->pixmap()->height() * lHMult / lHDiv;
                    lXCent -= ui->saSelf->width() / 2;
                    lYCent -= ui->saSelf->height() / 2;

                    // correct under mouse
                    lXCent += ui->saSelf->width() / 2 - lEvent->x();
                    lYCent += ui->saSelf->height() / 2 - lEvent->y();

                    ui->saSelf->horizontalScrollBar()->setValue(lXCent);
                    ui->saSelf->verticalScrollBar()->setValue(lYCent);
                }
            }
            return false;
        } else if (event->type() == QEvent::Resize
                   && !mIgnoreResize) {
            emit ui->twMain->currentItemChanged(ui->twMain->currentItem(), NULL);
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void LoadImagesDlg::StyleSheetChangedInSescendant() {
    int i, j, k;
    QString lOld;
    // I love qt
    for (k = 0; k < 2; k++) {
        for (i = 0; i < ui->twMain->columnCount() - 1; i++) {

            ui->twMain->resizeColumnToContents(i);
            ui->twMain->setColumnWidth(i, ui->twMain->columnWidth(i) + 15);

            for (j = 0; j < ui->twMain->topLevelItemCount(); j++) {
                QTreeWidgetItem *lItem = ui->twMain->topLevelItem(j);

                if (i == 3) {
                    lOld = lItem->text(3);
                    lItem->setText(3, tr("New version of"));
                }

                QFont lFont(lItem->font(i));
                QFontMetrics lFM(lFont);
                QSize lSize = lFM.size(0, lItem->text(i));
                lSize.setWidth(lSize.width() + 15); // don't know exacxtly
                lSize.setHeight(lSize.height() + gSettings->DocumentTree.AddRowHeight);
                lItem->setSizeHint(i, lSize);

                if (i == 3) {
                    lItem->setText(3, lOld);
                }
            }
        }
    }
}

//void LoadImagesDlg::LoadAdditionalSettings(QSettings &aSettings) {
//}

//void LoadImagesDlg::SaveAdditionalSettings(QSettings &aSettings) {
//}

void LoadImagesDlg::reject() {
    if (mDlgType != MainDoc
            || !ui->twMain->topLevelItemCount()
            || QMessageBox::question(this, windowTitle(), tr("Close this window?")) == QMessageBox::Yes) {
        QFCDialog::reject();
    }
}

void LoadImagesDlg::FirstInit() {
    ui->twMain->setRootIsDecorated(false);

    ui->lblImageInfo->setText("");

    if (mTreeData = gTreeData->FindByGroupId(4)) {
        TreeDataChanged();
    }

    if (mDlgType == MainDoc) {
        if (!mTreeData
                || !SelectFiles()) {
            if (qobject_cast<QMdiSubWindow *>(parent())) {
                QTimer::singleShot(0, parent(), SLOT(close()));
            } else {
                QTimer::singleShot(0, this, SLOT(close()));
            }
        } else {
            ui->leTypeText->setText(mTreeData->FullName());

            ProjectData *lProject;
            if (mIdProject
                    && (lProject = gProjects->FindByIdProject(mIdProject))) {
                ui->leIdProject->setText(QString::number(mIdProject));
                ui->leProjName->setText(lProject->FullShortName());
            }
        }
    } else {
        emit ui->twMain->currentItemChanged(ui->twMain->currentItem(), NULL);
    }
}

void LoadImagesDlg::OnCommitDataTimer() {
    int i, k;

    if (mEditedColumn == 3) {
        for (i = 0; i < mEditedItems.length(); i++) {
            mEditedItems.at(i)->setText(3, mNewString);
            mEditedItems.at(i)->setData(3, Qt::UserRole, mNewInt + 1);
        }
        if (mNewInt == 1 || mNewInt == 2) {
            ui->twMain->setColumnHidden(4, false);

            for (k = 0; k < 2; k++) {
                ui->twMain->resizeColumnToContents(4);
                ui->twMain->setColumnWidth(4, ui->twMain->columnWidth(4) + 15);
            }
        }
    } else if (mEditedColumn == 4) {
        ExistingIdChanged(static_cast<TreeWidgetItemLoad *>(mEditedItems[0]));
    } else if (mEditedColumn > 4) {
        //QMessageBox::critical(NULL, "", mNewString + " --- " + QString::number(mEditedColumn) + " --- " + QString::number(mEditedItems.length()));
        for (i = 0; i < mEditedItems.length(); i++) {
            mEditedItems.at(i)->setText(mEditedColumn, mNewString);

            for (k = 0; k < 2; k++) {
                ui->twMain->resizeColumnToContents(mEditedColumn);
                ui->twMain->setColumnWidth(mEditedColumn, ui->twMain->columnWidth(mEditedColumn) + 15);
                QFont lFont(mEditedItems.at(i)->font(mEditedColumn));
                QFontMetrics lFM(lFont);
                QSize lSize = lFM.size(0, mNewString);
                lSize.setWidth(lSize.width() + 15); // don't know exacxtly
                lSize.setHeight(lSize.height() + gSettings->DocumentTree.AddRowHeight);
                mEditedItems.at(i)->setSizeHint(mEditedColumn, lSize);
            }
        }
    }
}

void LoadImagesDlg::OnCommitData(QWidget *editor) {
    bool lStartTimer = false;
    QLineEdit *lLineEdit;
    QComboBox *lComboBox;

    if (lLineEdit = qobject_cast<QLineEdit *>(editor)) {
        if (!lLineEdit->isReadOnly()) {
            mNewString = lLineEdit->text();
            lStartTimer = true;
        }
    } else if (lComboBox = qobject_cast<QComboBox *>(editor)) {
        mNewInt = lComboBox->currentIndex();
        mNewString = lComboBox->currentText();
        lStartTimer = true;
    }
    if (lStartTimer) {
        QTimer::singleShot(0, this, SLOT(OnCommitDataTimer()));
    }
}

void LoadImagesDlg::SettingsChanged() {
    ShowSettings();
    // reprocess preview
    ProcessList(2);
    emit ui->twMain->currentItemChanged(ui->twMain->currentItem(), NULL);
}

bool LoadImagesDlg::SelectFiles() {
    bool res = false;
    QFileDialog dlg;

    mLoadImagesDlgAppEventFilter->SetInApplicationActivate(true);

    FileType *lFileType = gFileTypeList->FindById(13);
    if (lFileType
            && !lFileType->FileMasks_QTConst().isEmpty()) {
        QStringList lFilters = lFileType->FileMasks_QTConst().split(';');
        if (lFilters.length() > 1) lFilters.removeLast();
        dlg.setNameFilters(lFilters);
    }

    dlg.setDirectory(ui->leDirectory->text());
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    if (dlg.exec() == QDialog::Accepted) {
        ProcessStringList(dlg.selectedFiles());
        res = true;
    }

    mLoadImagesDlgAppEventFilter->SetInApplicationActivate(false);
    return res;
}

void LoadImagesDlg::TreeDataChanged() {
    ui->cbCode->blockSignals(true);
    ui->cbCode->clear();
    ui->cbCode->addItem(tr("File name"));
    ui->cbCode->addItem(tr("File date/time"));
    ui->cbCode->addItem(mTreeData->ActualCode());

    ui->cbCode->setCurrentIndex(2);
    ui->cbCode->blockSignals(false);
}

void LoadImagesDlg::ProcessStringList(const QStringList &aFilenames) {
    WaitDlg lWaitDlg(this);
    lWaitDlg.show();
    lWaitDlg.SetMessage(tr("In queue: loading images...\nIn queue: processing images..."));

    int i, j;
    bool lListIsEmpty = ui->twMain->topLevelItemCount() == 0;
    QStringList lAddedFiles, lNotExists;
    TreeWidgetItemLoad *lItem;

    for (i = 0; i < aFilenames.length(); i++) {
        lWaitDlg.SetMessage(tr("Loading images ") + QString::number(i + 1) + "/" + QString::number(aFilenames.length()) + "...\n" + tr("In queue: processing images"));


        if (!QFile::exists(aFilenames.at(i))) {
            lNotExists.append(aFilenames.at(i));
            continue;
        }

        qint64 lOrigFileSize; // it is dummy now; it is used as sum for directory

        XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(aFilenames.at(i)));
        if (!gFileUtils->InitDataForLoad(true, *lXchgFileData, lOrigFileSize)) {
            delete lXchgFileData;
            continue;
        }

        if (mDlgType == MainDoc) {
            bool lAlreadyIn = false;
            for (j = 0; j < ui->twMain->topLevelItemCount(); j++) {
                XchgFileData *lXchgFileDataExisting = static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(j))->XchgFileDataRef();
                if (lXchgFileDataExisting->HashOrigConst() == lXchgFileData->HashOrigConst()) {
                    lAlreadyIn = true;
                    break;
                }
            }
            if (lAlreadyIn) {
                delete lXchgFileData;
                continue;
            }
        }


        mFiles.append(lXchgFileData);

        if (!lListIsEmpty) lAddedFiles.append(aFilenames.at(i));

        QString lFilenameOnly = aFilenames.at(i).section('/', -1);
        QString lDirectoryOnly = aFilenames.at(i).section('/', 0, -2);

        // directory name
        if (!mDirectories.contains(lDirectoryOnly)) mDirectories.append(lDirectoryOnly);

        // 0 - orig file name
        lItem = new TreeWidgetItemLoad(lXchgFileData);
        lItem->setFlags(lItem->flags() | Qt::ItemIsEditable);

        ui->twMain->addTopLevelItem(lItem);

        lItem->setText(0, lFilenameOnly);
        lItem->setTextAlignment(0, Qt::AlignVCenter | Qt::AlignLeft);

        lItem->setTextAlignment(1, Qt::AlignVCenter | Qt::AlignRight); // 1 - filesize
        lItem->setTextAlignment(2, Qt::AlignCenter); // image size

        if (mDlgType == MainDoc) {
            lItem->setTextAlignment(3, Qt::AlignVCenter | Qt::AlignLeft); // what to do
            lItem->setTextAlignment(4, Qt::AlignVCenter | Qt::AlignRight); // existong ID

            lItem->setTextAlignment(5, Qt::AlignCenter); // version
            lItem->setTextAlignment(6, Qt::AlignVCenter | Qt::AlignLeft); // code

            lItem->setTextAlignment(7, Qt::AlignVCenter | Qt::AlignLeft); // name top
            lItem->setTextAlignment(8, Qt::AlignVCenter | Qt::AlignLeft); // name bottom
            lItem->setTextAlignment(9, Qt::AlignVCenter | Qt::AlignLeft); // comments
        }
    }
    ui->leDirectory->setText(mDirectories.join("; "));

    lWaitDlg.hide();

    ProcessList(7);

    if (!lNotExists.isEmpty()) {
        QMessageBox mb(this);
        mb.setWindowFlags(mb.windowFlags() | Qt::WindowMinMaxButtonsHint);
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(windowTitle());
        mb.setText(QString::number(lNotExists.length()) + tr(" files don't exist!"));
        mb.setDetailedText(lNotExists.join("\n"));
        mb.setStandardButtons(QMessageBox::Close);
        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        mb.exec();
    }

    if (!lListIsEmpty) {
        QMessageBox mb(this);
        mb.setWindowFlags(mb.windowFlags() | Qt::WindowMinMaxButtonsHint);
        mb.setIcon(QMessageBox::Information);
        mb.setWindowTitle(windowTitle());
        if (!lAddedFiles.isEmpty()) {
            mb.setText(QString::number(lAddedFiles.length()) + tr(" files added to list"));
            mb.setDetailedText(lAddedFiles.join("\n"));
        } else {
            mb.setText(tr("No files added to list"));
        }
        mb.setStandardButtons(QMessageBox::Close);
        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        mb.exec();
    }

    if (!ui->twMain->currentItem()
            && ui->twMain->topLevelItemCount()) {
        ui->twMain->setCurrentItem(ui->twMain->topLevelItem(0));
    }

    setFocus();
}

void LoadImagesDlg::ProcessList(uint aFlags) {
    int i, j, k;

    bool lAnyExisting = false;

    ProjectData *lProject = NULL, *lProjectMain = NULL;
    if (mIdProject
            && (lProject = gProjects->FindByIdProject(mIdProject))) {
        lProjectMain = lProject;
        while (lProjectMain->Parent()
               && lProjectMain->Parent()->Type() == ProjectData::PDProject) {
            lProjectMain = lProjectMain->Parent();
        }
    }

    // version
    int lVersionIndex = -1;
    for (k = 0; k < ui->cbVersion->count(); k++) {
        if (ui->cbVersion->currentText() == ui->cbVersion->itemText(k)) {
            lVersionIndex = k;
            break;
        }
    }

    ui->cbCodeWOE->setVisible(!ui->cbCode->currentIndex());

    // name top
    int lNameTopIndex = -1;
    for (k = 1; k < ui->cbNameTop->count(); k++) {
        if (ui->cbNameTop->currentText() == ui->cbNameTop->itemText(k)) {
            lNameTopIndex = k;
            break;
        }
    }
    ui->cbNameTopWOE->setVisible(lNameTopIndex == 1 || lNameTopIndex == 2 || lNameTopIndex == 4);

    // name bottom
    int lNameBottomIndex = -1;
    for (k = 0; k < ui->cbNameBottom->count(); k++) {
        if (ui->cbNameBottom->currentText() == ui->cbNameBottom->itemText(k)) {
            lNameBottomIndex = k;
            break;
        }
    }
    ui->cbNameBottomWOE->setVisible(lNameBottomIndex == 0 || lNameBottomIndex == 1 || lNameBottomIndex == 3);

    qint64 mStartMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();

    mOutOfMemory = false;
    int lCntForCode = 0;

    ui->lblLowMemory->setVisible(false);

    if (aFlags & 2) {
        WaitDlg lWaitDlg(this);
        lWaitDlg.show();
        lWaitDlg.SetMessage(tr("Cleaning images..."));

        int liEnd = ui->twMain->topLevelItemCount();
        // clean at first
        for (i = 0; i < liEnd; i++) {
            static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i))->ImageRef() = QImage();
        }

        mProcessedCnt = 0;
        int lThreadCount = QThread::idealThreadCount();

        if (lThreadCount < 0) lThreadCount = 4;

        while (liEnd < lThreadCount * 2) lThreadCount--;

        if (lThreadCount > 0) {
            if (liEnd >= lThreadCount * 2) {
                int lPerThreadCnt = ui->twMain->topLevelItemCount() / lThreadCount;
                int lPerThreadCntDiv = ui->twMain->topLevelItemCount() % lThreadCount;
                for (i = 0; i < lThreadCount - 1; i++) {
                    ProcessImagesThread *lProcessThread = new ProcessImagesThread(liEnd - lPerThreadCnt - (lPerThreadCntDiv?1:0), liEnd, this, ui->twMain, this);
                    lProcessThread->start(QThread::NormalPriority);
                    mProcessThreads.append(lProcessThread);
                    liEnd = liEnd - lPerThreadCnt - (lPerThreadCntDiv?1:0);
                    if (lPerThreadCntDiv) lPerThreadCntDiv--;
                }
            }
        }

        lWaitDlg.SetMessage(tr("Processing images..."));

        bool lAllCleaned = false;
        int lRetryCnt;
        for (i = 0; i < liEnd; i++) {
            lWaitDlg.SetMessage(tr("Processing images ") + QString::number(i + 1 + mProcessedCnt) + "/" + QString::number(ui->twMain->topLevelItemCount()) + "...");
            TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i));
            XchgFileData *lXchgFileData = lItem->XchgFileDataRef();

            QImage lImage;

            lRetryCnt = 600;
            while (lRetryCnt) {
                lImage.loadFromData(*lXchgFileData->BinaryDataConst());
                if (!lImage.isNull()) break;
                QThread::msleep(5);
                lRetryCnt--;
            }

            lItem->SetImageWidth(-1);
            lItem->SetImageHeight(-1);
            lItem->SetImageLoadError(false);

            if (!lImage.isNull()) {
                lItem->SetImageWidth(lImage.width());
                lItem->SetImageHeight(lImage.height());
                if (!mOutOfMemory) {
                    lRetryCnt = 600;
                    while (!mOutOfMemory && lRetryCnt) {
                        if (gSettings->Image.ResizeForPreview == 1
                                /*&& lXchgFileData->BinaryDataConst()->length() > gSettings->Image.MaxFileSize*/
                                && (lImage.width() > gSettings->Image.MaxPreviewWidth
                                    || lImage.height() > gSettings->Image.MaxPreviewHeight)) {
                            lItem->ImageRef() = lImage.scaled(gSettings->Image.MaxPreviewWidth, gSettings->Image.MaxPreviewHeight, Qt::KeepAspectRatio);
                        } else {
                            lItem->ImageRef() = lImage;
                        }
                        if (!lItem->ImageRef().isNull()) break;
                        QThread::msleep(5);
                        lRetryCnt--;
                    }
                } else {
                    lItem->ImageRef() = QImage();
                    if (!lAllCleaned) {
                        // clean all previous
                        for (j = 0; j < i; j++) {
                            static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(j))->ImageRef() = QImage();
                        }
                        lAllCleaned = true;
                    }
                }
            } else {
                mOutOfMemory = true;
                //clear all previous
                if (!lAllCleaned) {
                    for (j = 0; j < i; j++) {
                        static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(j))->ImageRef() = QImage();
                    }
                    lAllCleaned = true;
                    i--;
                }
            }
        }

        if (!mProcessThreads.isEmpty()) {
            int lProcessedCnt = liEnd;
            bool lAllIsDone;
            do {
                lAllIsDone = true;
                for (i = mProcessThreads.count() - 1; i >= 0; i--) {
                    if (!mProcessThreads.at(i)->isFinished()) {
                        lAllIsDone = false;
                    } else {
                        delete mProcessThreads.at(i);
                        mProcessThreads.removeAt(i);
                    }
                }
                lWaitDlg.SetMessage(tr("Processing images ") + QString::number(lProcessedCnt + mProcessedCnt) + "/" + QString::number(ui->twMain->topLevelItemCount()) + "...");
                if (!lAllIsDone) QThread::msleep(100);
            } while (!lAllIsDone);
        }

        if (mOutOfMemory) {
            ui->lblLowMemory->setVisible(true);

            for (i = 0; i < ui->twMain->topLevelItemCount(); i++) {
                static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i))->ImageRef() = QImage();
            }
        }

        for (i = 0; i < ui->twMain->topLevelItemCount(); i++) {
            TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i));
            if (lItem->ImageWidth() == -1 || lItem->ImageHeight() == -1) {
                lItem->setText(2, "");
            } else {
                lItem->setText(2, QString::number(lItem->ImageWidth()) + " x " + QString::number(lItem->ImageHeight()));
            }
        }
        setFocus();
    }

    if (!(aFlags & 2) || (aFlags & 1)) {
        for (i = 0; i < ui->twMain->topLevelItemCount(); i++) {
            TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i));
            XchgFileData *lXchgFileData = lItem->XchgFileDataRef();

            QString lFileNameWithExt = lXchgFileData->FileInfoOrigConst().fileName();
            QString lFileNameWithoutExt = lFileNameWithExt;

            if (lFileNameWithoutExt.contains('.')) lFileNameWithoutExt = lFileNameWithoutExt.left(lFileNameWithoutExt.lastIndexOf('.'));

            lItem->setText(1, gSettings->FormatNumber(lXchgFileData->FileInfoOrigConst().size()) + " / " + gSettings->FormatNumber((qint64) lXchgFileData->BinaryDataConst()->length()));

            if (mDlgType != MainDoc) continue;

            if (aFlags & 1) {
                QList<tPairIntIntString> lExistingIds;
                if (!gOracle->CollectAlreadyLoaded(lXchgFileData->HashOrigConst(), lExistingIds)) {
                    // it was error
                    break;
                }
                lItem->LoadedPlotsDataEqualRef() = lExistingIds;

                if (!lItem->LoadedPlotsDataEqualRef().isEmpty()) {
                    lItem->setText(3, tr("Don't load"));
                    lItem->setData(3, Qt::UserRole, 4);

                    lItem->setText(4, QString::number(lItem->LoadedPlotsDataEqualRef().at(0).first.first));
                    ExistingIdChanged(lItem);
                }
            }

            // 1 - load new
            // 2 - load in history
            // 3 - load new version
            // 4 - do not load
            if (!lItem->LoadedPlotsDataEqualRef().isEmpty()) {
                // already loaded, equal by data
                lAnyExisting = true;
            } else {
                // check for existing filename (blockname) in current project
                if (aFlags & 4) { // project changed
                    lItem->LoadedPlotsFNEqualRef().clear();

                    if (lProject) {
                        for (j = 0; j < lProject->PlotListConst().length(); j++) {
                            if (!lProject->PlotListConst().at(j)->BlockNameConst().compare(lItem->text(0), Qt::CaseInsensitive)) {
                                lItem->LoadedPlotsFNEqualRef().append(qMakePair(qMakePair(lProject->PlotListConst().at(j)->Id(), 0), QString()));
                                lAnyExisting = true;
                            }
                        }
                    }

                    if (!lItem->LoadedPlotsFNEqualRef().isEmpty()) {
                        switch (gSettings->Image.LoadWhenFNExists) {
                        case 0:
                            lItem->setText(3, tr("Instead of"));
                            lItem->setData(3, Qt::UserRole, 2);
                            break;
                        case 1:
                            lItem->setText(3, tr("New version of"));
                            lItem->setData(3, Qt::UserRole, 3);
                            break;
                        }

                        lItem->setText(4, QString::number(lItem->LoadedPlotsFNEqualRef().at(0).first.first));
                        ExistingIdChanged(lItem);
                    } else {
                        lItem->setText(3, tr("New"));
                        lItem->setData(3, Qt::UserRole, 1);

                        lItem->setText(4, "");
                    }
                }

                if (lItem->data(3, Qt::UserRole) == 1) {
                    // version
                    switch (lVersionIndex) {
                    case 0:
                        lItem->setText(5, lXchgFileData->FileInfoOrigConst().lastModified().toString("dd.MM.yy"));
                        break;
                    case 1:
                        lItem->setText(5, QDate::currentDate().toString("dd.MM.yy"));
                        break;
                    default:
                        lItem->setText(5, ui->cbVersion->currentText());
                        break;
                    }

                    // code
                    switch (ui->cbCode->currentIndex()) {
                    case 0:
                        lItem->setText(6, ui->cbCodeWOE->isChecked()?lFileNameWithoutExt:lFileNameWithExt);
                        break;
                    case 1:
                        lItem->setText(6, lXchgFileData->FileInfoOrigConst().lastModified().toString("dd.MM.yy HH.mm.ss"));
                        break;
                    case 2:
                        if ((aFlags & 4) || (aFlags & 8) || (aFlags & 0x10)) {
                            QString lStr1 = mTreeData->ActualCode();
                            if (lProject) {
                                lProjectMain->CodeTempleReplaceWithDataMain(lStr1);
                                lProject->CodeTempleReplaceWithDataSub(lStr1);
                                lStr1 = lProject->GenerateFixedCode(lStr1, lCntForCode, -1);
                            }
                            lItem->setText(6, lStr1);
                        }
                        break;
                    default:
                        lItem->setText(6, "");
                        break;
                    }

                    // name top
                    switch (lNameTopIndex) {
                    case 1:
                        lItem->setText(7, ui->cbNameTopWOE->isChecked()?lFileNameWithoutExt:lFileNameWithExt);
                        break;
                    case 2:
                        lItem->setText(7, lXchgFileData->FileInfoOrigConst().lastModified().toString("dd.MM.yy")
                                       + " - " + (ui->cbNameTopWOE->isChecked()?lFileNameWithoutExt:lFileNameWithExt));
                        break;
                    case 3:
                    {
                        QString lNumberStr = QString::number(i + 1);
                        if (lNumberStr.length() == 1) lNumberStr.prepend('0');
                        lItem->setText(7, lXchgFileData->FileInfoOrigConst().lastModified().toString("dd.MM.yy")
                                       + " - " + lNumberStr);
                    }
                        break;
                    case 4:
                        lItem->setText(7, QDate::currentDate().toString("dd.MM.yy")
                                       + " - " + (ui->cbNameTopWOE->isChecked()?lFileNameWithoutExt:lFileNameWithExt));
                        break;
                    case 5:
                    {
                        QString lNumberStr = QString::number(i + 1);
                        if (lNumberStr.length() == 1) lNumberStr.prepend('0');
                        lItem->setText(7, QDate::currentDate().toString("dd.MM.yy")
                                       + " - " + lNumberStr);
                    }
                        break;
                    default:
                        lItem->setText(7, ui->cbNameTop->currentText());
                        break;
                    }

                    // name bottom
                    switch (lNameBottomIndex) {
                    case 0:
                        lItem->setText(8, ui->cbNameBottomWOE->isChecked()?lFileNameWithoutExt:lFileNameWithExt);
                        break;
                    case 1:
                        lItem->setText(8, lXchgFileData->FileInfoOrigConst().lastModified().toString("dd.MM.yy")
                                       + " - " + (ui->cbNameBottomWOE->isChecked()?lFileNameWithoutExt:lFileNameWithExt));
                        break;
                    case 2:
                    {
                        QString lNumberStr = QString::number(i + 1);
                        if (lNumberStr.length() == 1) lNumberStr.prepend('0');
                        lItem->setText(8, lXchgFileData->FileInfoOrigConst().lastModified().toString("dd.MM.yy")
                                       + " - " + lNumberStr);
                    }
                        break;
                    case 3:
                        lItem->setText(8, QDate::currentDate().toString("dd.MM.yy")
                                       + " - " + (ui->cbNameBottomWOE->isChecked()?lFileNameWithoutExt:lFileNameWithExt));
                        break;
                    case 4:
                    {
                        QString lNumberStr = QString::number(i + 1);
                        if (lNumberStr.length() == 1) lNumberStr.prepend('0');
                        lItem->setText(8, QDate::currentDate().toString("dd.MM.yy")
                                       + " - " + lNumberStr);
                    }
                        break;
                    default:
                        lItem->setText(8, ui->cbNameBottom->currentText());
                        break;
                    }

                    // comments
                    lItem->setText(9, ui->leComments->text());

                    lCntForCode++;
                }
            }
        }

        ui->twMain->setColumnHidden(4, !lAnyExisting);
        StyleSheetChangedInSescendant();
    }

    ui->lblProcessTime->setText("Processed in, sec: " + QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - mStartMSecsSinceEpoch)) / 1000));
}

void LoadImagesDlg::Convert(bool aAll) { // true for all, false for selected
    int lWidth, lHeight, lPercent;

    if (ui->rbXY->isChecked()) {
        lWidth = ui->leMaxWidth->text().toInt();
        lHeight = ui->leMaxHeight->text().toInt();

        if (lWidth < 32) {
            QMessageBox::critical(this, windowTitle(), tr("New maximum width is too small!"));
            ui->leMaxWidth->setFocus();
            return;
        }
        if (lHeight < 32) {
            QMessageBox::critical(this, windowTitle(), tr("New maximum height is too small!"));
            ui->leMaxHeight->setFocus();
            return;
        }
    } else if (ui->rbPercent->isChecked()) {
        lPercent = ui->sbPercent->value();
    }

    quint64 lMaxFileSize = ui->leMaxFileSize->text().toUInt();

    WaitDlg lWaitDlg(this);
    lWaitDlg.show();
    lWaitDlg.SetMessage(tr("In queue: processing images..."));

    for (int i = 0; i < ui->twMain->topLevelItemCount(); i++) {
        lWaitDlg.SetMessage(tr("Processing images ") + QString::number(i + 1) + "/" + QString::number(ui->twMain->topLevelItemCount()) + "...");
        TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i));
        if (aAll || lItem->isSelected()) {
            QImage &lImage = lItem->ImageRef();
            if (lItem->XchgFileDataRef()->FileInfoOrigConst().size() > lMaxFileSize) {
                lImage.load(lItem->XchgFileDataRef()->FileInfoOrigConst().filePath());
                if (!lImage.isNull()) {
                    bool lHasChanged = false;
                    if (ui->rbXY->isChecked()) {
                        if (lImage.width() > lWidth || lImage.height() > lHeight) {
                            lImage = lImage.scaled(lWidth, lHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            lHasChanged = true;
                        }
                    } else if (ui->rbPercent->isChecked()) {
                        lImage = lImage.scaled(lImage.width() * lPercent / 100, lImage.height() * lPercent / 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        lHasChanged = true;
                    }

                    if (lHasChanged) {
                        lItem->XchgFileDataRef()->BinaryDataRef()->clear();
                        QBuffer lBuffer(lItem->XchgFileDataRef()->BinaryDataRef());
                        lBuffer.open(QIODevice::WriteOnly);
                        lImage.save(&lBuffer, lItem->XchgFileDataRef()->FileInfoOrigConst().suffix().toLatin1());
                        lBuffer.close();

                        lItem->setText(1, gSettings->FormatNumber(lItem->XchgFileDataRef()->FileInfoOrigConst().size()) + " / " + gSettings->FormatNumber((qint64) lItem->XchgFileDataRef()->BinaryDataConst()->length()));
                        lItem->setText(2, QString::number(lImage.width()) + " x " + QString::number(lImage.height()));
                    }
                } else {
                    QMessageBox::critical(this, windowTitle(), tr("Can't load image from file\n") + lItem->XchgFileDataRef()->FileInfoOrigConst().filePath());
                }
            }
        }
    }
    setFocus();
}

void LoadImagesDlg::ExistingIdChanged(TreeWidgetItemLoad *lItem) {
    // fill with existing plot data
    PlotData *lPlot = gProjects->FindByIdPlot(lItem->text(4).toInt());

    if (!lPlot) {
        lItem->setBackgroundColor(4, lItem->backgroundColor(3));
        return;
    } else if (lItem->IsPlotEqualByData()) {
       lItem->setBackgroundColor(4, QColor(0xff, 0x00, 0x00));
       lItem->setToolTip(4, tr("This file already loaded in Project Base"));
    } else if (lItem->IsPlotEqualByFilename()) {
        lItem->setBackgroundColor(4, QColor(0xff, 0xff, 0x00));
        lItem->setToolTip(4, tr("File with this name already loaded in this project"));
    } else {
        lItem->setBackgroundColor(4, lItem->backgroundColor(3));
    }

    lItem->setText(5, lPlot->VersionIntConst());
    lItem->setText(6, lPlot->CodeConst());
    lItem->setText(7, lPlot->NameTopConst());
    lItem->setText(8, lPlot->NameConst());
    lItem->setText(9, lPlot->NotesConst());
}

void LoadImagesDlg::on_cbVersion_editTextChanged(const QString &arg1) {
    ProcessList(0);
}

void LoadImagesDlg::on_toolButton_clicked() {
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(mIdProject);

    if (dSel.exec() == QDialog::Accepted
            && mIdProject != dSel.GetProjectData()->Id()) {
        mIdProject = dSel.GetProjectData()->Id();
        ui->leIdProject->setText(QString::number(mIdProject));
        ui->leProjName->setText(dSel.GetProjectData()->FullShortName());
        ProcessList(4);
    }
}

void LoadImagesDlg::on_leIdProject_editingFinished() {
    int lNewIdProject = ui->leIdProject->text().toInt();
    ProjectData *lProject;
    if (mIdProject != lNewIdProject
            && (lProject = gProjects->FindByIdProject(lNewIdProject))) {
        mIdProject = lNewIdProject;
        ui->leIdProject->setText(QString::number(mIdProject));
        ui->leProjName->setText(lProject->FullShortName());
        ProcessList(4);
    } else {
        ui->leIdProject->setText(QString::number(mIdProject));
    }
}

void LoadImagesDlg::on_tbTreeSel_clicked() {
    TypeTreeSelect dSel(mTreeData, this);

    if (dSel.exec() == QDialog::Accepted) {
        mTreeData = dSel.GetSelected();
        ui->leTypeText->setText(mTreeData->FullName());
        TreeDataChanged();
        ProcessList(8);
    }
}

void LoadImagesDlg::on_cbNameTop_editTextChanged(const QString &arg1) {
    ProcessList(0);
}

void LoadImagesDlg::on_cbNameBottom_editTextChanged(const QString &arg1) {
    ProcessList(0);
}

void LoadImagesDlg::on_leComments_textEdited(const QString &arg1) {
    ProcessList(0);
}

void LoadImagesDlg::on_cbCode_currentIndexChanged(int index) {
    ProcessList(0x10);
}

void LoadImagesDlg::on_pbLoad_clicked() {
    if (mDlgType != MainDoc) return;

    int i;

    ui->twMain->clearSelection();

    // checking
    for (i = 0; i < ui->twMain->topLevelItemCount(); i++) {
        QTreeWidgetItem *lItem = ui->twMain->topLevelItem(i);
        int lWhatToDo = lItem->data(3, Qt::UserRole).toInt();

        if (lWhatToDo == 4) continue; // don't load, need not any checking

        if ((lWhatToDo == 2
                || lWhatToDo == 3)) {
            // document in base must be specified
            if (lItem->text(4).isEmpty()) {
                QMessageBox::critical(this, windowTitle(), tr("Document must be specified!"));
                lItem->setSelected(true);
                return;
            }
            PlotData *lPlot = gProjects->FindByIdPlot(lItem->text(4).toInt());

            if (!lPlot) {
                QMessageBox::critical(this, windowTitle(), tr("Document with ID = ") + lItem->text(4) + tr(" not found!"));
                lItem->setSelected(true);
                return;
            }
            lItem->setData(4, Qt::UserRole, QVariant::fromValue(lPlot)); // need not found it anymore
        }

        if (lItem->text(5).isEmpty()) {
            QMessageBox::critical(this, windowTitle(), tr("Version must be specified!"));
            lItem->setSelected(true);
            return;
        }

        if (lItem->text(6).isEmpty()) {
            QMessageBox::critical(this, windowTitle(), tr("Code must be specified!"));
            lItem->setSelected(true);
            return;
        }

        if (lItem->text(8).isEmpty()) {
            QMessageBox::critical(this, windowTitle(), tr("Bottom name must be specified!"));
            lItem->setSelected(true);
            return;
        }

    }

    MyMutexLocker lLocker(gMainWindow->UpdateMutex(), 5000);
    if (!lLocker.IsLocked()) {
        gLogger->ShowError(this, windowTitle(), tr("In updating. Try again in a few seconds."));
        return; // skip to next timer cycle
    }

    QList<TreeWidgetItemLoad *> lSuccessfully;

    WaitDlg lWaitDlg(this);
    lWaitDlg.show();
    lWaitDlg.SetMessage(tr("Loading images..."));

    for (i = 0; i < ui->twMain->topLevelItemCount(); i++) {
        lWaitDlg.SetMessage(tr("Loading images ") + QString::number(i + 1) + "/" + QString::number(ui->twMain->topLevelItemCount()) + "...");

        TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(ui->twMain->topLevelItem(i));
        int lWhatToDo = lItem->data(3, Qt::UserRole).toInt();

        if (lWhatToDo == 4) {
            // don't load
            lSuccessfully.append(lItem);
            continue;
        }

        PlotData *lPlot = NULL;

        if (lWhatToDo == 2
                || lWhatToDo == 3) {
            lPlot = lItem->data(4, Qt::UserRole).value<PlotData *>();
            lPlot->InitIdDwgMax();
        }

        int lIdPlot, lIdCommon;
        int lIdDwgMax; // used as previous ID from dwg for coping xrefs;
        int lDWGMaxVersion;

        if (lWhatToDo == 1) {
            // new
            lIdPlot = 0;
            lIdCommon = 0;
            lIdDwgMax = 0;
            lDWGMaxVersion = 0;
        } else if (lWhatToDo == 2) {
            // 2 - history
            lIdPlot = lPlot->Id();
            lIdDwgMax = lPlot->IdDwgMax(); // for copy xrefs bugaga
            lDWGMaxVersion = lPlot->DwgVersionMax();
        } else if (lWhatToDo == 3) {
            // 3 - new version
            lIdPlot = 0;
            lIdCommon = lPlot->IdCommon();
            lIdDwgMax = lPlot->IdDwgMax(); // for copy xrefs bugaga
            lDWGMaxVersion = lPlot->DwgVersionMax();
        }

        if (db.transaction()) {
            quint64 lNewIdDwg = 0;
            bool lIsOk;
            XchgFileData *lXchgFileData = lItem->XchgFileDataRef();
            lIsOk = (lWhatToDo == 2
                        || PlotData::INSERT(lIdPlot, lIdCommon, mIdProject, mTreeData->Area(), mTreeData->Id(), lItem->text(5)/*ver*/, lItem->text(5)/*ver*/,
                                            "", "", lItem->text(6), "",
                                            lItem->text(7)/*name top*/, lItem->text(8)/*name bottom*/, lItem->text(0)/*block name*/, lItem->text(9)))
                    && PlotData::LOADFROMFILE(true, lIdPlot, lNewIdDwg, lIdDwgMax, lDWGMaxVersion,
                                              lXchgFileData->FileInfoOrigConst(), lXchgFileData->FileInfoOrigConst().size(), lXchgFileData->HashOrigConst(),
                                              *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                              lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef(), false, this);

            if (lIsOk
                    && lWhatToDo == 3) {
                lPlot->setWorking(0);
                lIsOk = lPlot->SaveData();
            }
            if (lIsOk) {
                if (lIsOk = db.commit()) {
                    lSuccessfully.append(lItem);
                    if (lWhatToDo == 3) {
                        lPlot->CommitEdit();
                    }
                } else {
                    gLogger->ShowSqlError(this, windowTitle(), tr("Can't commit"), db);
                }
            }
            if (!lIsOk) {
                db.rollback();
                if (lWhatToDo == 3) {
                    lPlot->RollbackEdit();
                }
            }
        } else {
            gLogger->ShowSqlError(this, windowTitle(), tr("Can't start transaction"), db);
        }
    }

    for (i = 0; i < lSuccessfully.length(); i++) {
        TreeWidgetItemLoad *lItem = lSuccessfully.at(i);
        XchgFileData *lXchgFileData = lItem->XchgFileDataRef();
        mFiles.removeAll(lXchgFileData);
        delete lItem;
        delete lXchgFileData;
    }

    gProjects->UpdatePlotList(mIdProject);

    setFocus();

    if (!ui->twMain->topLevelItemCount()) accept();
}

void LoadImagesDlg::on_cbCodeWOE_toggled(bool checked) {
    ProcessList(0);
}

void LoadImagesDlg::on_cbNameTopWOE_toggled(bool checked) {
    ProcessList(0);
}

void LoadImagesDlg::on_cbNameBottomWOE_toggled(bool checked) {
    ProcessList(0);
}

void LoadImagesDlg::on_twMain_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    mIgnoreResize = true;
    ui->lblImage->setPixmap(QPixmap());
    ui->lblImageInfo->setText("");

    if (current
            && !static_cast<TreeWidgetItemLoad *>(current)->ImageLoadError()) {
        QImage lImage = static_cast<TreeWidgetItemLoad *>(current)->ImageRef();
        int lWidthOrig = lImage.width(), lHeightOrig = lImage.height();
        if (lImage.isNull()) {
            lImage.loadFromData(*static_cast<TreeWidgetItemLoad *>(current)->XchgFileDataRef()->BinaryDataConst());
            if (!lImage.isNull()) {
                lWidthOrig = lImage.width();
                lHeightOrig = lImage.height();
            }
        }
        if (!lImage.isNull()) {
            if (ui->pbZoomToFit->isChecked()) {
                lImage = lImage.scaled(ui->lblImage->width(), ui->lblImage->height(), Qt::KeepAspectRatio);
            } else {
                ui->saContent->resize(lImage.width(), lImage.height());
            }
            ui->lblImage->setPixmap(QPixmap::fromImage(lImage));
            ui->lblImageInfo->setText(QString::number(lWidthOrig) + " x " + QString::number(lHeightOrig)
                                      + " - " + QString::number(ui->lblImage->width()) + " x " + QString::number(ui->lblImage->height()));
        } else {
            static_cast<TreeWidgetItemLoad *>(current)->SetImageLoadError(true);
        }
    }
    mIgnoreResize = false;
}

void LoadImagesDlg::on_lblImage_customContextMenuRequested(const QPoint &pos) {
    if (ui->twMain->currentItem()) {
        QString lFilename = static_cast<TreeWidgetItemLoad *>(ui->twMain->currentItem())->XchgFileDataRef()->FileInfoOrigConst().filePath();
        lFilename.replace('/' ,'\\');

        QMenu lMenu(this);
        QAction *lARes, *lAReload = NULL, *lAView = NULL, *lAEdit = NULL, *lAOpenDir = NULL;

        lAReload = lMenu.addAction(tr("View in original size"));
        lMenu.addSeparator();
        lAView = lMenu.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Open"));
        lAEdit = lMenu.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Open in MS Paint"));
        lAOpenDir = lMenu.addAction(tr("Open directory"));

        if (lARes = lMenu.exec(QCursor::pos())) {
            if (lARes == lAReload) {
                mIgnoreResize = true;
                // clean
                ui->lblImage->setPixmap(QPixmap());
                ui->lblImageInfo->setText("");

                // switch to 100% mode
                ui->pbZoomToFit->blockSignals(true);
                ui->pbZoomToFit->setChecked(false);
                ui->pbZoomToFit->blockSignals(false);

                ui->pbZoom100->blockSignals(true);
                ui->pbZoom100->setChecked(true);
                ui->pbZoom100->blockSignals(false);

                ui->saSelf->setWidgetResizable(false);

                QImage lImage;
                lImage.load(static_cast<TreeWidgetItemLoad *>(ui->twMain->currentItem())->XchgFileDataRef()->FileInfoOrigConst().filePath());

                if (!lImage.isNull()) {
                    ui->saContent->resize(lImage.width(), lImage.height());
                    ui->lblImage->setPixmap(QPixmap::fromImage(lImage));
                    ui->lblImageInfo->setText(QString::number(lImage.width()) + " x " + QString::number(lImage.height())
                                + " - " + QString::number(ui->lblImage->width()) + " x " + QString::number(ui->lblImage->height()));
                }
                mIgnoreResize = false;
            } else if (lARes == lAView) {
                QProcess::startDetached("Explorer \"" + lFilename + "\"");
            } else if (lARes == lAEdit) {
                QProcess::startDetached("mspaint.exe \"" + lFilename + "\"");
            } else if (lARes == lAOpenDir) {
                QProcess::startDetached("Explorer /select, \"" + lFilename + "\"");
            }
        }
    }
}

void LoadImagesDlg::on_pbConvertSelected_clicked() {
    Convert(false);
}

void LoadImagesDlg::on_pbConvertAll_clicked() {
    Convert(true);
}

void LoadImagesDlg::on_leDirectory_customContextMenuRequested(const QPoint &pos) {
    int i;
    QAction *lARes, *lAOpenDir = NULL;
    QList<QAction *> lDirectories;
    QMenu *lPopup = ui->leDirectory->createStandardContextMenu();

    lPopup->addSeparator();

    if (mDirectories.length() > 1) {
        QMenu *lSubMenu = lPopup->addMenu(tr("Open directory"));
        for (i = 0; i < mDirectories.length(); i++) {
            lDirectories.append(lSubMenu->addAction(mDirectories.at(i)));
        }
    } else if (!mDirectories.empty()) {
        lAOpenDir = lPopup->addAction(tr("Open directory"));
    }

    if (lARes = lPopup->exec(QCursor::pos())) {
        if (lARes == lAOpenDir) {
            QString lDir = mDirectories.at(0);
            QProcess::startDetached("Explorer \"" + lDir.replace('/' ,'\\') + "\"");
        } else {
            for (i = 0; i < lDirectories.length(); i++) {
                if (lARes == lDirectories.at(i)) {
                    QString lDir = mDirectories.at(i);
                    QProcess::startDetached("Explorer \"" + lDir.replace('/' ,'\\') + "\"");
                    break;
                }
            }
        }
    }
}

void LoadImagesDlg::on_twMain_customContextMenuRequested(const QPoint &pos) {
    QAction *lARes, *lAView = NULL, *lAEdit = NULL, *lAOpenDir = NULL, *lAGoto = NULL, *lASelect = NULL,
            *lAAdd = NULL, *lARemove = NULL;
    QMenu lPopup;
    QString lFilename;

    if (ui->twMain->currentItem()) {
        if (ui->twMain->currentColumn() < 3) {
            lFilename = static_cast<TreeWidgetItemLoad *>(ui->twMain->currentItem())->XchgFileDataRef()->FileInfoOrigConst().filePath();
            lFilename.replace('/' ,'\\');

            lAView = lPopup.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Open"));
            lAEdit = lPopup.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Open in MS Paint"));
            lAOpenDir = lPopup.addAction(tr("Open directory"));
        } else if (ui->twMain->currentColumn() == 4) {
            if (!ui->twMain->currentItem()->text(4).isEmpty()) {
                lAGoto = lPopup.addAction(tr("Go to document"));
                lPopup.addSeparator();
            }
            if (ui->twMain->currentItem()->data(3, Qt::UserRole).toInt() == 2
                    || ui->twMain->currentItem()->data(3, Qt::UserRole).toInt() == 3) {
                lASelect = lPopup.addAction(tr("Select document"));
            }
        }
        lPopup.addSeparator();
        lAAdd = lPopup.addAction(tr("Add files"));
        if (!ui->twMain->selectedItems().isEmpty()) {
            lARemove = lPopup.addAction(tr("Remove from list"));
        }
    }

    if (!lPopup.actions().isEmpty()
            && (lARes = lPopup.exec(QCursor::pos()))) {
        if (lARes == lAView) {
            QProcess::startDetached("Explorer \"" + lFilename + "\"");
        } else if (lARes == lAEdit) {
            QProcess::startDetached("mspaint.exe \"" + lFilename + "\"");
        } else if (lARes == lAOpenDir) {
            QProcess::startDetached("Explorer /select, \"" + lFilename + "\"");
        } else if (lARes == lAGoto) {
            PlotData * lPlotGoto = gProjects->FindByIdPlot(ui->twMain->currentItem()->text(4).toInt());
            if (lPlotGoto) {
                gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, NULL);
            } else {
                QMessageBox::critical(this, windowTitle(), tr("Cant find document with ID = ") + ui->twMain->currentItem()->text(4));
            }
        } else if (lARes == lASelect) {
            PlotData *lPlot = NULL;
            if (!ui->twMain->currentItem()->text(4).isEmpty()) {
                lPlot = gProjects->FindByIdPlot(ui->twMain->currentItem()->text(4).toInt());
            }
            PlotListDlg w(PlotListDlg::DTShowSelectOne, lPlot, NULL, this);

            w.SetProjectData(gProjects->FindByIdProject(mIdProject));

            if (w.exec() == QDialog::Accepted) {
                lPlot = w.SelectedPlot();
                ui->twMain->currentItem()->setText(4, QString::number(lPlot->Id()));
                //OtherPlotDataChanged();
            }
        } else if (lARes == lAAdd) {
            SelectFiles();
        } else if (lARes == lARemove) {
            if (QMessageBox::question(this, windowTitle(), tr("Remove selected files from list?")) == QMessageBox::Yes) {
                QList <QTreeWidgetItem *> lItems = ui->twMain->selectedItems();
                for (int i = 0; i < lItems.length(); i++) {
                    TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(lItems.at(i));
                    XchgFileData *lXchgFileData = lItem->XchgFileDataRef();
                    mFiles.removeAll(lXchgFileData);
                    delete lItem;
                    delete lXchgFileData;
                }
                ProcessList(0x10);
            }
        }
    }
}

void LoadImagesDlg::on_pbZoomToFit_toggled(bool checked) {
    if (checked) {
        ui->pbZoom100->blockSignals(true);
        ui->pbZoom100->setChecked(false);
        ui->pbZoom100->blockSignals(false);

        ui->saSelf->setWidgetResizable(true);

        emit ui->twMain->currentItemChanged(ui->twMain->currentItem(), NULL);
    } else {
        ui->pbZoomToFit->blockSignals(true);
        ui->pbZoomToFit->setChecked(true);
        ui->pbZoomToFit->blockSignals(false);
    }
}

void LoadImagesDlg::on_pbZoom100_toggled(bool checked) {
    if (checked) {
        ui->pbZoomToFit->blockSignals(true);
        ui->pbZoomToFit->setChecked(false);
        ui->pbZoomToFit->blockSignals(false);

        ui->saSelf->setWidgetResizable(false);

        if (!gSettings->Image.OnPreviewDblClick) {
            emit ui->twMain->currentItemChanged(ui->twMain->currentItem(), NULL);
        } else {
            QImage lImage;
            lImage.load(static_cast<TreeWidgetItemLoad *>(ui->twMain->currentItem())->XchgFileDataRef()->FileInfoOrigConst().filePath());

            if (!lImage.isNull()) {
                mIgnoreResize = true;
                ui->saContent->resize(lImage.width(), lImage.height());
                ui->lblImage->setPixmap(QPixmap::fromImage(lImage));
                ui->lblImageInfo->setText(QString::number(lImage.width()) + " x " + QString::number(lImage.height())
                            + " - " + QString::number(ui->lblImage->width()) + " x " + QString::number(ui->lblImage->height()));
                mIgnoreResize = false;
            }
        }

        ui->saSelf->horizontalScrollBar()->setValue(ui->saSelf->horizontalScrollBar()->maximum() / 2);
        ui->saSelf->verticalScrollBar()->setValue(ui->saSelf->verticalScrollBar()->maximum() / 2);
    } else {
        ui->pbZoom100->blockSignals(true);
        ui->pbZoom100->setChecked(true);
        ui->pbZoom100->blockSignals(false);
    }
}

void LoadImagesDlg::on_rbXY_toggled(bool checked) {
    ui->leMaxWidth->setEnabled(checked);
    ui->leMaxHeight->setEnabled(checked);
    ui->sbPercent->setEnabled(!checked);

    gSettings->Image.ConvertType = checked?0:1;
    gSettings->SaveSettingsImage();
}

//void LoadImagesDlg::on_rbPercent_toggled(bool checked) {
//    QMessageBox::critical(NULL, "", "rbPercent_toggled");
//    ui->leMaxWidth->setEnabled(!checked);
//    ui->leMaxHeight->setEnabled(!checked);
//    ui->sbPercent->setEnabled(checked);

//}

void LoadImagesDlg::on_leMaxWidth_editingFinished() {
    gSettings->Image.MaxConvertWidth = ui->leMaxWidth->text().toInt();
    gSettings->SaveSettingsImage();
}

void LoadImagesDlg::on_leMaxHeight_editingFinished() {
    gSettings->Image.MaxConvertHeight = ui->leMaxHeight->text().toInt();
    gSettings->SaveSettingsImage();
}

void LoadImagesDlg::on_sbPercent_valueChanged(int arg1) {
    gSettings->Image.ConvertPercent = arg1;
    gSettings->SaveSettingsImage();
}

void LoadImagesDlg::on_leMaxFileSize_editingFinished() {
    gSettings->Image.MaxFileSize = ui->leMaxFileSize->text().toUInt();
    gSettings->SaveSettingsImage();
}

// --------------------------------------------------------------------------------
LoadImagesDlgAppEventFilter::LoadImagesDlgAppEventFilter(LoadImagesDlg *aParent) :
    QObject(aParent),
    mInApplicationActivate(false), mLoadImagesDlg(aParent)
{
    QApplication::instance()->installEventFilter(this);
}

void LoadImagesDlgAppEventFilter::SetInApplicationActivate(bool aInApplicationActivate) {
    mInApplicationActivate = aInApplicationActivate;
}

bool LoadImagesDlgAppEventFilter::eventFilter(QObject *obj, QEvent *event) {
    if (obj == QApplication::instance()) {
        if (event->type() == QEvent::ApplicationActivate) {
            if (mInApplicationActivate) return false;
            mInApplicationActivate = true;
            mLoadImagesDlg->RescanForChanges();
            mInApplicationActivate = false;
            return false;
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

// --------------------------------------------------------------------------------
ProcessImagesThread::ProcessImagesThread(int aStart, int aEnd, LoadImagesDlg *aLoadImagesDlg, QTreeWidget *aMain, QObject *parent) :
    QThread(parent),
    mStart(aStart), mEnd(aEnd),
    mLoadImagesDlg(aLoadImagesDlg), mMain(aMain)
{
}

void ProcessImagesThread::run() {
    int i, j, lRetryCnt;
    bool lAllCleaned = false;
    for (i = mStart; i < mEnd; i++) {
        mLoadImagesDlg->mProcessedCnt++;
        //lWaitDlg.SetMessage(tr("Processing images ") + QString::number(i + 1) + "/" + QString::number(ui->twMain->topLevelItemCount()) + "...");
        TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(mMain->topLevelItem(i));
        XchgFileData *lXchgFileData = lItem->XchgFileDataRef();

        if (isInterruptionRequested()) break;
        QImage lImage;

        lRetryCnt = 600;
        while (lRetryCnt && !isInterruptionRequested()) {
            lImage.loadFromData(*lXchgFileData->BinaryDataConst());
            if (!lImage.isNull()) break;
            msleep(5);
            lRetryCnt--;
        }

        if (isInterruptionRequested()) break;

        lItem->SetImageWidth(-1);
        lItem->SetImageHeight(-1);
        lItem->SetImageLoadError(false);

        if (!lImage.isNull()) {
            lItem->SetImageWidth(lImage.width());
            lItem->SetImageHeight(lImage.height());
            if (!mLoadImagesDlg->mOutOfMemory) {
                lRetryCnt = 600;
                while (!mLoadImagesDlg->mOutOfMemory && lRetryCnt && !isInterruptionRequested()) {
                    if (gSettings->Image.ResizeForPreview == 1
                            /*&& lXchgFileData->BinaryDataConst()->length() > gSettings->Image.MaxFileSize*/
                            && (lImage.width() > gSettings->Image.MaxPreviewWidth
                                || lImage.height() > gSettings->Image.MaxPreviewHeight)) {
                        lItem->ImageRef() = lImage.scaled(gSettings->Image.MaxPreviewWidth, gSettings->Image.MaxPreviewHeight, Qt::KeepAspectRatio);
                    } else {
                        lItem->ImageRef() = lImage;
                    }
                    if (!lItem->ImageRef().isNull()) break;
                    msleep(5);
                    lRetryCnt--;
                }
                if (isInterruptionRequested()) break;
            } else {
                lItem->ImageRef() = QImage();
                if (!lAllCleaned) {
                    // clean all previous
                    for (j = mStart; j < i; j++) {
                        static_cast<TreeWidgetItemLoad *>(mMain->topLevelItem(j))->ImageRef() = QImage();
                    }
                    lAllCleaned = true;
                }
            }
        } else {
            mLoadImagesDlg->mOutOfMemory = true;
            if (!lAllCleaned) {
                //clear all previous
                for (j = mStart; j < i; j++) {
                    static_cast<TreeWidgetItemLoad *>(mMain->topLevelItem(j))->ImageRef() = QImage();
                }
                lAllCleaned = true;
                i--;
                mLoadImagesDlg->mProcessedCnt--;
            }
        }
    }

    exit();
}
