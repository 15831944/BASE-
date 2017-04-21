#include "CDwgLayout.h"
#include "DwgLayoutData.h"

#include "../VProject/common.h"

DwgLayoutData::DwgLayoutData(quint64 aId, quint64 aIdDwg, int aNum, const QString &aName, const QString &aSheet, const QString &aBottom) :
    InitParRO(Id), InitParRO(IdDwg), InitParRO(Num), InitPar(Name), InitPar(Sheet), InitPar(Bottom),
    mIsNew(false)
{
    QSqlQuery *qSelect = CDwgLayoutFactory1::GetInstance()->qSelectLayoutBlock();
    if (!qSelect) return;

    qSelect->bindValue(":id_dwg_layout", mId);
    if (!qSelect->exec()) {
        gLogger->ShowSqlError("Dwg layout block data select - exec", *qSelect);
    } else {
        while (qSelect->next()) {
            mBlocks.append(new DwgLayoutBlockData(qSelect->value("id").toInt(), qSelect->value("name").toString()));
        }
    }

    CollectProps();
}

DwgLayoutData::~DwgLayoutData() {
    qDeleteAll(mBlocks);
}

bool DwgLayoutData::HasAnyProp() const {
    for (int i = 0; i < mBlocks.length(); i++)
        if (mBlocks.at(i)->HasAnyProp()) return true;
    return false;
}

void DwgLayoutData::CollectProps() {

    for (int i = 0; i < mBlocks.length(); i++) {
        DwgLayoutBlockData *lBlock = mBlocks.at(i);
        lBlock->ClearProps();
        for (int j = 0; j < lBlock->AttrsConst().length(); j++) {
            const DwgLBAttrData *lAttr = lBlock->AttrsConst().at(j);

            switch (lAttr->FieldCode()) {
            case 2:
                lBlock->AddCode(lAttr->EncValueConst());
                break;
            case 3:
                lBlock->AddRevision(lAttr->EncValueConst());
                break;
            case 4:
                lBlock->AddRevDate(lAttr->EncValueConst());
                break;
            case 5:
                lBlock->AddNameTop(lAttr->EncValueConst());
                break;
            case 6:
                lBlock->AddNameBottom(lAttr->EncValueConst());
                break;
            case 7:
                lBlock->AddSheetFileName(lAttr->EncValueConst());
                break;
            case 8:
                lBlock->AddPlotFileName(lAttr->EncValueConst());
                break;
            }
        }
    }
}

void DwgLayoutData::GetMatzDFTexts(QString &aDText, QString &aFText) {
    aDText.clear();
    aFText.clear();
    foreach (const DwgLayoutBlockData *lBlock, mBlocks) {
        bool lHasSD = false, lHasPD = false, lHasDD = false;
        bool lHasFI = false, lHasFA = false, lHasFT = false, lHasFC = false;

        foreach (const DwgLBAttrData *lAttr, lBlock->AttrsConst()) {
            if (lAttr->TagConst() == "SD") {
                lHasSD = true;
                if (!lAttr->UFValueConst().isEmpty()) aDText = lAttr->PromptConst();
            } else if (lAttr->TagConst() == "PD") {
                lHasPD = true;
                if (!lAttr->UFValueConst().isEmpty()) aDText = lAttr->PromptConst();
            } else if (lAttr->TagConst() == "DD") {
                lHasDD = true;
                if (!lAttr->UFValueConst().isEmpty()) aDText = lAttr->PromptConst();
            } else if (lAttr->TagConst() == "FI") {
                lHasFI = true;
                if (!lAttr->UFValueConst().isEmpty()) aFText = lAttr->PromptConst();
            } else if (lAttr->TagConst() == "FA") {
                lHasFA = true;
                if (!lAttr->UFValueConst().isEmpty()) aFText = lAttr->PromptConst();
            } else if (lAttr->TagConst() == "FT") {
                lHasFT = true;
                if (!lAttr->UFValueConst().isEmpty()) aFText = lAttr->PromptConst();
            } else if (lAttr->TagConst() == "FC") {
                lHasFC = true;
                if (!lAttr->UFValueConst().isEmpty()) aFText = lAttr->PromptConst();
            }
        }

//        if (lHasSD && lHasPD && lHasDD) {
//            //SD - שלב תכנון - ראשוני;
//            //PD - שלב תכנון - מוקדם;
//            //DD - שלב תכנון - מפורט;
//            aDText.replace("שלב תכנון - ", "");
//        }
//        if (lHasFI && lHasFA && lHasFT && lHasFC) {
//            //FI - מטרה - לעיון;
//            //FA - מטרה - לאישור;
//            //FT- מטרה - למכרז;
//            //FC - מטרה - לביצוע;
//            aFText.replace("מטרה - ", "");
//        }
        if (lHasSD && lHasPD && lHasDD
                && lHasFI && lHasFA && lHasFT && lHasFC) {
            //SD - שלב תכנון - ראשוני;
            //PD - שלב תכנון - מוקדם;
            //DD - שלב תכנון - מפורט;
            aDText.replace("שלב תכנון - ", "");
            //FI - מטרה - לעיון;
            //FA - מטרה - לאישור;
            //FT- מטרה - למכרז;
            //FC - מטרה - לביצוע;
            aFText.replace("מטרה - ", "");
            break;
        }
    }
}

