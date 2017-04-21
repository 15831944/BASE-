#ifndef XREFTYPEDATA_H
#define XREFTYPEDATA_H

#include <QWidget>

void InitXrefTypeList(QWidget *aParentWidget);

class XrefTypeData
{
public:
    //XrefTypeData();
    XrefTypeData(int aId, QString aFilename, QString aDescription);

    bool IsType(const QString &aFilename, QString &aNumber) const;
    int GetId() const { return id; }
    QString GetFilename() const { return Filename; }
protected:
    int id;
    QString Filename, Description;
};

#endif // XREFTYPEDATA_H
