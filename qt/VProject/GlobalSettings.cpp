#include "GlobalSettings.h"
#include "common.h"
#include "oracle.h"
#include "acad.h"

#include "MainWindow.h"

#include "FileUtils.h"

#include "HomeData.h"
#include "MSOffice.h"

#include "../UsersDlg/UserRight.h"

#include <QCoreApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QDir>

#include <QApplication>
#include <QThread>
#include <QClipboard>

#include <QCheckBox>

#include <QStyleFactory>
//#include <QHostInfo> // temporary

#include <QInputDialog>

void RunAndShowReport(const QString &aVbsName, const QString &aOutName, bool aUseNew) {
    QProcess proc1;

    // don't work in right way; it stucks when use "cmd /c" (you can kill cmd then excel start working).
    // it is finished before the xls formed and saved
    proc1.start(QString(qgetenv("COMSPEC")));
    if (!proc1.waitForStarted(-1)) {
        gLogger->ShowError("VBS wait for started", proc1.errorString());
    } else {
        proc1.write(("start " + aVbsName + "\r\nexit\r\n").toLatin1());
        proc1.closeWriteChannel();

        if (!proc1.waitForFinished(-1)) {
            gLogger->ShowError("VBS wait for finished", proc1.errorString());
        } else {
            QString lResultFile(aVbsName), lErrorFile(aVbsName);

            if (!aUseNew) {
                lResultFile.resize(lResultFile.lastIndexOf(QChar('/')));
                lResultFile += "/" + aOutName;
            } else {
                lResultFile = aOutName;
            }

            lErrorFile.resize(lErrorFile.lastIndexOf(QChar('/')));
            lErrorFile += "/error.xls";

            int i = 0;
            while (!QFile::exists(lResultFile)
                   && !QFile::exists(lErrorFile)
                   && i < 10) {
                QThread::sleep(1);
                i++;
            }

            //QFile::remove(aVbsName);
            if (QFile::exists(lResultFile)) {
                QThread::msleep(100); // temporary
                QProcess::startDetached(QString(qgetenv("COMSPEC")) + " /c \"" + lResultFile + "\"");
            } else {
                gLogger->ShowError("VBS", "Was error in Visual Basic Script, report not prepared!");
            }
        }
    }
}

//---------------------------------------------------------------------------------------
InstalledAcadData::InstalledAcadData(const QString &aName, const QString &aKey, const QString &aPath, REGSAM aFuckingWow) :
    mDiffProduct(false), mDiffLang(false),
    mVersion(0), mName(aName), mKey(aKey),
    mPath(aPath), mFuckingWow(aFuckingWow)
{
    for (int i = 0; i < mName.length(); i++) {
        if (mName[i] == '2') {
            mVersion = mName.mid(i, 4).toInt();
            break;
        }
    }
}

InstalledAcadData::InstalledAcadData(InstalledAcadData *aInstalledAcadData) :
    mDiffProduct(aInstalledAcadData->mDiffProduct), mDiffLang(aInstalledAcadData->mDiffLang),
    mVersion(aInstalledAcadData->mVersion), mName(aInstalledAcadData->mName),
    mKey(aInstalledAcadData->mKey),
    mPath(aInstalledAcadData->mPath), mFuckingWow(aInstalledAcadData->mFuckingWow)
{

}

void InstalledAcadData::SetDiffProduct(bool aDiffProduct) {
    mDiffProduct = aDiffProduct;
}

void InstalledAcadData::SetDiffLang(bool aDiffLang) {
    mDiffLang = aDiffLang;
}

const QString &InstalledAcadData::ProductConst() const {
    return mProduct;
}

void InstalledAcadData::SetProduct(const QString &aProduct) {
    mProduct = aProduct;
}

const QString &InstalledAcadData::LangConst() const {
    return mLang;
}

void InstalledAcadData::SetLang(const QString &aLang) {
    mLang = aLang;
}

QString InstalledAcadData::FullProductName() const {
    return mName + " --- " + mProduct + " --- " + mLang;
}

QString InstalledAcadData::FullDisplayName() const {
    QString lName = mName;
    if (mDiffProduct && !mProduct.isEmpty()) lName += " - " + mProduct;
    if (mDiffLang && !mLang.isEmpty()) lName += " - " + mLang;
    return lName;
}

QString InstalledAcadData::FullCommandLine() const {
    QString lCmdLine;
    lCmdLine = "\"" + mPath + "\\acad.exe\"";
    if (!mProduct.isEmpty()) lCmdLine += " /product \"" + mProduct +"\"";
    if (!mLang.isEmpty()) lCmdLine += " /language \"" + mLang +"\"";
    return lCmdLine;
}

QStringList InstalledAcadData::Profiles() const {
    QStringList lProfiles;
    if (!mKey.isEmpty()) {
        QSettings lAcadProfiles("HKEY_CURRENT_USER\\" + mKey + "\\Profiles", QSettings::NativeFormat);
        lProfiles = lAcadProfiles.childGroups();
    }
    return lProfiles;
}

int InstalledAcadData::Version() const {
    return mVersion;
}

//const QString &InstalledAcadData::NameConst() const {
//    return mName;
//}

const QString &InstalledAcadData::KeyConst() const {
    return mKey;
}

const QString &InstalledAcadData::PathConst() const {
    return mPath;
}

REGSAM InstalledAcadData::FuckingWow() const {
    return mFuckingWow;
}

const InstalledAcadData *InstalledAcadList::GetByProductName(const QString &aFullProductName) const {
    for (int i = 0; i < length(); i++) {
        if (at(i)->FullProductName() == aFullProductName) return at(i);
    }
    return NULL;
}

AcadParamData::AcadParamData(const QString &aAddName, const QString &aFullProductName,
                             const QString &aUserProfile, bool aLoadAecBase, bool aDiffProfile) :
    mDiffProfile(aDiffProfile),
    mAddName(aAddName), mFullProductName(aFullProductName), mUserProfile(aUserProfile),
    mStartWithNoLogo(1), mLoadAecBase(aLoadAecBase), mDelayAfterStart(5), mMaxWaitForStart(30), mStartCheckInterval(500),
    mDelayAfterOpen(500), mLOBBufferSize(2)
{
    if (mDiffProfile && mAddName.isEmpty()) mAddName = mUserProfile;
}

AcadParamData::AcadParamData(QSettings &aSettings) {
    LoadSettings(aSettings);
}

AcadParamData::AcadParamData(const AcadParamData *aAcadParamData) :
    mDiffProfile(aAcadParamData->mDiffProfile),
    mAddName(aAcadParamData->mAddName), mFullProductName(aAcadParamData->mFullProductName),
    mUserProfile(aAcadParamData->mUserProfile), mCommandLine(aAcadParamData->mCommandLine),
    mStartWithNoLogo(aAcadParamData->mStartWithNoLogo),
    mLoadAecBase(aAcadParamData->mLoadAecBase),
    mDelayAfterStart(aAcadParamData->mDelayAfterStart),
    mMaxWaitForStart(aAcadParamData->mMaxWaitForStart), mStartCheckInterval(aAcadParamData->mStartCheckInterval),
    mDelayAfterOpen(aAcadParamData->mDelayAfterOpen), mLOBBufferSize(aAcadParamData->mLOBBufferSize)
{

}

void AcadParamData::CopyFromOther(const AcadParamData *aAcadParamData) {
    mDiffProfile = aAcadParamData->mDiffProfile;
    mAddName = aAcadParamData->mAddName;
    mFullProductName = aAcadParamData->mFullProductName;
    mUserProfile = aAcadParamData->mUserProfile;
    mCommandLine = aAcadParamData->mCommandLine,
    mStartWithNoLogo = aAcadParamData->mStartWithNoLogo;
    mLoadAecBase = aAcadParamData->mLoadAecBase;
    mDelayAfterStart = aAcadParamData->mDelayAfterStart;
    mMaxWaitForStart = aAcadParamData->mMaxWaitForStart;
    mStartCheckInterval = aAcadParamData->mStartCheckInterval;
    mDelayAfterOpen = aAcadParamData->mDelayAfterOpen;
    mLOBBufferSize = aAcadParamData->mLOBBufferSize;

}

void AcadParamData::LoadSettings(QSettings &aSettings) {
    mDiffProfile = aSettings.value("DiffProfile", false).toBool();
    mAddName = aSettings.value("AddName").toString();
    mFullProductName = aSettings.value("FullProductName").toString();
    mUserProfile = aSettings.value("UserProfile").toString();
    mCommandLine = aSettings.value("CommandLine").toString();
    mStartWithNoLogo = aSettings.value("StartWithNoLogo", 1).toUInt();
    mLoadAecBase = aSettings.value("LoadAecBase", mFullProductName.contains("civil", Qt::CaseInsensitive)?1:0).toUInt();
    mDelayAfterStart = aSettings.value("DelayAfterStart", 5).toUInt();
    mMaxWaitForStart = aSettings.value("MaxWaitForStart", 30).toUInt();
    mStartCheckInterval = aSettings.value("StartCheckInterval", 500).toUInt();
    mDelayAfterOpen = aSettings.value("DelayAfterOpen", 500).toUInt();
    mLOBBufferSize = aSettings.value("LOBBufferSize", 2).toUInt();
}

