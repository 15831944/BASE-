#include "GlobalSettings.h"
#include "common.h"
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QCoreApplication>

//http://www1.cbs.gov.il/reader/?MIval=%2Fprices_db%2FMachshevon_Results.html&MD=a&MySubject=37&MyCode=11120010&MultMin=19511016&MultMax=20131205&DateMin=16%2F10%2F1951&DateMax=05%2F12%2F2013&ssum=1000&koeff=1&Days_2=5&Months_2=12&Years_2=2013&Days_1=1&Months_1=1&Years_1=2000
//http://www1.cbs.gov.il/reader/?MIval=%2Fprices_db%2FMachshevon_Results.html&MD=d&MySubject=53&MyCode=11240010&MultMin=19830216&MultMax=20131205&DateMin=16%2F02%2F1983&DateMax=05%2F12%2F2013&ssum=1000&koeff=1&Days_2=5&Months_2=12&Years_2=2013&Days_1=1&Months_1=1&Years_1=2000

void GlobalSettings::CalcMadadFromSiteGetUrl(int aIndexingType, QDate dateFirst, QDate dateLast, QString aSum, QString &aUrl)
{
    QStringList reqFields;
    switch (aIndexingType) {
    case 0:
        reqFields << "MIval=%2Fprices_db%2FMachshevon_Results_E.html"
                  << "MD=a"
                  << "MySubject=37"
                  << "MyCode=11120010"
                  << "MultMin=19511016"
                  << QString("MultMax=") + QDate::currentDate().toString("yyyyMMdd")
                  << "DateMin=16%2F10%2F1951";
        break;
    case 1:
        reqFields << "MIval=%2Fprices_db%2FMachshevon_Results_E.html"
                  << "MD=d"
                  << "MySubject=53"
                  << "MyCode=11240010"
                  << "MultMin=19830216"
                  << QString("MultMax=") + QDate::currentDate().toString("yyyyMMdd")
                  << "DateMin=16%2F02%2F1983";
        break;
    }

    reqFields
            << QString("DateMax=") + QDate::currentDate().toString("dd%2FMM%2Fyyyy")
            << QString("ssum=") + aSum
            << "koeff=1"
            << QString("Days_1=") + dateFirst.toString("dd")
            << QString("Months_1=") + dateFirst.toString("MM")
            << QString("Years_1=") + dateFirst.toString("yyyy")
            << QString("Days_2=") + dateLast.toString("dd")
            << QString("Months_2=") + dateLast.toString("MM")
            << QString("Years_2=") + dateLast.toString("yyyy");

    aUrl = QString("http://www1.cbs.gov.il/reader/?") + reqFields.join('&');
}

void GlobalSettings::CalcMadadFromSite(int aIndexingType, QDate dateFirst, QDate dateLast, QString aSum, QLineEdit *leForIndexed)
{
    QString strUrl;
    CalcMadadFromSiteGetUrl(aIndexingType, dateFirst, dateLast, aSum, strUrl);

    lineEdits.append(leForIndexed);
    leForIndexed->setStyleSheet("background-color: #444444;");

    replies.append(am.get(QNetworkRequest(QUrl(strUrl))));
    connect(replies.at(replies.count() - 1), SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
}

void GlobalSettings::slotReadyRead()
{
    //gLogger->ShowError("חוזים", "QString::number(reply->error())");
    for (int i = replies.length() - 1; i >= 0; i--) {
        //if (replies.at(i)->waitForReadyRead(1000)) {
            QString replyStr;
            replyStr = replies.at(i)->readAll();
            //gLogger->ShowSqlError("חשבון", QString::number(replyStr.length()));

            if (replyStr.indexOf("font color=\"red\"") != -1) {
                replyStr = replyStr.mid(replyStr.indexOf("font color=\"red\""));
                if (replyStr.indexOf(">") != -1
                        && replyStr.indexOf("&") != -1) {

                    replyStr = replyStr.mid(replyStr.indexOf(">") + 1, replyStr.indexOf("&") - replyStr.indexOf(">") - 1);
                    if (replyStr.indexOf('.') == -1) replyStr += ".";
                    while (replyStr.indexOf('.') != replyStr.length() - 3) replyStr += "0";
                    replyStr.remove(',');
                    lineEdits.at(i)->setText(replyStr);
                    lineEdits.at(i)->setStyleSheet("");

                    replies.at(i)->abort();
                    replies.at(i)->deleteLater();
                    replies.removeAt(i);
                    lineEdits.removeAt(i);
                }
            }
        //}
    }
}

void GlobalSettings::CalcMadadAbort()
{
    for (int i = replies.length() - 1; i >= 0; i--) {
        replies.at(i)->abort();
        replies.at(i)->deleteLater();
        replies.removeAt(i);
        lineEdits.at(i)->setStyleSheet("");
        lineEdits.removeAt(i);
    }
}
