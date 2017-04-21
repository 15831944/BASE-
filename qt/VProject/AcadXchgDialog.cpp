#include "AcadXchgDialog.h"

#include "GlobalSettings.h"

#include "FileUtils.h"

#include"common.h"

#include <QFile>

XchgFileDataList::~XchgFileDataList() {
    qDeleteAll(*this);
    clear();
}

//--------------------------------------------------
XchgFileData::XchgFileData(const QFileInfo &aFileInfoOrig) :
    mProcessStatus(NotDone),
    mFileInfoOrig(aFileInfoOrig), mFileInfoPrcd(aFileInfoOrig),
    mAcadVersionOrig(0), mAcadVersionPrcd(0),
    mBinaryData(NULL)
{
}

XchgFileData::XchgFileData(const QString &aFileNameOrig, const QString &aGroup) :
    mProcessStatus(NotDone),
    mFileInfoOrig(aFileNameOrig), mFileInfoPrcd(aFileNameOrig),
    mAcadVersionOrig(0), mAcadVersionPrcd(0),
    mGroup(aGroup),
    mBinaryData(NULL)
{
}

XchgFileData::XchgFileData(const QFileInfo &aFileInfo, QByteArray *aBinaryData, const QString & aHash) :
    mProcessStatus(NotDone),
    mFileInfoOrig(aFileInfo), mFileInfoPrcd(aFileInfo),
    mAcadVersionOrig(0), mAcadVersionPrcd(0),
    mBinaryData(aBinaryData),
    mHashOrig(aHash), mHashPrcd(aHash)
{
}

XchgFileData::~XchgFileData() {
    if (mBinaryData) delete mBinaryData;
    qDeleteAll(mDwgLayouts);
}

XchgFileData::enumProcessStatus XchgFileData::ProcessStatus() const {
    return mProcessStatus;
}

void XchgFileData::SetProcessStatus(enumProcessStatus aProcessStatus) {
    mProcessStatus = aProcessStatus;
}

int XchgFileData::IdDwg() const {
    return mIdDwg;
}

void XchgFileData::SetIdDwg(int aIdDwg) {
    mIdDwg = aIdDwg;
}

const QString &XchgFileData::Group() const {
    return mGroup;
}

const QFileInfo &XchgFileData::FileInfoOrigConst() const {
    return mFileInfoOrig;
}

QFileInfo &XchgFileData::FileInfoOrigRef() {
    return mFileInfoOrig;
}

const QFileInfo &XchgFileData::FileInfoPrcdConst() const {
    return mFileInfoPrcd;
}

QFileInfo &XchgFileData::FileInfoPrcdRef() {
    return mFileInfoPrcd;
}

int XchgFileData::AcadVersionOrig() const {
    return mAcadVersionOrig;
}

void XchgFileData::SetAcadVersionOrig(int aAcadVersionOrig) {
    mAcadVersionOrig = aAcadVersionOrig;
}

int XchgFileData::AcadVersionPrcd() const {
    return mAcadVersionPrcd;
}

void XchgFileData::SetAcadVersionPrcd(int aAcadVersionPrcd) {
    mAcadVersionPrcd = aAcadVersionPrcd;
}

const QString &XchgFileData::HashOrigConst() const {
    return mHashOrig;
}

void XchgFileData::SetHashOrig(const QString &aHashOrig) {
    mHashOrig = aHashOrig;
}

const QString &XchgFileData::HashPrcdConst() const {
    return mHashPrcd;
}

void XchgFileData::SetHashPrcd(const QString &aHashPrcd) {
    mHashPrcd = aHashPrcd;
}

const QByteArray *XchgFileData::BinaryDataConst() const {
    return mBinaryData;
}

QByteArray *XchgFileData::BinaryDataRef() {
    return mBinaryData;
}

void XchgFileData::SetBinaryData(QByteArray *aBinaryData) {
    if (mBinaryData) delete mBinaryData;
    mBinaryData = aBinaryData;
}

const QList<CDwgLayout *> &XchgFileData::DwgLayoutsConst() const {
    return mDwgLayouts;
}

QList<CDwgLayout *> &XchgFileData::DwgLayoutsRef() {
    return mDwgLayouts;
}

const XchgFileDataList &XchgFileData::AddFilesConst() const {
    return mAddFiles;
}

XchgFileDataList &XchgFileData::AddFilesRef() {
    return mAddFiles;
}

//--------------------------------------------------
AcadXchgDialog::AcadXchgDialog() :
    mShowDataErrors(true), mFileCurrent(NULL)
{
}

AcadXchgDialog::~AcadXchgDialog() {
}

