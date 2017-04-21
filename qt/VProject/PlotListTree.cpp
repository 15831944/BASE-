#include "PlotListTree.h"
#include "common.h"
#include "GlobalSettings.h"
#include "BlobMemCache.h"

#include "../PlotLib/DwgLayoutData.h"
#include "../PlotLib/PlotSimpleListDlg.h"
#include "../PlotLib/PlotRightsDlg.h"
#include "../PlotLib/DwgData.h"

#include "MainWindow.h"
#include "PlotListDlg.h"
#include "AuditPurgeDlg.h"
#include "PublishDlg.h"
#include "ReplaceTextDlg.h"
#include "PlotListTreeChange.h"
#include "SentParamsDgl.h"

#include "SaveDialog.h"

#include "oracle.h"
#include "FileUtils.h"
#include "MSOffice.h"

#include "PublishReport.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"

#include "../SaveLoadLib/VPImageViewer.h"

#include <QGridLayout>
#include <QPlainTextEdit>
#include <QHeaderView>
#include <QMenu>
#include <QFileDialog>
#include <QScrollBar>
#include <QInputDialog>
#include <QProcess>
#include <QAbstractEventDispatcher>

void DoListForMetro(QWidget *aParent, const QList<const PlotData *> &aPlotList);

PlotListTree::PlotListTree(QWidget *parent) :
    QTreeWidget(parent), AcadXchgDialog(),
    mVersionSaved(0), mVersionCurrent(1),
    mIgnoreSectionResize(false), mInSaveTimer(0),
    mParentDlg(NULL), mDialogButtonBox(NULL),
    mPLTFlags(0), mSLT(GlobalSettings::DocumentTreeStruct::SLTNone/*gSettings->DocumentTree.SecondLevelType*/),
    mCurrentItemType(0), mCurrentItemId(0), mCurrentColumn(0),
    mEditType(-1), mEditColumn(-1),
    mProject(NULL),
    mHideCancelled(false), mNamedList(NULL),
    mVPImageViewer(NULL)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));

    // always selected in normal color (not gray)
    QPalette lPalette = palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    setPalette(lPalette);
    // ---------------------------------

    PlotListTreeItemDelegate *DocItemDelegate = new PlotListTreeItemDelegate(this);
    setItemDelegate(DocItemDelegate);
    connect(DocItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(StringChanged(QWidget *)));
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(OnItemDoubleClicked(QTreeWidgetItem *, int)));

    connect(gProjects, SIGNAL(PlotBeforeUpdate(PlotData *, int)), this, SLOT(OnPlotBeforeUpdate(PlotData *, int)));
    connect(gProjects, SIGNAL(PlotNeedUpdate(PlotData *, int)), this, SLOT(OnPlotNeedUpdate(PlotData *, int)));

    QObject *lParent = parent;
    while (!qobject_cast<QFCDialog *>(lParent) && lParent->parent()) lParent = lParent->parent();
    if (mParentDlg = qobject_cast<QFCDialog *>(lParent)) {
        connect(header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(OnSectionResized(int, int, int)));
        connect(header(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(OnSectionMoved(int, int, int)));
        connect(header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(OnSortIndicatorChanged(int, Qt::SortOrder)));

        header()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnSelectColumns(const QPoint &)));
    }
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(OnCurrentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
}

PlotListTree::~PlotListTree() {
    gOracle->Clean();
    gBlobMemCache->Clean();
}

void PlotListTree::InitColumns(qint32 aPLTFlags) {
//    PlotListDlg *lParentDialog;
//    QObject *lParent = parent();
//    while (!qobject_cast<PlotListDlg *>(lParent) && lParent->parent()) lParent = lParent->parent();
//    lParentDialog = qobject_cast<PlotListDlg *>(lParent);
//    QMessageBox::critical(NULL, lParentDialog?(lParentDialog->objectName()):"NULL", "PlotListTree::InitColumns: " + QString::number(aPLTFlags));

    bool lPrevBlockState = header()->blockSignals(true);

    QByteArray lBA;

    if (mParentDlg) {
        QSettings settings;
        settings.beginGroup("Windows");
        settings.beginGroup(mParentDlg->metaObject()->className() + mParentDlg->AddToClassName());
        settings.beginGroup("DocTree");


        if ((mVersionSaved = settings.value("Version").toInt()) == mVersionCurrent) {
            mVerExtVisible = settings.value("VerExtVisSLT" + QString::number(mSLT)).toBool();
            mVerDateVisible = settings.value("VerDateVisSLT" + QString::number(mSLT)).toBool();
            mSentDateVisible = settings.value("SentDateVisSLT" + QString::number(mSLT)).toBool();
            mSentUserVisible = settings.value("SentUserVisSLT" + QString::number(mSLT)).toBool();
            mComplectVisible = settings.value("ComplectVisSLT" + QString::number(mSLT)).toBool();
            mBlockNameVisible = settings.value("BlockNameVisSLT" + QString::number(mSLT)).toBool();

            lBA = settings.value("SLT" + QString::number(mSLT)).toByteArray();
        }

        //settings.endGroup();
        //settings.endGroup();
        //settings.endGroup();
    }
    //QMessageBox::critical(NULL, "", QString::number(lBA.length()));

    QAbstractItemModel *lModel = header()->model();
    int lColCount = 22, lCol = 0, lColIndex = 0;

    std::fill_n(mCols, colLAST - 1, -1);

    mPLTFlags = aPLTFlags;

    if (aPLTFlags & PLTForXrefs) {
        lColCount += 2;
    }
    if (aPLTFlags & (PLTWorking | PLTVersions)) lColCount++;
    if (aPLTFlags & PLTProject) lColCount += 2;
    if (aPLTFlags & PLTEditStatus) lColCount++;
    if (aPLTFlags & PLTDeleted) {
        lColCount += 2;
    }
    if (aPLTFlags & PLTNewVersion) lColCount += 2;

    if (qobject_cast<PlotListDlg *>(mParentDlg)
            && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts) lColCount += 4;

    if (qobject_cast<PlotListDlg *>(mParentDlg)
            && mSLT == GlobalSettings::DocumentTreeStruct::SLTHistory) lColCount += 3;

    setColumnCount(lColCount);

    if (aPLTFlags & PLTForXrefs) {
        lModel->setHeaderData(lCol, Qt::Horizontal, "Block name");
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 100);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        lCol++;

        lModel->setHeaderData(lCol, Qt::Horizontal, tr("U"));
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Use working"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 30);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex + 1] = lCol;
        lCol++;
    }
    lColIndex += 2;

    lModel->setHeaderData(lCol, Qt::Horizontal, "ID");
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 80);
        setColumnHidden(lCol, false);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    if (aPLTFlags & (PLTWorking | PLTVersions)) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("W"));
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Working"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 30);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        lCol++;
    }
    lColIndex++;

    if (aPLTFlags & PLTForXrefs) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Hist."));
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 40);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        lCol++;
    }
    lColIndex++;

    if (aPLTFlags & PLTProject) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("ID pr."));
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("ID of project"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 40);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        lCol++;

        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Project"));
        lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 120);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex + 1] = lCol;
        lCol++;
    }
    lColIndex += 2;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("ID common"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 40);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    if (qobject_cast<PlotListDlg *>(mParentDlg)
            && mSLT == GlobalSettings::DocumentTreeStruct::SLTHistory) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("ID hist."));
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 60);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        lCol++;

        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Hist."));
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 40);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex + 1] = lCol;
        lCol++;

        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Origin"));
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 40);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex + 2] = lCol;
        lCol++;
    }
    lColIndex += 3;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Ver. int."));
    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Version for internal use"), Qt::ToolTipRole);
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 60);
        setColumnHidden(lCol, false);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Ver. ext."));
    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Version for customer"), Qt::ToolTipRole);
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        mVerExtVisible = qobject_cast<PlotListDlg *>(mParentDlg)
                           && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts;
        setColumnWidth(lCol, 60);
        setColumnHidden(lCol, !mVerExtVisible);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    if (aPLTFlags & PLTNewVersion) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("New ver. int."));
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("New version for internal use"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 90);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        lCol++;

        lModel->setHeaderData(lCol, Qt::Horizontal, tr("New ver. ext."));
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("New version for customer"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 90);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex + 1] = lCol;
        lCol++;
    }
    lColIndex += 2;

    if (qobject_cast<PlotListDlg *>(mParentDlg)
            && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Ver. date"));
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Version date for customer"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 80);
            setColumnHidden(lCol, false);
            mVerDateVisible = true;
        }
        mCols[lColIndex] = lCol;
        lCol++;
    } else {
        mVerDateVisible = false;
    }
    lColIndex++;

    if (aPLTFlags & PLTDeleted) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Deleted"));
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 80);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        lCol++;

        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Deleted by"));
        lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 100);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex + 1] = lCol;
        lCol++;
    }
    lColIndex += 2;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Cancelled"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 80);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Cancelled by"));
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 100);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Sent"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 80);
        setColumnHidden(lCol, false);
        mSentDateVisible = true;
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Sent by"));
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 80);
        setColumnHidden(lCol, true);
        mSentUserVisible = false;
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Complect"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 40);
        setColumnHidden(lCol, false);
        mComplectVisible = true;
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    if (qobject_cast<PlotListDlg *>(mParentDlg)
            && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Layout name"));
        //lModel->setHeaderData(lCol, Qt::Horizontal, tr("Version date for customer"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 80);
        }
        setColumnHidden(lCol, false);
        mCols[lColIndex] = lCol;
        lCol++;
    }
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Code"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 100);
        setColumnHidden(lCol, false);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Sheet"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 40);
        setColumnHidden(lCol, false);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    if (qobject_cast<PlotListDlg *>(mParentDlg)
            && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Stage"));
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 100);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex] = lCol;
        //header()->moveSection(header()->visualIndex(lCol), header()->visualIndex(mCols[colSheet] + 1));
        lCol++;

        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Purpose"));
        lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 100);
            setColumnHidden(lCol, false);
        }
        mCols[lColIndex + 1] = lCol;
        //header()->moveSection(header()->visualIndex(lCol), header()->visualIndex(mCols[colSheet] + 1));
        lCol++;
    }
    lColIndex += 2;

    if (!(aPLTFlags & PLTForXrefs)) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Block name"));
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("For AutoCAD drawings - block name when using the drawing as xref\nNo meaning for other documents"), Qt::ToolTipRole);
        lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 100);
            setColumnHidden(lCol, true);
            mBlockNameVisible = false;
        }
        mCols[lColIndex] = lCol;
        lCol++;
    }
    lColIndex++;

//    if (qobject_cast<PlotListDlg *>(mParentDlg)
//            && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts) {
//        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Name top / DWG filename"));
//    } else {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Name top"));
//    }
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 180);
        setColumnHidden(lCol, false);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

