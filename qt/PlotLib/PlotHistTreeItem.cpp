#include "PlotHistTreeItem.h"

#include "../PlotLib/PlotData.h"
#include "../UsersDlg/UserData.h"
#include "../VProject/GlobalSettings.h"

#include "../Logger/logger.h"

PlotHistTreeItem::PlotHistTreeItem(PlotData *aPlot, PlotHistoryData * aHistory, PlotHistTreeItem *aItemPrev) :
    QTreeWidgetItem(NULL),
    mIdPlot(0),
    mPlot(aPlot), mHistory(aHistory),
    mItemPrev(aItemPrev)
{
    if (mPlot) mIdPlot = mPlot->Id();
    ShowData();
}

void PlotHistTreeItem::ShowData() {
    int lCol = 0;

    setFlags(flags() | Qt::ItemIsEditable);

    setText(lCol, QString::number(mHistory->Num()));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignVCenter);
    lCol++;

    setText(lCol, QString::number(mHistory->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (!mItemPrev || mItemPrev && mPlot != mItemPrev->PlotConst())
        setText(lCol, QString::number(mPlot->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (!mItemPrev || mItemPrev && mPlot != mItemPrev->PlotConst())
        setText(lCol, mPlot->VersionIntConst());
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if (!mItemPrev || mItemPrev && mPlot != mItemPrev->PlotConst())
        setText(lCol, mPlot->VersionExtConst());
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if (!mItemPrev || mItemPrev && mPlot != mItemPrev->PlotConst())
        setText(lCol, mPlot->CodeConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    if (!mItemPrev || mItemPrev && mPlot != mItemPrev->PlotConst())
        setText(lCol, mPlot->SheetConst());
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    switch (mHistory->Type()) {
    case 0:
        if (mHistory->IdPlot() == mHistory->FromIdPlot()) {
            if (mHistory->Num() == mHistory->FromVersion() + 1) {
                setText(lCol, QObject::tr("Edited"));
            } else {
                setText(lCol, QObject::tr("Edited") + " (" + QString::number(mHistory->FromVersion()) + ")");
            }
        } else {
            if (!mHistory->FromIdPlot()) {
                setText(lCol, QObject::tr("New"));
            } else {
                setText(lCol, QObject::tr("Edited") + " (" + QString::number(mHistory->FromIdPlot())
                        + "/" + QString::number(mHistory->FromVersion()) + ")");
            }
        }
        break;
    case 1:
        setText(lCol, QObject::tr("Add. file(s) edited"));
        break;
    case 2:
        setText(lCol, QObject::tr("Add. file(s) added/removed"));
        break;
    case 3:
        setText(lCol, QObject::tr("Add. file(s) changed in other doc"));
        break;
    case 4:
        setText(lCol, QObject::tr("Geobase copy"));
        break;
    case 5:
        setText(lCol, QObject::tr("Audit & purge done"));
        break;
    case 6:
        setText(lCol, QObject::tr("Recovered"));
        break;
    case 7:
        setText(lCol, QObject::tr("Saved after publishing"));
        break;
    case 8:
        setText(lCol, QObject::tr("Text replaced"));
        break;
    case 100:
        setText(lCol, QObject::tr("Loaded from file"));
        setToolTip(lCol, mHistory->SavedFromFileNameConst());
        break;
    default:
        setText(lCol, QObject::tr("Error/unknown"));
        break;
    }
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    setText(lCol, gUsers->GetName(mHistory->UserConst()));
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    setText(lCol, mHistory->WhenConst().toString("dd.MM.yy HH:mm:ss"));
    if (!mItemPrev || mItemPrev && mHistory->WhenConst().date() != mItemPrev->HistoryConst()->WhenConst().date()) {
        QFont lFont = font(lCol);
        lFont.setBold(true);
        setFont(lCol, lFont);

    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, mHistory->ComputerConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    setText(lCol, mHistory->IpAddrConst());
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if (mPlot->FileType() < 20 || mPlot->FileType() > 29) {
        setText(lCol, gSettings->FormatNumber(mHistory->DataLength()));
    }
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (mPlot->FileType() < 20 || mPlot->FileType() > 29) {
        setText(lCol, mHistory->ExtConst());
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, mHistory->StartTimeConst().toString("dd.MM.yy HH:mm:ss"));
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, mHistory->EndTimeConst().toString("dd.MM.yy HH:mm:ss"));
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, QString::number(mHistory->IdleSec()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (mHistory->Type() == 0) {
        setText(lCol, QString::number(mHistory->SaveCount()));
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, mHistory->LastSaveConst().toString("dd.MM.yy HH:mm:ss"));
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, mHistory->FTimeConst().toString("dd.MM.yy HH:mm:ss"));
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if (mHistory->Type() == 100
            && (mPlot->FileType() < 20 || mPlot->FileType() > 29)) {
        setText(lCol, gSettings->FormatNumber(mHistory->FileSize()));
    }
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (mHistory->Type() == 0
            && mHistory->SaveCount()
            && (mPlot->FileType() < 20 || mPlot->FileType() > 29)
            && mHistory->ExtConst().toLower() == "dwg") {
        setText(lCol, QString::number(mHistory->EntAdded()));
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if (mHistory->Type() == 0
            && mHistory->SaveCount()
            && (mPlot->FileType() < 20 || mPlot->FileType() > 29)
            && mHistory->ExtConst().toLower() == "dwg") {
        setText(lCol, QString::number(mHistory->EntChanged()));
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if (mHistory->Type() == 0
            && mHistory->SaveCount()
            && (mPlot->FileType() < 20 || mPlot->FileType() > 29)
            && mHistory->ExtConst().toLower() == "dwg") {
        setText(lCol, QString::number(mHistory->EntDeleted()));
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if ((mPlot->FileType() < 20 || mPlot->FileType() > 29)
            && mHistory->ExtConst().toLower() == "dwg") {
        setCheckState(lCol, mHistory->NeedNotProcess()?Qt::Checked:Qt::Unchecked);
        setFlags(flags() & ~Qt::ItemIsUserCheckable);
        //setText(lCol, QString::number(mHistory->EntChanged()));
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, mHistory->WorkingFileNameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    if (mHistory->Type() == 100
            || !mHistory->SavedFromFileNameConst().isEmpty()
                && mHistory->WorkingFileNameConst().toLower() != mHistory->SavedFromFileNameConst().toLower()) {
        setText(lCol, mHistory->SavedFromFileNameConst());
    }
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    for (int i = 0; i < columnCount(); i++) {
        QFont lFont(font(i));
        QFontMetrics lFM(lFont);
        QSize lSize = lFM.size(0, text(i));
        lSize.setWidth(lSize.width() + 15); // don't know exacxtly
        lSize.setHeight(lSize.height() + gSettings->Common.AddRowHeight);
        setSizeHint(i, lSize);
    }
}

int PlotHistTreeItem::IdPlot() const {
    return mIdPlot;
}

const PlotData * PlotHistTreeItem::PlotConst() const {
    return mPlot;
}

PlotData * PlotHistTreeItem::PlotRef() {
    return mPlot;
}

const PlotHistoryData * PlotHistTreeItem::HistoryConst() const {
    return mHistory;
}

PlotHistoryData * PlotHistTreeItem::HistoryRef() {
    return mHistory;
}

//void PlotHistTreeItem::ShowTempFileName(bool aShow) {
//    if (aShow) {
//        setText(mColWorkingFilename, mHistory->WorkingFileNameConst());
//    } else {
//        setText(mColWorkingFilename, "");
//    }
//}

//void PlotHistTreeItem::ShowSavedFromFileName(bool aShow) {
//    if (aShow) {
//        setText(mColWorkingFilename, mHistory->SavedFromFileNameConst());
//    } else {
//        if (mHistory->Type() != 100) {
//            setText(mColWorkingFilename, "");
//        }
//    }
//}