bool AcadXchgDialog::DoNativeEvent(const QByteArray & eventType, void * message, long * result) {
    if (((MSG *) message)->message == WM_COPYDATA) {
        PCOPYDATASTRUCT CpyData;

        CpyData = (PCOPYDATASTRUCT) (((MSG *) message)->lParam);
        switch (CpyData->dwData) {
        case 0:
            if (CpyData->cbData == RecordDataFromAcad::GetDataSize()) {
                RecordDataFromAcad lRecordDataFromAcad;

                memcpy(lRecordDataFromAcad.GetDataBuffer(), CpyData->lpData, CpyData->cbData);

                switch (lRecordDataFromAcad.DataType()) {
                case RecordDataFromAcad::MainFile:
                    {
                        QString lOldFileName = lRecordDataFromAcad.MFFileNameOld().replace('\\', '/');
                        bool lIsFound;
                        lIsFound = false;

                        for (int i = 0; i < mFiles.length(); i++) {
                            if (!mFiles.at(i)->FileInfoOrigConst().canonicalFilePath().compare(lOldFileName, Qt::CaseInsensitive)) {
                                mFileCurrent = mFiles.at(i);
                                mFileCurrent->SetProcessStatus(XchgFileData::Started);
                                mFileCurrent->FileInfoPrcdRef() = QFileInfo(lRecordDataFromAcad.MFFileNameNew());
                                lIsFound = true;
                                *result = RET_OK;
                                break;
                            }
                        }
                        if (mShowDataErrors && !lIsFound) {
                            gLogger->ShowError("WM_COPYDATA 0", "Data from RecordDataFromAcad: file " + lRecordDataFromAcad.MFFileNameOld()
                                               + " not expected!");
                        }
                    }
                    break;
                case RecordDataFromAcad::MainFileEnd:
                    if (mFileCurrent) {
                        if (!mFileCurrent->FileInfoOrigConst().canonicalFilePath().compare(lRecordDataFromAcad.MFFileNameOld().replace('\\', '/'), Qt::CaseInsensitive)) {
                            mFileCurrent->SetProcessStatus(XchgFileData::Done);
                            mFileCurrent = NULL;
                            *result = RET_OK;
                        } else if (mShowDataErrors) {
                            gLogger->ShowError("WM_COPYDATA 0", "Data from RecordDataFromAcad: file " + lRecordDataFromAcad.MFFileNameOld()
                                               + " not expected for ending!");
                        }
                    } else {
                        gLogger->ShowError("WM_COPYDATA 0", "Data from RecordDataFromAcad: current main file is undefined!");
                    }
                    break;
                case RecordDataFromAcad::AdditionalFile:
                    if (mFileCurrent) {
                        mFileCurrent->AddFilesRef().append(new XchgFileData(lRecordDataFromAcad.AFFileName(), lRecordDataFromAcad.AFGroup()));
                        *result = RET_OK;
                    } else if (mShowDataErrors) {
                        gLogger->ShowError("WM_COPYDATA 0", "Data from RecordDataFromAcad: current main file is undefined!");
                    }
                    break;
                }

                return true;
            } else {
                gLogger->ShowError("WM_COPYDATA 0", "Invalid size for RecordDataFromAcad: " + QString::number(CpyData->cbData)
                                      + ", expected " + QString::number(RecordDataFromAcad::GetDataSize()));
            }
            break;
        case 1:
            if (CpyData->cbData == CDwgLayout::GetDataSize()) {
                if (mFileCurrent) {
                    mFileCurrent->DwgLayoutsRef().append(new CDwgLayout(CpyData->lpData));
                    *result = RET_OK;
                } else if (mShowDataErrors) {
                    gLogger->ShowError("WM_COPYDATA 1", "Data from RecordDataFromAcad: current main file is undefined!");
                }
                return true;
            } else {
                gLogger->ShowError("WM_COPYDATA 1", "Invalid size for CDwgLayout: " + QString::number(CpyData->cbData)
                                   + ", expected " + QString::number(CDwgLayout::GetDataSize()));
            }
            break;
        case 2:
            if (mFileCurrent) {
                if (!mFileCurrent->DwgLayoutsConst().isEmpty()) {
                    if (CpyData->cbData == CDwgLayoutBlock::GetDataSize()) {
                        // append to last
                        mFileCurrent->DwgLayoutsConst().at(mFileCurrent->DwgLayoutsConst().length() - 1)->BlocksRef().append(new CDwgLayoutBlock(CpyData->lpData));
                        *result = RET_OK;
                        return true;
                    } else {
                        gLogger->ShowError("WM_COPYDATA 2", "Invalid size for CDwgLayoutBlock: " + QString::number(CpyData->cbData)
                                           + ", expected " + QString::number(CDwgLayoutBlock::GetDataSize()));
                    }
                } else {
                    gLogger->ShowError("WM_COPYDATA 2", "DWG layout block data received while layouts list is empty!");
                }
            } else if (mShowDataErrors) {
                gLogger->ShowError("WM_COPYDATA 2", "Data from RecordDataFromAcad: current main file is undefined!");
            }
            break;
        case 3:
            if (mFileCurrent) {
                if (!mFileCurrent->DwgLayoutsConst().isEmpty()) {
                    if (!mFileCurrent->DwgLayoutsConst().at(mFileCurrent->DwgLayoutsConst().length() - 1)->BlocksConst().isEmpty()) {
                        if (CpyData->cbData == CDwgLBAttr::GetDataSize()) {
                            // append to last
                            mFileCurrent->DwgLayoutsConst().at(mFileCurrent->DwgLayoutsConst().length() - 1)->BlocksRef().at(mFileCurrent->DwgLayoutsConst().at(mFileCurrent->DwgLayoutsConst().length() - 1)->BlocksConst().length() - 1)->AttrsRef().append(new CDwgLBAttr(CpyData->lpData));
                            *result = RET_OK;
                            return true;
                        } else {
                            gLogger->ShowError("WM_COPYDATA 3", "Invalid size for CDwgLBAttr: " + QString::number(CpyData->cbData)
                                               + ", expected " + QString::number(CDwgLBAttr::GetDataSize()));
                        }
                    } else {
                        gLogger->ShowError("WM_COPYDATA 3", "DWG layout block attribute data received while block list is empty!");
                    }
                } else {
                    gLogger->ShowError("WM_COPYDATA 3", "DWG layout block attribute data received while layouts list is empty!");
                }
            } else if (mShowDataErrors) {
                gLogger->ShowError("WM_COPYDATA 3", "Data from RecordDataFromAcad: current main file is undefined!");
            }
            break;
//        default:
//            gLogger->ShowError("WM_COPYDATA " + QString::number(CpyData->dwData), "Unexpected data: " + QString::number(CpyData->dwData)
//                                  + ", size " + QString::number(CpyData->cbData));
//            break;
        }
    }
    return false;
}

