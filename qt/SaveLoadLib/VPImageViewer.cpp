#include "VPImageViewer.h"
#include "ui_VPImageViewer.h"

#include "../VProject/WaitDlg.h"
#include "../VProject/FileUtils.h"
#include "../VProject/PlotListTree.h"
#include "../VProject/MainWindow.h"

#include "../ProjectLib/ProjectData.h"

#include "../PlotLib/PlotHistoryTree.h"
#include "../PlotLib/PlotHistTreeItem.h"
#include "../PlotLib/PlotAddFilesTree.h"
#include "../PlotLib/PlotAddFilesTreeItem.h"

#include "../Login//Login.h"

#include "../Logger/logger.h"

#include <QScrollBar>
#include <QCloseEvent>
#include <QDir>
#include <QProcess>

VPImageViewer::VPImageViewer(PlotListTree *aPlotListTree, bool aTrueForEdit, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::VPImageViewer),
    mJustStarted(true), mTrueForEdit(aTrueForEdit),
    mPlotListTree(aPlotListTree), mHistoryTree(NULL), mAddFilesTree(NULL),
    mTimer(new QTimer()),
    mInMouseDragImage(false), mImageViewerThread(NULL)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    ui->lblImage->setBackgroundRole(QPalette::Base);
    ui->lblImage->installEventFilter(this);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(ShowLoadedCnt()));
}

VPImageViewer::VPImageViewer(PlotHistoryTree *aHistoryTree, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::VPImageViewer),
    mJustStarted(true), mTrueForEdit(false),
    mPlotListTree(NULL), mHistoryTree(aHistoryTree), mAddFilesTree(NULL),
    mTimer(new QTimer()),
    mInMouseDragImage(false), mImageViewerThread(NULL)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    ui->lblImage->setBackgroundRole(QPalette::Base);
    ui->lblImage->installEventFilter(this);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(ShowLoadedCnt()));
}

VPImageViewer::VPImageViewer(PlotAddFilesTree *aAddFilesTree, QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::VPImageViewer),
    mJustStarted(true), mTrueForEdit(false),
    mPlotListTree(NULL), mHistoryTree(NULL), mAddFilesTree(aAddFilesTree),
    mTimer(new QTimer()),
    mInMouseDragImage(false), mImageViewerThread(NULL)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    ui->lblImage->setBackgroundRole(QPalette::Base);
    ui->lblImage->installEventFilter(this);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(ShowLoadedCnt()));
}


VPImageViewer::~VPImageViewer() {
    if (mImageViewerThread) {
        mImageViewerThread->requestInterruption();
        while (mImageViewerThread->isRunning()) QThread::msleep(100);
        delete mImageViewerThread;
    }

    delete ui;
}

void VPImageViewer::ShowImage() {
    int lIndex;
    bool lInRetry = false;

    ui->lblImage->setPixmap(QPixmap());
    ui->lblImageInfo->setText("");

    ui->lblCount->setText("");

    if (!mImageViewerThread) return;

    if (mPlotListTree
            && mPlotListTree->currentItem()
            && (static_cast<PlotListTreeItem *>(mPlotListTree->currentItem())->PlotConst()
                && static_cast<PlotListTreeItem *>(mPlotListTree->currentItem())->PlotConst()->IsPicture()
                || static_cast<PlotListTreeItem *>(mPlotListTree->currentItem())->PlotHistoryConst()
                && static_cast<PlotListTreeItem *>(mPlotListTree->currentItem())->PlotHistoryConst()->IsPicture()
                || static_cast<PlotListTreeItem *>(mPlotListTree->currentItem())->PlotAddFileConst()
                && static_cast<PlotListTreeItem *>(mPlotListTree->currentItem())->PlotAddFileConst()->IsPicture())
            || mHistoryTree
                && mHistoryTree->currentItem()
                && static_cast<PlotHistTreeItem *>(mHistoryTree->currentItem())->HistoryConst()->IsPicture()
            || mAddFilesTree
                && mAddFilesTree->currentItem()
                && static_cast<PlotAddFilesTreeItem *>(mAddFilesTree->currentItem())->AddFileConst()->IsPicture()) {
ShowImageRetry:
        if (mPlotListTree
                    && (lIndex = mImageViewerThread->ImageMIListRef().indexOf(mPlotListTree->currentIndex().sibling(mPlotListTree->currentIndex().row(), 0))) != -1
                || mHistoryTree
                    && (lIndex = mImageViewerThread->ImageMIListRef().indexOf(mHistoryTree->currentIndex().sibling(mHistoryTree->currentIndex().row(), 0))) != -1
                || mAddFilesTree
                    && (lIndex = mImageViewerThread->ImageMIListRef().indexOf(mAddFilesTree->currentIndex().sibling(mAddFilesTree->currentIndex().row(), 0))) != -1) {
            if (mPlotListTree) {
                ui->lblCount->setText(QString::number(mPlotListTree->currentIndex().row() + 1) + " / " + QString::number(mImageViewerThread->ImageListRef().length()));
            } else if (mHistoryTree) {
                ui->lblCount->setText(QString::number(mHistoryTree->currentIndex().row() + 1) + " / " + QString::number(mImageViewerThread->ImageListRef().length()));
            } else if (mAddFilesTree) {
                ui->lblCount->setText(QString::number(mAddFilesTree->currentIndex().row() + 1) + " / " + QString::number(mImageViewerThread->ImageListRef().length()));
            }

            mImageViewerThread->SetRequiredIndex(lIndex);
            while (!mImageViewerThread->ImageListDoneListRef().at(lIndex) && mImageViewerThread->isRunning()) QThread::msleep(100);

            QImage lImage;
            int lWOrig = mImageViewerThread->ImageListRef().at(lIndex)->width(), lHOrig = mImageViewerThread->ImageListRef().at(lIndex)->height();

            if (ui->pbZoomToFit->isChecked()) {
                lImage = mImageViewerThread->ImageListRef().at(lIndex)->scaled(ui->lblImage->width(), ui->lblImage->height(), Qt::KeepAspectRatio);
            } else {
                lImage = *mImageViewerThread->ImageListRef().at(lIndex);
                ui->saContent->resize(lImage.width(), lImage.height());
            }
            ui->lblImage->setPixmap(QPixmap::fromImage(lImage));
            ui->lblImageInfo->setText(QString::number(lWOrig) + " x " + QString::number(lHOrig)
                                      + " - " + QString::number(lImage.width()) + " x " + QString::number(lImage.height()));
        } else {
            if (!lInRetry) {
                // reinit and retry
                if (mImageViewerThread) {
                    mImageViewerThread->requestInterruption();
                    while (mImageViewerThread->isRunning()) QThread::msleep(100);
                    delete mImageViewerThread;
                }

                ThreadStart();

                lInRetry = true;
                goto ShowImageRetry;
            }
        }
    }
}

void VPImageViewer::ThreadStart() {
    if (mPlotListTree) {
        mImageViewerThread = new ImageViewerThread(ImageViewerThread::FromMainWindow, mPlotListTree, this, false,
                                                   mTrueForEdit,
                                                   mTrueForEdit?(gSettings->Image.EditorType && gSettings->Image.SaveAllForEditor):(!gSettings->Image.ViewerType || gSettings->Image.SaveAll),
                                                   gSettings->Image.ResizeForPreview?gSettings->Image.MaxPreviewWidth:1e6,
                                                   gSettings->Image.ResizeForPreview?gSettings->Image.MaxPreviewHeight:1e6);
    } else if (mHistoryTree) {
        mImageViewerThread = new ImageViewerThread(ImageViewerThread::FromHistWindow, mHistoryTree, this, false,
                                                   gSettings->Image.ResizeForPreview?gSettings->Image.MaxPreviewWidth:1e6,
                                                   gSettings->Image.ResizeForPreview?gSettings->Image.MaxPreviewHeight:1e6);
    } else if (mAddFilesTree) {
        mImageViewerThread = new ImageViewerThread(ImageViewerThread::FromAddFilesWindow, mAddFilesTree, this, false,
                                                   gSettings->Image.ResizeForPreview?gSettings->Image.MaxPreviewWidth:1e6,
                                                   gSettings->Image.ResizeForPreview?gSettings->Image.MaxPreviewHeight:1e6);
    }

    if (mImageViewerThread) {
        mImageViewerThread->start(QThread::NormalPriority);
        mTimer->start(1000);
    }
}

