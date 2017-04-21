#include "common.h"
#include "MainWindow.h"

#include "GlobalSettings.h"
#include "HomeData.h"
#include "../AcadSupFiles/AcadSupFiles.h"

#include "ChooseAcadDlg.h"

#include "../UsersDlg/UserRight.h"

#include <QApplication>
#include <QProcess>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QSpacerItem>
#include <QGridLayout>

long msg_CheckLoadedInAcad;
//bool gIsSPDS;

#pragma pack(push, 1)
typedef struct {
    long WhatToDo; // 0 - check, 1 - connect
    char Login[36]; // user name
    char Password[36]; // password
    char ConnectionString[36]; // connection string (ORACLE), database (POSTGRES)
    char SchemaName[36]; // schema name (ORACLE), ip (POSTGRES)
    char ModuleName[256]; // module name (ORACLE), port (POSTGRES)
} ORACLELOGINDATA, *PORACLELOGINDATA;
#pragma pack(pop)

void ShowSystemError(LONG error, const wchar_t *aPrefix = NULL) {
    wchar_t ErrBuf[2048];
    wchar_t errStr[4096];

    if (FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                ErrBuf, _countof(ErrBuf), NULL)) {
        if (aPrefix) {
            swprintf_s(errStr, L"%s - %s", aPrefix, ErrBuf);
        } else {
            swprintf_s(errStr, L"%s", ErrBuf);
        };
        gLogger->ShowError("Running AutoCAD", QString::fromWCharArray(errStr));
    } else {
        gLogger->ShowError("Running AutoCAD", "ShowSystemError: error FormatMessage!");
    }
}

void GlobalSettings::EnumProductsFromKeyInternal(const wchar_t *MainKey, QList<InstalledAcadData *> &aInstalledAcadList) {
    LONG retval;
    HKEY AcadKey, AcadSubKey, AcadSubKey2;
    wchar_t SubKeyStr[2048], MySubKeyStr[2048],
        SubKeyStr2[2048], MySubKeyStr2[2048];
    long i, j, k;
    unsigned long SubKeySize, SubKeySize2;
    wchar_t ProductName[2048], Language[2048], AcadPath[2048];
    DWORD ProductNameSize, ProductNameType, LanguageSize, LanguageType, AcadPathSize, AcadPathType;

    REGSAM wowBlya = 0;

    BOOL lIsWow64;
#ifndef _M_X64
    lIsWow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &lIsWow64);
#else
    lIsWow64 = TRUE;