void AcadParamData::SaveSettings(QSettings &aSettings) {
    aSettings.setValue("DiffProfile", mDiffProfile);
    aSettings.setValue("AddName", mAddName);
    aSettings.setValue("FullProductName", mFullProductName);
    aSettings.setValue("UserProfile", mUserProfile);
    aSettings.setValue("CommandLine", mCommandLine);
    aSettings.setValue("StartWithNoLogo", mStartWithNoLogo);
    aSettings.setValue("LoadAecBase", mLoadAecBase);
    aSettings.setValue("DelayAfterStart", mDelayAfterStart);
    aSettings.setValue("MaxWaitForStart", mMaxWaitForStart);
    aSettings.setValue("StartCheckInterval", mStartCheckInterval);
    aSettings.setValue("DelayAfterOpen", mDelayAfterOpen);
    aSettings.setValue("LOBBufferSize", mLOBBufferSize);
}

QString AcadParamData::FullDisplayName() const {
    const InstalledAcadData *lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(mFullProductName);
    if (!lInstalledAcadData) return "<Empty>";
    return lInstalledAcadData->FullDisplayName() + (mAddName.isEmpty()?"":(" - " + mAddName));
}

QString AcadParamData::FullCommandLine() const {
    const InstalledAcadData *lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(mFullProductName);
    if (!lInstalledAcadData) return "<Empty>";

    QString lCmdLine;

    lCmdLine = lInstalledAcadData->FullCommandLine();
    if (mStartWithNoLogo) lCmdLine += " /nologo";
    if (mLoadAecBase) lCmdLine += " /ld \"" + lInstalledAcadData->PathConst() + "\\AecBase.dbx\"";
    if (!mUserProfile.isEmpty()) lCmdLine += " /p \"" + mUserProfile + "\"";
    if (!mCommandLine.isEmpty()) lCmdLine += " " + mCommandLine;

    return lCmdLine;
}

const QString &AcadParamData::AddNameConst() const {
    return mAddName;
}

void AcadParamData::SetAddName(const QString &aAddName) {
    mAddName = aAddName;
}

const QString &AcadParamData::FullProductNameConst() const {
    return mFullProductName;
}

void AcadParamData::SetFullProductName(const QString &aFullProductName) {
    mFullProductName = aFullProductName;
}

const QString &AcadParamData::UserProfileConst() const {
    return mUserProfile;
}

void AcadParamData::SetUserProfile(const QString &aUserProfile) {
    mUserProfile = aUserProfile;
}

const QString &AcadParamData::CommandLineConst() const {
    return mCommandLine;
}

void AcadParamData::SetCommandLine(const QString &aCommandLine) {
    mCommandLine = aCommandLine;
}

uint AcadParamData::StartWithNoLogo() const {
    return mStartWithNoLogo;
}
void AcadParamData::SetStartWithNoLogo(uint aStartWithNoLogo) {
    mStartWithNoLogo = aStartWithNoLogo;
}

uint AcadParamData::LoadAecBase() const {
    return mLoadAecBase;
}

void AcadParamData::SetLoadAecBase(uint aLoadAecBase) {
    mLoadAecBase = aLoadAecBase;
}

uint AcadParamData::DelayAfterStart() const {
    return mDelayAfterStart;
}
void AcadParamData::SetDelayAfterStart(uint aDelayAfterStart) {
    mDelayAfterStart = aDelayAfterStart;
}

uint AcadParamData::MaxWaitForStart() const {
    return mMaxWaitForStart;
}

void AcadParamData::SetMaxWaitForStart(uint aMaxWaitForStart) {
    mMaxWaitForStart = aMaxWaitForStart;
}

uint AcadParamData::StartCheckInterval() const {
    return mStartCheckInterval;
}

void AcadParamData::SetStartCheckInterval(uint aStartCheckInterval) {
    mStartCheckInterval = aStartCheckInterval;
}

uint AcadParamData::DelayAfterOpen() const {
    return mDelayAfterOpen;
}

void AcadParamData::SetDelayAfterOpen(uint aDelayAfterOpen) {
    mDelayAfterOpen = aDelayAfterOpen;
}

uint AcadParamData::LOBBufferSize() const {
    return mLOBBufferSize;
}

void AcadParamData::SetLOBBufferSize(uint aLOBBufferSize) {
    mLOBBufferSize = aLOBBufferSize;
}

RunningAcadData::RunningAcadData(HWND aHwnd, const QString aCaption, bool aConnected) :
    mHwnd(aHwnd), mCaption(aCaption), mConnected(aConnected)
{
}

HWND RunningAcadData::GetHwnd() const {
    return mHwnd;
}

const QString &RunningAcadData::GetCaption() const {
    return mCaption;
}

bool RunningAcadData::GetConnected() const {
    return mConnected;
}

RunningAcadList::~RunningAcadList() {
    qDeleteAll(*this);
}