void VPImageViewer::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        // fill arrays
        ThreadStart();
        ShowImage();
        mJustStarted = false;
    }
}

bool VPImageViewer::eventFilter(QObject *obj, QEvent *event) {
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
                int lIndex;

                if (mPlotListTree
                            && (lIndex = mImageViewerThread->ImageMIListRef().indexOf(mPlotListTree->currentIndex().sibling(mPlotListTree->currentIndex().row(), 0))) != -1
                        || mHistoryTree
                            && (lIndex = mImageViewerThread->ImageMIListRef().indexOf(mHistoryTree->currentIndex().sibling(mHistoryTree->currentIndex().row(), 0))) != -1
                        || mAddFilesTree
                            && (lIndex = mImageViewerThread->ImageMIListRef().indexOf(mAddFilesTree->currentIndex().sibling(mAddFilesTree->currentIndex().row(), 0))) != -1) {
                    if (lEvent->angleDelta().y() > 0) {
                        if (lIndex) {
                            lIndex--;
                        } else {
                            lIndex = mImageViewerThread->ImageMIListRef().length() - 1;
                        }
                    } else if (lEvent->angleDelta().y() < 0) {
                        if (lIndex < mImageViewerThread->ImageMIListRef().length() - 1) {
                            lIndex++;
                        } else {
                            lIndex = 0;
                        }
                    }
                    if (mPlotListTree) {
                        mPlotListTree->setCurrentIndex(mImageViewerThread->ImageMIListRef().at(lIndex));
                        mPlotListTree->scrollTo(mImageViewerThread->ImageMIListRef().at(lIndex), QAbstractItemView::PositionAtCenter);
                    } else if (mHistoryTree) {
                        mHistoryTree->setCurrentIndex(mImageViewerThread->ImageMIListRef().at(lIndex));
                        mHistoryTree->scrollTo(mImageViewerThread->ImageMIListRef().at(lIndex), QAbstractItemView::PositionAtCenter);
                    } else if (mAddFilesTree) {
                        mAddFilesTree->setCurrentIndex(mImageViewerThread->ImageMIListRef().at(lIndex));
                        mAddFilesTree->scrollTo(mImageViewerThread->ImageMIListRef().at(lIndex), QAbstractItemView::PositionAtCenter);
                    }
                }
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
        } else if (event->type() == QEvent::Resize) {
            ShowImage();
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void VPImageViewer::ShowLoadedCnt() {
    int lCnt = 0;
    for (int i = 0; i < mImageViewerThread->ImageListDoneListRef().length(); i++) {
        if (mImageViewerThread->ImageListDoneListRef().at(i)) lCnt++;
    }

    if (lCnt == mImageViewerThread->ImageListDoneListRef().length()) {
        mTimer->stop();
    }

    ui->lblLoadedCnt->setText(QString::number(lCnt) + " / " + QString::number(mImageViewerThread->ImageListDoneListRef().length()));
}

void VPImageViewer::on_pbZoomToFit_toggled(bool checked) {
    if (checked) {
        ui->pbZoom100->blockSignals(true);
        ui->pbZoom100->setChecked(false);
        ui->pbZoom100->blockSignals(false);

        ui->saSelf->setWidgetResizable(true);

        ShowImage();
    } else {
        ui->pbZoomToFit->blockSignals(true);
        ui->pbZoomToFit->setChecked(true);
        ui->pbZoomToFit->blockSignals(false);
    }
}

void VPImageViewer::on_pbZoom100_toggled(bool checked) {
    if (checked) {
        ui->pbZoomToFit->blockSignals(true);
        ui->pbZoomToFit->setChecked(false);
        ui->pbZoomToFit->blockSignals(false);

        ui->saSelf->setWidgetResizable(false);

        ShowImage();

        ui->saSelf->horizontalScrollBar()->setValue(ui->saSelf->horizontalScrollBar()->maximum() / 2);
        ui->saSelf->verticalScrollBar()->setValue(ui->saSelf->verticalScrollBar()->maximum() / 2);
    } else {
        ui->pbZoom100->blockSignals(true);
        ui->pbZoom100->setChecked(true);
        ui->pbZoom100->blockSignals(false);
    }
}

//-------------------------------------------------------------------------------------------
ImageViewerThread::ImageViewerThread(enumFromMainWindow aDummy, PlotListTree *aPlotListTree, QObject *parent, bool aIsExtViewer,
                                     bool aTrueForEdit, bool aTrueForEditAll, int aWidth, int aHeight) :
    QThread(parent),
    mPlotListTree(aPlotListTree), mHistoryTree(NULL), mAddFilesTree(NULL),
    mIsExtViewer(aIsExtViewer), mTrueForEdit(aTrueForEdit), mTrueForEditAll(aTrueForEditAll),
    mWidth(aWidth), mHeight(aHeight),
    mRequiredIndex(-1), mWaitForRequired(false),
    mDeleteDirectory(true)
{
    InitInConstructor();
}

ImageViewerThread::ImageViewerThread(enumFromHistWindow aDummy, PlotHistoryTree *aHistoryTree, QObject *parent, bool aIsExtViewer,
                                     int aWidth, int aHeight) :
    QThread(parent),
    mPlotListTree(NULL), mHistoryTree(aHistoryTree), mAddFilesTree(NULL),
    mIsExtViewer(aIsExtViewer), mTrueForEdit(false), mTrueForEditAll(false),
    mWidth(aWidth), mHeight(aHeight),
    mRequiredIndex(-1), mWaitForRequired(false),
    mDeleteDirectory(true)
{
    InitInConstructor();
}

ImageViewerThread::ImageViewerThread(enumFromAddFilesWindow aDummy, PlotAddFilesTree *aAddFilesTree, QObject *parent, bool aIsExtViewer,
                           int aWidth, int aHeight) :
    QThread(parent),
    mPlotListTree(NULL), mHistoryTree(NULL), mAddFilesTree(aAddFilesTree),
    mIsExtViewer(aIsExtViewer), mTrueForEdit(false), mTrueForEditAll(false),
    mWidth(aWidth), mHeight(aHeight),
    mRequiredIndex(-1), mWaitForRequired(false),
    mDeleteDirectory(true)
{
    InitInConstructor();
}

void ImageViewerThread::InitInConstructor() {
    mDB = QSqlDatabase::cloneDatabase(db, "ImageViewer");

    if (mIsExtViewer) {
        mTempDir = QCoreApplication::applicationDirPath();
        mTempDir.resize(mTempDir.lastIndexOf(QChar('/')));
        mTempDir.resize(mTempDir.lastIndexOf(QChar('/')));

        mTempDir += "/temp/data/";
        if (mTrueForEdit) {
            mTempDir += "e";
        } else {
            mTempDir += "v";
        }
        mTempDir += "Images-" + QDateTime::currentDateTime().toString("yyMMdd-hhmmss");

        QDir lDir;
        if (!(mTempDirCreated = lDir.mkpath(mTempDir))) {
            gLogger->ShowError(tr("Image viewer"), tr("Can't create directory") + "\r\n" + mTempDir);
        }
    }

    QModelIndex lStart;
    if (mPlotListTree) {
        lStart = mPlotListTree->currentIndex().sibling(mPlotListTree->currentIndex().row(), 0);
    } else if (mHistoryTree) {
        lStart = mHistoryTree->currentIndex().sibling(mHistoryTree->currentIndex().row(), 0);
    } else if (mAddFilesTree) {
        lStart = mAddFilesTree->currentIndex().sibling(mAddFilesTree->currentIndex().row(), 0);
    }
    QModelIndex lCur = lStart, lTemp;
    PlotData *lPlot;
    QStringList lSkipForEditing;

    do {
        mImageList.append(new QImage());
        mFileInfos.append(QFileInfo());
        mImageListDoneList.append(false);
        mImageListClearedList.append(false);
        mImageMIList.append(lCur);

        if (mPlotListTree
                && (lPlot = mPlotListTree->itemFromIndex(lCur)->PlotRef())
                && lPlot->IsPicture()) {
            lPlot->InitIdDwgMax();

            if (mTrueForEdit) {
                lPlot->InitEditStatus();
                if (lPlot->ES() == PlotData::PESEditing) {
                    lSkipForEditing.append(QString::number(lPlot->Id()) + " " + lPlot->BlockNameConst());
                } else {

                    // start dwg_edit
                    qulonglong lNewIdDwgEdit = 0;
                    // file_name is temporary mTempDir
                    if (PlotData::STARTEDIT(lNewIdDwgEdit, lPlot->Id(), lPlot->IdDwgMax(), mTempDir)) {
                        mIdDwgEditList.append(lNewIdDwgEdit);
                    } else {
                        mIdDwgEditList.append(0);
                    }
                }

                if (!mTrueForEditAll) break; // save only one
            }
        }

        while (true) {
            lTemp = lCur;
            lCur = lTemp.sibling(lCur.row() + 1, lCur.column());
            if (!lCur.isValid()) {
                lCur = lTemp.sibling(0, lTemp.column());
            }
            if (!lCur.isValid()) break;
            if (lCur == lStart) break;

            if (mPlotListTree
                    && (mPlotListTree->itemFromIndex(lCur)->PlotConst()
                        && mPlotListTree->itemFromIndex(lCur)->PlotConst()->IsPicture()
                    || mPlotListTree->itemFromIndex(lCur)->PlotHistoryConst()
                        && mPlotListTree->itemFromIndex(lCur)->PlotHistoryConst()->IsPicture()
                    || mPlotListTree->itemFromIndex(lCur)->PlotAddFileConst()
                        && mPlotListTree->itemFromIndex(lCur)->PlotAddFileConst()->IsPicture())
                || mHistoryTree
                    && mHistoryTree->itemFromIndex(lCur)->HistoryConst()->IsPicture()
                || mAddFilesTree
                    && mAddFilesTree->itemFromIndex(lCur)->AddFileConst()->IsPicture()) {
                // next found
                break;
            }
        }

        //lCur
    } while (lCur != lStart && lCur.isValid());
    if (!lCur.isValid()) QMessageBox::critical(gMainWindow, "KG/AM", "Author mudag");

    if (!lSkipForEditing.isEmpty()) {
        QWidget *lParent = NULL;
        // we have so may sights to show you
        if (mPlotListTree) {
            lParent = mPlotListTree;
        } else if (mHistoryTree) {
            lParent = mHistoryTree;
        } else if (mAddFilesTree) {
            lParent = mAddFilesTree;
        } else {
            lParent = gMainWindow;
        }
        QMessageBox mb(lParent);
        mb.setWindowFlags(mb.windowFlags() | Qt::WindowMinMaxButtonsHint);
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(tr("Image editing - start"));
        mb.setText(QString::number(lSkipForEditing.length()) + tr(" files editing now!"));
        mb.setDetailedText(lSkipForEditing.join("\n"));
        mb.setStandardButtons(QMessageBox::Close);
        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        mb.exec();
    }
}

ImageViewerThread::~ImageViewerThread() {
    if (mTrueForEdit) {
        QSqlQuery qUpdate(db);

        qUpdate.prepare("update dwg_edit set endtime = current_timestamp where id = :id");

        if (qUpdate.lastError().isValid()) {
            gLogger->ShowSqlError(tr("Image editing - ending"), qUpdate);
        } else {
            QStringList lErrorStrs;
            for (int i = 0; i < mIdDwgEditList.count(); i++) {
                qUpdate.bindValue(":id", mIdDwgEditList.at(i));
                if (qUpdate.exec()) {
                    //
                } else {
                    lErrorStrs.append(QString::number(mIdDwgEditList.at(i)) + " - " + qUpdate.lastError().text());
                }
            }
            if (!lErrorStrs.isEmpty()) {
                gLogger->ShowError(tr("Image editing - ending"), lErrorStrs.join("\r\n"));
                mDeleteDirectory = false;
            }
        }
    }

    qDeleteAll(mImageList);

    if (mIsExtViewer
            && mTempDirCreated
            && mDeleteDirectory
            && !mTempDir.isEmpty()) {
        QDir lDir(mTempDir);
        lDir.removeRecursively();
    }
}

// it is running of external viewer
void ImageViewerThread::ModalViewList(PlotListTree *aPlotListTree) {
    int i;
    ImageViewerThread lImageViewerThread(ImageViewerThread::FromMainWindow, aPlotListTree, aPlotListTree, true, false, false, 0, 0);

    if (!lImageViewerThread.TempDirCreated()) {
        return;
    }

    if (!lImageViewerThread.ImageMIListRef().isEmpty()) {
        lImageViewerThread.start(QThread::NormalPriority);

        if (!gSettings->Image.ViewerType
                || !gSettings->Image.SaveAll) {
            // wait for first
            while (!lImageViewerThread.ImageListDoneListRef().at(0)) {
                QThread::msleep(100);
            }
        } else {
            // wait for all
            WaitDlg lWaitDlg(aPlotListTree);
            lWaitDlg.SetCanCancelled(true);
            lWaitDlg.show();
            lWaitDlg.SetMessage(tr("Saving images to local disk"));

            while (!lImageViewerThread.ImageListDoneListRef().at(lImageViewerThread.ImageListDoneListRef().length() - 1)) {
                for (i = lImageViewerThread.ImageListDoneListRef().length() - 1; i >= 0; i--) {
                    if (lImageViewerThread.ImageListDoneListRef().at(i)) {
                        break;
                    }
                }
                lWaitDlg.SetMessage(tr("Saving images to local disk: ") + QString::number(i + 1) + "/" + QString::number(lImageViewerThread.ImageListDoneListRef().length()));
                if (lWaitDlg.CancelRequested()) {
                    break;
                }
                QThread::msleep(100);
            }
        }

        if (!lImageViewerThread.FileInfosRef().at(0).filePath().isEmpty()) {
            QProcess proc;
            QString lFirstFileName = lImageViewerThread.FileInfosRef().at(0).filePath();

            lFirstFileName.replace('/', '\\');

            if (gSettings->Image.ViewerType == 1) {
                proc.start(QString(qgetenv("COMSPEC")) + " /c \"" + lFirstFileName + "\"");
            } else {
                proc.start(gSettings->Image.ViewerPath + " \"" + lFirstFileName + "\"");
            }
            if (!proc.waitForStarted(-1)) {
                QMessageBox::critical(aPlotListTree, tr("Image viewing"), tr("Can't start program") + "\n" + proc.errorString());
                lImageViewerThread.requestInterruption();
                while (lImageViewerThread.isRunning()) QThread::msleep(100);
            } else {
                if (proc.waitForFinished(-1)) {
                    if (gSettings->Image.ConfirmViewerClosed) {
                        QMessageBox::critical(aPlotListTree, tr("Image viewing"), tr("Viewer closed"));
                        lImageViewerThread.requestInterruption();
                        while (lImageViewerThread.isRunning()) QThread::msleep(100);
                    } else {
                        lImageViewerThread.requestInterruption();
                        while (lImageViewerThread.isRunning()) QThread::msleep(100);
                    }

                    QFileInfoList lChangedFileInfo;
                    QList<const PlotData *> lPlots;

                    for (i = 0; i < lImageViewerThread.ImageMIListRef().length(); i++) {
                        // we can change now only MAIN (plot) records; no changes for add. files or history;
                        if (aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()
                                && aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()->IsPicture()) {
                            if (!lImageViewerThread.FileInfosRef().at(i).filePath().isEmpty()) {
                                QFileInfo lFileInfo(lImageViewerThread.FileInfosRef().at(i).filePath());
                                if (lFileInfo.size() != lImageViewerThread.FileInfosRef().at(i).size()
                                        || lFileInfo.lastModified() != lImageViewerThread.FileInfosRef().at(i).lastModified()) {
                                    lChangedFileInfo.append(lFileInfo);
                                    lPlots.append(aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst());
                                } else {
                                    // remove unneeded file
                                    QFile::remove(lImageViewerThread.FileInfosRef().at(i).filePath());
                                }
                            }
                        }
                    }

                    if (!lChangedFileInfo.isEmpty()) {
                        QMessageBox mb(aPlotListTree);
                        mb.setWindowFlags(mb.windowFlags() | Qt::WindowMinMaxButtonsHint);
                        mb.setIcon(QMessageBox::Question);
                        mb.setWindowTitle(tr("Image viewing"));
                        mb.setText(tr("Some files are changed. Save them to Projects Base?"));
                        QStringList lFileNames;
                        for (i = 0; i < lChangedFileInfo.length(); i++)
                            lFileNames.append(lChangedFileInfo.at(i).fileName());
                        mb.setDetailedText(lFileNames.join("\n"));
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                        // motherfucker motherfucker
                        QSpacerItem * horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                        QGridLayout * layout = (QGridLayout *) mb.layout();
                        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

                        if (static_cast<QMessageBox::StandardButton>(mb.exec()) == QMessageBox::Yes) {
                            WaitDlg lWaitDlg(aPlotListTree);
                            lWaitDlg.show();
                            lWaitDlg.SetMessage(tr("Loading images..."));

                            QStringList lSkipped;
                            for (i = 0; i < lChangedFileInfo.length(); i++) {
                                lWaitDlg.SetMessage(tr("Loading images ") + QString::number(i + 1) + "/" + QString::number(lChangedFileInfo.length()) + "...");
                                const PlotData *lPlot = lPlots.at(i);
                                if (lPlot) {
                                    bool lIsOk = false;
                                    if (db.transaction()) {

                                        qint64 lOrigFileSize; // it is dummy now; it is used as sum for directory
                                        XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(lChangedFileInfo.at(i).filePath()));
                                        if (gFileUtils->InitDataForLoad(true, *lXchgFileData, lOrigFileSize)) {
                                            quint64 lNewIdDwg;
                                            if (PlotData::LOADFROMFILE(true, lPlot->Id(), lNewIdDwg, lPlot->IdDwgMax(), lPlot->DwgVersionMax(),
                                                                       lXchgFileData->FileInfoOrigConst(), lXchgFileData->FileInfoOrigConst().size(), lXchgFileData->HashOrigConst(),
                                                                       *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                                                       lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef(), true, NULL)) {

                                                QSqlQuery qInsert(db);

                                                qInsert.prepare("insert into dwg_edit(id_dwgin, id_dwgout, starttime, endtime, savecount, file_name, lastsave, saved_ftime)"
                                                                " values(:id_old, :id_new, current_timestamp, current_timestamp, 1, :file_name, current_timestamp, :saved_ftime)");

                                                if (qInsert.lastError().isValid()) {
                                                    gLogger->ShowSqlError(aPlotListTree, tr("Image viewing") + " - prepare dwg_edit", qInsert);
                                                } else {
                                                    qInsert.bindValue(":id_old", lPlot->IdDwgMax());
                                                    qInsert.bindValue(":id_new", lNewIdDwg);
                                                    qInsert.bindValue(":file_name", lXchgFileData->FileInfoOrigConst().filePath());
                                                    qInsert.bindValue(":saved_ftime", lXchgFileData->FileInfoOrigConst().lastModified());

                                                    if (qInsert.exec()) {
                                                        lIsOk = true;
                                                    } else {
                                                        gLogger->ShowSqlError(aPlotListTree, tr("Image viewing") + " - execute dwg_edit", qInsert);
                                                    }
                                                }
                                            }
                                        }

                                        delete lXchgFileData;

                                        if (lIsOk) {
                                            if (lIsOk = db.commit()) {
                                                QFile::remove(lChangedFileInfo.at(i).filePath());
                                            } else {
                                                gLogger->ShowSqlError(aPlotListTree, tr("Image viewing"), tr("Can't commit"), db);
                                            }
                                        }
                                        if (!lIsOk) {
                                            db.rollback();
                                        }
                                    } else {
                                        gLogger->ShowSqlError(aPlotListTree, tr("Image viewing"), tr("Can't start transaction"), db);
                                    }

                                    if (!lIsOk) {
                                        lSkipped.append(lChangedFileInfo.at(i).filePath());
                                    }
                                }
                            }

                            if (!lSkipped.isEmpty()) {
                                lImageViewerThread.SetDeleteDirectory(false);
                                QMessageBox::critical(aPlotListTree, tr("Image viewing"), tr("Some files not saved to Projects Base!"));
                                QString lDir(lImageViewerThread.TempDir());
                                QProcess::startDetached("Explorer \"" + lDir.replace('/' ,'\\') + "\"");
                            }
                        }
                    }
                }
            }
        }
    }
}

