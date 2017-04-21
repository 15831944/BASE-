#include "CDwgLayout.h"

#include "../VProject/common.h"

CDwgLayoutFactory1::CDwgLayoutFactory1() :
    mInsertDwgLayout(NULL), mInsertLayoutBlock(NULL), mInsertDwgLBAttr(NULL),
    mSelectLayoutBlock(NULL), mSelectLBAttr(NULL)
{

}

CDwgLayoutFactory1 * CDwgLayoutFactory1::GetInstance() {
    static CDwgLayoutFactory1 * lDwgLayoutFactory1 = NULL;
    if (!lDwgLayoutFactory1) {
        lDwgLayoutFactory1 = new CDwgLayoutFactory1();
    }
    return lDwgLayoutFactory1;
}

QSqlQuery *CDwgLayoutFactory1::qInsertDwgLayout() {
    if (mInsertDwgLayout) {
        return mInsertDwgLayout;
    }

    mInsertDwgLayout = new QSqlQuery(db);

    mInsertDwgLayout->prepare("insert into v_dwg_layout (id, id_dwg, num, name, sheet, namebottom, name_acad, sheet_acad, namebottom_acad, handle_hi, handle_lo) values"
                              " (:id, :id_dwg, :num, :name, :sheet, :namebottom, :name_acad, :sheet_acad, :namebottom_acad, :handle_hi, :handle_lo)");
    if (mInsertDwgLayout->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("DWG layout data insert"), *mInsertDwgLayout);
        delete mInsertDwgLayout;
        mInsertDwgLayout = NULL;
    }
    return mInsertDwgLayout;
}

QSqlQuery *CDwgLayoutFactory1::qInsertLayoutBlock() {
    if (mInsertLayoutBlock) {
        return mInsertLayoutBlock;
    }

    mInsertLayoutBlock = new QSqlQuery(db);

    mInsertLayoutBlock->prepare("insert into v_dwg_layout_block (id, id_dwg_layout, name, handle_hi, handle_lo) values"
                                " (:id, :id_dwg_layout, :name, :handle_hi, :handle_lo)");
    if (mInsertLayoutBlock->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Dwg layout block data insert"), *mInsertLayoutBlock);
        delete mInsertLayoutBlock;
        mInsertLayoutBlock = NULL;
    }
    return mInsertLayoutBlock;
}

QSqlQuery *CDwgLayoutFactory1::qInsertDwgLBAttr() {
    if (mInsertDwgLBAttr) {
        return mInsertDwgLBAttr;
    }

    mInsertDwgLBAttr = new QSqlQuery(db);

    mInsertDwgLBAttr->prepare("insert into v_dwg_lb_attr (id, id_dwg_lb, ordernum, tag, prompt, userfriendly_value, encoded_value, userfriendly_value_acad, encoded_value_acad,"
                              " text_style, text_font, text_hormode, text_vertmode, text_backward, text_upsidedown, text_height, text_width,"
                              " text_rotation, text_oblique, prop_layer, prop_linetype, prop_color, prop_lineweight, prop_plotstyle,"
                              " updatefromacad, handle_hi, handle_lo) values"
                              " (:id, :id_dwg_lb, :ordernum, :tag, :prompt, :userfriendly_value, :encoded_value, :userfriendly_value_acad, :encoded_value_acad,"
                              " :text_style, :text_font, :text_hormode, :text_vertmode, :text_backward, :text_upsidedown, :text_height, :text_width,"
                              " :text_rotation, :text_oblique, :prop_layer, :prop_linetype, :prop_color, :prop_lineweight, :prop_plotstyle,"
                              " current_timestamp, :handle_hi, :handle_lo)");
    if (mInsertDwgLBAttr->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Dwg layout block attribute data insert"), *mInsertDwgLBAttr);
        delete mInsertDwgLBAttr;
        mInsertDwgLBAttr = NULL;
    }
    return mInsertDwgLBAttr;
}

//--
QSqlQuery *CDwgLayoutFactory1::qSelectLayoutBlock() {
    if (mSelectLayoutBlock) {
        return mSelectLayoutBlock;
    }

    mSelectLayoutBlock = new QSqlQuery(db);

    mSelectLayoutBlock->prepare("select id, name from v_dwg_layout_block where id_dwg_layout = :id_dwg_layout");
    if (mSelectLayoutBlock->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Dwg layout block data select"), *mSelectLayoutBlock);
        delete mSelectLayoutBlock;
        mSelectLayoutBlock = NULL;
    }
    return mSelectLayoutBlock;
}