//    if (qobject_cast<PlotListDlg *>(mParentDlg)
//            && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts) {
//        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Name bottom / PLT filename"));
//    } else {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Name bottom"));
//    }
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 180);
        setColumnHidden(lCol, false);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Created"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 80);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Created by"));
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 100);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Edited"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 80);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Edited by"));
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 100);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    if (aPLTFlags & PLTEditStatus) {
        lModel->setHeaderData(lCol, Qt::Horizontal, tr("Status"));
        lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        if (lBA.isEmpty()) {
            setColumnWidth(lCol, 80);
            setColumnHidden(lCol, true);
        }
        mCols[lColIndex] = lCol;
        lCol++;
    }
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Ext."));
    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Extension"), Qt::ToolTipRole);
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignHCenter | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 40);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Size"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 80);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Xrefs"));
    lModel->setHeaderData(lCol, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnWidth(lCol, 25);
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    lModel->setHeaderData(lCol, Qt::Horizontal, tr("Notes"));
    lModel->setHeaderData(lCol, Qt::Horizontal, QVariant(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    if (lBA.isEmpty()) {
        setColumnHidden(lCol, true);
    }
    mCols[lColIndex] = lCol;
    lCol++;
    lColIndex++;

    if (lCol != lColCount) {
        gLogger->ShowError(this, "Invalid columns count", QString::number(lCol) + ":" + QString::number(lColCount));
    }

//    if (mSLT == GlobalSettings::DocumentTreeStruct::SLTHistory) {
//        setColumnHidden(mCols[colIdHist], false);
//        setColumnHidden(mCols[colHist], false);
//        setColumnHidden(mCols[colHistOrigin], false);
//    } else {
//        setColumnHidden(mCols[colIdHist], true);
//        setColumnHidden(mCols[colHist], true);
//        setColumnHidden(mCols[colHistOrigin], true);
//    }

    if (!lBA.isEmpty()) {
        header()->restoreState(lBA);
        setColumnHidden(mCols[colVersionExt], !mVerExtVisible);

        setColumnHidden(mCols[colSentDate], !mSentDateVisible);
        setColumnHidden(mCols[colSentBy], !mSentUserVisible);
        setColumnHidden(mCols[colSection], !mComplectVisible);
        setColumnHidden(mCols[colBlockName], !mBlockNameVisible);
    }

    header()->blockSignals(lPrevBlockState);
}

void PlotListTree::OnDeleteVPImageViewer(int) {
    mVPImageViewer = NULL;
}

void PlotListTree::OnCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (mVPImageViewer
            && current) mVPImageViewer->ShowImage();
}

void PlotListTree::ShowContextMenu(const QPoint & pos) {
    MyMutexLocker lLocker(gMainWindow->UpdateMutex(), 0);
    if (!gSettings->IsDialogModal(parentWidget()) && !lLocker.IsLocked()) return; // something wrong

    int i;
    QMenu lMenu(this);
    PlotListTreeItem * lSingleItem = NULL;
    QList <QTreeWidgetItem *> selected;
    QAction *lARes;
    QAction *lAExpandAll = NULL, *lACollapseAll = NULL,
            *lAProjProps = NULL, *lAProjDocs = NULL,
            *lASelectSingle = NULL, *lAFindGoTo = NULL, *lAUndelete = NULL,
            *lAEdit = NULL,
            *lAView = NULL, *lAViewNoXrefs = NULL, *lAProps = NULL, *lARights = NULL, *lARightsNew = NULL,
            *lANew = NULL, *lAMakeActive = NULL, *lANewVersion = NULL,
            *lAVersions = NULL, *lAHistory = NULL, *lAAddFiles = NULL, *lAAttr = NULL,
            *lASave = NULL, *lASaveAddFiles = NULL, *lALoad = NULL, *lALoadImages = NULL, *lALoadXrefs = NULL,
            *lAGoto = NULL;

    QMenu *lSubMenu;
    QAction *lAAudit = NULL, *lAPublish = NULL, *lAReplaceText = NULL, *lARecover = NULL;
    QAction *lAXrefs = NULL, *lAXrefsAllOLD = NULL, *lAXrefFor = NULL;

    QAction *lAFieldEdit = NULL, *lAFieldReplace = NULL;

    QList <QAction *> lAAddToList;
    QAction *lANewListInProject = NULL;
    QAction *lARemoveFromList = NULL;

    QAction *lAMarkCancelled = NULL, *lAMarkSent = NULL;
    QAction *lAExcel = NULL, *lAMaazXls = NULL, *lAMetroListXls = NULL, *lACopyToClipboard = NULL;
    QAction *lAMarkDeleted = NULL, *lAMarkDeletedWithAllVersions = NULL;


    QList <QTreeWidgetItem *> lForMakeActive;
    QList <PlotAndHistoryData> lForNewVersions;


    if (mSLT != GlobalSettings::DocumentTreeStruct::SLTNone
            && pos.x() < indentation()) {
        lAExpandAll = lMenu.addAction(tr("Expand all"));
        lACollapseAll = lMenu.addAction(tr("Collapse all"));
    } else {
        selected = selectedItems();
        if (gSettings->DocumentTree.SingleSelect == gSettings->DocumentTreeStruct::SSCurrent) {
            lSingleItem = static_cast<PlotListTreeItem *>(currentItem());
        } else if (selected.count() == 1) {
            lSingleItem = static_cast<PlotListTreeItem *>(selected.at(0));
        }

        if ((mPLTFlags & PLTProject)
                && (currentColumn() == mCols[colIdProject] || currentColumn() == mCols[colProject])
                && lSingleItem) {
            lAProjProps = lMenu.addAction(tr("Project properties"));
            lAProjDocs = lMenu.addAction(tr("Project documents"));
            lMenu.addSeparator();
        }

        bool lAnyPlot = false, lAnyHistory = false, lAnyXrefs = false,
                lAnyLayout = false, lAnyUnsentPlot = false, lAnyDWG = false,
                lAnyDeleted = false;
        bool lDefActionSetted = false;

        for (i = 0; i < selected.length(); i++) {
            PlotData * lPlot;
            if (lPlot = static_cast<PlotListTreeItem *>(selected.at(i))->PlotRef()) {
                lAnyPlot = true;
                if (/*!lPlot->IsSent()*/ lPlot->SentDateConst().isNull()) lAnyUnsentPlot = true;
                if (lPlot->Deleted()) lAnyDeleted = true;
                if ((lPlot->FileType() < 20 || lPlot->FileType() > 29) && lPlot->ExtensionConst().toLower() == "dwg") {
                    lAnyDWG = true;
                    if (lPlot->XrefsCnt()) lAnyXrefs = true;
                }
            }

            if (static_cast<PlotListTreeItem *>(selected.at(i))->PlotHistoryConst()) {
                lAnyHistory = true;
                lPlot = static_cast<PlotListTreeItem *>(selected.at(i)->parent())->PlotRef();
                if ((lPlot->FileType() < 20 || lPlot->FileType() > 29) && lPlot->ExtensionConst().toLower() == "dwg") {
                    //lAnyDWG = true; it is used below only for last record in history, yaya
                    if (static_cast<PlotListTreeItem *>(selected.at(i))->PlotHistoryConst()->XrefsCnt()) lAnyXrefs = true;
                }
            }

            if (static_cast<PlotListTreeItem *>(selected.at(i))->DwgLayoutConst()) lAnyLayout = true;
            if (lAnyPlot && lAnyHistory && lAnyXrefs && lAnyLayout && lAnyUnsentPlot && lAnyDWG) break;
        }

        if ((mPLTFlags & PLTSingleSelMode)
                && mDialogButtonBox
                && lSingleItem
                && lSingleItem->PlotConst()) {
            lASelectSingle = lMenu.addAction(tr("Select"));
            lMenu.setDefaultAction(lASelectSingle);
            lDefActionSetted = true;
            lMenu.addSeparator();
        }

        if ((mPLTFlags & PLTFindMode)
                && lSingleItem
                && lSingleItem->PlotConst()) {
            lAFindGoTo = lMenu.addAction(tr("Go to document"));
            lMenu.setDefaultAction(lAFindGoTo);
            lDefActionSetted = true;
            lMenu.addSeparator();
        }

        if (mPLTFlags & PLTDeleted
                && lAnyDeleted) {
            lAUndelete = lMenu.addAction(/*QIcon(":/some/ico/ico/minus.png"), */tr("Undelete"));
            lMenu.addSeparator();
        }

        if (gSettings->DocumentTree.OpenSingleDocument) {
            if (currentItem()) {
                PlotListTreeItem * lItem = static_cast<PlotListTreeItem *>(currentItem());
                if (lItem->PlotConst() || lItem->PlotHistoryConst()) {
                    // Edit --------------------------------------------------
                    if (!(mPLTFlags & PLTNoEditMenu)) {
                        lAEdit = lMenu.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Edit"));
                    }

                    // View --------------------------------------------------
                    lAView = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View"));
                    if (!lDefActionSetted
                            && (gSettings->DocumentTree.OnDocDblClick == GlobalSettings::DocumentTreeStruct::DDView/*
                                                                   || gSettings->DocumentTree.OnDocDblClick == GlobalSettings::DocumentTreeStruct::DDEditInGridView*/)) {
                        lMenu.setDefaultAction(lAView);
                        lDefActionSetted = true;
                    }
                }
                if (lItem->PlotConst() && lItem->PlotConst()->XrefsCnt()
                        || lItem->PlotHistoryConst() && lItem->PlotHistoryConst()->XrefsCnt()) {
                    lAViewNoXrefs = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View w/o xrefs"));
                }
            }
        } else {
            if (lAnyPlot || lAnyHistory) {
                // Edit --------------------------------------------------
                if (!(mPLTFlags & PLTNoEditMenu)) {
                    lAEdit = lMenu.addAction(QIcon(":/some/ico/ico/DocEdit.png"), tr("Edit"));
                }

                // View --------------------------------------------------
                lAView = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View"));
                if (!lDefActionSetted
                        && (gSettings->DocumentTree.OnDocDblClick == GlobalSettings::DocumentTreeStruct::DDView/*
                                                               || gSettings->DocumentTree.OnDocDblClick == GlobalSettings::DocumentTreeStruct::DDEditInGridView*/)) {
                    lMenu.setDefaultAction(lAView);
                    lDefActionSetted = true;
                }
            }
            if (lAnyXrefs) {
                lAViewNoXrefs = lMenu.addAction(QIcon(":/some/ico/ico/view.png"), tr("View w/o xrefs"));
            }
        }

        if (lSingleItem && (lSingleItem->PlotConst() || lSingleItem->PlotHistoryConst())) {
            // Properties... --------------------------------------------------
            lAProps = lMenu.addAction(QIcon(":/some/ico/ico/DocProps.png"), tr("Properties..."));
            if (!lDefActionSetted
                    && (gSettings->DocumentTree.OnDocDblClick == GlobalSettings::DocumentTreeStruct::DDProps/*
                                                    || gSettings->DocumentTree.OnDocDblClick == GlobalSettings::DocumentTreeStruct::DDEditInGridProps*/)) {
                lMenu.setDefaultAction(lAProps);
                lDefActionSetted = true;
            }
        }

        // Rights... --------------------------------------------------
        if (!(mPLTFlags & PLTNoEditMenu) && lSingleItem && lSingleItem->PlotConst()) {
            //lARights = lMenu.addAction(QIcon(":/some/ico/ico/security.png"), tr("Rights..."));
            lARightsNew = lMenu.addAction(QIcon(":/some/ico/ico/security.png"), tr("Rights..."));
        }

        bool lVerSeparator = false; // flag that separator already inserted before versions
        if (!(mPLTFlags & PLTNoEditMenu)) {
            // New document... --------------------------------------------------
            if (gUserRight->CanInsertAnyColumn("v_plot_simple"))
                lANew = lMenu.addAction(QIcon(":/some/ico/ico/DocNew.png"), tr("New document..."));

            if (!selected.isEmpty() && (lAnyDWG || lAnyLayout)
                    || lSingleItem && lSingleItem->PlotConst()
                    && (lSingleItem->PlotConst()->FileType() < 20 || lSingleItem->PlotConst()->FileType() > 29)
                    && lSingleItem->PlotConst()->ExtensionConst().toLower() == "dwg") {
                // Utilities submenu --------------------------------------------------
                lSubMenu = lMenu.addMenu(tr("Utilities"));
                if (!selected.isEmpty() && lAnyDWG) {
                    lAAudit = lSubMenu->addAction(QIcon(":/some/ico/ico/purge.png"), tr("Audit && purge..."));
                }
                if (!selected.isEmpty() && (lAnyPlot || lAnyLayout)) {
                    lAPublish = lSubMenu->addAction(QIcon(":/some/ico/ico/publish.png"), tr("Publish..."));
                }
                if (!selected.isEmpty() && lAnyDWG) {
                    lAReplaceText = lSubMenu->addAction(QIcon(":/some/ico/ico/find-text.png"), tr("Replace text..."));
                }
                if (lSingleItem
                        && lSingleItem->PlotConst()
                        && (lSingleItem->PlotConst()->FileType() < 20 || lSingleItem->PlotConst()->FileType() > 29)
                        && lSingleItem->PlotConst()->ExtensionConst().toLower() == "dwg") {
                    if (!selected.isEmpty()) lSubMenu->addSeparator();
                    lARecover = lSubMenu->addAction(QIcon(":/some/ico/ico/recover.png"), tr("Recover"));
                }
            }

            // Make active --------------------------------------------------
            if (mSLT == GlobalSettings::DocumentTreeStruct::SLTVersions) {
                if (gUserRight->CanUpdate("v_plot_simple", "working")) {
                    QList <QTreeWidgetItem *> parents;
                    for (i = 0; i < selected.length(); i++) {
                        if (selected.at(i)->parent()) {
                            if (parents.contains(selected.at(i)->parent())) {
                                lForMakeActive.clear();
                                break;
                            } else {
                                parents.append(selected.at(i)->parent());
                                lForMakeActive.append(selected.at(i));
                            }
                        }
                    }
                    if (!lForMakeActive.isEmpty()) {
                        if (!lVerSeparator) {
                            lMenu.addSeparator();
                            lVerSeparator = true;
                        }
                        lAMakeActive = lMenu.addAction(tr("Make active"));
                    }
                }
            }
        } // endof if (!(mPLTFlags & PLTNoEditMenu))

        // New version... --------------------------------------------------
        if (!(mPLTFlags & PLTNoEditMenu)
                || (mPLTFlags & PLTVersions)) {
            if (gUserRight->CanInsertAnyColumn("v_plot_simple")) {
                QList <int> lIdCommon;
                for (i = 0; i < selected.length(); i++) {
                    PlotData * lPlotData = NULL;
                    PlotHistoryData * lPlotHistoryData = NULL;
                    if (!(lPlotData = static_cast<PlotListTreeItem *>(selected.at(i))->PlotRef())) {
                        if ((lPlotHistoryData = static_cast<PlotListTreeItem *>(selected.at(i))->PlotHistoryRef())
                                && selected.at(i)->parent()) {
                            // get parent of history
                            lPlotData = static_cast<PlotListTreeItem *>(selected.at(i)->parent())->PlotRef();
                        }
                    }
                    if (lPlotData) {
                        if (lIdCommon.contains(lPlotData->IdCommon())) {
                            lForNewVersions.clear();
                            break;
                        } else {
                            lIdCommon.append(lPlotData->IdCommon());
                            // NB: lPlotHistoryData can be null here
                            lForNewVersions.append(qMakePair(lPlotData, lPlotHistoryData));
                        }
                    }
                }

                if (!lForNewVersions.isEmpty()) {
                    if (!lVerSeparator) {
                        lMenu.addSeparator();
                        lVerSeparator = true;
                    }

                    if (lForNewVersions.length() > 1) {
                        lANewVersion = lMenu.addAction(tr("New versions..."));
                    } else {
                        lANewVersion = lMenu.addAction(tr("New version..."));
                    }
                }
            }
        }

        // Add. files and History --------------------------------------------------
        if (lSingleItem && lSingleItem->PlotConst()) {
            lMenu.addSeparator();
            if (!(mPLTFlags & (PLTVersions | PLTDeleted))) {
                lAVersions = lMenu.addAction(/*QIcon(":/some/ico/ico/File-History.png"),*/ tr("Versions"));
            }
            lAHistory = lMenu.addAction(QIcon(":/some/ico/ico/File-History.png"), tr("History"));
            lAAddFiles = lMenu.addAction(QIcon(":/some/ico/ico/document-list.png"), tr("Add. files"));
        }

        if (lAnyPlot || lAnyHistory || lAnyLayout) {
            lAAttr = lMenu.addAction(/*QIcon(":/some/ico/ico/document-list.png"),*/ tr("Layouts"));
        }

        // xrefs submenu
        if (lSingleItem
                && lSingleItem->PlotConst()
                && (lSingleItem->PlotConst()->FileType() < 20 || lSingleItem->PlotConst()->FileType() > 29)
                && lSingleItem->PlotConst()->ExtensionConst().toLower() == "dwg") {
            lSubMenu = lMenu.addMenu(QIcon(":/some/ico/ico/xrefs.png"), tr("Xrefs"));
            lAXrefs = lSubMenu->addAction(tr("Xrefs..."));
            lAXrefsAllOLD = lSubMenu->addAction(tr("Xrefs (old, temp.)..."));
            lAXrefFor = lSubMenu->addAction(tr("Xref for..."));
        }

        if (lAnyPlot || lAnyHistory) {
            // Save... --------------------------------------------------
            lMenu.addSeparator();
            lASave = lMenu.addAction(QIcon(":/some/ico/ico/SaveFromDatabase.png"), tr("Save..."));
        }

        if (!(mPLTFlags & PLTNewVersion)
                && selected.count()) {
            // Save add. files... --------------------------------------------------
            bool lCanSaveAddFiles = false;
            int lAddFilesPlotId = 0;
            QList<int> lESGetted;
            for (i = 0; i < selected.length(); i++) {
                PlotListTreeItem * lItem = static_cast<PlotListTreeItem *>(selected.at(i));

                if (lItem->PlotAddFileConst()) {
                    PlotData * lPlot = lItem->PlotRef();
                    if (!lPlot
                            && lItem->parent()) lPlot = static_cast<PlotListTreeItem *>(lItem->parent())->PlotRef();

                    if (!lPlot) continue;

                    if (!lESGetted.contains(lPlot->Id())) {
                        lPlot->InitEditStatus();
                        lESGetted.append(lPlot->Id());
                    }

                    if (lPlot->ES() == PlotData::PESEditing) continue;

                    if (lItem->PlotAddFileConst()->IdLob() < 0
                            || gOracle->IsDwgRecordNOTEditingNow(lItem->PlotAddFileConst()->IdLob())) {

                        lCanSaveAddFiles = true;
                        if (!lAddFilesPlotId) {
                            lAddFilesPlotId = lPlot->Id();
                        } else {
                            if (lAddFilesPlotId != lPlot->Id()) {
                                lCanSaveAddFiles = false;
                                break;
                            }
                        }
                    }
                }
            }

            if (lCanSaveAddFiles) {
                if (selected.count() == 1)
                    lASaveAddFiles = lMenu.addAction(QIcon(":/some/ico/ico/SaveFromDatabase.png"), tr("Save add. file..."));
                else
                    lASaveAddFiles = lMenu.addAction(QIcon(":/some/ico/ico/SaveFromDatabase.png"), tr("Save add. files..."));
            }
        }

        // Load... --------------------------------------------------
        if (!(mPLTFlags & PLTNoEditMenu)
                && !(mPLTFlags & PLTNewVersion)
                && lSingleItem
                && lSingleItem->PlotConst()
                && lSingleItem->PlotConst()->SentDateConst().isNull()) {
            lALoad = lMenu.addAction(QIcon(":/some/ico/ico/LoadToDatabase.png"), tr("Load..."));
        }
        if (!(mPLTFlags & PLTNoEditMenu)
                && !(mPLTFlags & PLTNewVersion)) {
            lALoadImages = lMenu.addAction(QIcon(":/some/ico/ico/LoadToDatabase.png"), tr("Load images"));
        }


        //        if (!(mPLTFlags & PLTNoEditMenu)
        //                && !(mPLTFlags & PLTNewVersion)) {
        //            lSubMenu = lMenu.addMenu(QIcon(":/some/ico/ico/LoadToDatabase.png"), tr("Load"));
        //            lALoadXrefs = lSubMenu->addAction(QIcon(":/some/ico/ico/xrefs.png"), tr("Xrefs..."));
        //        }

        // Go to document
        if (lSingleItem && lSingleItem->PlotHistoryConst()) {
            const PlotHistoryData * lHistory = lSingleItem->PlotHistoryConst();
            if (lHistory->Type() == 0
                    && lHistory->FromIdPlot()
                    && lHistory->IdPlot() != lHistory->FromIdPlot()) {
                lMenu.addSeparator();
                lAGoto = lMenu.addAction(tr("Go to document"));
            }
        }

        if (lAnyUnsentPlot
                && !(mPLTFlags & PLTNoEditMenu)
                && !(mPLTFlags & PLTNewVersion)
                && (currentColumn() == mCols[PlotListTree::colCode]
                    || currentColumn() == mCols[PlotListTree::colSheet]
                    || currentColumn() == mCols[PlotListTree::colNameTop]
                    || currentColumn() == mCols[PlotListTree::colNameBottom]
                    || currentColumn() == mCols[PlotListTree::colComments]
                    || currentColumn() == mCols[PlotListTree::colSection])) {
            lMenu.addSeparator();
            lSubMenu = lMenu.addMenu(QIcon(":/some/ico/ico/edit.png"),  tr("Change"));
            lAFieldEdit = lSubMenu->addAction(tr("Edit"));
            lAFieldEdit->setShortcut(QKeySequence("F2"));

            lAFieldReplace = lSubMenu->addAction(tr("Replace..."));
            //lAFieldReplace->setShortcut(QKeySequence("F4")); need to manually add hot key, too laizy
        }

        if (lAnyPlot
                && !(mPLTFlags & PLTNewVersion)) {
            bool lSeparatorAdded = false;
            if (!mNamedList) {
                if (mProject) {
                    lMenu.addSeparator();
                    lSeparatorAdded = true;
                    lSubMenu = lMenu.addMenu(tr("Add to list"));
                    foreach (PlotNamedListData * lNamedList, mProject->NamedListsConst()) {
                        QAction *lAction = lSubMenu->addAction(lNamedList->NameConst());
                        lAction->setData(QVariant::fromValue(lNamedList));
                        lAAddToList.append(lAction);
                    }
                    lANewListInProject = lSubMenu->addAction(tr("New list for project..."));
                }
            } else {
                // remove from list
                lMenu.addSeparator();
                lSeparatorAdded = true;
                lARemoveFromList = lMenu.addAction(tr("Remove from list"));
            }
            if (!lSeparatorAdded) {
                lMenu.addSeparator();
            }
            if (!(mPLTFlags & PLTNoEditMenu)) {
                lSubMenu = lMenu.addMenu(tr("Mark as"));
                lAMarkCancelled = lSubMenu->addAction(tr("Cancelled", "Cancelled"));
                lAMarkSent = lSubMenu->addAction(tr("Sent...", "Sent..."));
            }

            lSubMenu = lMenu.addMenu(tr("Export list"));

            if (gMSOffice->IsExcelInstalled()) {
                lAExcel = lSubMenu->addAction(QIcon(":/some/ico/ico/table-excel.png"), tr("Make XLS..."));
            }
            if (gSettings->Features.ReportMaatzXls) {
                lAMaazXls = lSubMenu->addAction(QIcon(":/some/ico/ico/table-excel.png"), tr("Make XLS for Maatz"));
            }
            if (gSettings->Features.ReportMetroXls) {
                lAMetroListXls = lSubMenu->addAction(QIcon(":/some/ico/ico/table-excel.png"), tr("Make XLS for David"));
            }
            lACopyToClipboard = lSubMenu->addAction(QIcon(":/some/ico/ico/copy.png"), tr("Copy to clipboard"));

            if (!(mPLTFlags & PLTNoEditMenu)
                    && lAnyUnsentPlot) {
                lMenu.addSeparator();
                lAMarkDeleted = lMenu.addAction(QIcon(":/some/ico/ico/minus.png"), tr("Delete"));
                //if (lAnyUnsentPlot) {
                lAMarkDeletedWithAllVersions = lMenu.addAction(QIcon(":/some/ico/ico/minus.png"), tr("Delete with all versions"));
                //}
            }
        }
    }

    if (lARes = lMenu.exec(QCursor::pos())) {
        if (lARes == lAFindGoTo) {
            // "owner" mode, can't send the pointer
            PlotData * lPlotGoto = gProjects->FindByIdPlot(lSingleItem->PlotConst()->Id());
            if (lPlotGoto) {
                gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, NULL);
            } else {
                QMessageBox::critical(this, tr("Project data"), tr("Cant find document with ID = ") + QString::number(lSingleItem->PlotConst()->Id()));
            }
        } else if (lARes == lASelectSingle) {
            emit mDialogButtonBox->accepted();
        } else if (lARes == lAExpandAll) {
            expandAll();
        } else if (lARes == lACollapseAll) {
            collapseAll();
        } else if (lARes == lAEdit || lARes == lAView || lARes == lAViewNoXrefs) {
            ViewEdit(lARes == lAEdit, lARes == lAViewNoXrefs);
        } else if (lARes == lAProps) {
            if (lSingleItem->PlotConst()) {
                gMainWindow->ShowPlotProp(lSingleItem->PlotRef(), NULL);
            } else if (lSingleItem->PlotHistoryConst()
                       && static_cast<PlotListTreeItem *>(lSingleItem->parent())
                       && static_cast<PlotListTreeItem *>(lSingleItem->parent())->PlotConst()) {
                gMainWindow->ShowPlotProp(static_cast<PlotListTreeItem *>(lSingleItem->parent())->PlotRef(), lSingleItem->PlotHistoryRef());
            }
        } else if (lARes == lARights) {
            gMainWindow->ShowPlotRights(lSingleItem->PlotRef());
        } else if (lARes == lARightsNew) {
            PlotRightsDlg dlg(lSingleItem->PlotConst()->Id(),gMainWindow);
            if (dlg.exec() == QDialog::Accepted)
            {
            //New My dialog
            }
        } else if (lARes == lANew) {
            emit AskMakeNewDocument(); // it through owner cos current tree type needed
        } else if (lARes == lAAudit) {
            AuditPurgeDlg dlg(gMainWindow);
            if (dlg.exec() == QDialog::Accepted) {
                // run audit-purge
                DoAuditPurge();
            }
        } else if (lARes == lAPublish) {
            PublishDlg dlg(gMainWindow);
            if (dlg.exec() == QDialog::Accepted) {
                // run publish
                DoPublish(false);
            }
        } else if (lARes == lAReplaceText) {
            ReplaceTextDlg dlg(gMainWindow);
            if (dlg.exec() == QDialog::Accepted) {
                DoReplaceText(&dlg);
            }
        } else if (lARes == lARecover) {
            gMainWindow->RecoverPlot(lSingleItem->PlotRef());
        } else if (lARes == lAMakeActive) {
            QMessageBox::StandardButton lMakeActive = QMessageBox::No;
            QList <int> lProjectsForUpdate;

            mSelectedPlotIds.clear();
            mSelectedPlotIdsCommon.clear();

            for (i = 0 ; i < lForMakeActive.length(); i++) {
                PlotData *lNewActive = static_cast<PlotListTreeItem *>(lForMakeActive.at(i))->PlotRef();
                if (lMakeActive != QMessageBox::YesToAll
                        && lMakeActive != QMessageBox::NoToAll) {
                    QMessageBox mb(this);
                    //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                    mb.setIcon(QMessageBox::Question);
                    mb.setWindowTitle(tr("Make document(s) active"));
                    mb.setText(tr("Are you sure you want to make this version active?") + "\n"
                               + QString::number(lNewActive->Id()) + " - " + lNewActive->CodeSheetConst() + " - " + lNewActive->VersionIntConst());
                    if (lForMakeActive.length() > 1)
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
                    else
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

                    lMakeActive = (QMessageBox::StandardButton) mb.exec();
                    if (lMakeActive == QMessageBox::NoToAll) break;
                }

                if (lMakeActive == QMessageBox::Yes
                        || lMakeActive == QMessageBox::YesToAll) {
                    if (lNewActive->MakeVersionActive()) { // it is actual update in database
                        // and this part - is update in iternal structures
                        PlotData *lOldActive = static_cast<PlotListTreeItem *>(lForMakeActive.at(i)->parent())->PlotRef();

                        // set working/non-working
                        lOldActive->setWorking(0, true);
                        lNewActive->setWorking(1, true);

                        // exchange versions list
                        lNewActive->VersionsRef() = lOldActive->VersionsConst();
                        lNewActive->VersionsRef().removeAll(lNewActive); // remove self
                        lNewActive->VersionsRef().append(lOldActive); // add old
                        lOldActive->VersionsRef().clear(); // clear versions list at old

                        mSelectedPlotIds.append(lNewActive->Id());

                        ProjectData * lProjectData = gProjects->FindByIdProject(lOldActive->IdProject());
                        if (lProjectData) {
                            lProjectData->PlotListRef().removeAll(lOldActive);
                            lProjectData->PlotListRef().append(lNewActive);

                            if (!lProjectsForUpdate.contains(lOldActive->IdProject())) {
                                lProjectsForUpdate.append(lOldActive->IdProject());
                                gProjects->EmitPlotListBeforeUpdate(lOldActive->IdProject());
                            }
                        } else {
                            // reread all projects' data (because very strange that we can't find project)
                            gProjects->InitProjectList(false);
                        }
                    }
                }
            }
            // plots of some projects need update
            foreach (i, lProjectsForUpdate) {
                gProjects->EmitPlotListNeedUpdate(i);
            }
        } else if (lARes == lANewVersion) {
            QList <int> lProjectsForUpdate;

            for (i = 0; i < lForNewVersions.length(); i++) {
                if (!lProjectsForUpdate.contains(lForNewVersions.at(i).first->IdProject()))
                    lProjectsForUpdate.append(lForNewVersions.at(i).first->IdProject());
            }

            PlotSimpleListDlg w(PlotSimpleListDlg::NDTNewVersion, lForNewVersions, this);
            if (w.exec()) {
                foreach (i, lProjectsForUpdate) {
                    gProjects->EmitPlotListBeforeUpdate(i);
                }
                // for this widnow
                mSelectedPlotIds.clear();
                mSelectedPlotIdsCommon.clear();
                foreach (PlotAndHistoryData lPlotNewVer, w.PlotsConst())
                    mSelectedPlotIds.append(lPlotNewVer.first->Id());
                // update all windows with this project
                foreach (i, lProjectsForUpdate) {
                    gProjects->EmitPlotListNeedUpdate(i);
                }
            }
        } else if (lARes == lAVersions) {
            //lSingleItem->PlotRef()->LoadVersions();
            gMainWindow->ShowPlotVersions(lSingleItem->PlotRef(), true);
        } else if (lARes == lAHistory) {
            gMainWindow->ShowPlotHist(lSingleItem->PlotRef(), 0, true, gSettings->IsDialogModal(parentWidget()));
        } else if (lARes == lAAddFiles) {
            gMainWindow->ShowPlotAddFiles(lSingleItem->PlotRef(), NULL, gSettings->IsDialogModal(parentWidget()));
        } else if (lARes == lAAttr) {
            gMainWindow->ShowPlotAttrs();
            emit itemSelectionChanged();
        } else if (lARes == lAXrefs) {
            gMainWindow->ShowPlotXrefs(lSingleItem->PlotRef(), NULL, true, false);
        } else if (lARes == lAXrefsAllOLD) {
            gMainWindow->ShowPlotXrefsAllOLD(lSingleItem->PlotRef());
        } else if (lARes == lAXrefFor) {
            gMainWindow->ShowPlotXrefFor(lSingleItem->PlotRef());
        } else if (lARes == lASave) {
            SaveDocuments();
        } else if (lARes == lASaveAddFiles) {
            SaveAddFiles();
        } else if (lARes == lALoad) {
            LoadOneDocument(lSingleItem->PlotRef());
        } else if (lARes == lALoadImages) {
            gMainWindow->ShowLoadImages(mProject->Id());
        } else if (lARes == lALoadXrefs) {
            gMainWindow->LoadXrefs(mProject->Id());
        } else if (lARes == lAGoto) {
            PlotHistoryData * lHistory = lSingleItem->PlotHistoryRef();
            PlotData * lPlotGoto = gProjects->FindByIdPlot(lHistory->FromIdPlot());
            if (lPlotGoto) {
                PlotHistoryData * lHistoryGoto = NULL;
                foreach (PlotHistoryData * lHistoryFound, lPlotGoto->HistoryConst()) {
                    if (lHistoryFound->Num() == lHistory->FromVersion()) {
                        lHistoryGoto = lHistoryFound;
                        break;
                    }
                }
                gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, lHistoryGoto);
            } else {
                QMessageBox::critical(this, tr("Project data"), tr("Cant find document with ID = ") + QString::number(lHistory->FromIdPlot()));
            }
        } else if (lARes == lAFieldEdit) {
            editItem(currentItem(), currentColumn());
        } else if (lARes == lAFieldReplace) {
            PlotListTreeChange w(this);
            PlotListTree::PLTCols aCol;

            if (currentColumn() == mCols[PlotListTree::colCode]) { aCol = PlotListTree::colCode;
            } else if (currentColumn() == mCols[PlotListTree::colSheet]) { aCol = PlotListTree::colSheet;
            } else if (currentColumn() == mCols[PlotListTree::colNameTop]) { aCol = PlotListTree::colNameTop;
            } else if (currentColumn() == mCols[PlotListTree::colNameBottom]) { aCol = PlotListTree::colNameBottom;
            } else if (currentColumn() == mCols[PlotListTree::colComments]) { aCol = PlotListTree::colComments;
            } else if (currentColumn() == mCols[PlotListTree::colSection]) aCol = PlotListTree::colSection;

            if (w.SetData(aCol, selected)) {
                if (w.exec() == QDialog::Accepted) {
                    const QList<int> & lProjectIds = w.ProjectIds();
                    foreach (int lIdProject, lProjectIds) {
                        gProjects->EmitPlotListBeforeUpdate(lIdProject);
                        gProjects->EmitPlotListNeedUpdate(lIdProject);
                    }
                }
            }
        } else if (lAAddToList.contains(lARes)) {
            PlotNamedListData * lPlotNamedList = lARes->data().value<PlotNamedListData *>();

            bool lIsAny = false;
            int lAddedCnt = 0;
            for (i = 0; i < selected.length(); i++) {
                const PlotData * lPlotData = static_cast<PlotListTreeItem *>(selected[i])->PlotConst();
                if (lPlotData
                        && !lPlotNamedList->IdsCommonConst().contains(lPlotData->IdCommon())) {
                    if (!lIsAny) {
                        if (QMessageBox::question(this, tr("Adding documents to list"),
                                                  tr("Add selected documents to list") + " \"" + lPlotNamedList->NameConst() + "\"?") != QMessageBox::Yes) break;
                        lIsAny = true;
                    }
                    lPlotNamedList->IdsCommonRef().append(lPlotData->IdCommon());
                    lAddedCnt++;
                }
            }
            if (lIsAny) {
                if (db.transaction()) {
                    bool lIsOk = false;
                    if (lPlotNamedList->SaveData()) {
                        if (db.commit()) {
                            lIsOk = true;
                            lPlotNamedList->CommitEdit();
                            emit gProjects->PlotsNamedListNeedUpdate(lPlotNamedList);
                            QMessageBox::information(this, tr("Adding documents to list"),
                                                     QString::number(lAddedCnt) + " " + tr("document(s) added to list") + " \"" + lPlotNamedList->NameConst() + "\"");
                        } else {
                            gLogger->ShowSqlError(this, tr("Adding documents to list"), tr("Can't commit"), db);
                        }
                    }
                    if (!lIsOk) {
                        db.rollback();
                        lPlotNamedList->RollbackEdit();
                    }
                } else {
                    gLogger->ShowSqlError(this, tr("Adding documents to list"), tr("Can't start transaction"), db);
                }
            } else {
                QMessageBox::information(this, tr("Adding documents to list"), tr("No documents added to list"));
            }
        } else if (lARes == lANewListInProject) {
            QInputDialog lDlg(this);
            lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
            lDlg.setWindowTitle(tr("New list"));
            lDlg.setLabelText(tr("Enter name for new list"));
            if (lDlg.exec() == QDialog::Accepted) {
                int lId = 0;
                PlotNamedListData * lPlotNamedList;
                if (lPlotNamedList = PlotNamedListData::INSERT(lId, mProject->Id(), lDlg.textValue())) {
                    mProject->NamedListsRef().append(lPlotNamedList);
                    bool lIsAny = false;
                    int lAddedCnt = 0;
                    for (i = 0; i < selected.length(); i++) {
                        const PlotData * lPlotData = static_cast<PlotListTreeItem *>(selected[i])->PlotConst();
                        if (lPlotData
                                && !lPlotNamedList->IdsCommonConst().contains(lPlotData->IdCommon())) {
                            if (!lIsAny) {
                                if (QMessageBox::question(this, tr("Adding documents to list"),
                                                          tr("Add selected documents to list") + " \"" + lPlotNamedList->NameConst() + "\"?") != QMessageBox::Yes) break;
                                lIsAny = true;
                            }
                            lPlotNamedList->IdsCommonRef().append(lPlotData->IdCommon());
                            lAddedCnt++;
                        }
                    }
                    if (lIsAny) {
                        if (db.transaction()) {
                            bool lIsOk = false;
                            if (lPlotNamedList->SaveData()) {
                                if (db.commit()) {
                                    lIsOk = true;
                                    lPlotNamedList->CommitEdit();
                                    emit gProjects->PlotsNamedListNeedUpdate(lPlotNamedList);
                                    QMessageBox::information(this, tr("Adding documents to list"),
                                                             QString::number(lAddedCnt) + " " + tr("document(s) added to list") + " \"" + lPlotNamedList->NameConst() + "\"");
                                } else {
                                    gLogger->ShowSqlError(this, tr("Adding documents to list"), tr("Can't commit"), db);
                                }
                            }
                            if (!lIsOk) {
                                db.rollback();
                                lPlotNamedList->RollbackEdit();
                            }
                        } else {
                            gLogger->ShowSqlError(this, tr("Adding documents to list"), tr("Can't start transaction"), db);
                        }
                    } else {
                        QMessageBox::information(this, tr("Adding documents to list"), tr("No documents added to list"));
                    }


                    gProjects->EmitPlotListBeforeUpdate(mProject->Id());
                    gProjects->EmitPlotListNeedUpdate(mProject->Id());
                }
            }
        } else if (lARes == lARemoveFromList) {
            if (mNamedList) {
                bool lIsAny = false;
                int lRemovedCnt = 0;
                for (i = 0; i < selected.length(); i++) {
                    const PlotData * lPlotData = static_cast<PlotListTreeItem *>(selected[i])->PlotConst();
                    if (lPlotData
                            && mNamedList->IdsCommonConst().contains(lPlotData->IdCommon())) {
                        if (!lIsAny) {
                            if (QMessageBox::question(this, tr("Removing documents from list"),
                                                      tr("Remove selected documents from list") + " \"" + mNamedList->NameConst() + "\"?") != QMessageBox::Yes) break;
                            lIsAny = true;
                        }
                        mNamedList->IdsCommonRef().removeAll(lPlotData->IdCommon());
                        lRemovedCnt++;
                    }
                }
                if (lIsAny) {
                    if (db.transaction()) {
                        bool lIsOk = false;
                        if (mNamedList->SaveData()) {
                            if (db.commit()) {
                                lIsOk = true;
                                mNamedList->CommitEdit();
                                emit gProjects->PlotsNamedListNeedUpdate(mNamedList);
                                QMessageBox::information(this, tr("Removing documents from list"),
                                                         QString::number(lRemovedCnt) + " " + tr("document(s) removed from list") + " \"" + mNamedList->NameConst() + "\"");
                            } else {
                                gLogger->ShowSqlError(this, tr("Removing documents from list"), tr("Can't commit"), db);
                            }
                        }
                        if (!lIsOk) {
                            db.rollback();
                            mNamedList->RollbackEdit();
                        }
                    } else {
                        gLogger->ShowSqlError(this, tr("Removing documents from list"), tr("Can't start transaction"), db);
                    }
                } else {
                    QMessageBox::information(this, tr("Removing documents from list"), tr("No documents removed from list"));
                }
            }
        } else if (lARes == lAMarkCancelled) {
            if (QMessageBox::question(this, tr("Cancelling documents"),
                                      tr("Cancel selected documents?")) == QMessageBox::Yes) {
                QList<int> lProjectIdsForUpdate;
                QList<QVariant> lValues;
                lValues.append(1);
                for (i = 0; i < selected.length(); i++) {
                    PlotData * lPlotData = static_cast<PlotListTreeItem *>(selected[i])->PlotRef();
                    if (lPlotData) {
                        if (lPlotData->SetPropWithVersions(true, true, PlotData::MATCancelled, lValues)) {
                           if (!lProjectIdsForUpdate.contains(lPlotData->IdProject()))
                               lProjectIdsForUpdate.append(lPlotData->IdProject());
                        }
                    }
                }
                foreach (i, lProjectIdsForUpdate) {
                    gProjects->FindByIdProject(i)->ReinitLists();
                }
            }
        } else if (lARes == lAMarkSent) {
            SentParamsDgl w(this);
            if (w.exec() == QDialog::Accepted) {
                QList<int> lProjectIdsForUpdate;
                QStringList lSkipped;

                for (i = 0; i < selected.length(); i++) {
                    PlotData * lPlotData = static_cast<PlotListTreeItem *>(selected[i])->PlotRef();
                    if (lPlotData) {
                        if (lPlotData->SentDateConst().isNull()
                                && lPlotData->SentUserConst().isEmpty()) {
                            lPlotData->setSentDate(w.SentDate());
                            lPlotData->setSentUser(w.SentUser());
                            if (lPlotData->SaveData()) {
                                lPlotData->CommitEdit();
                            } else {
                                lPlotData->RollbackEdit();
                            }

                        } else {
                            lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst());
                        }
                    }
                }
                foreach (i, lProjectIdsForUpdate) {
                    gProjects->FindByIdProject(i)->ReinitLists();
                }

                if (!lSkipped.isEmpty()) {
                    QMessageBox mb(this);
                    //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                    mb.setIcon(QMessageBox::Warning);
                    mb.setWindowTitle(tr("Sending documents"));
                    if (lSkipped.length() == 1) {
                        mb.setText(tr("Document") + " " + lSkipped.at(0) + " " + tr("already marked as sent"));
                    } else {
                        mb.setText(tr("Some documents already marked as sent"));
                        mb.setDetailedText(lSkipped.join("\n"));
                    }
                    mb.addButton(QMessageBox::Ok);

                    // motherfucker motherfucker
                    QSpacerItem * horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                    QGridLayout * layout = (QGridLayout *) mb.layout();
                    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

                    mb.exec();
                }
            }
        } else if (lARes == lAMarkDeleted
                   || lARes == lAMarkDeletedWithAllVersions) {
/*            QInputDialog lDlg(this);
            lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
            QMessageBox::critical(NULL, "", QString::number(lDlg.windowFlags(), 16));
            lDlg.setWindowTitle((lARes == lAMarkDeleted)?tr("Deleting documents"):tr("Deleting documents with all versions"));
            lDlg.setLabelText((lARes == lAMarkDeleted)?tr("Enter your password for delete selected documents"):tr("Enter your password for delete selected documents with all versions"));
            lDlg.setTextEchoMode(QLineEdit::Password);
            if (lDlg.exec() == QDialog::Accepted
                    && lDlg.textValue() == db.password()) {*/

            bool lPassOK = false;
            if (gSettings->DocumentTree.AskPasswordOnDelete) {
                lPassOK = gSettings->AskPassword((lARes == lAMarkDeleted)?tr("Deleting documents"):tr("Deleting documents with all versions"),
                                                 (lARes == lAMarkDeleted)?tr("Enter your password for delete selected documents"):tr("Enter your password for delete selected documents with all versions"));
            }

            if (!gSettings->DocumentTree.AskPasswordOnDelete
                    && QMessageBox::question(this, (lARes == lAMarkDeleted)?tr("Deleting documents"):tr("Deleting documents with all versions"),
                                             (lARes == lAMarkDeleted)?tr("Delete selected documents?"):tr("Delete selected documents with all versions?")) == QMessageBox::Yes
                    || gSettings->DocumentTree.AskPasswordOnDelete && lPassOK) {
                QList<int> lProjectIdsForUpdate;
                QStringList lSkipped;
                QList<QVariant> lValues;
                int lDeletedCnt = 0;
                lValues.append(1);

                for (i = 0; i < selected.length(); i++) {
                    PlotData * lPlotData = static_cast<PlotListTreeItem *>(selected[i])->PlotRef();
                    if (lPlotData) {
                        if (lARes == lAMarkDeleted) {
                            // remove single document
                            if (lPlotData->SentDateConst().isNull()) {
                                if (!db.transaction()) {
                                    gLogger->ShowSqlError(this, tr("Deleting documents"), tr("Can't start transaction"), db);
                                } else {
                                    bool b;

                                    // update it even it will be error
                                    if (!lProjectIdsForUpdate.contains(lPlotData->IdProject()))
                                        lProjectIdsForUpdate.append(lPlotData->IdProject());

                                    if (lPlotData->Working()) {
                                        if (!lPlotData->LoadVersions()) {
                                            b = false; // can't load versions
                                        } else if (lPlotData->VersionsConst().length()) {
                                            // make other version active
                                            std::sort(lPlotData->VersionsRef().begin(), lPlotData->VersionsRef().end(),
                                                      [] (const PlotData * d1, const PlotData * d2) { return d1->EditDateConst() < d2->EditDateConst(); });
                                            b = lPlotData->VersionsRef().at(0)->MakeVersionActive();
                                        } else {
                                            b = true; // ok, no other versions
                                        }
                                    } else {
                                        b = true; // ok, remove non-working version
                                    }

                                    if (b) {
                                        lPlotData->setDeleted(1);
                                        b = lPlotData->SaveData();
                                    }

                                    if (b) {
                                        if (db.commit()) {
                                            lPlotData->CommitEdit();
                                            lDeletedCnt++;
                                        } else {
                                            gLogger->ShowSqlError(this, tr("Deleting documents"), tr("Can't commit"), db);
                                            b = false;
                                        }
                                    }
                                    if (!b) {
                                        db.rollback();
                                        lPlotData->RollbackEdit();
                                        lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst());
                                    }
                                }
                            } else {
                                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst());
                            }
                        } else {
                            // remove with all versions
                            if (!lPlotData->IsSent()) {
                                if (lPlotData->SetPropWithVersions(true, true, PlotData::MATDeleted, lValues)) {
                                    if (!lProjectIdsForUpdate.contains(lPlotData->IdProject()))
                                        lProjectIdsForUpdate.append(lPlotData->IdProject());
                                    lDeletedCnt++;
                                }
                            } else {
                                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst());
                            }
                        }
                    }
                }

                foreach (i, lProjectIdsForUpdate) {
                    gProjects->FindByIdProject(i)->ReinitLists();
                }

                if (!lSkipped.isEmpty()) {
                    QMessageBox mb(this);
                    //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                    mb.setIcon(QMessageBox::Warning);
                    mb.setWindowTitle(tr("Deleting documents"));
                    if (lSkipped.length() == 1) {
                        mb.setText(tr("Document") + " " + lSkipped.at(0) + " " + tr("can't be deleted because it is already sent"));
                    } else {
                        mb.setText(tr("Some documents can't be deleted because it is already sent"));
                        mb.setDetailedText(lSkipped.join("\n"));
                    }
                    mb.addButton(QMessageBox::Ok);

                    // motherfucker motherfucker
                    QSpacerItem * horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                    QGridLayout * layout = (QGridLayout *) mb.layout();
                    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

                    mb.exec();
                }

                QMessageBox::information(this, tr("Deleting documents"), QString::number(lDeletedCnt) + " " + tr("documents was deleted"));
            }
        } else if (lARes == lAExcel) {
            MakeXLS();
        } else if (lARes == lAMaazXls) {
            DoPublish(true);
        } else if (lARes == lAMetroListXls) {
            QList <const PlotData *> lPlots;
            for (i = 0; i < selected.length(); i++) {
                const PlotData * lPlot = static_cast<PlotListTreeItem *>(selected[i])->PlotConst();
                if (lPlot
                        && !lPlot->Deleted()
                        && lPlot->Working()
                        && lPlot->ExtensionConst().toLower() == "dwg") {
                    lPlots.append(lPlot);
                }
            }
            if (!lPlots.isEmpty()) {
                DoListForMetro(this, lPlots);
            }
        } else if (lARes == lACopyToClipboard) {
            gSettings->CopyToClipboard(this);
        } else if (lARes == lAUndelete) {
            bool lPassOK = false;
            if (gSettings->DocumentTree.AskPasswordOnDelete) {
                lPassOK = gSettings->AskPassword(tr("Undeleting documents"), tr("Enter your password for undelete selected documents"));
            }

            if (!gSettings->DocumentTree.AskPasswordOnDelete
                    && QMessageBox::question(this, tr("Undeleting documents"), tr("Undelete selected documents?")) == QMessageBox::Yes
                    || gSettings->DocumentTree.AskPasswordOnDelete && lPassOK) {
                QList <int> lUndeletedPlotIds;
                QList <int> lProjectIds;
                for (i = 0; i < selected.length(); i++) {
                    PlotData * lPlot = static_cast<PlotListTreeItem *>(selected[i])->PlotRef();
                    if (lPlot && lPlot->Deleted()) {
                        lPlot->Undelete();
                        if (lPlot->SaveData()) {
                            lPlot->CommitEdit();
                            lUndeletedPlotIds.append(lPlot->Id());
                            if (!lProjectIds.contains(lPlot->IdProject()))
                                lProjectIds.append(lPlot->IdProject());
                        } else {
                            lPlot->RollbackEdit();
                        }
                    }
                }
                for (i = 0; i < lProjectIds.length(); i++) {
                    ProjectData *lProject = gProjects->FindByIdProject(lProjectIds.at(i));
                    if (lProject) {
                        lProject->ReinitLists();
                    }
                }
                if (!lUndeletedPlotIds.isEmpty()) {
                    emit WasUndeleted();
                }

                if (QMessageBox::question(this, tr("Undeleting documents"), QString::number(lUndeletedPlotIds.length()) + " " + tr("documents was undeleted")
                                          + "\n" + tr("Go to undeleted documents?")) == QMessageBox::Yes) {
                    for (i = 0; i < lUndeletedPlotIds.length(); i++) {
                        PlotData * lPlotGoto = gProjects->FindByIdPlot(lUndeletedPlotIds.at(i));
                        if (lPlotGoto) {
                            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, NULL);
                        } else {
                            QMessageBox::critical(this, tr("Project data"), tr("Cant find document with ID = ") + QString::number(lUndeletedPlotIds.at(i)));
                        }
                    }
                }

            } else {
                QMessageBox::critical(this, tr("Undeleting documents"), tr("Documents was not undeleted"));
            }
        } else if (lARes == lAProjProps) {
            ProjectData *lProject = gProjects->FindByIdProject(lSingleItem->text(mCols[colIdProject]).toLong());
            if (lProject) {
                lProject->ShowProps(this);
            }
        } else if (lARes == lAProjDocs) {
            ProjectData *lProject = gProjects->FindByIdProject(lSingleItem->text(mCols[colIdProject]).toLong());
            if (lProject) {
                gMainWindow->ShowPlotList(lProject, NULL, NULL);
            }
        }
    }
    qDeleteAll(lAAddToList);
}

