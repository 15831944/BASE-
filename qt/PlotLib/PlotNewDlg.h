#ifndef PLOTNEWDLG_H
#define PLOTNEWDLG_H

#include "../VProject/AcadXchgDialog.h"
#include "../VProject/qfcdialog.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/oracle.h"
#include "../VProject/FileUtils.h"

#include "../Login/Login.h"

#include "../VProject/TreeData.h"

#include <QFileInfo>

class ProjectData;
class LoadImagesDlg;

namespace Ui {
class PlotNewDlg;
}

class PlotNewDlg : public QFCDialog, public AcadXchgDialog
{
    Q_OBJECT
protected:
    int mIdPlot;
    quint64 mIdDwg;
    ProjectData * mProject;
    const TreeDataRecord * mTreeData;
    const FileType * mFileType;
    PlotData * mPlotDataFrom;
    PlotHistoryData * mPlotHistoryDataFrom;
    bool mSheetSetted;
    QString mPrevCode;
    bool mCodeSetted;

    LoadImagesDlg *mAddFilesDlg;

    // file data
    qint64 mOrigFileSize; // sum size of directory calced after select; for single file - size of file, which also refrereshed on pressed OK

    QList<tPairIntIntString> mExistingIds;
    QMenu *mExistsListMenu;

    void TreeDataChanged();
    void OtherPlotDataChanged();

    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);
public:
    explicit PlotNewDlg(ProjectData * aProject, const TreeDataRecord * aTreeData, const QString & aComplect,
                        PlotData * aPlotDataFrom, PlotHistoryData * aPlotHistoryDataFrom, QWidget *parent = 0);
    ~PlotNewDlg();

    int IdPlot() const { return mIdPlot; }

protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    void on_tbTreeSel_clicked();
    void Accept();
    void RegenCodeOld();
    void RegenCode();

    void on_tbPlotSel_clicked();

    void on_tbTreeSel_2_clicked();

    void on_cbComplect_editTextChanged(const QString &arg1);

    void on_pbAlreadyInBase_clicked();

    void on_cbCreateFrom_currentIndexChanged(int index);

    void on_leIdPlotFrom_editingFinished();

    void on_cbStage_currentIndexChanged(int index);

    void on_leVersionExt_textEdited(const QString &arg1);

    void on_leVersionExt_editingFinished();

    void on_cbSheet_editTextChanged(const QString &arg1);

    void on_cbCode_customContextMenuRequested(const QPoint &pos);

    void on_cbCode_editTextChanged(const QString &arg1);

    //void on_cbCode_currentTextChanged(const QString &arg1);

    void on_cbStage_editTextChanged(const QString &arg1);

    void on_pbAddFiles_clicked();

private:
    Ui::PlotNewDlg *ui;
};

#endif // PLOTNEWDLG_H