#endif

    for (k = 0; k < 2; k++) {
        if (lIsWow64) {
            if (!k)
                wowBlya = KEY_WOW64_32KEY;
            else
                wowBlya = KEY_WOW64_64KEY;
        } else {
            if (k) break;
            // it is simple 32 bit, no fucking "wow"
            wowBlya = 0;
        };

        retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, MainKey, 0, KEY_READ | wowBlya, &AcadKey);

        if (retval != ERROR_SUCCESS) {
            // autocad (or viewer) not installed
            //ShowSystemError(retval);
            continue;
        };

        for (i = 0; ; i++) {
            SubKeySize = sizeof(SubKeyStr);
            if (RegEnumKeyEx(AcadKey, i, SubKeyStr, &SubKeySize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                if (wcsstr(SubKeyStr, L"R") == SubKeyStr) {
                    wcscpy_s(MySubKeyStr, MainKey);
                    wcscat_s(MySubKeyStr, L"\\");
                    wcscat_s(MySubKeyStr, SubKeyStr);

                    retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, MySubKeyStr, 0, KEY_READ | wowBlya, &AcadSubKey);
                    if (retval != ERROR_SUCCESS) {
                        RegCloseKey(AcadKey);
                        ShowSystemError(retval);
                        continue;
                    };

                    InstalledAcadData *lAcadRegData = NULL;
                    for (j = 0; ; j++) {
                        SubKeySize2 = sizeof(SubKeyStr2);
                        if (RegEnumKeyEx(AcadSubKey, j, SubKeyStr2, &SubKeySize2, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                            if (wcsstr(SubKeyStr2, L"ACAD") == SubKeyStr2
                                    || wcsstr(SubKeyStr2, L"DWGV") == SubKeyStr2
                                    || wcsstr(SubKeyStr2, L"dwgv") == SubKeyStr2) {

                                wcscpy_s(MySubKeyStr2, MySubKeyStr);
                                wcscat_s(MySubKeyStr2, L"\\");
                                wcscat_s(MySubKeyStr2, SubKeyStr2);

                                retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, MySubKeyStr2, 0, KEY_READ | wowBlya, &AcadSubKey2);

                                if (retval != ERROR_SUCCESS) {
                                    continue;
                                }

                                ProductNameSize = sizeof(ProductName);
                                retval = RegQueryValueEx(AcadSubKey2, L"ProductName", NULL,
                                    &ProductNameType, (unsigned char *) &ProductName, &ProductNameSize);
                                if (retval != ERROR_SUCCESS) {
                                    RegCloseKey(AcadSubKey2);
                                    continue;
                                }

                                LanguageSize = sizeof(Language);
                                retval = RegQueryValueEx(AcadSubKey2, L"Language", NULL,
                                    &LanguageType, (unsigned char *) &Language, &LanguageSize);
                                if (retval != ERROR_SUCCESS) {
                                    RegCloseKey(AcadSubKey2);
                                    continue;
                                }

                                AcadPathSize = sizeof(AcadPath);
                                retval = RegQueryValueEx(AcadSubKey2, L"AcadLocation", NULL,
                                    &AcadPathType, (unsigned char *) &AcadPath, &AcadPathSize);
                                if (retval != ERROR_SUCCESS) {
                                    RegCloseKey(AcadSubKey2);
                                    continue;
                                }

                                RegCloseKey(AcadSubKey2);

                                if (!wcsstr(ProductName, Language)) {
                                    wcscat_s(ProductName, L" - ");
                                    wcscat_s(ProductName, Language);
                                }

                                //QMessageBox::critical(NULL, "", QString::fromWCharArray(MySubKeyStr2));

                                lAcadRegData = new InstalledAcadData(QString::fromWCharArray(ProductName), QString::fromWCharArray(MySubKeyStr2),
                                                                QString::fromWCharArray(AcadPath), wowBlya);
                                break;
                            }
                        } else {
                            break;
                        }
                    }

                    if (lAcadRegData) {
                        QStringList lInstalledProducts, lInstalledLangs;
                        wcscat_s(MySubKeyStr, L"\\InstalledProducts");
                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, MySubKeyStr, 0, KEY_READ | wowBlya, &AcadSubKey2) == ERROR_SUCCESS) {
                            for (j = 0; ; j++) {
                                SubKeySize2 = sizeof(SubKeyStr2);
                                if (RegEnumKeyEx(AcadSubKey2, j, SubKeyStr2, &SubKeySize2, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                                    //QMessageBox::critical(NULL, "", QString::fromWCharArray(SubKeyStr2));
                                    DWORD lIndex = 0;

                                    // enumerate
                                    HKEY lAcadSubKey3;
                                    wchar_t lProductSubKey[2048];
                                    wcscpy_s(lProductSubKey, MySubKeyStr);
                                    wcscat_s(lProductSubKey, L"\\");
                                    wcscat_s(lProductSubKey, SubKeyStr2);

                                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lProductSubKey, 0, KEY_READ | wowBlya, &lAcadSubKey3) == ERROR_SUCCESS) {
                                        wchar_t lValueName[128], lData[128];
                                        DWORD lValueNameSize = _countof(lValueName);
                                        DWORD lDataSize = sizeof(lData);
                                        while (RegEnumValue(lAcadSubKey3, lIndex, (LPWSTR) &lValueName, &lValueNameSize, NULL, NULL, (LPBYTE) &lData, &lDataSize) == ERROR_SUCCESS) {
                                            lInstalledProducts.append(QString::fromWCharArray(SubKeyStr2));
                                            lInstalledLangs.append(QString::fromWCharArray(lValueName));

                                            lValueNameSize = _countof(lValueName);
                                            lDataSize = _countof(lData);
                                            lIndex++;
                                        }
                                        RegCloseKey(lAcadSubKey3);
                                    }

                                    if (!lIndex) {
                                        lInstalledProducts.append(QString::fromWCharArray(SubKeyStr2));
                                        lInstalledLangs.append("");
                                    }
                                } else {
                                    break;
                                }
                            }
                            RegCloseKey(AcadSubKey2);
                        }

                        if (lInstalledProducts.contains("C3D")) {
                            for (j = lInstalledProducts.length() - 1; j >= 0; j--)
                                if (lInstalledProducts.at(j) == "MAP") {
                                    lInstalledProducts.removeAt(j);
                                    lInstalledLangs.removeAt(j);
                                }
                        }

                        bool lDiffProduct = false, lDiffLang = false;
                        if (lInstalledProducts.length() > 1) {
                            for (j = 0; j < lInstalledProducts.length() - 1; j++) {
                                if (lInstalledProducts.at(j) != lInstalledProducts.at(j + 1)) lDiffProduct = true;
                                if (lInstalledLangs.at(j) != lInstalledLangs.at(j + 1)) lDiffLang = true;
                            }
                        }

                        lAcadRegData->SetDiffProduct(lDiffProduct);
                        lAcadRegData->SetDiffLang(lDiffLang);

                        if (!lInstalledProducts.isEmpty()) {
                            lAcadRegData->SetProduct(lInstalledProducts.at(0));
                            lAcadRegData->SetLang(lInstalledLangs.at(0));
                        }
                        aInstalledAcadList.append(lAcadRegData);
                        if (lInstalledProducts.length() > 1) {
                            for (j = 1; j < lInstalledProducts.length(); j++) {
                                InstalledAcadData *lAcadRegDataNew = new InstalledAcadData(lAcadRegData);
                                lAcadRegDataNew->SetProduct(lInstalledProducts.at(j));
                                lAcadRegDataNew->SetLang(lInstalledLangs.at(j));
                                aInstalledAcadList.append(lAcadRegDataNew);
                            }
                        }
                    }

                    RegCloseKey(AcadSubKey);
                }
            } else {
                break;
            }
        }
        RegCloseKey(AcadKey);
    }
}