bool DwgLayoutData::SaveData() {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;

    if (!mIsNew) {
        CheckParChange(Name, name);
        CheckParChange(Sheet, sheet);
        CheckParChange(Bottom, namebottom);

        if (!lUpdStr.isEmpty()) {
            lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

            QSqlQuery qUpdate(db);

            qUpdate.prepare("update v_dwg_layout set " + lUpdStr + " where id = ?");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(QObject::tr("Drawing layout data"), qUpdate);
            } else {
                for (int i = 0; i < lUpdValues.length(); i++) {
                    qUpdate.addBindValue(lUpdValues.at(i));
                }
                qUpdate.addBindValue(mId);
                if (!qUpdate.exec()) {
                    gLogger->ShowSqlError(QObject::tr("Drawing layout data"), qUpdate);
                } else {
                    res = true;
                }
            }
        } else {
            res = true;
        }
    } else {
        // no insert, need not; use CDwgLayoutData instead
//        if (!mId) {
//            // so we can refresh later
//            if (!gSettings->GetSeqNextVal("dwg_layout_id_seq", mId)) return false;
//        }

//        QSqlQuery qInsert(db);

//        qInsert.prepare("insert into v_dwg_layout (id, id_dwg, num, name, namebottom, sheet, handle_hi, handle_lo)"
//                        " values (:id, :id_dwg, :num, :name, :namebottom, :sheet, :handle_hi, :handle_lo)");
//        if (qInsert.lastError().isValid()) {
//            gLogger->ShowSqlError(QObject::tr("Drawing layout data"), qInsert);
//        } else {
//            qInsert.bindValue(":id", mId);
//            qInsert.bindValue(":id_dwg", mIdDwg);
//            qInsert.bindValue(":num", mNum);
//            qInsert.bindValue(":name", mName);
//            qInsert.bindValue(":namebottom", mBottom);
//            qInsert.bindValue(":sheet", mSheet);
//            qInsert.bindValue(":handle_hi", mHandleHigh);
//            qInsert.bindValue(":handle_lo", mHandleLow);
//            if (!qInsert.exec()) {
//                gLogger->ShowSqlError(QObject::tr("Drawing layout data"), qInsert);
//            } else {
//                //res = RefreshData(); need not now
//                res = true;
//                mIsNew = !res;
//            }
//        }
    }
    return res;
}

void DwgLayoutData::RollbackEdit() {
    RollbackPar(Name)
    RollbackPar(Sheet)
    RollbackPar(Bottom)
}


void DwgLayoutData::CommitEdit() {
    CommitPar(Name)
    CommitPar(Sheet)
    CommitPar(Bottom)
}

