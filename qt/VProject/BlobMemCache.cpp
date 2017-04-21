#include "BlobMemCache.h"
#include "GlobalSettings.h"
#include "common.h"
#include "oracle.h"
#include "FileUtils.h"

#include <QFile>

BlobMemCache::BlobMemCache(QObject *parent) :
    QObject(parent)
{

}

BlobMemCache::~BlobMemCache() {
    Clean();
}

void BlobMemCache::Clean() {
    mDwgCache.clear();
    mDwgCacheCache.clear();
}

BlobMemCache *BlobMemCache::GetInstance() {
    static BlobMemCache * lBlobMemCache = NULL;
    if (!lBlobMemCache) {
        lBlobMemCache = new BlobMemCache();
        //qAddPostRoutine(ProjectList::clean);
    }
    return lBlobMemCache;
}

bool BlobMemCache::IsCached(RecordType rt, int aId) const {
    switch(rt) {
    case Dwg: return mDwgCache.contains(aId);
    case DwgCache: return mDwgCacheCache.contains(aId);
    }
    return true;
}

void BlobMemCache::AddToCache(RecordType rt, int aId) {
    QFile data;

    switch(rt) {
    case Dwg:
        if (!mDwgCache.contains(aId)) {
            data.setFileName(gSettings->LocalCache.Path + "d-" + QString::number(aId));
        }
        break;
    case DwgCache:
        if (!mDwgCacheCache.contains(aId)) {
            data.setFileName(gSettings->LocalCache.Path + "c-" + QString::number(aId));
        }
        break;
    }

    if (data.fileName().isEmpty()) {
        return;
    }

    QByteArray lData;
    if (gSettings->LocalCache.UseLocalCache) {
        // try load from local cache
        if (data.open(QFile::ReadOnly)) {
            lData = data.readAll();
            data.close();

            switch(rt) {
            case Dwg:
                //gLogger->ShowError("BlobMemCache::AddToCache", "DWR Loaded from LC: " + QString::number(aId) + " = " + QString::number(lData.length()), false);
                mDwgCache.insert(aId, lData);
                break;
            case DwgCache:
                //gLogger->ShowError("BlobMemCache::AddToCache", "DWG_CACHE Loaded from LC: " + QString::number(aId) + " = " + QString::number(lData.length()), false);
                mDwgCacheCache.insert(aId, lData);
                break;
            }
            gFileUtils->SetFileTime(data.fileName(), QDateTime::currentDateTime());
            return;
        }
    }

    // load from db
    QSqlQuery query(db);

    switch(rt) {
    case Dwg:
        query.prepare("select data from v_dwg a where id = ?");
        break;
    case DwgCache:
        query.prepare("select data from v_dwg_cache a where id = ?");
        break;
    }

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("BlobMemCache::AddToCache - prepare", "type: " + QString::number(rt) + ", id: " + QString::number(aId), query);
    } else {
        query.addBindValue(aId);
        if (!query.exec()) {
            gLogger->ShowSqlError("BlobMemCache::AddToCache - execute", "type: " + QString::number(rt) + ", id: " + QString::number(aId), query);
        } else {
            if (query.next()) {
                lData = query.value("data").toByteArray();

                if (gSettings->LocalCache.UseLocalCache) {
                    if (gFileUtils->HasEnoughSpace(gSettings->LocalCache.Path, gSettings->LocalCache.MinDiskSize)) {
                        bool lNotEditing;

                        if (rt == Dwg) {
                            lNotEditing = gOracle->IsDwgRecordNOTEditingNow(aId);
                        } else {
                            // no ediditing state for non-dwg data
                            lNotEditing = true;
                        }
                        if (lNotEditing) {
                            if (data.open(QFile::WriteOnly)) {
                                data.write(lData);
                                data.close();
                            }
                        }
                    } else {
                        // not enough space, remove
                        data.remove();
                    }
                }

                switch(rt) {
                case Dwg:
                    //gLogger->ShowError("BlobMemCache::AddToCache", "DWG Loaded from base: " + QString::number(aId) + " = " + QString::number(lData.length()));
                    mDwgCache.insert(aId, lData);
                    break;
                case DwgCache:
                    //gLogger->ShowSqlError("BlobMemCache::AddToCache", "DWG_CACHE Loaded from base: " + QString::number(aId) + " = " + QString::number(lData.length()));
                    mDwgCacheCache.insert(aId, lData);
                    break;
                }
            } else {
                gLogger->ShowError("BlobMemCache::AddToCache", "Data not found!");
            }
        }
    }
}

QByteArray &BlobMemCache::GetData(RecordType rt, int aId) {
    if (!IsCached(rt, aId)) AddToCache(rt, aId);

    switch(rt) {
    case Dwg: return mDwgCache[aId];
    case DwgCache: return mDwgCacheCache[aId];
    }
}