QSqlQuery *CDwgLayoutFactory1::qSelectLBAttr() {
    if (mSelectLBAttr) {
        return mSelectLBAttr;
    }

    mSelectLBAttr = new QSqlQuery(db);

    mSelectLBAttr->prepare("select a.id, a.ordernum, a.tag, a.prompt, a.userfriendly_value, a.encoded_value, a.userfriendly_value_acad, a.encoded_value_acad,"
                           " a.text_font, a.text_backward, b.field_code"
                           " from v_dwg_lb_attr a, v_dwg_field_def b where a.id_dwg_lb = :id_dwg_lb and a.tag = b.tag(+) order by a.ordernum");
    if (mSelectLBAttr->lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Dwg layout block attribute data select"), *mSelectLBAttr);
        delete mSelectLBAttr;
        mSelectLBAttr = NULL;
    }
    return mSelectLBAttr;
}

void CDwgLayoutFactory1::Clean() {
    if (mInsertDwgLayout) {
        mInsertDwgLayout->finish();
        delete mInsertDwgLayout;
        mInsertDwgLayout = NULL;
    }

    if (mInsertLayoutBlock) {
        mInsertLayoutBlock->finish();
        delete mInsertLayoutBlock;
        mInsertLayoutBlock = NULL;
    }

    if (mInsertDwgLBAttr) {
        mInsertDwgLBAttr->finish();
        delete mInsertDwgLBAttr;
        mInsertDwgLBAttr = NULL;
    }

    //--
    if (mSelectLayoutBlock) {
        mSelectLayoutBlock->finish();
        delete mSelectLayoutBlock;
        mSelectLayoutBlock = NULL;
    }

    if (mSelectLBAttr) {
        mSelectLBAttr->finish();
        delete mSelectLBAttr;
        mSelectLBAttr = NULL;
    }
}

//----------------------------------
CDwgLayout::CDwgLayout(void *aS1) :
    mId(0)
{
    memcpy(&s1, aS1, sizeof(s1));
}

CDwgLayout::~CDwgLayout() {
    qDeleteAll(mBlocks);
}

long CDwgLayout::GetDataSize() {
    return sizeof(s1Type);
}

const QList<CDwgLayoutBlock *> &CDwgLayout::BlocksConst() const {
    return mBlocks;
}

QList<CDwgLayoutBlock *> &CDwgLayout::BlocksRef() {
    return mBlocks;
}

bool CDwgLayout::InsertToBase(quint64 aIdDwg) {
    if (!mId && !gOracle->GetSeqNextVal("dwg_layout_id_seq", mId)) {
        return false;
    }

    QSqlQuery *qInsert = CDwgLayoutFactory1::GetInstance()->qInsertDwgLayout();
    if (!qInsert) return false;

    bool res = true;

    qInsert->bindValue(":id", mId);
    qInsert->bindValue(":id_dwg", aIdDwg);
    qInsert->bindValue(":num", s1.mOrderNum);
    qInsert->bindValue(":name", QString::fromWCharArray(s1.mName));
    qInsert->bindValue(":sheet", QString::fromWCharArray(s1.mSheet));
    qInsert->bindValue(":namebottom", QString::fromWCharArray(s1.mNameBottom));
    qInsert->bindValue(":name_acad", QString::fromWCharArray(s1.mNameAcad));
    qInsert->bindValue(":sheet_acad", QString::fromWCharArray(s1.mSheetAcad));
    qInsert->bindValue(":namebottom_acad", QString::fromWCharArray(s1.mNameBottomAcad));
    qInsert->bindValue(":handle_hi", s1.mHandleHigh);
    qInsert->bindValue(":handle_lo", s1.mHandleLo);
    if (!qInsert->exec()) {
        gLogger->ShowSqlError(QObject::tr("DWG layout data insert - exec"), *qInsert);
        res = false;
    } else {
        foreach (CDwgLayoutBlock *lBlock, mBlocks) {
            if (!lBlock->InsertToBase(mId)) {
                res = false;
                break;
            }
        }
    }

    return res;
}


//----------------------------------
CDwgLayoutBlock::CDwgLayoutBlock(void *aS1) :
    mId(0)
{
    memcpy(&s1, aS1, sizeof(s1));
}

CDwgLayoutBlock::~CDwgLayoutBlock() {
    qDeleteAll(mAttrs);
}

long CDwgLayoutBlock::GetDataSize() {
    return sizeof(s1Type);
}

