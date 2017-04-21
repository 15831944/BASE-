#ifndef QCONTRACTTREE_H
#define QCONTRACTTREE_H

#pragma warning(disable:4100)

#include <Qdate>
#include <QTreeWidget>
#include <QStyledItemDelegate>
#include <QHeaderView>

class MyBaseHeaderView : public QHeaderView
{
public:
    MyBaseHeaderView(const QHeaderView *headerOrig, Qt::Orientation orientation, QWidget* parent);
protected:
    int mColHiddenStart;
};

class QContractTreeItem : public QTreeWidgetItem
{
protected:
    int mIdProject, mIdContract, mIdPlot, mIdContractStage;
    QDate mStartDate, mEndDate;
    qlonglong mNDS, mSum, mSumFull; // mNDS - vat in %
    QString mSumStr, mSumNdsStr, mSumFullStr; // mSumNdsStr - just preformatted sum for drawing
    int mIndexingType;
    qlonglong mPayIndexed, mPayFullIndexed;
    QString mPayIndexedStr, mPayFullIndexedStr;

    qlonglong mRestThisYear, mRestFuture;
    QString mRestThisYearStr, mRestFutureStr, mRestFullStr;

    int mColNum, mColName;
public:
    explicit QContractTreeItem();
    explicit QContractTreeItem(int aIdProject, const QString &aProjName);

    void SetContractData(int aIdContract, int aIdPlot, const QString &aCustomer, const QString &aNum,
                         const QDate &aStartDate, const QDate &aEndDate, const QString &aName,
                         qlonglong aSum, qlonglong aNDS, qlonglong aSumFull, int aIndexingType,
                         qlonglong aPayIndexed, qlonglong aPayFullIndexed,
                         qlonglong aRestThisYear, qlonglong aRestFuture, qlonglong aDummy,
                         const QString &aComments);
    void SetContractStageData(int aIdContractStage, const QString &aNum, const QDate &aStartDate, const QString &aName,
                              qlonglong aSum, qlonglong aNDS, qlonglong aSumFull,
                              qlonglong aPayIndexed, qlonglong aPayFullIndexed,
                              qlonglong aRestThisYear, qlonglong aRestFuture, qlonglong aDummy,
                              const QString &aComments);
    void ReloadContractFromBase(int aIdContract);
    void ReloadContractStageFromBase(int aIdContractStage);

    QString ProjectName() const { return text(0); }
    QString Num() const { return text(mColNum); }
    QString Name() const { return text(mColName); }

    int IdProject() const { return mIdProject; }
    int IdContract() const { return mIdContract; }
    int IdPlot() const { return mIdPlot; }
    int IdContractStage() const { return mIdContractStage; }

    const QDate &StartDate() const { return mStartDate; }
    const QDate &EndDate() const { return mEndDate; }

    qlonglong Sum() const { return mSum; }
    const QString &SumStr() const { return mSumStr; }
    void SetSum(qlonglong aSum);

    const QString &SumNdsStr() const { return mSumNdsStr; }

    qlonglong SumFull() const { return mSumFull; }
    const QString &SumFullStr() const { return mSumFullStr; }
    void SetSumFull(qlonglong aSumFull);

    int IndexingType() const { return mIndexingType; }

    qlonglong PayIndexed() const { return mPayIndexed; }
    const QString &PayIndexedStr() const { return mPayIndexedStr; }
    void SetPayIndexed(qlonglong aPayIndexed);

    qlonglong PayFullIndexed() const { return mPayFullIndexed; }
    const QString &PayFullIndexedStr() const { return mPayFullIndexedStr; }
    void SetPayFullIndexed(qlonglong aPayFullIndexed);

    qlonglong RestThisYear() const { return mRestThisYear; }
    const QString &RestThisYearStr() const { return mRestThisYearStr; }
    void SetRestThisYear(qlonglong aRestThisYear);

    qlonglong RestFuture() const { return mRestFuture; }
    const QString &RestFutureStr() const { return mRestFutureStr; }
    void SetRestFuture(qlonglong aRestFuture);

    const QString &RestFullStr() const { return mRestFullStr; }
};

class QContractTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QContractTree(QWidget *parent = 0);

    void AddContract(int aIdContract);
    void AddContractStage(int aIdContractStage, QContractTreeItem *aParentItem);

    QContractTreeItem *itemFromIndex(const QModelIndex & index) const { return static_cast<QContractTreeItem *>(QTreeWidget::itemFromIndex(index)); }

protected:
    bool justStart;

    void showEvent(QShowEvent * event);
    void PopulateTreeInternal();
    void RecalcProjectAmounts(QContractTreeItem *aProjectItem);
    void PopulateForProject(int aIdProject, QContractTreeItem *aParentItem);
    void PopulateForContract(int aIdContract, QContractTreeItem *aParentItem);

signals:

public slots:
    void PopulateTree();
};

class QContractItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QContractItemDelegate(QObject * parent = 0);
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class ContractHeaderView : public MyBaseHeaderView
{
public:
    ContractHeaderView(const QHeaderView *headerOrig, Qt::Orientation orientation, QWidget* parent = 0);
protected:
    QSize sectionSizeFromContents(int logicalIndex) const;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
class QHashbonTreeItem : public QTreeWidgetItem
{
protected:
    int mIdCheck;
    int mColSumStart, mColHiddenStart;
public:
    explicit QHashbonTreeItem();
    explicit QHashbonTreeItem(int aIdCheck, int aPercent,
                              const QDate &aOrigDate, const QDate &aExpectDate, const QString &aInvoice,
                              qlonglong aOrigSumBrutto, qlonglong aOrigSumFull,
                              qlonglong aOrigSumBruttoIndexed, qlonglong aOrigSumFullIndexed,
                              const QDate &aSignDate,
                              qlonglong aSignSumBrutto, qlonglong aSignSumFull,
                              qlonglong aSignSumBruttoIndexed, qlonglong aSignSumFullIndexed,
                              const QDate &aPayDate, const QString &aPayInvoice,
                              qlonglong aPaySumBrutto, qlonglong aPaySumFull,
                              qlonglong aPaySumBruttoIndexed, qlonglong aPaySumFullIndexed,
                              const QString &aComments);

    int IdCheck() const { return mIdCheck; }
    int ColSumStart() const { return mColSumStart; }
    int ColHiddenStart() const { return mColHiddenStart; }
};

class QHashbonTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QHashbonTree(QWidget *parent = 0);

    void ClearTree();
    void PopulateTree(int aIdContract, int aIdContractStage, qlonglong aContractSum);
//    void PopulateTree();
//    void AddContract(int aIdContract);

protected:
    bool justStart;
    void showEvent(QShowEvent * event);
//    void PopulateTreeInternal();
//    void PopulateForProject(int aIdProject, QContractTreeItem *aParentItem);
//    void PopulateForContract(int aIdContract, QContractTreeItem *aParentItem);

signals:

public slots:

};

class QHashbonItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QHashbonItemDelegate(QObject * parent = 0);
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class HashbonHeaderView : public MyBaseHeaderView
{
public:
    HashbonHeaderView(const QHeaderView *headerOrig, Qt::Orientation orientation, QWidget* parent = 0);
    void SetColSumStart(int aColSumStart) { mColSumStart = aColSumStart; }
protected:
    int mColSumStart;

    QSize sectionSizeFromContents(int logicalIndex) const;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;
};

#endif // QCONTRACTTREE_H
