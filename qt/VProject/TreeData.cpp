#include "TreeData.h"

#include "common.h"

#include <QVariant>

TreeDataRecord::TreeDataRecord(TreeDataRecord * aParent, int aArea, int aId, int aOrderBy, const QString & aText, int aFileType,
                               int aEditNA, int aLoadNA, int aEditPropNA, int aDeleteNA, int aNewNA, int aViewNA, int aNewVerNA,
                               int aNewBlankNA, int aNewFromFileNA, int aNewFromOtherNA, const QString & aCode,
                               int aIdGroup, int aCanExists, int aHidden) :
    mParent(aParent),
    InitPar(Area), InitPar(Id), InitPar(OrderBy), InitPar(Text), InitPar(FileType),
    InitPar(EditNA), InitPar(LoadNA), InitPar(EditPropNA), InitPar(DeleteNA), InitPar(NewNA), InitPar(ViewNA), InitPar(NewVerNA),
    InitPar(NewBlankNA), InitPar(NewFromFileNA), InitPar(NewFromOtherNA), InitPar(Code),
    InitPar(IdGroup), InitPar(CanExists), InitPar(Hidden)
{

    if (!aParent) {
        mParentArea = 0;
        mParentId = 0;
    } else {
        mParentArea = aParent->mArea;
        mParentId = aParent->mId;
    }
}

TreeDataRecord::~TreeDataRecord() {
    qDeleteAll(mLeafs);
}

QString TreeDataRecord::FullName() const {
    if (mParent)
        return mParent->FullName() + "/" + mText;
    else
        return mText;

}

int TreeDataRecord::ActualFileType() const {
    if (mFileType != -2) return mFileType; // -2 means null
    if (mParent) return mParent->ActualFileType();
    return -1; // denied, can't create
}

int TreeDataRecord::ActualIdGroup() const {
    if (mIdGroup) return mIdGroup;
    if (mParent) return mParent->ActualIdGroup();
    return 0;
}

QString TreeDataRecord::ActualCode() const {
    if (!mCode.isEmpty()) {
        return mCode;
    } else if (mParent) {
        return mParent->ActualCode();
    } else return "";
}

bool TreeDataRecord::ActualShortData() const {
    if (mIdGroup == 2
            || mIdGroup == 9) {
        return true;
    } else if (mParent) {
        return mParent->ActualShortData();
    } else return false;
}

bool TreeDataRecord::ActualIsXref() const {
    if (mIdGroup == 2
            || mIdGroup == 9) {
        return true;
    } else if (mParent) {
        return mParent->ActualIsXref();
    } else return false;
}

TreeDataRecord * TreeDataRecord::Parent() {
    return mParent;
}

//-------------------------------------------------------------------------------------
TreeData::TreeData() {
    InitTreeData();
}

TreeData::~TreeData() {
    qDeleteAll(mLeafs);
}

void TreeData::CheckDataInternal() {
    if (mLeafs.isEmpty())
        InitTreeData();
}

void TreeData::InitTreeDataInternal(TreeDataRecord *aParent) {
    QSqlQuery query(db);

    query.prepare("select area, id, order_by, text, coalesce(file_type, -2) as file_type, edit_na, load_na, editprop_na, delete_na, new_na, view_na, newver_na,"
                  " newblank_na, newfromfile_na, newfromother_na, code, group_id, canexists, hidden"
                  " from v_treedata where coalesce(parent_area, -1) = :parent_area"
                  " and coalesce(parent_id, -1) = :parent_id"
                  " order by order_by");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Tree data", query);
    } else {
        if (!aParent) {
            query.bindValue(":parent_area", -1);
            query.bindValue(":parent_id", -1);
        } else {
            query.bindValue(":parent_area", aParent->Area());
            query.bindValue(":parent_id", aParent->Id());
        }
        if (!query.exec()) {
            gLogger->ShowSqlError("Tree data", query);
        } else {
            while (query.next()) {
                TreeDataRecord * lTreeDataRecord =
                        new TreeDataRecord(aParent, query.value("area").toInt(), query.value("id").toInt(), query.value("order_by").toInt(),
                                           query.value("text").toString(), query.value("file_type").toInt(), query.value("edit_na").toInt(),
                                           query.value("load_na").toInt(), query.value("editprop_na").toInt(), query.value("delete_na").toInt(),
                                           query.value("new_na").toInt(), query.value("view_na").toInt(), query.value("newver_na").toInt(),
                                           query.value("newblank_na").toInt(), query.value("newfromfile_na").toInt(), query.value("newfromother_na").toInt(),
                                           query.value("code").toString(), query.value("group_id").toInt(),
                                           query.value("canexists").toInt(), query.value("hidden").toInt());
                InitTreeDataInternal(lTreeDataRecord);
                if (!aParent) {
                    mLeafs.append(lTreeDataRecord);
                } else {
                    aParent->LeafsRef().append(lTreeDataRecord);
                }
            }
        }
    }
}

