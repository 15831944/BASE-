#ifndef USERRIGHT_H
#define USERRIGHT_H

#include <QString>
#include <QMap>

#if defined(VPROJECT_MAIN_IMPORT)
    #define USERSDLG_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(USERSDLG_LIBRARY)
        #define USERSDLG_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define USERSDLG_LIBRARY_EXP_IMP
    #endif
#endif

class UserRightData
{
public:
    enum UserRightAction { URAInsert, URASelect, URAUpdate, URADelete, URAPriv };
protected:
    UserRightAction mAction;
    QString mTable, mColumn;
public:
    explicit UserRightData(UserRightAction aAction, const QString &aTable, const QString &aColumn);

    bool operator==(const UserRightData & other) const;
    bool operator<(const UserRightData & other) const;

    // need not in fact
//    const UserRightData& operator= (const UserRightData& src) {
//        CopyFrom(src);
//        return src;
//    }

//    virtual void CopyFrom(const UserRightData& src) {
//        mAction = src.mAction;
//        mTable = src.mTable;
//        mColumn = src.mColumn;
//    }
};

class USERSDLG_LIBRARY_EXP_IMP UserRight
{
protected:
    QMap<UserRightData, bool> mRights;
    QMap<QString, QString> mTablesFound;

    explicit UserRight();

    bool CheckRightInternalInDb(UserRightData::UserRightAction aAction, const QString &aTableName, const QString &aColumnName);
    bool CheckRightInternal(UserRightData::UserRightAction aAction, const QString &aTableName, const QString &aColumnName);

    bool CanAnyColumn(UserRightData::UserRightAction aAction, const QString &aTableName);
public:
    static UserRight * GetInstance() {
        static UserRight * lUserRight = NULL;
        if (!lUserRight) {
            lUserRight = new UserRight();
            //qAddPostRoutine(ProjectList::clean);
        }
        return lUserRight;
    }

    bool CanInsert(const QString &aTableName, const QString &aColumnName = "");
    bool CanSelect(const QString &aTableName);
    bool CanUpdate(const QString &aTableName, const QString &aColumnName = "");
    bool CanDelete(const QString &aTableName);

    bool CanUpdateAnyColumn(const QString &aTableName);
    bool CanInsertAnyColumn(const QString &aTableName);

    bool FindTableName(const QString &aTableStart, QString &aTableName);

    bool HasColumn(const QString &aTableName, const QString &aColumnName);

    bool HasPrivelege(const QString &aPrivelegeName);

    bool CanCreateUser();
    bool CanManageUser();
    bool CanDropUser();
    bool CanGrantAnyRole();

    void UpdateRightsOnServer();
};

#define gUserRight UserRight::GetInstance()

// temporary
#define gHasVersionExt UserRight::GetInstance()->HasColumn("v_plot_simple", "version_ext")

#endif // USERRIGHT_H
