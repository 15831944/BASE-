#include "TreeWidgetItemLoad.h"

#include "LoadImagesDlg.h"

TreeWidgetItemLoad::TreeWidgetItemLoad(XchgFileData *aXchgFileData) :
    QTreeWidgetItem(),
    mXchgFileData(aXchgFileData),
    mImageWidth(-1), mImageHeight(-1),
    mImageLoadError(false)
{

}

void TreeWidgetItemLoad::SetImageWidth(int aWidth) {
    mImageWidth = aWidth;
}

int TreeWidgetItemLoad::ImageWidth() {
    return mImageWidth;
}

void TreeWidgetItemLoad::SetImageHeight(int aHeight) {
    mImageHeight = aHeight;
}

int TreeWidgetItemLoad::ImageHeight() {
    return mImageHeight;
}

void TreeWidgetItemLoad::SetImageLoadError(bool aImageLoadError) {
    mImageLoadError = aImageLoadError;
}

bool TreeWidgetItemLoad::ImageLoadError() {
    return mImageLoadError;
}

QImage &TreeWidgetItemLoad::ImageRef() {
    return mImage;
}

XchgFileData *TreeWidgetItemLoad::XchgFileDataRef() const {
    return mXchgFileData;
}

QList<tPairIntIntString> &TreeWidgetItemLoad::LoadedPlotsDataEqualRef() {
    return mLoadedPlotsDataEqual;
}

QList<tPairIntIntString> &TreeWidgetItemLoad::LoadedPlotsFNEqualRef() {
    return mLoadedPlotsFNEqual;
}

bool TreeWidgetItemLoad::IsPlotEqualByData() {
    int i;
    int lIdPlot = text(4).toInt();

    if (!lIdPlot) return false;

    for (i = 0; i < mLoadedPlotsDataEqual.length(); i++) {
        if (mLoadedPlotsDataEqual.at(i).first.first == lIdPlot) return true;
    }
    return false;
}

bool TreeWidgetItemLoad::IsPlotEqualByFilename() {
    int i;
    int lIdPlot = text(4).toInt();

    if (!lIdPlot) return false;

    for (i = 0; i < mLoadedPlotsFNEqual.length(); i++) {
        if (mLoadedPlotsFNEqual.at(i).first.first == lIdPlot) return true;
    }
    return false;
}

//-----------------------------------------------------------------------
TreeWidgetItemLoadDelegate::TreeWidgetItemLoadDelegate(QWidget *parent) :
    PlotListItemDelegate(parent)
{

}

QWidget *TreeWidgetItemLoadDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QWidget *lEditor = NULL;
    LoadImagesDlg *lDlg;
    if (lDlg = qobject_cast<LoadImagesDlg *>(this->parent())) {

        const QList <QTreeWidgetItem *>  &lSelectedItems = lDlg->SaveSelectedItems();

        switch (index.column()) {
        case 3:
            {
                QComboBox *lComboBox = new QComboBox(parent);

                lComboBox->addItem(tr("New"));
                lComboBox->addItem(tr("Instead of"));
                lComboBox->addItem(tr("New version of"));
                lComboBox->addItem(tr("Don't load"));

                lComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

                lEditor = lComboBox;
            }
            break;
        case 4:
            if (lSelectedItems.length() == 1) {
                TreeWidgetItemLoad *lItem = static_cast<TreeWidgetItemLoad *>(lSelectedItems.at(0));

                if (lItem->LoadedPlotsDataEqualRef().length() > 1) {
                    QComboBox *lComboBox = new QComboBox(parent);

                    for (int i = 0; i < lItem->LoadedPlotsDataEqualRef().length(); i++) {
                        lComboBox->addItem(QString::number(lItem->LoadedPlotsDataEqualRef().at(i).first.first));
                    }

                    lComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

                    lEditor = lComboBox;
                } else {
                    QLineEdit *lLineEdit = new QLineEdit(parent);

                    lLineEdit->setAlignment(Qt::AlignCenter);
                    lLineEdit->setInputMask("000000000");
                    lEditor = lLineEdit;
                }
            }
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            {
                QLineEdit *lLineEdit = new QLineEdit(parent);
                lEditor = lLineEdit;
            }
            break;
        default:
            QLineEdit *lLineEdit = new QLineEdit(parent);
            lLineEdit->setReadOnly(true);
            lEditor = lLineEdit;
            break;
        }
    }

    return lEditor;
}