const XchgFileDataList &AcadXchgDialog::FilesConst() const {
    return mFiles;
}

XchgFileDataList &AcadXchgDialog::FilesRef() {
    return mFiles;
}

bool AcadXchgDialog::ProcessDwgsForLoad(ULONG aProcessType, ULONG aColorBlocks, ULONG aColorEntities, ULONG aLWBlocks, ULONG aLWEntities,
                        const QString &aLayer0Name,  const QString &aUserCommands, WId aWinId) {
    bool res = false;

    mFileCurrent = NULL;

    MainDataForCopyToAcad lDataForAcad(aProcessType, aColorBlocks, aColorEntities, aLWBlocks, aLWEntities, aLayer0Name, aUserCommands, aWinId);
    for (int i = 0; i < mFiles.length(); i++) {
        lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eLP, mFiles.at(i)->FileInfoOrigConst().canonicalFilePath()));
        mFiles.at(i)->SetProcessStatus(XchgFileData::NotDone);
    }

    gSettings->DoOpenDwgNew(lDataForAcad); // process in AutoCAD

    for (int i = 0; i < mFiles.length(); i++) {
        XchgFileData *lMainFileData = mFiles.at(i);
        QFile lFile(lMainFileData->FileInfoPrcdConst().canonicalFilePath());
        if (lFile.open(QFile::ReadOnly)) {
            lMainFileData->SetBinaryData(new QByteArray(lFile.readAll()));
            lFile.close();

            QCryptographicHash lHash(QCryptographicHash::Sha256);
            lHash.addData(*lMainFileData->BinaryDataConst());
            lMainFileData->SetHashPrcd(QString(lHash.result().toHex()).toUpper());
            lMainFileData->SetAcadVersionPrcd(gFileUtils->AcadVersion(lMainFileData->BinaryDataConst()));

            res = true;

            for (int j = 0; j < lMainFileData->AddFilesConst().length(); j++) {
                XchgFileData *lAddFileData = lMainFileData->AddFilesConst().at(j);
                QFile lAddFile(lAddFileData->FileInfoPrcdConst().canonicalFilePath());
                if (lAddFile.open(QFile::ReadOnly)) {
                    lAddFileData->SetBinaryData(new QByteArray(lAddFile.readAll()));
                    lAddFile.close();

                    QCryptographicHash hash2(QCryptographicHash::Sha256);
                    hash2.addData(*lAddFileData->BinaryDataConst());
                    lAddFileData->SetHashPrcd(QString(hash2.result().toHex()).toUpper());
                } else {
                    gLogger->LogError("AcadXchgDialog::ProcessDwgsForLoad - add. file");
                    gLogger->ShowError(QObject::tr("Load document - opening file"),
                                       QObject::tr("Error opening file") + " " + QString::number(j) + ":\r\n" + lAddFile.fileName() + "\r\n" + QObject::tr("Error") +": " + lAddFile.errorString());
                    res = false;
                    break;
                }
            }
        } else {
            gLogger->LogError("AcadXchgDialog::ProcessDwgsForLoad - main file");
            gLogger->ShowError(QObject::tr("Load document - opening file"),
                               QObject::tr("Error opening file") + " " + QString::number(i) + ":\r\n" + lFile.fileName() + "\r\n" + QObject::tr("Error") +": " + lFile.errorString());
            res = false;
            break;
        }
    }

    return res;
}
