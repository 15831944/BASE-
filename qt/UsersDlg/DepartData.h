#ifndef DEPARTDATA_H
#define DEPARTDATA_H

#include <QObject>
#include <QString>
#include <QList>

#include "../VProject/CommonData.h"

class DepartData
{
    DeclareParRO(int, Id)
    DefParStr(Name)
public:
    DepartData(int aId, const QString &aName);

    bool RemoveFromDB();
    bool SaveData();
    void RollbackEdit();
    //void CommitEdit();
};

class DepartDataList : public QObject
{
    Q_OBJECT
protected:
    bool mIsInited;
    QList<DepartData *> mDepartList;

    explicit DepartDataList();
    virtual ~DepartDataList();

public:
    static DepartDataList * GetInstance() {
        static DepartDataList * lDepartDataList = NULL;
        if (!lDepartDataList) {
            lDepartDataList = new DepartDataList();
            //qAddPostRoutine(ProjectList::clean);
        }
        return lDepartDataList;
    }

    void InitDepartList();

    QList<DepartData *> & DepartListRef();
    const QList<DepartData *> & DepartListConst();

    DepartData * FindById(int aId);
signals:
    void DepartsNeedUpdate();
};

#define gDeparts DepartDataList::GetInstance()

#endif // DEPARTDATA_H