//-----------------------------------------------------------------------
DwgLayoutBlockData::DwgLayoutBlockData(long aId, const QString &aName) :
    InitParRO(Id), InitParRO(Name), mHasAnyProp(false)
{
    QSqlQuery *qSelect = CDwgLayoutFactory1::GetInstance()->qSelectLBAttr();
    if (!qSelect) return;

    qSelect->bindValue(":id_dwg_lb", mId);
    if (!qSelect->exec()) {
        gLogger->ShowSqlError("Dwg lb attr data select - exec", *qSelect);
    } else {
        while (qSelect->next()) {
            mAttrs.append(new DwgLBAttrData(qSelect->value("id").toInt(), qSelect->value("ordernum").toInt(), qSelect->value("tag").toString(), qSelect->value("prompt").toString(),
                                            qSelect->value("userfriendly_value").toString(), qSelect->value("encoded_value").toString(),
                                            qSelect->value("userfriendly_value_acad").toString(), qSelect->value("encoded_value_acad").toString(),
                                            qSelect->value("text_font").toString(), qSelect->value("text_backward").toInt(), qSelect->value("field_code").toInt()));
        }
    }
}

DwgLayoutBlockData::~DwgLayoutBlockData() {
    qDeleteAll(mAttrs);
}

void DwgLayoutBlockData::ClearProps() {
    mSheetFileNames.clear();
    mPlotFileNames.clear();
    mCodes.clear();
    mRevisions.clear();
    mRevDates.clear();

    mHasAnyProp = false;
}

bool DwgLayoutBlockData::HasAnyProp() const {
    return mHasAnyProp;
}

#define AddParam(ParamName) \
    void DwgLayoutBlockData::Add##ParamName(const QString &a##ParamName) { \
        m##ParamName##s.append(a##ParamName); \
        mHasAnyProp = true; \
    }

AddParam(Code)
AddParam(Revision)
AddParam(RevDate)
AddParam(NameTop)
AddParam(NameBottom)
AddParam(SheetFileName)
AddParam(PlotFileName)

#undef AddParam

#define GetParams(ParamName) \
    const QStringList &DwgLayoutBlockData::ParamName##s() { \
        return m##ParamName##s; \
    }

GetParams(Code)
GetParams(Revision)
GetParams(RevDate)
GetParams(NameTop)
GetParams(NameBottom)
GetParams(SheetFileName)
GetParams(PlotFileName)

#undef GetParams

//-----------------------------------------------------------------------
DwgLBAttrData::DwgLBAttrData(int aId, int aOrderNum, const QString &aTag, const QString &aPrompt,
                       const QString &aUFValue, const QString &aEncValue,
                       const QString &aUFValueAcad, const QString &aEncValueAcad,
                       const QString &aTextFont, int aTextBackward, int aFieldCode) :
    InitParRO(Id), InitParRO(OrderNum), InitParRO(Tag), InitParRO(Prompt),
    InitPar(UFValue), InitPar(EncValue), InitParRO(UFValueAcad), InitParRO(EncValueAcad),
    InitParRO(TextFont), InitParRO(TextBackward), InitParRO(FieldCode)
{
}

