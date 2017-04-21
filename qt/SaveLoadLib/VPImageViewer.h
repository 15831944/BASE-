#ifndef VPIMAGEVIEWER_H
#define VPIMAGEVIEWER_H

#include "../VProject/qfcdialog.h"

#include "saveloadlib_global.h"

#include <QThread>
#include <QFileInfoList>
#include <QTimer>
#include <QTimer>
#include <QtSql/QSqlDatabase>

class PlotData;
class PlotHistoryData;

class PlotListTree;
class PlotHistoryTree;
class PlotAddFilesTree;

namespace Ui {
class VPImageViewer;
}

class ImageViewerThread;

class SAVELOADLIBSHARED_EXPORT VPImageViewer : public QFCDialog
{
friend ImageViewerThread;
    Q_OBJECT

public:
    explicit VPImageViewer(PlotListTree *aPlotListTree, bool aTrueForEdit, QWidget *parent = 0);
    explicit VPImageViewer(PlotHistoryTree *aHistoryTree, QWidget *parent = 0);
    explicit VPImageViewer(PlotAddFilesTree *aAddFilesTree, QWidget *parent = 0);
    virtual ~VPImageViewer();

    void ShowImage();
protected:
    void ThreadStart();
    virtual void showEvent(QShowEvent* event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void ShowLoadedCnt();
    void on_pbZoomToFit_toggled(bool checked);
    void on_pbZoom100_toggled(bool checked);

private:
    bool mJustStarted;
    bool mTrueForEdit;
    PlotListTree *mPlotListTree;
    PlotHistoryTree *mHistoryTree;
    PlotAddFilesTree *mAddFilesTree;

    QTimer *mTimer;

    bool mInMouseDragImage;
    int mMouseX, mMouseY;

    ImageViewerThread *mImageViewerThread;

    Ui::VPImageViewer *ui;
};

class SAVELOADLIBSHARED_EXPORT ImageViewerThread : public QThread
{
    Q_OBJECT
protected:
    QSqlDatabase mDB;

    PlotListTree *mPlotListTree;
    PlotHistoryTree *mHistoryTree;
    PlotAddFilesTree *mAddFilesTree;

    bool mIsExtViewer, mTrueForEdit, mTrueForEditAll;
    // for internal view
    int mWidth, mHeight;
    int mRequiredIndex;
    bool mWaitForRequired;
    //VPImageViewer *mVPImageViewer;
    QString mTempDir;
    bool mTempDirCreated;

    QList<QImage *> mImageList;
    QFileInfoList mFileInfos;
    qint64 mSizeDummy;
    QList<bool> mImageListDoneList; // image processed in thread
    QList<bool> mImageListClearedList; // image cleared in thread - out of memory
    QList<QModelIndex> mImageMIList;
    QList<qulonglong> mIdDwgEditList; // dwg_edit id if record successfully created

    bool mDeleteDirectory;

    void InitInConstructor();

public:
    enum enumFromMainWindow {FromMainWindow};
    enum enumFromHistWindow {FromHistWindow};
    enum enumFromAddFilesWindow {FromAddFilesWindow};

    explicit ImageViewerThread(enumFromMainWindow aDummy, PlotListTree *aPlotListTree, QObject *parent, bool aIsExtViewer,
                               bool aTrueForEdit, bool aTrueForEditAll,
                               int aWidth, int aHeight);
    explicit ImageViewerThread(enumFromHistWindow aDummy, PlotHistoryTree *aHistoryTree, QObject *parent, bool aIsExtViewer,
                               int aWidth, int aHeight);
    explicit ImageViewerThread(enumFromAddFilesWindow aDummy, PlotAddFilesTree *aAddFilesTree, QObject *parent, bool aIsExtViewer,
                               int aWidth, int aHeight);
    virtual ~ImageViewerThread();

    static void ModalViewList(PlotListTree *aPlotListTree);
    static void ModalEdit(PlotListTree *aPlotListTree);

    static void ModalViewList(PlotHistoryTree *aHistoryTree);
    static void ModalEdit(PlotData *aPlot, PlotHistoryData *aHistory);

    void SetRequiredIndex(int aRequiredIndex);
    bool TempDirCreated() const;
    QString TempDir() const;
    void SetDeleteDirectory(bool aDeleteDirectory);

    void run() Q_DECL_OVERRIDE;

    QList<QImage *> &ImageListRef();
    QFileInfoList &FileInfosRef();
    QList<bool> &ImageListDoneListRef();
    QList<bool> &ImageListClearedListRef();
    QList<QModelIndex> &ImageMIListRef();
    QList<qulonglong> &IdDwgEditListRef();

signals:
public slots:
};

#endif // VPIMAGEVIEWER_H