//---------------------------------------------------------------------------------------
GlobalSettings::GlobalSettings(QObject *parent) :
    QObject(parent), mNDS(0), ContractMode(-1),
    DebugOutput(0)
{
    QSettings settings;

    EnumInstalledProducts(mInstalledAcadList);

    gAcad = 0;
    //locale = new QLocale(QLocale::Hebrew, QLocale::Israel);
    locale = new QLocale();

    settings.beginGroup(this->metaObject()->className());

    // it is part for "first run"
    // ----------------------------------------
    bool lIsFirstRun;
    settings.beginGroup("Common");
    lIsFirstRun = !settings.contains("TabPos");
    settings.endGroup();
    if (lIsFirstRun) {
        settings.remove("");
    }

    // ----------------------------------------
    settings.beginGroup("Common");

    QFont lFont;
    lFont.fromString(settings.value("Font", QApplication::font().toString()).toString());
    QApplication::setFont(lFont);

    if (QStyleFactory::keys().contains("WindowsXP")) {
        Common.VisualStyle = settings.value("VisualStyle", "WindowsXP").toString();
    } else {
        Common.VisualStyle = settings.value("VisualStyle").toString();
    }

    Common.ConfirmQuit = settings.value("ConfirmQuit", 0).toInt();
    Common.SaveWinState = settings.value("SaveWinState", true).toBool();
    Common.RereadAfterSave = settings.value("RereadAfterSave", true).toBool();
    Common.AddRowHeight = settings.value("AddRowHeight", 0).toInt();
    Common.SubRowHeight = settings.value("SubRowHeight", 5).toInt();
    Common.AddColWidth = settings.value("AddColWidth", 10).toInt();
    Common.UseTabbedView = settings.value("UseTabbedView", false).toBool();
    Common.TabPos = static_cast<QTabWidget::TabPosition>(settings.value("TabPos", 0).toInt());
    Common.ShowAfterCopy = settings.value("ShowAfterCopy", true).toBool();
    Common.RequiredFieldColor = settings.value("RequiredFieldColor", qRgba(210, 255, 255, 0xff)).toUInt();
    Common.DisabledFieldColor = settings.value("DisabledFieldColor", qRgba(200, 200, 200, 0xff)).toUInt();
    settings.endGroup();

    settings.beginGroup("LocalCache");
    LocalCache.UseLocalCache = settings.value("UseLocalCache", true).toBool();
    LocalCache.MinDiskSize = settings.value("MinDiskSize", (qulonglong) 3000000000).toULongLong();
    LocalCache.PathAuto = settings.value("PathAuto", true).toBool();
    if (!LocalCache.PathAuto) {
        LocalCache.Path = settings.value("Path", "").toString();
        QDir lDir(LocalCache.Path);
        if (LocalCache.Path.isEmpty()
                || !lDir.exists()) {
            LocalCache.PathAuto = true;
        }
    }
    settings.endGroup();

    settings.beginGroup("LoadFiles");
    LoadFiles.LastDir = settings.value("LastDir").toString();
    settings.endGroup();

    settings.beginGroup("SaveFiles");
    SaveFiles.LastDir = settings.value("LastDir").toString();
    settings.endGroup();

    settings.beginGroup("Contract");
    Contract.UseProjectColor = settings.value("UseProjectColor", true).toBool();
    Contract.UseContractColor = settings.value("UseContractColor", true).toBool();
    Contract.UseStageColor = settings.value("UseStageColor", true).toBool();

    Contract.ProjectColor.setRgba(settings.value("ProjectColor", qRgba(0xff, 0xdd, 0xdd, 0xff)).toUInt());
    Contract.ContractColor.setRgba(settings.value("ContractColor", qRgba(0xdd, 0xff, 0xdd, 0xff)).toUInt());
    Contract.StageColor.setRgba(settings.value("StageColor", qRgba(0xdd, 0xdd, 0xdd, 0xff)).toUInt());

    Contract.ExpandOnStart = settings.value("ExpandOnStart", 2).toInt();
    Contract.MultiSelect = settings.value("MultiSelect", true).toBool();
    settings.endGroup();

    settings.beginGroup("Hashbon");
    Hashbon.AutoHideEmptySigned = settings.value("AutoHideEmptySigned", true).toBool();
    Hashbon.AutoHideSignIfPay = settings.value("AutoHideSignIfPay", true).toBool();
    Hashbon.AutoHideEmptyPayed = settings.value("AutoHideEmptyPayed", true).toBool();
    Hashbon.AutoHideEmptyIndexed = settings.value("AutoHideEmptyIndexed", true).toBool();
    settings.endGroup();

    settings.beginGroup("Geobase");
    Geobase.OnDblClick = settings.value("OnDblClick", 0).toInt();
    Geobase.SelectBeh = settings.value("SelectBeh", 0).toInt();
    Geobase.SelectMode = settings.value("SelectMode", 1).toInt();
    Geobase.DrawingShowMode = settings.value("DrawingShowMode", 0).toInt();
    settings.endGroup();

    settings.beginGroup("SaveDialog");
//    SaveDialog.ShowImages = settings.value("ShowImages", 1).toBool();
//    SaveDialog.ShowXrefs = settings.value("ShowXrefs", 1).toBool();
//    SaveDialog.ShowAddFiles = settings.value("ShowAddFiles", 1).toBool();
    settings.endGroup();

    settings.beginGroup("TypeTree");
    TypeTree.ExpandLevel = settings.value("ExpandLevel", 2).toInt();
    TypeTree.FontPlusOne = settings.value("FontPlusOne", false).toBool();
    TypeTree.FontBold = settings.value("FontBold", false).toBool();
    settings.endGroup();

    settings.beginGroup("DocumentTree");
    DocumentTree.SingleSelect = static_cast<DocumentTreeStruct::enumSingleSelect>(settings.value("SingleSelect", 0).toInt());

    DocumentTree.WindowTitleType = static_cast<DocumentTreeStruct::WNDTitleType>(settings.value("WindowTitleType", 0).toInt());

    DocumentTree.OpenSingleDocument = settings.value("OpenSingleDocument", true).toBool();
    DocumentTree.ShowGridLines = settings.value("ShowGridLines", true).toBool();
    DocumentTree.AutoWidth = settings.value("AutoWidth", true).toBool();
    DocumentTree.DocFontPlusOne = settings.value("DocFontPlusOne", true).toBool();
    DocumentTree.DocFontBold = settings.value("DocFontBold", true).toBool();
    DocumentTree.DragDrop = settings.value("DragDrop", false).toBool();
    DocumentTree.AskPasswordOnDelete = settings.value("AskPasswordOnDelete", true).toBool();

    DocumentTree.OnDocDblClick = static_cast<DocumentTreeStruct::DBLDOC>(settings.value("OnDocDblClick", 0).toInt());
    DocumentTree.SecondLevelType = static_cast<DocumentTreeStruct::SLT>(settings.value("SecondLevelType", 0).toInt());
    DocumentTree.ExpandOnShow = settings.value("ExpandOnShow", false).toBool();
    DocumentTree.AddRowHeight = settings.value("AddRowHeight", 0).toInt();

    DocumentTree.UseDocColor = settings.value("UseDocColor", true).toBool();
    DocumentTree.UseSecondColor = settings.value("UseLayoutColor", true).toBool();
    DocumentTree.DocColor.setRgba(settings.value("DocColor", qRgba(0xdd, 0xff, 0xdd, 0xff)).toUInt());
    DocumentTree.SecondColor.setRgba(settings.value("SecondColor", qRgba(0xdd, 0xdd, 0xff, 0xff)).toUInt());

    //DocumentTree.RefreshOnWindowOpen = settings.value("RefreshOnWindowOpen", true).toBool();
    settings.endGroup();

    settings.beginGroup("DocumentHistory");
    //DocumentHistory.MDI = settings.value("MDI", false).toBool();
    DocumentHistory.MDI = false;

    //DocumentHistory.AutoWidth = settings.value("AutoWidth", true).toBool();
    DocumentHistory.AutoWidth = true;

    //DocumentHistory.ShowTempFilenames = settings.value("ShowTempFilenames", false).toBool();
    settings.endGroup();

    settings.beginGroup("AuditPurge");
    AuditPurge.PurgeRegapps = settings.value("PurgeRegapps", true).toBool();
    AuditPurge.PurgeAll = settings.value("PurgeAll", false).toBool();
    AuditPurge.ExplodeProxy = settings.value("ExplodeProxy", false).toBool();
    AuditPurge.RemoveProxy = settings.value("RemoveProxy", false).toBool();
    AuditPurge.Audit = settings.value("Audit", true).toBool();
    settings.endGroup();

    settings.beginGroup("Publish");
    Publish.PDF = settings.value("PDF", true).toBool();
    Publish.DWF = settings.value("DWF", false).toBool();
    Publish.PLT = settings.value("PLT", false).toBool();
    Publish.DontScale = settings.value("DontScale", false).toBool();
    Publish.UseVersion = settings.value("UseVersion", -1).toInt(); // always ask by default
    Publish.CTBType = settings.value("CTBType", 0).toInt(); // auto by default
    Publish.CTBName = settings.value("CTBName", "").toString();
    Publish.PlotterName = settings.value("PlotterName", "NEW plotter COLOR").toString();
    settings.endGroup();

    Compare.Readed = settings.childGroups().contains("Compare");
    settings.beginGroup("Compare");
    Compare.OldColor = settings.value("OldColor", 7).toUInt();
    Compare.NewColor = settings.value("NewColor", 1).toUInt();
    Compare.OutputDates = settings.value("OutputDates", true).toBool();
    Compare.TextHeight = settings.value("TextHeight", 5).toUInt();
    Compare.AlwaysAskOutputDir = settings.value("AlwaysAskOutputDir", true).toBool();
    Compare.OutputDir = settings.value("OutputDir", "").toString();
    Compare.AlwaysAskAll = settings.value("AlwaysAskAll", false).toBool();
    settings.endGroup();

    settings.beginGroup("Image");
    Image.LoadWhenFNExists = settings.value("LoadWhenFNExists", 1).toInt();
    Image.ResizeForPreview = settings.value("ResizeForPreview", 1).toInt();
    Image.MaxPreviewWidth = settings.value("MaxPreviewWidth", 1920).toInt();
    Image.MaxPreviewHeight = settings.value("MaxPreviewHeight", 1980).toInt();
    Image.OnPreviewDblClick = settings.value("OnPreviewDblClick", 0).toInt();
    Image.ConvertType = settings.value("ConvertType", 0).toInt();
    Image.MaxConvertWidth = settings.value("MaxConvertWidth", 2048).toInt();
    Image.MaxConvertHeight = settings.value("MaxConvertHeight", 2048).toInt();
    Image.ConvertPercent = settings.value("ConvertPercent", 50).toInt();
    Image.MaxFileSize = settings.value("MaxFileSize", 500000).toInt();

    Image.ViewerType = settings.value("ViewerType", 0).toInt();
    Image.ViewerPath = settings.value("ViewerPath", "").toString();
    Image.SaveAll = settings.value("SaveAll", false).toBool();
    Image.ConfirmViewerClosed = settings.value("ConfirmViewerClosed", true).toBool();

    Image.EditorType = settings.value("EditorType", 0).toInt();
    Image.EditorPath = settings.value("EditorPath", "").toString();
    Image.SaveAllForEditor = settings.value("SaveAllForEditor", false).toBool();
    Image.ConfirmEditorClosed = settings.value("ConfirmEditorClosed", true).toBool();

    settings.endGroup();

    settings.endGroup();

    // just splitting on parts
    LoadAcadParams();

    ServerSecure.AskPassForUserMan = -1;
    ServerSecure.DropOnFire = -1;
}

GlobalSettings::~GlobalSettings() {
    if (Common.ConfirmQuit == 2) {
        RemoveSettings();
    } else {
        SaveSettings();
    }
    delete locale;
}

void GlobalSettings::clean() {
    delete GetInstance();
}

GlobalSettings *GlobalSettings::GetInstance() {
    static GlobalSettings * lGlobalSettings = NULL;
    if (!lGlobalSettings) {
        lGlobalSettings = new GlobalSettings();
        qAddPostRoutine(GlobalSettings::clean);
    }
    return lGlobalSettings;
}

