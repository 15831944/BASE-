#include "FileLoadData.h"

#include "../VProject/GlobalSettings.h"

#include <QFileInfo>

FileLoadData::FileLoadData(const QString &aFileName) :
    QTreeWidgetItem(),
    mLoad(false), mId(0),
    mFileNameFull(aFileName)
{
    setFlags(flags() | Qt::ItemIsEditable);

    mFileName = mFileNameFull.section('/', -1);
    setText(0, mFileName);
    InitFileData();

    setText(1, "нов");
    setTextAlignment(1, Qt::AlignCenter);

    setText(2, mFileDate.toString("dd.MM.yyyy hh:mm:ss"));
    setTextAlignment(2, Qt::AlignCenter);

    setText(3, gSettings->FormatNumber(mFileSize));
    setTextAlignment(3, Qt::AlignRight);

    setText(5, aFileName);
}

FileLoadData::FileLoadData(int aId, const QString &aFileName, const QString &aFileNameFull,
                           const QDateTime &aFileDate, qint64 aFileSize, const QString &aComments, const QString &aSha256) :
    QTreeWidgetItem(),
    mLoad(false), mId(aId),
    mFileName(aFileName),
    mFileNameFull(aFileNameFull),
    mFileDate(aFileDate),
    mFileSize(aFileSize),
    mComments(aComments),
    mSha256(aSha256)
{
    setFlags(flags() | Qt::ItemIsEditable);

    setText(0, aFileName);

    setText(2, mFileDate.toString("dd.MM.yyyy hh:mm:ss"));
    setTextAlignment(2, Qt::AlignCenter);

    setText(3, gSettings->FormatNumber(mFileSize));
    setTextAlignment(3, Qt::AlignRight);

    setText(4, aComments);

    setText(5, aFileNameFull);
}

void FileLoadData::ReloadFrom(const QString &aFileName) {
    if (!aFileName.isEmpty()) mFileNameFull = aFileName;

    mLoad = true;
    // use old file name!
    //mFileName = mFileNameFull.section('/', -1);
    //setText(0, mFileName);
    InitFileData();

    setText(1, "загр");
    setTextAlignment(1, Qt::AlignCenter);

    setText(2, mFileDate.toString("dd.MM.yyyy hh:mm:ss"));
    setTextAlignment(2, Qt::AlignCenter);

    setText(3, gSettings->FormatNumber(mFileSize));
    setTextAlignment(3, Qt::AlignRight);

    setText(5, mFileNameFull);
}

void FileLoadData::InitFileData()
{
    QFile file(mFileNameFull);
    QFileInfo fileinfo(file);

    mFileDate = fileinfo.lastModified();

    if (file.open(QFile::ReadOnly)) {
        mFileSize = file.size();
        // das ist fantastish!
        QCryptographicHash hash1(QCryptographicHash::Sha256);
        hash1.addData(&file);

        mSha256 = QString(hash1.result().toHex()).toUpper();

//        file.seek(0);
//        char lData[8];
//        memset(lData, 0, sizeof(lData));
//        if (file.read(lData, 6) == 6) {
//            QString verStr(lData);
//            if (verStr == "AC1027") AcadVersion = 2013;
//            else if (verStr == "AC1024") AcadVersion = 2010;
//            else if (verStr == "AC1021") AcadVersion = 2007;
//            else if (verStr == "AC1018") AcadVersion = 2004;
//            else if (verStr == "AC1015") AcadVersion = 2000;
//            else if (verStr.left(4) == "AC10") AcadVersion = 14;
//            else AcadVersion = -1;
//        }


//        // ungzip test - YES IT IS WORKING
//        file.seek(60);
//        QByteArray orig, umcompressed;
//        orig = file.readAll();
//        const unsigned int size = 0; //orig.size();

//        orig.prepend((char) ((size >> 24) & 0xFF));
//        orig.prepend((char) ((size >> 16) & 0xFF));
//        orig.prepend((char) ((size >> 8) & 0xFF));
//        orig.prepend((char) ((size >> 0) & 0xFF));
//        umcompressed = qUncompress(orig);

        file.close();

//        file.setFileName(file.fileName() + ".txt");
//        if (file.open(QFile::WriteOnly)) {
//            file.write(umcompressed);
//            file.close();
//        };
    }
}
