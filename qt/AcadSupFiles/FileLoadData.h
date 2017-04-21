#ifndef FILELOADDATA_H
#define FILELOADDATA_H

#include <QTreeWidgetItem>
#include <QDateTime>

class FileLoadData : public QTreeWidgetItem
{
protected:
    bool mLoad;
    int mId;
    QString mFileName, mFileNameFull;
    QDateTime mFileDate;
    qint64 mFileSize;
    QString mComments, mSha256;

    void InitFileData();

public:
    explicit FileLoadData(const QString &aFileName);
    explicit FileLoadData(int aId, const QString &aFileName, const QString &aFileNameFull,
                          const QDateTime &aFileDate, qint64 aFileSize,
                          const QString &aComments, const QString &aSha256);

    void ReloadFrom(const QString &aFileName);

    bool Load() const { return mLoad; }
    int Id() const { return mId; }
    QString FileNameShort() const { return mFileName; }
    QString FileNameFull() const { return mFileNameFull; }
    QDateTime FileDate() const { return mFileDate; }
    qint64 FileSize() const { return mFileSize; }
    QString Comments() const { return mComments; }
    QString Sha256() const { return mSha256; }


signals:

public slots:

};

#endif // FILELOADDATA_H
