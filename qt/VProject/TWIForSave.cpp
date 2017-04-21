#include "GlobalSettings.h"
#include "TWIForSave.h"

#include "common.h"

#include "../UsersDlg/UserData.h"
#include "../PlotLib/DwgData.h"

TWIForSaveMain::TWIForSaveMain(ItemType aItemType, QTreeWidget * parent) :
    QTreeWidgetItem(parent),
    mItemType(aItemType),
    mColSentDate(-1), mColEditDate(-1), mColStatus(-1), mColDatalength(-1)

{

}

bool TWIForSaveMain::operator<(const QTreeWidgetItem & other) const {
    int sortCol = treeWidget()->sortColumn();

    if (!sortCol) {
        if (text(sortCol).toInt() && other.text(sortCol).toInt())
            return text(sortCol).toInt() < other.text(sortCol).toInt();
    } else if (sortCol == mColSentDate || sortCol == mColEditDate) {
        QDate d1, d2;
        d1 = d1.fromString(text(sortCol), "dd.MM.yyyy");
        d2 = d2.fromString(other.text(sortCol), "dd.MM.yyyy");

        return d1 < d2;
    } else if (sortCol == mColDatalength) {
        return text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt() < other.text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt();
    }

    return QTreeWidgetItem::operator<(other);
}

TWIForSaveMain::ItemType TWIForSaveMain::GetItemType() const {
    return mItemType;
}

void TWIForSaveMain::ShowStatus(PlotData::enumPES aES, const QString &aUserLogin) {
    switch (aES) {
    case PlotData::PESFree:
        setText(mColStatus, QObject::tr("Free"));
        setBackground(mColStatus, background(mColStatus + 1));
        break;
    case PlotData::PESError:
        setText(mColStatus, QObject::tr("Error: ") + gUsers->GetName(aUserLogin));
        setBackgroundColor(mColStatus, MY_COLOR_WARNING);
        break;
    case PlotData::PESEditing:
        setText(mColStatus, QObject::tr("Editing by ") + gUsers->GetName(aUserLogin));
        setBackgroundColor(mColStatus, MY_COLOR_ERROR);
        break;
    }
    setTextAlignment(mColStatus, Qt::AlignLeft | Qt::AlignTop);
}

