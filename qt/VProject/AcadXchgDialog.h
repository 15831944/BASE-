#ifndef ACADXCHGDIALOG_H
#define ACADXCHGDIALOG_H

#include "qfcdialog.h"

#include "../PlotLib/CDwgLayout.h"
//#include "ProjectData.h"

#include <QFileInfo>
#include <QMessageBox>

#define NOMINMAX
#include <windows.h>

#include "def_expimp.h"

class XchgFileData;

class XchgFileDataList : public QList<XchgFileData *>
{
public:
    ~XchgFileDataList();
};

class EXP_IMP XchgFileData {
public:
    enum enumProcessStatus { NotDone, Started, DoneWithError, DoneWithWarning, Done };
protected:
    enumProcessStatus mProcessStatus;
    int mIdDwg;
    QString mGroup;
    QFileInfo mFileInfoOrig, mFileInfoPrcd;
    int mAcadVersionOrig, mAcadVersionPrcd;
    QString mHashOrig, mHashPrcd;
    QByteArray *mBinaryData;

    QList<CDwgLayout *> mDwgLayouts;

    XchgFileDataList mAddFiles;
public:
    explicit XchgFileData(const QFileInfo &aFileInfoOrig);
    explicit XchgFileData(const QString &aFileNameOrig, const QString &aGroup);
    explicit XchgFileData(const QFileInfo &aFileInfo, QByteArray *aBinaryData, const QString & aHash);
    virtual ~XchgFileData();

    enumProcessStatus ProcessStatus() const;
    void SetProcessStatus(enumProcessStatus aProcessStatus);

    int IdDwg() const;
    void SetIdDwg(int aIdDwg);

    const QString & Group() const;

    const QFileInfo &FileInfoOrigConst() const;
    QFileInfo &FileInfoOrigRef();

    const QFileInfo &FileInfoPrcdConst() const;
    QFileInfo &FileInfoPrcdRef();

    int AcadVersionOrig() const;
    void SetAcadVersionOrig(int aAcadVersionOrig);

    int AcadVersionPrcd() const;
    void SetAcadVersionPrcd(int aAcadVersionPrcd);

    const QString &HashOrigConst() const;
    void SetHashOrig(const QString &aHashOrig);

    const QString &HashPrcdConst() const;
    void SetHashPrcd(const QString &aHashPrcd);

    const QByteArray *BinaryDataConst() const;
    QByteArray *BinaryDataRef();
    void SetBinaryData(QByteArray *aBinaryData);

    const QList<CDwgLayout *> &DwgLayoutsConst() const;
    QList<CDwgLayout *> &DwgLayoutsRef();

    const XchgFileDataList &AddFilesConst() const;
    XchgFileDataList &AddFilesRef();
};

class EXP_IMP AcadXchgDialog
{
protected:
    bool mShowDataErrors;
    XchgFileData *mFileCurrent;
    XchgFileDataList mFiles;

    bool DoNativeEvent(const QByteArray & eventType, void * message, long * result);
public:
    explicit AcadXchgDialog();
    virtual ~AcadXchgDialog();

    const XchgFileDataList &FilesConst() const;
    XchgFileDataList &FilesRef();

    bool ProcessDwgsForLoad(ULONG aProcessType, ULONG aColorBlocks, ULONG aColorEntities, ULONG aLWBlocks, ULONG aLWEntities,
                            const QString &aLayer0Name,  const QString &aUserCommands, WId aWinId);
signals:

public slots:

};

#endif // ACADXCHGDIALOG_H