void PlotListTree::OnItemDoubleClicked(QTreeWidgetItem * item, int column) {
    if (item) {
        if (mPLTFlags & PLTSingleSelMode) {
            if (mDialogButtonBox
                    && (static_cast<PlotListTreeItem *>(item)->PlotConst()
                        /*|| static_cast<PlotListTreeItem *>(item)->PlotHistoryConst()*/)) {
                emit mDialogButtonBox->accepted();
            }
        } else if (mPLTFlags & PLTFindMode) {
            // "owner" mode, can't send the pointer
            PlotData * lPlotGoto = gProjects->FindByIdPlot(static_cast<PlotListTreeItem *>(item)->PlotConst()->Id());
            if (lPlotGoto) {
                gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, NULL);
            } else {
                QMessageBox::critical(this, tr("Project data"), tr("Cant find document with ID = ") + QString::number(static_cast<PlotListTreeItem *>(item)->PlotConst()->Id()));
            }
        } else {
            switch (gSettings->DocumentTree.OnDocDblClick) {
            case GlobalSettings::DocumentTreeStruct::DDView: {
                ViewEdit(false, false);
                break;
            }

            case GlobalSettings::DocumentTreeStruct::DDProps:
                if (static_cast<PlotListTreeItem *>(item)->PlotConst()) {
                    gMainWindow->ShowPlotProp(static_cast<PlotListTreeItem *>(item)->PlotRef(), NULL);
                } else if (static_cast<PlotListTreeItem *>(item)->PlotHistoryConst()
                           && static_cast<PlotListTreeItem *>(item->parent())
                           && static_cast<PlotListTreeItem *>(item->parent())->PlotConst()) {
                    gMainWindow->ShowPlotProp(static_cast<PlotListTreeItem *>(item->parent())->PlotRef(), static_cast<PlotListTreeItem *>(item)->PlotHistoryRef());
                }
                //                PlotListTreeItem * lSingleItem = NULL;
//                QList <QTreeWidgetItem *> selected = selectedItems();

//                if (gSettings->DocumentTree.SingleSelect == gSettings->DocumentTreeStruct::SSCurrent) {
//                    lSingleItem = static_cast<PlotListTreeItem *>(currentItem());
//                } else if (selected.count() == 1) {
//                    lSingleItem = static_cast<PlotListTreeItem *>(selected.at(0));
//                }

//                if (lSingleItem) {
//                    if (lSingleItem->PlotConst()) {
//                        gMainWindow->ShowPlotProp(lSingleItem->PlotRef(), NULL);
//                    } else if (lSingleItem->PlotHistoryConst()
//                               && static_cast<PlotListTreeItem *>(lSingleItem->parent())
//                               && static_cast<PlotListTreeItem *>(lSingleItem->parent())->PlotConst()) {
//                        gMainWindow->ShowPlotProp(static_cast<PlotListTreeItem *>(lSingleItem->parent())->PlotRef(), lSingleItem->PlotHistoryRef());
//                    }
//                }
                break;
            }
        }
    }
}


