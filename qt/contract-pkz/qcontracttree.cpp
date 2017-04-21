//#include "common.h"
#include "qcontracttree.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

MyBaseHeaderView::MyBaseHeaderView(const QHeaderView *headerOrig, Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent),
      mColHiddenStart(32767)
{
    setModel(headerOrig->model());
    setVisible(headerOrig->isVisible());
    setDefaultSectionSize(headerOrig->defaultSectionSize());
    setStretchLastSection(headerOrig->stretchLastSection());
    setLayoutDirection(headerOrig->layoutDirection());
    setBaseSize(headerOrig->baseSize());
    setCascadingSectionResizes(headerOrig->cascadingSectionResizes());
    setFrameRect(headerOrig->frameRect());
    setOffset(headerOrig->offset());
    setStyle(headerOrig->style());
    setMinimumSectionSize(headerOrig->minimumSectionSize());
}

QContractTreeItem::QContractTreeItem() :
    QTreeWidgetItem(), mIdProject(0), mIdContract(0), mIdPlot(0), mIdContractStage(0),
    mNDS(0), mSum(0), mSumFull(0),
    mIndexingType(0),
    mPayIndexed(0), mPayFullIndexed(0),
    mRestThisYear(0), mRestFuture(0),
    mColNum(2), mColName(5)
{
}

QContractTreeItem::QContractTreeItem(int aIdProject, const QString &aProjName) :
    QTreeWidgetItem(),
    mIdProject(aIdProject), mIdContract(0), mIdPlot(0), mIdContractStage(0),
    mNDS(0), mSum(0), mSumFull(0),
    mIndexingType(0),
    mPayIndexed(0), mPayFullIndexed(0),
    mRestThisYear(0), mRestFuture(0),
    mColNum(2), mColName(5)
{
    setText(0, aProjName);
    if (gSettings->Contract.UseProjectColor)
        setBackground(0, gSettings->Contract.ProjectColor);

    setFirstColumnSpanned(true);
    if (gSettings->Contract.ExpandOnStart < 2)
        setExpanded(true);
    if (!gSettings->Contract.MultiSelect) {
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setCheckState(0, Qt::Unchecked);
    }
}

void QContractTreeItem::SetContractData(int aIdContract, int aIdPlot, const QString &aCustomer, const QString &aNum,
                                        const QDate &aStartDate, const QDate &aEndDate, const QString &aName,
                                        qlonglong aSum, qlonglong aNDS, qlonglong aSumFull, int aIndexingType,
                                        qlonglong aPayIndexed, qlonglong aPayFullIndexed,
                                        qlonglong aRestThisYear, qlonglong aRestFuture, qlonglong aDummy,
                                        const QString &aComments)
{
    mIdContract = aIdContract;
    mIdPlot = aIdPlot;
    mStartDate = aStartDate;
    mEndDate = aEndDate;
    mSum = aSum;
    mNDS = aNDS;
    mSumFull = aSumFull;
    mIndexingType = aIndexingType;
    mPayIndexed = aPayIndexed;
    mPayFullIndexed = aPayFullIndexed;
    mRestThisYear = aRestThisYear;
    mRestFuture = aRestFuture;

    QFont f;

    int col = 1;

    setText(col, aCustomer);
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    setText(col, aNum);
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    mColNum = col;
    col++;

    setText(col, mStartDate.toString("dd.MM.yyyy"));
    setTextAlignment(col, Qt::AlignHCenter | Qt::AlignTop);
    col++;

    setText(col, mEndDate.toString("dd.MM.yyyy"));
    setTextAlignment(col, Qt::AlignHCenter | Qt::AlignTop);
    col++;

    setText(col, aName);
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    mColName = col;
    col++;

    setText(col, gSettings->FormatSumForList(mSum));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;
    setText(col, gSettings->FormatSumForList(mSumFull - mSum));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;
    setText(col, gSettings->FormatSumForList(mSumFull));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    setText(col, gSettings->FormatSumForList(mPayIndexed));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
//    f = font(col);
//    f.setBold(true);
//    setFont(col, f);
    col++;

    setText(col, gSettings->FormatSumForList(mPayFullIndexed));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
//    f = font(col);
//    f.setBold(true);
//    setFont(col, f);
    col++;

    setText(col, gSettings->FormatSumForList(mRestThisYear));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    f = font(col);
    f.setBold(true);
    setFont(col, f);
    col++;

    setText(col, gSettings->FormatSumForList(mRestFuture));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    f = font(col);
    f.setBold(true);
    setFont(col, f);
    col++;

    setText(col, gSettings->FormatSumForList(mRestThisYear + mRestFuture));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    f = font(col);
    f.setBold(true);
    setFont(col, f);
    col++;

    setText(col, aComments);
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    if (gSettings->Contract.UseContractColor) {
        for (int i = 1; i < columnCount(); i++) {
            setBackground(i, gSettings->Contract.ContractColor);
        }
    }
}

void QContractTreeItem::SetContractStageData(int aIdContractStage, const QString &aNum, const QDate &aStartDate, const QString &aName,
                                             qlonglong aSum, qlonglong aNDS, qlonglong aSumFull,
                                             qlonglong aPayIndexed, qlonglong aPayFullIndexed,
                                             qlonglong aRestThisYear, qlonglong aRestFuture, qlonglong aDummy,
                                             const QString &aComments)
{
    mIdContractStage = aIdContractStage;
    mStartDate = aStartDate;
    mSum = aSum;
    mNDS = aNDS;
    mSumFull = aSumFull;
    mPayIndexed = aPayIndexed;
    mPayFullIndexed = aPayFullIndexed;
    mRestThisYear = aRestThisYear;
    mRestFuture = aRestFuture;

    int col = 2;

    setText(col, aNum);
    setTextAlignment(col, Qt::AlignCenter | Qt::AlignTop);
    mColNum = col;
    col++;

    setText(col, mStartDate.toString("dd.MM.yyyy"));
    setTextAlignment(col, Qt::AlignHCenter | Qt::AlignTop);
    col++;

    // no end date for stage
    col++;

    setText(col, aName);
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    mColName = col;
    col++;

    setText(col, gSettings->FormatSumForList(mSum));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;
    setText(col, gSettings->FormatSumForList(mSumFull - mSum));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;
    setText(col, gSettings->FormatSumForList(mSumFull));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    setText(col, gSettings->FormatSumForList(mPayIndexed));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
//    f = font(col);
//    f.setBold(true);
//    setFont(col, f);
    col++;

    setText(col, gSettings->FormatSumForList(mPayFullIndexed));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
//    f = font(col);
//    f.setBold(true);
//    setFont(col, f);
    col++;

    setText(col, gSettings->FormatSumForList(mRestThisYear));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    setText(col, gSettings->FormatSumForList(mRestFuture));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    setText(col, gSettings->FormatSumForList(mRestThisYear + mRestFuture));
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    setText(col, aComments);
    setTextAlignment(col, Qt::AlignLeft | Qt::AlignTop);
    col++;

    if (gSettings->Contract.UseStageColor)
        for (int i = 1; i < columnCount(); i++)
            setBackground(i, gSettings->Contract.StageColor);
}

