#ifndef ORACLEHELPER_H
#define ORACLEHELPER_H

#include <QtSql/QSqlQuery>

#include <QPair>
#include <QList>
#include <QDateTime>
#include <QVariant>

typedef QPair<int, QString> tPairIntString;
typedef QPair<int, int> tPairIntInt;
typedef QPair<tPairIntInt, QString> tPairIntIntString;

#if defined(LOGIN_LIBRARY)
#  define LOGIN_EXPORT Q_DECL_EXPORT
#else
#  define LOGIN_EXPORT Q_DECL_IMPORT
#endif

class LOGIN_EXPORT OracleHelper
{
protected:
    QSqlQuery *mSqlInsertXref, *mSqlFindXref, *mSqlFindDwg, *mSqlTryMoveToCommon;
    QSqlQuery *mIsNotEditing;

    // it can't change state to editing
    // when the editing done - it is done
    // when need to edit again - the copy of record in dwg made
    QList<int> mDwgNOTEditing;

    explicit OracleHelper();
    virtual ~OracleHelper();

    bool GetSeqVal(QString aSeqName, const QString &aValType, int &aVal) const;
    bool GetSeqVal(QString aSeqName, const QString &aValType, qulonglong &aVal) const;
public:
    static OracleHelper * GetInstance() {
        static OracleHelper * lOracleHelper = NULL;
        if (!lOracleHelper) {
            lOracleHelper = new OracleHelper();
            //qAddPostRoutine(ProjectDataList::clean);
        }
        return lOracleHelper;
    }

    void Clean();

    bool GetSeqNextVal(const QString &aSeqName, int &aVal) const;
    bool GetSeqCurVal(const QString &aSeqName, int &aVal) const;
    bool GetSeqNextVal(const QString &aSeqName, qulonglong &aVal) const;
    bool GetSeqCurVal(const QString &aSeqName, qulonglong &aVal) const;

    bool GetSysTimeStamp(QDateTime &aDateTime) const;

    bool InsertXref(quint64 &aId, quint64 aIdDwg, const QString &aFileName, quint64 aIdXref, int aVersion, const QString &aGrpName = "");
    bool InsertXref(quint64 aIdDwg, const QString &aFileName, quint64 aIdXref, int aVersion, const QString &aGrpName = "");

    bool FindXref(quint64 aIdDwgMain, const QString &aFileName, const QString &aSha256, quint64 &aIdDwgXref, int &aVersion, bool &aShaIsEqual);
    bool FindDwgBySha256(const QString &aSha256, quint64 &aIdDwg);

    bool InsertDwgFile(quint64 aIdDwg, bool aTrueForOut, const QString &aFileName, qint64 aFileSize, const QDateTime & aFileModified, const QString & aSha256 = "");

    bool IsDwgRecordNOTEditingNow(quint64 aIdDwg);

    bool CollectAlreadyLoaded(const QString &aHash, QList<tPairIntIntString> &aExistingIds); // format is ((ID_PLOT, HISTORY), CODE-SHEET)
    bool CollectByBlockName(int aIdProject, const QString &aBlockName, QList<tPairIntString> &aExistingIds); // format is (ID_PLOT, CODE-SHEET)

};

#endif // ORACLEHELPER_H
