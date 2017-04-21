#ifndef BLOBMEMCACHE_H
#define BLOBMEMCACHE_H

#include <QObject>
#include <QMap>
#include <QByteArray>

#include <QtSql/QSqlQuery>

#include "def_expimp.h"

class EXP_IMP BlobMemCache : public QObject
{
    Q_OBJECT
protected:
    QMap <int, QByteArray> mDwgCache, mDwgCacheCache;

    explicit BlobMemCache(QObject *parent = 0);
    virtual ~BlobMemCache();
public:
    enum RecordType {
        Dwg, DwgCache
    };

    static BlobMemCache * GetInstance();

    bool IsCached(RecordType rt, int aId) const;
    void AddToCache(RecordType rt, int aId);

    // if add const at start then the result is unpredictable... is it some kind of error???
    QByteArray &GetData(RecordType rt, int aId);

    void Clean();
};

// this object did not self delete on program exit
#define gBlobMemCache BlobMemCache::GetInstance()

#endif // BLOBMEMCACHE_H
