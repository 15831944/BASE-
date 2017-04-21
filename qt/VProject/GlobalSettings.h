#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QCoreApplication>
#include <QObject>
#include <QColor>
#include <QLineEdit>
#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTableWidget>
#include <QSettings>

#include "AcadExchange.h"

#define RET_OK  12378L
#define RET_OK1 12379L
#define RET_OK2 12380L
#define RET_ERROR 45781L

#include "def_expimp.h"

EXP_IMP void RunAndShowReport(const QString &aVbsName, const QString &aOutName, bool aUseNew = false);
EXP_IMP bool CmpStringsWithNumbers(const QString & aStr1, const QString & aStr2);
EXP_IMP bool CmpStringsWithNumbersNoCase(const QString & aStr1, const QString & aStr2);

class InstalledAcadData {
private:
    bool mDiffProduct, mDiffLang;
    int mVersion;
    QString mName, mKey, mProduct, mLang;
    QString mPath;
    REGSAM mFuckingWow;
public:
    InstalledAcadData(const QString &aName, const QString &aKey, const QString &aPath, REGSAM aFuckingWow);
    InstalledAcadData(InstalledAcadData *aInstalledAcadData);

    void SetDiffProduct(bool aDiffProduct);
    void SetDiffLang(bool aDiffLang);

    const QString &ProductConst() const;
    void SetProduct(const QString &aProduct);

    const QString &LangConst() const;
    void SetLang(const QString &aLang);

    QString FullProductName() const;
    QString FullDisplayName() const;
    QString FullCommandLine() const;
    QStringList Profiles() const;

    int Version() const;
//    const QString &NameConst() const; need not
    const QString &KeyConst() const;
    const QString &PathConst() const;
    REGSAM FuckingWow() const;
};

class InstalledAcadList : public QList<InstalledAcadData *>
{
public:
    const InstalledAcadData *GetByProductName(const QString &aFullProductName) const;
};

class AcadParamData {
private:
    bool mDiffProfile;
    QString mAddName, mFullProductName;

    QString mUserProfile, mCommandLine;
    uint mStartWithNoLogo, mLoadAecBase; // bool in fact
    uint mDelayAfterStart, mMaxWaitForStart, mStartCheckInterval, mDelayAfterOpen, mLOBBufferSize;
public:
    AcadParamData(const QString &aAddName, const QString &aFullProductName,
                  const QString &aUserProfile, bool aLoadAecBase, bool aDiffProfile);
    AcadParamData(QSettings &aSettings);
    AcadParamData(const AcadParamData *aAcadParamData);

    void CopyFromOther(const AcadParamData *aAcadParamData);

    void LoadSettings(QSettings &aSettings);
    void SaveSettings(QSettings &aSettings);

    QString FullDisplayName() const;
    QString FullCommandLine() const;

    const QString &AddNameConst() const;
    void SetAddName(const QString &aAddName);

    const QString &FullProductNameConst() const;
    void SetFullProductName(const QString &aFullProductName);

    const QString &UserProfileConst() const;
    void SetUserProfile(const QString &aUserProfile);

    const QString &CommandLineConst() const;
    void SetCommandLine(const QString &aCommandLine);

    uint StartWithNoLogo() const;
    void SetStartWithNoLogo(uint aStartWithNoLogo);

    uint LoadAecBase() const;
    void SetLoadAecBase(uint aLoadAecBase);

    uint DelayAfterStart() const;
    void SetDelayAfterStart(uint aDelayAfterStart);

    uint MaxWaitForStart() const;
    void SetMaxWaitForStart(uint aMaxWaitForStart);

    uint StartCheckInterval() const;
    void SetStartCheckInterval(uint aStartCheckInterval);

    uint DelayAfterOpen() const;
    void SetDelayAfterOpen(uint aDelayAfterOpen);

    uint LOBBufferSize() const;
    void SetLOBBufferSize(uint aLOBBufferSize);
};

Q_DECLARE_METATYPE(AcadParamData *)

class RunningAcadData
{
private:
    HWND mHwnd;
    QString mCaption;
    bool mConnected;
public:
    RunningAcadData(HWND aHwnd, const QString aCaption, bool aConnected);

    HWND GetHwnd() const;
    const QString &GetCaption() const;
    bool GetConnected() const;
};

Q_DECLARE_METATYPE(RunningAcadData *)

class RunningAcadList : public QList<RunningAcadData *>
{
public:
    ~RunningAcadList();

};

class EXP_IMP GlobalSettings : public QObject
{
    Q_OBJECT
protected:
    explicit GlobalSettings(QObject *parent = NULL);
    ~GlobalSettings();

