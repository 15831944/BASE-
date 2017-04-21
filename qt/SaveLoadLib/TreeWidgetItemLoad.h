#ifndef TREEWIDGETITEMLOAD_H
#define TREEWIDGETITEMLOAD_H

#include <QTreeWidgetItem>

#include "../Login/Login.h"

#include "../VProject/AcadXchgDialog.h"
#include "../VProject/PlotListTree.h"

class TreeWidgetItemLoad : public QTreeWidgetItem
{
public:
    TreeWidgetItemLoad(XchgFileData *aXchgFileData);

    void SetImageWidth(int aWidth);
    int ImageWidth();

    void SetImageHeight(int aHeight);
    int ImageHeight();

    void SetImageLoadError(bool aImageLoadError);
    bool ImageLoadError();

    QImage &ImageRef();
    XchgFileData *XchgFileDataRef() const;

    QList<tPairIntIntString> &LoadedPlotsDataEqualRef();
    QList<tPairIntIntString> &LoadedPlotsFNEqualRef();

    bool IsPlotEqualByData();
    bool IsPlotEqualByFilename();
private:
    int mImageWidth, mImageHeight;
    bool mImageLoadError;
    QImage mImage;
    XchgFileData *mXchgFileData;

    QList<tPairIntIntString> mLoadedPlotsDataEqual, mLoadedPlotsFNEqual;
};

class TreeWidgetItemLoadDelegate : public PlotListItemDelegate
{
public:
    explicit TreeWidgetItemLoadDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // TREEWIDGETITEMLOAD_H
