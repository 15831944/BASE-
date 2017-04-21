#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include "qfcdialog.h"
#include "TWIForSave.h"

namespace Ui {
class SaveDialog;
}

class SaveDialog : public QFCDialog
{
    Q_OBJECT

private:
    bool mJustStarted;
    qint64 mStartMSecsSinceEpoch;
public:
    explicit SaveDialog(QWidget *parent = 0);
    virtual ~SaveDialog();

    void AddDocument(int aIdPlot, int aIdDwg);

protected:
    // plot for save - it is input data, see AddDocument (and SetIdPlotList exists temporary)
    QList<PlotForSaveData *> mPlotForSaveList;

    // list for storing current versions of xrefs (it can be now current version in Plot data)
    // the primary and only need is to delete it in window destructor
    QList<const XrefForSaveData *> mXrefForSaveForDel;

    //QList<int> lPlotIdList;
    int mMaxAcadVersionForProcess;
    bool mHasAcadDocs;

    int mViewModePrev;
    QByteArray mBADocsSettingsAcad, mBADocsSettingsNonAcad;

    virtual void showEvent(QShowEvent* event);
    virtual void SaveAdditionalSettings(QSettings &aSettings);
    virtual void LoadAdditionalSettings(QSettings &aSettings);

    // it means populate images for drawing and files for non-drawings in main list (twDocs)
    // last param - to add or not to add to the common list (by calling AddImageToCommonList)
    void PopulateImages(TWIForSaveMain * item, bool aIsDwg, bool aAddToList);
    // it means populate "additional files" for autocad
    void PopulateAddFiles(TWIForSaveMain * item);

    void GenerateFilenames();

    void InitListHeader(QTreeWidget *tw);
    // it is "additional files" for AutoCAD drawings
    void AddSuppFileToCommonList(PlotDwgData * aPlotData, DwgForSaveData * aDwgData);
    // it is files for non AutoCAD documents and images for AutoCAD drawings; files for documents-directory not included here
    // images for AutoCAD saved from this list (and can be renamed) while files saved from data in twDocs list (and can't be renamed)
    void AddImageToCommonList(PlotDwgData * aPlotData, DwgForSaveData * aDwgData, TWIForSaveAddFile::RecordTypeEnum aRecordType);
    // it is xrefs for AutoCAD drawings
    void AddXrefToCommonList(PlotForSaveData * aPlot, XrefForSaveData * aXref);

    bool ImageListCheckForDuplicate();
    bool FileListCheckForDuplicate();
    void XrefListCheckForDuplicate();
    void XrefRenameDuplicates(); // this part also called after user select different xref (in dropdown combobox)
    void ProcessLists(int aMode);
    void RecollectAdditionalFiles();
    virtual void StyleSheetChangedInSescendant();
public slots:
private slots:
    void Accept();
    void RecollectData();
    void XrefTreeChanged();
    void on_twDocs_customContextMenuRequested(const QPoint &pos);

    void on_actionCollapse_all_triggered();

    void on_actionExpand_all_triggered();

    void on_tbSelectPath_clicked();

    void on_lePrefix_textEdited(const QString &arg1);

    void on_cbFNPart1_currentIndexChanged(int index);

    void on_leMiddlexif_textEdited(const QString &arg1);

    void on_cbFNPart2_currentIndexChanged(int index);

    void on_lePostfix_textEdited(const QString &arg1);

    void on_cbChanger_currentIndexChanged(int index);

    void on_cbChanger_editTextChanged(const QString &arg1);

    void on_cbNameCase_currentIndexChanged(int index);

    void on_cbExtCase_currentIndexChanged(int index);

    void IdChanged(QWidget *editor);
    void on_twImages_customContextMenuRequested(const QPoint &pos);

    void on_twFiles_customContextMenuRequested(const QPoint &pos);

    void on_twXrefs_customContextMenuRequested(const QPoint &pos);

    void CurVarChanged(QWidget *editor);
    void DocNameChanged(QWidget *editor);
    void on_cbAddFiles_toggled(bool checked);

    void on_cbXrefs_toggled(bool checked);

    void on_cbXrefNameCase_currentIndexChanged(int index);

    void on_cbSaveNewXrefs_toggled(bool checked);

    void on_cbViewMode_currentIndexChanged(int index);

    void on_cbMakeSubdir_toggled(bool checked);

    void on_cbUseX_currentIndexChanged(int index);

    void on_cbUseAutocad_currentIndexChanged(int index);

    void on_pbSettings_clicked();

    void on_twDocs_expanded(const QModelIndex &index);

    void on_twImages_expanded(const QModelIndex &index);

    void on_twXrefs_expanded(const QModelIndex &index);

    void on_twFiles_expanded(const QModelIndex &index);

    void on_cbSaveAcad_currentIndexChanged(int index);

private:
    Ui::SaveDialog *ui;
};

#endif // SAVEDIALOG_H
