#include "FileUtils.h"
#include "common.h"

#include <QDir>
#include <QDate>
#include <QCryptographicHash>

FileUtils::FileUtils()
{
}

bool FileUtils::IsFileChanged(const QString &aFileName, qint64 aOldFileSize, const QString &aOldSha256) {
    bool res = false;
    QFile file(aFileName);

    if (file.exists()) {
        if (file.size() != aOldFileSize) {
            res = true;
        } else {
            if (file.open(QFile::ReadOnly)) {
                QCryptographicHash hash1(QCryptographicHash::Sha256);
                hash1.addData(&file);
                file.close();

                if (QString(hash1.result().toHex()).toUpper() != aOldSha256.toUpper()) {
                    res = true;
                }
            }
        }
    }

    return res;
}

FILETIME FileUtils::ToWinFileTime(const QDateTime &dateTime) {
    // Definition of FILETIME from MSDN:
    // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
    QDateTime origin(QDate(1601, 1, 1), QTime(0, 0, 0, 0), Qt::UTC);
    // Get offset - note we need 100-nanosecond intervals, hence we multiply by
    // 10000.
    qint64 _100nanosecs = 10000 * origin.msecsTo(dateTime);
    // Pack _100nanosecs into the structure.
    FILETIME fileTime;
    fileTime.dwLowDateTime = _100nanosecs;
    fileTime.dwHighDateTime = (_100nanosecs >> 32);
    return fileTime;
}

void FileUtils::SetFileTime(const QString &aFileName, const QDateTime &aDateTime) {
    HANDLE hFile;
    FILETIME ft = ToWinFileTime(aDateTime);
    QString lFileName = aFileName;
    wchar_t aFN[2048];

    aFN[lFileName.length()] = 0; // toWCharArray doesn't set 0 at the end
    lFileName.replace('/', '\\').toWCharArray(aFN);
    if ((hFile = CreateFile(aFN, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE) {
        ::SetFileTime(hFile, &ft, &ft, &ft);
        if (!::SetFileTime(hFile, &ft, &ft, &ft))
            gLogger->ShowError("Set file time", "Set file time error: " + QString::number(GetLastError()));
        CloseHandle(hFile);
    } else {
        gLogger->ShowError(NULL, "Set file time",
                          QObject::tr("Can't open file") + ":\r\n" + QString::fromWCharArray(aFN) +
                          "\r\n" + QObject::tr("Error") + ": " + QString::number(GetLastError()));
    }
}

bool FileUtils::HasEnoughSpace(const QString &aPath, qulonglong aSpace) {
    ULARGE_INTEGER lFreeSpace;
    wchar_t lDisk[4];

    if (aPath.length() >= 3
            && aPath.at(1) == ':'
            && aPath.at(2) == '/') {

        lDisk[0] = aPath.at(0).unicode();
        lDisk[1] = aPath.at(1).unicode();
        lDisk[2] = aPath.at(2).unicode();
        lDisk[3] = 0;

        if (GetDiskFreeSpaceEx(lDisk, &lFreeSpace, NULL, NULL)
                && lFreeSpace.QuadPart >= (ULONGLONG) aSpace) {
            return true;
        }
    }
    return false;
}

int FileUtils::AcadVersion(const QString &aHead6Char) {
    int lAcadVersion = 0;

    if (aHead6Char == "AC1032") lAcadVersion = 2018;
    else if (aHead6Char == "AC1027") lAcadVersion = 2013;
    else if (aHead6Char == "AC1024") lAcadVersion = 2010;
    else if (aHead6Char == "AC1021") lAcadVersion = 2007;
    else if (aHead6Char == "AC1018") lAcadVersion = 2004;
    else if (aHead6Char == "AC1015") lAcadVersion = 2000;
    else if (aHead6Char.left(4) == "AC10") lAcadVersion = 14;

    return lAcadVersion;
}

int FileUtils::AcadVersion(const QByteArray *aBinData) {
    return AcadVersion(QString::fromLatin1(*aBinData, 6));
}

int FileUtils::AcadVersion(QFile &aFile) {
    char lSmall[12];
    int lAcadVersion = 0;

    aFile.seek(0);
    memset(&lSmall, 0, sizeof(lSmall));
    aFile.read(lSmall, sizeof(lSmall) - 2);

    return AcadVersion(QString::fromLatin1(lSmall, 6));
}

bool FileUtils::InitDataForLoad(bool aIsFile, XchgFileData &aFile, qint64 &aOrigFileSize) {

    QCryptographicHash hash1(QCryptographicHash::Sha256);

    if (aFile.FileInfoOrigConst().canonicalFilePath().isEmpty()) {
        gLogger->LogError("FileUtils::InitDataForLoad - canonicalFilePath() is empty");
        gLogger->LogError(aFile.FileInfoOrigConst().filePath());
    }

    QFile file(aFile.FileInfoOrigConst().canonicalFilePath());

    qDeleteAll(aFile.AddFilesRef());
    aFile.AddFilesRef().clear();

    aOrigFileSize = 0;

    if (aIsFile) {
        if (file.open(QFile::ReadOnly)) {
            aOrigFileSize = file.size();
            aFile.SetBinaryData(new QByteArray(file.readAll()));
            file.close();
            // das ist fantastish!
            hash1.addData(*aFile.BinaryDataConst());

            if (aFile.FileInfoOrigConst().suffix().toLower() == "dwg") {
                aFile.SetAcadVersionOrig(AcadVersion(aFile.BinaryDataConst()));
            }
        } else {
            gLogger->LogError("FileUtils::InitDataForLoad - file " + aFile.FileInfoOrigConst().filePath());
            gLogger->LogError("canonical: " + aFile.FileInfoOrigConst().canonicalFilePath());
            gLogger->ShowError(QObject::tr("Load document - opening file"),
                               QObject::tr("Error opening file") + ":\r\n" + file.fileName() + "\r\n" + QObject::tr("Error") +": " + file.errorString());
            return false;
        }
    } else {
        QStringList lFiles;
        QDir dir(aFile.FileInfoOrigConst().canonicalFilePath());
        lFiles = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
        for (int i = 0; i < lFiles.length(); i++) {
            QFile file(aFile.FileInfoOrigConst().canonicalFilePath() + "/" + lFiles.at(i));
            if (file.open(QFile::ReadOnly)) {
                aOrigFileSize += file.size();
                QByteArray *lData = new QByteArray(file.readAll());
                file.close();

                // common hash for directory
                hash1.addData(*lData);

                // hash for this file
                QCryptographicHash hash2(QCryptographicHash::Sha256);
                //hash2.reset();
                hash2.addData(*lData);

                aFile.AddFilesRef().append(new XchgFileData(QFileInfo(file), lData, QString(hash2.result().toHex()).toUpper()));
            } else {
                gLogger->LogError("FileUtils::InitDataForLoad - directory");
                gLogger->ShowError(QObject::tr("Load document - opening file"),
                                   QObject::tr("Error opening file") + ":\r\n" + file.fileName() + "\r\n" + QObject::tr("Error") +": " + file.errorString());
                return false;
            }
        }
    }

    aFile.SetHashOrig(QString(hash1.result().toHex()).toUpper());
    return true;
}
