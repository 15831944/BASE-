#include "xreftypedata.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QVariant>
#include <QMessageBox>

QList <XrefTypeData> XrefTypeList;

//XrefTypeData::XrefTypeData() :
//    id (0)
//{
//}

XrefTypeData::XrefTypeData(int aId, QString aFilename, QString aDescription) {
    id =aId;
    Filename = aFilename;
    Description = aDescription;
}

bool XrefTypeData::IsType(const QString &aFilename, QString &aNumber) const {
    int i;
    QString lFN = aFilename.section('/', -1);
    lFN = lFN.toLower();
    lFN.replace("output", "");

    if (lFN.indexOf(Filename + ".", Qt::CaseInsensitive) != -1) return true;

    if ((i = lFN.indexOf(Filename, Qt::CaseInsensitive)) != -1) {
        // make number string
        aNumber.clear();
        i += Filename.length();
        while (i < lFN.length() && lFN[i] >= '0' && lFN[i] <= '9') {
            aNumber += lFN[i];
            i++;
        }
        return true;
    } else {
        return false;
    }
}

void InitXrefTypeList(QWidget *aParentWidget) {
    XrefTypeList.clear();

    QSqlQuery query("SELECT ID, FILENAME, DESCRIPTION FROM V_XREF_LIST_T", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(aParentWidget, "InitXrefTypeList", query.lastError());
    } else {
        while (query.next()) {
            XrefTypeData xreftypedata(query.value(0).toInt(), query.value(1).toString(), query.value(2).toString());
            XrefTypeList.append(xreftypedata);
        }
    }
}