void GlobalSettings::InitAcadParamsList(QList<AcadParamData *> &aParams) {
    int i, j;
    for (i = 0; i < mInstalledAcadList.length(); i++) {
        bool lIsFound = false;
        for (j = 0; j < aParams.length(); j++) {
            if (mInstalledAcadList.at(i)->FullProductName() == aParams.at(j)->FullProductNameConst()) {
                lIsFound = true;
                break;
            }
        }
        if (!lIsFound) {
            QStringList lProfiles;
            if (mInstalledAcadList.at(i)->ProductConst() == "C3D") lProfiles = mInstalledAcadList.at(i)->Profiles();
            if (lProfiles.isEmpty()) {
                aParams.append(new AcadParamData("", mInstalledAcadList.at(i)->FullProductName(),
                                                 "", mInstalledAcadList.at(i)->ProductConst() == "C3D", false));
            } else {
                for (int k = 0; k < lProfiles.length(); k++) {
                    aParams.append(new AcadParamData("", mInstalledAcadList.at(i)->FullProductName(),
                                                     lProfiles.at(k), mInstalledAcadList.at(i)->ProductConst() == "C3D" && lProfiles.at(k).contains("C3D", Qt::CaseInsensitive), true));
                }
            }
        }
    }
}

void GlobalSettings::LoadAcadParams() {
    int i, cnt;
    AcadParamData *lAcadParamData;
    QSettings settings;
    settings.beginGroup("AutocadParams");
    AcadParams.UseAcadParamIndex = settings.value("UseAcadParamIndex", -1).toInt();
    AcadParams.AlwaysAsk = settings.value("AutoCad always ask", 0).toUInt();
    AcadParams.KeepAfterEdit = settings.value("KeepAfterEdit", 0).toUInt();
    AcadParams.KeepAllBackupsLocally = settings.value("KeepAllBackupsLocally", 0).toUInt();

    cnt = settings.value("ParamsCnt", -1).toInt();
    for (i = 0; i < cnt; i++) {
        settings.beginGroup(QString::number(i));
        lAcadParamData = new AcadParamData(settings);
        if (mInstalledAcadList.GetByProductName(lAcadParamData->FullProductNameConst())) {
            AcadParams.Params.append(lAcadParamData);
        } else {
            delete lAcadParamData;
        }
        settings.endGroup();
    }

    InitAcadParamsList(AcadParams.Params);

//    std::sort(AcadParams.Params.begin(), AcadParams.Params.end(),
//              [] (const AcadParamData *p1, const AcadParamData *p2) { return p1->FullDisplayName() < p2->FullDisplayName();});

    settings.endGroup();
}

void GlobalSettings::SaveAcadParams() {
    QSettings settings;

    settings.beginGroup("AutocadParams");
    settings.setValue("UseAcadParamIndex", AcadParams.UseAcadParamIndex);
    settings.setValue("AutoCad always ask", AcadParams.AlwaysAsk);
    settings.setValue("KeepAfterEdit", AcadParams.KeepAfterEdit);
    settings.setValue("KeepAllBackupsLocally", AcadParams.KeepAllBackupsLocally);
    settings.setValue("ParamsCnt", AcadParams.Params.length());

    for (int i = 0; i < AcadParams.Params.length(); i++) {
        settings.beginGroup(QString::number(i));
        AcadParams.Params.at(i)->SaveSettings(settings);
        settings.endGroup();
    }
    settings.endGroup();
}

void GlobalSettings::SaveSettings() {
    QSettings settings;

    settings.beginGroup(this->metaObject()->className());

    settings.beginGroup("Common");
    settings.setValue("Font", QApplication::font().toString());

    settings.setValue("VisualStyle", Common.VisualStyle);

    settings.setValue("ConfirmQuit", Common.ConfirmQuit);
    settings.setValue("SaveWinState", Common.SaveWinState);
    settings.setValue("RereadAfterSave", Common.RereadAfterSave);
    settings.setValue("AddRowHeight", Common.AddRowHeight);
    settings.setValue("SubRowHeight", Common.SubRowHeight);
    settings.setValue("AddColWidth", Common.AddColWidth);
    settings.setValue("UseTabbedView", Common.UseTabbedView);
    settings.setValue("TabPos", Common.TabPos);
    settings.setValue("ShowAfterCopy", Common.ShowAfterCopy);

    settings.setValue("RequiredFieldColor", Common.RequiredFieldColor.rgba());
    settings.setValue("DisabledFieldColor", Common.DisabledFieldColor.rgba());
    settings.endGroup();

    settings.beginGroup("LocalCache");
    settings.setValue("UseLocalCache", LocalCache.UseLocalCache);
    settings.setValue("MinDiskSize", LocalCache.MinDiskSize);
    settings.setValue("PathAuto", LocalCache.PathAuto);
    if (!LocalCache.PathAuto) {
        // save without base name
        settings.setValue("Path", LocalCache.Path.remove(QRegularExpression("/[^/]*$")).remove(QRegularExpression("/[^/]*$")) + "/");
    }
    settings.endGroup();


    settings.beginGroup("LoadFiles");
    settings.setValue("LastDir", LoadFiles.LastDir);
    settings.endGroup();

    settings.beginGroup("SaveFiles");
    settings.setValue("LastDir", SaveFiles.LastDir);
    settings.endGroup();

    settings.beginGroup("Contract");
    settings.setValue("UseProjectColor", Contract.UseProjectColor);
    settings.setValue("UseContractColor", Contract.UseContractColor);
    settings.setValue("UseStageColor", Contract.UseStageColor);

    settings.setValue("ProjectColor", Contract.ProjectColor.rgba());
    settings.setValue("ContractColor", Contract.ContractColor.rgba());
    settings.setValue("StageColor", Contract.StageColor.rgba());

    settings.setValue("ExpandOnStart", Contract.ExpandOnStart);
    settings.setValue("MultiSelect", Contract.MultiSelect);
    settings.endGroup();

    //-----------------------------------------------------------------------------------------------------
    settings.beginGroup("Hashbon");
    settings.setValue("AutoHideEmptySigned", Hashbon.AutoHideEmptySigned);
    settings.setValue("AutoHideSignIfPay", Hashbon.AutoHideSignIfPay);
    settings.setValue("AutoHideEmptyPayed", Hashbon.AutoHideEmptyPayed);
    settings.setValue("AutoHideEmptyIndexed", Hashbon.AutoHideEmptyIndexed);
    settings.endGroup();

    //-----------------------------------------------------------------------------------------------------
    settings.beginGroup("Geobase");
    settings.setValue("OnDblClick", Geobase.OnDblClick);
    settings.setValue("SelectBeh", Geobase.SelectBeh);
    settings.setValue("SelectMode", Geobase.SelectMode);
    settings.setValue("DrawingShowMode", Geobase.DrawingShowMode);
    settings.endGroup();

    //-----------------------------------------------------------------------------------------------------
    settings.beginGroup("SaveDialog");
    //    settings.setValue("ShowImages", SaveDialog.ShowImages);
    //    settings.setValue("ShowXrefs", SaveDialog.ShowXrefs);
    //    settings.setValue("ShowAddFiles", SaveDialog.ShowAddFiles);
    settings.endGroup();

    settings.beginGroup("TypeTree");
    settings.setValue("ExpandLevel", TypeTree.ExpandLevel);
    settings.setValue("FontPlusOne", TypeTree.FontPlusOne);
    settings.setValue("FontBold", TypeTree.FontBold);
    settings.endGroup();

    settings.beginGroup("DocumentTree");
    settings.setValue("SingleSelect", DocumentTree.SingleSelect);

    settings.setValue("WindowTitleType", DocumentTree.WindowTitleType);

    settings.setValue("OpenSingleDocument", DocumentTree.OpenSingleDocument);
    settings.setValue("ShowGridLines", DocumentTree.ShowGridLines);
    settings.setValue("AutoWidth", DocumentTree.AutoWidth);
    settings.setValue("DocFontPlusOne", DocumentTree.DocFontPlusOne);
    settings.setValue("DocFontBold", DocumentTree.DocFontBold);
    settings.setValue("DragDrop", DocumentTree.DragDrop);
    settings.setValue("AskPasswordOnDelete", DocumentTree.AskPasswordOnDelete);

    settings.setValue("OnDocDblClick", DocumentTree.OnDocDblClick);
    settings.setValue("SecondLevelType", DocumentTree.SecondLevelType);
    settings.setValue("ExpandOnShow", DocumentTree.ExpandOnShow);
    settings.setValue("AddRowHeight", DocumentTree.AddRowHeight);

    settings.setValue("UseDocColor", DocumentTree.UseDocColor);
    settings.setValue("UseLayoutColor", DocumentTree.UseSecondColor);
    settings.setValue("DocColor", DocumentTree.DocColor.rgba());
    settings.setValue("SecondColor", DocumentTree.SecondColor.rgba());

    //settings.setValue("RefreshOnWindowOpen", DocumentTree.RefreshOnWindowOpen);
    settings.endGroup();

    settings.beginGroup("DocumentHistory");
    //settings.setValue("MDI", DocumentHistory.MDI);
    //settings.setValue("AutoWidth", DocumentHistory.AutoWidth);
    ////settings.setValue("ShowTempFilenames", DocumentHistory.ShowTempFilenames);
    settings.endGroup();

    settings.beginGroup("AuditPurge");
    settings.setValue("PurgeRegapps", AuditPurge.PurgeRegapps);
    settings.setValue("PurgeAll", AuditPurge.PurgeAll);
    settings.setValue("ExplodeProxy", AuditPurge.ExplodeProxy);
    settings.setValue("RemoveProxy", AuditPurge.RemoveProxy);
    settings.setValue("Audit", AuditPurge.Audit);
    settings.endGroup();

    settings.beginGroup("Publish");
    settings.setValue("PDF", Publish.PDF);
    settings.setValue("DWF", Publish.DWF);
    settings.setValue("PLT", Publish.PLT);
    settings.setValue("DontScale", Publish.DontScale);
    if (Publish.UseVersion != 2)
        settings.setValue("UseVersion", Publish.UseVersion);
    settings.setValue("CTBType", Publish.CTBType);
    settings.setValue("CTBName", Publish.CTBName);
    settings.setValue("PlotterName", Publish.PlotterName);
    settings.endGroup();

    settings.beginGroup("Compare");
    settings.setValue("OldColor", Compare.OldColor);
    settings.setValue("NewColor", Compare.NewColor);
    settings.setValue("OutputDates", Compare.OutputDates);
    settings.setValue("TextHeight", Compare.TextHeight);
    settings.setValue("AlwaysAskOutputDir", Compare.AlwaysAskOutputDir);
    settings.setValue("OutputDir", Compare.OutputDir);
    settings.setValue("AlwaysAskAll", Compare.AlwaysAskAll);
    settings.endGroup();

    SaveSettingsImage(&settings);

    settings.endGroup();

    SaveAcadParams();
}