    static void clean();
public:
    static GlobalSettings *GetInstance();

    uint DebugOutput;

    void InitAcadParamsList(QList<AcadParamData *> &aParams);
    void LoadAcadParams();
    void SaveAcadParams();

    void SaveSettings();
    void SaveSettingsImage(QSettings *aSettings = NULL);
    void RemoveSettings();

    bool InitDB(const QString &aSchemaName);
    bool InitDB(const QString &aSchemaName, const QString &aBaseName);

    bool AskPassword(const QString &aTitle, const QString &aText);

    //void InitAll();

    typedef QPair<QDate, double> tNDSPair;
    typedef QList<tNDSPair> tNDSList;
protected:
    // current nds for new contracts and etc.
    double mNDS;
    tNDSList mNDSList;
public:
    void InitNDS();

    double GetNDS(const QDate aStartDate);

    // output sum
    QString FormatSumForList(double aSum);

    QString FormatSumForEdit(qlonglong aSum);
    QVariant GetSumFromEdit(QString lStr);

    QString FormatNdsForEdit(qlonglong aSum);
    QString FormatNumber(double aNum);
    QString FormatNumber(qint64 aNum);

    unsigned int maPrime2cHash(const QByteArray &aBA);

    void CalcNdsAndFull(const QLineEdit *aSum, const QLineEdit *aNdsPerCent, QLineEdit *aSumNds, QLineEdit *aSumFull);
    void CalcFullWhenNdsSumChanged(const QLineEdit *aSum, const QLineEdit *aSumNds, QLineEdit *aSumFull);
    void CalcNdsAndBrutto(const QLineEdit *aSumFull, const QLineEdit *aNdsPerCent, QLineEdit *aSumNds, QLineEdit *aSum);

    QString MonthName(int aMonth);

    void CopyToClipboard(const QTreeWidget *tw);
    void CopyToClipboard(const QTableWidget *tw);

    QString CurrentSchema; // working scheme

    QString BaseName; // base name for windows captions (with username)
    QString BaseNameOnly; // base name without username

    HWND gAcad;

    // let base setting (means server setting, not local) start here

    // 0 - "promos" style, contract numbers for code get from contract
    // 1 - simple style, contract numbers get from "dogovor" field from project and if it null - then from start from short project name
    int ContractMode;
    //

    // I don't know how create it with no pointer
    // used for numbers & currensy conversion
    QLocale *locale;

    struct {
        bool AcadSupFiles;
        bool ReportMaatzXls, ReportMetroXls;
        bool CanRLO;
    } Features;

    struct {
        QString VisualStyle;
        int ConfirmQuit;
        bool SaveWinState, RereadAfterSave;
        int AddRowHeight; // for tree
        int SubRowHeight; // for table widgets
        int AddColWidth; // for table widgets
        bool UseTabbedView;
        QTabWidget::TabPosition TabPos;
        bool ShowAfterCopy;
        QColor RequiredFieldColor, DisabledFieldColor;
    } Common;

    struct {
        bool UseLocalCache;
        qulonglong MinDiskSize;

        bool PathAuto;
        QString Path; // direcrtory (with ending /) for cache
    } LocalCache;

    struct {
        QString LastDir;
    } LoadFiles;

    struct {
        QString LastDir;
    } SaveFiles;

    struct {
        bool UseProjectColor, UseContractColor, UseStageColor;
        QColor ProjectColor, ContractColor, StageColor;
        int ExpandOnStart;
        bool MultiSelect;
    } Contract;

    struct {
        bool AutoHideEmptySigned;
        bool AutoHideSignIfPay;
        bool AutoHideEmptyPayed;

        bool AutoHideEmptyIndexed;
    } Hashbon;

    struct {
        int OnDblClick;
        int SelectBeh;
        int SelectMode;
        int DrawingShowMode;
    } Geobase;

    struct {
        //bool ShowImages, ShowXrefs, ShowAddFiles;

    } SaveDialog;

    struct TypeTreeStruct {
        int ExpandLevel;
        bool FontPlusOne, FontBold;
    } TypeTree;

    struct DocumentTreeStruct {
        enum enumSingleSelect { SSSelected = 0, SSCurrent } SingleSelect;

        enum WNDTitleType { WNDTLong = 0, WNDTShort, WNDTNoGroup } WindowTitleType;

        bool OpenSingleDocument, ShowGridLines, AutoWidth, DocFontPlusOne, DocFontBold, DragDrop, AskPasswordOnDelete;
        enum DBLDOC { DDView = 0, DDProps/*, DDEditInGridView, DDEditInGridProps */} OnDocDblClick;
        enum SLT { SLTNone = 0, SLTLayouts, SLTVersions, SLTHistory, SLTAddFiles } SecondLevelType;
        bool ExpandOnShow;
        int AddRowHeight;

