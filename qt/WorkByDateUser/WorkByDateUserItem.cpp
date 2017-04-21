#include "workbydateuser.h"
#include "../VProject/GlobalSettings.h"

#include <QHeaderView>

bool WorkByDateUserItem::LessByCodeSheet(const QTreeWidgetItem & other, int aSortCol) const {
    // return "not less" if equal
    if (text(mWorkByDateUser->mColCode) == other.text(mWorkByDateUser->mColCode)
            && text(mWorkByDateUser->mColSheet) == other.text(mWorkByDateUser->mColSheet)) return false;

    bool res;
    if (text(mWorkByDateUser->mColCode) != other.text(mWorkByDateUser->mColCode))
        res = CmpStringsWithNumbersNoCase(text(mWorkByDateUser->mColCode), other.text(mWorkByDateUser->mColCode));
    else
        res = CmpStringsWithNumbersNoCase(text(mWorkByDateUser->mColSheet), other.text(mWorkByDateUser->mColSheet));
    // code+sheet is always in straight order
    if (treeWidget()->header()->sortIndicatorOrder() == Qt::AscendingOrder
            || aSortCol == mWorkByDateUser->mColCode)
        return res;
    else
        return !res;
}

void WorkByDateUserItem::SetMainWidget(WorkByDateUser * aWorkByDateUser) {
    mWorkByDateUser = aWorkByDateUser;
}

bool WorkByDateUserItem::operator<(const QTreeWidgetItem & other) const {
    int sortCol = treeWidget()->sortColumn();

    if (sortCol == mWorkByDateUser->mColChangeDate
            || sortCol == mWorkByDateUser->mColStartTime
            || sortCol == mWorkByDateUser->mColEndTime) {
        QDateTime dt1 = QDateTime::fromString(text(sortCol), "dd.MM.yyyy HH:mm"),
                dt2 = QDateTime::fromString(other.text(sortCol), "dd.MM.yyyy HH:mm");
        return dt1 < dt2;
    } else {
        if (text(sortCol) != other.text(sortCol))
            return CmpStringsWithNumbersNoCase(text(sortCol), other.text(sortCol));
        else {
            if (text(mWorkByDateUser->mColCode) != other.text(mWorkByDateUser->mColCode)
                    || text(mWorkByDateUser->mColSheet) != other.text(mWorkByDateUser->mColSheet))
                return LessByCodeSheet(other, sortCol);
            else
                return text(mWorkByDateUser->mColHistOut).toInt() < other.text(mWorkByDateUser->mColHistOut).toInt();
        }
    }
//    } else if (sortCol == mWorkByDateUser->mColIdProj
//               || sortCol == mWorkByDateUser->mColHistIn
//               || sortCol == mWorkByDateUser->mColHistOut
//               || sortCol == mWorkByDateUser->mColHistMax) {
//        int i1 = text(sortCol).toInt(), i2  = other.text(sortCol).toInt();
//        if (i1 != i2)
//            return i1 < i2;
//        else {
//            if (text(mWorkByDateUser->mColCode) != other.text(mWorkByDateUser->mColCode)
//                    || text(mWorkByDateUser->mColSheet) != other.text(mWorkByDateUser->mColSheet))
//                return LessByCodeSheet(other, sortCol);
//            else
//                return text(mWorkByDateUser->mColHistOut).toInt() < other.text(mWorkByDateUser->mColHistOut).toInt();
//        }
//    } else if (sortCol == mWorkByDateUser->mColIdPlot) {
//        if (text(sortCol) != other.text(sortCol))
//            return CmpStringsWithNumbersNoCase(text(sortCol), other.text(sortCol));
//        else
//            return text(mWorkByDateUser->mColHistOut).toInt() < other.text(mWorkByDateUser->mColHistOut).toInt();
//    }

    //return QTreeWidgetItem::operator<(other);
}