void PlotListTree::HideColumn(PLTCols aColName, bool aHide) {
    if (mCols[aColName] != -1) setColumnHidden(mCols[aColName], aHide);
}

void PlotListTree::SetSecondLevelType(GlobalSettings::DocumentTreeStruct::SLT aSLT) {
    mSLT = aSLT;
    InitColumns(mPLTFlags);
    //gLogger->ShowErrorInList(NULL, QTime::currentTime().toString(), "SetSecondLevelType");
}

GlobalSettings::DocumentTreeStruct::SLT PlotListTree::SLT() const {
    return mSLT;
}

void PlotListTree::SetSelectedPlotId(int aSelectedPlotId) {
    mSelectedPlotIds.clear();

    mCurrentItemType = 0;
    mCurrentColumn = 0;
    mCurrentItemId = aSelectedPlotId;
    if (aSelectedPlotId) {
        mSelectedPlotIds.append(aSelectedPlotId);
    }
}

PlotListTreeItem *PlotListTree::itemFromIndex(const QModelIndex & index) const {
    return static_cast<PlotListTreeItem *>(QTreeWidget::itemFromIndex(index));
}

void PlotListTree::Populate(const PlotTreeItem * aPlotTreeItem) {
    bool lScrollToCurrent = false;

    clear();

    if (aPlotTreeItem->TreeDataConst()->ActualShortData()) {
        setColumnHidden(mCols[colVersionExt], true);
        setColumnHidden(mCols[colVersionDate], true);
        setColumnHidden(mCols[colSentDate], true);
        setColumnHidden(mCols[colSentBy], true);
        setColumnHidden(mCols[colSection], true);
    } else {
        setColumnHidden(mCols[colVersionExt], !mVerExtVisible);
        setColumnHidden(mCols[colVersionDate], !mVerDateVisible);
        setColumnHidden(mCols[colSentDate], !mSentDateVisible);
        setColumnHidden(mCols[colSentBy], !mSentUserVisible);
        setColumnHidden(mCols[colSection], !mComplectVisible);
    }

    if (aPlotTreeItem->TreeDataConst()->ActualIsXref()) {
        setColumnHidden(mCols[colBlockName], false);
    } else {
        setColumnHidden(mCols[colBlockName], !mBlockNameVisible);
    }

    PopulateInternal(aPlotTreeItem);

    if (mProject) {
        //setColumnHidden(mCols[colSection], mProject->ComplectListConst().isEmpty());
        setColumnHidden(mCols[colSheet], mProject->SheetDigitsActual() == -1);
    } else {
        //setColumnHidden(mCols[colSection], false);
    }

    // expanded items
    if (!mExpandedPlotIdsCommon.isEmpty()) {
        for (int i = 0; i < topLevelItemCount(); i++) {
            if (static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()
                    && mExpandedPlotIdsCommon.contains(static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()->IdCommon()))
                topLevelItem(i)->setExpanded(true);
        }
        mExpandedPlotIdsCommon.clear();
    }

    // current item
    if (mCurrentItemId) {
        bool b = true;
        lScrollToCurrent = true;
        for (int i = 0; i < topLevelItemCount(); i++) {
            if (!mCurrentItemType
                    && static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()
                    && mCurrentItemId == static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()->Id()) {
                setCurrentItem(topLevelItem(i), mCurrentColumn);
                //lScrollToCurrent = (mSelectedPlotIdsCommon.length() == 1 && mSelectedPlotIdsCommon.at(0) == mCurrentItemId);
                break;
            }
            for (int j = 0; j < topLevelItem(i)->childCount(); j++) {
                switch(mCurrentItemType) {
                case 0:
                    if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotConst()
                            && mCurrentItemId == static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotConst()->IdCommon()) {
                        setCurrentItem(topLevelItem(i)->child(j), mCurrentColumn);
                        b = false;
                    }
                    break;
                case GlobalSettings::DocumentTreeStruct::SLTLayouts:
                    if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->DwgLayoutConst()
                            && mCurrentItemId == static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->DwgLayoutConst()->Id()) {
                        setCurrentItem(topLevelItem(i)->child(j), mCurrentColumn);
                        b = false;
                    }
                    break;
                case GlobalSettings::DocumentTreeStruct::SLTHistory:
                    if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotHistoryConst()
                            && mCurrentItemId == static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotHistoryConst()->Id()) {
                        setCurrentItem(topLevelItem(i)->child(j), mCurrentColumn);
                        b = false;
                    }
                    break;
                case GlobalSettings::DocumentTreeStruct::SLTAddFiles:
                    if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotAddFileConst()
                            && mCurrentItemId == static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotAddFileConst()->Id()) {
                        setCurrentItem(topLevelItem(i)->child(j), mCurrentColumn);
                        b = false;
                    }
                    break;
                }
                if (!b) break;
            }
            if (!b) break;
        }
        mCurrentItemType = 0;
        mCurrentItemId = 0;
        mCurrentColumn = 0;
    }

    if (!mSelectedPlotIds.isEmpty()
            || !mSelectedPlotIdsCommon.isEmpty()
            || !mSelectedLayoutIds.isEmpty()
            || !mSelectedHistoryIds.isEmpty()
            || !mSelectedAddFilesIds.isEmpty()) {

        for (int i = 0; i < topLevelItemCount(); i++) {
            if (static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()
                    && (mSelectedPlotIds.contains(static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()->Id())
                        || mSelectedPlotIdsCommon.contains(static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()->IdCommon()))) {
                topLevelItem(i)->setSelected(true);
            }
            for (int j = 0; j < topLevelItem(i)->childCount(); j++) {
                if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotConst()
                        && (mSelectedPlotIds.contains(static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotConst()->Id())
                            || mSelectedPlotIdsCommon.contains(static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotConst()->IdCommon()))) {
                    topLevelItem(i)->child(j)->setSelected(true);
                }
                if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->DwgLayoutConst()
                        && mSelectedLayoutIds.contains(static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->DwgLayoutConst()->Id())) {
                    topLevelItem(i)->child(j)->setSelected(true);
                }
                if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotHistoryConst()
                        && mSelectedHistoryIds.contains(static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotHistoryConst()->Id())) {
                    topLevelItem(i)->child(j)->setSelected(true);
                }
                if (static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotAddFileConst()
                        && mSelectedAddFilesIds.contains(static_cast<PlotListTreeItem *>(topLevelItem(i)->child(j))->PlotAddFileConst()->Id())) {
                    topLevelItem(i)->child(j)->setSelected(true);
                }
            }
        }
        mSelectedPlotIds.clear();
        mSelectedPlotIdsCommon.clear();
        mSelectedLayoutIds.clear();
        mSelectedHistoryIds.clear();
        mSelectedAddFilesIds.clear();
    }

    if (gSettings->DocumentTree.AutoWidth) {
        mIgnoreSectionResize = true;
        for (int i = 0; i < columnCount(); i++) resizeColumnToContents(i);
        mIgnoreSectionResize = false;
    }

    if (lScrollToCurrent) scrollToItem(currentItem(), QAbstractItemView::PositionAtCenter);
}

void PlotListTree::PopulateLayoutsInternal(PlotListTreeItem * aParentItem, PlotData *aPlot) {
    int j, k;
    for (j = 0; j < aPlot->LayoutsConst().length(); j++) {
        DwgLayoutData *lLayout = aPlot->LayoutsConst().at(j);
        bool lHasAnyBlock = false;
        if (!lLayout->BlocksConst().isEmpty()) {
            for (k = 0; k < lLayout->BlocksConst().length();k ++) {
                if (lLayout->BlocksConst().at(k)->HasAnyProp()) {
                    PlotListTreeItem * pdLayout = new PlotListTreeItem(this, lLayout, lLayout->BlocksConst().at(k));
                    aParentItem->addChild(pdLayout);
                    lHasAnyBlock = true;
                }
            }
        }
        if (!lHasAnyBlock) {
            PlotListTreeItem * pdLayout = new PlotListTreeItem(this, lLayout, NULL);
            aParentItem->addChild(pdLayout);
        }
    }
}

void PlotListTree::PopulateInternal(const PlotTreeItem * aPlotTreeItem) {
    int i, j;

    for (i = 0; i < aPlotTreeItem->PlotsConst().count(); i++) {
        if (mHideCancelled && aPlotTreeItem->PlotsConst().at(i)->Cancelled()) continue;
        if (!mComplect.isEmpty() && aPlotTreeItem->PlotsConst().at(i)->SectionConst() != mComplect) continue;
        if (mNamedList && !mNamedList->IdsCommonConst().contains(aPlotTreeItem->PlotsConst().at(i)->IdCommon())) continue;
        PlotListTreeItem * pd = new PlotListTreeItem(this, aPlotTreeItem->PlotsConst().at(i), NULL, 0);
        addTopLevelItem(pd);

        switch (mSLT) {

        case GlobalSettings::DocumentTreeStruct::SLTLayouts:
            PopulateLayoutsInternal(pd, aPlotTreeItem->PlotsConst().at(i));
            break;
        case GlobalSettings::DocumentTreeStruct::SLTVersions:
            for (j = 0; j < aPlotTreeItem->PlotsConst().at(i)->VersionsConst().length(); j++) {
                if (aPlotTreeItem->PlotsConst().at(i)->VersionsConst().at(j)->Deleted()) continue; // skip deleted
                PlotListTreeItem * pdVersion = new PlotListTreeItem(this, aPlotTreeItem->PlotsConst().at(i)->VersionsConst().at(j), NULL, 0, true);
                pd->addChild(pdVersion);

            }
            break;
        case GlobalSettings::DocumentTreeStruct::SLTHistory:
            if (mProject && gProjects->IsPlotListInUpdate(mProject->Id())) {
                for (j = 0; j < aPlotTreeItem->PlotsConst().at(i)->HistoryConst().length(); j++) {
                    PlotListTreeItem * pdHistory = new PlotListTreeItem(this, aPlotTreeItem->PlotsConst().at(i)->HistoryConst().at(j));
                    pd->addChild(pdHistory);
                }
            } else {
                aPlotTreeItem->PlotsConst().at(i)->ReinitHistory();
            }
            break;
        case GlobalSettings::DocumentTreeStruct::SLTAddFiles:
            for (j = 0; j < aPlotTreeItem->PlotsConst().at(i)->AddFilesConst().length(); j++) {
                PlotListTreeItem * pdAddFile = new PlotListTreeItem(this, aPlotTreeItem->PlotsConst().at(i)->AddFilesConst().at(j));
                pd->addChild(pdAddFile);

            }
            break;
        }
        if (gSettings->DocumentTree.ExpandOnShow) {
            pd->setExpanded(true);
        }
    }

    for (i = 0; i < aPlotTreeItem->childCount(); i++) {
        PopulateInternal(static_cast<PlotTreeItem *>(aPlotTreeItem->child(i)));
    }
}