#define SaveSettingsStart \
    QSettings *settings = aSettings; \
\
    if (!aSettings) { \
        settings = new QSettings(); \
        settings->beginGroup(this->metaObject()->className()); \
    }

#define SaveSettingsEnd \
    if (!aSettings) { \
        settings->endGroup(); \
        delete settings; \
    }

void GlobalSettings::SaveSettingsImage(QSettings *aSettings) {
    SaveSettingsStart

    settings->beginGroup("Image");
    settings->setValue("LoadWhenFNExists", Image.LoadWhenFNExists);
    settings->setValue("ResizeForPreview", Image.ResizeForPreview);
    settings->setValue("MaxPreviewWidth", Image.MaxPreviewWidth);
    settings->setValue("MaxPreviewHeight", Image.MaxPreviewHeight);
    settings->setValue("OnPreviewDblClick", Image.OnPreviewDblClick);
    settings->setValue("ConvertType", Image.ConvertType);
    settings->setValue("MaxConvertWidth", Image.MaxConvertWidth);
    settings->setValue("MaxConvertHeight", Image.MaxConvertHeight);
    settings->setValue("ConvertPercent", Image.ConvertPercent);
    settings->setValue("MaxFileSize", Image.MaxFileSize);

    settings->setValue("ViewerType", Image.ViewerType);
    settings->setValue("ViewerPath", Image.ViewerPath);
    settings->setValue("SaveAll", Image.SaveAll);
    settings->setValue("ConfirmViewerClosed", Image.ConfirmViewerClosed);

    settings->setValue("EditorType", Image.EditorType);
    settings->setValue("EditorPath", Image.EditorPath);
    settings->setValue("SaveAllForEditor", Image.SaveAllForEditor);
    settings->setValue("ConfirmEditorClosed", Image.ConfirmEditorClosed);

    settings->endGroup();

    SaveSettingsEnd
}

#undef SaveSettingsStart
#undef SaveSettingsEnd

void GlobalSettings::RemoveSettings() {
    QSettings settings;
    settings.remove("");
}

//void GlobalSettings::InitAll() {
//    InitNDS();
//}

bool GlobalSettings::InitDB(const QString &aSchemaName) {
    bool res = false;
    QSqlQuery query("alter session set current_schema = " + aSchemaName, db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("InitDB", query);
    } else {
        if (!query.exec()) {
            gLogger->ShowSqlError("InitDB", query);
        } else {
            QSqlQuery query2("select pp.getbasename from dual", db);
            if (query2.lastError().isValid()) {
                gLogger->ShowSqlError("InitDB", query2);
            } else {
                if (query2.next()) {
                    CurrentSchema = aSchemaName;
                    BaseName = query2.value(0).toString();
                    BaseNameOnly = BaseName.left(BaseName.indexOf('/'));

                    if (LocalCache.PathAuto) {
                        LocalCache.Path = QCoreApplication::applicationDirPath().remove(QRegularExpression("/[^/]*$")).remove(QRegularExpression("/[^/]*$"));
                        LocalCache.Path += "/temp/cache-" + BaseName;
                        LocalCache.Path = LocalCache.Path.remove(QRegularExpression("/[^/]*$"));
                    }
                    if (LocalCache.Path.right(1) != "/") LocalCache.Path += "/";
                    QDir lDir;
                    if (!lDir.mkpath(LocalCache.Path)) {
                        gLogger->ShowError(tr("Local cache"), tr("Can't create directory for local cache") + "\r\n" + LocalCache.Path);
                    } else {
                        if (!gFileUtils->HasEnoughSpace(LocalCache.Path, LocalCache.MinDiskSize)) {
                            QMessageBox::warning(gMainWindow, tr("Local cache"), tr("Low space on disk") + " " + LocalCache.Path.left(2));
                        }
                    }

                    QSqlQuery query3("select contract_mode from settings", db);
                    if (query3.lastError().isValid()) {
                        gLogger->ShowSqlError("InitDB", query3);
                    } else {
                        if (query3.next()) {
                            ContractMode = query3.value(0).toInt();
                            RunOldMessageData("");
                            res = true;
                        } else {
                            gLogger->ShowError("Settings", "No data found");
                        }
                    }
                } else {
                    gLogger->ShowError("Settings", "No data found");
                }
            }
        }
    }
    return res;
}

bool GlobalSettings::InitDB(const QString &aSchemaName, const QString &aBaseName) {
    bool res = false;
    CurrentSchema = aSchemaName;
    BaseName = aBaseName;
    BaseNameOnly = BaseName.left(BaseName.indexOf('/'));

    if (LocalCache.PathAuto) {
        LocalCache.Path = QCoreApplication::applicationDirPath().remove(QRegularExpression("/[^/]*$")).remove(QRegularExpression("/[^/]*$"));
        LocalCache.Path += "/temp/cache-" + BaseName;
        LocalCache.Path = LocalCache.Path.remove(QRegularExpression("/[^/]*$"));
    }
    if (LocalCache.Path.right(1) != "/") LocalCache.Path += "/";
    QDir lDir;
    if (!lDir.mkpath(LocalCache.Path)) {
        gLogger->ShowError(tr("Local cache"), tr("Can't create directory for local cache") + "\r\n" + LocalCache.Path);
    } else {
        if (!gFileUtils->HasEnoughSpace(LocalCache.Path, LocalCache.MinDiskSize)) {
            QMessageBox::warning(gMainWindow, tr("Local cache"), tr("Low space on disk") + " " + LocalCache.Path.left(2));
        }
    }

    QSqlQuery query3("select contract_mode from settings", db);
    if (query3.lastError().isValid()) {
        gLogger->ShowSqlError("InitDB", query3);
    } else {
        if (query3.next()) {
            ContractMode = query3.value(0).toInt();
            RunOldMessageData("");
            res = true;
        } else {
            gLogger->ShowError("Settings", "No data found");
        }
    }

    // ----------------------------------------
    Features.AcadSupFiles = gHasModule("AcadSupFiles")
            && gUserRight->CanSelect("v_as_file")
            && gHomeData->Get("PLOT_STYLES").toInt() == 1;
    Features.ReportMaatzXls = gMSOffice->IsExcelInstalled()
            && QFile::exists(QCoreApplication::applicationDirPath() + "/../../common/templates/matz-v1.xls");
    Features.ReportMetroXls = gMSOffice->IsExcelInstalled()
            && QFile::exists(QCoreApplication::applicationDirPath() + "/../../common/templates/metro-v1.xls");
    Features.CanRLO = gHomeData->Get("CAN_RLO").toInt() == 1;

    return res;
}