void ImageViewerThread::ModalEdit(PlotListTree *aPlotListTree) {
    int i;
    ImageViewerThread lImageViewerThread(ImageViewerThread::FromMainWindow, aPlotListTree, aPlotListTree, true,
                                         true, gSettings->Image.EditorType && gSettings->Image.SaveAllForEditor, 0, 0);

    if (!lImageViewerThread.TempDirCreated()) {
        return;
    }

    if (!lImageViewerThread.ImageMIListRef().isEmpty()) {
        lImageViewerThread.start(QThread::NormalPriority);

        if (!gSettings->Image.EditorType
                || !gSettings->Image.SaveAllForEditor) {
            // wait for first
            while (!lImageViewerThread.ImageListDoneListRef().at(0)) {
                QThread::msleep(100);
            }
        } else {
            // wait for all
            WaitDlg lWaitDlg(aPlotListTree);
            lWaitDlg.SetCanCancelled(true);
            lWaitDlg.show();
            lWaitDlg.SetMessage(tr("Saving images to local disk"));

            while (!lImageViewerThread.ImageListDoneListRef().at(lImageViewerThread.ImageListDoneListRef().length() - 1)) {
                for (i = lImageViewerThread.ImageListDoneListRef().length() - 1; i >= 0; i--) {
                    if (lImageViewerThread.ImageListDoneListRef().at(i)) {
                        break;
                    }
                }
                lWaitDlg.SetMessage(tr("Saving images to local disk: ") + QString::number(i + 1) + "/" + QString::number(lImageViewerThread.ImageListDoneListRef().length()));
                if (lWaitDlg.CancelRequested()) {
                    break;
                }
                QThread::msleep(100);
            }
        }

        if (!lImageViewerThread.FileInfosRef().at(0).filePath().isEmpty()) {
            QProcess proc;
            QString lFirstFileName = lImageViewerThread.FileInfosRef().at(0).filePath();

            lFirstFileName.replace('/', '\\');

            if (!gSettings->Image.EditorType) {
                proc.start("mspaint \"" + lFirstFileName + "\"");
            } else {
                proc.start(gSettings->Image.EditorPath + " \"" + lFirstFileName + "\"");
            }
            if (!proc.waitForStarted(-1)) {
                QMessageBox::critical(aPlotListTree, tr("Image editing"), tr("Can't start program") + "\n" + proc.errorString());
                lImageViewerThread.requestInterruption();
                while (lImageViewerThread.isRunning()) QThread::msleep(100);
            } else {
                if (proc.waitForFinished(-1)) {
                    if (gSettings->Image.EditorType
                            && gSettings->Image.ConfirmEditorClosed) {
                        QMessageBox::critical(aPlotListTree, tr("Image editing"), tr("Editor closed"));
                        lImageViewerThread.requestInterruption();
                        while (lImageViewerThread.isRunning()) QThread::msleep(100);
                    } else {
                        lImageViewerThread.requestInterruption();
                        while (lImageViewerThread.isRunning()) QThread::msleep(100);
                    }

                    QList<int> lChangedFiles;

                    for (i = 0; i < lImageViewerThread.ImageMIListRef().length(); i++) {
                        // we can change now only MAIN (plot) records; no changes for add. files or history;
                        if (aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()
                                && aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()->IsPicture()) {
                            if (!lImageViewerThread.FileInfosRef().at(i).filePath().isEmpty()) {
                                QFileInfo lFileInfo(lImageViewerThread.FileInfosRef().at(i).filePath());
                                if (lFileInfo.size() != lImageViewerThread.FileInfosRef().at(i).size()
                                        || lFileInfo.lastModified() != lImageViewerThread.FileInfosRef().at(i).lastModified()) {
                                    // collecting
                                    lChangedFiles.append(i);
                                } else {
                                    // remove unneeded file
                                    QFile::remove(lImageViewerThread.FileInfosRef().at(i).filePath());
                                }
                            }
                        }
                    }

                    if (!lChangedFiles.isEmpty()) {
                        WaitDlg lWaitDlg(aPlotListTree);
                        lWaitDlg.show();
                        lWaitDlg.SetMessage(tr("Loading images..."));

                        QStringList lSkipped;
                        for (i = 0; i < lChangedFiles.length(); i++) {
                            lWaitDlg.SetMessage(tr("Loading images ") + QString::number(i + 1) + "/" + QString::number(lChangedFiles.length()) + "...");
                            const PlotData *lPlot = aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(lChangedFiles.at(i)))->PlotConst();

                            if (lPlot) {
                                bool lIsOk = false;
                                if (db.transaction()) {
                                    qint64 lOrigFileSize; // it is dummy now; it is used as sum for directory
                                    XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(lImageViewerThread.FileInfosRef().at(lChangedFiles.at(i)).filePath()));
                                    if (gFileUtils->InitDataForLoad(true, *lXchgFileData, lOrigFileSize)) {
                                        quint64 lNewIdDwg;
                                        if (PlotData::LOADFROMFILE(true, lPlot->Id(), lNewIdDwg, lPlot->IdDwgMax(), lPlot->DwgVersionMax(),
                                                                   lXchgFileData->FileInfoOrigConst(), lXchgFileData->FileInfoOrigConst().size(), lXchgFileData->HashOrigConst(),
                                                                   *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                                                   lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef(), true, NULL)) {

                                            QSqlQuery qUpdate(db);

                                            qUpdate.prepare("update dwg_edit set id_dwgout = :id_dwgout, file_name = :file_name, saved_ftime = :saved_ftime"
                                                            " where id = :id");

                                            if (qUpdate.lastError().isValid()) {
                                                gLogger->ShowSqlError(aPlotListTree, tr("Image editing") + " - prepare dwg_edit update", qUpdate);
                                            } else {
                                                qUpdate.bindValue(":id_dwgout", lNewIdDwg);
                                                qUpdate.bindValue(":file_name", lXchgFileData->FileInfoOrigConst().filePath());
                                                qUpdate.bindValue(":saved_ftime", lXchgFileData->FileInfoOrigConst().lastModified());
                                                qUpdate.bindValue(":id", lImageViewerThread.IdDwgEditListRef().at(lChangedFiles.at(i)));

                                                if (qUpdate.exec()) {
                                                    lIsOk = true;
                                                } else {
                                                    gLogger->ShowSqlError(aPlotListTree, tr("Image editing") + " - execute dwg_edit update", qUpdate);
                                                }
                                            }
                                        }
                                    }

                                    delete lXchgFileData;

                                    if (lIsOk) {
                                        if (lIsOk = db.commit()) {
                                            QFile::remove(lImageViewerThread.FileInfosRef().at(lChangedFiles.at(i)).filePath());
                                        } else {
                                            gLogger->ShowSqlError(aPlotListTree, tr("Image editing"), tr("Can't commit"), db);
                                        }
                                    }
                                    if (!lIsOk) {
                                        db.rollback();
                                    }
                                } else {
                                    gLogger->ShowSqlError(aPlotListTree, tr("Image editing"), tr("Can't start transaction"), db);
                                }

                                if (!lIsOk) {
                                    lSkipped.append(lImageViewerThread.FileInfosRef().at(lChangedFiles.at(i)).filePath());
                                }
                            }
                        }

                        if (!lSkipped.isEmpty()) {
                            lImageViewerThread.SetDeleteDirectory(false);
                            QMessageBox::critical(aPlotListTree, tr("Image editing"), tr("Some files not saved to Projects Base!"));
                            QString lDir(lImageViewerThread.TempDir());
                            QProcess::startDetached("Explorer \"" + lDir.replace('/' ,'\\') + "\"");
                        }
                    }
                }
            }
        }
    }
}