void QContractTreeItem::ReloadContractFromBase(int aIdContract) {
    if (aIdContract && !mIdContract) mIdContract = aIdContract;

    QSqlQuery query(
                "SELECT A.ID, A.CUSTOMER, A.NUM, A.START_DATE, A.END_DATE, A.NAME, c.id_plot,"
                " B.SUM_BRUTTO, B.NDS_PERCENT, B.SUM_FULL, A.INDEXING_TYPE, A.COMMENTS,"
                " ((SELECT NVL(SUM(NVL(NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                "    WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "       WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) PAY_INDEXED,"
                " ((SELECT NVL(SUM(NVL(NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL), 0)), 0) FROM V_PKZ_HASHBON"
                "    WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "       WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) PAY_FULL_INDEXED,"

                " (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                "   WHERE TO_CHAR(EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                "     AND ID_CONTRACT = A.ID AND PAY_SUM_BRUTTO IS NULL)"
                "  + (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "      WHERE TO_CHAR(B.EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                "        AND C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID AND B.PAY_SUM_BRUTTO IS NULL) REST_THIS_YEAR,"

                " B.SUM_BRUTTO -"
                "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT = A.ID)"
                "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST_FULL"

                " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B, (select * from v_plot2pkz_contract where nvl(deleted, 0) = 0) c"
                " WHERE A.ID = " + QString::number(mIdContract) +
                " AND B.ID_CONTRACT = A.ID"
                " and c.id_pkz_contract(+) = a.id"
                " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                " ORDER BY A.NUM", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("חוזים", query);
        return;
    }

    if (query.next()) {
        SetContractData(query.value("ID").toInt(), query.value("ID_PLOT").toInt(),
                        query.value("CUSTOMER").toString(), query.value("NUM").toString(),
                        query.value("START_DATE").toDate(), query.value("END_DATE").toDate(),
                        query.value("NAME").toString(), query.value("SUM_BRUTTO").toLongLong(), query.value("NDS_PERCENT").toLongLong(),
                        query.value("SUM_FULL").toLongLong(), query.value("INDEXING_TYPE").toInt(),
                        query.value("PAY_INDEXED").toLongLong(), query.value("PAY_FULL_INDEXED").toLongLong(),
                        query.value("REST_THIS_YEAR").toLongLong(), query.value("REST_FULL").toLongLong() - query.value("REST_THIS_YEAR").toLongLong(), 0,
                        query.value("COMMENTS").toString());
    } else {
        QMessageBox::critical(NULL, "חוזים",
                              QString("חוזה id = ") + QString::number(mIdContract) + " לא קיימ!");
    }
}

void QContractTreeItem::ReloadContractStageFromBase(int aIdContractStage) {
    if (aIdContractStage && !mIdContractStage) mIdContractStage = aIdContractStage;

    QSqlQuery query(
                "SELECT ID, ORDER_NUM, START_DATE, NAME,"
                " SUM_BRUTTO, NDS_PERCENT, SUM_FULL, COMMENTS,"
                " (SELECT NVL(SUM(NVL(NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                "    WHERE ID_CONTRACT_STAGE = A.ID) PAY_INDEXED,"
                " (SELECT NVL(SUM(NVL(NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL), 0)), 0) FROM V_PKZ_HASHBON"
                "    WHERE ID_CONTRACT_STAGE = A.ID) PAY_FULL_INDEXED,"

                " (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                "   WHERE TO_CHAR(EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                "     AND ID_CONTRACT_STAGE = A.ID AND PAY_SUM_BRUTTO IS NULL) REST_THIS_YEAR,"

                " SUM_BRUTTO -"
                "    (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT_STAGE = A.ID) REST_FULL"

                " FROM V_PKZ_CONTRACT_STAGE A"
                " WHERE A.ID = " + QString::number(mIdContractStage), db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("חוזים", query);
        return;
    }

    if (query.next()) {
        SetContractStageData(query.value("ID").toInt(),
                             query.value("ORDER_NUM").toString(), query.value("START_DATE").toDate(), query.value("NAME").toString(),
                             query.value("SUM_BRUTTO").toLongLong(), query.value("NDS_PERCENT").toLongLong(), query.value("SUM_FULL").toLongLong(),
                             query.value("PAY_INDEXED").toLongLong(), query.value("PAY_FULL_INDEXED").toLongLong(),
                             query.value("REST_THIS_YEAR").toLongLong(), query.value("REST_FULL").toLongLong() - query.value("REST_THIS_YEAR").toLongLong(), 0,
                             query.value("COMMENTS").toString());
    } else {
        QMessageBox::critical(NULL, "חוזים",
                              QString("שלב id = ") + QString::number(mIdContract) + " לא קיימ!");
    }
}

void QContractTreeItem::SetSum(qlonglong aSum)
{
    mSum = aSum;
    mSumStr = gSettings->FormatSumForList(mSum);
    mSumNdsStr = gSettings->FormatSumForList(mSumFull - mSum);
}

void QContractTreeItem::SetSumFull(qlonglong aSumFull) {
    mSumFull = aSumFull;
    mSumFullStr = gSettings->FormatSumForList(mSumFull);
    mSumNdsStr = gSettings->FormatSumForList(mSumFull - mSum);
}

void QContractTreeItem::SetPayIndexed(qlonglong aPayIndexed) {
    mPayIndexed = aPayIndexed;
    mPayIndexedStr = gSettings->FormatSumForList(mPayIndexed);
}

void QContractTreeItem::SetPayFullIndexed(qlonglong aPayFullIndexed) {
    mPayFullIndexed = aPayFullIndexed;
    mPayFullIndexedStr = gSettings->FormatSumForList(mPayFullIndexed);
}

void QContractTreeItem::SetRestThisYear(qlonglong aRestThisYear) {
    mRestThisYear = aRestThisYear;
    mRestThisYearStr = gSettings->FormatSumForList(mRestThisYear);
    mRestFullStr = gSettings->FormatSumForList(mRestThisYear + mRestFuture);
}

void QContractTreeItem::SetRestFuture(qlonglong aRestFuture)
{
    mRestFuture = aRestFuture;
    mRestFutureStr = gSettings->FormatSumForList(mRestFuture);
    mRestFullStr = gSettings->FormatSumForList(mRestThisYear + mRestFuture);
}

QContractTree::QContractTree(QWidget *parent) :
    QTreeWidget(parent), justStart(true)
{
    QContractItemDelegate *ItemDelegate;
    ItemDelegate = new QContractItemDelegate(this);
    setItemDelegate(ItemDelegate);
}

void QContractTree::showEvent(QShowEvent * event) {
    //for(int i = 0; i < header()->count(); i++) resizeColumnToContents(i);
    QTreeWidget::showEvent(event);

    if (justStart) {
        justStart = false;
        ContractHeaderView *hv = new ContractHeaderView(header(), Qt::Horizontal, this);
        hv->model()->setHeaderData(3, Qt::Horizontal, "תאריך\nהתחלת");
        hv->model()->setHeaderData(4, Qt::Horizontal, "תאריך\nסיום");

        /*hv->model()->setHeaderData(hv->model()->columnCount() - 3, Qt::Horizontal,
                                   hv->model()->headerData(hv->model()->columnCount() - 3, Qt::Horizontal).toString()
                                   + "\n"
                                   + QString::number(QDateTime::currentDateTime().date().year() + 1));*/
        this->setHeader(hv);
        PopulateTree();
    }
}

void QContractTree::PopulateTree() {
    PopulateTreeInternal();
    currentItemChanged(NULL, NULL);
}

void QContractTree::AddContract(int aIdContract) {
    QSqlQuery query(
                "SELECT B.ID ID_PROJECT, PP.GETPROJECTSHORTNAME(A.ID_PROJECT) PROJNAME"
                " FROM V_PKZ_CONTRACT A, V_PROJECT B"
                " WHERE A.ID_PROJECT = B.ID"
                " AND A.ID = " + QString::number(aIdContract), db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", query);
        return;
    };

    if (query.next()) {
        QContractTreeItem *parentItem = NULL;
        int i;
        for (i = 0; i< topLevelItemCount(); i++)
            if (((QContractTreeItem *) topLevelItem(i))->IdProject() == query.value("ID_PROJECT").toInt()) {
                parentItem = (QContractTreeItem *) topLevelItem(i);
                break;
            }

        if (!parentItem) {
            parentItem = new QContractTreeItem(query.value("ID_PROJECT").toInt(), query.value("PROJNAME").toString());
            addTopLevelItem(parentItem);
        }

        QContractTreeItem *item = new QContractTreeItem();

        item->ReloadContractFromBase(aIdContract);
        parentItem->addChild(item);

        parentItem->setFirstColumnSpanned(true);
        parentItem->setExpanded(true);

        setCurrentItem(item);
        setFocus();
    } else {
        QMessageBox::critical(this, "חוזים",
                              QString("חוזה id = ") + QString::number(aIdContract) + " לא קיימ!");
    }
}

void QContractTree::AddContractStage(int aIdContractStage, QContractTreeItem *aParentItem) {
    aParentItem->ReloadContractFromBase(0);

    QContractTreeItem *item = new QContractTreeItem();

    item->ReloadContractStageFromBase(aIdContractStage);
    aParentItem->addChild(item);
    aParentItem->setExpanded(true);


    setCurrentItem(item);
    setFocus();
}

void QContractTree::PopulateTreeInternal() {
    model()->removeRows(0, model()->rowCount());

    QSqlQuery query(
                "SELECT DISTINCT A.ID_PROJECT ID_PROJECT, PP.GETPROJECTSHORTNAME(A.ID_PROJECT) PROJNAME"
                " FROM V_PKZ_CONTRACT A"
                " ORDER BY PP.GETPROJECTSHORTNAME(A.ID_PROJECT)", db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", query);
        return;
    };

    while (query.next()) {
        QContractTreeItem *item = new QContractTreeItem(query.value("ID_PROJECT").toInt(), query.value("PROJNAME").toString());
        addTopLevelItem(item);
        item->setFirstColumnSpanned(true);

        if (gSettings->Contract.ExpandOnStart < 1)
            item->setExpanded(true);
        PopulateForProject(query.value("ID_PROJECT").toInt(), item);

        RecalcProjectAmounts(item);
    }
}

void QContractTree::RecalcProjectAmounts(QContractTreeItem *aProjectItem) {
    int i;
    qlonglong aSum = 0, aSumFull = 0, aPayIndexed = 0, aPayFullIndexed = 0, aRestThisYear = 0, aRestFuture = 0;

    for (i = 0; i < aProjectItem->childCount(); i++) {
        QContractTreeItem *lItem = static_cast<QContractTreeItem *> (aProjectItem->child(i));
        aSum += lItem->Sum();
        aSumFull += lItem->SumFull();
        aPayIndexed += lItem->PayIndexed();
        aPayFullIndexed += lItem->PayFullIndexed();
        aRestThisYear += lItem->RestThisYear();
        aRestFuture += lItem->RestFuture();
    }

    aProjectItem->SetSum(aSum);
    aProjectItem->SetSumFull(aSumFull);
    aProjectItem->SetPayIndexed(aPayIndexed);
    aProjectItem->SetPayFullIndexed(aPayFullIndexed);
    aProjectItem->SetRestThisYear(aRestThisYear);
    aProjectItem->SetRestFuture(aRestFuture);
}

void QContractTree::PopulateForProject(int aIdProject, QContractTreeItem *aParentItem) {
    QSqlQuery query(db);

    query.prepare("SELECT A.ID, A.CUSTOMER, A.NUM, A.START_DATE, A.END_DATE, A.NAME, c.id_plot,"
                  " B.SUM_BRUTTO, B.NDS_PERCENT, B.SUM_FULL, A.INDEXING_TYPE, A.COMMENTS,"
                  " ((SELECT NVL(SUM(NVL(NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                  "    WHERE ID_CONTRACT = A.ID)"
                  "  + (SELECT NVL(SUM(NVL(NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                  "      WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) PAY_INDEXED,"
                  " ((SELECT NVL(SUM(NVL(NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL), 0)), 0) FROM V_PKZ_HASHBON"
                  "    WHERE ID_CONTRACT = A.ID)"
                  "  + (SELECT NVL(SUM(NVL(NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                  "      WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) PAY_FULL_INDEXED,"

                  " (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                  "   WHERE TO_CHAR(EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                  "     AND ID_CONTRACT = A.ID AND PAY_SUM_BRUTTO IS NULL)"
                  "  + (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                  "      WHERE TO_CHAR(B.EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                  "        AND C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID AND B.PAY_SUM_BRUTTO IS NULL) REST_THIS_YEAR,"

                  " B.SUM_BRUTTO -"
                  "    ((SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                  "        WHERE ID_CONTRACT = A.ID)"
                  "   + (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON B, V_PKZ_CONTRACT_STAGE C"
                  "        WHERE C.ID_PKZ_CONTRACT = A.ID AND B.ID_CONTRACT_STAGE = C.ID)) REST_FULL"

                  " FROM V_PKZ_CONTRACT A, V_PKZ_CONTRACT_SUM B, (select * from v_plot2pkz_contract where nvl(deleted, 0) = 0) c"
                  " WHERE A.ID_PROJECT = ?"
                  " AND B.ID_CONTRACT = A.ID"
                  " and c.id_pkz_contract(+) = a.id"
                  " AND B.ORDER_NUM = (SELECT MAX(ORDER_NUM) FROM V_PKZ_CONTRACT_SUM WHERE ID_CONTRACT = A.ID)"
                  " ORDER BY A.NUM");
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", query);
        return;
    }

    query.addBindValue(aIdProject);

    if (!query.exec()) {
        gLogger->ShowSqlError(this, "חוזים", query);
        return;
    }

    while (query.next()) {
        QContractTreeItem *item = new QContractTreeItem();

        item->SetContractData(query.value("ID").toInt(), query.value("ID_PLOT").toInt(),
                              query.value("CUSTOMER").toString(), query.value("NUM").toString(),
                              query.value("START_DATE").toDate(), query.value("END_DATE").toDate(),
                              query.value("NAME").toString(), query.value("SUM_BRUTTO").toLongLong(), query.value("NDS_PERCENT").toLongLong(),
                              query.value("SUM_FULL").toLongLong(), query.value("INDEXING_TYPE").toInt(),
                              query.value("PAY_INDEXED").toLongLong(), query.value("PAY_FULL_INDEXED").toLongLong(),
                              query.value("REST_THIS_YEAR").toLongLong(), query.value("REST_FULL").toLongLong() - query.value("REST_THIS_YEAR").toLongLong(), 0,
                              query.value("COMMENTS").toString());

        aParentItem->addChild(item);

        PopulateForContract(query.value("ID").toInt(), item);
        if (gSettings->Contract.ExpandOnStart < 2)
            item->setExpanded(true);
    }
}