bool GlobalSettings::AskPassword(const QString &aTitle, const QString &aText) {
    // caps off
    bool res = false;
    if (GetKeyState(VK_CAPITAL) & 1) {
        INPUT lInput[2];
        lInput[0].type = INPUT_KEYBOARD;
        lInput[0].ki.wVk = VK_CAPITAL;
        lInput[0].ki.dwFlags = 0;
        lInput[0].ki.time = 0;
        lInput[0].ki.dwExtraInfo = 0;
        memcpy(&lInput[1], &lInput[0], sizeof(INPUT));
        lInput[0].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(2, (LPINPUT) &lInput, sizeof(INPUT));
    }
    // set english
    wchar_t lOrigLayoutName[KL_NAMELENGTH];
    if (GetKeyboardLayoutName(lOrigLayoutName)) {
        if ((QString::fromWCharArray(lOrigLayoutName).toInt(NULL, 16) & 0xFFFF) != 0x0409) {
            LoadKeyboardLayout(L"00000409", KLF_ACTIVATE);
        }
    }
    bool lOk;
reAskPass:
    if (!(res = QInputDialog::getText(gMainWindow, aTitle, aText,
                                    QLineEdit::Password, QString(), &lOk, Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::Dialog /*0x8003003*/) == db.password())) {
        if (lOk) {
            QString lNewLine, lCaps, lLocaleName;
            wchar_t lLayoutName[KL_NAMELENGTH];

            if (GetKeyState(VK_CAPITAL) & 1) {
                lNewLine = "\n";
                lCaps = "\n" + tr("CAPS LOCK IS ON!");
            }

            if (GetKeyboardLayoutName(lLayoutName)) {
                if ((QString::fromWCharArray(lLayoutName).toInt(NULL, 16) & 0xFFFF) != 0x0409) {
                    if (lNewLine.isEmpty()) lNewLine = "\n";
                    lLocaleName = "\n" + tr("Current input locale is not EN!");
                }
            }

            QMessageBox::critical(gMainWindow, aTitle, tr("Entered password is wrong!") + lNewLine + lCaps + lLocaleName);
            goto reAskPass;
        }
    }
    return res;
}

void GlobalSettings::InitNDS() {
    // old style
    mNDS = gHomeData->Get("NDS").toDouble();

    // new style - with history
    QSqlQuery query("select start_date, nds from v_pkz_nds order by start_date desc", db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(tr("GlobalSettings::InitNDS()"), query);
    } else {
        while (query.next()) {
            mNDSList.append(qMakePair(qDate("start_date"), qDouble("nds")));
        }
    }
}

double GlobalSettings::GetNDS(const QDate aStartDate) {
    for (int i = 0; i < mNDSList.length(); i++) {
        if (aStartDate >= mNDSList.at(i).first) {
            return mNDSList.at(i).second;
        }
    }
    return mNDS;
}

QString GlobalSettings::FormatSumForList(double aSum) {
    //return locale->toCurrencyString(aSum);
    return locale->toString(aSum / 100, 'f', 2);
}

QString GlobalSettings::FormatSumForEdit(qlonglong aSum) {
    QString res;

    if (aSum) {
        res = QString::number(aSum);
        if (aSum > 0) {
            if (res.length() < 2) res.prepend('0');
            res.insert(res.length() - 2, '.');
            if (res.left(1) == ".") res.prepend('0');
        } else {
            if (res.length() < 3) res.insert(1, '0');
            res.insert(res.length() - 2, '.');
            if (res.mid(1, 1) == ".") res.insert(1, '0');
        }
    } else {
        res = "0.00";
    }

    return res;
}

QVariant GlobalSettings::GetSumFromEdit(QString lStr) {
    QVariant res(QVariant::ULongLong);

    if (lStr.isEmpty()) return res;

    if (lStr.indexOf('.') == -1) {
        lStr += ".00";
    } else {
        if (lStr.indexOf('.') < lStr.length() - 3) {
            lStr.truncate(lStr.indexOf('.') + 3);
        }

        if (lStr.mid(lStr.length() - 3, 1) != ".") lStr += "0";
        if (lStr.mid(lStr.length() - 3, 1) != ".") lStr += "0";
    }

    lStr.remove(lStr.length() - 3, 1);

    res = lStr.toLongLong();

    return res;
}

QString GlobalSettings::FormatNdsForEdit(qlonglong aSum) {
    QString res;

    if (aSum != 0) {
        res = QString::number(aSum);
        res.insert(res.length() - 2, '.');

        if (res.right(1) == "0") res.truncate(res.length() - 1);
        if (res.right(1) == "0") res.truncate(res.length() - 1);
        if (res.right(1) == ".") res.truncate(res.length() - 1);
    } else {
        res = "0";
    }

    return res;
}

QString GlobalSettings::FormatNumber(double anum) {
    QString res;
    res = QString::number(anum, 'f', 2);
    if (res.indexOf('.') != -1 || res.indexOf(',') != -1)
        while (res.right(1) == "0") res.truncate(res.length() - 1);
    if (res.right(1) == "." || res.right(1) == ",") res.truncate(res.length() - 1);
    return res;
}

QString GlobalSettings::FormatNumber(qint64 aNum) {
    return locale->toString(aNum);
}

unsigned int GlobalSettings::maPrime2cHash(const QByteArray &aBA)
{
    static const unsigned char sTable[256] =
    {
        0xa3,0xd7,0x09,0x83,0xf8,0x48,0xf6,0xf4,0xb3,0x21,0x15,0x78,0x99,0xb1,0xaf,0xf9,
        0xe7,0x2d,0x4d,0x8a,0xce,0x4c,0xca,0x2e,0x52,0x95,0xd9,0x1e,0x4e,0x38,0x44,0x28,
        0x0a,0xdf,0x02,0xa0,0x17,0xf1,0x60,0x68,0x12,0xb7,0x7a,0xc3,0xe9,0xfa,0x3d,0x53,
        0x96,0x84,0x6b,0xba,0xf2,0x63,0x9a,0x19,0x7c,0xae,0xe5,0xf5,0xf7,0x16,0x6a,0xa2,
        0x39,0xb6,0x7b,0x0f,0xc1,0x93,0x81,0x1b,0xee,0xb4,0x1a,0xea,0xd0,0x91,0x2f,0xb8,
        0x55,0xb9,0xda,0x85,0x3f,0x41,0xbf,0xe0,0x5a,0x58,0x80,0x5f,0x66,0x0b,0xd8,0x90,
        0x35,0xd5,0xc0,0xa7,0x33,0x06,0x65,0x69,0x45,0x00,0x94,0x56,0x6d,0x98,0x9b,0x76,
        0x97,0xfc,0xb2,0xc2,0xb0,0xfe,0xdb,0x20,0xe1,0xeb,0xd6,0xe4,0xdd,0x47,0x4a,0x1d,
        0x42,0xed,0x9e,0x6e,0x49,0x3c,0xcd,0x43,0x27,0xd2,0x07,0xd4,0xde,0xc7,0x67,0x18,
        0x89,0xcb,0x30,0x1f,0x8d,0xc6,0x8f,0xaa,0xc8,0x74,0xdc,0xc9,0x5d,0x5c,0x31,0xa4,
        0x70,0x88,0x61,0x2c,0x9f,0x0d,0x2b,0x87,0x50,0x82,0x54,0x64,0x26,0x7d,0x03,0x40,
        0x34,0x4b,0x1c,0x73,0xd1,0xc4,0xfd,0x3b,0xcc,0xfb,0x7f,0xab,0xe6,0x3e,0x5b,0xa5,
        0xad,0x04,0x23,0x9c,0x14,0x51,0x22,0xf0,0x29,0x79,0x71,0x7e,0xff,0x8c,0x0e,0xe2,
        0x0c,0xef,0xbc,0x72,0x75,0x6f,0x37,0xa1,0xec,0xd3,0x8e,0x62,0x8b,0x86,0x10,0xe8,
        0x08,0x77,0x11,0xbe,0x92,0x4f,0x24,0xc5,0x32,0x36,0x9d,0xcf,0xf3,0xa6,0xbb,0xac,
        0x5e,0x6c,0xa9,0x13,0x57,0x25,0xb5,0xe3,0xbd,0xa8,0x3a,0x01,0x05,0x59,0x2a,0x46
    };

#define PRIME_MULT 1717
    int i;
    unsigned int hash = aBA.length();

    for (i = 0; i < aBA.length(); i++)
    {

        hash ^= sTable[((unsigned char) aBA.at(i)) & 255];
        hash = hash * PRIME_MULT;
    }

#undef PRIME_MULT

    return hash;
}

void GlobalSettings::CalcNdsAndFull(const QLineEdit *aSum, const QLineEdit *aNdsPerCent, QLineEdit *aSumNds, QLineEdit *aSumFull) {
    if (!aSum->text().isEmpty() && !aNdsPerCent->text().isEmpty()) {
        qlonglong lSum, lSumFull;
        lSum = GetSumFromEdit(aSum->text()).toLongLong();
        lSumFull = (5000L + lSum * (GetSumFromEdit(aNdsPerCent->text()).toLongLong() + 10000L)) / 10000L;

        aSumFull->setText(FormatSumForEdit(lSumFull));
        aSumNds->setText(FormatSumForEdit(lSumFull - lSum));
    } else {
        aSumFull->setText("");
        aSumNds->setText("");
    }
}

