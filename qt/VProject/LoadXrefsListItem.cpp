#include "LoadXrefsDlg.h"

#include "GlobalSettings.h"
#include "oracle.h"

#include "../Login/Login.h"

#include "../ProjectLib/ProjectData.h"

#include "FileUtils.h"

#include <QCryptographicHash>

LoadXrefsListItem::LoadXrefsListItem(int aIdProject, const QString &aOrigFileName) :
    QTreeWidgetItem(),
    mIsError(false), mIdProject(aIdProject),
    mFileInfoOrig(aOrigFileName),
    mAcadVersionOrig(0),
    mXchgFileData(NULL),
    mWhatToDo(-1),
    mTDArea(-1), mTDId(-1)
{
    setFlags(flags() | Qt::ItemIsEditable);

    TreeDataRecord *lTree;
    if (mFileInfoOrig.suffix().toLower() == "dwg") {
        lTree = gTreeData->FindByGroupId(2);
    } else {
        lTree = gTreeData->FindByGroupId(4);
    }
    if (lTree) {
        mTDArea = lTree->Area();
        mTDId = lTree->Id();
    }

    QFile lFile(aOrigFileName);
    QCryptographicHash lHash1(QCryptographicHash::Sha256);

    if (lFile.open(QFile::ReadOnly)) {
        lHash1.addData(&lFile);

        if (mFileInfoOrig.suffix().toLower() == "dwg") {
            mAcadVersionOrig = gFileUtils->AcadVersion(lFile);
        }
        lFile.close();

        mHashOrig = QString(lHash1.result().toHex()).toUpper();
    }

    ShowData();
}

LoadXrefsListItem::~LoadXrefsListItem() {
    if (mXchgFileData) delete mXchgFileData;
}

void LoadXrefsListItem::GenerateNames() {
    // NB: field code regenerated in top level

    if (mFileInfoOrig.suffix().toLower() == "dwg") {
        setText(lllLXLVersion, QDate::currentDate().toString("dd.MM.yy"));
        setTextAlignment(lllLXLVersion, Qt::AlignHCenter | Qt::AlignVCenter);

        //setText(lllLXLCode, "");
        setTextAlignment(lllLXLCode, Qt::AlignLeft | Qt::AlignVCenter);

        setText(lllLXLBlockName, mFileInfoOrig.baseName());
        setTextAlignment(lllLXLBlockName, Qt::AlignLeft | Qt::AlignVCenter);

        setText(lllLXLNameTop, mFileInfoOrig.baseName());
        setTextAlignment(lllLXLNameTop, Qt::AlignLeft | Qt::AlignVCenter);
    } else {
        setText(lllLXLVersion, /*QDate::currentDate().toString("dd.MM.yy")*/
                mFileInfoOrig.lastModified().toString("dd.MM.yy"));
        setTextAlignment(lllLXLVersion, Qt::AlignHCenter | Qt::AlignVCenter);

        //setText(lllLXLCode, "");
        setTextAlignment(lllLXLCode, Qt::AlignLeft | Qt::AlignVCenter);

        setTextAlignment(lllLXLNameTop, Qt::AlignLeft | Qt::AlignVCenter);

        setText(lllLXLNameTop + 1, mFileInfoOrig.fileName());
        setTextAlignment(lllLXLNameTop + 1, Qt::AlignLeft | Qt::AlignVCenter);
    }
}

void LoadXrefsListItem::ShowData() {
    int lCol = 0;

    setText(lCol, mFileInfoOrig.fileName());
    setToolTip(lCol, mFileInfoOrig.filePath());
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    setText(lCol, gSettings->FormatNumber(mFileInfoOrig.size()));
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (mFileInfoOrig.suffix().toLower() == "dwg") {
        if (mXchgFileData) {
            setText(lCol, gSettings->FormatNumber(mXchgFileData->FileInfoPrcdConst().size()));
            setBackground(lCol, background(lCol - 1));
        } else {
            setText(lCol, "");
            setBackgroundColor(lCol, QColor(0xe3, 0x26, 0x36));
        }
    }
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (mAcadVersionOrig) {
        setText(lCol, QString::number(mAcadVersionOrig));
    } else {
        setText(lCol, "No");
    }
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignVCenter);
    lCol++;

    if (mFileInfoOrig.suffix().toLower() == "dwg") {
        if (mXchgFileData) {
            if (mXchgFileData->AcadVersionPrcd()) {
                setText(lCol, QString::number(mXchgFileData->AcadVersionPrcd()));
            } else {
                setText(lCol, "No");
            }
            setBackground(lCol, background(lCol - 1));
        } else {
            setText(lCol, "");
            setBackgroundColor(lCol, QColor(0xe3, 0x26, 0x36));
        }
    }
    setTextAlignment(lCol, Qt::AlignHCenter | Qt::AlignVCenter);
    lCol++;

    int lIdPlot = 0;
    if (mFileInfoOrig.suffix().toLower() == "dwg") {
        QList<tPairIntIntString> lExistingIds1;
        if (gOracle->CollectAlreadyLoaded(mHashOrig, lExistingIds1)) {
            if (!lExistingIds1.isEmpty()) {
                setText(lCol, "Already in Base");
                mWhatToDo = 0; // do not load
                QStringList lStrList;
                for (int j = 0; j < lExistingIds1.length(); j++) {
                    if (!lIdPlot) lIdPlot = lExistingIds1.at(j).first.first;
                    lStrList.append(QString::number(lExistingIds1.at(j).first.first) + "/" + QString::number(lExistingIds1.at(j).first.second));
                }
                setText(lCol + 1, lStrList.join(';'));
            } else {
                QList<tPairIntString> lExistingIds2;
                if (gOracle->CollectByBlockName(mIdProject, mFileInfoOrig.baseName(), lExistingIds2)) {
                    if (lExistingIds2.isEmpty()) {
                        if (!gOracle->CollectByBlockName(0, mFileInfoOrig.baseName(), lExistingIds2)) {
                            // it is mean oracle error
                            mIsError = true;
                            return;
                        }
                    }
                    if (!lExistingIds2.isEmpty()) {
                        setText(lCol, "Load as new version");
                        mWhatToDo = 1;
                        //QStringList lStrList;
                        lIdPlot = lExistingIds2.at(0).first;
                        setText(lCol + 1, QString::number(lIdPlot));
                        if (lExistingIds2.length() > 1) {
                            QFont lFont = font(lCol + 1);
                            lFont.setBold(true);
                            setFont(lCol + 1, lFont);
                        }
                    } else {

                    }
                } else {
                    // it is mean oracle error
                    mIsError = true;
                    return;
                }
            }
        } else {
            // it is mean oracle error
            mIsError = true;
            return;
        }
    } else {
        setText(lCol, "Load as new document");
        mWhatToDo = 3;
    }
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;
    // it's for id
    setTextAlignment(lCol, Qt::AlignRight | Qt::AlignVCenter);
    lCol++;

    if (lIdPlot) {
        PlotData *lPlot = gProjects->FindByIdPlot(lIdPlot);
        if (lPlot) {
            mTDArea = lPlot->TDArea();
            mTDId = lPlot->TDId();
        }
    }
    TreeDataRecord *lTree = gTreeData->FindById(mTDArea, mTDId);
    if (lTree) {
        setText(lCol, lTree->TextConst());
        setToolTip(lCol, lTree->FullName());
    }
    setTextAlignment(lCol, Qt::AlignLeft | Qt::AlignVCenter);
    lCol++;

    GenerateNames();
}
