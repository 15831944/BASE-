#include "common.h"
#include "oracle.h"
#include "GlobalSettings.h"
#include "../PlotLib/DwgData.h"

#include <QFileInfo>




// debug333 - deprecated, need to rewrite
bool CreateDocument(int &aIdPlot, int aIdProject, int aIdTypeArea, int aIdType, QString aBlockName, QString aCode,
                    QString aNameTop, QString aName, QString aExtension, QString aFilename) {

    bool res = false;
    quint64 lIdDwg;

    if (!gOracle->GetSeqNextVal("plot_id_seq", aIdPlot)
            || !gOracle->GetSeqNextVal("dwg_id_seq", lIdDwg)) return false;

    QFile data(aFilename);
    if (data.open(QFile::ReadOnly)) {
        QByteArray buffer(data.readAll());
        data.close();

        QCryptographicHash lHash(QCryptographicHash::Sha256);
        lHash.addData(buffer);

        if (!db.transaction()) {
            gLogger->ShowSqlError("CreateDocument", QObject::tr("Can't start transaction"), db);
        }

        QSqlQuery qInsert(db);

        qInsert.prepare(
                    "insert into v_plot_simple (id, id_project, type_area, type, block_name, code, nametop, name, extension)"
                    " values (?, ?, ?, ?, ?, ?, ?, ?, ?)");
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError("Creating document - prepare", qInsert);
        } else {
            qInsert.addBindValue(aIdPlot);
            qInsert.addBindValue(aIdProject);
            qInsert.addBindValue(aIdTypeArea);
            qInsert.addBindValue(aIdType);
            qInsert.addBindValue(aBlockName);
            qInsert.addBindValue(aCode);
            qInsert.addBindValue(aNameTop);
            qInsert.addBindValue(aName);
            qInsert.addBindValue(aExtension);
            if (!qInsert.exec()) {
                gLogger->ShowSqlError("Creating document - execute", qInsert);
            } else {
                QFileInfo fileinfo(data);
                if (DwgData::INSERT(lIdDwg, aIdPlot, 0, aExtension, QString(lHash.result().toHex()).toUpper(), 0, -1, fileinfo.lastModified(), &buffer)
                        && gOracle->InsertDwgFile(lIdDwg, false, aFilename, data.size(), fileinfo.lastModified())) {
                    res = true;
                }
            }
        }
    }

    if (res) {
        res = db.commit();
    }

    if (!res) {
        db.rollback();
    }

    return res;
}

bool GetTreeTypeForGroup(int aIdGroup, int &aArea, int &aType) {
    bool res = false;
    QSqlQuery query("select area, id from v_treedata where group_id = " + QString::number(aIdGroup), db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Get type", query);
    } else {
        if (query.next()) {
            aArea = query.value(0).toInt();
            aType = query.value(1).toInt();
            res = true;
        } else {
            gLogger->ShowError("Get type", "No tree type found for group " + QString::number(aIdGroup));
        }
    }

    return res;
}

bool GetNextPlotCode(int aArea, int aType, int aIdProject, QString &aCode) {
    bool res = false;
    QSqlQuery query(
                "select pp.getPlotCode("
                + QString::number(aArea) + ", "
                + QString::number(aType) + ", "
                + QString::number(aIdProject) + ", null) from dual", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Get next plot code", query);
    } else {
        if (query.next()) {
            aCode = query.value(0).toString();
            res = true;
        } else {
            gLogger->ShowError("Get next plot code", "No data!");
        }
    }

    return res;
}
