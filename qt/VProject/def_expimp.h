#ifndef DEF_EXPIMP_H
#define DEF_EXPIMP_H

#if defined(VPROJECT_MAIN_IMPORT)
#  define EXP_IMP Q_DECL_IMPORT
#else
#if defined(VPROJECT_MAIN)
#  define EXP_IMP Q_DECL_EXPORT
#else
#  define EXP_IMP
#endif
#endif

#endif // DEF_EXPIMP_H