void QContractTree::PopulateForContract(int aIdContract, QContractTreeItem *aParentItem) {
    QSqlQuery query(
                "SELECT ID, ORDER_NUM, START_DATE, NAME,"
                " SUM_BRUTTO, NDS_PERCENT, SUM_FULL, COMMENTS,"
                " (SELECT NVL(SUM(NVL(NVL(PAY_SUM_BRUTTO_INDEXED, PAY_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                "    WHERE ID_CONTRACT_STAGE = A.ID) PAY_INDEXED,"
                " (SELECT NVL(SUM(NVL(NVL(PAY_SUM_FULL_INDEXED, PAY_SUM_FULL), 0)), 0) FROM V_PKZ_HASHBON"
                "    WHERE ID_CONTRACT_STAGE = A.ID) PAY_FULL_INDEXED,"

                " (SELECT NVL(SUM(NVL(NVL(SIGN_SUM_BRUTTO, ORIG_SUM_BRUTTO), 0)), 0) FROM V_PKZ_HASHBON"
                "   WHERE TO_CHAR(EXPECT_DATE, 'YYYY') = TO_CHAR(SYSDATE, 'YYYY')"
                "     AND ID_CONTRACT_STAGE = A.ID AND PAY_SUM_BRUTTO IS NULL) REST_THIS_YEAR,"

                " SUM_BRUTTO -"
                "    (SELECT NVL(SUM(NVL(PAY_SUM_BRUTTO, 0)), 0) FROM V_PKZ_HASHBON"
                "        WHERE ID_CONTRACT_STAGE = A.ID) REST_FULL"

                " FROM V_PKZ_CONTRACT_STAGE A"
                " WHERE A.ID_PKZ_CONTRACT = " + QString::number(aIdContract) +
                " ORDER BY ORDER_NUM", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חוזים", query);
        return;
    }

    while (query.next()) {
        QContractTreeItem *item = new QContractTreeItem();
        item->SetContractStageData(query.value("ID").toInt(),
                                   query.value("ORDER_NUM").toString(), query.value("START_DATE").toDate(), query.value("NAME").toString(),
                                   query.value("SUM_BRUTTO").toLongLong(), query.value("NDS_PERCENT").toLongLong(), query.value("SUM_FULL").toLongLong(),
                                   query.value("PAY_INDEXED").toLongLong(), query.value("PAY_FULL_INDEXED").toLongLong(),
                                   query.value("REST_THIS_YEAR").toLongLong(), query.value("REST_FULL").toLongLong() - query.value("REST_THIS_YEAR").toLongLong(), 0,
                                   query.value("COMMENTS").toString());

        aParentItem->addChild(item);
    }
}

