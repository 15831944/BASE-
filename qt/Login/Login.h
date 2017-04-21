#ifndef LOGIN_H
#define LOGIN_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#if defined(LOGIN_LIBRARY)
#  define LOGIN_EXPORT Q_DECL_EXPORT
#else
#  define LOGIN_EXPORT Q_DECL_IMPORT
#endif

LOGIN_EXPORT QSqlDatabase db;

LOGIN_EXPORT bool Login(QString &aSelectedLang, QString &aSchemaName, QString &aBaseName);

#include "OracleHelper.h"

#define gOracle OracleHelper::GetInstance()

#endif // LOGIN_H
