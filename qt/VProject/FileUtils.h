#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QFileInfo>
#include <QObject>

#define NOMINMAX
#include <windows.h>

#include "AcadXchgDialog.h"

#include "def_expimp.h"

class EXP_IMP FileUtils
{
protected:
    explicit FileUtils();
    ~FileUtils();

public:
    static FileUtils * GetInstance() {
        static FileUtils * lFileUtils = NULL;
        if (!lFileUtils) {
            lFileUtils = new FileUtils();
            //qAddPostRoutine(GlobalSettings::clean);
        }
        return lFileUtils;
    }

    bool IsFileChanged(const QString &aFileName, qint64 aOldFileSize, const QString &aOldSha256);

    FILETIME ToWinFileTime(const QDateTime &dateTime);
    void SetFileTime(const QString &aFileName, const QDateTime &aDateTime);

    bool HasEnoughSpace(const QString &aPath, qulonglong aSpace);

    int AcadVersion(const QString &aHead6Char);
    int AcadVersion(const QByteArray *aBinData);
    int AcadVersion(QFile &aFile);

    bool InitDataForLoad(bool aIsFile, XchgFileData &aFile, qint64 &aOrigFileSize);

};

#define gFileUtils FileUtils::GetInstance()

#endif // FILEUTILS_H