void GlobalSettings::CalcFullWhenNdsSumChanged(const QLineEdit *aSum, const QLineEdit *aSumNds, QLineEdit *aSumFull) {
    if (!aSum->text().isEmpty() && !aSumNds->text().isEmpty())
        aSumFull->setText(FormatSumForEdit(GetSumFromEdit(aSum->text()).toLongLong() + GetSumFromEdit(aSumNds->text()).toLongLong()));
    else
        aSumFull->setText("");
}

void GlobalSettings::CalcNdsAndBrutto(const QLineEdit *aSumFull, const QLineEdit *aNdsPerCent, QLineEdit *aSumNds, QLineEdit *aSum) {
    // lSum = lSumFull * 100 / (NDSPerCent + 100)

    if (!aSumFull->text().isEmpty() && !aNdsPerCent->text().isEmpty()) {
        qlonglong lSum, lSumFull, lNdsPercent;
        lSumFull = GetSumFromEdit(aSumFull->text()).toLongLong();
        lNdsPercent = GetSumFromEdit(aNdsPerCent->text()).toLongLong();
        lSum = ((lNdsPercent + 10000L) / 2 + lSumFull * 10000L) / (lNdsPercent + 10000L);

        aSum->setText(FormatSumForEdit(lSum));
        aSumNds->setText(FormatSumForEdit(lSumFull - lSum));
    } else {
        aSum->setText("");
        aSumNds->setText("");
    }
}

QString GlobalSettings::MonthName(int aMonth) {
    return locale->monthName(aMonth);
}

void GlobalSettings::CopyToClipboard(const QTreeWidget *tw) {
    QList <QTreeWidgetItem *> selected = tw->selectedItems();

    if (!selected.count()) return; // some internal shit happened

    int i, j;
    QString lCopy;

    for (i = 0; i < tw->columnCount(); i++) {
        if (!tw->isColumnHidden(i)) {
            lCopy += tw->headerItem()->text(i).trimmed().replace('\r\n', ". ") + '\t';
        }
    }
    lCopy += "\r\n";

    for (i = 0; i < selected.count(); i++) {
        for (j = 0; j < tw->columnCount(); j++) {
            if (!tw->isColumnHidden(j)) {
                lCopy += selected.at(i)->text(j).trimmed().replace('\r\n', ". ") + '\t';
            }
        }
        lCopy += "\r\n";
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(lCopy);

    if (gSettings->Common.ShowAfterCopy) {
        QMessageBox mb(gMainWindow);
        mb.setIcon(QMessageBox::Information);
        mb.setWindowTitle(tr("Projects Base"));
        mb.setText(tr("You can paste copied data into Microsoft Excel or Microsoft Word"));
        mb.addButton(QMessageBox::Ok);

        QCheckBox *cb = new QCheckBox(&mb);
        cb->setText(tr("Don't show this message again"));
        mb.setCheckBox(cb);
        mb.exec();
        gSettings->Common.ShowAfterCopy = !cb->isChecked();
    }
}

void GlobalSettings::CopyToClipboard(const QTableWidget *tw) {
    QList <QTableWidgetItem *> selected = tw->selectedItems();

    if (!selected.count()) return; // some internal shit happened

    int i, j;
    QString lCopy;
    QList<int> lRows;

    for (i = 0; i < selected.count(); i++) {
        if (!lRows.contains(selected.at(i)->row())) lRows.append(selected.at(i)->row());
    }

    std::sort(lRows.begin(), lRows.end(),
              [] (int d1, int d2) { return d1 < d2;});

    for (i = 0; i < lRows.count(); i++) {
        for (j = 0; j < tw->columnCount(); j++) {
            QTableWidgetItem * lItem = tw->item(lRows.at(i), j);
            if (!tw->isColumnHidden(j)) {
                lCopy += lItem->text().trimmed().replace('\r\n', ". ") + '\t';
            }
        }
        lCopy += "\r\n";
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(lCopy);

    if (gSettings->Common.ShowAfterCopy) {
        QMessageBox mb(gMainWindow);
        mb.setIcon(QMessageBox::Information);
        mb.setWindowTitle(tr("Projects Base"));
        mb.setText(tr("You can paste copied data into Microsoft Excel or Microsoft Word"));
        mb.addButton(QMessageBox::Ok);

        QCheckBox *cb = new QCheckBox(&mb);
        cb->setText(tr("Don't show this message again"));
        mb.setCheckBox(cb);
        mb.exec();
        gSettings->Common.ShowAfterCopy = !cb->isChecked();
    }
}

void GlobalSettings::PopulateCustListForContract()
{
    QSqlQuery query("SELECT DISTINCT CUSTOMER FROM V_PKZ_CONTRACT ORDER BY UPPER(CUSTOMER)", db);

    mCustListForContract.clear();

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("PopulateCustListForContract", query);
    } else {
        while (query.next()) {
            mCustListForContract.append(query.value(0).toString());
        }
    }
}

void GlobalSettings::DoExternalExe(const QString &aCommand, int aId, int aIdType, int aWhatToDo, const QString &aAddParams) {
    QString cmdLine;
    cmdLine = QCoreApplication::applicationDirPath();
    cmdLine.resize(cmdLine.lastIndexOf(QChar('/')));

    cmdLine += "/oraint.exe \"" + db.databaseName() + "\" \""
            + db.userName() + "\" \"" + db.password() + "\" \""
            + ((db.driverName() == "QPSQL")?db.hostName():CurrentSchema) + "\" " + aCommand + " " + QString::number(aIdType) + " " + QString::number(aId)
            + " " + QString::number(aWhatToDo) + " " + aAddParams;

//    QMessageBox mb;
//    mb.setDetailedText(cmdLine);
//    mb.exec();

    //QProcess::startDetached(cmdLine);
    QProcess proc1;
    proc1.start(cmdLine);
    if (!proc1.waitForStarted(-1)) {
        gLogger->ShowError("Interface - wait for started", proc1.errorString());
    } else {
        if (!proc1.waitForFinished(-1)) {
            gLogger->ShowError("Interface - wait for finished", proc1.errorString());
        } else {
            // everything is ok, baby
        }
    }
}

void GlobalSettings::DoOpenNonDwg(int aId, int aIdType, int aWhatToDo, const QString &aAddParams) {
    DoExternalExe("OpenNonDwg", aId, aIdType, aWhatToDo, aAddParams);
}

void GlobalSettings::DoOpenDwgNew(MainDataForCopyToAcad &aOpenData, int aAcadParamListIndex) {
    if (InitAcad(aAcadParamListIndex)) {
        int i, j;
        COPYDATASTRUCT CpyData;

        //WaitAcadDlgThread lThread;
        //lThread.start();
        //WaitAcadDlg w;

        //w.show();

        //w.AddString("What to do: " + QString::number(aOpenData.WhatToDo()));

        CpyData.dwData = 5;
        CpyData.cbData = MainDataForCopyToAcad::GetDataSize();
        CpyData.lpData = aOpenData.GetDataBuffer();

        if (SendMessage(gAcad, WM_COPYDATA, 0, (LPARAM) &CpyData) != RET_OK) {
            gLogger->ShowError("Sending data to AutoCAD", "Error sending data (start) to AutoCAD!");
        } else {
            bool lIsOk = true;
            for (i = 0; i < aOpenData.ListConst().length() && lIsOk; i++) {

                CpyData.dwData = 6;
                CpyData.cbData = RecordDataForCopyToAcad::GetDataSize();
                CpyData.lpData = aOpenData.ListConst()[i]->GetDataBuffer();

                //w.AddString(QString::number(i + 1));
                //w.update();

                if (SendMessage(gAcad, WM_COPYDATA, 0, (LPARAM) &CpyData) == RET_OK) {
                    if (aOpenData.ListConst()[i]->XrefPropsConst()) {
                        for (j = 0; j < aOpenData.ListConst()[i]->XrefPropsConst()->XrefPropsConst().length() && lIsOk; j++) {
                            CpyData.dwData = 7;
                            CpyData.cbData = XrefOnePropData::GetDataSize();
                            CpyData.lpData = aOpenData.ListConst()[i]->XrefPropsRef()->XrefPropsRef()[j]->GetDataBuffer();
                            if (SendMessage(gAcad, WM_COPYDATA, 0, (LPARAM) &CpyData) != RET_OK) {
                                gLogger->ShowError("Sending data to AutoCAD", "Error sending data (properties list) to AutoCAD!");
                                lIsOk = false;
                                break;
                            }
                        }
                    }

                    for (j = 0; j < aOpenData.ListConst()[i]->XrefRenameListRef().length() && lIsOk; j++) {
                        CpyData.dwData = 8;
                        CpyData.cbData = XrefRenameData::GetDataSize();
                        CpyData.lpData = aOpenData.ListConst()[i]->XrefRenameListRef()[j]->GetDataBuffer();
                        if (SendMessage(gAcad, WM_COPYDATA, 0, (LPARAM) &CpyData) != RET_OK) {
                            gLogger->ShowError("Sending data to AutoCAD", "Error sending data (xref rename list) to AutoCAD!");
                            lIsOk = false;
                            break;
                        }

                    }

                    for (j = 0; j < aOpenData.ListConst()[i]->ImageRenameListRef().length() && lIsOk; j++) {
                        CpyData.dwData = 9;
                        CpyData.cbData = XrefRenameData::GetDataSize();
                        CpyData.lpData = aOpenData.ListConst()[i]->ImageRenameListRef()[j]->GetDataBuffer();
                        if (SendMessage(gAcad, WM_COPYDATA, 0, (LPARAM) &CpyData) != RET_OK) {
                            gLogger->ShowError("Sending data to AutoCAD", "Error sending data (image rename list) to AutoCAD!");
                            lIsOk = false;
                            break;
                        }

                    }
                } else {
                    gLogger->ShowError("Sending data to AutoCAD", "Error sending data (list) to AutoCAD!");
                    lIsOk = false;
                    break;
                }
            }

            if (lIsOk) {
                MainDataForCopyToAcad lMainData(-1);

                CpyData.dwData = 5;
                CpyData.cbData = MainDataForCopyToAcad::GetDataSize();
                CpyData.lpData = lMainData.GetDataBuffer();

                LRESULT lRes = SendMessage(gAcad, WM_COPYDATA, 0, (LPARAM) &CpyData);

                int lI = 0;
                while (lRes == 1 /* no acDocManager or not in application context */
                       && lI < 60) {
                    Sleep(1000);
                    lRes = SendMessage(gAcad, WM_COPYDATA, 0, (LPARAM) &CpyData);
                    lI++;
                }
                if (lI > 0) {
                    gLogger->LogErrorToDb("lRes: " + QString::number(lRes) + ", lI: " + QString::number(lI));
                }

                if (lRes != RET_OK) {
                    gLogger->ShowError("Sending data to AutoCAD", "Error sending data (end) to AutoCAD!");
                }
            }
        }
    }
}

void GlobalSettings::RunForm(const QString &aFormname, const QString &aParams) const {
    QString lBinPath, lLang = "EN", lML = "10";

    {
        QSettings lSettings;
        lSettings.beginGroup("LastLogin");
        lLang = lSettings.value("Lang").toString();
        lSettings.endGroup();
    }

    QString cmdLine;

    lBinPath = QCoreApplication::applicationDirPath();
    lBinPath.replace('/', '\\');
    lBinPath.resize(lBinPath.lastIndexOf(QChar('\\')) + 1);
    cmdLine = QString(qgetenv("COMSPEC")) + " /c " + lBinPath;

    cmdLine.resize(cmdLine.lastIndexOf(QChar('\\')));
    cmdLine.resize(cmdLine.lastIndexOf(QChar('\\')) + 1);

    cmdLine += "common\\runform.bat \"" + db.databaseName() + "\" \"" + db.userName() + "\" \"" + db.password() + "\" \""
            + lBinPath + "\" " + lLang + " " + lML + " \""
            + CurrentSchema + "\" " + aFormname + " " + aParams;

    QProcess proc1;
    proc1.start(cmdLine);
    if (!proc1.waitForStarted(-1)) {
        gLogger->ShowError("Interface - wait for started", proc1.errorString());
    } else {
        if (!proc1.waitForFinished(-1)) {
            gLogger->ShowError("Interface - wait for finished", proc1.errorString());
        }
    }
}

void GlobalSettings::RunOldMessageData(const QString &aParams) const {
    QString cmdLine;

    cmdLine = QCoreApplication::applicationDirPath();
    cmdLine.resize(cmdLine.lastIndexOf(QChar('/')) + 1);
    cmdLine += "MessageData.exe \"" + db.databaseName() + "\" \"" + db.userName() + "\" \"" + db.password() + "\" \""
            + CurrentSchema + "\" "
            + aParams;

    QProcess::startDetached(cmdLine);
}

void GlobalSettings::SaveToLocalCache(int aIdDwg, const QByteArray &aData, const QString &aSha256) {
    if (!LocalCache.UseLocalCache) return;

    QFile lFile(LocalCache.Path + "d-" + QString::number(aIdDwg));

    if (lFile.exists()) {
        // you can turn paranoja mode on here and compare by sha256;
        QCryptographicHash lHash(QCryptographicHash::Sha256);
        if (lFile.open(QFile::ReadOnly)) {
            lHash.addData(&lFile);
            lFile.close();
            if (QString(lHash.result().toHex()).toUpper() != aSha256.toUpper()) {
                gLogger->ShowError(tr("Local cache"), "ERROR IN LOCAL CACHE, HASH IS DIFFERENT\r\n" + lFile.fileName());
            } else {
                gFileUtils->SetFileTime(lFile.fileName(), QDateTime::currentDateTime());
            }
        } else {
            gLogger->ShowError(tr("Local cache"), tr("Can't open file in local cache") + "\r\n" + lFile.fileName() + "\r\n" + lFile.errorString());
        }
    } else {
        if (gFileUtils->HasEnoughSpace(LocalCache.Path, LocalCache.MinDiskSize)) {
            if (lFile.open(QFile::WriteOnly)) {
                lFile.write(aData);
                lFile.close();
            }
        } else {
            lFile.remove();
        }
    }
}

void GlobalSettings::CheckLocalCache() {
    //qint64 lStart = QDateTime::currentMSecsSinceEpoch();

    QDir lDir(LocalCache.Path);
    QStringList lFilters;
    lFilters << "d-*";

    QSqlQuery query(db);
    query.prepare("select count(*) from v_dwg where id = :id and sha256 = :sha256");
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("GlobalSettings::CheckLocalCache", query);
    } else {
        QFileInfoList lFiles = lDir.entryInfoList(lFilters, QDir::Files);
        foreach (const QFileInfo &lFileInfo, lFiles) {

            QFile lFile(lFileInfo.filePath());
            if (lFile.open(QFile::ReadOnly)) {
                QCryptographicHash hash1(QCryptographicHash::Sha256);
                hash1.addData(&lFile);
                lFile.close();

                query.bindValue(":id", lFileInfo.fileName().mid(2).toInt());
                query.bindValue(":sha256", QString(hash1.result().toHex()).toUpper());

                if (!query.exec()) {
                    gLogger->ShowSqlError("GlobalSettings::CheckLocalCache", query);
                } else {
                    if (query.next()) {
                        if (query.value(0).toInt() != 1) {
                            if (lFile.remove()) {
                                gLogger->LogError(lFileInfo.filePath() + " - BAD - DELETED");
                            } else {
                                gLogger->ShowError("GlobalSettings::CheckLocalCache", lFileInfo.filePath() + " - BAD - CAN'T DELETE!");
                            }
                        }
                    } else {
                        gLogger->ShowError("GlobalSettings::CheckLocalCache", "count(*) - no data");
                    }
                }
            }
        }
    }
    //gLogger->LogError("Local cache checked in " + QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStart)) / 1000) + " s.");
}