void GlobalSettings::EnumInstalledProducts(QList<InstalledAcadData *> &aInstalledAcadList) {
    EnumProductsFromKeyInternal(L"SOFTWARE\\Autodesk\\AutoCAD", aInstalledAcadList);
}

static bool CreateKey(HKEY OpenedKey, const wchar_t *SubKey, HKEY &TempKey, REGSAM aFoundRegSam) {
    DWORD Disposition;
    LONG retval = RegCreateKeyEx(OpenedKey,
                        SubKey, 0, L"", REG_OPTION_NON_VOLATILE,
                        KEY_WRITE | KEY_READ | aFoundRegSam, NULL, &TempKey, &Disposition);

    if (retval != ERROR_SUCCESS) {
        ShowSystemError(retval);
        return false;
    }
    return true;
}

static bool CreateValue(HKEY TempKey, const wchar_t *VName, DWORD dwtype, const void *VValue,
                 DWORD length) {
    LONG retval = RegSetValueEx(TempKey, VName, 0, dwtype,
                            (const unsigned char *) VValue, length);
    if (retval != ERROR_SUCCESS) {
        ShowSystemError(retval);
        return false;
    }
    return true;
}

bool GlobalSettings::SetAutoLoadARX(const InstalledAcadData *aAcadRegData) {
    LONG retval;
    HKEY AcadKey;
    wchar_t wAcadKeyStr[2048], SubKey[2048];
    wchar_t SuppPath[4096], SuppPathNew[4096];
    DWORD SuppPathSize, SuppPathType;
    long i, j;
    long AcadVersion;

    AcadVersion = 0;

    const QString &AcadKeyStr = aAcadRegData->KeyConst();
    REGSAM lRegSam = aAcadRegData->FuckingWow();

    if (AcadKeyStr.indexOf("\\R22.0\\") > 0) {
        AcadVersion = 18;
    } else if (AcadKeyStr.indexOf("\\R21.0\\") > 0) {
        AcadVersion = 17;
    } else if (AcadKeyStr.indexOf("\\R20.1\\") > 0) {
        AcadVersion = 16;
    } else if (AcadKeyStr.indexOf("\\R20.0\\") > 0) {
        AcadVersion = 15;
    } else if (AcadKeyStr.indexOf("\\R19.1\\") > 0) {
        AcadVersion = 14;
    } else if (AcadKeyStr.indexOf("\\R19.0\\") > 0) {
        AcadVersion = 13;
    } else if (AcadKeyStr.indexOf("\\R18.2\\") > 0) {
        AcadVersion = 12;
    } else if (AcadKeyStr.indexOf("\\R18.1\\") > 0) {
        AcadVersion = 11;
    } else if (AcadKeyStr.indexOf("\\R18.0\\") > 0) {
        AcadVersion = 10;
    } else if (AcadKeyStr.indexOf("\\R17.2\\") > 0) {
        AcadVersion = 9;
    } else if (AcadKeyStr.indexOf("\\R17.1\\") > 0) {
        AcadVersion = 8;
    } else if (AcadKeyStr.indexOf("\\R17.0\\") > 0) {
        AcadVersion = 7;
    }

    if (AcadVersion == 0) {
        gLogger->ShowError("Running AutoCAD", "Unknown version of AutoCAD: " + AcadKeyStr);
        return false;
    }

    wAcadKeyStr[AcadKeyStr.length()] = 0;
    AcadKeyStr.toWCharArray(wAcadKeyStr);

    wchar_t ArxNameOnly[128], ArxFile[MAX_PATH];

    wchar_t lAddPaalications[][24] = {L"ProjOraLoading", L"MosPro", L"DrawPileRus", L"ExplodeProxy", L""};
    DWORD lLoadCtrls[] = {2, 13, 4, 4};
    wchar_t lLoaders[][36] = {
        L"loading%i"L".arx",
        L"MosPro\\MosPro%i"L".arx",
        L"DrawPileRus%i"L".arx",
        L"ExplodeProxy\\ExplodeProxy%i"L".arx"
    };
    wchar_t lDescriptions[][64] = {
        L"Vladimir's interface with Oracle",
        L"Vladimir's bridge geometry calculation",
        L"Albert's drawing piles (PKZ)",
        L"Explode proxy by Alexander Rivilis (rivilis@mail.ru)"
    };
    wchar_t lGroups[][18] = {L"", L"SHCH_MOSPRO", L"AS_PILES", L"EXPLODEPROXY"};
    wchar_t lAddCommands[][12][18] = {
        {L""},

        {L"MPCALCCHOICE", L"MPSETS", L"MPDEFINEBRIDGE",
        L"MPCURVELENGTH", L"MPCURVEDIVIDE", L"MPDIST2D", L"MPMIDLINES", L""},

        {L"AS_PILERU", L"AS_CHECK", L""},

        {L"EXPLODEALLPROXY", L"REMOVEALLPROXY", L""}
    };

    QString qStrBinPath;
    qStrBinPath = QCoreApplication::applicationDirPath();
    qStrBinPath.resize(qStrBinPath.lastIndexOf(QChar('/')));

    wchar_t lWBinPath[512];
    qStrBinPath.replace('/', '\\');
    lWBinPath[qStrBinPath.length()] = 0;
    qStrBinPath.toWCharArray(lWBinPath);

    for (i = 0; wcslen(lAddPaalications[i]); i++) {
        wcscpy_s(SubKey, wAcadKeyStr);
        wcscat_s(SubKey, L"\\Applications\\");
        wcscat_s(SubKey, lAddPaalications[i]);

        //if (!CreateKey(HKEY_LOCAL_MACHINE, SubKey, AcadKey, lRegSam)) {
        if (!CreateKey(HKEY_CURRENT_USER, SubKey, AcadKey, 0)) {
            QMessageBox::critical(NULL, "", "Can't create key in HKCU: " + QString::fromWCharArray(SubKey));
            return false;
        }

        if (!CreateValue(AcadKey, L"LOADCTRLS", REG_DWORD, &lLoadCtrls[i], sizeof(lLoadCtrls[i]))) {
            RegCloseKey(AcadKey);
            return false;
        }

        wcscpy_s(ArxFile, lWBinPath);
        // there is no such sing as 64 bit now (09 nov 2014)
#ifdef _M_X64
        // in 64 bits, if 32 registry - remove 64 from path
        if (lRegSam == KEY_WOW64_32KEY && (wcsstr(ArxFile, L"64") == &ArxFile[wcslen(ArxFile) - 2])) ArxFile[wcslen(ArxFile) - 2] = 0;
#else
        // in 32bits, if 64 registry - then add 64 to bin path
        if (lRegSam == KEY_WOW64_64KEY && (wcsstr(ArxFile, L"64") != &ArxFile[wcslen(ArxFile) - 2])) wcscat_s(ArxFile, L"64");
#endif
        wcscat_s(ArxFile, L"\\");
        swprintf_s(ArxNameOnly, lLoaders[i], AcadVersion);
        wcscat_s(ArxFile, ArxNameOnly);

        if (!CreateValue(AcadKey, L"LOADER", REG_SZ, ArxFile, wcslen(ArxFile) * 2 + 2)) {
            RegCloseKey(AcadKey);
            return false;
        }

        if (!CreateValue(AcadKey, L"DESCRIPTION", REG_SZ, lDescriptions[i], wcslen(lDescriptions[i]) * 2 + 2)) {
            RegCloseKey(AcadKey);
            return false;
        }

        RegCloseKey(AcadKey);

        if (wcslen(lGroups[i])) {
            wcscpy_s(SubKey, wAcadKeyStr);
            wcscat_s(SubKey, L"\\Applications\\");
            wcscat_s(SubKey, lAddPaalications[i]);
            wcscat_s(SubKey, L"\\Groups");
//            if (!CreateKey(HKEY_LOCAL_MACHINE, SubKey, AcadKey, lRegSam)) {
            if (!CreateKey(HKEY_CURRENT_USER, SubKey, AcadKey, 0)) {
                QMessageBox::critical(NULL, "", "Can't create key in HKCU: " + QString::fromWCharArray(SubKey));
                return false;
            }

            if (!CreateValue(AcadKey, lGroups[i], REG_SZ, lGroups[i], wcslen(lGroups[i]) * 2 + 2)) {
                RegCloseKey(AcadKey);
                return false;
            }

            RegCloseKey(AcadKey);

            wcscpy_s(SubKey, wAcadKeyStr);
            wcscat_s(SubKey, L"\\Applications\\");
            wcscat_s(SubKey, lAddPaalications[i]);
            wcscat_s(SubKey, L"\\Commands");
//            if (!CreateKey(HKEY_LOCAL_MACHINE, SubKey, AcadKey, lRegSam)) {
            if (!CreateKey(HKEY_CURRENT_USER, SubKey, AcadKey, 0)) {
                QMessageBox::critical(NULL, "", "Can't create key in HKCU: " + QString::fromWCharArray(SubKey));
                return false;
            }

            for (j = 0; wcslen(lAddCommands[i][j]); j++) {
                if (!CreateValue(AcadKey, lAddCommands[i][j], REG_SZ, lAddCommands[i][j], wcslen(lAddCommands[i][j]) * 2 + 2)) {
                    RegCloseKey(AcadKey);
                    return false;
                }
            }

        }
    }

    swprintf_s(SubKey, L"%s\\Applications\\ExplodeProxy%i\\Commands", wAcadKeyStr, AcadVersion);
    RegDeleteKey(HKEY_CURRENT_USER, SubKey);
    *wcsrchr(SubKey, L'\\') = 0;
    RegDeleteKey(HKEY_CURRENT_USER, SubKey);

    // KP3 menu - coping files
/*	if (!CopyDir((CString) BinPath + L"\\Menu\\", (CString) AcadPath + L"\\Support\\")) {
        ShowError(L"Error coping menu files!");
        return false;
    }
*/
    HKEY MenuKey;
    wchar_t ProfileName[128];
    unsigned long ProfileNameSize;

    wcscpy_s(SubKey, wAcadKeyStr);
    wcscat_s(SubKey, L"\\Profiles\\");
    if ((retval = RegOpenKeyEx(HKEY_CURRENT_USER, SubKey, 0, KEY_READ | lRegSam, &AcadKey)) != ERROR_SUCCESS) {
        ShowSystemError(retval);
        return false;
    }

    i = 0;
    while (true) {
        // цикл по профайлам
        ProfileNameSize = sizeof(ProfileName);
        retval = RegEnumKeyEx(AcadKey, i, ProfileName, &ProfileNameSize, NULL, NULL, NULL, NULL);
        if ((retval != ERROR_SUCCESS)
                && (retval != ERROR_MORE_DATA)
                && (retval != ERROR_NO_MORE_ITEMS)) {
            RegCloseKey(AcadKey);
            ShowSystemError(retval);
            return false;
        }
        if (retval == ERROR_NO_MORE_ITEMS) break;

        //gIsSPDS = wcscmp(ProfileName, L"<<SPDS>>") == 0;

        // add support path
        wcscpy_s(SubKey, wAcadKeyStr);
        wcscat_s(SubKey, L"\\Profiles\\");
        wcscat_s(SubKey, ProfileName);
        wcscat_s(SubKey, L"\\General\\");

        if ((retval = RegOpenKeyEx(HKEY_CURRENT_USER, SubKey, 0, KEY_WRITE | KEY_READ | lRegSam, &MenuKey)) == ERROR_SUCCESS) {
            SuppPathSize = sizeof(SuppPath);
            if ((retval = RegQueryValueEx(MenuKey, L"ACAD", NULL, &SuppPathType, (unsigned char *) &SuppPath, &SuppPathSize)) == ERROR_SUCCESS) {
                wcscpy_s(ArxFile, lWBinPath);
                *(wcsrchr(ArxFile, L'\\') + 1) = 0;
                wcscat_s(ArxFile, L"temp\\support");

                CreateDirectory(ArxFile, NULL);

                if (!wcsstr(SuppPath, ArxFile)) {
                    swprintf_s(SuppPathNew, L"%s;%s", ArxFile, SuppPath);
                    CreateValue(MenuKey, L"ACAD", REG_SZ, SuppPathNew, wcslen(SuppPathNew) * 2 + 2);
                }
            }

            RegCloseKey(MenuKey);
        }
        // end of add support path

        // autoload - aec base, different file for different versions
        wcscpy_s(SubKey, wAcadKeyStr);
        wcscat_s(SubKey, L"\\Profiles\\");
        wcscat_s(SubKey, ProfileName);
        wcscat_s(SubKey, L"\\Dialogs\\Appload\\Startup\\");

        bool lStartupSubkeyOk;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, SubKey, 0, KEY_WRITE | KEY_READ | lRegSam, &MenuKey) == ERROR_SUCCESS) {
            lStartupSubkeyOk = true;
        } else {
            lStartupSubkeyOk = CreateKey(HKEY_CURRENT_USER, SubKey, MenuKey, lRegSam);
        }

        if (lStartupSubkeyOk) {
            QString lAecArx;

            switch (aAcadRegData->Version()) {
            case 2007:
                lAecArx = aAcadRegData->PathConst() + "\\AecUiBase50.arx";
                break;

            // it is checked - the same name for this versions
            case 2011:
            case 2012:
            case 2013:
            case 2014:
            case 2015:
            case 2016:
            case 2017:
            case 2018:
                lAecArx = aAcadRegData->PathConst() + "\\AecLoader.arx";
                break;
            }

            if (!lAecArx.isEmpty()) {
                //
                bool lIsFound = false;
                DWORD lIndex = 0;
                wchar_t lValueName[128], lData[128];
                DWORD lValueNameSize = _countof(lValueName);
                DWORD lDataSize = sizeof(lData);
                while (RegEnumValue(MenuKey, lIndex, (LPWSTR) &lValueName, &lValueNameSize, NULL, NULL, (LPBYTE) &lData, &lDataSize) == ERROR_SUCCESS) {
                    if (!lAecArx.compare(QString::fromWCharArray(lData), Qt::CaseInsensitive)) {
                        lIsFound = true;
                        break;
                    }

                    lValueNameSize = _countof(lValueName);
                    lDataSize = _countof(lData);
                    lIndex++;
                }

                if (!lIsFound) {
                    wchar_t lNumStartup[16];

                    DWORD lNumStartupSize = sizeof(lNumStartup), lNumStartupType;

                    retval = RegQueryValueEx(MenuKey, L"NumStartup", NULL, &lNumStartupType, (unsigned char *) &lNumStartup, &lNumStartupSize);

                    if (retval == ERROR_FILE_NOT_FOUND) {
                        lNumStartupType = REG_SZ;
                        wcscpy_s(lNumStartup, L"0");
                        retval = ERROR_SUCCESS;
                    }

                    if (retval == ERROR_SUCCESS) {
                        QString lNumStartupStr;
                        wchar_t lNumStartupNew[16];

                        lNumStartupStr = QString::number(QString::fromWCharArray(lNumStartup).toInt() + 1);
                        lNumStartupNew[lNumStartupStr.length()] = 0;
                        lNumStartupStr.toWCharArray(lNumStartupNew);
                        if ((retval = RegSetValueEx(MenuKey, L"NumStartup", NULL, lNumStartupType, (unsigned char *) &lNumStartupNew, wcslen(lNumStartupNew) * 2 + 2)) == ERROR_SUCCESS) {
                            swprintf_s(lNumStartupNew, L"%iStartup", lNumStartupStr.toInt());
                            SuppPath[lAecArx.length()] = 0;
                            lAecArx.toWCharArray(SuppPath);
                            if ((retval = RegSetValueEx(MenuKey, lNumStartupNew, NULL, lNumStartupType, (unsigned char *) &SuppPath, wcslen(SuppPath) * 2 + 2)) == ERROR_SUCCESS) {
                            }
                        }
                    }
                }
            }

            RegCloseKey(MenuKey);
        }
        // end of add autoload


        // add trusted path for acad 2014
        if (AcadVersion > 13) {
            // end of add trusted path
            wcscpy_s(SubKey, wAcadKeyStr);
            wcscat_s(SubKey, L"\\Profiles\\");
            wcscat_s(SubKey, ProfileName);
            wcscat_s(SubKey, L"\\Variables\\");
            if ((retval = RegOpenKeyEx(HKEY_CURRENT_USER, SubKey, 0, KEY_WRITE | KEY_READ | lRegSam, &MenuKey)) == ERROR_SUCCESS) {
                wcscpy_s(ArxFile, lWBinPath);
#ifdef _M_X64
                // in 64 bits, if 32 registry - remove 64 from path
                if (lRegSam == KEY_WOW64_32KEY && (wcsstr(ArxFile, L"64") == &ArxFile[wcslen(ArxFile) - 2]))
                    ArxFile[wcslen(ArxFile) - 2] = 0;
#else
                // in 32bits, if 64 registry - then add 64 to bin path
                if (lRegSam == KEY_WOW64_64KEY && (wcsstr(ArxFile, L"64") != &ArxFile[wcslen(ArxFile) - 2]))
                    wcscat_s(ArxFile, L"64");
#endif
                wcscat_s(ArxFile, L"\\..."); // ... for include all subdirs (mospro, explodeproxy...)
                _wcslwr(ArxFile);

                // путь маленькими буквами, в конце не должно стоять точки с запятой
                SuppPathSize = sizeof(SuppPath);
                if ((retval = RegQueryValueEx(MenuKey, L"TRUSTEDPATHS", NULL, &SuppPathType, (unsigned char *) &SuppPath, &SuppPathSize)) == ERROR_SUCCESS) {
                    if (!wcsstr(SuppPath, ArxFile)) {
                        if (wcslen(SuppPath)) {
                            swprintf_s(SuppPathNew, L"%s;%s", ArxFile, SuppPath);
                            CreateValue(MenuKey, L"TRUSTEDPATHS", REG_SZ, SuppPathNew, wcslen(SuppPathNew) * 2 + 2);
                        } else {
                            CreateValue(MenuKey, L"TRUSTEDPATHS", REG_SZ, ArxFile, wcslen(ArxFile) * 2 + 2);
                        }
                    }
                } else {
                    CreateValue(MenuKey, L"TRUSTEDPATHS", REG_SZ, ArxFile, wcslen(ArxFile) * 2 + 2);
                }
                RegCloseKey(MenuKey);
            }
        }
        // end of add trusted path for acad 2014

        i++;
    }

    RegCloseKey(AcadKey);

    return true; // всё прописано, запустить Автокад
}




