bool PlotListTree::nativeEvent(const QByteArray & eventType, void * message, long * result) {
    if (AcadXchgDialog::DoNativeEvent(eventType, message, result)) return true;

    if (((MSG *) message)->message == WM_COPYDATA) {
        PCOPYDATASTRUCT CpyData;

        CpyData = (PCOPYDATASTRUCT) (((MSG *) message)->lParam);
        if (CpyData->dwData == 100
                && CpyData->cbData == RecordDataFromAcad::GetDataSize()) {

            RecordDataFromAcad lRecordDataFromAcad;

            memcpy(lRecordDataFromAcad.GetDataBuffer(), CpyData->lpData, CpyData->cbData);
            if (lRecordDataFromAcad.DataType() == RecordDataFromAcad::PublishData) {
                ::SetWindowPos((HWND) gMainWindow->effectiveWinId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
                ::SetWindowPos((HWND) gMainWindow->effectiveWinId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
                switch (lRecordDataFromAcad.PUBWhatToDo()) {
                case 0:
                    // show report
                    gMainWindow->ShowPublishReport(lRecordDataFromAcad.PUBId());
                    break;
                case 1:
                    // make list for Maatz in Excel
                    DoPublishReport(gMainWindow, lRecordDataFromAcad.PUBId());
                    break;
                }
            }

            *result = RET_OK;
            return true;
        }
    }

    return QTreeWidget::nativeEvent(eventType, message, result);
}

void PlotListTree::ViewEdit(bool aTrueForEdit, bool aNoXrefs) {
    int i;
    QStringList lSkipped;
    MainDataForCopyToAcad lDataForAcad(aTrueForEdit?2:1, aNoXrefs);
    QList<int> lIdPlotForSkipping;
    QList<PlotAndHistoryData> lPlotForChecking;
    QList<QTreeWidgetItem *> lSelected;
    QMessageBox::StandardButton lUserEditOld = QMessageBox::No;
    QMessageBox::StandardButton lUserEditNonActive = QMessageBox::No;

    if (currentItem()
            && (static_cast<PlotListTreeItem *>(currentItem())->PlotConst()
                    && static_cast<PlotListTreeItem *>(currentItem())->PlotConst()->IsPicture()
                || static_cast<PlotListTreeItem *>(currentItem())->PlotHistoryConst()
                    && static_cast<PlotListTreeItem *>(currentItem())->PlotHistoryConst()->IsPicture()
                || static_cast<PlotListTreeItem *>(currentItem())->PlotAddFileConst()
                    && static_cast<PlotListTreeItem *>(currentItem())->PlotAddFileConst()->IsPicture())) {
        if (aTrueForEdit) {
            if (static_cast<PlotListTreeItem *>(currentItem())->PlotConst()) {
                ImageViewerThread::ModalEdit(this);
            } else if (static_cast<PlotListTreeItem *>(currentItem())->PlotHistoryConst()) {
                ImageViewerThread::ModalEdit(static_cast<PlotListTreeItem *>(currentItem()->parent())->PlotRef(),
                                             static_cast<PlotListTreeItem *>(currentItem())->PlotHistoryRef());
            }
            return;
        } else {
            switch (gSettings->Image.ViewerType) {
            case 0:
                if (mVPImageViewer) return;
                mVPImageViewer = new VPImageViewer(this, false, this/*it is parent, can be gMainWindow*/);
                connect(mVPImageViewer, SIGNAL(finished(int)), this, SLOT(OnDeleteVPImageViewer(int)));
                mVPImageViewer->setAttribute(Qt::WA_DeleteOnClose);
                mVPImageViewer->show();
                break;
            case 1:
            case 2:
                ImageViewerThread::ModalViewList(this);
                break;
            }
            return;
        }
    }

    if (gSettings->DocumentTree.OpenSingleDocument) {
        if (currentItem()
                && (static_cast<PlotListTreeItem *>(currentItem())->PlotConst()
                    || static_cast<PlotListTreeItem *>(currentItem())->PlotHistoryConst())) {
            lSelected.append(currentItem());
        } else {
            return;
        }
    } else {
        lSelected  = selectedItems();
    }

    // --------------------------
    // skip with no PlotData (doesn't matter, just to be sure)
    for (i = lSelected.length() - 1; i >= 0; i--) {
        PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(lSelected[i]);

        PlotData * lPlotData;
        if (!(lPlotData = lItem->PlotRef())) {
            if (lItem->parent()) lItem = static_cast<PlotListTreeItem *>(lItem->parent());
            lPlotData = lItem->PlotRef();
        }

        if (!lPlotData) {
            // no plot data
            lSelected.removeAt(i);
            continue;
        }
    }

    for (i = 0; i < lSelected.length(); i++) {
        PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(lSelected[i]);

        PlotHistoryData * lPlotHistoryData = lItem->PlotHistoryRef();
        PlotData * lPlotData;
        if (!(lPlotData = lItem->PlotRef())) {
            if (lItem->parent()) lItem = static_cast<PlotListTreeItem *>(lItem->parent());
            lPlotData = lItem->PlotRef();
        }

        if (lIdPlotForSkipping.contains(lPlotData->Id())) continue;

        lPlotData->RefreshData();

        // it is check that record in DWG exist (or open for edit)
        if (aTrueForEdit) {

            int lFoundStatus = 0; // not found
            PlotAndHistoryData lPlotForRemove;

            foreach(PlotAndHistoryData lCheckPlot, lPlotForChecking) {
                // id is equal
                if (lCheckPlot.first->Id() == lPlotData->Id()){
                    // at least one of this was history
                    if (lCheckPlot.second || lPlotHistoryData) {
                        // don't open for edit, skip both
                        lFoundStatus = 1;
                        lPlotForRemove = lCheckPlot;
                        break;
                    } else {
                        // no history data, it is duplicate, skip it
                        // don't know how can it be
                        lFoundStatus = 2;
                        break;
                    }
                }
            }

            if (lFoundStatus == 2) continue; // this IdPlot already in list, no history

            if (aTrueForEdit) {
                if (lFoundStatus == 1) {
//                    // this plot id already was in list, so we have undefined history id
//                    // remove from open list
//                    for (int j = lDataForAcad.ListConst().length() - 1; j >= 0; j--) {
//                        if (lDataForAcad.ListConst().at(j)->VEIdPlot() == lPlotData->Id()) {
//                            delete lDataForAcad.ListConst().at(j);
//                            lDataForAcad.ListRef().removeAt(j);
//                        }
//                    }
                    // remove from checking list
                    lPlotForChecking.removeAll(lPlotForRemove);
                    // skip this IdPlot
                    lIdPlotForSkipping.append(lPlotData->Id());

                    lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": "
                                    + tr("two or more history records selected"));
                    continue;
                }
            }
            lPlotForChecking.append(qMakePair(lPlotData, lPlotHistoryData));
        }
    }
    // --------------------------

    for (i = 0; i < lSelected.length(); i++) {
        PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(lSelected[i]);

        PlotHistoryData * lPlotHistoryData = lItem->PlotHistoryRef();
        PlotData * lPlotData;
        bool lSkipWarning = false;
        if (!(lPlotData = lItem->PlotRef())) {
            if (lItem->parent()) lItem = static_cast<PlotListTreeItem *>(lItem->parent());
            lPlotData = lItem->PlotRef();
        }

        if (lIdPlotForSkipping.contains(lPlotData->Id())) continue;

        lPlotData->InitIdDwgMax();

        // it is check that record in DWG exist (or open for edit)
        if (!lPlotHistoryData && lPlotData->IdDwgMax()
                || lPlotHistoryData && lPlotHistoryData->Id()
                || aTrueForEdit) {

            if (aTrueForEdit) {
                // check edit_na
                if (lPlotData->EditNA()) {
                    QString lDocStr = QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": "
                            + tr("editing is not available");
                    if (!lPlotData->SentDateConst().isNull()) {
                        lDocStr += ". " + tr("Document sent to customer");
                    }
                    lSkipped.append(lDocStr);
                    continue;
                }

                // check edit status
                lPlotData->InitEditStatus();
                lItem->ShowEditStatus();

                switch (lPlotData->ES()) {
                case PlotData::PESEditing:
                    // skipped - editing now
                    lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(lPlotData->ESUserConst()));
                    continue;
                    break;
                case PlotData::PESError:
                    if (QMessageBox::question(this, tr("Opening documents for editing"), tr("It was error while editing document")
                                              + " " + QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + " "
                                              + tr("by user") + " " + gUsers->GetName(lPlotData->ESUserConst()) + "\n\n"
                                              + tr("Want to edit this document?")) != QMessageBox::Yes) {
                        continue;
                    } else {
                        // it is means that user already saw the warning
                        // it is really need, cos some changes must be commited from other machine to database
                        // while Autocad opening on this machine
                        lSkipWarning = true;
                    }
                    break;
                }
            }

            if (!aTrueForEdit
                    && (lPlotData->FileType() < 20 || lPlotData->FileType() > 29)
                    && (!lPlotHistoryData && !lPlotData->DataLength()
                        || lPlotHistoryData && !lPlotHistoryData->DataLength())) {
                // skipped - file size is 0, can't view it
                if (!lPlotHistoryData) {
                    lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("file size is 0"));
                } else {
                    lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + " (" + QString::number(lPlotHistoryData->Num()) + "): " + tr("file size is 0"));
                }
                continue;
            }

            if (aTrueForEdit && lPlotHistoryData && lPlotHistoryData->Num() < lPlotData->DwgVersionMax()) {
                if (lUserEditOld != QMessageBox::YesToAll
                        && lUserEditOld != QMessageBox::NoToAll) {
                    // ask if want edit old version
                    QMessageBox mb(this);
                    //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                    mb.setIcon(QMessageBox::Question);
                    mb.setWindowTitle(tr("Opening documents for editing"));
                    mb.setText(tr("Are you sure you want to edit old version of this document?") + "\n" + QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst());
                    if (lSelected.length() > 1) {
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
                    } else {
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    }

                    lUserEditOld = (QMessageBox::StandardButton) mb.exec();
                }
                if (lUserEditOld == QMessageBox::No
                        || lUserEditOld == QMessageBox::NoToAll) {
                    continue;
                }
            }

            if (aTrueForEdit && !lPlotData->Working()) {
                if (lUserEditNonActive != QMessageBox::YesToAll
                        && lUserEditNonActive != QMessageBox::NoToAll) {
                    // ask if want edit old version
                    QMessageBox mb(this);
                    //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                    mb.setIcon(QMessageBox::Question);
                    mb.setWindowTitle(tr("Opening documents for editing"));
                    mb.setText(tr("Are you sure you want to edit non-active version of this document?") + "\n" + QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst());
                    if (lSelected.length() > 1) {
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
                    } else {
                        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    }

                    lUserEditNonActive = (QMessageBox::StandardButton) mb.exec();
                }
                if (lUserEditNonActive == QMessageBox::No
                        || lUserEditNonActive == QMessageBox::NoToAll) {
                    continue;
                }
            }

            lPlotForChecking.append(qMakePair(lPlotData, lPlotHistoryData));

            if ((lPlotData->FileType() < 20 || lPlotData->FileType() > 29) && lPlotData->ExtensionConst().toLower() == "dwg") {
                lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, lPlotData->Id(), lPlotHistoryData?lPlotHistoryData->Id():0, 0, lSkipWarning));
            } else {
                // view/edit non-acad document
                if (lPlotHistoryData) {
                    gSettings->DoOpenNonDwg(lPlotHistoryData->Id(), 2 /*id_dwg*/, aTrueForEdit?1:0, "");
                } else {
                    gSettings->DoOpenNonDwg(lPlotData->Id(), 1 /*id_plot*/, aTrueForEdit?1:0, "");
                }
            }
        } else {
            // view and no file loaded
            lSkipped.append(lPlotData->CodeSheetConst() + " - " + tr("no file loaded"));
        }

    }

    if (!lSkipped.isEmpty()) {
        QMessageBox mb(this);
        //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
        mb.setIcon(QMessageBox::Critical);
        if (aTrueForEdit) {
            mb.setWindowTitle(tr("Opening documents for editing"));
        } else {
            mb.setWindowTitle(tr("Opening documents for viewing"));
        }
        if (lSkipped.length() == 1) {
            mb.setText(lSkipped.at(0));
        } else {
            mb.setText(tr("Some documents skipped"));
            mb.setDetailedText(lSkipped.join("\n"));
        }
        mb.addButton(QMessageBox::Ok);

        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        mb.exec();
    }

    if (!lDataForAcad.ListConst().isEmpty()) {
        gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
    }
}

void PlotListTree::DoAuditPurge() {
    QStringList lSkipped;
    MainDataForCopyToAcad lDataForAcad(gSettings->AuditPurge.PurgeRegapps,
                                       gSettings->AuditPurge.PurgeAll,
                                       gSettings->AuditPurge.ExplodeProxy,
                                       gSettings->AuditPurge.RemoveProxy,
                                       gSettings->AuditPurge.Audit, 0);
    QList<QTreeWidgetItem *> lSelected  = selectedItems();

    for (int i = 0; i < lSelected.length(); i++) {
        PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(lSelected[i]);

        PlotData * lPlotData;
        if (!(lPlotData = lItem->PlotRef())) continue;

        lPlotData->InitIdDwgMax();

        // it is check that record in DWG exist (or open for edit)
        if (lPlotData->IdDwgMax()) {
            // check edit_na
            if (lPlotData->EditNA()) {
                QString lDocStr = QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": "
                        + tr("editing is not available");
                if (!lPlotData->SentDateConst().isNull()) {
                    lDocStr += ". " + tr("Document sent to customer");
                }
                lSkipped.append(lDocStr);
                continue;
            }

            if ((lPlotData->FileType() < 20 || lPlotData->FileType() > 29)
                    && lPlotData->ExtensionConst().toLower() != "dwg") {
                // skipped - non dwg
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("not AutoCAD drawing"));
                continue;
            }

            if ((lPlotData->FileType() < 20 || lPlotData->FileType() > 29)
                    && !lPlotData->DataLength()) {
                // skipped - file size is 0, can't audit it
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("file size is 0"));
                continue;
            }

            // check edit status
            lPlotData->InitEditStatus();
            lItem->ShowEditStatus();

            if (lPlotData->ES() == PlotData::PESEditing) {
                // skipped - editing now
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(lPlotData->ESUserConst()));
                continue;
            }

            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eAP, lPlotData->Id()));
        } else {
            // view and no file loaded
            lSkipped.append(lPlotData->CodeSheetConst() + " - " + tr("no file loaded"));
        }

    }
    if (!lSkipped.isEmpty()) {
        QMessageBox mb(this);
        //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(tr("Opening documents for audit & purge"));
        if (lSkipped.length() == 1) {
            mb.setText(lSkipped.at(0));
        } else {
            mb.setText(tr("Some documents skipped"));
            mb.setDetailedText(lSkipped.join("\n"));
        }
        mb.addButton(QMessageBox::Ok);

        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(1000, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        mb.exec();
    }

    if (!lDataForAcad.ListConst().isEmpty()) {
        gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
    }
}

void PlotListTree::DoPublish(bool aJustListMaatz) {
    QStringList lSkipped;
    MainDataForCopyToAcad lDataForAcad(aJustListMaatz?false:gSettings->Publish.PDF, aJustListMaatz?false:gSettings->Publish.DWF,
                                       aJustListMaatz?false:gSettings->Publish.PLT, gSettings->Publish.DontScale,
                                       gSettings->Publish.UseVersion,
                                       gSettings->Publish.CTBType, gSettings->Publish.CTBName,
                                       gSettings->Publish.PlotterName, gSettings->Publish.OutDir, winId());
    QList<QTreeWidgetItem *> lSelected  = selectedItems();

    QMap<PlotData *, quint64> lSelLayouts;

    for (int i = 0; i < lSelected.length(); i++) {
        PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(lSelected[i]);

        PlotData * lPlotData;

        if (lItem->DwgLayoutConst()
                && static_cast<PlotListTreeItem *>(lItem->parent())
                && (lPlotData = static_cast<PlotListTreeItem *>(lItem->parent())->PlotRef())) {

            lPlotData->InitIdDwgMax();

            // it is check that record in DWG exist (or open for edit)
            if (lPlotData->IdDwgMax()) {
                QMap<PlotData *, quint64>::iterator itr = lSelLayouts.find(lPlotData);
                if (itr != lSelLayouts.end() && itr.key() == lPlotData) {
                    itr.value() |= ((quint64) 1) << (lItem->DwgLayoutConst()->Num() - 1);
                } else {
                    lSelLayouts.insert(lPlotData, ((quint64) 1) << (lItem->DwgLayoutConst()->Num() - 1));
                }
            } else {
                // view and no file loaded
                lSkipped.append(lPlotData->CodeSheetConst() + " - " + tr("no file loaded"));
            }

            continue;
        }

        if (!(lPlotData = lItem->PlotRef())) continue;

        lPlotData->InitIdDwgMax();

        // it is check that record in DWG exist (or open for edit)
        if (lPlotData->IdDwgMax()) {
            // need not edit, yaya
//            // check edit_na
//            if (lPlotData->EditNA()) {
//                QString lDocStr = QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": "
//                        + tr("editing is not available");
//                if (!lPlotData->SentDateConst().isNull()) {
//                    lDocStr += ". " + tr("Document sent to customer");
//                }
//                lSkipped.append(lDocStr);
//                continue;
//            }

            if ((lPlotData->FileType() < 20 || lPlotData->FileType() > 29)
                    && lPlotData->ExtensionConst().toLower() != "dwg") {
                // skipped - non dwg
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("not AutoCAD drawing"));
                continue;
            }

            if ((lPlotData->FileType() < 20 || lPlotData->FileType() > 29)
                    && !lPlotData->DataLength()) {
                // skipped - file size is 0, can't audit it
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("file size is 0"));
                continue;
            }

            // check edit status
            lPlotData->InitEditStatus();
            lItem->ShowEditStatus();

            if (lPlotData->ES() == PlotData::PESEditing) {
                // skipped - editing now
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(lPlotData->ESUserConst()));
                continue;
            }

            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::ePUB, lPlotData->Id(), 0, (quint64) 0));
        } else {
            // view and no file loaded
            lSkipped.append(lPlotData->CodeSheetConst() + " - " + tr("no file loaded"));
        }

    }

    QMap<PlotData *, quint64>::const_iterator itr = lSelLayouts.constBegin();
    while (itr != lSelLayouts.constEnd()) {
        lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::ePUB, itr.key()->Id(), 0, itr.value()));
        itr++;
    }

    if (!lSkipped.isEmpty()) {
        QMessageBox mb(this);
        //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(tr("Documents"));
        if (lSkipped.length() == 1) {
            mb.setText(lSkipped.at(0));
        } else {
            mb.setText(tr("Some documents skipped"));
            mb.setDetailedText(lSkipped.join("\n"));
        }
        mb.addButton(QMessageBox::Ok);

        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        mb.exec();
    }

    if (!lDataForAcad.ListConst().isEmpty()) {
        gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
//    } else {
//        if (lSelected.length() > 1)
//            QMessageBox::critical(this, tr("Documents"), tr("List of documents is empty"));
    }
}

void PlotListTree::DoReplaceText(const ReplaceTextDlg *aReplaceTextDlg) {
    QStringList lSkipped;
    MainDataForCopyToAcad lDataForAcad(aReplaceTextDlg->FindTextCopy(), aReplaceTextDlg->ReplaceWithCopy(),
                                       aReplaceTextDlg->MoveType(), aReplaceTextDlg->DX(), aReplaceTextDlg->DY());

    QList<QTreeWidgetItem *> lSelected  = selectedItems();

    for (int i = 0; i < lSelected.length(); i++) {
        PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(lSelected[i]);

        PlotData * lPlotData;
        if (!(lPlotData = lItem->PlotRef())) continue;

        lPlotData->InitIdDwgMax();

        // it is check that record in DWG exist (or open for edit)
        if (lPlotData->IdDwgMax()) {
            // check edit_na
            if (lPlotData->EditNA()) {
                QString lDocStr = QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": "
                        + tr("editing is not available");
                if (!lPlotData->SentDateConst().isNull()) {
                    lDocStr += ". " + tr("Document sent to customer");
                }
                lSkipped.append(lDocStr);
                continue;
            }

            if ((lPlotData->FileType() < 20 || lPlotData->FileType() > 29)
                    && lPlotData->ExtensionConst().toLower() != "dwg") {
                // skipped - non dwg
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("not AutoCAD drawing"));
                continue;
            }

            if ((lPlotData->FileType() < 20 || lPlotData->FileType() > 29)
                    && !lPlotData->DataLength()) {
                // skipped - file size is 0, can't audit it
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("file size is 0"));
                continue;
            }

            // check edit status
            lPlotData->InitEditStatus();
            lItem->ShowEditStatus();

            if (lPlotData->ES() == PlotData::PESEditing) {
                // skipped - editing now
                lSkipped.append(QString::number(lPlotData->Id()) + " - " + lPlotData->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(lPlotData->ESUserConst()));
                continue;
            }

            lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eRT, lPlotData->Id()));
        } else {
            // view and no file loaded
            lSkipped.append(lPlotData->CodeSheetConst() + " - " + tr("no file loaded"));
        }

    }
    if (!lSkipped.isEmpty()) {
        QMessageBox mb(this);
        //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(tr("Opening documents for text replace"));
        if (lSkipped.length() == 1) {
            mb.setText(lSkipped.at(0));
        } else {
            mb.setText(tr("Some documents skipped"));
            mb.setDetailedText(lSkipped.join("\n"));
        }
        mb.addButton(QMessageBox::Ok);

        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(1000, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        mb.exec();
    }

    if (!lDataForAcad.ListConst().isEmpty()) {
        gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD
    }
}

void PlotListTree::NewPlot(ProjectData *aProjectData, const TreeDataRecord *aTreeData, const QString &aComplect, PlotData *aPlotDataFrom,
                           PlotHistoryData * aPlotHistoryDataFrom) {
    PlotData *lPlotData;
    if (lPlotData = gMainWindow->NewPlot(aProjectData, aTreeData, aComplect, aPlotDataFrom, aPlotHistoryDataFrom)) {
        gProjects->EmitPlotListBeforeUpdate(aProjectData->Id());
        aProjectData->PlotListRef().append(lPlotData);
        SetSelectedPlotId(lPlotData->Id());
        gProjects->EmitPlotListNeedUpdate(aProjectData->Id());
    }
}

void PlotListTree::SaveDocuments() {
    QStringList lSkipped;
    QList<QTreeWidgetItem *> lSelected  = selectedItems();
    QList<PlotAndHistoryData> lPlotForSave;

    for (int i = 0; i < lSelected.length(); i++) {
        PlotListTreeItem *lItem = static_cast<PlotListTreeItem *>(lSelected[i]);

        PlotHistoryData * lPlotHistoryData = lItem->PlotHistoryRef();
        PlotData * lPlotData;
        if (!(lPlotData = lItem->PlotRef())) {
            if (lItem->parent()) lItem = static_cast<PlotListTreeItem *>(lItem->parent());
            lPlotData = lItem->PlotRef();
        }

        if (!lPlotData) continue;

        lPlotData->InitIdDwgMax();

        if (!lPlotHistoryData && lPlotData->IdDwgMax()
                || lPlotHistoryData && lPlotHistoryData->Id()) {

            int lIsFound = false; // not found

            foreach(PlotAndHistoryData lCheckPlot, lPlotForSave) {
                // id is equal
                if (lCheckPlot.first->Id() == lPlotData->Id()
                        && (!lCheckPlot.second && !lPlotHistoryData
                            || lCheckPlot.second && lPlotHistoryData
                                && lCheckPlot.second->Id() == lPlotHistoryData->Id()
                            || lCheckPlot.second && !lPlotHistoryData
                                && lCheckPlot.second->Id() == lPlotData->IdDwgMax()
                            || !lCheckPlot.second && lPlotHistoryData
                                && lCheckPlot.first->IdDwgMax() == lPlotHistoryData->Id())){
                    lIsFound = true;
                    break;
                }
            }

            if (lIsFound) continue; // this IdPlot/hist already in list

            lPlotForSave.append(qMakePair(lPlotData, lPlotHistoryData));
        } else {
            lSkipped.append(lPlotData->CodeSheetConst() + " - " + tr("no file loaded"));
        }
    }

    if (!lSkipped.isEmpty()) {
        QMessageBox mb(this);
        //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(tr("Saving documents"));
        if (lSkipped.length() == 1) {
            mb.setText(lSkipped.at(0));
        } else {
            mb.setText(tr("Some documents skipped"));
            mb.setDetailedText(lSkipped.join("\n"));
        }
        mb.addButton(QMessageBox::Ok);

        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) mb.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        mb.exec();
    }

    SaveDialog lSaveDlg(this);

    if (!lPlotForSave.isEmpty()) {
        foreach (PlotAndHistoryData lPlot, lPlotForSave) {
            lSaveDlg.AddDocument(lPlot.first->Id(), lPlot.second?lPlot.second->Id():0);
        }
        lSaveDlg.exec();
    }
}