const QList<CDwgLBAttr *> &CDwgLayoutBlock::AttrsConst() const {
    return mAttrs;
}

QList<CDwgLBAttr *> &CDwgLayoutBlock::AttrsRef() {
    return mAttrs;
}

bool CDwgLayoutBlock::InsertToBase(quint64 aIdDwgLayout) {
    if (!mId && !gOracle->GetSeqNextVal("dwg_layout_block_id_seq", mId)) {
        return false;
    }

    QSqlQuery *qInsert = CDwgLayoutFactory1::GetInstance()->qInsertLayoutBlock();
    if (!qInsert) return false;

    bool res = true;

    qInsert->bindValue(":id", mId);
    qInsert->bindValue(":id_dwg_layout", aIdDwgLayout);
    qInsert->bindValue(":name", QString::fromWCharArray(s1.mName));
    qInsert->bindValue(":handle_hi", s1.mHandleHigh);
    qInsert->bindValue(":handle_lo", s1.mHandleLo);
    if (!qInsert->exec()) {
        gLogger->ShowSqlError(QObject::tr("DWG layout block data insert - exec"), *qInsert);
        res = false;
    } else {
        foreach (CDwgLBAttr *lAttr, mAttrs) {
            if (!lAttr->InsertToBase(mId)) {
                res = false;
                break;
            }
        }
    }
    return res;
}

//----------------------------------
CDwgLBAttr::CDwgLBAttr(void *aS1) :
    mId(0)
{
    memcpy(&s1, aS1, sizeof(s1));
}

CDwgLBAttr::~CDwgLBAttr() {

}

long CDwgLBAttr::GetDataSize() {
    return sizeof(s1Type);
}

bool CDwgLBAttr::InsertToBase(quint64 aIdDwgLB) {
    if (!mId && !gOracle->GetSeqNextVal("dwg_lb_attr_id_seq", mId)) {
        return false;
    }

    QSqlQuery *qInsert = CDwgLayoutFactory1::GetInstance()->qInsertDwgLBAttr();
    if (!qInsert) return false;

    bool res = true;

    qInsert->bindValue(":id", mId);
    qInsert->bindValue(":id_dwg_lb", aIdDwgLB);

    qInsert->bindValue(":ordernum", s1.mOrderNum);
    qInsert->bindValue(":tag", QString::fromWCharArray(s1.mTag));
    qInsert->bindValue(":prompt", QString::fromWCharArray(s1.mPrompt));
    qInsert->bindValue(":userfriendly_value", QString::fromWCharArray(s1.mUFValue));
    qInsert->bindValue(":encoded_value", QString::fromWCharArray(s1.mEncValue));
    qInsert->bindValue(":userfriendly_value_acad", QString::fromWCharArray(s1.mUFValueAcad));
    qInsert->bindValue(":encoded_value_acad", QString::fromWCharArray(s1.mEncValueAcad));
    qInsert->bindValue(":text_style", QString::fromWCharArray(s1.mTextStyle));
    qInsert->bindValue(":text_font", QString::fromWCharArray(s1.mTextFont));
    qInsert->bindValue(":text_hormode", s1.mTextHorMode);
    qInsert->bindValue(":text_vertmode", s1.mTextVertMode);
    qInsert->bindValue(":text_backward", s1.mTextBackward);
    qInsert->bindValue(":text_upsidedown", s1.mTextUpsidedown);
    qInsert->bindValue(":text_height", s1.mTextHeight);
    qInsert->bindValue(":text_width", s1.mTextWidth);
    qInsert->bindValue(":text_rotation", s1.mTextRotation);
    qInsert->bindValue(":text_oblique", s1.mTextOblique);
    qInsert->bindValue(":prop_layer", QString::fromWCharArray(s1.mPropLayer));
    qInsert->bindValue(":prop_linetype", QString::fromWCharArray(s1.mPropLinetype));
    qInsert->bindValue(":prop_color", s1.mPropColor);
    qInsert->bindValue(":prop_lineweight", s1.mPropLineWeight);
    qInsert->bindValue(":prop_plotstyle", QString::fromWCharArray(s1.mPropPlotStyle));
    qInsert->bindValue(":handle_hi", s1.mHandleHigh);
    qInsert->bindValue(":handle_lo", s1.mHandleLo);
    if (!qInsert->exec()) {
        gLogger->ShowSqlError(QObject::tr("Dwg layout block attribute data insert - exec"), *qInsert);
        res = false;
    }
    return res;
}
