#ifndef QGEOBASETREE_H
#define QGEOBASETREE_H

#include <QTreeWidget>
#include <QStyledItemDelegate>

class QGeobaseTreeItem : public QTreeWidgetItem
{
protected:
    int mIdGeobase, mIdProject;
    QString mMaker;

    std::map<int, int> mGeobase2PlotIdMap, mPlotIdMap;
    std::map<int, QString> mPlotNameMap;
public:
    explicit QGeobaseTreeItem(int aIdGeobase, int aIdProject, const QString &aMaker, const QString &aProjectName,
                              const QString &aGip, const QString &aStage, const QString &aOrderNum, const QString &aSiteNum,
                              const QString &aRecDate, const QString &aExpireDate,
                              const QString &aComments);
    explicit QGeobaseTreeItem();

    void SetParamForGeobase(int aIdGeobase, int aIdProject, const QString &aMaker, const QString &aProjectName,
                            const QString &aGip, const QString &aStage,
                            const QString &aOrderNum, const QString &aSiteNum, const QString &aRecDate,
                            const QString &aExpireDate, const QString &aComments);
    void SetParamForDrawing(int param, int aGeobase2PlotId, int aPlotId, const QString &aFilename);

    int IdGeobase() { return mIdGeobase; }
    int IdProject() { return mIdProject; }

    int IdGeobase2Plot(int aType) { return mGeobase2PlotIdMap[aType]; }
    int IdPlot(int aType) { return mPlotIdMap[aType]; }
    QString PlotName(int aType) { return mPlotNameMap[aType]; }

    QString Maker() const { return mMaker; }
    QString OrderNum() const { return text(3); }
    QString SiteNum() const { return text(4); }

};

class QGeobaseTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QGeobaseTree(QWidget *parent = 0);

    QTreeWidgetItem * itemFromModelIndex(const QModelIndex & index) const { return QTreeWidget::itemFromIndex(index); }

    void PopulateTree(int aGeobaseSelected);

protected:
    void PopulateDocs(QGeobaseTreeItem *aParentItem, int aIdGeobase, int aType);

    virtual void drawBranches(QPainter *, const QRect &, const QModelIndex &) const {}
};

class QGeobaseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QGeobaseItemDelegate(QObject * parent = 0);
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};


#endif // QGEOBASETREE_H
