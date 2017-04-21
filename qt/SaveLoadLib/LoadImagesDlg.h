#ifndef LOADIMAGESDLG_H
#define LOADIMAGESDLG_H

#include "TreeWidgetItemLoad.h"

#include "../VProject/qfcdialog.h"
#include "../VProject/AcadXchgDialog.h"
#include "../VProject/TreeData.h"

#include "saveloadlib_global.h"

#include <QThread>

namespace Ui {
class LoadImagesDlg;
}

class LoadImagesDlgAppEventFilter;
class ProcessImagesThread;

class SAVELOADLIBSHARED_EXPORT LoadImagesDlg : public QFCDialog, public AcadXchgDialog
{
friend ProcessImagesThread;
    Q_OBJECT

public:

    explicit LoadImagesDlg(int aIdProject, QWidget *parent = 0);
    explicit LoadImagesDlg(const QStringList &aFilenames, QWidget *parent = 0);
//    explicit LoadImagesDlg(QSettings &aSettings, QWidget *parent = 0);
    virtual ~LoadImagesDlg();

    virtual QString AddToClassName() const;

    //virtual void SaveState(QSettings &aSettings);

    const QList <QTreeWidgetItem *> &SaveSelectedItems();

    void RescanForChanges();

protected:
    enum enumDlgType { MainDoc = 0, AddFilesForNew };
    enumDlgType mDlgType;
    void InitInConstructor();
    void ShowSettings();
    virtual void showEvent(QShowEvent* event);
    virtual void closeEvent(QCloseEvent* event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

    virtual void StyleSheetChangedInSescendant();
//    virtual void LoadAdditionalSettings(QSettings &aSettings);
//    virtual void SaveAdditionalSettings(QSettings &aSettings);
private slots:
    virtual void reject();
    void FirstInit();
    void OnCommitDataTimer();
    void OnCommitData(QWidget *editor);
    void SettingsChanged();

    void on_cbVersion_editTextChanged(const QString &arg1);

    void on_toolButton_clicked();

    void on_leIdProject_editingFinished();

    void on_tbTreeSel_clicked();

    void on_cbNameTop_editTextChanged(const QString &arg1);

    void on_cbNameBottom_editTextChanged(const QString &arg1);

    void on_leComments_textEdited(const QString &arg1);

    void on_cbCode_currentIndexChanged(int index);

    void on_pbLoad_clicked();

    void on_cbCodeWOE_toggled(bool checked);

    void on_cbNameTopWOE_toggled(bool checked);

    void on_cbNameBottomWOE_toggled(bool checked);

    void on_twMain_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_lblImage_customContextMenuRequested(const QPoint &pos);

    void on_pbConvertSelected_clicked();

    void on_pbConvertAll_clicked();

    void on_leDirectory_customContextMenuRequested(const QPoint &pos);

    void on_twMain_customContextMenuRequested(const QPoint &pos);

    void on_pbZoomToFit_toggled(bool checked);

    void on_pbZoom100_toggled(bool checked);

    void on_rbXY_toggled(bool checked);

    void on_leMaxWidth_editingFinished();

    void on_leMaxHeight_editingFinished();

    void on_sbPercent_valueChanged(int arg1);

    void on_leMaxFileSize_editingFinished();

private:
    int mMode; //
    bool mJustStarted, mIgnoreResize;
    int mIdProject;
    TreeDataRecord *mTreeData;

    LoadImagesDlgAppEventFilter *mLoadImagesDlgAppEventFilter;

    bool mInMouseDragImage;
    int mMouseX, mMouseY;

    int mNewInt;
    QString mNewString;
    int mEditedColumn;
    QList <QTreeWidgetItem *> mEditedItems;

    bool mOutOfMemory;
    QList<ProcessImagesThread *> mProcessThreads;
    int mProcessedCnt;

    Ui::LoadImagesDlg *ui;

    QStringList mDirectories;

    bool SelectFiles();

    void TreeDataChanged();
    void ProcessStringList(const QStringList &aFilenames);
    // 1 - first init, 2 - image changed (converted), 4 - project changed, 8 - treedata, 0x10 - regen code
    void ProcessList(uint aFlags);
    void Convert(bool aAll); // true for all, false for selected
    void ExistingIdChanged(TreeWidgetItemLoad *lItem);
};

class LoadImagesDlgAppEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit LoadImagesDlgAppEventFilter(LoadImagesDlg *aParent = 0);
    void SetInApplicationActivate(bool aInApplicationActivate);
protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
private:
    bool mInApplicationActivate;
    LoadImagesDlg *mLoadImagesDlg;
};

class ProcessImagesThread : public QThread
{
    Q_OBJECT
protected:
    LoadImagesDlg *mLoadImagesDlg;
    QTreeWidget *mMain;
    int mStart, mEnd;
public:
    explicit ProcessImagesThread(int aStart, int aEnd, LoadImagesDlg *aLoadImagesDlg, QTreeWidget *aMain, QObject *parent = 0);

    void run() Q_DECL_OVERRIDE;

signals:
public slots:
};

#endif // LOADIMAGESDLG_H
