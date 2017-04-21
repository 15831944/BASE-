#ifndef PROJECTCARD_H
#define PROJECTCARD_H

#include "../VProject/qfcdialog.h"

#include <QStyledItemDelegate>

namespace Ui {
class ProjectCard;
}

class ProjectCard : public QFCDialog
{
    Q_OBJECT

public:
    explicit ProjectCard(QWidget *parent = 0);
    ~ProjectCard();

    void SetIdProject(int aIdProject) { mIdProject = aIdProject; }

protected:
    virtual void showEvent(QShowEvent* event);

    bool DoSave();

private slots:
    void Accept();
    void ShowInMSWord();
    void PropDataChanged(QWidget *editor);
    void PropDeptCustChanged(QWidget *editor);
private:
    Ui::ProjectCard *ui;

    bool mNewRecordProp, mNewRecordSR;
    int mIdProject;
    int creatorId, signerId, creatorPostId, signerPostId;
    QString origCreator, origSigner;
};

class QPCPropItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QPCPropItemDelegate(QWidget *parent = 0);

    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const;
    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class QPCDepCustItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QPCDepCustItemDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class QPCPersonItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QPCPersonItemDelegate(QWidget *parent = 0);

    virtual QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // PROJECTCARD_H