void PlotListTree::SaveAddFiles() {
    QList <QTreeWidgetItem *> selected = selectedItems();

    bool lIsFile;
    QFileDialog dlg;
    PlotListTreeItem * lItem;
    const PlotAddFileData * lAddFile;

    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setDirectory(gSettings->SaveFiles.LastDir);

    if (selected.count() == 1) {
        lItem = static_cast<PlotListTreeItem *>(selected.at(0));
        lAddFile = lItem->PlotAddFileConst();

        dlg.setFileMode(QFileDialog::AnyFile);
        dlg.selectFile(lAddFile->NameConst());

        lIsFile = true;
    } else {
        dlg.setFileMode(QFileDialog::DirectoryOnly);

        // recommended - not work as usual
        //dlg.setFileMode(QFileDialog::Directory);
        //dlg.setOption(QFileDialog::ShowDirsOnly, true);

        lIsFile = false;
    }
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        if (files.length() == 1) {
            if (lIsFile) {
                gSettings->SaveFiles.LastDir = files.at(0).left(files.at(0).lastIndexOf('/'));

                // no additional checking - we can save it. right?

                QFile file(files.at(0));
                if (file.open(QFile::WriteOnly)) {
                    file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, lAddFile->IdLob()));
                    file.close();
                    gFileUtils->SetFileTime(file.fileName(), lAddFile->FTimeConst());
                }
            } else {
                gSettings->SaveFiles.LastDir = files.at(0);

                QList<int> lESGetted;
                QMessageBox::StandardButton lOverwrite = QMessageBox::No;

                for (int i = 0; i < selected.length(); i++) {
                    lItem = static_cast<PlotListTreeItem *>(selected.at(i));

                    if (lAddFile = lItem->PlotAddFileConst()) {
                        PlotData * lPlot = lItem->PlotRef();
                        if (!lPlot) lPlot = static_cast<PlotListTreeItem *>(lItem->parent())->PlotRef();
                        if (!lESGetted.contains(lPlot->Id())) {
                            lPlot->InitEditStatus();
                            lESGetted.append(lPlot->Id());
                        }
                        if (lPlot->ES() == PlotData::PESEditing) {
                            continue;
                        }


                        QFile file(files.at(0) + "/" + lAddFile->NameConst());

                        if (file.exists()) {
                            if (lOverwrite == QMessageBox::NoToAll) continue;
                            if (lOverwrite != QMessageBox::YesToAll) {
                                // check by sha256

                                QMessageBox mb(this);
                                //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                                mb.setIcon(QMessageBox::Question);
                                mb.setWindowTitle(tr("Saving additional files"));
                                mb.setText(tr("File") + "\n" + file.fileName() + "\n" + tr("already exists.\n\nOverwrite?"));
                                mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);

                                lOverwrite = (QMessageBox::StandardButton) mb.exec();
                                if (lOverwrite == QMessageBox::NoToAll
                                        || lOverwrite == QMessageBox::No) continue;
                            }
                        } else {
                            lOverwrite = QMessageBox::Yes;
                        }

                        if ((lOverwrite == QMessageBox::Yes || lOverwrite == QMessageBox::YesToAll)) {
                            if (file.open(QFile::WriteOnly)) {
                                file.write(gBlobMemCache->GetData(BlobMemCache::Dwg, lAddFile->IdLob()));
                                file.close();
                                gFileUtils->SetFileTime(file.fileName(), lAddFile->FTimeConst());
                            }
                        } else {
                            gLogger->ShowError(this, tr("Saving additional files"), tr("Error creating file") + ":\r\n" + file.fileName() + "\r\n" + tr("Error") +": " + file.errorString());
                        }
                    }
                }
            }
        }
    }
}

// it is simple realisation, like in old version; just temporary
void PlotListTree::LoadOneDocument(PlotData *aPlot) {
    if (!aPlot) return;

    aPlot->InitEditStatus();
    if (aPlot->ES() == PlotData::PESEditing) {
        // skipped - editing now
        QMessageBox::critical(this, tr("Loading document"),
                              QString::number(aPlot->Id()) + " - " + aPlot->CodeSheetConst() + ": " + tr("now editing by") + " " + gUsers->GetName(aPlot->ESUserConst()));
        return;
    }

    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setDirectory(gSettings->LoadFiles.LastDir);

    FileType * lFileType = aPlot->ActualFileType();
    if (!lFileType) {
        gLogger->ShowError(this, tr("Loading document"), tr("Can't find filetype"));
        return;
    }

    if (!lFileType->FileMasks_QTConst().isEmpty()) dlg.setNameFilters(lFileType->FileMasks_QTConst().split(';'));
    if (lFileType->LoadMode() == 3) {
        dlg.setFileMode(QFileDialog::DirectoryOnly);
    } else {
        dlg.setFileMode(QFileDialog::ExistingFile);
    }

    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        if (files.length() == 1) {
            if (lFileType->LoadMode() != 3) {
                // file selected, save dir
                gSettings->LoadFiles.LastDir = files.at(0).left(files.at(0).lastIndexOf('/'));
            } else {
                // dir selected, save it
                gSettings->LoadFiles.LastDir = files.at(0);
            }

            qint64 lOrigFileSize;

            XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(files.at(0)));

            if (!gFileUtils->InitDataForLoad(lFileType->LoadMode() != 3, *lXchgFileData, lOrigFileSize)) {
                delete lXchgFileData;
                return;
            }

            if (lFileType->LoadMode() == 3) {
                if (lXchgFileData->AddFilesConst().isEmpty()) {
                    QMessageBox::critical(this, tr("Loading document"), tr("No files exist in this directory!"));
                    delete lXchgFileData;
                    return;
                }
            }

            QList<tPairIntIntString> lExistingIds;

            if (!gOracle->CollectAlreadyLoaded(lXchgFileData->HashOrigConst(), lExistingIds)) {
                delete lXchgFileData;
                return;
            }

            if (!lExistingIds.isEmpty()) {
                QMessageBox mb(this);
                mb.setWindowTitle(tr("Loading document"));
                mb.setIcon(QMessageBox::Question);
                mb.setText(tr("File already loaded in Projects Base"));
                mb.addButton(tr("Abort loading and goto existing document"), QMessageBox::YesRole);
                mb.addButton(tr("Ignore and continue loading"), QMessageBox::NoRole);
                mb.setWindowFlags((mb.windowFlags() & ~(Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)) | Qt::CustomizeWindowHint);

                QString lDetails;
                foreach (tPairIntIntString lExistingId, lExistingIds) {
                    lDetails += QString::number(lExistingId.first.first) + "/" + QString::number(lExistingId.first.second) + " - " + lExistingId.second + "\n";
                }
                mb.setDetailedText(lDetails);

                // motherfucker motherfucker
                QSpacerItem * horizontalSpacer = new QSpacerItem(1000, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                QGridLayout * layout = (QGridLayout *) mb.layout();
                layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

                // the return values is button number
                if (!mb.exec()) {
                    // "YES"
                    gMainWindow->ShowPlotLists(lExistingIds);
                    delete lXchgFileData;
                    return;
                }
            }

            //QString lHashForProcessedStr = lOrigFileHash; // it is by default

            qDeleteAll(mFiles);
            mFiles.clear();
            mFiles.append(lXchgFileData);
            if (lFileType->LoadMode() != 3
                    && lXchgFileData->FileInfoOrigConst().suffix().toLower() == "dwg") {
                // AutoCAD drawing
                if (!ProcessDwgsForLoad(lpClearAnnoScales | lpPurgeRegapps | lpExplodeAllProxies | lpRemoveAllProxies | lpAudit, 0, 0, 0, 0, "", "", winId())) return;
            }

            if (db.transaction()) {
                //gProjects->EmitProjectBeforeUpdate(aPlot->IdProject());

                aPlot->InitIdDwgMax();
                quint64 lMainIdDwg;
                bool lIsOk = PlotData::LOADFROMFILE(lFileType->LoadMode() != 3, aPlot->Id(), lMainIdDwg, aPlot->IdDwgMax(), aPlot->DwgVersionMax(),
                                                    lXchgFileData->FileInfoOrigConst(), lOrigFileSize, lXchgFileData->HashOrigConst(),
                                                    *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                                    lXchgFileData->DwgLayoutsConst(), lXchgFileData->AddFilesRef(), false, this)
                        && DwgData::CopyDwgXrefs(aPlot->IdDwgMax(), lMainIdDwg);

                if (lIsOk) {
                    if (!db.commit()) {
                        gLogger->ShowSqlError(this, tr("Loading document"), tr("Can't commit"), db);
                        lIsOk = false;
                    } else {
                        if (lFileType->LoadMode() != 3) {
                            gSettings->SaveToLocalCache(lMainIdDwg, *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst());
                        }
                        for (int i = 0; i < lXchgFileData->AddFilesConst().length(); i++) {
                            gSettings->SaveToLocalCache(lXchgFileData->AddFilesConst().at(i)->IdDwg(),
                                                        *lXchgFileData->AddFilesConst().at(i)->BinaryDataConst(), lXchgFileData->AddFilesConst().at(i)->HashPrcdConst());
                        }

                        aPlot->RefreshData();
                        //gProjects->EmitProjectNeedUpdate(aPlot->IdProject());
                    }
                }
                if (!lIsOk) {
                    db.rollback();
                }
            } else {
                gLogger->ShowSqlError(this, tr("Loading document"), tr("Can't start transaction"), db);
            }
            qDeleteAll(mFiles);
            mFiles.clear();
        }
    }
}