        bool UseDocColor, UseSecondColor;
        QColor DocColor, SecondColor;

        //bool RefreshOnWindowOpen;// refresh project data on new PlotListDlg
    } DocumentTree;

    struct DocumentHistoryStruct {
        bool MDI, AutoWidth;
    } DocumentHistory;

    struct AuditPurgeStruct {
        bool PurgeRegapps, PurgeAll, ExplodeProxy, RemoveProxy, Audit;
    } AuditPurge;

    struct PublishStruct {
        bool PDF, DWF, PLT;
        bool DontScale;
        long UseVersion; // -1 - always ask,
        long CTBType;
        QString CTBName, PlotterName, OutDir;
    } Publish;

    struct CompareStruct {
        bool Readed;
        uint OldColor, NewColor;
        bool OutputDates;
        double TextHeight;
        bool AlwaysAskOutputDir;
        QString OutputDir;
        bool AlwaysAskAll;
    } Compare;

    struct ImageStruct {
        int LoadWhenFNExists; // 0 - load to existing, 1 - voad new version
        int ResizeForPreview;
        int MaxPreviewWidth, MaxPreviewHeight;
        int OnPreviewDblClick;
        int ConvertType;
        int MaxConvertWidth, MaxConvertHeight;
        int ConvertPercent;
        quint64 MaxFileSize;

        int ViewerType;
        QString ViewerPath;
        bool SaveAll, ConfirmViewerClosed;

        int EditorType;
        QString EditorPath;
        bool SaveAllForEditor, ConfirmEditorClosed;
    } Image;

    struct AcadParamsStruct {
        int UseAcadParamIndex;
        uint AlwaysAsk; // 0 - use selected, 1 - ask at first run, 2 - always ask
        uint KeepAfterEdit, KeepAllBackupsLocally; // bool in fact

        QList<AcadParamData *> Params;
    } AcadParams;

    struct ServerSecureStruct {
        int AskPassForUserMan, DropOnFire;
    } ServerSecure;

    void CalcMadadFromSiteGetUrl(int aIndexingType, QDate dateFirst, QDate dateLast, QString aSum, QString &aUrl);
    void CalcMadadFromSite(int aIndexingType, QDate dateFirst, QDate dateLast, QString aSum, QLineEdit *leForIndexed);
    void CalcMadadAbort();

    // -------------------------------------------------------
    QStringList mCustListForContract;
    void PopulateCustListForContract();

    // what to do: 0 - view, 1 - edit, 2 - audit, 3 - recover, 4 - publish, 5 - save to file, 6 - process before load to base, 7 - process after saving
    void DoExternalExe(const QString &aCommand, int aId, int aIdType, int aWhatToDo, const QString &aAddParams);
    //void DoOpenDwg(int aId, int aIdType, int aWhatToDo, const QString &aAddParams);
    void DoOpenNonDwg(int aId, int aIdType, int aWhatToDo, const QString &aAddParams);

    void DoOpenDwgNew(MainDataForCopyToAcad &aOpenData, int aAcadParamListIndex = -1);
    void RunForm(const QString &aFormname, const QString &aParams) const; // deprecated, need rewrite all forms
    void RunOldMessageData(const QString &aParams) const; // deprecated, need rewrite messenger

    void SaveToLocalCache(int aIdDwg, const QByteArray &aData, const QString &aSha256);
    void CheckLocalCache();

    bool static IsDialogModal(const QWidget *aParent);

    QString GetCommonString(const QString &aStr1, const QString &aStr2, const QChar &aX);
    // acad.cpp
private:
    InstalledAcadList mInstalledAcadList;
public:
    const InstalledAcadList &InstalledAcadListConst() const;
    QList<AcadParamData *> &AcadParamsRef();
    long InitAcad(int aAcadParamListIndex = -1);
    void EnumPlotStyles(QStringList &aPlotStyles);
private:
    bool SetAutoLoadARX(const InstalledAcadData *aAcadRegData);
    static void EnumProductsFromKeyInternal(const wchar_t *MainKey, QList<InstalledAcadData *> &aInstalledAcadList);
    static void EnumInstalledProducts(QList<InstalledAcadData *> &aInstalledAcadList);

private:
    QNetworkAccessManager am;
    QList<QNetworkReply *> replies;
    QList<QLineEdit *> lineEdits;
signals:
    void DocTreeSettingsChanged();
    void StyleSheetChanged();

public slots:

    void slotReadyRead();
};

#define gSettings GlobalSettings::GetInstance()

#endif // GLOBALSETTINGS_H