LRESULT SendConnectToAcad(HWND aHwnd, long aWhatToDo, const QString &aModuleNameForOracle) {
    COPYDATASTRUCT CpyData;
    ORACLELOGINDATA OracleLoginData;

    OracleLoginData.WhatToDo = aWhatToDo;

    strcpy_s(OracleLoginData.Login, db.userName().toLatin1().data());
    strcpy_s(OracleLoginData.Password, db.password().toLatin1().data());

    strcpy_s(OracleLoginData.ConnectionString, db.databaseName().toLatin1().data());
    if (db.driverName() == "QPSQL") {
        strcpy_s(OracleLoginData.SchemaName, db.hostName().toLatin1().data());
        strcpy_s(OracleLoginData.ModuleName, QString::number(db.port()).toLatin1().data());
    } else {
        strcpy_s(OracleLoginData.SchemaName, gSettings->CurrentSchema.toLatin1().data());
        strcpy_s(OracleLoginData.ModuleName, aModuleNameForOracle.toLatin1().data());
    }

    CpyData.dwData = 0;
    CpyData.cbData = sizeof(OracleLoginData);
    CpyData.lpData = &OracleLoginData;

    // trying connect
    return SendMessage(aHwnd, WM_COPYDATA, 0, (LPARAM) &CpyData);
}

