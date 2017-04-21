#ifndef TABLEINSERT_H
#define TABLEINSERT_H


#include <QTableWidget>



class TableInsert : public QTableWidget
{
    Q_OBJECT

public:
    explicit TableInsert(QWidget *parent = 0);
    ~TableInsert(){}

public slots:
    void import(QString fileName, int sheetNumber=1);

private:
    void setHorizontalAlignmentForCell(QTableWidgetItem *twi);
    //int junk = 0;
};

#endif // TABLEINSERT_H
