#ifndef LOADXREFSDLG_H
#define LOADXREFSDLG_H

#include <QTreeWidgetItem>

#include "qfcdialog.h"
#include "PreloadParamsDlg.h"
#include "AcadXchgDialog.h"
#include "PlotListItemDelegate.h"
#include "TreeData.h"


namespace Ui {
class LoadXrefsDlg;
}

#define lllLXLColStatus 5
#define lllLXLColType   7
#define lllLXLVersion   8
#define lllLXLCode      9
#define lllLXLBlockName 10
#define lllLXLNameTop   11

class LoadXrefsListItem : public QTreeWidgetItem
{
protected:
    bool mIsError;
    int mIdProject;
    QFileInfo mFileInfoOrig;
    int mAcadVersionOrig;
    QString mHashOrig;

    XchgFileData *mXchgFileData;

    int mWhatToDo;

    int mTDArea, mTDId;
public:
    explicit LoadXrefsListItem(int aIdProject, const QString &aOrigFileName);
    virtual ~LoadXrefsListItem();

    void GenerateNames();
    void ShowData();

    bool IsError() const { return mIsError; }

    const QFileInfo &FileInfoOrigConst() const { return mFileInfoOrig; }

    bool Processed() const { return mXchgFileData != NULL; }
    void SetProcessed(XchgFileData *aXchgFileData) {
        if (mXchgFileData) delete mXchgFileData;
        mXchgFileData = aXchgFileData;
    }
    void SetNotProcessed() {
        if (mXchgFileData) delete mXchgFileData;
        mXchgFileData = NULL;
    }

    const XchgFileData *XchgFileDataConst() const { return mXchgFileData; }

    int WhatToDo() const { return mWhatToDo; }
    void SetWhatToDo(int aWhatToDo) { mWhatToDo = aWhatToDo; }

    TreeDataRecord * TreeData() const { return gTreeData->FindById(mTDArea, mTDId); }
    void SetTreeData(int aTDArea, int aTDId) { mTDArea = aTDArea; mTDId = aTDId; }
    //int TDARea() const { return mTDArea; }
};

class LoadXrefsDlg : public QFCDialog, public AcadXchgDialog
{
    Q_OBJECT
protected:
    PreloadParamsDlg *mPreloadParamsDlg;
    int mIdProject;

    bool mAnyDwg, mAnyNonDwg;
public:
    explicit LoadXrefsDlg(int aIdProject, QWidget *parent = 0);
    ~LoadXrefsDlg();

//    XchgFileData *FindFileData(const QString &aOrigFileName);
protected:
    virtual void showEvent(QShowEvent* event);
    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);

private slots:
    void FirstInit();

    void on_twMain_customContextMenuRequested(const QPoint &pos);

    void on_pbProcessSettings_clicked();

    void on_pbProceed_clicked();

    void OnCommitData(QWidget *editor);
private:
    Ui::LoadXrefsDlg *ui;

    bool mJustStarted;

    void RegenCodes();
    bool SelectFiles();

};

class LoadXrefsListItemDelegate : public ROPlotListItemDelegate
{
    Q_OBJECT
protected:
    LoadXrefsDlg *mLoadXrefsDlg;

    void ProcessTreeData(const TreeDataRecord *aTreeData, QComboBox *aComboBox, int aLevel = 0) const;
public:
    explicit LoadXrefsListItemDelegate(QWidget *parent, LoadXrefsDlg *aLoadXrefsDlg);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // LOADXREFSDLG_H
