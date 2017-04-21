#ifndef ORACLE_H
#define ORACLE_H

#include "def_expimp.h"

bool CreateDocument(int &aIdPlot, int aIdProject, int aIdTypeArea, int aIdType, QString aBlockName, QString aCode,
                    QString aNameTop, QString aName, QString aExtension, QString aFilename);

EXP_IMP bool GetTreeTypeForGroup(int aIdGroup, int &aArea, int &aType);
EXP_IMP bool GetNextPlotCode(int aArea, int aType, int aIdProject, QString &aCode);

#endif // ORACLE_H
