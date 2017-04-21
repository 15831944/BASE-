#ifndef TREEDATA_H
#define TREEDATA_H

#include <QString>
#include <QDate>
#include <QList>
#include <QMap>
#include "CommonData.h"

#include "def_expimp.h"

class EXP_IMP TreeDataRecord
{
private:
    TreeDataRecord * mParent;

    DeclarePar(int, Area)
    DeclarePar(int, Id)
    DeclarePar(int, OrderBy)
    DefParStr(Text)
    DeclarePar(int, FileType) // some kind of trick - "-2" here means null ("0" & "-1" alreadey used - see TreeData, FileType on server)

    DeclarePar(int, ParentArea)
    DeclarePar(int, ParentId)

    // "NA" means not available
    DeclarePar(int, EditNA)
    DeclarePar(int, LoadNA)
    DeclarePar(int, EditPropNA)
    DeclarePar(int, DeleteNA)
    DeclarePar(int, NewNA)
    DeclarePar(int, ViewNA)
    DeclarePar(int, NewVerNA)

    DeclarePar(int, NewBlankNA)
    DeclarePar(int, NewFromFileNA)
    DeclarePar(int, NewFromOtherNA)

    //DefParStr(Comments)
    DefParStr(Code)

    DeclarePar(int, IdGroup)

    DeclarePar(int, CanExists) // documents can exists in than bench
    DeclarePar(int, Hidden) // hidden bench

    // nested tree
    DefParCompl(QList<TreeDataRecord *>, Leafs)
public:
    TreeDataRecord(TreeDataRecord * aParent, int aArea, int aId, int aOrderBy, const QString & aText, int aFileType,
                   int aEditNA, int aLoadNA, int aEditPropNA, int aDeleteNA, int aNewNA, int aViewNA, int aNewVerNA,
                   int aNewBlankNA, int aNewFromFileNA, int aNewFromOtherNA, const QString & aCode,
                   int aIdGroup, int aCanExists, int aHidden);

    ~TreeDataRecord();

    QString FullName() const;
    int ActualFileType() const;
    int ActualIdGroup() const;
    QString ActualCode() const;
    bool ActualShortData() const;
    bool ActualIsXref() const;

    TreeDataRecord * Parent();
};

class EXP_IMP TreeData
{
protected:
    QList<TreeDataRecord *> mLeafs;

    explicit TreeData();
    ~TreeData();

    void CheckDataInternal();
    void InitTreeDataInternal(TreeDataRecord *aParent = NULL);
    TreeDataRecord * FindByIdInternal(int aArea, int aId, const QList<TreeDataRecord *> &aLeafs) const;
    TreeDataRecord * FindByGroupIdInternal(int aGroupId, const QList<TreeDataRecord *> &aLeafs) const;
public:
    static TreeData * GetInstance() {
        static TreeData * lTreeData = NULL;
        if (!lTreeData) lTreeData = new TreeData();
        return lTreeData;
    }

    void InitTreeData();

    QList<TreeDataRecord *> Leafs();
    const QList<TreeDataRecord *> LeafsConst();

    TreeDataRecord * FindById(int aArea, int aId);
    TreeDataRecord * FindByGroupId(int aGroupId);
};

#define gTreeData TreeData::GetInstance()

//-------------------------------------------------------------------------------------
class EXP_IMP FileType
{
    DeclareParRO(int, Id)
    DefParStrRO(Description)
    DefParStrRO(Extension)

    DefParStrRO(ViewName)
    DefParStrRO(EditName)
    DefParStrRO(NewName)

    DeclareParRO(int, LoadMode)
    DeclareParRO(int, OpenMode)
    DeclareParRO(int, XrefMode)
    DeclareParRO(int, NotifyAcad)

    DefParStrRO(FileMasks)
    DefParStrRO(FileMasks_QT)
public:
    FileType(int aId, const QString &aDescription, const QString &aExtension,
                   const QString &aViewName, const QString &aEditName, const QString &aNewName,
                   int aLoadMode, int aOpenMode, int aXrefMode, int aNotifyAcad, const QString &aFileMasks, const QString &aFileMasks_QT);

};

class EXP_IMP FileTypeList
{
protected:
    QMap<int, FileType *> mFileTypes;

    explicit FileTypeList();
    ~FileTypeList();
public:
    static FileTypeList * GetInstance() {
        static FileTypeList * lFileTypeList = NULL;
        if (!lFileTypeList) {
            lFileTypeList = new FileTypeList();
            //qAddPostRoutine(ProjectList::clean);
        }
        return lFileTypeList;
    }

    void InitFileTypeList();

    FileType * FindById(int aId);
};

#define gFileTypeList FileTypeList::GetInstance()

#endif // TREEDATA_H