TWIForSaveMainDoc::TWIForSaveMainDoc(PlotForSaveData *aPlot, QTreeWidget * parent) :
    TWIForSaveMain(ICTMain, parent),
    mPlot(aPlot)
{
    int lCol = 0;

    // QMessageBox::critical(NULL, "AutoCAD support files", QString::number(flags()));
    // default is 61 = Qt::ItemIsEnabled (32) | Qt::ItemIsUserCheckable (16) | Qt::ItemIsDropEnabled (8) | Qt::ItemIsDragEnabled(4) | Qt::ItemIsSelectable(1)
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    setText(lCol, QString::number(mPlot->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    // place for file name
    QFont lFont = font(lCol);
    lFont.setBold(true);
    setFont(lCol, lFont);
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->VersionIntConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->VersionExtConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, QString::number(mPlot->DwgConst()->Version()) + "/" + QString::number(mPlot->DwgVersionMax()));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->SentDateConst().toString("dd.MM.yyyy"));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColSentDate = lCol;
    lCol++;

    setText(lCol, mPlot->CodeConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->SheetConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->NameTopConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->NameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->EditDateConst().toString("dd.MM.yyyy"));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColEditDate = lCol;
    lCol++;

    setText(lCol, mPlot->EditUserConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    // that is status
    mColStatus = lCol;
    ShowStatus(mPlot->ES(), mPlot->ESUserConst());
    lCol++;

    if (mPlot->FileType() < 20 || mPlot->FileType() > 29) {
        setText(lCol, mPlot->DwgConst()->ExtensionConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    }
    lCol++;

    setText(lCol, gSettings->FormatNumber(mPlot->DwgConst()->DataLength()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColDatalength = lCol;
    lCol++;

    setText(lCol, mPlot->DwgConst()->AcadVerStr());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    if (mPlot->DwgConst()->LayoutCnt() == -1) {
        setText(lCol, "unk");
        setToolTip(lCol, "Unknown");
    } else {
        setText(lCol, QString::number(mPlot->DwgConst()->LayoutCnt()));
    }    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    int lNUF = mPlot->NeedUpdateFields();
    if (lNUF) {
        QString lTip = QObject::tr("Need update field(s):");
        setText(lCol, QString::number(lNUF));
        // order as in AutoCAD menu
        if (lNUF & 1) lTip += "\n" + QObject::tr("Code");
        if (lNUF & 0x40) lTip += "\n" + QObject::tr("Sheet");
        if (lNUF & 0x80) lTip += "\n" + QObject::tr("Code + sheet");
        if (lNUF & 2) lTip += "\n" + QObject::tr("Name top");
        if (lNUF & 4) lTip += "\n" + QObject::tr("Name bottom");
        if (lNUF & 0x100) lTip += "\n" + QObject::tr("Layout name in stamp");
        if (lNUF & 0x10) lTip += "\n" + QObject::tr("Version");
        if (lNUF & 8) lTip += "\n" + QObject::tr("Project name");
        if (lNUF & 0x20) lTip += "\n" + QObject::tr("Project stage");
        setToolTip(lCol, lTip);
    } else {
        setToolTip(lCol, "");
    }
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);

    setCheckState(lCol, lNUF?Qt::Checked:Qt::Unchecked);
    lCol++;


    // neednot process - for xref's only
    lCol++;

    // id in cache - for xref's only
    lCol++;

    setCheckState(lCol, mPlot->DwgConst()->InSubs()?Qt::Checked:Qt::Unchecked);
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->NotesConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;
}

QString TWIForSaveMainDoc::GetSheetWithX(bool aUpper) const {
    if (mPlot->DwgConst()->LayoutCnt() < 2) return mPlot->SheetConst();
    if (mPlot->SheetConst().right(1) < "0"
            || mPlot->SheetConst().right(1) > "9") return mPlot->SheetConst();

    if (mPlot->DwgConst()->LayoutCnt() + mPlot->SheetConst().rightRef(1).toInt() > 9)
        if (aUpper) {
            return mPlot->SheetConst().left(mPlot->SheetConst().length() - 2) + "XX";
        }else {
            return mPlot->SheetConst().left(mPlot->SheetConst().length() - 2) + "xx";
        }

    else
        if (aUpper) {
            return mPlot->SheetConst().left(mPlot->SheetConst().length() - 1) + "X";
        } else {
            return mPlot->SheetConst().left(mPlot->SheetConst().length() - 1) + "x";
        }
}

TWIForSaveMainXref::TWIForSaveMainXref(XrefForSaveData * aXref, QTreeWidget * parent) :
    TWIForSaveMain(ICTXref, parent),
    mXref(aXref)
{
    int lCol = 0;

    // QMessageBox::critical(NULL, "AutoCAD support files", QString::number(flags()));
    // default is 61 = Qt::ItemIsEnabled (32) | Qt::ItemIsUserCheckable (16) | Qt::ItemIsDropEnabled (8) | Qt::ItemIsDragEnabled(4) | Qt::ItemIsSelectable(1)
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    //----------------------------------------------------------------------------------
    setText(lCol, QString::number(mXref->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    //----------------------------------------------------------------------------------
    setText(lCol, mXref->BlockNameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->VersionIntConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->VersionExtConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, QString::number(mXref->DwgConst()->Version()) + "/" + QString::number(mXref->DwgVersionMax()));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    // sent date
    //mColSentDate = lCol;
    lCol++;

    setText(lCol, mXref->CodeConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->SheetConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->NameTopConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->NameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->EditDateConst().toString("dd.MM.yyyy"));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColEditDate = lCol;
    lCol++;

    setText(lCol, gUsers->GetName(mXref->EditUserConst()));
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    // that is status
    mColStatus = lCol;
    ShowStatus(mXref->ES(), mXref->ESUserConst());
    lCol++;

    // extension - always dwg for xref, need nopt output
    lCol++;

    setText(lCol, gSettings->FormatNumber(mXref->DwgConst()->DataLength()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColDatalength = lCol;
    lCol++;

    setText(lCol, mXref->DwgConst()->AcadVerStr());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    // layout count - need not in xrefs
    lCol++;

    // need field updates - need not in xrefs
    lCol++;


    // for xref's only
    setCheckState(lCol, mXref->DwgConst()->NeedNotProcess()?Qt::Checked:Qt::Unchecked);
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    // id in cache
    if (mXref->DwgConst()->IdCache()) {
        setText(lCol, QString::number(mXref->DwgConst()->IdCache()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
        setBackground(lCol, background(lCol + 1));
        setBackground(lCol - 1, background(lCol + 1));
        setToolTip(lCol, "");
        setToolTip(lCol - 1, "");
    } else if (!mXref->DwgConst()->NeedNotProcess()) {
        setText(lCol, "");
        setBackgroundColor(lCol, MY_COLOR_WARNING);
        setBackgroundColor(lCol - 1, MY_COLOR_WARNING);
        setToolTip(lCol, "This xref required processing in AutoCAD");
        setToolTip(lCol - 1, "This xref required processing in AutoCAD");
    }
    lCol++;

    setCheckState(lCol, mXref->DwgConst()->InSubs()?Qt::Checked:Qt::Unchecked);
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->NotesConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;
}

TWIForSaveMainAddFile::TWIForSaveMainAddFile(DwgForSaveData * aDwg, QTreeWidget * parent) :
    TWIForSaveMain(ICTAddFile, parent),
    mDwg(aDwg)
{
    int lCol = 0;

    // QMessageBox::critical(NULL, "AutoCAD support files", QString::number(flags()));
    // default is 61 = Qt::ItemIsEnabled (32) | Qt::ItemIsUserCheckable (16) | Qt::ItemIsDropEnabled (8) | Qt::ItemIsDragEnabled(4) | Qt::ItemIsSelectable(1)
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    //----------------------------------------------------------------------------------
    setText(lCol, QString::number(mDwg->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    //----------------------------------------------------------------------------------
    setText(lCol, mDwg->FilenameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    lCol++;

    lCol++;

    lCol++;

    lCol++;

    lCol++;

    lCol++;

    lCol++;

    lCol++;

    //mColEditDate = lCol;
    lCol++;

    lCol++;

    // that is status
    lCol++;

    lCol++;

    setText(lCol, gSettings->FormatNumber(mDwg->DataLength()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColDatalength = lCol;
    lCol++;

    lCol++;

    lCol++;

    lCol++;

    // for xref's only
    lCol++;

    // id in cache
    lCol++;

    lCol++;

    lCol++;

}

//-----------------------------------------------------------------------------------------
TWIForSaveHeader::TWIForSaveHeader(const QString &aHeader) :
    QTreeWidgetItem()
{
    setText(0, aHeader);
}

bool TWIForSaveHeader::operator<(const QTreeWidgetItem & other) const {
    return false; // in that way it never do sort, always order is as added
}

//-----------------------------------------------------------------------------------------
TWIForSaveXrefTop::TWIForSaveXrefTop(XrefForSaveData * aXref, XrefPropsData * aXrefProps, int aCase, QTreeWidget * parent) :
    TWIForSaveMain(ICTXrefTop, parent),
    mXref(aXref), mXrefProps(aXrefProps)
{
    if (!mXrefProps) mXrefProps = &lXrefPropsDataNULL;
    InitColumns(aCase);
}

void TWIForSaveXrefTop::InitColumns(int aCase) {
    int lCol = 0;
    QFont lBoldFont = font(0);
    lBoldFont.setBold(true);

    // QMessageBox::critical(NULL, "AutoCAD support files", QString::number(flags()));
    // default is 61 = Qt::ItemIsEnabled (32) | Qt::ItemIsUserCheckable (16) | Qt::ItemIsDropEnabled (8) | Qt::ItemIsDragEnabled(4) | Qt::ItemIsSelectable(1)
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    //----------------------------------------------------------------------------------

    switch (aCase) {
    case 0:
        setText(lCol, mXref->BlockNameConst());
        break;
    case 1:
        setText(lCol, mXref->BlockNameConst().toLower());
        break;
    case 2:
        setText(lCol, mXref->BlockNameConst().toUpper());
        break;
    }

    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    // new name here
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    // xref properties
    if (mXrefProps) setText(lCol, mXrefProps->GetAbrv());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    //----------------------------------------------------------------------------------
    setText(lCol, QString::number(mXref->Id()));
    setFont(lCol, lBoldFont);
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    setText(lCol, QString::number(mXref->DwgConst()->Version()) + "/" + QString::number(mXref->DwgVersionMax()));
    setFont(lCol, lBoldFont);
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->VersionIntConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->VersionExtConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    lCol++;

    setText(lCol, mXref->SentDateConst().toString("dd.MM.yyyy"));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColSentDate = lCol;
    lCol++;

    setText(lCol, mXref->CodeConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->SheetConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->NameTopConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->NameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->EditDateConst().toString("dd.MM.yyyy"));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColEditDate = lCol;
    lCol++;

    setText(lCol, mXref->EditUserConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    mColStatus = lCol;
    ShowStatus(mXref->ES(), mXref->ESUserConst());
    lCol++;

    setText(lCol, gSettings->FormatNumber(mXref->DwgConst()->DataLength()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColDatalength = lCol;
    lCol++;

    setText(lCol, mXref->DwgConst()->AcadVerStr());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setCheckState(lCol, mXref->DwgConst()->NeedNotProcess()?Qt::Checked:Qt::Unchecked);
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    // id in cache
    //debug111
    if (mXref->DwgConst()->IdCache()) {
        setText(lCol, QString::number(mXref->DwgConst()->IdCache()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
        setBackground(lCol, background(lCol + 1));
        setBackground(lCol - 1, background(lCol + 1));
        setToolTip(lCol, "");
        setToolTip(lCol - 1, "");
    } else {
        setText(lCol, "");
        if (!mXref->DwgConst()->NeedNotProcess()) {
            setBackgroundColor(lCol, MY_COLOR_WARNING);
            setBackgroundColor(lCol - 1, MY_COLOR_WARNING);
            setToolTip(lCol, "This xref required processing in AutoCAD");
            setToolTip(lCol - 1, "This xref required processing in AutoCAD");
        } else {
            setBackground(lCol, background(lCol + 1));
            setBackground(lCol - 1, background(lCol + 1));
            setToolTip(lCol, "");
            setToolTip(lCol - 1, "");
        }
    }
    lCol++;

    setCheckState(lCol, mXref->DwgConst()->InSubs()?Qt::Checked:Qt::Unchecked);
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mXref->NotesConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

}

bool TWIForSaveXrefTop::operator<(const QTreeWidgetItem & other) const {
    int sortCol = treeWidget()->sortColumn();

    if (!sortCol) {
        if (text(sortCol).toInt() && other.text(sortCol).toInt())
            return text(sortCol).toInt() < other.text(sortCol).toInt();
        else
            return QTreeWidgetItem::operator<(other);
    } else if (sortCol == mColSentDate || sortCol == mColEditDate) {
        QDate d1, d2;
        d1 = d1.fromString(text(sortCol), "dd.MM.yyyy");
        d2 = d2.fromString(other.text(sortCol), "dd.MM.yyyy");

        return d1 < d2;
    } else if (sortCol == mColDatalength) {
        return text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt() < other.text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt();
    }

    return QTreeWidgetItem::operator<(other);
}

//-----------------------------------------------------------------------------------------
TWIForSaveXrefChild::TWIForSaveXrefChild(XrefForSaveData * aXref, PlotDwgData * aPlot, int aCase, QTreeWidget * parent) :
    TWIForSaveMain(ICTXrefChild, parent),
    mXref(aXref), mPlot(aPlot)
{
    int lCol = 0;

    // QMessageBox::critical(NULL, "AutoCAD support files", QString::number(flags()));
    // default is 61 = Qt::ItemIsEnabled (32) | Qt::ItemIsUserCheckable (16) | Qt::ItemIsDropEnabled (8) | Qt::ItemIsDragEnabled(4) | Qt::ItemIsSelectable(1)
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    //----------------------------------------------------------------------------------

    setText(lCol, QString::number(mPlot->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    // new name here
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    lCol++;

    //----------------------------------------------------------------------------------
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    lCol++;

    setText(lCol, QString::number(mXref->DwgConst()->Version()) + "/" + QString::number(mXref->DwgVersionMax()));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->VersionIntConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->VersionExtConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, QString::number(mPlot->DwgConst()->Version()) + "/" + QString::number(mPlot->DwgVersionMax()));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->SentDateConst().toString("dd.MM.yyyy"));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColSentDate = lCol;
    lCol++;

    setText(lCol, mPlot->CodeConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->SheetConst());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->NameTopConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->NameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->EditDateConst().toString("dd.MM.yyyy"));
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    mColEditDate = lCol;
    lCol++;

    setText(lCol, mPlot->EditUserConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

    mColStatus = lCol;
    ShowStatus(mPlot->ES(), mPlot->ESUserConst());
    lCol++;

    setText(lCol, gSettings->FormatNumber(mPlot->DwgConst()->DataLength()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    mColDatalength = lCol;
    lCol++;

    setText(lCol, mPlot->DwgConst()->AcadVerStr());
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    lCol++;

    // id in cache
    //debug111
    if (mXref->DwgConst()->IdCache()) {
        setText(lCol, QString::number(mXref->DwgConst()->IdCache()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
        setBackground(lCol, background(lCol + 1));
        setBackground(lCol - 1, background(lCol + 1));
        setToolTip(lCol, "");
        setToolTip(lCol - 1, "");
    }
    lCol++;

    setCheckState(lCol, mPlot->DwgConst()->InSubs()?Qt::Checked:Qt::Unchecked);
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
    lCol++;

    setText(lCol, mPlot->NotesConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    lCol++;

}

bool TWIForSaveXrefChild::operator<(const QTreeWidgetItem & other) const {
    int sortCol = treeWidget()->sortColumn();

    if (!sortCol) {
        if (text(sortCol).toInt() && other.text(sortCol).toInt())
            return text(sortCol).toInt() < other.text(sortCol).toInt();
        else
            return QTreeWidgetItem::operator<(other);
    } else if (sortCol == mColSentDate || sortCol == mColEditDate) {
        QDate d1, d2;
        d1 = d1.fromString(text(sortCol), "dd.MM.yyyy");
        d2 = d2.fromString(other.text(sortCol), "dd.MM.yyyy");

        return d1 < d2;
    } else if (sortCol == mColDatalength) {
        return text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt() < other.text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt();
    }

    return QTreeWidgetItem::operator<(other);
}

//-----------------------------------------------------------------------------------------
TWIForSaveAddFile::TWIForSaveAddFile(DwgForSaveData * aDwgData, RecordTypeEnum aRecordType) :
    QTreeWidgetItem(), mPlot(NULL), mDwg(aDwgData), mRecordType(aRecordType)
{
    InitColumns(0);
}

TWIForSaveAddFile::TWIForSaveAddFile(PlotDwgData * aPlotData, DwgForSaveData * aDwgData, RecordTypeEnum aRecordType) :
    QTreeWidgetItem(), mPlot(aPlotData), mDwg(aDwgData), mRecordType(aRecordType)
{
    InitColumns(1);
}

void TWIForSaveAddFile::InitColumns(int aInitType)
{
    int lCol = 0;
    // main record
    QFont lFont;
    const QString &lFileName = mDwg->FilenameConst();

    if (!aInitType) {
        // file name
        setText(lCol, lFileName);
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
    } else {
        // id
        setText(lCol, QString::number(mPlot->Id()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    };
    lCol++;

    if (mRecordType != AcadNonImage) lCol++; // new name

    // type (extension)
    if (!aInitType) {
        if (lFileName.lastIndexOf('.') != -1) {
            setText(lCol, lFileName.right(lFileName.length() - lFileName.lastIndexOf('.') - 1).toLower());
            setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        }
    }
    lCol++;


    // use id
    setText(lCol, QString::number(mDwg->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    if (!aInitType) {
        lFont = font(lCol);
        lFont.setBold(true);
        setFont(lCol, lFont);
    }
    lCol++;


    // use size
    setText(lCol, gSettings->FormatNumber(mDwg->DataLength()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
    if (!aInitType) {
        lFont = font(lCol);
        lFont.setBold(true);
        setFont(lCol, lFont);
    }
    lCol++;


    if (aInitType) {
        setText(lCol, mPlot->VersionIntConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlot->VersionExtConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, QString::number(mPlot->DwgConst()->Version()) + "/" + QString::number(mPlot->DwgVersionMax()));
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlot->SentDateConst().toString("dd.MM.yyyy"));
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlot->CodeConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlot->SheetConst());
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlot->NameTopConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlot->NameConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, mPlot->EditDateConst().toString("dd.MM.yyyy"));
        setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignTop);
        //mColEditDate = lCol;
        lCol++;

        setText(lCol, mPlot->EditUserConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;

        setText(lCol, gSettings->FormatNumber(mPlot->DwgConst()->DataLength()));
        setTextAlignment(lCol, Qt::AlignRight | Qt::AlignTop);
        lCol++;


        setText(lCol, mPlot->NotesConst());
        setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignTop);
        lCol++;
    }

}

bool TWIForSaveAddFile::operator<(const QTreeWidgetItem & other) const {
    int sortCol = treeWidget()->sortColumn();

    if (!sortCol && other.parent() || sortCol == 2) {
        return text(sortCol).toInt() < other.text(sortCol).toInt();
    } else if (sortCol == 3) {
        return text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt() < other.text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt();
    }

    return QTreeWidgetItem::operator<(other);
}