// it is running of external viewer
void ImageViewerThread::ModalViewList(PlotHistoryTree *aHistoryTree) {
    int i;
    ImageViewerThread lImageViewerThread(ImageViewerThread::FromHistWindow, aHistoryTree, aHistoryTree, true, 0, 0);

    if (!lImageViewerThread.TempDirCreated()) {
        return;
    }

    if (!lImageViewerThread.ImageMIListRef().isEmpty()) {
        lImageViewerThread.start(QThread::NormalPriority);

        if (!gSettings->Image.SaveAll) {
            // wait for first
            while (!lImageViewerThread.ImageListDoneListRef().at(0)) {
                QThread::msleep(100);
            }
        } else {
            // wait for all
            WaitDlg lWaitDlg(aHistoryTree);
            lWaitDlg.SetCanCancelled(true);
            lWaitDlg.show();
            lWaitDlg.SetMessage(tr("Saving images to local disk"));

            while (!lImageViewerThread.ImageListDoneListRef().at(lImageViewerThread.ImageListDoneListRef().length() - 1)) {
                for (i = lImageViewerThread.ImageListDoneListRef().length() - 1; i >= 0; i--) {
                    if (lImageViewerThread.ImageListDoneListRef().at(i)) {
                        break;
                    }
                }
                lWaitDlg.SetMessage(tr("Saving images to local disk: ") + QString::number(i + 1) + "/" + QString::number(lImageViewerThread.ImageListDoneListRef().length()));
                if (lWaitDlg.CancelRequested()) {
                    break;
                }
                QThread::msleep(100);
            }
        }

        if (!lImageViewerThread.FileInfosRef().at(0).filePath().isEmpty()) {
            QProcess proc;
            QString lFirstFileName = lImageViewerThread.FileInfosRef().at(0).filePath();

            lFirstFileName.replace('/', '\\');

            if (gSettings->Image.ViewerType == 1) {
                proc.start(QString(qgetenv("COMSPEC")) + " /c \"" + lFirstFileName + "\"");
            } else {
                proc.start(gSettings->Image.ViewerPath + " \"" + lFirstFileName + "\"");
            }
            if (!proc.waitForStarted(-1)) {
                QMessageBox::critical(aHistoryTree, tr("Image viewing"), tr("Can't start program") + "\n" + proc.errorString());
                lImageViewerThread.requestInterruption();
                while (lImageViewerThread.isRunning()) QThread::msleep(100);
            } else {
                if (proc.waitForFinished(-1)) {
                    if (gSettings->Image.ConfirmViewerClosed) {
                        QMessageBox::critical(aHistoryTree, tr("Image viewing"), tr("Viewer closed"));
                        lImageViewerThread.requestInterruption();
                        while (lImageViewerThread.isRunning()) QThread::msleep(100);
                    } else {
                        lImageViewerThread.requestInterruption();
                        while (lImageViewerThread.isRunning()) QThread::msleep(100);
                    }

/*                    QFileInfoList lChangedFileInfo;
                    QList<int> lPlotIds;

                    for (i = 0; i < lImageViewerThread.ImageMIListRef().length(); i++) {
                        // we can change now only MAIN (plot) records; no changes for add. files or history;
                        if (aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()
                                && aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()->IsPicture()) {
                            if (!lImageViewerThread.FileInfosRef().at(i).filePath().isEmpty()) {
                                QFileInfo lFileInfo(lImageViewerThread.FileInfosRef().at(i).filePath());
                                if (lFileInfo.size() != lImageViewerThread.FileInfosRef().at(i).size()
                                        || lFileInfo.lastModified() != lImageViewerThread.FileInfosRef().at(i).lastModified()) {
                                    lChangedFileInfo.append(lFileInfo);
                                    lPlotIds.append(aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()->Id());
                                }
                            }
                        }
                    }

                    if (!lChangedFileInfo.isEmpty()) {
                        // remove unneeded files
                        for (i = 0; i < lImageViewerThread.ImageMIListRef().length(); i++) {
                            if (aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()
                                    && !lPlotIds.contains(aPlotListTree->itemFromIndex(lImageViewerThread.ImageMIListRef().at(i))->PlotConst()->Id())) {
                                QFile::remove(lImageViewerThread.FileInfosRef().at(i).filePath());
                            }
                        }

                        QMessageBox mb(aPlotListTree);
                        mb.setWindowFlags(mb.windowFlags() | Qt::WindowMinMaxButtonsHint);
                        mb.setIcon(QMessageBox::Question);
                        mb.setWindowTitle(tr("Image viewing"));
                        mb.setText(tr("Some files are changed. Save them to Projects Base?"));
                        QStringList lFileNames;
                        for (i = 0; i < lChangedFileInfo.length(); i++)
                            lFileNames.append(lChangedFileInfo.at(i).fileName());
                        mb.setDetailedText(lFileNames.join("\n"));
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                        // motherfucker motherfucker
                        QSpacerItem * horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                        QGridLayout * layout = (QGridLayout *) mb.layout();
                        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

                        if (static_cast<QMessageBox::StandardButton>(mb.exec()) == QMessageBox::Yes) {
                            WaitDlg lWaitDlg(aPlotListTree);
                            lWaitDlg.show();
                            lWaitDlg.SetMessage(tr("Loading images..."));

                            QStringList lSkipped;
                            for (i = 0; i < lChangedFileInfo.length(); i++) {
                                lWaitDlg.SetMessage(tr("Loading images ") + QString::number(i + 1) + "/" + QString::number(lChangedFileInfo.length()) + "...");
                                PlotData *lPlot = gProjects->FindByIdPlot(lPlotIds.at(i));
                                if (lPlot) {
                                    bool lIsOk = false;
                                    if (db.transaction()) {

                                        qint64 lOrigFileSize; // it is dummy now; it is used as sum for directory
                                        XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(lChangedFileInfo.at(i).filePath()));
                                        if (gFileUtils->InitDataForLoad(true, *lXchgFileData, lOrigFileSize)) {
                                            int lNewIdDwg;

                                            if (PlotData::LOADFROMFILE(true, lPlot->Id(), lNewIdDwg, lPlot->IdDwgMax(), lPlot->DwgVersionMax(),
                                                                       lXchgFileData->FileInfoOrigConst(), lXchgFileData->FileInfoOrigConst().size(), lXchgFileData->HashOrigConst(),
                                                                       *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                                                       lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef(), true)) {

                                                QSqlQuery qInsert(db);

                                                qInsert.prepare("insert into dwg_edit(id_dwgin, id_dwgout, starttime, endtime, savecount)"
                                                                " values(:id_old, :id_new, current_timestamp, current_timestamp, 1)");

                                                if (qInsert.lastError().isValid()) {
                                                    gLogger->ShowSqlError(aPlotListTree, tr("Image viewing") + " - prepare dwg_edit", qInsert);
                                                } else {
                                                    qInsert.bindValue(":id_old", lPlot->IdDwgMax());
                                                    qInsert.bindValue(":id_new", lNewIdDwg);

                                                    if (qInsert.exec()) {
                                                        lIsOk = true;
                                                    } else {
                                                        gLogger->ShowSqlError(aPlotListTree, tr("Image viewing") + " - execute dwg_edit", qInsert);
                                                    }
                                                }
                                            }
                                        }

                                        delete lXchgFileData;

                                        if (lIsOk) {
                                            if (lIsOk = db.commit()) {
                                                QFile::remove(lChangedFileInfo.at(i).filePath());
                                            } else {
                                                gLogger->ShowSqlError(aPlotListTree, tr("Image viewing"), tr("Can't commit"), db);
                                            }
                                        }
                                        if (!lIsOk) {
                                            db.rollback();
                                        }
                                    } else {
                                        gLogger->ShowSqlError(aPlotListTree, tr("Image viewing"), tr("Can't start transaction"), db);
                                    }

                                    if (!lIsOk) {
                                        lSkipped.append(lChangedFileInfo.at(i).filePath());
                                    }
                                }
                            }

                            if (!lSkipped.isEmpty()) {
                                lImageViewerThread.SetDeleteDirectory(false);
                                QMessageBox::critical(aPlotListTree, tr("Image viewing"), tr("Some files not saved to Projects Base!"));
                                QString lDir(lImageViewerThread.TempDir());
                                QProcess::startDetached("Explorer \"" + lDir.replace('/' ,'\\') + "\"");
                            }
                        }
                    }*/
                }
            }
        }
    }
}

