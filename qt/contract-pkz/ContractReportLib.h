#ifndef CONTRACTREPORTLIB_H
#define CONTRACTREPORTLIB_H

typedef struct {
    int MNForName, MNForNum;
    int Year;
} GetMonthNameDATA, *PGetMonthNameDATA;

void GetMonthNameNumCallback(const QSqlQuery *aQuery, const QString &aFieldName, int aRecordNum, QVariant &aValue, void *aDataPtr);
void ColorToRedExceptPayment(const QSqlQuery *aQuery, const QString &aFieldName, QString &aNewStyleId);

void MakeQuerySingleYear(bool aWithFeaturedPay, QQueryForExcelData *&aQueryYears, PGetMonthNameDATA aMonthData);
void MakeQueryYears(bool aWithFeaturedPay, QQueryForExcelData *&aQueryYears, const QString &aAndWhere);
void MakeQueryProjects(bool aWithFeaturedPay, QQueryForExcelData *&aQueryProjects, const QList<int> &aProjects,
                       const QString &aAndWhere, int aSumQueryNum, int aRestQueryNum, bool aWithTotal, bool aContractSumAsChildSum);
void MakeQueryRest(bool aWithFeaturedPay, QQueryForExcelData *&aQueryRests, const QString &aAndWhere);
void MakeQueryContracts(bool aWithFeaturedPay, QQueryForExcelData *&aQueryContracts, const QString &aAndWhere, int aSumQueryNum);
void MakeQueryPayments(bool aWithFeaturedPay, QQueryForExcelData *&aQueryPayments, const QString &aAndWhere, bool aSortDesc);

#endif // CONTRACTREPORTLIB_H