QContractItemDelegate::QContractItemDelegate(QObject * parent) :
    QStyledItemDelegate(parent)
{
}

void QContractItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QRect r = option.rect;
    int curColumn = index.column();

    if (true || curColumn < 5 || curColumn > 7) {
        QStyleOptionViewItem o = option;

        if (index.parent() == QModelIndex()) {
            painter->save();

            QFont f = painter->font();
            f.setBold(true);
//            if (f.pixelSize() != -1)
//                f.setPixelSize(f.pixelSize() + 1);
//            else if (f.pointSize() != -1)
//                f.setPointSize(f.pointSize() + 1);
            painter->setFont(f);
            o.font = f;
        }

        if (o.state & QStyle::State_Selected
                && curColumn < 1
                && index.parent() != QModelIndex())
            o.state &= !QStyle::State_Selected;
        QStyledItemDelegate::paint(painter, o, index);

        if (index.parent() == QModelIndex()) {
            // for project
            QStyleOptionViewItem opt = option;
            int i, cX = 0;

            // for text drawing
            QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled
                    ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
                cg = QPalette::Inactive;

            if (opt.state & QStyle::State_Selected) {
                painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
            } else {
                painter->setPen(opt.palette.color(cg, QPalette::Text));
            }
            QFont f = painter->font();
            f.setBold(true);
//            if (f.pixelSize() != -1)
//                f.setPixelSize(f.pixelSize() + 1);
//            else if (f.pointSize() != -1)
//                f.setPointSize(f.pointSize() + 1);
            painter->setFont(f);

            // cacling widths and drawing
            cX = - static_cast<const QContractTree *const>(option.widget)->indentation();
            for (i = 0; i < 6; i++) {
                cX += static_cast<const QContractTree *const>(option.widget)->header()->sectionSize(i);
            }
            for (int i = 6; i < index.model()->columnCount(index.parent()); i++) {
                // text
                if (i >= 6 && i <= 13) {
                    QString outStr;
                    switch (i) {
                    case 6:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->SumStr();
                        break;
                    case 7:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->SumNdsStr();
                        break;
                    case 8:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->SumFullStr();
                        break;
                    case 9:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->PayIndexedStr();
                        break;
                    case 10:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->PayFullIndexedStr();
                        break;
                    case 11:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->RestThisYearStr();
                        break;
                    case 12:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->RestFutureStr();
                        break;
                    case 13:
                        outStr = static_cast<const QContractTree *const>(option.widget)->itemFromIndex(index)->RestFullStr();
                        break;
                    }
                    painter->drawText(QRectF(r.right() - cX - static_cast<const QContractTree *const>(option.widget)->header()->sectionSize(i) + 2, r.top(),
                                             static_cast<const QContractTree *const>(option.widget)->header()->sectionSize(i) - 4, r.height()),
                                      Qt::AlignRight | Qt::AlignTop, outStr);
                }

                // vertical line
                painter->drawLine(r.right() - cX, r.top(), r.right() - cX, r.bottom());
                cX += static_cast<const QContractTree *const>(option.widget)->header()->sectionSize(i);
            }
            painter->restore();
        }
    } else {
        // 5, 6, 7 columns with money values
        // I draw it myself because in right-to-left order it is drawed incorrect (ie, "1-" for -1)
        painter->save();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        if (option.state & QStyle::State_Selected) {
            //QPen lPen(painter->pen());
            //lPen.setColor(option.palette.highlightedText().color());
            //painter->setPen(lPen);
        }
        // part 0 - background
        opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        // some parts skipped

        // part 1 - text (it is broken by default
        //painter->drawText(QRectF(r.left() + 2, r.top(), r.width() - 5, r.height()), Qt::AlignRight | Qt::AlignTop, index.data().toString());
        if (!opt.text.isEmpty()) {
            //QRect textRect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);

            QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled
                    ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
                cg = QPalette::Inactive;

            if (opt.state & QStyle::State_Selected) {
                painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
            } else {
                painter->setPen(opt.palette.color(cg, QPalette::Text));
            }
//            if (opt.state & QStyle::State_Editing) {
//                painter->setPen(opt.palette.color(cg, QPalette::Text));
//                painter->drawRect(textRect.adjusted(0, 0, -1, -1));
//            }
            painter->drawText(QRectF(r.left() + 2, r.top(), r.width() - 5, r.height()), Qt::AlignRight | Qt::AlignTop, opt.text);
            //d->viewItemDrawText(p, vopt, textRect);
        }






        // focus rect (can't understand, in some standard styles it don't appear in standard elements, but appear here
        // draw the focus rect
        if (opt.state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(opt);
            o.rect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt, opt.widget);
            o.state |= QStyle::State_KeyboardFocusChange;
            o.state |= QStyle::State_Item;
            QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                    ? QPalette::Normal : QPalette::Disabled;
            o.backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                                    ? QPalette::Highlight : QPalette::Window);
            opt.widget->style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, opt.widget);
        }
        // old version
        //        if (option.state & QStyle::State_HasFocus) {