void ImageViewerThread::ModalEdit(PlotData *aPlot, PlotHistoryData *aHistory) {
    aPlot->InitIdDwgMax();
    if (aPlot->DwgVersionMax() > aHistory->Num()) {
        if (QMessageBox::question(gMainWindow, tr("Image editing"), tr("Are you sure you want to edit old version of image?")) != QMessageBox::Yes) {
            return;
        }
    }

    QString lTempDir;
    lTempDir = QCoreApplication::applicationDirPath();
    lTempDir.resize(lTempDir.lastIndexOf(QChar('/')));
    lTempDir.resize(lTempDir.lastIndexOf(QChar('/')));

    lTempDir += "/temp/data/eImages-" + QDateTime::currentDateTime().toString("yyMMdd-hhmmss");

    QDir lDir(lTempDir);
    if (!lDir.mkpath(lTempDir)) {
        gLogger->ShowError(tr("Image editing"), tr("Can't create directory") + "\r\n" + lTempDir);
    } else {
        bool lRemoveTempDir = true;
        QString lTempFilename = lTempDir + "/" + aPlot->BlockNameConst();
        // start dwg_edit
        qulonglong lNewIdDwgEdit = 0;
        quint64 lNewIdDwg = 0;
        if (PlotData::STARTEDIT(lNewIdDwgEdit, aPlot->Id(), aHistory->Id(), lTempFilename)) {

            bool lFileIsOk = false;
            QString lFilenameInCache = gSettings->LocalCache.Path + "d-" + QString::number(aHistory->Id());
            if (gSettings->LocalCache.UseLocalCache) {

                if (QFile::copy(lFilenameInCache, lTempFilename)) {
                    gFileUtils->SetFileTime(lFilenameInCache, QDateTime::currentDateTime());
                    lFileIsOk = true;
                }
            }

            QFile lFile(lTempFilename);
            if (!lFileIsOk) {
                QSqlQuery query(db);

                query.prepare("select a.data from v_dwg a where a.id = :id");
                if (query.lastError().isValid()) {
                    gLogger->ShowSqlError(tr("Image editing"), query);
                } else {
                    query.bindValue(":id", aHistory->Id());
                    if (!query.exec()) {
                        gLogger->ShowSqlError(tr("Image editing"), query);
                    } else {
                        if (query.next()) {
                            QByteArray lData(query.value("data").toByteArray());
                            if (gSettings->LocalCache.UseLocalCache) {
                                // write file to cache
                                QFile data(lFilenameInCache);
                                if (gFileUtils->HasEnoughSpace(gSettings->LocalCache.Path, gSettings->LocalCache.MinDiskSize)) {
                                    if (data.open(QFile::WriteOnly)) {
                                        data.write(lData);
                                        data.close();
                                    }
                                } else {
                                    data.remove();
                                }
                            }

                            if (lFile.open(QFile::WriteOnly)) {
                                lFile.write(lData);
                                lFile.close();
                                lFileIsOk = true;
                            } else {
                                gLogger->ShowError(tr("Image viewer"), tr("Can't create file") + "\r\n" + lFile.fileName() + "\r\n" + lFile.errorString());
                            }
                        } else {
                            gLogger->ShowError(tr("Image editing"), tr("No data found, ID = ") + QString::number(aHistory->Id()) + "\r\n\r\n" + query.lastQuery());
                        }
                    }
                }
            }

            if (lFileIsOk) {

                QFileInfo lFileInfoOrig = QFileInfo(lFile.fileName());
                qint64 lSizeOrig = lFileInfoOrig.size();

                QProcess proc;
                QString lWinFileName = lTempFilename;

                lWinFileName.replace('/', '\\');

                if (!gSettings->Image.EditorType) {
                    proc.start("mspaint \"" + lWinFileName + "\"");
                } else {
                    proc.start(gSettings->Image.EditorPath + " \"" + lWinFileName + "\"");
                }

                if (!proc.waitForStarted(-1)) {
                    QMessageBox::critical(gMainWindow, tr("Image editing"), tr("Can't start program") + "\n" + proc.errorString());
                } else {
                    if (!proc.waitForFinished(-1)) {
                        QMessageBox::critical(gMainWindow, tr("Image editing"), tr("Can't finish program") + "\n" + proc.errorString());
                    } else {
                        if (gSettings->Image.EditorType
                                && gSettings->Image.ConfirmEditorClosed) {
                            QMessageBox::critical(gMainWindow, tr("Image editing"), tr("Editor closed"));
                        }

                        QFileInfo lFileInfo(lFile.fileName());
                        if (lFileInfo.size() != lSizeOrig
                                || lFileInfo.lastModified() != lFileInfoOrig.lastModified()) {
                            // save to database
                            bool lIsOk = false;
                            if (db.transaction()) {
                                qint64 lOrigFileSize; // it is dummy now; it is used as sum for directory
                                XchgFileData *lXchgFileData = new XchgFileData(lFileInfo);
                                if (gFileUtils->InitDataForLoad(true, *lXchgFileData, lOrigFileSize)) {
                                    if (PlotData::LOADFROMFILE(true, aPlot->Id(), lNewIdDwg, aHistory->Id(), aPlot->DwgVersionMax(),
                                                               lXchgFileData->FileInfoOrigConst(), lXchgFileData->FileInfoOrigConst().size(), lXchgFileData->HashOrigConst(),
                                                               *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                                               lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef(), true, NULL)) {

                                        QSqlQuery qUpdate(db);

                                        qUpdate.prepare("update dwg_edit set id_dwgout = :id_dwgout, file_name = :file_name, saved_ftime = :saved_ftime"
                                                        " where id = :id");

                                        if (qUpdate.lastError().isValid()) {
                                            gLogger->ShowSqlError(tr("Image editing") + " - prepare dwg_edit update", qUpdate);
                                        } else {
                                            qUpdate.bindValue(":id_dwgout", lNewIdDwg);
                                            qUpdate.bindValue(":file_name", lXchgFileData->FileInfoOrigConst().filePath());
                                            qUpdate.bindValue(":saved_ftime", lXchgFileData->FileInfoOrigConst().lastModified());
                                            qUpdate.bindValue(":id", lNewIdDwgEdit);

                                            if (qUpdate.exec()) {
                                                lIsOk = true;
                                            } else {
                                                gLogger->ShowSqlError(tr("Image editing") + " - execute dwg_edit update", qUpdate);
                                            }
                                        }
                                    }
                                }

                                delete lXchgFileData;

                                if (lIsOk) {
                                    if (!(lIsOk = db.commit())) {
                                        gLogger->ShowSqlError(tr("Image editing"), tr("Can't commit"), db);
                                    }
                                }
                                if (!lIsOk) {
                                    db.rollback();
                                }
                            } else {
                                gLogger->ShowSqlError(tr("Image editing"), tr("Can't start transaction"), db);
                            }

                            if (!lIsOk) {
                                lRemoveTempDir = false;
                            }
                        } else {
                            // remove unneeded file, no changes
                            QFile::remove(lFile.fileName());
                        }
                    }
                }
            }

            // end editing

            if (!PlotData::ENDEDIT(lNewIdDwgEdit)) {
                lRemoveTempDir = false;
            }
        }
        if (lRemoveTempDir) {
            if (lNewIdDwg
                    && gSettings->LocalCache.UseLocalCache
                    && gFileUtils->HasEnoughSpace(gSettings->LocalCache.Path, gSettings->LocalCache.MinDiskSize)) {
                QFile::rename(lTempFilename, gSettings->LocalCache.Path + "d-" + QString::number(lNewIdDwg));
            }
            lDir.removeRecursively();
        }
    }
}