void TreeData::InitTreeData() {
    qDeleteAll(mLeafs);
    mLeafs.clear();

    InitTreeDataInternal();
}

QList<TreeDataRecord *> TreeData::Leafs() {
    CheckDataInternal();
    return mLeafs;
}

const QList<TreeDataRecord *> TreeData::LeafsConst() {
    CheckDataInternal();
    return mLeafs;
}

TreeDataRecord * TreeData::FindByIdInternal(int aArea, int aId, const QList<TreeDataRecord *> &aLeafs) const {
    TreeDataRecord *lTreeDataRecord;
    for (int i = 0; i < aLeafs.length(); i++) {
        if (aLeafs.at(i)->Area() == aArea
                && aLeafs.at(i)->Id() == aId)
            return aLeafs.at(i);
        if (lTreeDataRecord = FindByIdInternal(aArea, aId, aLeafs.at(i)->LeafsConst())) return lTreeDataRecord;
    }
    return NULL;
}


TreeDataRecord * TreeData::FindById(int aArea, int aId) {
    CheckDataInternal();
    return FindByIdInternal(aArea, aId, mLeafs);
}

TreeDataRecord * TreeData::FindByGroupIdInternal(int aGroupId, const QList<TreeDataRecord *> &aLeafs) const {
    TreeDataRecord *lTreeDataRecord;
    for (int i = 0; i < aLeafs.length(); i++) {
        if (aLeafs.at(i)->IdGroup() == aGroupId)
            return aLeafs.at(i);
        if (lTreeDataRecord = FindByGroupIdInternal(aGroupId, aLeafs.at(i)->LeafsConst())) return lTreeDataRecord;
    }
    return NULL;
}

TreeDataRecord * TreeData::FindByGroupId(int aGroupId) {
    CheckDataInternal();
    return FindByGroupIdInternal(aGroupId, mLeafs);
}

//-------------------------------------------------------------------------------------

FileType::FileType(int aId, const QString &aDescription, const QString &aExtension,
               const QString &aViewName, const QString &aEditName, const QString &aNewName,
               int aLoadMode, int aOpenMode, int aXrefMode, int aNotifyAcad, const QString &aFileMasks, const QString &aFileMasks_QT) :
    InitParRO(Id), InitParRO(Description), InitParRO(Extension),
    InitParRO(ViewName), InitParRO(EditName), InitParRO(NewName),
    InitParRO(LoadMode), InitParRO(OpenMode), InitParRO(XrefMode), InitParRO(NotifyAcad), InitParRO(FileMasks), InitParRO(FileMasks_QT)
{
}

FileTypeList::FileTypeList() {
    InitFileTypeList();
}

FileTypeList::~FileTypeList() {
    qDeleteAll(mFileTypes.values());
    mFileTypes.clear();
}

void FileTypeList::InitFileTypeList() {
    qDeleteAll(mFileTypes.values());
    mFileTypes.clear();

    // load from base
    QSqlQuery query("select id, description, xtension, view_name, edit_name, new_name,"
                    " loadmode, openmode, xrefmode, notify_acad, filemasks, filemasks_qt"
                    " from filetype order by id", db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("File types", query);
    } else {
        while (query.next()) {
            FileType * lFileType = new FileType(query.value("id").toInt(), query.value("description").toString(), query.value("xtension").toString(),
                                                query.value("view_name").toString(), query.value("edit_name").toString(), query.value("new_name").toString(),
                                                query.value("loadmode").toInt(), query.value("openmode").toInt(), query.value("xrefmode").toInt(),
                                                query.value("notify_acad").toInt(), query.value("filemasks").toString(), query.value("filemasks_qt").toString());
            mFileTypes[query.value("id").toInt()] = lFileType;
        }
    }
}

FileType * FileTypeList::FindById(int aId) {
    QMap<int, FileType *>::const_iterator itr = mFileTypes.find(aId);
    if (itr != mFileTypes.end() && itr.key() == aId) {
        return itr.value();
    }
    return NULL;
}