QString DwgLBAttrData::GetValue() const {
    int i, j;
    bool lHasHebrew = false, lHasNonHebrew = false;
    QString lBothChars(",.:;()=+-/\\*%^?!|$@'\"");

    for (i = 0; i < mUFValue.length(); i++) {
        if (mUFValue.at(i) >= 0x0590 && mUFValue.at(i) <= 0x05ff) lHasHebrew = true;
        if (!(mUFValue.at(i) >= 0x0590 && mUFValue.at(i) <= 0x05ff)
                && !(mUFValue.at(i) == ' ')
                && !lBothChars.contains(mUFValue.at(i))
                )
            lHasNonHebrew = true;

        if (lHasHebrew && lHasNonHebrew) break;
    }

    if (lHasHebrew && lHasNonHebrew) {
        QString lRes = mUFValue.trimmed();
        int lLast = 0; // 0 - initial value, 1 - hebrew, 2 - non hebrew

        for (i = lRes.length() - 1; i >= 0; i--) {
            if (lLast == 2) {
                if (lRes.at(i) == '(') {
                    lRes.replace(i, 1, ')');
                    continue;
                }
                if (lRes.at(i) == ')') {
                    lRes.replace(i, 1, '(');
                    continue;
                }
            }
            if (lRes.at(i) == ' ' || lBothChars.contains(lRes.at(i))) continue;
            if (!(lRes.at(i) >= 0x0590 && lRes.at(i) <= 0x05ff)) {
                // non hebrew
                if (lLast == 1) {
                    // was hebrew
                    j = i;
                    // get other spaces or symbols
                    //while (j + 1 < lRes.length() && (lRes.at(j + 1) == ' ' || lBothChars.contains(lRes.at(j + 1)))) j++;
                    //QMessageBox::critical(gMainWindow, QObject::tr("right to left"), QString("!") + lRes.at(j) + "!");
                    // skip last space, leave it with hebrew direction
                    if (lRes.at(j) == ' ') j--;
                    //QMessageBox::critical(gMainWindow, QObject::tr("right to left"), "!" + lRes.mid(j + 1) + "!");
                    //QMessageBox::critical(gMainWindow, QObject::tr("right to left"), QString("!") + lRes.at(j) + "!");
                    lRes.insert(j + 1, QChar(0x200f)); // r2l
                }
                lLast = 2;
            } else {
                // hebrew
                if (lLast == 2) {
                    // was non-hebrew
                    j = i;
                    // get other spaces or symbols
                    //while (j + 1 < lRes.length() && (lRes.at(j + 1) == ' ' || lBothChars.contains(lRes.at(j + 1)))) j++;
                    // get nexp space too
                    //if (j + 1 < lRes.length() && lRes.at(j + 1) == ' ') j++;
                    //QMessageBox::critical(gMainWindow, QObject::tr("left 2 right"), QString("!") + lRes.at(j - 1) + "!");
                    lRes.insert(j + 1, QChar(0x200e)); // l2r
                }
                lLast = 1;
            }
        }
        if (lLast == 1) {
            //QMessageBox::critical(gMainWindow, QObject::tr("right to left"), "!" + lRes + "!");
            lRes.insert(0, QChar(0x200f));
        } else if (lLast == 2) {
            //QMessageBox::critical(gMainWindow, QObject::tr("left 2 right"), "!" + lRes + "!");
            lRes.insert(0, QChar(0x200e));
        }

        return lRes;
    } else {
        return mUFValue.trimmed();
    }
}

