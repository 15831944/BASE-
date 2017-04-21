#include "qgeobasetree.h"

#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QPainter>
#include <QStyleOptionViewItem>

QGeobaseTree::QGeobaseTree(QWidget *parent) :
    QTreeWidget(parent)
{
    QGeobaseItemDelegate *ItemDelegate;
    ItemDelegate = new QGeobaseItemDelegate(this);
    setItemDelegate(ItemDelegate);
}

void QGeobaseTree::PopulateTree(int aGeobaseSelected) {
    int lIdProjectPrev = -1;

    model()->removeRows(0, model()->rowCount());

    QSqlQuery query(
                "SELECT A.ID ID, B.ID ID_PROJECT, PP.GETPROJECTSHORTNAME(A.ID_PROJECT) PROJNAME,"
                " C.SHORTNAME MAKER,"
                " PP.GETUSERNAMEDISP(B.GIP) GIP, B.STAGE,"
                " ORDERNUM, SITE_NUM, TO_CHAR(RECEIVE_DATE, 'DD.MM.YY') RECEIVE_DATE,"
                " TO_CHAR(EXPIRE_DATE, 'DD.MM.YY') EXPIRE_DATE, A.COMMENTS"
                " FROM V_GEOBASE A, V_PROJECT B, V_CUSTOMER C"
                " WHERE A.ID_PROJECT = B.ID"
                " AND A.ID_CUSTOMER = C.ID"
                " ORDER BY PP.GETPROJECTSHORTNAME(A.ID_PROJECT), PP.NUMSORT(ORDERNUM), PP.NUMSORT(SITE_NUM)", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
        return;
    }

    while (query.next()) {
        QGeobaseTreeItem *item;
        if (query.value("ID_PROJECT").toInt() != lIdProjectPrev) {
            item = new QGeobaseTreeItem(query.value("ID").toInt(), query.value("ID_PROJECT").toInt(),
                                        query.value("MAKER").toString(),
                                        query.value("PROJNAME").toString(), query.value("GIP").toString(),
                                        query.value("STAGE").toString(),
                                        query.value("ORDERNUM").toString(), query.value("SITE_NUM").toString(),
                                        query.value("RECEIVE_DATE").toString(), query.value("EXPIRE_DATE").toString(),
                                        query.value("COMMENTS").toString());
            lIdProjectPrev = query.value("ID_PROJECT").toInt();
        } else {
            item = new QGeobaseTreeItem(query.value("ID").toInt(), query.value("ID_PROJECT").toInt(),
                                        query.value("MAKER").toString(),
                                        "", "", "", query.value("ORDERNUM").toString(), query.value("SITE_NUM").toString(),
                                        query.value("RECEIVE_DATE").toString(), query.value("EXPIRE_DATE").toString(),
                                        query.value("COMMENTS").toString());
        }

        addTopLevelItem(item);
        for (int i = 0; i < 80; i += 10)
            PopulateDocs(item, query.value("ID").toInt(), i);

//        if (query.value(0).toInt() == mType
//                && query.value(1).toInt() == mProject) {
//            item->setSelected(true);
//            setCurrentItem(item);
//            // doesn't work right way; 5.1.0.
//            scrollToItem(item, QAbstractItemView::PositionAtCenter);
//        };

//        PopulateTreeInternal(item, query.value(1).toInt(), query.value(0).toInt(), aLavel + 1);

        item->setExpanded(true);

        if (query.value("ID").toInt() == aGeobaseSelected)
            setCurrentItem(item, 3);
    }
}

void QGeobaseTree::PopulateDocs(QGeobaseTreeItem *aParentItem, int aIdGeobase, int aType) {
    QSqlQuery query("SELECT A.ID, A.ID_PLOT, B.BLOCK_NAME FROM V_GEOBASE2PLOT A, V_PLOT_SIMPLE B "
                    "WHERE A.ID_GEOBASE = :ID_GEOBASE "
                    "AND A.ID_PLOT = B.ID "
                    "AND NVL(A.ID_XREFTYPE, 0) = :ID_XREFTYPE", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
        return;
    }

    query.bindValue(":ID_GEOBASE", aIdGeobase);
    query.bindValue(":ID_XREFTYPE", aType);

    if (!query.exec()) {
        gLogger->ShowSqlError(this, tr("Геоподосновы"), query.lastError());
    } else {
        int cnt = 0;
        while (query.next()) {
            if (!cnt) {
                aParentItem->SetParamForDrawing(aType, query.value("ID").toInt(),
                                                query.value("ID_PLOT").toInt(),
                                                query.value("BLOCK_NAME").toString());
            } else {
                QGeobaseTreeItem *item;
                if (cnt > aParentItem->childCount()) {
                    item = new QGeobaseTreeItem();
                    aParentItem->addChild(item);
                } else {
                    item = (QGeobaseTreeItem *) aParentItem->child(cnt - 1);
                }
                item->SetParamForDrawing(aType, query.value("ID").toInt(),
                                         query.value("ID_PLOT").toInt(),
                                         query.value("BLOCK_NAME").toString());
            }

            cnt++;
        }
    }
}