//            painter->setBackgroundMode(Qt::TransparentMode);
//            QColor bg_col = option.palette.highlight().color();
//            if (!bg_col.isValid()) bg_col = painter->background().color();
//            // Create an "XOR" color.
//            QColor patternCol((bg_col.red() ^ 0xff) & 0xff,
//                              (bg_col.green() ^ 0xff) & 0xff,
//                              (bg_col.blue() ^ 0xff) & 0xff);
//            painter->setBrush(QBrush(patternCol, Qt::Dense4Pattern));
//            painter->setBrushOrigin(r.topLeft());
//            painter->setPen(Qt::NoPen);

//            painter->drawRect(r.left(), r.top(), r.width(), 1);    // Top
//            painter->drawRect(r.left(), r.bottom(), r.width(), 1); // Bottom
//            painter->drawRect(r.left(), r.top(), 1, r.height());   // Left
//            painter->drawRect(r.right(), r.top(), 1, r.height());  // Right
//        }
        painter->restore();
    }

    if (curColumn > 0) {
        painter->save();
        if ((option.state & QStyle::State_Selected) && curColumn > 1) {
            QPen lPen(painter->pen());
            lPen.setColor(option.palette.highlightedText().color());
            painter->setPen(lPen);
        }
        painter->drawLine(r.right(), r.top(), r.right(), r.bottom());
        painter->restore();
    }

    if ((index.row() || index.parent() != QModelIndex())
            && (index.parent() == QModelIndex() || curColumn > 0)) {
        painter->drawLine(r.left(), r.top(), r.right(), r.top());
//        painter->save();
//        if (!index.model()->data(index.model()->index(index.row(), 0, index.parent())).isNull()) {
//            QPen lPen(painter->pen());
//            lPen.setWidth(2);
//            painter->setPen(lPen);
//            painter->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
//        } else {
//            painter->drawLine(r.left(), r.top(), r.right(), r.top());
//        }
//        painter->restore();
    }
}

//case CE_ItemViewItem:
//    if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
//        p->save();
//        p->setClipRect(opt->rect);

//        QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, vopt, widget);
//        QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, vopt, widget);
//        QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, vopt, widget);

//        // draw the background
//        proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, widget);

//        // draw the check mark
//        if (vopt->features & QStyleOptionViewItem::HasCheckIndicator) {
//            QStyleOptionViewItem option(*vopt);
//            option.rect = checkRect;
//            option.state = option.state & ~QStyle::State_HasFocus;

//            switch (vopt->checkState) {
//            case Qt::Unchecked:
//                option.state |= QStyle::State_Off;
//                break;
//            case Qt::PartiallyChecked:
//                option.state |= QStyle::State_NoChange;
//                break;
//            case Qt::Checked:
//                option.state |= QStyle::State_On;
//                break;
//            }
//            proxy()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, p, widget);
//        }

//        // draw the icon
//        QIcon::Mode mode = QIcon::Normal;
//        if (!(vopt->state & QStyle::State_Enabled))
//            mode = QIcon::Disabled;
//        else if (vopt->state & QStyle::State_Selected)
//            mode = QIcon::Selected;
//        QIcon::State state = vopt->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
//        vopt->icon.paint(p, iconRect, vopt->decorationAlignment, mode, state);

//        // draw the text
//        if (!vopt->text.isEmpty()) {
//            QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
//                                  ? QPalette::Normal : QPalette::Disabled;
//            if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
//                cg = QPalette::Inactive;

//            if (vopt->state & QStyle::State_Selected) {
//                p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
//            } else {
//                p->setPen(vopt->palette.color(cg, QPalette::Text));
//            }
//            if (vopt->state & QStyle::State_Editing) {
//                p->setPen(vopt->palette.color(cg, QPalette::Text));
//                p->drawRect(textRect.adjusted(0, 0, -1, -1));
//            }

//            d->viewItemDrawText(p, vopt, textRect);
//        }

//        // draw the focus rect
//         if (vopt->state & QStyle::State_HasFocus) {
//            QStyleOptionFocusRect o;
//            o.QStyleOption::operator=(*vopt);
//            o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vopt, widget);
//            o.state |= QStyle::State_KeyboardFocusChange;
//            o.state |= QStyle::State_Item;
//            QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
//                          ? QPalette::Normal : QPalette::Disabled;
//            o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
//                                         ? QPalette::Highlight : QPalette::Window);
//            proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, widget);
//        }

//         p->restore();
//    }
//    break;




ContractHeaderView::ContractHeaderView(const QHeaderView *headerOrig, Qt::Orientation orientation, QWidget* parent)
    : MyBaseHeaderView(headerOrig, orientation, parent)
{
    mColHiddenStart = 15;

    for (int i = mColHiddenStart; i < count(); i++)
        setSectionHidden(i, true);
}

QSize ContractHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize s = MyBaseHeaderView::sectionSizeFromContents(logicalIndex);
    s.setHeight((2 + (s.height() / 2)) * 2); // round for multiple of 3
    return s;
}

void ContractHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    QRect r;
    int i;

