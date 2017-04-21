#include "tableinsert.h"

TableInsert::TableInsert(QWidget *parent) : QTableWidget(parent)
{

}

void TableInsert::import(QString fileName, int sheetNumber){

    int sheetCount = sheets->dynamicCall("Count()").toInt();        //worksheets count
    if (sheetNumber > sheetCount)
        return;

    int intRowStart = usedrange->property("Row").toInt();
    int intColStart = usedrange->property("Column").toInt();
    int intCols = columns->property("Count").toInt();
    int intRows = rows->property("Count").toInt();

    // replicate the Excel content in the QTableWidget
    this->setColumnCount(intColStart+intCols);
    this->setRowCount(intRowStart+intRows);
    for (int row=intRowStart ; row < intRowStart+intRows ; row++) {
        for (int col=intColStart ; col < intColStart+intCols ; col++) {



            QTableWidgetItem * twi = new QTableWidgetItem;
            twi->setData(Qt::DisplayRole, value);


            this->setItem(row-1, col-1, twi);
        }
    }



}

void TableInsert::setHorizontalAlignmentForCell(QTableWidgetItem *twi){



    int horizAlign = horzAlignVariant.toInt();

    switch (horizAlign) { // note these are all MS "Magic Numbers"
    case -4117: twi->setTextAlignment(Qt::AlignHCenter); break;
    case -4130: twi->setTextAlignment(Qt::AlignJustify); break;
    case -4131: twi->setTextAlignment(Qt::AlignLeft); break;
    case -4108: twi->setTextAlignment(Qt::AlignHCenter); break;
    case -4152: twi->setTextAlignment(Qt::AlignRight); break;
    default:
        break;
    }

}