typedef struct {
    ULONG ListMode; // 0 - check/collect, 1 - connect
    bool LogonError;
    wchar_t CurrentCaption[256];
    QString *TempStr;
    RunningAcadList *RunningAcadList;
} EnumWindowsProcData, *PEnumWindowsProcData;

// проверка, что арх загружен в найденном автокаде
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    PEnumWindowsProcData lData = (PEnumWindowsProcData) lParam;

    if (lData->ListMode == 1
            && lData->RunningAcadList)
        for (int i = 0; i < lData->RunningAcadList->length(); i++)
            if (lData->RunningAcadList->at(i)->GetHwnd() == hwnd)
                return TRUE;

    GetWindowText(hwnd, lData->CurrentCaption, _countof(lData->CurrentCaption));
    if (!lData->CurrentCaption[0]) return(TRUE);

    *lData->TempStr = QString::fromWCharArray(lData->CurrentCaption);
    if (lData->TempStr->indexOf("AutoCAD") == 0 /* autocad has this caption */
            || lData->TempStr->indexOf("Autodesk AutoCAD") == 0 /* civil has this caption*/
            || lData->TempStr->indexOf("Autodesk Map 3D") == 0) {
        // quick check
        if (SendMessage(hwnd, msg_CheckLoadedInAcad, 0, 0) == RET_OK) {
            // long check (for right connection)
            LRESULT lRes = SendConnectToAcad(hwnd, lData->ListMode, *lData->TempStr);

            switch (lData->ListMode) {
            case 0:
                if (lRes == RET_OK1
                        || lRes == RET_OK2) {
                    lData->RunningAcadList->append(new RunningAcadData(hwnd, *lData->TempStr, lRes == RET_OK2));
                }
                break;
            case 1:
                if (lRes == RET_OK) {
                    gSettings->gAcad = hwnd;
                    return(FALSE);
                } else if (lRes == RET_ERROR) {
                    lData->LogonError = true;
                    return(FALSE);
                }
                break;
            }
        }
    }
    return(TRUE);
}

