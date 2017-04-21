#ifndef GEOBASE_GLOBAL_H
#define GEOBASE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GEOBASE_LIBRARY)
#  define GEOBASESHARED_EXPORT Q_DECL_EXPORT
#else
#  define GEOBASESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GEOBASE_GLOBAL_H