void ImageViewerThread::SetRequiredIndex(int aRequiredIndex) {
    mRequiredIndex = aRequiredIndex;
}

bool ImageViewerThread::TempDirCreated() const {
    return mTempDirCreated;
}

QString ImageViewerThread::TempDir() const {
    return mTempDir;
}

void ImageViewerThread::SetDeleteDirectory(bool aDeleteDirectory) {
    mDeleteDirectory = aDeleteDirectory;
}

void ImageViewerThread::run() {
    int i, j, k;

    if (mIsExtViewer
            && !mTempDirCreated) {
        exit();
        return;
    }

    if (!mDB.open()) {
        gLogger2->ShowError(tr("Image viewer"), mDB.lastError().text());
    } else {
        bool lSessAltered;
        if (mDB.driverName() == "QPSQL") {
            lSessAltered = true;
        } else {
            QSqlQuery qAlter("alter session set current_schema = " + gSettings->CurrentSchema, mDB);

            if (qAlter.lastError().isValid()) {
                gLogger2->ShowError(tr("Image viewer"), qAlter.lastError().text());
            } else {
                if (!(lSessAltered = qAlter.exec())) {
                    gLogger2->ShowError(tr("Image viewer"), qAlter.lastError().text());
                }
            }
        }
        if (lSessAltered) {
            QSqlQuery query(mDB);

            query.prepare("select a.data from v_dwg a where a.id = :id");
            if (query.lastError().isValid()) {
                gLogger2->ShowError(tr("Image viewer"), query.lastError().text() + "\r\n\r\n" + query.lastQuery());
            } else {

                bool lDoubleNames = false;
                // check for double names (it is possible in PlotListTree when versions opened)
                if (mPlotListTree) {
                    QStringList lFilenames;
                    for (i = 0; i < mImageMIList.length(); i++) {
                        PlotListTreeItem *lItem = mPlotListTree->itemFromIndex(mImageMIList.at(i));
                        if (!lItem->PlotHistoryConst()
                                && !lItem->PlotAddFileConst()) {
                            if (lFilenames.contains(lItem->PlotConst()->BlockNameConst())) {
                                lDoubleNames = true;
                                break;
                            } else {
                                lFilenames.append(lItem->PlotConst()->BlockNameConst());
                            }
                        }
                    }
                }

                bool lHasUndone = true, lWasError = false;
                while (lHasUndone && !lWasError) {
                    for (i = 0; i < mImageMIList.length(); i++) {
                        if (mWaitForRequired) {
                            while (mRequiredIndex == -1) {
                                msleep(100);
                                if (isInterruptionRequested()) break;
                            }
                            mWaitForRequired = false;
                        }

                        if (mRequiredIndex != -1) {
                            i = mRequiredIndex;
                            mRequiredIndex = -1;
                        }

                        if (isInterruptionRequested()) break;

                        if (mImageListDoneList.at(i)
                                && !mImageListClearedList.at(i)) continue;

                        const PlotData *lPlot = NULL;
                        const PlotHistoryData *lHistory = NULL;
                        const PlotAddFileData *lAddFile = NULL;

                        if (mPlotListTree) {
                            PlotListTreeItem *lItem = mPlotListTree->itemFromIndex(mImageMIList.at(i));
                            lHistory = lItem->PlotHistoryConst();
                            lAddFile = lItem->PlotAddFileConst();

                            if (lHistory) {
                                // parent for history
                                lPlot = mPlotListTree->itemFromIndex(mImageMIList.at(i).parent())->PlotConst();
                            } else if (!lAddFile) {
                                // plot by self, history is null
                                lPlot = lItem->PlotConst();
                            }
                        } else if (mHistoryTree) {
                            lHistory = mHistoryTree->itemFromIndex(mImageMIList.at(i))->HistoryConst();
                            lPlot = mHistoryTree->itemFromIndex(mImageMIList.at(i))->PlotConst();
                        } else if (mAddFilesTree) {
                            lAddFile = mAddFilesTree->itemFromIndex(mImageMIList.at(i))->AddFileConst();
                        }

                        QByteArray lData;

                        bool lFileIsOk = false;
                        int lIdDwg = lHistory?lHistory->Id():(lAddFile?lAddFile->IdLob():lPlot->IdDwgMax());
                        QString lFilenameInCache = gSettings->LocalCache.Path + "d-" + QString::number(lIdDwg);
                        QString lTempFilename = mTempDir + "/"
                                + (lAddFile?lAddFile->NameConst():(lHistory?(lPlot->BlockNameConst() + "-h" + QString::number(lHistory->Num()) + "." + lHistory->ExtConst()):
                                                                            (lPlot->BlockNameConst() + (lDoubleNames?("-" + QString::number(lPlot->Id()) + "." + lPlot->ExtensionConst()):""))));

                        // cache here
                        if (gSettings->LocalCache.UseLocalCache) {
                            QFile data(lFilenameInCache);
                            // QFile::copy (both ot them) doesn't work in thread
                            /*if (mIsExtViewer) {
                                                    // not copy to memory; copy from disk to disk later
                                                    lFileIsOk = data.exists();
                                                } else */{
                                // load to memory from cache
                                if (data.open(QFile::ReadOnly)) {
                                    lData = data.readAll();
                                    data.close();
                                    gFileUtils->SetFileTime(lFilenameInCache, QDateTime::currentDateTime());
                                    lFileIsOk = true;
                                }
                            }
                        }

                        if (!lFileIsOk) {
                            // file not exists in cache
                            // read from database
                            query.bindValue(":id", lIdDwg);
                            if (!query.exec()) {
                                gLogger2->ShowError(tr("Image viewer"), query.lastError().text() + "\r\n\r\n" + query.lastQuery());
                                lWasError = true;
                                break;
                            } else {
                                if (query.next()) {
                                    lData = query.value("data").toByteArray();
                                    if (gSettings->LocalCache.UseLocalCache) {
                                        // write file to cache
                                        QFile data(lFilenameInCache);
                                        if (gFileUtils->HasEnoughSpace(gSettings->LocalCache.Path, gSettings->LocalCache.MinDiskSize)) {
                                            if (data.open(QFile::WriteOnly)) {
                                                data.write(lData);
                                                data.close();
                                            }
                                        } else {
                                            data.remove();
                                        }
                                    }
                                } else {
                                    gLogger2->ShowError(tr("Image viewer"), tr("No data found, ID = ") + QString::number(lIdDwg) + "\r\n\r\n" + query.lastQuery());
                                    lWasError = true;
                                    break;
                                }
                            }
                        }

                        if (isInterruptionRequested()) break;

                        if (mIsExtViewer) {
                            /*if (lFileIsOk) {
                                                    QFile data2(lFilenameInCache);
                                                    if (!data2.copy(lTempFilename)) {
                                                        gLogger2->ShowError(tr("Image viewer"), tr("Can't copy file from")
                                                                            + "\r\n" + lFilenameInCache + "\r\n" + tr("to")
                                                                            + "\r\n" + lTempFilename);
                                                        lWasError = true;
                                                        break;
                                                    }
                                                } else */{
                                // write to file
                                QFile lFile(lTempFilename);

                                if (lFile.open(QFile::WriteOnly)) {
                                    lFile.write(lData);
                                    lFile.close();
                                    mFileInfos[i] = QFileInfo(lFile.fileName());
                                    //mFileInfos[i].refresh();
                                    mSizeDummy = mFileInfos.at(i).size();
                                } else {
                                    gLogger2->ShowError(tr("Image viewer"), tr("Can't create file") + "\r\n" + lFile.fileName() + "\r\n" + lFile.errorString());
                                    lWasError = true;
                                    break;
                                }
                            }
                        } else {
                            if (mImageList.length() > i
                                    && mImageList.at(i)) {
                                mImageList.at(i)->loadFromData(lData);
                                *mImageList.at(i) = mImageList.at(i)->scaled(mWidth, mHeight, Qt::KeepAspectRatio);

                                if (mImageList.at(i)->isNull()) {
                                    // second try
                                    j = i - 1;
                                    k = 0;
                                    while (k < 3) { // 3 - count for clean
                                        if (j < 0) j = mImageList.length() + j;
                                        if (j == i) break; // full circle done

                                        // clean only non-empty
                                        if (!mImageList.at(j)->isNull()) {
                                            *mImageList.at(j) = QImage();
                                            mImageListDoneList[j] = false;
                                            k++;
                                        }
                                        j--;
                                    }
                                    mImageList.at(i)->loadFromData(lData);
                                    mWaitForRequired = true;
                                }

                                if (mImageList.at(i)->isNull()) {
                                    // out of memory
                                    mImageListClearedList[i] = true;
                                } else {
                                    mImageListClearedList[i] = false;
                                }
                            }
                        }
                        if (isInterruptionRequested()) break;
                        // it is done anyway - successfully or unsuccessfully
                        mImageListDoneList[i] = true;
                    }

                    lHasUndone = false;
                    if (!isInterruptionRequested()) {
                        for (i = 0; i < mImageListDoneList.length(); i++) {
                            if (!mImageListDoneList[i]) {
                                lHasUndone = true;
                                break;
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
        }
        mDB.close();
    }

    exit();
}

QList<QImage *> &ImageViewerThread::ImageListRef() {
    return mImageList;
}

QFileInfoList &ImageViewerThread::FileInfosRef() {
    return mFileInfos;
}

QList<bool> &ImageViewerThread::ImageListDoneListRef() {
    return mImageListDoneList;
}

QList<bool> &ImageViewerThread::ImageListClearedListRef() {
    return mImageListClearedList;
}

QList<QModelIndex> &ImageViewerThread::ImageMIListRef() {
    return mImageMIList;
}

QList<qulonglong> &ImageViewerThread::IdDwgEditListRef() {
    return mIdDwgEditList;
}
