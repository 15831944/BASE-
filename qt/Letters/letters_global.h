#ifndef LETTERS_GLOBAL_H
#define LETTERS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LETTERS_LIBRARY)
#  define LETTERSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LETTERSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LETTERS_GLOBAL_H
