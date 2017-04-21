#ifndef COMMON_H
#define COMMON_H

#include <QMessageBox>

#include "../Login/Login.h"
#include "../Logger/logger.h"

//#define ERROR_COLOR (QColor(0xfa, 0x00, 0x03))
#define MY_COLOR_ERROR (QColor(0xe3, 0x26, 0x36))
#define MY_COLOR_WARNING (QColor(0xff, 0xaa, 0xaa))
#define MY_COLOR_DISABLED (QColor(0xaa, 0xaa, 0xaa))

#define gHasModule(aModuleName) (QFile::exists(QApplication::applicationDirPath() + "/" + aModuleName + ".dll"))

#define qInt(FieldName) query.value(FieldName).toInt()
#define qUInt(FieldName) query.value(FieldName).toUInt()
#define qString(FieldName) query.value(FieldName).toString()
#define qDate(FieldName) query.value(FieldName).toDate()
#define qDouble(FieldName) query.value(FieldName).toDouble()

//Q_DECLARE_METATYPE(QList<int>)

#endif // COMMON_H
