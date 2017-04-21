#ifndef TYPETREESELECT_H
#define TYPETREESELECT_H

#include <QDialog>
#include <QTreeWidget>

#include "TreeData.h"

#include "def_expimp.h"

namespace Ui {
class TypeTreeSelect;
}

class EXP_IMP TypeTreeSelect : public QDialog
{
    Q_OBJECT

protected:
    int mArea, mType;
    //TreeDataRecord * mTreeDataRecord;
public:
    explicit TypeTreeSelect(const TreeDataRecord * aSelected, QWidget *parent = 0);
    ~TypeTreeSelect();

    //void SetSelected(const TreeDataRecord * aTreeDataRecord);
    TreeDataRecord * GetSelected();
    //int Area() { return mArea; };
    //int Type() { return mType; };

private slots:

    void on_buttonBox_accepted();
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    Ui::TypeTreeSelect *ui;
};

#endif // TYPETREESELECT_H