const InstalledAcadList &GlobalSettings::InstalledAcadListConst() const {
    return mInstalledAcadList;
}

QList<AcadParamData *> &GlobalSettings::AcadParamsRef() {
    return AcadParams.Params;
}

long GlobalSettings::InitAcad(int aAcadParamListIndex) {
    QString lTempStr;
    msg_CheckLoadedInAcad = RegisterWindowMessageA("AcadProjPromosVVVIsMyProgramLoadedInAcad");

    gAcad = 0;
    RunningAcadList lRunningAcadList;
    EnumWindowsProcData lData;
    lData.ListMode = (AcadParams.AlwaysAsk == 2)?0:1;
    lData.LogonError = false;
    lData.TempStr = &lTempStr;
    lData.RunningAcadList = &lRunningAcadList;
    EnumWindows(EnumWindowsProc, (LPARAM) &lData);

    if (AcadParams.AlwaysAsk == 2) {
        ChooseAcadDlg w(&lRunningAcadList, gMainWindow);
TryAgain:
        if (w.exec() == QDialog::Accepted) {
            const RunningAcadData *lRunningAcadData;
            aAcadParamListIndex = gSettings->AcadParams.UseAcadParamIndex;
            if ((lRunningAcadData = w.GetRunningAcadData()))
                if (lRunningAcadData->GetConnected()
                        || SendConnectToAcad(lRunningAcadData->GetHwnd(), 1, lRunningAcadData->GetCaption()) == RET_OK) {
                    gAcad = lRunningAcadData->GetHwnd();
                } else {
                    goto TryAgain;
                }
        } else {
            // cancelled in window
            return 0;
        }
    }

    if (gAcad) return 1; // opened and connected acad is found

    if (aAcadParamListIndex == -1) aAcadParamListIndex = gSettings->AcadParams.UseAcadParamIndex;
    if (aAcadParamListIndex == -1
            || AcadParams.AlwaysAsk == 1) {
        ChooseAcadDlg w(NULL, gMainWindow);
        if (w.exec() == QDialog::Accepted) {
            aAcadParamListIndex = gSettings->AcadParams.UseAcadParamIndex;
        } else {
            return 0;
        }
    }
    if (aAcadParamListIndex == -1) {
        return 0;
    }

    const InstalledAcadData *lInstalledAcadData = NULL;
    const AcadParamData *lAcadParamData = NULL;
    if (aAcadParamListIndex >= 0
            && aAcadParamListIndex < gSettings->AcadParams.Params.length()) {
        lAcadParamData = gSettings->AcadParams.Params.at(aAcadParamListIndex);
        lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(lAcadParamData->FullProductNameConst());
    }
    if (!lInstalledAcadData) return 0;
    if (!SetAutoLoadARX(lInstalledAcadData)) return 0;

    if (Features.AcadSupFiles) {
        ASF_Sync();
    }

    QString lCmdLine = lAcadParamData->FullCommandLine();

    if (!QProcess::startDetached(lCmdLine)) {
        QMessageBox lMB(gMainWindow);
        lMB.setIcon(QMessageBox::Critical);
        lMB.setWindowTitle(tr("Run AutoCAD"));
        lMB.setText(tr("Error running AutoCAD (CreateProcess failed)!"));
        lMB.setDetailedText(lCmdLine);
        lMB.addButton(QMessageBox::Close);

        // motherfucker motherfucker
        QSpacerItem * horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout * layout = (QGridLayout *) lMB.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        lMB.exec();
        return 0;
    } else {
        uint i = 0;
        lData.ListMode = 1; // connect
        EnumWindows(EnumWindowsProc, (LPARAM) &lData);
        while (!gAcad && !lData.LogonError && (i <= lAcadParamData->MaxWaitForStart() * 1000)) {
            QThread::msleep(lAcadParamData->StartCheckInterval());
            EnumWindows(EnumWindowsProc, (LPARAM) &lData);
            i += lAcadParamData->StartCheckInterval();
        }
        if (!gAcad) {
            if (!lData.LogonError) {
                QMessageBox::critical(gMainWindow, tr("Run AutoCAD"), tr("Error running AutoCAD (window not found by enumirating)!"));
            }
            return 0;
        }
        return 1;
    }
}