//    if (rect.isValid()) {
//        if (!logicalIndex && r.left())
//            QMessageBox::critical(NULL, "חשבון", QString::number(r.left()));
//    }

    r = rect;

    if (logicalIndex >= 9 && logicalIndex <= 10
            || logicalIndex >= 11 && logicalIndex <= 13) {
        // only 1/2 of bottom - not indexed amouonts and debt
        r.setTop(r.top() + r.height() / 2);
    }
    MyBaseHeaderView::paintSection(painter, r, logicalIndex);

    if (logicalIndex >= 9 && logicalIndex <= 10) {
        // "payed" header
        r = rect;
        // only 1/2 of top
        r.setBottom(r.bottom() - r.height() / 2);

        for (i = 9; i <= 10; i++) {
            if (i == logicalIndex) continue;
            if (i > logicalIndex)
                r.setLeft(r.left() - sectionSize(i));
            else
                r.setRight(r.right() + sectionSize(i));
        }
        MyBaseHeaderView::paintSection(painter, r, mColHiddenStart);
    } else if (logicalIndex >= 11 && logicalIndex <= 13) {
        // debt header
        r = rect;
        // only 1/2 of top
        r.setBottom(r.bottom() - r.height() / 2);

        for (i = 11; i <= 13; i++) {
            if (i == logicalIndex) continue;
            if (i > logicalIndex)
                r.setLeft(r.left() - sectionSize(i));
            else
                r.setRight(r.right() + sectionSize(i));
        }
        MyBaseHeaderView::paintSection(painter, r, mColHiddenStart + 1);
    }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------
QHashbonTreeItem::QHashbonTreeItem() :
    QTreeWidgetItem(),
    mColSumStart(1), mColHiddenStart(26)
{
}

QHashbonTreeItem::QHashbonTreeItem(int aIdCheck, int aPercent,
                                   const QDate &aOrigDate, const QDate &aExpectDate, const QString &aInvoice,
                                   qlonglong aOrigSumBrutto, qlonglong aOrigSumFull,
                                   qlonglong aOrigSumBruttoIndexed, qlonglong aOrigSumFullIndexed,
                                   const QDate &aSignDate,
                                   qlonglong aSignSumBrutto, qlonglong aSignSumFull,
                                   qlonglong aSignSumBruttoIndexed, qlonglong aSignSumFullIndexed,
                                   const QDate &aPayDate, const QString &aPayInvoice,
                                   qlonglong aPaySumBrutto, qlonglong aPaySumFull,
                                   qlonglong aPaySumBruttoIndexed, qlonglong aPaySumFullIndexed,
                                   const QString &aComments) :
    QTreeWidgetItem()
{
    mIdCheck = aIdCheck;
    int col = 0;
    setText(col, QString::number(aPercent));
    setTextAlignment(col, Qt::AlignCenter);
    col++;

    setText(col, aOrigDate.toString("dd.MM.yyyy"));
    setTextAlignment(col, Qt::AlignCenter);
    mColSumStart = col;
    col++;

    if (!aExpectDate.isNull()) {
        setText(col, aExpectDate.toString("dd.MM.yyyy"));
        setTextAlignment(col, Qt::AlignCenter);
    }
    col++;

    setText(col, aInvoice);
    setTextAlignment(col, Qt::AlignCenter);
    col++;

    if (aOrigSumBrutto) setText(col, gSettings->FormatSumForList(aOrigSumBrutto));
    col++;
    if (aOrigSumFull && aOrigSumBrutto) setText(col, gSettings->FormatSumForList(aOrigSumFull - aOrigSumBrutto));
    col++;
    if (aOrigSumFull) setText(col, gSettings->FormatSumForList(aOrigSumFull));
    col++;

    if (aOrigSumBruttoIndexed) setText(col, gSettings->FormatSumForList(aOrigSumBruttoIndexed));
    col++;
    if (aOrigSumFullIndexed && aOrigSumBruttoIndexed) setText(col, gSettings->FormatSumForList(aOrigSumFullIndexed - aOrigSumBruttoIndexed));
    col++;
    if (aOrigSumFullIndexed) setText(col, gSettings->FormatSumForList(aOrigSumFullIndexed));
    col++;

    setText(col, aSignDate.toString("dd.MM.yyyy"));
    setTextAlignment(col, Qt::AlignCenter);
    col++;

    if (!aSignDate.isNull()) {
        if (aSignSumBrutto) setText(col, gSettings->FormatSumForList(aSignSumBrutto));
        col++;
        if (aSignSumFull && aSignSumBrutto) setText(col, gSettings->FormatSumForList(aSignSumFull - aSignSumBrutto));
        col++;
        if (aSignSumFull) setText(col, gSettings->FormatSumForList(aSignSumFull));
        col++;

        if (aSignSumBruttoIndexed) setText(col, gSettings->FormatSumForList(aSignSumBruttoIndexed));
        col++;
        if (aSignSumFullIndexed && aSignSumBruttoIndexed) setText(col, gSettings->FormatSumForList(aSignSumFullIndexed - aSignSumBruttoIndexed));
        col++;
        if (aSignSumFullIndexed) setText(col, gSettings->FormatSumForList(aSignSumFullIndexed));
        col++;
    } else {
        col += 6;
    }

    setText(col, aPayDate.toString("dd.MM.yyyy"));
    setTextAlignment(col, Qt::AlignCenter);
    col++;

    setText(col, aPayInvoice);
    setTextAlignment(col, Qt::AlignCenter);
    col++;

    if (!aPayDate.isNull()) {
        if (aPaySumBrutto) setText(col, gSettings->FormatSumForList(aPaySumBrutto));
        col++;
        if (aPaySumFull && aPaySumBrutto) setText(col, gSettings->FormatSumForList(aPaySumFull - aPaySumBrutto));
        col++;
        if (aPaySumFull) setText(col, gSettings->FormatSumForList(aPaySumFull));
        col++;

        if (aPaySumBruttoIndexed) setText(col, gSettings->FormatSumForList(aPaySumBruttoIndexed));
        col++;
        if (aPaySumFullIndexed && aPaySumBruttoIndexed) setText(col, gSettings->FormatSumForList(aPaySumFullIndexed - aPaySumBruttoIndexed));
        col++;
        if (aPaySumFullIndexed) setText(col, gSettings->FormatSumForList(aPaySumFullIndexed));
        col++;
    } else {
        col += 6;
    }

    setText(col, aComments);
    col++;

    mColHiddenStart = col;
}

QHashbonTree::QHashbonTree(QWidget *parent) :
    QTreeWidget(parent), justStart(true)
{
    header()->setSectionsMovable(false);

    QHashbonItemDelegate *ItemDelegate;
    ItemDelegate = new QHashbonItemDelegate(this);
    setItemDelegate(ItemDelegate);

}

void QHashbonTree::showEvent(QShowEvent * event)
{
    QTreeWidget::showEvent(event);

    if (justStart) {
        HashbonHeaderView *hv = new HashbonHeaderView(header(), Qt::Horizontal, this);
        hv->model()->setHeaderData(3, Qt::Horizontal, "מספר\nח-ן עסקה");
        hv->model()->setHeaderData(18, Qt::Horizontal, "מספר\nח-ן מס");
        this->setHeader(hv);
        justStart = false;

    }
}

void QHashbonTree::ClearTree()
{
    model()->removeRows(0, model()->rowCount());

}