bool GlobalSettings::IsDialogModal(const QWidget *aParent) {
    const QWidget *lWidget = aParent;
    while (lWidget && !qobject_cast<const QDialog *>(lWidget)) {
        lWidget = lWidget->parentWidget();
    }
    if (qobject_cast<const QDialog *>(lWidget)) {
        return qobject_cast<const QDialog *>(lWidget)->isModal();
    } else {
        return false;
    }
}

QString GetCommonString(const QString &aStr1, const QString &aStr2, const QChar &aX) {
    QString lRes;

    for (int i = 0; i < aStr1.length() && i < aStr2.length(); i++) {
        if (aStr1.at(i) == aStr2.at(i)) {
            lRes += aStr1.at(i);
        } else {
            lRes += aX;
        }
    }

    if (lRes.length() < aStr1.length()) lRes = lRes.leftJustified(aStr1.length() - lRes.length(), aX);
    if (lRes.length() < aStr2.length()) lRes = lRes.leftJustified(aStr2.length() - lRes.length(), aX);

    return lRes;
}

QString ExpandNumbersInString(const QString & aStr1) {
    int i, iStart, j;
    QString res;
    for (i = 0; i < aStr1.length(); i++) {
        if (aStr1[i] < '0'
                || aStr1[i] > '9') {
            res += aStr1[i];
        } else {
            iStart = i;
            i++;
            while (i < aStr1.length() && aStr1[i] >= '0' && aStr1[i] <= '9') i++;
            for (j = 0; j < 18 - (i - iStart); j++) {
                res += '0';
            }
            res += aStr1.mid(iStart, i - iStart + 1);
        }
    }
    return res;
}

bool CmpStringsWithNumbers(const QString & aStr1, const QString & aStr2) {
    return ExpandNumbersInString(aStr1) < ExpandNumbersInString(aStr2);
}

bool CmpStringsWithNumbersNoCase(const QString & aStr1, const QString & aStr2) {
    return ExpandNumbersInString(aStr1).toUpper() < ExpandNumbersInString(aStr2).toUpper();
}
