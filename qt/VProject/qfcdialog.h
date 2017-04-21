#ifndef QFCDIALOG_H
#define QFCDIALOG_H

#pragma warning(disable:4100)

#include <QDialog>
#include <QSettings>

#include "def_expimp.h"

class EXP_IMP QFCDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QFCDialog(QWidget *parent, bool aAutoSaveState);
    virtual ~QFCDialog();
    
    virtual QString AddToClassName() const;

    virtual void SaveState(QSettings &aSettings);
    //virtual void LoadState(QSettings &aSettings);
    void LoadSettings(QSettings &settings);
    void SaveSettings(QSettings &settings);

    void SaveWindowDefaults();

    void MakeNonMDI();
    void MakeMDI();

signals:
    
public slots:
    virtual void done(int r);
    virtual int exec();
private slots:
    void StyleSheetChangedSlot(); // dummy??? TODO1
private:
    bool mJustStarted, mAutoSaveState;

    void LoadWindowDefaults();
protected:
    bool mDoNotSave, mLoadSettings;
    int ReadVersion, CurrentVersion;

    void ReadChildren(QSettings &settings, const QObjectList &c);
    void SaveChildren(QSettings &settings, const QObjectList &c);
    virtual void showEvent(QShowEvent* event);
    //virtual void closeEvent(QCloseEvent* event);
    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);
    virtual void StyleSheetChangedInSescendant();
    virtual void LoadAdditionalSettings(QSettings &aSettings);
    virtual void SaveAdditionalSettings(QSettings &aSettings);

    virtual bool eventFilter(QObject *obj, QEvent *event);
//    virtual void changeEvent(QEvent * event);
};

#endif // QFCDIALOG_H