void PlotListTree::StringChangedTimer() {
    int i;
    bool lIsOk = true, lReplace = false;
    int lCurNum, lNumStart, lNumLen;
    QString lNumStr, lCurStr;

    if (mStrFromEdit.contains('#')) {
        lNumLen = 1;
        lNumStart = mStrFromEdit.indexOf('#');
        for (i = mStrFromEdit.indexOf('#') + 1; i < mStrFromEdit.length(); i++) {
            if (mStrFromEdit[i] != '#') break;
            lNumLen++;
        }

        QInputDialog lInput(this);

        lInput.setWindowFlags(lInput.windowFlags() & ~Qt::WindowContextHelpButtonHint);

        //lInput.setOptions();
        lInput.setWindowTitle(tr("Counter for #"));
        lInput.setLabelText(tr("Enter start value\nCancel - do not count, use # itself"));
        lInput.setInputMode(QInputDialog::IntInput);
        lInput.setIntMinimum(1);
        lInput.setIntMaximum(1000000);
        if (lInput.exec() == QDialog::Accepted) {
            lReplace = true;
            lCurNum = lInput.intValue();
        }
    }

    if (!(mPLTFlags & PLTNewVersion)
            && !db.transaction()) {
        gLogger->ShowSqlError(this, tr("Documents list"), tr("Can't start transaction"), db);
        lIsOk = false;
    } else {
        QList <QVariant> lValues;
        QList <PlotData *> lPlots;
        QList<tPairIntString> lNewComplects;

        // it is list of pair - "layout data" and parent PlotListTreeItem
        typedef QPair<DwgLayoutData *, PlotListTreeItem *> tPairForLayouts;
        QList <tPairForLayouts> lLayouts;


        // setting new values
        for (i = 0; i < mEditItems.length(); i++) {
            switch (mEditType) {
            case 1:
                PlotData * lPlot;
                if (lPlot = static_cast<PlotListTreeItem *>(mEditItems.at(i))->PlotRef()) {
                    if (!(mPLTFlags & PLTNewVersion)) lPlots.append(lPlot);

                    if (!lReplace) {
                        lCurStr = mStrFromEdit;
                    } else {
                        lCurStr = mStrFromEdit;
                        lNumStr = QString::number(lCurNum);
                        while (lNumStr.length() < lNumLen) lNumStr.prepend('0');
                        lCurStr.replace(lNumStart, lNumLen, lNumStr);
                    }

                    if (mEditColumn == mCols[PlotListTree::colCode]) {
                        if (lPlot->CodeConst() != lCurStr) {
                            lPlot->CodeRef() = lCurStr;
//                            lValues.clear();
//                            lValues.append(lCurStr);
//                            lIsOk = lPlot->LoadVersions();
//                            lPlot->SetPropWithVersions(false, false, PlotData::MATCode, lValues);
                        }
                    } else if (mEditColumn == mCols[PlotListTree::colSheet]) {
                        if (lPlot->SheetConst() != lCurStr) {
                            lPlot->SheetRef() = lCurStr;
//                            lValues.clear();
//                            lValues.append(lCurStr);
//                            lIsOk = lPlot->LoadVersions();
//                            lPlot->SetPropWithVersions(false, false, PlotData::MATSheet, lValues);
                        }
                    } else if (mEditColumn == mCols[PlotListTree::colVersionInt]) {
                        if (lPlot->VersionIntConst() != lCurStr) {
                            lPlot->VersionIntRef() = lCurStr;
                        }
                    } else if (mEditColumn == mCols[PlotListTree::colVersionExt]) {
                        if (lPlot->VersionExtConst() != lCurStr) {
                            lPlot->VersionExtRef() = lCurStr;
                        }
                    } else if (mEditColumn == mCols[PlotListTree::colNameTop]) {
                        if (lPlot->NameTopConst() != lCurStr) {
                            lValues.clear();
                            lValues.append(lCurStr);
                            lIsOk = lPlot->LoadVersions();
                            lPlot->SetPropWithVersions(false, false, PlotData::MATNameTop, lValues);
                        }
                    } else if (mEditColumn == mCols[PlotListTree::colNameBottom]) {
                        if (lPlot->NameConst() != lCurStr) {
                            lValues.clear();
                            lValues.append(lCurStr);
                            lIsOk = lPlot->LoadVersions();
                            lPlot->SetPropWithVersions(false, false, PlotData::MATNameBottom, lValues);
                        }
                    } else if (mEditColumn == mCols[PlotListTree::colComments]) {
                        if (lPlot->NotesConst() != lCurStr) {
                            lPlot->NotesRef() = lCurStr;
                        }
                    } else if (mEditColumn == mCols[PlotListTree::colSection]) {
                        if (lPlot->SectionConst() != lCurStr) {
                            lPlot->SectionRef() = lCurStr;

                            tPairIntString lPair = qMakePair(lPlot->IdProject(), lCurStr);
                            if (!lNewComplects.contains(lPair)) lNewComplects.append(lPair);
                        }
                    // for new versions
                    } else if (mPLTFlags & PLTNewVersion) {
                        if (mEditColumn == mCols[PlotListTree::colVersionIntNew]) {
                            mEditItems.at(i)->setText(mEditColumn, lCurStr);
                        }
                        if (mEditColumn == mCols[PlotListTree::colVersionExtNew]) {
                            QString lOldVal = mEditItems.at(i)->text(mEditColumn);
                            QString lCode = mEditItems.at(i)->text(mCols[PlotListTree::colCode]);
                            mEditItems.at(i)->setText(mEditColumn, lCurStr);
                            lPlot->SetPropWithCodeForming(PlotData::PPWCVersionExt, lOldVal, lCurStr, lCode);
                            mEditItems.at(i)->setText(mCols[PlotListTree::colCode], lCode);
                        }
                    }
                }
                break;
            case 2:
                // layout
                DwgLayoutData * lLayoutData;
                if ((lLayoutData = static_cast<PlotListTreeItem *>(mEditItems.at(i))->DwgLayoutRef())
                        && mEditItems.at(i)->parent()) {
                    lLayouts.append(qMakePair(lLayoutData, static_cast<PlotListTreeItem *>(mEditItems.at(i)->parent())));

                    if (!lReplace) {
                        lCurStr = mStrFromEdit;
                    } else {
                        lCurStr = mStrFromEdit;
                        lNumStr = QString::number(lCurNum);
                        while (lNumStr.length() < lNumLen) lNumStr.prepend('0');
                        lCurStr.replace(lNumStart, lNumLen, lNumStr);
                    }

                    if (mEditColumn == mCols[PlotListTree::colCode]) {
                        // name of layout (in AutoCAD tab)
                        //if (lLayoutData->NameConst() != lCurStr) {
                            lLayoutData->NameRef() = lCurStr;
                        //}
                    } else if (mEditColumn == mCols[PlotListTree::colSheet]) {
                        // sheet number
                        //if (lLayoutData->SheetConst() != lCurStr) {
                            lLayoutData->SheetRef() = lCurStr;
                        //}
                    } else if (mEditColumn == mCols[PlotListTree::colNameBottom]) {
                        // layout name in stamp
                        //if (lLayoutData->BottomConst() != lCurStr) {
                            lLayoutData->BottomRef() = lCurStr;
                        //}
                    }
                }
                break;
            }
            if (!lIsOk) break;
            if (lReplace) lCurNum++;
        }

        if (!(mPLTFlags & PLTNewVersion)) {
            if (lIsOk) {
                QString lError;
                QStringList lErrors;

                // checking for now duplicates
                if (!lPlots.isEmpty()
                        && mEditColumn != mCols[PlotListTree::colComments]
                        && mEditColumn != mCols[PlotListTree::colSection]) {
                    foreach (PlotData * lPlotChanged, lPlots) {
                        ProjectData * lProject = gProjects->FindByIdProject(lPlotChanged->IdProject());
                        foreach (PlotData * lPlotAll, lProject->PlotListConst()) {
                            if (lPlotChanged->IdCommon() != lPlotAll->IdCommon()) {
                                lError.clear();
                                if ((mEditColumn == mCols[PlotListTree::colCode]
                                     || mEditColumn == mCols[PlotListTree::colSheet])
                                        && lPlotChanged->CodeConst() == lPlotAll->CodeConst().trimmed()
                                        && lPlotChanged->SheetConst() == lPlotAll->SheetConst().trimmed()) {
                                    if (!lPlotChanged->SheetConst().isEmpty()) {
                                        lError = lPlotChanged->CodeConst() + "-" + lPlotChanged->SheetConst() +": " +tr("code and sheet number is not unique");
                                    } else {
                                        lError = lPlotChanged->CodeConst() + ": " +tr("code and sheet number is not unique!");
                                    }
                                } else if ((mEditColumn == mCols[PlotListTree::colNameTop]
                                            || mEditColumn == mCols[PlotListTree::colNameBottom])
                                           && lPlotChanged->NameTopConst() == lPlotAll->NameTopConst().trimmed()
                                           && lPlotChanged->NameConst() == lPlotAll->NameConst().trimmed()) {
                                    lError = lPlotChanged->NameTopConst() + ". " + lPlotChanged->NameConst() +": " +tr("name is not unique");
                                }
                                if (!lError.isEmpty() && !lErrors.contains(lError)) lErrors.append(lError);
                            }
                        }
                    }
                }

                if (!lLayouts.isEmpty()) {
                    foreach (tPairForLayouts lLayoutChecked, lLayouts) {
                        for (int j = 0; j < lLayoutChecked.second->childCount(); j++) {
                            const DwgLayoutData * lLayoutSecond;
                            if (lLayoutSecond = static_cast<PlotListTreeItem *>(lLayoutChecked.second->child(j))->DwgLayoutConst()) {
                                lError.clear();
                                if (lLayoutChecked.first->Id() != lLayoutSecond->Id()
                                        && lLayoutChecked.first->IdDwg() == lLayoutSecond->IdDwg()) {
                                    if (mEditColumn == mCols[PlotListTree::colCode]
                                            && lLayoutChecked.first->NameConst() == lLayoutSecond->NameConst().trimmed()) {
                                        lError = lLayoutChecked.first->NameConst() +": " +tr("layout short name is not unique");;
                                    } else if (mEditColumn == mCols[PlotListTree::colSheet]
                                               && lLayoutChecked.first->SheetConst() == lLayoutSecond->SheetConst().trimmed()) {
                                        lError = lLayoutChecked.first->NameConst() +": " +tr("sheet number is not unique");;
                                    } else if (mEditColumn == mCols[PlotListTree::colNameBottom]
                                               && lLayoutChecked.first->BottomConst() == lLayoutSecond->BottomConst().trimmed()) {
                                        lError = lLayoutChecked.first->NameConst() +": " +tr("layout name is not unique");;
                                    }
                                }
                                if (!lError.isEmpty() && !lErrors.contains(lError)) lErrors.append(lError);
                            }
                        }
                    }
                }

                if (!lErrors.isEmpty()) {
                    QMessageBox mb(this);
                    //mb.setWindowFlags(mb.windowFlags() | Qt::WindowFlags);
                    mb.setIcon(QMessageBox::Critical);
                    mb.setWindowTitle(tr("Documents list"));
                    mb.setText(tr("Can't do this, some duplicates"));
                    mb.setDetailedText(lErrors.join("\n"));
                    mb.addButton(QMessageBox::Ok);

                    // motherfucker motherfucker
                    QSpacerItem * horizontalSpacer = new QSpacerItem(1000, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                    QGridLayout * layout = (QGridLayout *) mb.layout();
                    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

                    mb.exec();

                    lIsOk = false;
                } else {
                    // saving
                    foreach (PlotData * lPlotChanged, lPlots) {
                        if (!lPlotChanged->SaveDataWithVersions()) {
                            lIsOk = false;
                            break;
                        }
                    }
                    foreach (tPairForLayouts lLayout, lLayouts) {
                        if (!lLayout.first->SaveData()) {
                            lIsOk = false;
                            break;
                        }
                    }
                }
            }

            if (lIsOk) {
                if (!db.commit()) {
                    gLogger->ShowSqlError(this, tr("Documents list"), tr("Can't commit"), db);
                    lIsOk = false;
                }
            }
            if (!lIsOk) {
                db.rollback();
            }

            QList<int> lIdsProject;

            if (gSettings->Common.RereadAfterSave) {
                // version with reload from base
                // doesn't matter, error or success
                foreach (PlotData * lPlotChanged, lPlots) {
                    PlotData * lPlot;
                    if (lPlotChanged->Working()) {
                        lPlot = lPlotChanged;
                    } else {
                        lPlot = gProjects->FindByIdProject(lPlotChanged->IdProject())->GetPlotByIdCommon(lPlotChanged->IdCommon());
                    }
                    lPlot->RefreshData();
                    lPlot->UninitVersions();
                    if (!lIdsProject.contains(lPlot->IdProject())) lIdsProject.append(lPlot->IdProject());
                }

                foreach (tPairForLayouts lLayout, lLayouts) {
                    PlotData * lPlot;
                    if (lPlot = lLayout.second->PlotRef()) {
                        //lPlot->UninitLayouts();
                        if (!lIdsProject.contains(lPlot->IdProject())) lIdsProject.append(lPlot->IdProject());
                    }
                }
            } else {
                // version with just internally commit/rollback edit
                foreach (PlotData * lPlotChanged, lPlots) {
                    if (!lIdsProject.contains(lPlotChanged->IdProject())) lIdsProject.append(lPlotChanged->IdProject());
                    if (lIsOk) {
                        lPlotChanged->CommitEdit();
                    } else {
                        lPlotChanged->RollbackEdit();
                    }
                    if (lPlotChanged->Working()) {
                        foreach (PlotData * lPlotChangedVersion, lPlotChanged->VersionsConst()) {
                            if (lIsOk) {
                                lPlotChangedVersion->CommitEdit();
                            } else {
                                lPlotChangedVersion->RollbackEdit();
                            }
                        }
                    } else {
                        lPlotChanged->UninitVersions();
                    }
                }

                foreach (tPairForLayouts lLayout, lLayouts) {
                    PlotData * lPlot;
                    if (lPlot = lLayout.second->PlotRef()) {
                        if (!lIdsProject.contains(lPlot->IdProject())) lIdsProject.append(lPlot->IdProject());
                    }
                    if (lIsOk) {
                        lLayout.first->CommitEdit();
                    } else {
                        lLayout.first->RollbackEdit();
                    }
                }
            }

            if (lIsOk) {
                if (!lNewComplects.isEmpty()) {
                    foreach (tPairIntString lNewComplect, lNewComplects) {
                        ProjectData * lProject = gProjects->FindByIdProject(lNewComplect.first);
                        if (lProject
                                && !lNewComplect.second.isEmpty()
                                && !lProject->ComplectListConst().contains(lNewComplect.second)) {
                            lProject->ComplectListRef().append(lNewComplect.second);
                            std::sort(lProject->ComplectListRef().begin(), lProject->ComplectListRef().end(), CmpStringsWithNumbersNoCase);
                            if (!lIdsProject.contains(lProject->Id())) lIdsProject.append(lProject->Id());
                        }
                    }
                }
            }

            foreach (i, lIdsProject) {
                gProjects->EmitPlotListBeforeUpdate(i);
                gProjects->EmitPlotListNeedUpdate(i);
            }
        }
    }
}

void PlotListTree::StringChanged(QWidget *editor) {
    // we can't show any QMessages here, because the signal fire second time and then application crashs
    if (mEditType < 1) return;
    bool lIsData = false;
    if (qobject_cast<QLineEdit *> (editor)) {
        mStrFromEdit = qobject_cast<QLineEdit *> (editor)->text().trimmed();
        lIsData = true;
    } else if (qobject_cast<QComboBox *> (editor)) {
        mStrFromEdit = qobject_cast<QComboBox *> (editor)->currentText().trimmed();
        lIsData = true;
    } else if (qobject_cast<QPlainTextEdit *> (editor)) {
        mStrFromEdit = qobject_cast<QPlainTextEdit *> (editor)->toPlainText().trimmed();
        lIsData = true;
    }
    if (lIsData
            && mStrFromEdit != mEditOldValue) {
        QTimer::singleShot(0, this, SLOT(StringChangedTimer()));
    }
}

//-------------------------------------------------------------------------------------------------------------
PlotListTreeItem::PlotListTreeItem(PlotListTree *aPlotListTree, PlotData * aPlotData, PlotHistoryData *aPlotHistoryData, int aMaxDwgVersion, bool aIsSecondLevel) :
    QTreeWidgetItem(NULL),
    mIsOwner(false),
    mPlotListTree(aPlotListTree), mPlotData(aPlotData), mMaxDwgVersion(aMaxDwgVersion),
    mDwgLayout(NULL), mDwgLayoutBlock(NULL), mPlotHistoryData(aPlotHistoryData),
    mPlotAddFileData(NULL)
{
    // plot data can be edited by default
    setFlags(flags() | Qt::ItemIsEditable);
    setFlags(flags() & ~(Qt::ItemIsUserCheckable));

    if (!aIsSecondLevel) {
        if (gSettings->DocumentTree.DragDrop) {
            setFlags(flags() | Qt::ItemIsDragEnabled);
        } else {
            setFlags(flags() & ~(Qt::ItemIsDragEnabled));
        }
        if (!(aPlotListTree->PLTFlags() & PLTNoColors)
                && gSettings->DocumentTree.UseDocColor) {
            for (int i = 0; i < mPlotListTree->columnCount(); i++) {
                if (!(aPlotListTree->PLTFlags() & PLTNewVersion)
                        || i != aPlotListTree->Cols()[PlotListTree::colVersionIntNew]
                        && i != aPlotListTree->Cols()[PlotListTree::colVersionExtNew])
                    setBackground(i, gSettings->DocumentTree.DocColor);
            }
        } else {
            if (aPlotListTree->PLTFlags() & PLTNewVersion) {
                if (aPlotListTree->Cols()[PlotListTree::colVersionIntNew] != -1) {
                    setBackground(aPlotListTree->Cols()[PlotListTree::colVersionIntNew], gSettings->Common.RequiredFieldColor);
                }
                if (aPlotListTree->Cols()[PlotListTree::colVersionExtNew] != -1) {
                    setBackground(aPlotListTree->Cols()[PlotListTree::colVersionExtNew], gSettings->Common.RequiredFieldColor);
                }
            }

        }
        if ((gSettings->DocumentTree.DocFontPlusOne || gSettings->DocumentTree.DocFontBold)
             && !(aPlotListTree->PLTFlags() & PLTNoColors)) {
            QFont lFont = font(0);
            if (gSettings->DocumentTree.DocFontPlusOne) {
                lFont.setPointSize(lFont.pointSize() + 1);
                /*if (lFont.pointSize() != -1)
                    lFont.setPointSize(lFont.pointSize() + 1);
                else if (lFont.pixelSize() != -1)
                    lFont.setPixelSize(lFont.pixelSize() + 1);*/
            }
            if (gSettings->DocumentTree.DocFontBold) lFont.setBold(true);
            for (int i = 0; i < mPlotListTree->columnCount(); i++)
                setFont(i, lFont);
        }
    } else {
        setFlags(flags() & ~(Qt::ItemIsDragEnabled));
        if (!(aPlotListTree->PLTFlags() & PLTNoColors)
                && gSettings->DocumentTree.UseSecondColor)
            for (int i = 0; i < mPlotListTree->columnCount(); i++)
                setBackground(i, gSettings->DocumentTree.SecondColor);
    }

    ShowData();
}

PlotListTreeItem::PlotListTreeItem(PlotListTree *aPlotListTree, DwgLayoutData * aDwgLayout, DwgLayoutBlockData *aDwgLayoutBlock) :
    QTreeWidgetItem(NULL),
    mIsOwner(false),
    mPlotListTree(aPlotListTree), mPlotData(NULL), mMaxDwgVersion(0),
    mDwgLayout(aDwgLayout), mDwgLayoutBlock(aDwgLayoutBlock),
    mPlotHistoryData(NULL),
    mPlotAddFileData(NULL)
{
    // layouts can be edited and can't be dragged or dropped on
    setFlags((flags() | Qt::ItemIsEditable) & ~(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));

    if (gSettings->DocumentTree.UseSecondColor)
        for (int i = 0; i < mPlotListTree->columnCount(); i++)
            setBackground(i, gSettings->DocumentTree.SecondColor);

    ShowData();
}

PlotListTreeItem::PlotListTreeItem(PlotListTree * aPlotListTree, PlotHistoryData * aPlotHistoryData) :
    QTreeWidgetItem(NULL),
    mIsOwner(false),
    mPlotListTree(aPlotListTree), mPlotData(NULL), mMaxDwgVersion(0),
    mDwgLayout(NULL), mDwgLayoutBlock(NULL), mPlotHistoryData(aPlotHistoryData),
    mPlotAddFileData(NULL)
{
    // history can't be dragged, dropped on or edited
    setFlags(flags() & ~(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable));

    if (gSettings->DocumentTree.UseSecondColor)
        for (int i = 0; i < mPlotListTree->columnCount(); i++)
            setBackground(i, gSettings->DocumentTree.SecondColor);

    ShowData();
}

PlotListTreeItem::PlotListTreeItem(PlotListTree * aPlotListTree, PlotAddFileData * aPlotAddFileData) :
    QTreeWidgetItem(NULL),
    mIsOwner(false),
    mPlotListTree(aPlotListTree), mPlotData(NULL), mMaxDwgVersion(0),
    mDwgLayout(NULL), mDwgLayoutBlock(NULL), mPlotHistoryData(NULL),
    mPlotAddFileData(aPlotAddFileData)
{
    // can't be dragged, dropped on or edited
    setFlags(flags() & ~(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable));

    if (gSettings->DocumentTree.UseSecondColor)
        for (int i = 0; i < mPlotListTree->columnCount(); i++)
            setBackground(i, gSettings->DocumentTree.SecondColor);

    ShowData();
}

PlotListTreeItem::~PlotListTreeItem() {
    if (mIsOwner) {
        if (mPlotData) delete mPlotData;
        if (mDwgLayout) delete mDwgLayout;
        if (mDwgLayoutBlock) delete mDwgLayoutBlock;
        if (mPlotHistoryData) delete mPlotHistoryData;
        if (mPlotAddFileData) delete mPlotAddFileData;
    }
}

void PlotListTreeItem::SetIsOwner(bool aIsOwner) {
    mIsOwner = aIsOwner;
}

bool PlotListTreeItem::LessByCodeSheet(const PlotListTreeItem & other, int aSortCol) const {
    // return "not less" if equal
    if (text(mPlotListTree->Cols()[PlotListTree::colCode]) == other.text(mPlotListTree->Cols()[PlotListTree::colCode])
            && text(mPlotListTree->Cols()[PlotListTree::colSheet]) == other.text(mPlotListTree->Cols()[PlotListTree::colSheet])) return false;

    bool res;
    if (text(mPlotListTree->Cols()[PlotListTree::colCode]) != other.text(mPlotListTree->Cols()[PlotListTree::colCode]))
        res = CmpStringsWithNumbersNoCase(text(mPlotListTree->Cols()[PlotListTree::colCode]), other.text(mPlotListTree->Cols()[PlotListTree::colCode]));
    else
        res = CmpStringsWithNumbersNoCase(text(mPlotListTree->Cols()[PlotListTree::colSheet]), other.text(mPlotListTree->Cols()[PlotListTree::colSheet]));
    // code+sheet is always in straight order
    if (mPlotListTree->header()->sortIndicatorOrder() == Qt::AscendingOrder
            || aSortCol == mPlotListTree->Cols()[PlotListTree::colCode])
        return res;
    else
        return !res;
}

bool PlotListTreeItem::operator<(const QTreeWidgetItem & other) const {
    if (mDwgLayout && static_cast<const PlotListTreeItem &>(other).mDwgLayout) {
        // always direct order of layouts and history, never sort it
        if (mPlotListTree->header()->sortIndicatorOrder() == Qt::AscendingOrder)
            return mDwgLayout->Num() < static_cast<const PlotListTreeItem &>(other).mDwgLayout->Num();
        else
            return mDwgLayout->Num() > static_cast<const PlotListTreeItem &>(other).mDwgLayout->Num();
    }
    if (mPlotHistoryData && static_cast<const PlotListTreeItem &>(other).mPlotHistoryData) {
        // always direct order of layouts and history, never sort it
        if (mPlotListTree->header()->sortIndicatorOrder() == Qt::AscendingOrder)
            return mPlotHistoryData->Num() > static_cast<const PlotListTreeItem &>(other).mPlotHistoryData->Num();
        else
            return mPlotHistoryData->Num() < static_cast<const PlotListTreeItem &>(other).mPlotHistoryData->Num();
    }

    int sortCol = treeWidget()->sortColumn();

    if (sortCol == mPlotListTree->Cols()[PlotListTree::colID]
            || sortCol == mPlotListTree->Cols()[PlotListTree::colIdProject]
            || sortCol == mPlotListTree->Cols()[PlotListTree::colIdCommon]
            || sortCol == mPlotListTree->Cols()[PlotListTree::colIdHist]
            || sortCol == mPlotListTree->Cols()[PlotListTree::colHist]
            || sortCol == mPlotListTree->Cols()[PlotListTree::colXrefs]) {
        // it is int fields
        if (text(sortCol).toInt() != other.text(sortCol).toInt())
            return text(sortCol).toInt() < other.text(sortCol).toInt();
        else
            return LessByCodeSheet(static_cast<const PlotListTreeItem &>(other), sortCol);
    } else if (sortCol == mPlotListTree->Cols()[PlotListTree::colSize]) {
        // it is size (with spaces, points and ,)
        if (text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt() != other.text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt())
            return text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt() < other.text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt();
        else
            return LessByCodeSheet(static_cast<const PlotListTreeItem &>(other), sortCol);
    } else if (sortCol == mPlotListTree->Cols()[PlotListTree::colCancelDate]
               || sortCol == mPlotListTree->Cols()[PlotListTree::colSentDate]
               || sortCol == mPlotListTree->Cols()[PlotListTree::colCreated]
               || sortCol == mPlotListTree->Cols()[PlotListTree::colEdited]) {
        QDateTime d1, d2;
        d1 = data(sortCol, Qt::UserRole).toDateTime();
        d2 = other.data(sortCol, Qt::UserRole).toDateTime();

        if (mPlotListTree->header()->sortIndicatorOrder() == Qt::AscendingOrder) {
            if (!d1.isNull() && d2.isNull())
                return true;
            else if (d1.isNull() && !d2.isNull())
                return false;
        }
        if (mPlotListTree->header()->sortIndicatorOrder() == Qt::DescendingOrder) {
            if (d1.isNull() && !d2.isNull())
                return true;
            else if (!d1.isNull() && d2.isNull())
                return false;
        }

        if (d1 != d2)
            return d1 < d2;
        else
            return LessByCodeSheet(static_cast<const PlotListTreeItem &>(other), sortCol);
    } else if (sortCol == mPlotListTree->Cols()[PlotListTree::colCode]) {
        return LessByCodeSheet(static_cast<const PlotListTreeItem &>(other), sortCol);
    }

    if (text(sortCol) != other.text(sortCol))
        return CmpStringsWithNumbersNoCase(text(sortCol), other.text(sortCol));
    else
        return LessByCodeSheet(static_cast<const PlotListTreeItem &>(other), sortCol);

    //return QTreeWidgetItem::operator<(other);
}

PlotData * PlotListTreeItem::PlotRef() const {
    return mPlotData;
}

const PlotData * PlotListTreeItem::PlotConst() const {
    return mPlotData;
}

DwgLayoutData * PlotListTreeItem::DwgLayoutRef() const {
    return mDwgLayout;
}

const DwgLayoutData * PlotListTreeItem::DwgLayoutConst() const {
    return mDwgLayout;
}

PlotHistoryData * PlotListTreeItem::PlotHistoryRef() const {
    return mPlotHistoryData;
}

const PlotHistoryData * PlotListTreeItem::PlotHistoryConst() const {
    return mPlotHistoryData;
}

PlotAddFileData * PlotListTreeItem::PlotAddFileRef() const {
    return mPlotAddFileData;
}

const PlotAddFileData * PlotListTreeItem::PlotAddFileConst() const {
    return mPlotAddFileData;
}

void PlotListTreeItem::ShowEditStatus() {
    if (!mPlotData) return;

    int lColEditStatus = mPlotListTree->Cols()[PlotListTree::colStatus];

    setTextAlignment(lColEditStatus, Qt::AlignLeft | Qt::AlignTop);
    if (gSettings->DocumentTree.UseDocColor) setBackground(lColEditStatus, gSettings->DocumentTree.DocColor);

    switch (mPlotData->ES()) {
    case PlotData::PESFree:
        setText(lColEditStatus, "");
        setToolTip(lColEditStatus, "");
        setBackground(lColEditStatus, background(lColEditStatus + 1));
        break;
    case PlotData::PESError:
        setText(lColEditStatus, QObject::tr("Error"));
        setToolTip(lColEditStatus, QObject::tr("Error while editing, user ") + gUsers->GetName(mPlotData->ESUserConst()));
        setBackgroundColor(lColEditStatus, MY_COLOR_WARNING);
        break;
    case PlotData::PESEditing:
        setText(lColEditStatus, QObject::tr("Editing"));
        setToolTip(lColEditStatus, QObject::tr("Now editing by ") + gUsers->GetName(mPlotData->ESUserConst()));
        setBackgroundColor(lColEditStatus, MY_COLOR_ERROR);
        break;
    }
}

void PlotListTreeItem::ShowData() {
    if (mPlotData) {
        int lCol = 0;

        // data for drag&drop
        setData(0, Qt::UserRole + 0, gSettings->BaseNameOnly + "/" + gSettings->CurrentSchema);
        setData(0, Qt::UserRole + 1, "PLOT");
        setData(0, Qt::UserRole + 2, mPlotData->IdProject());
        setData(0, Qt::UserRole + 3, mPlotData->Id());

        if (mPlotListTree->PLTFlags() & PLTForXrefs) {
            // it is place for block name in XREF
            setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
            lCol++;
            setCheckState(lCol, Qt::Unchecked);
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
            lCol++;
        }

        setText(lCol, QString::number(mPlotData->Id()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
        if (mPlotData->Cancelled()
                && mPlotListTree->isColumnHidden(mPlotListTree->Cols()[PlotListTree::colVersionInt])
                && mPlotListTree->isColumnHidden(mPlotListTree->Cols()[PlotListTree::colVersionExt])
                && mPlotListTree->isColumnHidden(mPlotListTree->Cols()[PlotListTree::colSentDate])) {
            setBackgroundColor(lCol, QColor(0x77, 0x77, 0x77));
        }
        lCol++;

        if (mPlotListTree->PLTFlags() & (PLTWorking | PLTVersions)) {
            setCheckState(lCol, mPlotData->Working()?Qt::Checked:Qt::Unchecked);
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
            lCol++;
        }

        if (mPlotListTree->PLTFlags() & PLTForXrefs) {
            // it is place for history in XREF
            setTextAlignment(lCol, Qt::AlignHCenter| Qt::AlignTop);
            lCol++;
        }

        if (mPlotListTree->PLTFlags() & PLTProject) {
            setText(lCol, QString::number(mPlotData->IdProject()));
            setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
            lCol++;

            setText(lCol, gProjects->ProjectFullShortName(mPlotData->IdProject()));
            setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
            lCol++;
        }

        setText(lCol, QString::number(mPlotData->IdCommon()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
        lCol++;

        if (qobject_cast<PlotListDlg *>(mPlotListTree->ParentDlg())
                            && mPlotListTree->SLT() == GlobalSettings::DocumentTreeStruct::SLTHistory) {
            if (mPlotHistoryData) {
                setText(lCol, QString::number(mPlotHistoryData->Id()));
            } else {
                setText(lCol, QString::number(mPlotData->IdDwgMax()));
            }
            setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
            lCol++;

            if (mPlotHistoryData) {
                setText(lCol, QString::number(mPlotHistoryData->Num()) + "/" + QString::number(mPlotData->DwgVersionMax()));
            } else {
                if (mMaxDwgVersion && mMaxDwgVersion != mPlotData->DwgVersionMax()) {
                    setText(lCol, QString::number(mPlotData->DwgVersionMax()) + "/" + QString::number(mMaxDwgVersion));
                    setBackgroundColor(lCol, QColor(0x77, 0x77, 0x77));
                } else {
                    setText(lCol, QString::number(mPlotData->DwgVersionMax()));
                }
            }
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
            lCol++;

            // origin
            lCol++;
        }

        setText(lCol, mPlotData->VersionIntConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        if (mPlotData->Cancelled()
                && !mPlotListTree->isColumnHidden(lCol)
                && mPlotListTree->isColumnHidden(mPlotListTree->Cols()[PlotListTree::colSentDate])) {
            setBackgroundColor(lCol, QColor(0x77, 0x77, 0x77));
        }
        lCol++;

        setText(lCol, mPlotData->VersionExtConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        if (mPlotData->Cancelled()
                && !mPlotListTree->isColumnHidden(lCol)
                && mPlotListTree->isColumnHidden(mPlotListTree->Cols()[PlotListTree::colSentDate])) {
            setBackgroundColor(lCol, QColor(0x77, 0x77, 0x77));
        }
        lCol++;

        if (mPlotListTree->PLTFlags() & PLTNewVersion) {
            // we will set it in window, not here
            //setBackground(lCol, QColor(210, 255, 255, 255));
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
            lCol++;

            //setBackground(lCol, QColor(210, 255, 255, 255));
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
            lCol++;
        }

        if (mPlotListTree->Cols()[PlotListTree::colVersionDate] != -1) {
            lCol++;
        }

        if (mPlotListTree->PLTFlags() & PLTDeleted) {
            setText(lCol, mPlotData->DeleteDateConst().toString("dd.MM.yy"));
            if (!mPlotData->DeleteDateConst().isNull())
                setData(lCol, Qt::UserRole, mPlotData->DeleteDateConst());
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
            lCol++;

            setText(lCol, gUsers->GetName(mPlotData->DeleteUserConst()));
            setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
            lCol++;
        }

        setText(lCol, mPlotData->CancelDateConst().toString("dd.MM.yy"));
        if (!mPlotData->CancelDateConst().isNull())
            setData(lCol, Qt::UserRole, mPlotData->CancelDateConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, gUsers->GetName(mPlotData->CancelUserConst()));
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;


        setText(lCol, mPlotData->SentDateConst().toString("dd.MM.yy"));
        if (!mPlotData->SentDateConst().isNull())
            setData(lCol, Qt::UserRole, mPlotData->SentDateConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        if (mPlotData->Cancelled()
                && !mPlotListTree->isColumnHidden(lCol)) {
            setBackgroundColor(lCol, QColor(0x77, 0x77, 0x77));
        }
        lCol++;

        setText(lCol, gUsers->GetName(mPlotData->SentUserConst()));
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlotData->SectionConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        if (qobject_cast<PlotListDlg *>(mPlotListTree->ParentDlg())
                            && mPlotListTree->SLT() == GlobalSettings::DocumentTreeStruct::SLTLayouts) {
            lCol++; // skip layout name
        }
        setText(lCol, mPlotData->CodeConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlotData->SheetConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        if (mPlotListTree->Cols()[PlotListTree::colStage] != -1) {
            lCol++;
        }

        if (mPlotListTree->Cols()[PlotListTree::colPurpose] != -1) {
            lCol++;
        }

        if (!(mPlotListTree->PLTFlags() & PLTForXrefs)) {
            setText(lCol, mPlotData->BlockNameConst());
            setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
            lCol++;
        }

        setText(lCol, mPlotData->NameTopConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlotData->NameConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlotData->CrDateConst().toString("dd.MM.yy"));
        if (!mPlotData->CrDateConst().isNull())
            setData(lCol, Qt::UserRole, mPlotData->CrDateConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, gUsers->GetName(mPlotData->CrUserConst()));
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlotData->EditDateConst().toString("dd.MM.yy HH:mm:ss"));
        if (!mPlotData->EditDateConst().isNull())
            setData(lCol, Qt::UserRole, mPlotData->EditDateConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, gUsers->GetName(mPlotData->EditUserConst()));
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        if (mPlotListTree->PLTFlags() & PLTEditStatus) {
            ShowEditStatus();
            lCol++;
        }

        if (mPlotData->FileType() < 20 || mPlotData->FileType() > 29) {
            setText(lCol, mPlotData->ExtensionConst());
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        }
        lCol++;

        setText(lCol, gSettings->FormatNumber(mPlotData->DataLength()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
        lCol++;

    //    if (mPlotData->AcadVer() >= 4) {
    //        setText(lCol, QString::number(mPlotData->AcadVer() + 2000));
    //    } else if (mPlotData->AcadVer() == 2) {
    //        setText(lCol, "2000");
    //    } else if (mPlotData->AcadVer() == 1) {
    //        setText(lCol, "R14");
    //    } else {
    //        setText(lCol, "");
    //    }
    //    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    //    lCol++;

        if (mPlotData->ExtensionConst().toLower() == "dwg") {
            setText(lCol, QString::number(mPlotData->XrefsCnt()));
        }
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

    //    if (!aInitType) {
    //        if (mPlotData->mDwgData.mLayoutCnt == -1) {
    //            setText(lCol, "unk");
    //            setToolTip(lCol, "Unknown");
    //        } else {
    //            setText(lCol, QString::number(mPlotData->mDwgData.mLayoutCnt));
    //        }
    //        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    //    }
    //    lCol++;

        setText(lCol, mPlotData->NotesConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;
    } else if (mDwgLayout) {
        setText(mPlotListTree->Cols()[PlotListTree::colID], QString::number(mDwgLayout->Num()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colID], Qt::AlignRight | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colLayoutName], mDwgLayout->NameConst());
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colLayoutName], Qt::AlignLeft | Qt::AlignTop);

        if (mDwgLayoutBlock) {
            if (mDwgLayoutBlock->Revisions().isEmpty()) {
                setText(mPlotListTree->Cols()[PlotListTree::colVersionExt], "");
            } else if (mDwgLayoutBlock->Revisions().length() == 1) {
                setText(mPlotListTree->Cols()[PlotListTree::colVersionExt], mDwgLayoutBlock->Revisions().at(0));
            } else {
                setText(mPlotListTree->Cols()[PlotListTree::colVersionExt], "X");
            }
            setTextAlignment(mPlotListTree->Cols()[PlotListTree::colVersionExt], Qt::AlignHCenter | Qt::AlignTop);

            if (mDwgLayoutBlock->RevDates().isEmpty()) {
                setText(mPlotListTree->Cols()[PlotListTree::colVersionDate], "");
            } else if (mDwgLayoutBlock->RevDates().length() == 1) {
                setText(mPlotListTree->Cols()[PlotListTree::colVersionDate], mDwgLayoutBlock->RevDates().at(0));
            } else {
                setText(mPlotListTree->Cols()[PlotListTree::colVersionDate], "XX.XX.XX");
            }
            setTextAlignment(mPlotListTree->Cols()[PlotListTree::colVersionDate], Qt::AlignHCenter | Qt::AlignTop);

            if (mDwgLayoutBlock->Codes().isEmpty()) {
                setText(mPlotListTree->Cols()[PlotListTree::colCode], "");
            } else if (mDwgLayoutBlock->Codes().length() == 1) {
                setText(mPlotListTree->Cols()[PlotListTree::colCode], mDwgLayoutBlock->Codes().at(0));
            } else {
                setText(mPlotListTree->Cols()[PlotListTree::colCode], "X");
            }
            setTextAlignment(mPlotListTree->Cols()[PlotListTree::colCode], Qt::AlignLeft | Qt::AlignTop);

            if (mDwgLayoutBlock->SheetFileNames().isEmpty()) {
                setText(mPlotListTree->Cols()[PlotListTree::colNameTop], "");
            } else if (mDwgLayoutBlock->SheetFileNames().length() == 1) {
                setText(mPlotListTree->Cols()[PlotListTree::colNameTop], mDwgLayoutBlock->SheetFileNames().at(0));
            } else {
                setText(mPlotListTree->Cols()[PlotListTree::colNameTop], "X");
            }
            setTextAlignment(mPlotListTree->Cols()[PlotListTree::colNameTop], Qt::AlignLeft | Qt::AlignTop);

            //setText(mPlotListTree->Cols()[PlotListTree::colNameBottom], QString::number(mDwgLayoutBlock->PlotFileNames().length()));
            if (mDwgLayoutBlock->PlotFileNames().isEmpty()) {
                setText(mPlotListTree->Cols()[PlotListTree::colNameBottom], "");
            } else if (mDwgLayoutBlock->PlotFileNames().length() == 1) {
                setText(mPlotListTree->Cols()[PlotListTree::colNameBottom], mDwgLayoutBlock->PlotFileNames().at(0));
            } else {
                setText(mPlotListTree->Cols()[PlotListTree::colNameBottom], "X");
            }
            setTextAlignment(mPlotListTree->Cols()[PlotListTree::colNameBottom], Qt::AlignLeft | Qt::AlignTop);

        }

        setText(mPlotListTree->Cols()[PlotListTree::colSheet], mDwgLayout->SheetConst());
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colSheet], Qt::AlignHCenter | Qt::AlignTop);

        QString lDText, lFText;

        // do't know what is it
        mDwgLayout->GetMatzDFTexts(lDText, lFText);

        setText(mPlotListTree->Cols()[PlotListTree::colStage], lDText);
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colStage], Qt::AlignLeft | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colPurpose], lFText);
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colPurpose], Qt::AlignLeft | Qt::AlignTop);
    } else if (mPlotHistoryData) {
        setText(mPlotListTree->Cols()[PlotListTree::colIdHist], QString::number(mPlotHistoryData->Id()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colIdHist], Qt::AlignRight | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colHist], QString::number(mPlotHistoryData->Num()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colHist], Qt::AlignHCenter | Qt::AlignTop);

        switch (mPlotHistoryData->Type()) {
        case 0:
            if (mPlotHistoryData->IdPlot() == mPlotHistoryData->FromIdPlot()) {
                if (mPlotHistoryData->Num() == mPlotHistoryData->FromVersion() + 1) {
                    setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Edited"));
                } else {
                    setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Edited") + " (" + QString::number(mPlotHistoryData->FromVersion()) + ")");
                }
            } else {
                if (!mPlotHistoryData->FromIdPlot()) {
                    setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("New"));
                } else {
                    setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Edited") + " (" + QString::number(mPlotHistoryData->FromIdPlot())
                            + "/" + QString::number(mPlotHistoryData->FromVersion()) + ")");
                }
            }
            break;
        case 1:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Add. file(s) edited"));
            break;
        case 2:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Add. file(s) added/removed"));
            break;
        case 3:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Add. file(s) changed in other doc"));
            break;
        case 4:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Geobase copy"));
            break;
        case 5:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Audit & purge done"));
            break;
        case 6:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Recovered"));
            break;
        case 7:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Saved after publishing"));
            break;
        case 100:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Loaded from file"));
            setToolTip(mPlotListTree->Cols()[PlotListTree::colHistOrigin], mPlotHistoryData->SavedFromFileNameConst());

            setText(mPlotListTree->Cols()[PlotListTree::colNameBottom], mPlotHistoryData->SavedFromFileNameConst());
            setTextAlignment(mPlotListTree->Cols()[PlotListTree::colNameBottom], Qt::AlignLeft | Qt::AlignTop);
            break;
        default:
            setText(mPlotListTree->Cols()[PlotListTree::colHistOrigin], QObject::tr("Error/unknown"));
            break;
        }
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colHistOrigin], Qt::AlignLeft | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colCode], mPlotHistoryData->ComputerConst().toLower() + "; " + mPlotHistoryData->IpAddrConst());
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colCode], Qt::AlignLeft | Qt::AlignTop);

        // created
        if (!mPlotHistoryData->StartTimeConst().isNull()) {
            setText(mPlotListTree->Cols()[PlotListTree::colCreated], mPlotHistoryData->StartTimeConst().toString("dd.MM.yy HH:mm:ss"));
            if (!mPlotHistoryData->StartTimeConst().isNull())
                setData(mPlotListTree->Cols()[PlotListTree::colCreated], Qt::UserRole, mPlotHistoryData->StartTimeConst());
        }
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colCreated], Qt::AlignHCenter | Qt::AlignTop);

        // last saved
        if (!mPlotHistoryData->LastSaveConst().isNull()) {
            setText(mPlotListTree->Cols()[PlotListTree::colEdited], mPlotHistoryData->LastSaveConst().toString("dd.MM.yy HH:mm:ss"));
            if (!mPlotHistoryData->LastSaveConst().isNull())
                setData(mPlotListTree->Cols()[PlotListTree::colEdited], Qt::UserRole, mPlotHistoryData->LastSaveConst());
        }
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colEdited], Qt::AlignHCenter | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colEditedBy], gUsers->GetName(mPlotHistoryData->UserConst()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colEditedBy], Qt::AlignLeft | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colSize], gSettings->FormatNumber(mPlotHistoryData->DataLength()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colSize], Qt::AlignRight | Qt::AlignTop);

        if (mPlotHistoryData->ExtConst().toLower() == "dwg") {
            setText(mPlotListTree->Cols()[PlotListTree::colXrefs], QString::number(mPlotHistoryData->XrefsCnt()));
        }
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colXrefs], Qt::AlignHCenter | Qt::AlignTop);
    } else if (mPlotAddFileData) {
        setText(mPlotListTree->Cols()[PlotListTree::colID], QString::number(mPlotAddFileData->Id()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colID], Qt::AlignRight | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colIdCommon], QString::number(mPlotAddFileData->IdLob()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colIdCommon], Qt::AlignRight | Qt::AlignTop);

        int aAFNameCol;

        if (!mPlotListTree->isColumnHidden(mPlotListTree->Cols()[PlotListTree::colBlockName])) {
            aAFNameCol = mPlotListTree->Cols()[PlotListTree::colBlockName];
        } else if (!mPlotListTree->isColumnHidden(mPlotListTree->Cols()[PlotListTree::colNameTop])){
            aAFNameCol = mPlotListTree->Cols()[PlotListTree::colNameTop];
        } else {
            aAFNameCol = mPlotListTree->Cols()[PlotListTree::colNameBottom];
        }

        setText(aAFNameCol, mPlotAddFileData->NameConst());
        setTextAlignment(aAFNameCol, Qt::AlignLeft | Qt::AlignTop);

        if (mPlotAddFileData->Id() > 0)
            setText(mPlotListTree->Cols()[PlotListTree::colVersionInt], QString::number(mPlotAddFileData->Version()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colVersionInt], Qt::AlignHCenter | Qt::AlignTop);

        setText(mPlotListTree->Cols()[PlotListTree::colSize], gSettings->FormatNumber(mPlotAddFileData->DataLength()));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colSize], Qt::AlignRight | Qt::AlignTop);

        if (!mPlotAddFileData->FTimeConst().isNull()) {
            setText(mPlotListTree->Cols()[PlotListTree::colEdited], mPlotAddFileData->FTimeConst().toString("dd.MM.yy HH:mm:ss"));
            if (!mPlotAddFileData->FTimeConst().isNull())
                setData(mPlotListTree->Cols()[PlotListTree::colEdited], Qt::UserRole, mPlotAddFileData->FTimeConst());
        }
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colEdited], Qt::AlignHCenter | Qt::AlignTop);


        if (mPlotAddFileData->NameConst().indexOf('.') != -1)
            setText(mPlotListTree->Cols()[PlotListTree::colExt], mPlotAddFileData->NameConst().mid(mPlotAddFileData->NameConst().lastIndexOf('.') + 1));
        setTextAlignment(mPlotListTree->Cols()[PlotListTree::colExt], Qt::AlignHCenter | Qt::AlignTop);
    }

    for (int i = 0; i < columnCount(); i++) {
        QFont lFont(font(i));
        QFontMetrics lFM(lFont);
        QSize lSize = lFM.size(0, text(i));
        lSize.setWidth(lSize.width() + 15); // don't know exacxtly
        lSize.setHeight(lSize.height() + gSettings->DocumentTree.AddRowHeight);
        setSizeHint(i, lSize);
    }
}

