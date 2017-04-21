#include "PlotAddFilesTreeItem.h"

#include "PlotData.h"

#include "../VProject/GlobalSettings.h"

PlotAddFilesTreeItem::PlotAddFilesTreeItem(PlotAddFileData * aAddFile) :
    QTreeWidgetItem(NULL),
    mAddFile(aAddFile)
{
    ShowData();
}

void PlotAddFilesTreeItem::ShowData() {
    int lCol = 0;

    setFlags(flags() | Qt::ItemIsEditable);

    setText(lCol, QString::number(mAddFile->Id()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    setText(lCol, QString::number(mAddFile->IdLob()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    setText(lCol, mAddFile->NameConst());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    if (mAddFile->NameConst().lastIndexOf('.') != -1) {
        setText(lCol, mAddFile->NameConst().mid(mAddFile->NameConst().lastIndexOf('.') + 1).toLower());
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    if (mAddFile->Id() > 0) {
        setText(lCol, QString::number(mAddFile->Version()));
    }
    setTextAlignment(lCol, Qt::AlignCenter);
    lCol++;

    setText(lCol, gSettings->FormatNumber(mAddFile->DataLength()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    setText(lCol, mAddFile->FTimeConst().toString("dd.MM.yy HH:mm:ss"));
    setTextAlignment(lCol, Qt::AlignCenter);
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

bool PlotAddFilesTreeItem::operator<(const QTreeWidgetItem & other) const {
    int sortCol = treeWidget()->sortColumn();

    if (sortCol == 0
            || sortCol == 1
            || sortCol == 4) {
        return text(sortCol).toInt() < other.text(sortCol).toInt();
    } else if (sortCol == 5) {
        return text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt() < other.text(sortCol).replace(",", "").replace(".", "").replace(" ", "").toInt();
    } else if (sortCol == 6) {
        return QDateTime::fromString(text(sortCol), "dd.MM.yy HH:mm:ss") < QDateTime::fromString(other.text(sortCol), "dd.MM.yy HH:mm:ss");
    }
    return QTreeWidgetItem::operator<(other);
}

const PlotAddFileData * PlotAddFilesTreeItem::AddFileConst() const {
    return mAddFile;
}

