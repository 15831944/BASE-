#ifndef GEOBASELOADFILES_H
#define GEOBASELOADFILES_H

#include "../VProject/qfcdialog.h"
#include "../VProject/AcadXchgDialog.h"

#include <QTableWidgetItem>
#include <QStyledItemDelegate>

class XrefTypeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit XrefTypeItemDelegate(QWidget *parent = 0);

    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class XrefCommentsItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit XrefCommentsItemDelegate(QWidget *parent = 0);

//    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class XrefTypeItem : public QTableWidgetItem
{
public:
    explicit XrefTypeItem(QString aDispStr, int aId) : QTableWidgetItem(aDispStr) { mId = aId; }

    int XrefTypeId() const { return mId; }
    void SetXrefTypeId(int aId) { mId = aId; }
protected:
    int mId;
};

namespace Ui {
class GeobaseLoadFiles;
}

class GeobaseLoadFiles : public QFCDialog, public AcadXchgDialog
{
    Q_OBJECT

public:
    explicit GeobaseLoadFiles(QWidget *parent = 0);
    virtual ~GeobaseLoadFiles();

    void SetGeobaseData(int aIdGeobase, int aIdProject, const QString &aMaker, const QString &aOrder, const QString &aSite);

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void SaveAdditionalSettings(QSettings &settings);
    virtual void LoadAdditionalSettings(QSettings &settings);

    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);
public slots:
private slots:
    void on_cbMakeFilename_stateChanged(int arg1);
    void Accept();

    void on_cbSplitter_editTextChanged(const QString &arg1);

    void on_cbSplitter_currentIndexChanged(int index);

    void on_lePrefix_textChanged(const QString &arg1);

    void on_cbFNPart1_currentIndexChanged(int index);

    void on_cbFNPart2_currentIndexChanged(int index);

    void on_cbFNPart3_currentIndexChanged(int index);

    void on_lePostfix_textChanged(const QString &arg1);

    void on_cbChanger_currentIndexChanged(int index);

    void on_pushButton_clicked();

    void on_actionDelete_from_list_triggered();

    void on_tbFiles_customContextMenuRequested(const QPoint &pos);

    void on_pbLoadToBase_clicked();

    void XrefTypeCahnged(QWidget *editor);

    void on_cbUseFN_stateChanged(int arg1);

private:
    //XrefTypeItemDelegate *XrefTypeDelegate;
    //XrefCommentsItemDelegate *XrefCommentsDelegate;
    Ui::GeobaseLoadFiles *ui;

    int IdGeobase, IdProject;
    QString Order, Site, Maker;

    bool SelectFiles();
};

#endif // GEOBASELOADFILES_H
