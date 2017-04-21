#include "MSOffice.h"
#include "common.h"

#include <QDateTime>
#include <QCoreApplication>
#include <QSettings>
#include <QRegularExpression>

MSOfficeHelper::MSOfficeHelper() :
    mIsWordInstalled(-1), mIsExcelInstalled(-1)
{
}

int MSOfficeHelper::IsInstalledInternal(const QString &lSubKeyname) {
    QSettings lProduct("HKEY_CURRENT_USER\\Software\\Microsoft\\Office", QSettings::NativeFormat);
    QStringList lGroups = lProduct.childGroups();

    QString lTempPath = QCoreApplication::applicationDirPath().remove(QRegularExpression("/[^/]*$")).remove(QRegularExpression("/[^/]*$")) + "\\temp\\";
    lTempPath.replace('/', '\\');

    foreach (QString lVersion, lGroups) {
        lProduct.beginGroup(lVersion);
        if (lProduct.childGroups().contains(lSubKeyname)) {
            lProduct.beginGroup(lSubKeyname);
            if (!lProduct.value(lSubKeyname + "Name").isNull()) {
                if (lProduct.childGroups().contains("Security")) {
                    lProduct.beginGroup("Security");
                    if (lProduct.childGroups().contains("Trusted Locations")) {
                        lProduct.beginGroup("Trusted Locations");
                        bool lIsFound = false;

                        foreach (QString lLocation, lProduct.childGroups()) {
                            lProduct.beginGroup(lLocation);
                            if (lProduct.value("Path").toString() == lTempPath) lIsFound = true;
                            lProduct.endGroup();
                        }
                        if (!lIsFound) {
                            lProduct.beginGroup("Location" + QString::number(lProduct.childGroups().length()));
                            lProduct.setValue("Path", lTempPath);
                            lProduct.setValue("AllowSubfolders", 1L);
                            lProduct.setValue("Date", QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm"));
                            lProduct.setValue("Description", "Documents from Projects Base");
                            lProduct.endGroup();
                        }
                    }
                }

                return 1;
            }
            lProduct.endGroup();
        }
        lProduct.endGroup();
    }

//    foreach (QString lVersion, lProduct.childGroups()) {
//        QSettings lProduct2("HKEY_CURRENT_USER\\Software\\Microsoft\\Office\\" + lVersion, QSettings::NativeFormat);
//        if ((ind = lProduct2.childGroups().indexOf(lSubKeyname)) != -1) {
//            QSettings lProduct3("HKEY_CURRENT_USER\\Software\\Microsoft\\Office\\" + lVersion + "\\" + lProduct2.childGroups().at(ind), QSettings::NativeFormat);
//            if (!lProduct3.value(lSubKeyname + "Name").isNull()) {
//                gLogger->ShowError(QObject::tr("Project data"), lProduct3.value(lSubKeyname + "Name").toString());
//                return 1;
//            }
//        }
//    }
    return 0;
}

bool MSOfficeHelper::IsWordInstalled() {
    if (mIsWordInstalled == -1) {
        mIsWordInstalled = IsInstalledInternal("Word");
    }
    return mIsWordInstalled == 1;
}

bool MSOfficeHelper::IsExcelInstalled() {
    if (mIsExcelInstalled == -1) {
        mIsExcelInstalled = IsInstalledInternal("Excel");
    }
    return mIsExcelInstalled == 1;
}