void DwgLBAttrData::SetValue(QString aValue) {
    if (aValue.trimmed() == mUFValue.trimmed()) return; // no changes, I think; may be trimmed need not

    QString lRes;
    int i;
    bool lIsMirym = false;

    aValue.replace(QChar(0x200e), "");
    aValue.replace(QChar(0x200f), "");

    bool lHasHebrew = false, lHasNonHebrew = false;
    QString lBothChars(",.:;()=+-/\\*%^?!|$@'\"");

    for (i = 0; i < aValue.length(); i++) {
        if (aValue.at(i) >= 0x0590 && aValue.at(i) <= 0x05ff) lHasHebrew = true;
        if (!(aValue.at(i) >= 0x0590 && aValue.at(i) <= 0x05ff)
                && !(aValue.at(i) == ' ')
                && !lBothChars.contains(aValue.at(i)))
            lHasNonHebrew = true;

        if (lHasHebrew && lHasNonHebrew) break;
    }

    if (lHasHebrew && lHasNonHebrew) {
        int lLast = 0; // 0 - initial value, 1 - hebrew, 2 - non hebrew

        // it is for brackets swapping only
        for (i = aValue.length() - 1; i >= 0; i--) {
            if (lLast == 2) {
                // for non-hebrew only
                if (aValue.at(i) == '(') {
                    aValue.replace(i, 1, ')');
                    continue;
                }
                if (aValue.at(i) == ')') {
                    aValue.replace(i, 1, '(');
                    continue;
                }
            }
            if (aValue.at(i) == '('
                    || aValue.at(i) == ')'
                    || aValue.at(i) == ','
                    || aValue.at(i) == '.'
                    || aValue.at(i) == ':'
                    || aValue.at(i) == '-') continue;
            if (!(aValue.at(i) >= 0x0590 && aValue.at(i) <= 0x05ff)
                    && !(aValue.at(i) == ' ')
                    && !(aValue.at(i) == ',')
                    && !(aValue.at(i) == '.')
                    && !(aValue.at(i) == ':')
                    && !(aValue.at(i) == '-')
                    && !(aValue.at(i) == '(')
                    && !(aValue.at(i) == ')')) {
                // non hebrew
                lLast = 2;
            } else {
                // hebrew
                lLast = 1;
            }
        }
    }

    if (mTextFont.compare("0-miryl.shx", Qt::CaseInsensitive) == 0
            || mTextFont.compare("0-miryl", Qt::CaseInsensitive) == 0
            || mTextFont.compare("miryl.shx", Qt::CaseInsensitive) == 0
            || mTextFont.compare("miryl", Qt::CaseInsensitive) == 0
            || mTextFont.compare("0-mirym.shx", Qt::CaseInsensitive) == 0
            || mTextFont.compare("0-mirym", Qt::CaseInsensitive) == 0
            || mTextFont.compare("mirym.shx", Qt::CaseInsensitive) == 0
            || mTextFont.compare("mirym", Qt::CaseInsensitive) == 0) {
        lIsMirym = true;

        QString lMirylStr("tcdsvuzjyhlfkonibxg;p.mera,");

        for (i = 0; i < aValue.length(); i++) {
            if (aValue.at(i) == 0x5f3) {
                lRes += 'w';
            } else if (aValue.at(i) == 0x5f4) {
                lRes += '"';
            } else if (aValue.at(i) == '.') {
                lRes += '/';
            } else if (aValue.at(i) == ',') {
                lRes += '\'';
            } else if (aValue.at(i) == '/') {
                lRes += 'q';
            } else if (aValue.at(i) >= 0x5d0 && aValue.at(i) < 0x5d0 + lMirylStr.length()) {
                lRes += lMirylStr.at(aValue.at(i).unicode() - 0x5d0);
            } else {
                gLogger->ShowError(QObject::tr("Hebrew decode"), QString("Invalid character: '") + aValue.at(i) + "' (0x" + QString::number(aValue.at(i).unicode(), 16) + ")");
            }
        }
    } else {
        lRes = aValue;
    }

    if (mTextBackward || lIsMirym && !mTextBackward) {
        // rotate english text;
        int lRStart = -1, lREnd;
        for (i = 0; i < lRes.length(); i++) {
            if (!(lRes.at(i) >= 0x0590 && lRes.at(i) <= 0x05ff)
                    && !(lRes.at(i) == ' ')
                    && !(lRes.at(i) == ',')
                    /*&& !(lRes.at(i) == '.')*/
                    && !(lRes.at(i) == ':')
                    && !(lRes.at(i) == '-')
                    ) {
                // non hebrew
                if (lRStart == -1) lRStart = i;
            } else {
                // hebrew
                if (lRStart != -1) {
                    lREnd = i - 1;
                    while (lREnd > lRStart) {
                        // swap
                        QChar lChar = lRes.at(lRStart);
                        lRes.replace(lRStart, 1, lRes.at(lREnd));
                        lRes.replace(lREnd, 1, lChar);
                        lRStart++;
                        lREnd--;
                    }
                    lRStart = -1;
                }
            }
        }
        if (lRStart != -1) {
            lREnd = lRes.length() - 1;
            while (lREnd > lRStart) {
                // swap
                QChar lChar = lRes.at(lRStart);
                lRes.replace(lRStart, 1, lRes.at(lREnd));
                lRes.replace(lREnd, 1, lChar);
                lRStart++;
                lREnd--;
            }
        }
    }
    mUFValue = aValue;
    mEncValue = lRes;
}

bool DwgLBAttrData::SaveData() {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;

    CheckParChange(UFValue, userfriendly_value)
    CheckParChange(EncValue, encoded_value)
    if (!lUpdStr.isEmpty()) {
        lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

        QSqlQuery qUpdate(db);

        qUpdate.prepare("update v_dwg_lb_attr set " + lUpdStr + ", updatefromgui = current_timestamp where id = ?");
        if (qUpdate.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Dwg layout block attribute data"), qUpdate);
        } else {
            for (int i = 0; i < lUpdValues.length(); i++) {
                qUpdate.addBindValue(lUpdValues.at(i));
            }
            qUpdate.addBindValue(mId);
            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(QObject::tr("Dwg layout block attribute data"), qUpdate);
            } else {
                res = true;
            }
        }
    } else {
        res = true;
    }
    return res;
}

void DwgLBAttrData::RollbackEdit() {
    RollbackPar(UFValue)
    RollbackPar(EncValue)
}

void DwgLBAttrData::CommitEdit() {
    CommitPar(UFValue)
    CommitPar(EncValue)
}