void QHashbonTree::PopulateTree(int aIdContract, int aIdContractStage, qlonglong aContractSum)
{
    int i;

    model()->removeRows(0, model()->rowCount());

    QSqlQuery query(
                "SELECT ID, DONEPERCENT, ORIG_DATE, EXPECT_DATE, ORIG_SUM_BRUTTO, ORIG_SUM_FULL, ORIG_SUM_BRUTTO_INDEXED, ORIG_SUM_FULL_INDEXED,"
                " SIGN_DATE, SIGN_SUM_BRUTTO, SIGN_SUM_FULL, SIGN_SUM_BRUTTO_INDEXED, SIGN_SUM_FULL_INDEXED,"
                " PAY_DATE, PAY_INVOICE, PAY_SUM_BRUTTO, PAY_SUM_FULL, PAY_SUM_BRUTTO_INDEXED, PAY_SUM_FULL_INDEXED,"
                " INVOICE, COMMENTS"
                " FROM V_PKZ_HASHBON"
                " WHERE ID_CONTRACT = " + QString::number(aIdContract) +
                " OR ID_CONTRACT_STAGE = " + QString::number(aIdContractStage) +
                " ORDER BY ORIG_DATE", db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "חשבון", query);
        return;
    };

    int lDonePercent = 0;
    qlonglong lSumBrutto = 0;
    bool lAnyExpectDate = false;
    bool lAnyOrigIndexed = false, lAnyUnpayed = false;
    bool lAnySigned = false, lAnySignedIndexed = false;
    bool lAnyPayed = false, lAnyPayedIndexed = false;
    bool lAllSignedPayed = true;

    while (query.next()) {
        QHashbonTreeItem *item =
                new QHashbonTreeItem(query.value("ID").toInt(), query.value("DONEPERCENT").toInt(),
                                     query.value("ORIG_DATE").toDate(), query.value("EXPECT_DATE").toDate(), query.value("INVOICE").toString(),
                                     query.value("ORIG_SUM_BRUTTO").toLongLong(), query.value("ORIG_SUM_FULL").toLongLong(),
                                     query.value("ORIG_SUM_BRUTTO_INDEXED").toLongLong(), query.value("ORIG_SUM_FULL_INDEXED").toLongLong(),
                                     query.value("SIGN_DATE").toDate(),
                                     query.value("SIGN_SUM_BRUTTO").toLongLong(), query.value("SIGN_SUM_FULL").toLongLong(),
                                     query.value("SIGN_SUM_BRUTTO_INDEXED").toLongLong(), query.value("SIGN_SUM_FULL_INDEXED").toLongLong(),
                                     query.value("PAY_DATE").toDate(), query.value("PAY_INVOICE").toString(),
                                     query.value("PAY_SUM_BRUTTO").toLongLong(), query.value("PAY_SUM_FULL").toLongLong(),
                                     query.value("PAY_SUM_BRUTTO_INDEXED").toLongLong(), query.value("PAY_SUM_FULL_INDEXED").toLongLong(),
                                     query.value("COMMENTS").toString());

        if (!topLevelItemCount()) {
            ((HashbonHeaderView *) header())->SetColSumStart(item->ColSumStart());
            //((HashbonHeaderView *) header())->SetColHiddenStart(item->ColHiddenStart());
        }

        addTopLevelItem(item);

        lDonePercent += query.value("DONEPERCENT").toInt();
        lSumBrutto += query.value("ORIG_SUM_BRUTTO").toLongLong();

        if (!query.value("EXPECT_DATE").isNull() && query.value("PAY_DATE").isNull()) lAnyExpectDate = true;

        if (query.value("ORIG_SUM_BRUTTO_INDEXED").toLongLong() || query.value("ORIG_SUM_FULL_INDEXED").toLongLong()) lAnyOrigIndexed = true;

        if (!query.value("SIGN_DATE").isNull()) lAnySigned = true;
        if (query.value("SIGN_SUM_BRUTTO_INDEXED").toLongLong() || query.value("SIGN_SUM_FULL_INDEXED").toLongLong()) lAnySignedIndexed = true;
        if (!query.value("SIGN_DATE").isNull() && query.value("PAY_DATE").isNull()) lAllSignedPayed = false;

        if (!query.value("PAY_DATE").isNull()) lAnyPayed = true;
        if (query.value("PAY_DATE").isNull()) lAnyUnpayed = true;
        if (query.value("PAY_SUM_BRUTTO_INDEXED").toLongLong() || query.value("PAY_SUM_FULL_INDEXED").toLongLong()) lAnyPayedIndexed = true;
    }

    QStringList hls;
    for (i = 0; i < header()->model()->columnCount(); i++) {
        QString s1;
        s1 = header()->model()->headerData(i, Qt::Horizontal).toString();
        if (!i) {
            if (s1.lastIndexOf('\n') != -1) s1 = s1.left(s1.lastIndexOf('\n'));
            s1 = s1.replace(" " , "\n");
            s1 += "\n" + QString::number(100 - lDonePercent);
        } else if (i == 4) {
            if (s1.lastIndexOf('\n') != -1) s1 = s1.left(s1.lastIndexOf('\n'));
            s1 += "\n" + gSettings->FormatSumForList(aContractSum - lSumBrutto);
        }
        hls.append(s1);
    }
    setHeaderLabels(hls);

    if (!lAnySigned) lAnySignedIndexed = false;
    if (!lAnyPayed) lAnyPayedIndexed = false;

    // expected pay date
    header()->setSectionHidden(2, !lAnyExpectDate);

    // "auto hide empty signed" or "autohide signed if payment"
    for (i = 10; i <= 13; i++)
        header()->setSectionHidden(i, gSettings->Hashbon.AutoHideEmptySigned && !lAnySigned || gSettings->Hashbon.AutoHideSignIfPay && lAllSignedPayed && !lAnyUnpayed);

    // auto hide empty payed
    for (i = 17; i <= 21; i++)
        header()->setSectionHidden(i, gSettings->Hashbon.AutoHideEmptyPayed && !lAnyPayed);

    // auto hide empty indexed
    for (i = 7; i <= 9; i++)
        header()->setSectionHidden(i, gSettings->Hashbon.AutoHideEmptyIndexed && !lAnyOrigIndexed);
    for (i = 14; i <= 16; i++)
        header()->setSectionHidden(i, gSettings->Hashbon.AutoHideEmptyIndexed && !lAnySignedIndexed || gSettings->Hashbon.AutoHideSignIfPay && lAllSignedPayed && !lAnyUnpayed);
    for (i = 22; i <= 24; i++)
        header()->setSectionHidden(i, gSettings->Hashbon.AutoHideEmptyIndexed && !lAnyPayedIndexed);
}

QHashbonItemDelegate::QHashbonItemDelegate(QObject * parent) :
    QStyledItemDelegate(parent)
{
}

void QHashbonItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QRect r = option.rect;
    int curColumn = index.column();

    // do not use now - my drawing is slightly different from default, don't know how to correct
    if (false && (curColumn > 1 && curColumn < 8
            || curColumn > 8 && curColumn < 15)) {
        // columns with money values
        // I draw it myself because in right-to-left order it is drawed incorrect (ie, "1-" for -1)
        painter->save();
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(r, option.palette.highlight());
            QPen lPen(painter->pen());
            lPen.setColor(option.palette.highlightedText().color());
            painter->setPen(lPen);
        } else {
            painter->fillRect(r, qvariant_cast<QBrush>(index.data(Qt::BackgroundRole)));
        }
        painter->drawText(QRectF(r.left() + 2, r.top(), r.width() - 5, r.height()), Qt::AlignRight | Qt::AlignTop, index.data().toString());

        if (option.state & QStyle::State_HasFocus) {
            painter->setBackgroundMode(Qt::TransparentMode);
            QColor bg_col = option.palette.highlight().color();
            if (!bg_col.isValid()) bg_col = painter->background().color();
            // Create an "XOR" color.
            QColor patternCol((bg_col.red() ^ 0xff) & 0xff,
                              (bg_col.green() ^ 0xff) & 0xff,
                              (bg_col.blue() ^ 0xff) & 0xff);
            painter->setBrush(QBrush(patternCol, Qt::Dense4Pattern));
            painter->setBrushOrigin(r.topLeft());
            painter->setPen(Qt::NoPen);

            painter->drawRect(r.left(), r.top(), r.width(), 1);    // Top
            painter->drawRect(r.left(), r.bottom(), r.width(), 1); // Bottom
            painter->drawRect(r.left(), r.top(), 1, r.height());   // Left
            painter->drawRect(r.right(), r.top(), 1, r.height());  // Right
        }
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }

    //if (index.column() < index.model()->columnCount() - 1)
    if (index.column() > 0)
    {
        painter->save();
        if (option.state & QStyle::State_Selected && index.column() > 1) {
            QPen lPen(painter->pen());
            lPen.setColor(option.palette.highlightedText().color());
            painter->setPen(lPen);
        }
        //painter->drawLine(r.left(), r.top(), r.left(), r.bottom());
        painter->drawLine(r.right(), r.top(), r.right(), r.bottom());
        painter->restore();
    }

    if (index.row())
    {
        painter->drawLine(r.left(), r.top(), r.right(), r.top());
//        painter->save();
//        if (!index.model()->data(index.model()->index(index.row(), 0, index.parent())).isNull()) {
//            QPen lPen(painter->pen());
//            lPen.setWidth(2);
//            painter->setPen(lPen);
//            painter->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
//        } else {
//            painter->drawLine(r.left(), r.top(), r.right(), r.top());
//        }
//        painter->restore();
    }
}


HashbonHeaderView::HashbonHeaderView(const QHeaderView *headerOrig, Qt::Orientation orientation, QWidget* parent)
    : MyBaseHeaderView(headerOrig, orientation, parent),
      mColSumStart(1)
{
    mColHiddenStart = 26;

    for (int i = mColHiddenStart; i < count(); i++)
        setSectionHidden(i, true);
}

QSize HashbonHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize s = MyBaseHeaderView::sectionSizeFromContents(logicalIndex);
    s.setHeight((2 + (s.height() / 3)) * 3); // round for multiple of 3
    return s;
}

void HashbonHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    QRect r;
    int i, j;

//    if (rect.isValid()) {
//        if (!logicalIndex && r.left())
//            QMessageBox::critical(NULL, "חשבון", QString::number(r.left()));
//    }

    r = rect;

    // widths of sections - orig, signed, paues
    int mOrigSectionWidth = 9, mSignSectionWidth = 7, mPaySectionWidth = 8;
    int mSectWidths[] = { mOrigSectionWidth, mSignSectionWidth, mPaySectionWidth };
    // width of unindexed part of orig section
    int mOrigSectionUnindexedWidth = 6, mSignSectionUnindexedWidth = 4, mPaySectionUnindexedWidth = 5;
    int mSectUnindexedWidths[] = { mOrigSectionUnindexedWidth, mSignSectionUnindexedWidth, mPaySectionUnindexedWidth };

    if (logicalIndex >= mColSumStart && logicalIndex < mColSumStart + mOrigSectionUnindexedWidth
            || logicalIndex >= mColSumStart + mOrigSectionWidth && logicalIndex < mColSumStart + mOrigSectionWidth + mSignSectionUnindexedWidth
            || logicalIndex >= mColSumStart + mOrigSectionWidth + mSignSectionWidth
                && logicalIndex < mColSumStart + mOrigSectionWidth + mSignSectionWidth + mPaySectionUnindexedWidth) {
        // only 2/3 of bottom - not indexed amouonts
        r.setTop(r.top() + r.height() / 3);
    } else if (logicalIndex >= mColSumStart + mOrigSectionUnindexedWidth && logicalIndex < mColSumStart + mOrigSectionWidth + mSignSectionWidth + mPaySectionWidth) {
        // only 1/3 of bottom
        r.setTop(r.top() + r.height() * 2 / 3);
    }
    MyBaseHeaderView::paintSection(painter, r, logicalIndex);

    // "orig", "signed", "payed" headers
    int lIndStart = 0, lIndEnd = mSectWidths[0];
    for (j = 0; j < 3; j++) {
        if (logicalIndex >= lIndStart + mColSumStart && logicalIndex < lIndEnd + mColSumStart) {
            r = rect;
            // only 1/3 of top
            r.setBottom(r.bottom() - r.height() * 2 / 3);

            for (i = lIndStart + mColSumStart; i < lIndEnd + mColSumStart; i++) {
                if (i == logicalIndex) continue;
                if (i > logicalIndex)
                    r.setLeft(r.left() - sectionSize(i));
                else
                    r.setRight(r.right() + sectionSize(i));
            }

            MyBaseHeaderView::paintSection(painter, r, mColHiddenStart + j);
        }

        if (j < 2) {
            lIndStart = lIndEnd;
            lIndEnd += mSectWidths[j + 1];
        }
    }

    // "indexed" header
    lIndStart = mSectUnindexedWidths[0];
    for (j = 0; j < 3; j++) {
        if (logicalIndex >= mColSumStart + lIndStart && logicalIndex < mColSumStart + lIndStart + 3) {
            r = rect;
            r.setTop(r.top() + rect.height() / 3);
            r.setBottom(r.bottom() - rect.height() / 3);

            for (i = lIndStart + 1; i < lIndStart + 3; i++) {
                if (i == logicalIndex) continue;
                if (i > logicalIndex)
                    r.setLeft(r.left() - sectionSize(i));
                else
                    r.setRight(r.right() + sectionSize(i));
            }
            MyBaseHeaderView::paintSection(painter, r, mColHiddenStart + 3);
        }
        if (j < 2) {
            lIndStart -= mSectUnindexedWidths[j];
            lIndStart += mSectWidths[j] + mSectUnindexedWidths[j + 1];
        }
    }

//    for (j = mColSumStart + 3; j <= mColSumStart + 17; j += 7)
//        if (logicalIndex > j && logicalIndex < j + 4) {
//            r = rect;
//            r.setTop(r.top() + rect.height() / 3);
//            r.setBottom(r.bottom() - rect.height() / 3);

//            for (i = j + 1; i < j + 4; i++) {
//                if (i == logicalIndex) continue;
//                if (i > logicalIndex)
//                    r.setLeft(r.left() - sectionSize(i));
//                else
//                    r.setRight(r.right() + sectionSize(i));
//            }
//            MyBaseHeaderView::paintSection(painter, r, mColHiddenStart + 3);
//        }
}