//-------------------------------------------------------------------------------------------------------------
PlotListTreeItemDelegate::PlotListTreeItemDelegate(QWidget *parent) :
    PlotListItemDelegate(parent)
{
}

QWidget *PlotListTreeItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    PlotListTree *tw = qobject_cast<PlotListTree *> (this->parent());

    if (tw->PLTFlags() & PLTDeleted) {
        QLineEdit *l = new QLineEdit(parent);
        l->setReadOnly(true);
        return l;
    }


    if (tw) {
        tw->SetEditType(0); // none!
        if (tw->PLTFlags() & (PLTFindMode | PLTVersions)) {
            if (!index.data().toString().isEmpty()) {
                // it is mean that user can copy any text
                QLineEdit *l = new QLineEdit(parent);
                l->setReadOnly(true);
                return l;
            } else {
                return NULL;
            }
        }

        PlotListTreeItem * curItem = static_cast<PlotListTreeItem *>(tw->currentItem());
        if (curItem) {
            if (tw->PLTFlags() & PLTNewVersion) {
                if (index.column() == tw->Cols()[PlotListTree::colVersionIntNew]
                        || index.column() == tw->Cols()[PlotListTree::colVersionExtNew]) {

                    tw->SetEditType(1); // documents
                    if (!curItem->isSelected()) curItem->setSelected(true);
                    tw->SetEditItems(tw->selectedItems());
                    tw->SetEditColumn(index.column());
                    tw->SetEditOldValue(index.data().toString());

                    return QStyledItemDelegate::createEditor(parent, option, index);
                } else {
                    return NULL;
                }
            } else {
                bool lStd = false, lSingleLine = false;
                QList <QTreeWidgetItem *> lSelected;
                ProjectData * lProject = NULL;
                // edit Plots
                if (curItem->PlotConst()) {
                    // do not start editing if current item belongs to document that was sent
                    if (index.column() == tw->Cols()[PlotListTree::colCode]
                            || index.column() == tw->Cols()[PlotListTree::colSheet]

                            || index.column() == tw->Cols()[PlotListTree::colVersionInt]
                            || index.column() == tw->Cols()[PlotListTree::colVersionExt]

                            || index.column() == tw->Cols()[PlotListTree::colNameTop]
                            || index.column() == tw->Cols()[PlotListTree::colNameBottom]
                            || index.column() == tw->Cols()[PlotListTree::colComments]
                            || index.column() == tw->Cols()[PlotListTree::colSection]) {
                        if (lStd = (index.column() == tw->Cols()[PlotListTree::colCode]
                                        || index.column() == tw->Cols()[PlotListTree::colSheet]
                                        || index.column() == tw->Cols()[PlotListTree::colVersionInt]
                                        || index.column() == tw->Cols()[PlotListTree::colVersionExt]
                                        || index.column() == tw->Cols()[PlotListTree::colComments]
                                        || index.column() == tw->Cols()[PlotListTree::colSection])
                                        && curItem->PlotConst()->SentDateConst().isNull()
                                    || (index.column() == tw->Cols()[PlotListTree::colNameTop]
                                        || index.column() == tw->Cols()[PlotListTree::colNameBottom])
                                        && curItem->PlotConst()->SentDateConst().isNull() /*!curItem->PlotRef()->IsSent()*/) {
                            tw->SetEditType(1); // documents
                            lSingleLine = index.column() == tw->Cols()[PlotListTree::colCode]
                                    || index.column() == tw->Cols()[PlotListTree::colSheet]
                                    || index.column() == tw->Cols()[PlotListTree::colVersionInt]
                                    || index.column() == tw->Cols()[PlotListTree::colVersionExt];

                            lSelected = tw->selectedItems();
                            for (int i = 0; i < lSelected.length(); i++) {
                                // unselect not main items or without Plotdata or Sent documents
                                if (lSelected.at(i)->parent()
                                        || !static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()
                                        || !static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotRef()->SentDateConst().isNull()/*IsSent()*/) {
                                    lSelected.at(i)->setSelected(false);
                                } else if (index.column() == tw->Cols()[PlotListTree::colSection]
                                           && !lProject
                                           && static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()) {
                                    lProject = gProjects->FindByIdProject(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()->IdProject());
                                }
                            }
                        } else {
                            QMessageBox::critical(parent, tr("Documents"), tr("Current document marked as sent"));
                        }
                    }
                    // edit Layouts
                } else if (curItem->DwgLayoutConst()
                           && (index.column() == tw->Cols()[PlotListTree::colCode]
                               || index.column() == tw->Cols()[PlotListTree::colSheet]
                               || index.column() == tw->Cols()[PlotListTree::colNameBottom])) {
                    tw->SetEditType(2); // layouts
                    lSingleLine = index.column() == tw->Cols()[PlotListTree::colCode]
                            || index.column() == tw->Cols()[PlotListTree::colSheet];
                    lStd = true;
                }
                if (lStd) {
                    if (!curItem->isSelected()) curItem->setSelected(true);
                    tw->SetEditItems(tw->selectedItems());
                    tw->SetEditColumn(index.column());
                    tw->SetEditOldValue(index.data().toString());

                    if (index.column() == tw->Cols()[PlotListTree::colSection]) {
                        if (lProject) {
                            QComboBox *cb = new QComboBox(parent);
                            cb->addItems(lProject->ComplectListConst());
                            cb->setEditable(true);
                            return cb;
                        }
                    } else if (lSingleLine) {
                        return QStyledItemDelegate::createEditor(parent, option, index);
                    } else {
                        return new QPlainTextEdit(parent);
                    }
                }
                if (!index.data().toString().isEmpty()) {
                    // it is mean that user can copy any text
                    QLineEdit *l = new QLineEdit(parent);
                    l->setReadOnly(true);
                    return l;
                }
            }
        }
    }
    return NULL;
}

void PlotListTreeItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    if (qobject_cast<QPlainTextEdit *>(editor)) {
        int lCnt = index.data().toString().count('\n');
        if (lCnt < 3) {
            QFontMetrics fm(editor->font());
            qobject_cast<QPlainTextEdit *>(editor)->resize(editor->width(), (fm.lineSpacing() + 2) * 4);
        }
    }
}
