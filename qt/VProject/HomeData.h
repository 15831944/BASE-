#ifndef HOMEDATA_H
#define HOMEDATA_H

#include <QString>
#include <QMap>

class HomeData
{
protected:
    QMap<QString, QString> mData;

    HomeData();
public:
    static HomeData * GetInstance() {
        static HomeData * lHomeData = NULL;
        if (!lHomeData) {
            lHomeData = new HomeData();
            //qAddPostRoutine(ProjectList::clean);
        }
        return lHomeData;
    }

    QString Get(const QString &aName);
    void Set(const QString &aName, const QString &aValue);

};

#define gHomeData HomeData::GetInstance()

#endif // HOMEDATA_H