void GlobalSettings::EnumPlotStyles(QStringList &aPlotStyles) {
    aPlotStyles.clear();

    const InstalledAcadData *lInstalledAcadData = NULL;
    const AcadParamData *lAcadParamData = NULL;
    if (AcadParams.UseAcadParamIndex >= 0
            && AcadParams.UseAcadParamIndex < AcadParams.Params.length()) {
        lAcadParamData = AcadParams.Params.at(AcadParams.UseAcadParamIndex);
        lInstalledAcadData = InstalledAcadListConst().GetByProductName(lAcadParamData->FullProductNameConst());
    }

    if (lInstalledAcadData) {
        QSettings lAcadProfiles("HKEY_CURRENT_USER\\" + lInstalledAcadData->KeyConst() + "\\Profiles", QSettings::NativeFormat);
        QSettings lGeneral(lAcadProfiles.fileName() + "\\" + lAcadProfiles.value(".", "<<None motherfucker>>").toString() + "\\General", QSettings::NativeFormat);
        QString lDirStr = lGeneral.value("PrinterStyleSheetDir", "").toString();
        if (!lDirStr.isEmpty()) {
            lDirStr.replace("%USERPROFILE%", QProcessEnvironment::systemEnvironment().value("USERPROFILE"));

            QDir lDir(lDirStr);
            if (lDir.exists()) {
                QStringList lFiles, lFilters;
                lFilters << "*.ctb" << "*.stb";
                lFiles = lDir.entryList(lFilters, QDir::Files | QDir::Readable);
                lFiles.sort(Qt::CaseInsensitive);
                for (int i = 0; i < lFiles.length(); i++) {
                    //QFile file(aFile.FileInfoOrigConst().canonicalFilePath() + "/" + lFiles.at(i));
                    //QMessageBox::critical(NULL, "", lFiles.at(i));
                    aPlotStyles.append(lFiles.at(i));
                }
            }
        }
    }

}