QGeobaseTreeItem::QGeobaseTreeItem(int aIdGeobase, int aIdProject, const QString &aMaker, const QString &aProjectName,
                                   const QString &aGip, const QString &aStage, const QString &aOrderNum, const QString &aSiteNum,
                                   const QString &aRecDate, const QString &aExpireDate,
                                   const QString &aComments) :
    QTreeWidgetItem()
{

    SetParamForGeobase(aIdGeobase, aIdProject, aMaker, aProjectName, aGip, aStage,
                                     aOrderNum, aSiteNum, aRecDate, aExpireDate,
                                     aComments);
}

QGeobaseTreeItem::QGeobaseTreeItem() :
    QTreeWidgetItem(),
    mIdGeobase(0), mIdProject(0)
{
    // the best thing we can to do - nothing
}

void QGeobaseTreeItem::SetParamForGeobase(int aIdGeobase, int aIdProject, const QString &aMaker, const QString &aProjectName,
                                          const QString &aGip, const QString &aStage,
                                          const QString &aOrderNum, const QString &aSiteNum, const QString &aRecDate,
                                          const QString &aExpireDate, const QString &aComments) {
    mIdGeobase = aIdGeobase;
    mIdProject = aIdProject;
    mMaker = aMaker;
    if (aProjectName.length()) setText(0, aProjectName);
    else setData(0, Qt::DisplayRole, QVariant());

    if (aGip.length()) setText(1, aGip);
    else setData(1, Qt::DisplayRole, QVariant());

    if (aStage.length()) setText(2, aStage);
    else setData(2, Qt::DisplayRole, QVariant());
    setTextAlignment(2, Qt::AlignHCenter);

    if (aOrderNum.length()) setText(3, aOrderNum);
    else setData(3, Qt::DisplayRole, QVariant());
    setTextAlignment(3, Qt::AlignHCenter);

    if (aSiteNum.length()) setText(4, aSiteNum);
    else setData(4, Qt::DisplayRole, QVariant());
    setTextAlignment(4, Qt::AlignHCenter);

    if (aRecDate.length()) setText(5, aRecDate);
    else setData(5, Qt::DisplayRole, QVariant());
    setTextAlignment(5, Qt::AlignHCenter);

    if (aExpireDate.length()) setText(6, aExpireDate);
    else setData(6, Qt::DisplayRole, QVariant());
    setTextAlignment(6, Qt::AlignHCenter);

    if (aComments.length()) setText(15, aComments);
    else setData(15, Qt::DisplayRole, QVariant());

    // !!!!! setFlags(flags() | Qt::ItemIsEditable); // it's working blya!
}

void QGeobaseTreeItem::SetParamForDrawing(int param, int aGeobase2PlotId, int aPlotId, const QString &aFilename) {
    QList<int> list;
    list << 40 << 30 << 10 << 20 << 60 << 70 << 50 << 0;

    if (list.indexOf(param) == -1) return;

    switch (gSettings->Geobase.DrawingShowMode) {
    case 0:
        setText(list.indexOf(param) + 7, QString::number(aPlotId));
        setTextAlignment(list.indexOf(param) + 7, Qt::AlignRight | Qt::AlignAbsolute);
        setToolTip(list.indexOf(param) + 7, aFilename);
        break;
    case 1:
        setText(list.indexOf(param) + 7, aFilename);
        setTextAlignment(list.indexOf(param) + 7, Qt::AlignLeft | Qt::AlignAbsolute);
        setToolTip(list.indexOf(param) + 7, QString::number(aPlotId));
        break;
    case 2:
        setText(list.indexOf(param) + 7, QString::number(aPlotId) + " - " + aFilename);
        setTextAlignment(list.indexOf(param) + 7, Qt::AlignLeft | Qt::AlignAbsolute);
        break;
    }

    mGeobase2PlotIdMap[param] = aGeobase2PlotId;
    mPlotIdMap[param] = aPlotId;
    mPlotNameMap[param] = aFilename;
}

QGeobaseItemDelegate::QGeobaseItemDelegate(QObject * parent) :
    QStyledItemDelegate(parent)
{
}

void QGeobaseItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QStyledItemDelegate::paint(painter, option, index);

    QRect r = option.rect;

    //painter->drawText(QPoint(r.left(), r.top()), QString::number(index.row()));

    if (index.row()
            && (!index.model()->data(index.model()->index(index.row(), 0, index.parent())).isNull()
                || index.column() >= 3 && !index.model()->data(index.model()->index(index.row(), 3, index.parent())).isNull())) {
        if (!index.model()->data(index.model()->index(index.row(), 0, index.parent())).isNull()) {
            QPen lPen(painter->pen());
            lPen.setWidth(2);
            painter->save();
            painter->setPen(lPen);
            painter->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
            painter->restore();
        } else {
            painter->drawLine(r.left(), r.top(), r.right(), r.top());
        }
    }
    if (index.column()) {
        if (index.column() == 7) {
            QPen lPen(painter->pen());
            lPen.setWidth(2);
            painter->save();
            painter->setPen(lPen);
            painter->drawLine(r.left() + 1, r.top(), r.left() + 1, r.bottom());
            painter->restore();
        } else {
            painter->drawLine(r.left(), r.top(), r.left(), r.bottom());
        }
    }
}
