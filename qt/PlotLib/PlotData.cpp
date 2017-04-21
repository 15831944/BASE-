#include "PlotData.h"

#include "DwgLayoutData.h"
#include "DwgData.h"

#include "../VProject/TreeData.h"
#include "../VProject/MainWindow.h"
#include "../VProject/AcadXchgDialog.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/WaitDlg.h"
#include "../VProject/FileUtils.h"

#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectTypeData.h"

PlotData::PlotData() :
    mId(0), mIdProject(0), mIdCommon(0), mTDArea(0), mTDId(0),
    mIdDwgMax(0), mDwgVersionMax(0),
    mWorking(0), mCancelled(0), mDeleted(0),
    mEditNA(0), mLoadNA(0), mEditPropNA(0), mDeleteNA(0), mViewNA(0), mNewVerNA(0)
{

}

PlotData::PlotData(int aId, int aIdProject, int aIdCommon, int aTDArea, int aTDId, int aIdDwgMax, int aDwgVersionMax,
                   int aWorking,
                   int aCancelled, const QDate &aCancelDate, const QString &aCancelUser,
                   int aDeleted, const QDate &aDeleteDate, const QString &aDeleteUser,
                   const QString &aVersionInt, const QString &aVersionExt, const QDate &aSentDate, const QString &aSentUser,
                   const QString &aSection, const QString &aStage, const QString &aCode, const QString &aSheet,
                   const QString &aExtension, const QString &aNameTop, const QString &aName, const QString &aBlockName,
                   const QDateTime &aCrDate, const QString &aCrUser,
                   const QDateTime &aEditDate, const QString &aEditUser,
                   qlonglong aDataLength, int aXrefsCnt,
                   int aEditNA, int aLoadNA, int aEditPropNA, int aDeleteNA, int aViewNA, int aNewVerNA,
                   const QString &aNotes) :
    InitParRO(Id), InitPar(IdProject), InitParRO(IdCommon), InitPar(TDArea), InitPar(TDId), InitParRO(IdDwgMax), InitParRO(DwgVersionMax),
    InitPar(Working),
    InitPar(Cancelled), InitParRO(CancelDate), InitParRO(CancelUser),
    InitPar(Deleted), InitParRO(DeleteDate), InitParRO(DeleteUser),
    InitPar(VersionInt), InitPar(VersionExt), InitPar(SentDate), InitPar(SentUser), InitPar(Section), InitPar(Stage), InitPar(Code), InitPar(Sheet),
    InitPar(Extension), InitPar(NameTop), InitPar(Name), InitPar(BlockName),
    InitParRO(CrDate), InitParRO(CrUser),
    InitParRO(EditDate), InitParRO(EditUser),
    InitParRO(DataLength), InitParRO(XrefsCnt),
    InitParRO(EditNA), InitParRO(LoadNA), InitParRO(EditPropNA), InitParRO(DeleteNA), InitParRO(ViewNA), InitParRO(NewVerNA),
    InitPar(Notes)
{
    mInited.append(0); // main data is inited
}

PlotData::PlotData(int aId) :
    InitParRO(Id)
{
    RefreshData();
}

PlotData::~PlotData() {
    qDeleteAll(mLayouts);
    qDeleteAll(mComments);
    qDeleteAll(mVersions);
    qDeleteAll(mHistory);
    qDeleteAll(mAddFiles);
}


PlotHistoryData * PlotData::GetHistoryById(int aIdHistory) {
    int i;
    InitGrpData(6);

    for (i = 0; i < mHistory.length(); i++)
        if (mHistory.at(i)->Id() == aIdHistory)
            return mHistory.at(i);

    return NULL;
}

PlotHistoryData * PlotData::GetHistoryByNum(int aHistoryNum) {
    int i;
    InitGrpData(6);

    for (i = 0; i < mHistory.length(); i++)
        if (mHistory.at(i)->Num() == aHistoryNum)
            return mHistory.at(i);

    return NULL;
}

void PlotData::RefreshData() {
    gProjects->EmitPlotBeforeUpdate(this, 0);

    // mark all data uninited
    while (!mInited.isEmpty()) mInited.removeAt(0);

    QSqlQuery queryPlot(db);

    queryPlot.prepare(QString() + "select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id id_dwg, b.version dwg_version,"
                      " a.working,"
                      " a.cancelled, a.cancdate, a.cancuser,"
                      " a.deleted, a.deldate, a.deluser,"
                      " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.stage, a.code, a.sheet_number, a.extension,"
                      " a.nametop, a.name, a.block_name,"
                      " a.crdate, a.cruser, a.edit_date, a.edit_user,"
                      + ((db.driverName()== "QPSQL")?" length(b.data) as data_length,":" dbms_lob.getlength(b.data) data_length,")
                      + " (select count(1) from v_xref2dwg where id_dwg_main = b.id) xrefs_cnt,"
                      " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
                      " a.comments"
                      " from (select * from v_plot_simple where id = :Id) a"
                      " left outer join v_dwg b on b.id_plot = a.id"
                      " where (b.version = (select max(version) from v_dwg where id_plot = a.id) or b.version is null)");

    if (queryPlot.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Document data"), queryPlot);
    } else {
        queryPlot.bindValue(":Id", mId);
        if (!queryPlot.exec()) {
            gLogger->ShowSqlError(QObject::tr("Document data"), queryPlot);
        } else {
            if (queryPlot.next()) {
                setIdProject(queryPlot.value("id_project").toInt(), true);
                mIdCommon = queryPlot.value("id_common").toInt();
                setTDArea(queryPlot.value("type_area").toInt(), true);
                setTDId(queryPlot.value("type").toInt(), true);
                mIdDwgMax = queryPlot.value("id_dwg").toULongLong();
                mDwgVersionMax = queryPlot.value("dwg_version").toInt();
                setWorking(queryPlot.value("working").toInt(), true);

                setCancelled(queryPlot.value("cancelled").toInt(), true);
                mCancelDate = queryPlot.value("cancdate").toDate();
                mCancelUser = queryPlot.value("cancuser").toString();

                setDeleted(queryPlot.value("deleted").toInt(), true);
                mDeleteDate = queryPlot.value("deldate").toDate();
                mDeleteUser = queryPlot.value("deluser").toString();

                setVersionInt(queryPlot.value("version").toString(), true);
                setVersionExt(queryPlot.value("version_ext").toString(), true);
                setSentDate(queryPlot.value("sentdate").toDate(), true);
                setSentUser(queryPlot.value("sentuser").toString(), true);
                setSection(queryPlot.value("section").toString(), true);
                setStage(queryPlot.value("stage").toString(), true);
                setCode(queryPlot.value("code").toString(), true);
                setSheet(queryPlot.value("sheet_number").toString(), true);
                setExtension(queryPlot.value("extension").toString(), true);

                setNameTop(queryPlot.value("nametop").toString(), true);
                setName(queryPlot.value("name").toString(), true);
                setBlockName(queryPlot.value("block_name").toString(), true);

                mCrDate = queryPlot.value("crdate").toDateTime();
                mCrUser = queryPlot.value("cruser").toString();

                mEditDate = queryPlot.value("edit_date").toDateTime();
                mEditUser = queryPlot.value("edit_user").toString();

                mDataLength = queryPlot.value("data_length").toLongLong();
                mXrefsCnt = queryPlot.value("xrefs_cnt").toInt();

                mEditNA = queryPlot.value("edit_na").toInt();
                mLoadNA = queryPlot.value("load_na").toInt();
                mEditPropNA = queryPlot.value("editprop_na").toInt();
                mDeleteNA = queryPlot.value("delete_na").toInt();
                mViewNA = queryPlot.value("view_na").toInt();
                mNewVerNA = queryPlot.value("newver_na").toInt();

                setNotes(queryPlot.value("comments").toString(), true);

                mInited.append(0); // mark main data is inited
            } else {
                gLogger->ShowError(QObject::tr("Document data"),
                                      QObject::tr("Data not found") + "\nv_plot_simple: id = " + QString::number(mId));
            }
        }
    }
    gProjects->EmitPlotNeedUpdate(this, 0);
}

bool PlotData::IsMainInited() {
    return mInited.contains(0);
}

const QString PlotData::CodeSheetConst() const {
    if (mSheet.isEmpty()) return mCode; else return mCode + "-" + mSheet;
}

void PlotData::InitIdDwgMax() {
    QSqlQuery query(
                "select id, version from v_dwg a where a.id_plot = " + QString::number(mId) +
                " and a.version = (select max(version) from v_dwg where id_plot = a.id_plot)", db);

    mIdDwgMax = 0;
    mDwgVersionMax = -1;

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Document data"), query);
    } else {
        if (query.next()) {
            mIdDwgMax = query.value("id").toULongLong();
            mDwgVersionMax = query.value("version").toInt();
        }
    }
}

void PlotData::SetIdDwgMax(quint64 aIdDwgMax) {
    mIdDwgMax = aIdDwgMax;
}

void PlotData::SetDwgVersionMax(int aDwgVersionMax) {
    mDwgVersionMax = aDwgVersionMax;
}

bool PlotData::MakeVersionActive() {
    bool res = false;

    QSqlQuery qUpdate(db);

    qUpdate.prepare("update v_plot_simple set working = decode(id, :id, 1, 0) where id_common = :id_common");
    if (qUpdate.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Document data"), qUpdate);
    } else {
        qUpdate.bindValue(":id", mId);
        qUpdate.bindValue(":id_common", mIdCommon);
        if (!qUpdate.exec()) {
            gLogger->ShowSqlError(QObject::tr("Document data"), qUpdate);
        } else {
            res = true;
            setWorking(1, true);
        }
    }

    return res;
}

// true - was undeleted; false - no changes
bool PlotData::Undelete() {
    if (!mDeleted) return false;

    mDeleted = 0;

    PlotData *lPlot = gProjects->FindByIdProject(mIdProject)->GetPlotByIdCommon(mIdCommon);
    if (lPlot) {
        mWorking = 0;
    } else {
        mWorking = 1;
    }

    return true;
}

void PlotData::SetPropWithCodeForming(PlotPropWithCode aPPWC, const ProjectTypeData *aProjectType, const QString &aOldVal, const QString &aNewVal,
                                     const QString &aCodeTemplate, QString &aCode) {
    QString lMark;

    switch (aPPWC) {
    case PPWCVersionExt:
        lMark = "%VER%";
        break;
    case PPWCComplect:
        lMark = "%SECT%";
        break;
    case PPWCStage:
        lMark = "%STAGE%";
        break;
    case PPWCSheet:
        lMark = "%SHEET%";
        break;
    default:
        return;
    }

    //QMessageBox::critical(NULL, "", aProjectType->NoNumTemplConst().left(aProjectType->NoNumTemplConst().indexOf("%")));
    if (aCode.startsWith(aProjectType->NoNumTemplConst().left(aProjectType->NoNumTemplConst().indexOf("%")))) return;
    /*if (!(aCode.count('-') == aCodeTemplate.count('-')
            || aCode.count('-') + 1 == aCodeTemplate.count('-'))) return;*/

    // it is usable for version
    if (aCodeTemplate.endsWith("-" + lMark)) {
        if (aCode.endsWith("-" + aOldVal)
                || aCode.count('-') >= aCodeTemplate.count('-')) {
            aCode.remove(QRegExp("-[^-]*$"));
            aCode += "-" + aNewVal;
        } else if (aCodeTemplate.count('-') == aCode.count('-') + 1) {
            // not enough minuses, add at end
            aCode += "-" + aNewVal;
        }
    } else if (aCodeTemplate.contains("-" + lMark + "-")) {
        // need also check that aOldVal is not empty (but what do if empty?)
        if (!aOldVal.isEmpty() && aCode.contains("-" + aOldVal + "-")) {
            aCode.replace("-" + aOldVal + "-", "-" + aNewVal + "-");
        } else {
            // need calc pos of minuses
            int lSectNum = aCodeTemplate.left(aCodeTemplate.indexOf("-" + lMark + "-") + 1).count('-');
            // was just for debug
            //QString lS1 = aNewCode.section('-', 0, lSectNum - 1, QString::SectionIncludeLeadingSep | QString::SectionIncludeTrailingSep);
            //QString lS2 = aNewCode.section('-', lSectNum + 1, -1, QString::SectionIncludeLeadingSep | QString::SectionIncludeTrailingSep);
            //QString lTemplatePart = lTemplate.left(lTemplate.indexOf("-" + aMark + "-") + 1);
            //QMessageBox::critical(NULL, "", lS1 + "|" + lS2);
            if (aOldVal.isEmpty()) {
                aCode = aCode.section('-', 0, lSectNum - 1, QString::SectionIncludeLeadingSep | QString::SectionIncludeTrailingSep)
                        + aNewVal
                        + aCode.section('-', lSectNum, -1, QString::SectionIncludeLeadingSep | QString::SectionIncludeTrailingSep);
            } else {
                aCode = aCode.section('-', 0, lSectNum - 1, QString::SectionIncludeLeadingSep | QString::SectionIncludeTrailingSep)
                        + aNewVal
                        + aCode.section('-', lSectNum + 1, -1, QString::SectionIncludeLeadingSep | QString::SectionIncludeTrailingSep);
            }
        }
    }
    while (aCode.indexOf("--") != -1) aCode.replace("--", "-");
}

// it is not static method for change properties and code
void PlotData::SetPropWithCodeForming(PlotPropWithCode aPPWC, const QString &aNewVal, QString &aCode) {
    QString lOldVal;

    switch (aPPWC) {
    case PPWCVersionExt:
        lOldVal = mVersionExt;
        break;
    case PPWCComplect:
        lOldVal = mSection;
        break;
    case PPWCStage:
        lOldVal = mStage;
        break;
    case PPWCSheet:
        lOldVal = mSheet;
        break;
    default:
        return;
    }

    SetPropWithCodeForming(aPPWC, lOldVal, aNewVal, aCode);
}

void PlotData::SetPropWithCodeForming(PlotPropWithCode aPPWC, const QString &aOldVal, const QString &aNewVal, QString &aCode) {
    ProjectData * lProject = gProjects->FindByIdProject(mIdProject);
    TreeDataRecord * lTreeData = gTreeData->FindById(mTDArea, mTDId);

    if (lProject) {
        ProjectData * lProjectMain = lProject;
        while (lProjectMain->Parent()
               && lProjectMain->Parent()->Type() == ProjectData::PDProject
               && lProjectMain->CodeTemplateConst().isEmpty()) {
            //if (lConstructNumber.isEmpty()) lConstructNumber = lProjectMain->ShortNumConst();
            lProjectMain = lProjectMain->Parent();
        }

        if (lTreeData &&  lTreeData->ActualIdGroup() > 1
                || lProjectMain->CodeTemplateConst().isEmpty()) {
            // it was simple code generating, need not regenerate
            // !!!!??? need regenerate too (it is simple)
        } else {
            //regen
            PlotData::SetPropWithCodeForming(aPPWC, lProjectMain->ProjectType(), aOldVal, aNewVal, lProjectMain->CodeTemplateConst(), aCode);
        }
    }
}

void PlotData::RegenCodeStatic(ProjectData *aProject, const TreeDataRecord *aTreeData,
                               const QString &aComplect, const QString &aStage, const QString &aVersionExt,
                               QStringList &aCodeList, QString &aCodeNew,
                               QStringList &aSheetList, QString &aSheet, bool aSheetSetted,
                               int aIgnoreIdCommon) {
    int i;
    QString lStr1;

    if (!aProject) return;

    qulonglong lMinNumber = aProject->ProjectType()->SheetStart();

    aCodeList.clear();
    aSheetList.clear();

    ProjectData * lProjectMain = aProject;
    while (lProjectMain->Parent()
           && lProjectMain->Parent()->Type() == ProjectData::PDProject
           && lProjectMain->CodeTemplateConst().isEmpty()) {
        lProjectMain = lProjectMain->Parent();
    }

    //const ProjectTypeData *lProjectType = gProjectTypes->GetById(lProjectMain->IdProjType());

    if (aTreeData &&  aTreeData->ActualIdGroup() > 1
            || lProjectMain->CodeTemplateConst().isEmpty()) {
        // it's simple code generating
        if (aTreeData) {
            lStr1 = aTreeData->ActualCode();
            lProjectMain->CodeTempleReplaceWithDataMain(lStr1);
            aProject->CodeTempleReplaceWithDataSub(lStr1);
            lStr1 = aProject->GenerateFixedCode(lStr1, 0, aIgnoreIdCommon);
        }
    } else {
        bool lEndsWithVer;

        lStr1 = lProjectMain->CodeTemplateConst();
        lProjectMain->CodeTempleReplaceWithDataMain(lStr1);
        aProject->CodeTempleReplaceWithDataSub(lStr1);

        lStr1.replace("%SECT%", aComplect);
        lStr1.replace("%STAGE%", aStage);

        if (lStr1.endsWith("-%VER%")) {
            lEndsWithVer = true;
            lStr1 = lStr1.left(lStr1.length() - QString("-%VER%").length());
        } else {
            lEndsWithVer = false;
            lStr1.replace("%VER%", aVersionExt);
        }

        while (lStr1.indexOf("--") != -1) lStr1.replace("--", "-");

        int lIndexStart, lIndexEnd;
        QList<qulonglong> lNums, lNewNums;
        QString lStart, lEnd;

        // no number string
        QString lNoNumberCode = lProjectMain->ProjectType()->NoNumTemplConst();
        lProjectMain->CodeTempleReplaceWithDataMain(lNoNumberCode);
        aProject->CodeTempleReplaceWithDataSub(lNoNumberCode);
        lNoNumberCode = aProject->GenerateFixedCode(lNoNumberCode, 0, aIgnoreIdCommon);
        aCodeList.append(lNoNumberCode);
        //

        // true - code change; false - sheet change
        bool aTrueForCodeChange = (lIndexStart = lStr1.indexOf("%N")) != -1;

        if (aTrueForCodeChange) {
            qulonglong lCurNum;

            lStart = lStr1.left(lIndexStart);
            lIndexEnd = lStr1.indexOf("N%") + 2;
            lEnd = lStr1.mid(lIndexEnd);

            for (i = 0; i < aProject->PlotListConst().length(); i++) {
                if (aProject->PlotListConst().at(i)->IdCommon() == aIgnoreIdCommon) continue;
                QString lCodeOther = aProject->PlotListConst().at(i)->CodeConst();
                if (lEndsWithVer) lCodeOther = lCodeOther.left(lCodeOther.lastIndexOf('-'));
                if (lCodeOther.left(lStart.length()) == lStart) {
                    if (lEnd.isEmpty()) {
                        lCurNum = lCodeOther.mid(lStart.length()).toULongLong();
                        if (lCurNum && !lNums.contains(lCurNum))
                            lNums.append(lCurNum);
                    } else if (lCodeOther.lastIndexOf(lEnd) == lCodeOther.length() - lEnd.length()){
                        lCurNum = lCodeOther.mid(lStart.length(), lCodeOther.lastIndexOf(lEnd) - lStart.length()).toULongLong();
                        if (lCurNum && !lNums.contains(lCurNum))
                            lNums.append(lCurNum);
                    }
                }
            }
        } else {
            QString lStrForCmp = lStr1;
            if (!aSheet.isEmpty())
                lStrForCmp.replace("%SHEET%", aSheet);
            else {
                QString lStrSheet = QString::number(lMinNumber);
                if (aProject->SheetDigits() > 0) {
                    while (lStrSheet.length() < aProject->SheetDigits()) lStrSheet.prepend('0');
                }
                lStrForCmp.replace("%SHEET%", lStrSheet);
            }
            //QMessageBox::critical(NULL, "", lStrForCmp);

            for (i = 0; i < aProject->PlotListConst().length(); i++) {
                if (aProject->PlotListConst().at(i)->IdCommon() == aIgnoreIdCommon) continue;
                qulonglong lSheet;
                QString lCodeOther = aProject->PlotListConst().at(i)->CodeConst();

                if (lEndsWithVer
                        && lCodeOther.endsWith("-" + aProject->PlotListConst().at(i)->VersionExtConst().trimmed())) {
                    lCodeOther = lCodeOther.left(lCodeOther.length() - aProject->PlotListConst().at(i)->VersionExtConst().trimmed().length() - 1);
                }

                if (lCodeOther == lStrForCmp) {
                    lSheet = aProject->PlotListConst().at(i)->SheetConst().toULongLong();
                    if (!lNums.contains(lSheet)) lNums.append(lSheet);
                }
            }
        }

        if (!lNums.isEmpty()) {
            std::sort(lNums.begin(), lNums.end());
            if (lNums.at(0) > lMinNumber) {
                // before first existing
                lNewNums.insert(0, lNums.at(0) - 1);
            }
            for (i = 0; i < lNums.length() - 1; i++) {
                if (i && lNums.at(i) - 1 != lNums.at(i - 1)) {
                    if (!lNewNums.contains(lNums.at(i) - 1)
                            && lNums.at(i) > lMinNumber)
                        lNewNums.append(lNums.at(i) - 1);
                }
                if (lNums.at(i) + 1 != lNums.at(i + 1)) {
                    if (!lNewNums.contains(lNums.at(i) + 1))
                        lNewNums.append(lNums.at(i) + 1);
                }
            }
            // it is skipped
            if (lNums.length() > 1)
                if (lNums.at(lNums.length() - 2) != lNums.at(lNums.length() - 1) - 1
                        && !lNewNums.contains(lNums.at(lNums.length() - 1) - 1))
                    lNewNums.append(lNums.at(lNums.length() - 1) - 1);
            // aftee last existing
            lNewNums.append(lNums.at(lNums.length() - 1) + 1);

            if (aTrueForCodeChange) {
                for (i = 0; i < lNewNums.length(); i++) {
                    QString lNewNumStr = QString::number(lNewNums.at(i));
                    while (lNewNumStr.length() < lIndexEnd - lIndexStart - 2) lNewNumStr.prepend('0');
                    lStr1 = lStart + lNewNumStr + lEnd;
                    if (lEndsWithVer) lStr1 += "-" + aVersionExt;
                    aCodeList.append(lStr1);
                }
                //aCodeListIndex = aCodeList.length() - 1;
            } else {
                QString lStrSheet;
                for (i = 0; i < lNewNums.length(); i++) {
                    lStrSheet = QString::number(lNewNums.at(i));
                    if (aProject->SheetDigits() > 0) {
                        while (lStrSheet.length() < aProject->SheetDigits()) lStrSheet.prepend('0');
                    }
                    aSheetList.append(lStrSheet);
                }
                if (aSheetSetted) {
                    lStrSheet = aSheet;
                } else {
                    /*if (aSheetList.length())
                        aSheetListIndex = aSheetList.length() - 1;*/
                    //ui->cbSheet->setCurrentText(lStrSheet);
                    aSheet = lStrSheet;
                }

                // form code
                lStr1.replace("%SHEET%", lStrSheet);
                if (lEndsWithVer) lStr1 += "-" + aVersionExt;
                aCodeList.append(lStr1);
            }
        } else {
            // lNums is empty
            if (aTrueForCodeChange) {
                lStr1 = lStart + QString::number(lMinNumber) + lEnd;
                if (lEndsWithVer) lStr1 += "-" + aVersionExt;
                aCodeList.append(lStr1);
                //aCodeListIndex = aCodeList.length() - 1;
            } else {
                QString lStrSheet;

                // add first number to list any way
                lStrSheet = QString::number(lProjectMain->ProjectType()->SheetStart());
                while (lStrSheet.length() < lProjectMain->ProjectType()->SheetLen()) lStrSheet.prepend('0');

                aSheetList.append(lStrSheet);

                if (aSheetSetted) {
                    lStrSheet = aSheet;
                } else {
                    /*if (aSheetList.length())
                        aSheetListIndex = aSheetList.length() - 1;*/
                    aSheet = lStrSheet;
                }

                // form code
                lStr1.replace("%SHEET%", lStrSheet);
                if (lEndsWithVer) lStr1 += "-" + aVersionExt;
                aCodeList.append(lStr1);
                //aCodeListIndex = aCodeList.length() - 1;
            }
        }
    }

    aCodeNew = lStr1;
}

int PlotData::CheckCodeDupStatic(ProjectData *aProject, const QString &aVersionExt, const QString &aCode, const QString &aSheet, bool aUseSheet,
                                  const QString &aNameTop, const QString &aNameBottom,
                                  int aIgnoreIdCommon) {
    ProjectData * lProjectMain = aProject;
    while (lProjectMain->Parent()
           && lProjectMain->Parent()->Type() == ProjectData::PDProject
           && lProjectMain->CodeTemplateConst().isEmpty()) {
        //if (lConstructNumber.isEmpty()) lConstructNumber = lProjectMain->ShortNumConst();
        lProjectMain = lProjectMain->Parent();
    }

    bool lCutVersion = false;
    QString lNewCodeForCompare = aCode.trimmed();

    if (!lProjectMain->CodeTemplateConst().isEmpty()
            && lProjectMain->CodeTemplateConst().endsWith("-%VER%")) {
        lCutVersion = true;
        if (lNewCodeForCompare.endsWith("-" + aVersionExt.trimmed())) {
            lNewCodeForCompare = lNewCodeForCompare.left(lNewCodeForCompare.length() - aVersionExt.trimmed().length() - 1);
        }
    }

    foreach (PlotData * lPlotAll, aProject->PlotListConst()) {
        QString lCodeForCompare = lPlotAll->CodeConst().trimmed();

        if (lCutVersion
                && lCodeForCompare.endsWith("-" + lPlotAll->VersionExtConst().trimmed())) {
            lCodeForCompare = lCodeForCompare.left(lCodeForCompare.length() - lPlotAll->VersionExtConst().trimmed().length() - 1);
        }

        if (lNewCodeForCompare == lCodeForCompare
                && (!aUseSheet
                    || aSheet.trimmed() == lPlotAll->SheetConst().trimmed())) {
            return 1;
        } else if (aNameTop.trimmed() == lPlotAll->NameTopConst().trimmed()
                   && aNameBottom.trimmed() == lPlotAll->NameConst().trimmed()) {
            return 2;
        }
    }
    return 0;
}

void PlotData::InitGrpData(int aGrpNum) {
    if (!mInited.contains(aGrpNum)) {
        switch (aGrpNum) {
        case 1:
            LoadLayouts();
            break;
        case 2:
            InitEditStatus();
            break;
        case 3:
            InitAcadVer();
            break;
        case 4:
            LoadComments();
            break;
        case 5:
            LoadVersions();
            break;
        case 6:
            ReinitHistory();
            break;
        case 7:
            LoadAddFiles();
            break;
        }
    }
}

void PlotData::LoadLayouts() {
    if (!mInited.contains(1)) mInited.append(1);

    qDeleteAll(mLayouts);
    mLayouts.clear();
    if (!mIdDwgMax) return;

    QSqlQuery query(db);
    query.prepare("select id, num, name, sheet, namebottom from v_dwg_layout where id_dwg = :id_dwg order by num");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("PlotData::LoadLayouts", query);
    } else {
        query.bindValue(":id_dwg", mIdDwgMax);
        if (!query.exec()) {
            gLogger->ShowSqlError("PlotData::LoadLayouts", query);
        } else {
            while (query.next()) {
                DwgLayoutData *lLayoutData = new DwgLayoutData(query.value("id").toULongLong(), mIdDwgMax, query.value("num").toInt(),
                                          query.value("name").toString(), query.value("sheet").toString(), query.value("namebottom").toString());
                mLayouts.append(lLayoutData);
            }
        }
    }
}

//void PlotData::UninitLayouts() {
//    mInited.removeAll(1);
//}

void PlotData::InitEditStatus() {

    if (!mInited.contains(2)) mInited.append(2);

    mESUser.clear();
    mES = PESError;

    QSqlQuery query(db);
    query.prepare("select session_id, coalesce(a.killed, 0) killed, starttime, endtime, username"
                  " from dwg_edit a, v_dwg b"
                  " where coalesce(a.id_dwgout, a.id_dwgin) = b.id"
                  " and b.id_plot = ?"
                  " and a.endtime is null"
                  " and session_id is not null"
                  " and a.\"WHEN\" ="
                  " (select max(\"WHEN\") from dwg_edit c, v_dwg d"
                  " where coalesce(c.id_dwgout, c.id_dwgin) = d.id"
                  " and d.id_plot = b.id_plot"
                  " and c.session_id is not null)");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Document data"), query);
    } else {
        query.addBindValue(mId);
        if (!query.exec()) {
            gLogger->ShowSqlError(QObject::tr("Document data"), query);
        } else {
            if (query.next()) {
                if (!query.value("starttime").isNull()) {
                    if (query.value("killed").toInt() == 1) {
                        mES = PESError;
                    } else {
                        mES = PESEditing;
                    }
                    mESUser = query.value("username").toString();
                } else {
                    mES = PESFree;
                }
            } else {
                mES = PESFree;
            }
        }
    }
}

static int InitAcadVerInternal(quint64 aIdDwg) {
    int lRes = 0;

    QSqlQuery query(db);
    query.prepare(QString("select ")
                  + ((db.driverName()== "QPSQL")?" substring(data from 1 for 6)":"utl_raw.cast_to_varchar2(dbms_lob.substr(data, 6))") + " from v_dwg where id = :IdDwg");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Document data"), query);
    } else {
        query.bindValue(":IdDwg", aIdDwg);
        if (!query.exec()) {
            gLogger->ShowSqlError(QObject::tr("Document data"), query);
        } else {
            if (query.next()) {
                lRes = gFileUtils->AcadVersion(query.value(0).toString());
            }
        }
    }

    return lRes;
}

void PlotData::InitAcadVer() {
    if (!mInited.contains(3)) mInited.append(3);

    mAcadVer = InitAcadVerInternal(mIdDwgMax);
}

QString PlotData::AcadVerStr() {
    InitGrpData(3);
    if (mAcadVer > 14) {
        return QString::number(mAcadVer);
    } else if (mAcadVer == 14) {
        return "R14";
    } else {
        return "";
    }
}

FileType * PlotData::ActualFileType() const {
    int lFileTypeInt = -1;
    TreeDataRecord * lTreeData = gTreeData->FindById(mTDArea, mTDId);
    if (lTreeData) lFileTypeInt = lTreeData->ActualFileType();

    // override by extension
    if (lFileTypeInt < 20
            || lFileTypeInt > 29) {
        if (mExtension.toLower() == "dwg") {
            lFileTypeInt = 0;
        } else if (mExtension.toLower() == "doc"
                   || mExtension.toLower() == "docx") {
            lFileTypeInt = 10;
        } else if (mExtension.toLower() == "xls"
                   || mExtension.toLower() == "xlsx") {
            lFileTypeInt = 11;
        } else if (mExtension.toLower() == "jpg"
                   || mExtension.toLower() == "jpeg"
                   || mExtension.toLower() == "png"
                   || mExtension.toLower() == "bmp"
                   || mExtension.toLower() == "tif"
                   || mExtension.toLower() == "tiff"
                   || mExtension.toLower() == "gif") {
            lFileTypeInt = 12;
        }
    }

    return gFileTypeList->FindById(lFileTypeInt);
}

int PlotData::FileType() const {
    TreeDataRecord * lTreeData = gTreeData->FindById(mTDArea, mTDId);
    if (lTreeData)
        return lTreeData->ActualFileType();
    else
        return -1;
}

bool PlotData::IsSent() {
    if (!mSentDate.isNull()) return true;

    //!!!??? return false for non-working version (it is not right, nut ok now)
    if (!mWorking) return false;

    InitGrpData(5);

    for (int i = 0; i < mVersions.length(); i++) {
        if (!mVersions.at(i)->mSentDate.isNull()) {
            return true;
        }
    }
    return false;
}

bool PlotData::IsPicture() const {
    return !mExtension.compare("jpg", Qt::CaseInsensitive)
            || !mExtension.compare("jpeg", Qt::CaseInsensitive)
            || !mExtension.compare("png", Qt::CaseInsensitive)
            || !mExtension.compare("bmp", Qt::CaseInsensitive)
            || !mExtension.compare("tif", Qt::CaseInsensitive)
            || !mExtension.compare("tiff", Qt::CaseInsensitive)
            || !mExtension.compare("gif", Qt::CaseInsensitive);
}

void PlotData::LoadComments() {

    if (!mInited.contains(4)) mInited.append(4);

    qDeleteAll(mComments);
    mComments.clear();

    QSqlQuery query(db);
    query.prepare("select id, comments_date, insert_user, comments from v_plot_comments a where id_plot = :IdPlot order by comments_date");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("PlotData::LoadComments", query);
    } else {
        query.addBindValue(mId);
        if (!query.exec()) {
            gLogger->ShowSqlError("PlotData::LoadComments", query);
        } else {
            while (query.next()) {
                mComments.append(new PlotCommentData(query.value("id").toInt(), query.value("comments_date").toDate(),
                                                     query.value("insert_user").toString(), query.value("comments").toString()));
            }
        }
    }
}

//void PlotData::UninitComments() {
//    mInited.removeAll(4);
//}

bool PlotData::LoadVersions() {
    if (!mInited.contains(5)) mInited.append(5);

    //gLogger->ShowErrorInList(NULL, QTime::currentTime().toString("hh:mm:ss"), QString::number(mId) + " - PlotData::LoadVersions() start", false);

    qDeleteAll(mVersions);
    mVersions.clear();

    // almost like in ProjectData::InitPlotList
    QSqlQuery queryPlot(db);

    //qint64 lStart = QDateTime::currentMSecsSinceEpoch();

    queryPlot.prepare(QString() + "select a.id, a.id_project, a.id_common, a.type_area, a.type, b.id id_dwg, b.version dwg_version,"
                      " a.working, a.cancelled, a.cancdate, a.cancuser,"
                      " a.deleted, a.deldate, a.deluser,"
                      " a.version, a.version_ext, a.sentdate, a.sentuser, a.section, a.stage, a.code, a.sheet_number, a.extension,"
                      " a.nametop, a.name, a.block_name,"
                      " a.crdate, a.cruser, a.edit_date, a.edit_user,"
                      + ((db.driverName()== "QPSQL")?" length(b.data) as data_length,":" dbms_lob.getlength(b.data) data_length,")
                      + " (select count(1) from v_xref2dwg where id_dwg_main = b.id) xrefs_cnt,"
                      " a.edit_na, a.load_na, a.editprop_na, a.delete_na, a.view_na, a.newver_na,"
                      " a.comments"
                      " from (select * from v_plot_simple where id_common = :IdCommon and id <> :Id and deleted = 0) a"
                      " left outer join v_dwg b on b.id_plot = a.id"
                      " where (b.version = (select max(version) from v_dwg where id_plot = a.id) or b.version is null)");

    if (queryPlot.lastError().isValid()) {
        gLogger->ShowSqlError("PlotData::LoadVersions", queryPlot);
        return false;
    } else {
        queryPlot.bindValue(":IdCommon", mIdCommon);
        queryPlot.bindValue(":Id", mId);
        if (!queryPlot.exec()) {
            gLogger->ShowSqlError("PlotData::LoadVersions", queryPlot);
            return false;
        } else {
            //qint64 lStart = QDateTime::currentMSecsSinceEpoch();
            while (queryPlot.next()) {
                PlotData *lPlotData = new PlotData(queryPlot.value("id").toInt(), queryPlot.value("id_project").toInt(), queryPlot.value("id_common").toInt(),
                                                   queryPlot.value("type_area").toInt(), queryPlot.value("type").toInt(),
                                                   queryPlot.value("id_dwg").toInt(), queryPlot.value("dwg_version").toInt(),
                                                   queryPlot.value("working").toInt(),
                                                   queryPlot.value("cancelled").toInt(), queryPlot.value("cancdate").toDate(), queryPlot.value("cancuser").toString(),
                                                   queryPlot.value("deleted").toInt(), queryPlot.value("deldate").toDate(), queryPlot.value("deluser").toString(),
                                                   queryPlot.value("version").toString(), queryPlot.value("version_ext").toString(),
                                                   queryPlot.value("sentdate").toDate(), queryPlot.value("sentuser").toString(),
                                                   queryPlot.value("section").toString(), queryPlot.value("stage").toString(),
                                                   queryPlot.value("code").toString(), queryPlot.value("sheet_number").toString(),
                                                   queryPlot.value("extension").toString(),
                                                   queryPlot.value("nametop").toString(), queryPlot.value("name").toString(),
                                                   queryPlot.value("block_name").toString(),
                                                   queryPlot.value("crdate").toDateTime(), queryPlot.value("cruser").toString(),
                                                   queryPlot.value("edit_date").toDateTime(), queryPlot.value("edit_user").toString(),
                                                   queryPlot.value("data_length").toLongLong(),
                                                   queryPlot.value("xrefs_cnt").toInt(),
                                                   queryPlot.value("edit_na").toInt(), queryPlot.value("load_na").toInt(), queryPlot.value("editprop_na").toInt(),
                                                   queryPlot.value("delete_na").toInt(), queryPlot.value("view_na").toInt(), queryPlot.value("newver_na").toInt(),
                                                   queryPlot.value("comments").toString());
                mVersions.append(lPlotData);
            }
            //QMessageBox::critical(NULL, QObject::tr("Document data"), QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStart)) / 1000));
        }
    }
    //QMessageBox::critical(NULL, QObject::tr("Document data"), QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStart)) / 1000));
    return true;
}

void PlotData::UninitVersions() {
    mInited.removeAll(5);
}

void PlotData::ReinitHistory() {
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "PlotData::ReinitHistory", QString::number(mId), false);

    gProjects->EmitPlotBeforeUpdate(this, 6);

    if (!mInited.contains(6)) mInited.append(6);

    qDeleteAll(mHistory);
    mHistory.clear();

    QSqlQuery queryHist(db);

    //qint64 lStart = QDateTime::currentMSecsSinceEpoch();

    queryHist.prepare(QString() + "select a.id, a.version, a.id_plot, b.type, b.username, b.\"WHEN\","
                      " b.computer, b.ip_addr, b.starttime, b.endtime, b.idlesec,"
                      " b.entchanged, b.entadded, b.entdeleted, b.savecount, b.lastsave, a.ftime,"
                      + ((db.driverName()== "QPSQL")?" length(a.data) as DataLength,":" dbms_lob.getlength(a.data) DataLength,")
                      + " a.neednotprocess as nnp,"
                      " (select count(1) from v_xref2dwg where id_dwg_main = a.id) as xrefs_cnt,"
                      " a.extension, b.file_name as working_file_name, b.saved_from as saved_from,"
                      " 0 as FileSize,"
                      " c.id_plot as from_id_plot, c.version as from_version"
                      " from v_dwg a join dwg_edit b on a.id = b.id_dwgout"
                      " left outer join v_dwg c on b.id_dwgin = c.id"
                      " where a.id_plot = :id_plot"
                      " union"
                      " select a.id, a.version, a.id_plot, 100, b.username, b.\"WHEN\","
                      " b.computer, b.ip_addr, null, null, null,"
                      " null, null, null, null, b.file_date as lastsave, a.ftime,"
                      + ((db.driverName()== "QPSQL")?" length(a.data) as DataLength,":" dbms_lob.getlength(a.data) DataLength,")
                      + " a.neednotprocess as nnp,"
                      " (select count(1) from v_xref2dwg where id_dwg_main = a.id) as xrefs_cnt,"
                      " a.extension, null as working_file_name, b.file_name as saved_from,"
                      " b.file_size as FileSize,"
                      " null as from_id_plot, null as from_version"
                      " from v_dwg a, dwg_file b where a.id = b.id_dwg and b.inout = 0"
                      " and a.id_plot = :id_plot"
                      " union"
                      " select a.id, a.version, a.id_plot, -1, null, null,"
                      " null, null, null, null, null,"
                      " null, null, null, null, null, a.ftime,"
                      + ((db.driverName()== "QPSQL")?" length(a.data) as DataLength,":" dbms_lob.getlength(a.data) DataLength,")
                      + " a.neednotprocess as nnp,"
                      " (select count(1) from v_xref2dwg where id_dwg_main = a.id) as xrefs_cnt,"
                      " a.extension, null, null,"
                      " 0 as FileSize,"
                      " null as from_id_plot, null as from_version"
                      " from v_dwg a"
                      " where a.id_plot = :id_plot"
                      " and not exists (select 1 from dwg_edit where id_dwgout = a.id)"
                      " and not exists (select 1 from dwg_file where inout = 0 and id_dwg = a.id)"
                      " order by 2 desc");

    if (queryHist.lastError().isValid()) {
        gLogger->ShowSqlError("PlotData::LoadHistory", queryHist);
    } else {
        queryHist.bindValue(":id_plot", mId); // yes, it works!
        if (!queryHist.exec()) {
            gLogger->ShowSqlError("PlotData::LoadHistory", queryHist);
        } else {
            //qint64 lStart = QDateTime::currentMSecsSinceEpoch();
            while (queryHist.next()) {
                PlotHistoryData * lPlotHistoryData =
                        new PlotHistoryData(queryHist.value("id").toInt(), queryHist.value("version").toInt(), queryHist.value("id_plot").toInt(), queryHist.value("type").toInt(),
                                            queryHist.value("username").toString(), queryHist.value("WHEN").toDateTime(),
                                            queryHist.value("computer").toString(), queryHist.value("ip_addr").toString(),
                                            queryHist.value("starttime").toDateTime(), queryHist.value("endtime").toDateTime(), queryHist.value("idlesec").toInt(),
                                            queryHist.value("entchanged").toInt(), queryHist.value("entadded").toInt(), queryHist.value("entdeleted").toInt(),
                                            queryHist.value("savecount").toInt(), queryHist.value("lastsave").toDateTime(), queryHist.value("ftime").toDateTime(),
                                            queryHist.value("DataLength").toLongLong(), queryHist.value("xrefs_cnt").toInt(),
                                            queryHist.value("extension").toString(),
                                            queryHist.value("working_file_name").toString(),
                                            queryHist.value("saved_from").toString(),
                                            queryHist.value("FileSize").toLongLong(), queryHist.value("nnp").toInt(),
                                            queryHist.value("from_id_plot").toInt(), queryHist.value("from_version").toInt());
                mHistory.append(lPlotHistoryData);
            }
            //QMessageBox::critical(NULL, QObject::tr("Document data"), QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStart)) / 1000));
        }
    }
    //QMessageBox::critical(NULL, QObject::tr("Document data"), QString::number(((float) (QDateTime::currentMSecsSinceEpoch() - lStart)) / 1000));
    gProjects->EmitPlotNeedUpdate(this, 6);
}

void PlotData::LoadAddFiles() {
    if (gSettings->DebugOutput & 1) gLogger->ShowErrorInList(gMainWindow, "PlotData::LoadAddFiles", QString::number(mId), false);

    gProjects->EmitPlotBeforeUpdate(this, 7);

    if (!mInited.contains(7)) mInited.append(7);

    qDeleteAll(mAddFiles);
    mAddFiles.clear();

    QSqlQuery queryAddFiles(db);

    queryAddFiles.prepare(QString() + "select /*+INDEX(b) */ a.id as id, b.id as id_lob, a.file_name as name, a.version as version, "
                          " a.id_xref as IdXref,"
                          + ((db.driverName()== "QPSQL")?" length(b.data) as DataLength,":" dbms_lob.getlength(b.data) DataLength,")
                          + " b.ftime as ftime"
                          " from xref a, v_dwg b where b.id = a.id_xref and a.deleted = 0 and a.id_dwg = :id_dwg");

    if (queryAddFiles.lastError().isValid()) {
        gLogger->ShowSqlError("PlotData::LoadAddFiles", queryAddFiles);
    } else {
//        if (mSelectedHistory) {
//            queryAddFiles.bindValue(":id_dwg1", mSelectedHistory->Id());
//        } else {
            InitIdDwgMax();
            queryAddFiles.bindValue(":id_dwg", mIdDwgMax);
//        }
        if (!queryAddFiles.exec()) {
            gLogger->ShowSqlError("PlotData::LoadAddFiles", queryAddFiles);
        } else {
            while (queryAddFiles.next()) {
                PlotAddFileData * lPlotAddFileData =
                        new PlotAddFileData(queryAddFiles.value("id").toInt(), queryAddFiles.value("id_lob").toInt(), queryAddFiles.value("name").toString(),
                                            queryAddFiles.value("version").toInt(), queryAddFiles.value("DataLength").toLongLong(),
                                            queryAddFiles.value("ftime").toDateTime());
                mAddFiles.append(lPlotAddFileData);
            }
        }
    }
    gProjects->EmitPlotNeedUpdate(this, 7);
}

void PlotData::UninitAddFiles() {
    mInited.removeAll(7);
}

bool PlotData::SaveData() {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;

    CheckParChange(IdProject, id_project)
    CheckParChange(TDArea, type_area)
    CheckParChange(TDId, type)

    CheckParChange(Working, working)
    CheckParChange(Cancelled, cancelled)
    CheckParChange(Deleted, deleted)

    CheckParChange(VersionInt, version)
    CheckParChange(VersionExt, version_ext)
    CheckParChange(SentDate, sentdate)
    CheckParChange(SentUser, sentuser)
    CheckParChange(Section, section)
    CheckParChange(Stage, stage)
    CheckParChange(Code, code)
    CheckParChange(Sheet, sheet_number)
    CheckParChange(Extension, extension)
    CheckParChange(NameTop, nametop)
    CheckParChange(Name, name)
    CheckParChange(BlockName, block_name)

    CheckParChange(Notes, comments)

    if (!lUpdStr.isEmpty()) {
        lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

        QSqlQuery qUpdate(db);

        qUpdate.prepare("update v_plot_simple set " + lUpdStr + " where id = ?");
        if (qUpdate.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Document data"), qUpdate);
        } else {
            for (int i = 0; i < lUpdValues.length(); i++) {
                qUpdate.addBindValue(lUpdValues.at(i));
            }
            qUpdate.addBindValue(mId);
            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(QObject::tr("Document data"), qUpdate);
            } else {
                res = true;
            }
        }
    } else {
        res = true;
    }
    return res;
}

void PlotData::RollbackEdit() {
    RollbackPar(IdProject)
    RollbackPar(TDArea)
    RollbackPar(TDId)

    RollbackPar(Working)
    RollbackPar(Cancelled)
    RollbackPar(Deleted)

    RollbackPar(VersionInt)
    RollbackPar(VersionExt)
    RollbackPar(SentDate)
    RollbackPar(SentUser)
    RollbackPar(Section)
    RollbackPar(Stage)
    RollbackPar(Code)
    RollbackPar(Sheet)
    RollbackPar(Extension)
    RollbackPar(NameTop)
    RollbackPar(Name)
    RollbackPar(BlockName)

    RollbackPar(Notes)
}

void PlotData::CommitEdit() {
    CommitPar(IdProject)
    CommitPar(TDArea)
    CommitPar(TDId)

    CommitPar(Working)
    CommitPar(Cancelled)
    CommitPar(Deleted)

    CommitPar(VersionInt)
    CommitPar(VersionExt)
    CommitPar(SentDate)
    CommitPar(SentUser)
    CommitPar(Section)
    CommitPar(Stage)
    CommitPar(Code)
    CommitPar(Sheet)
    CommitPar(Extension)
    CommitPar(NameTop)
    CommitPar(Name)
    CommitPar(BlockName)

    CommitPar(Notes)
}

//------------------------------------------------------------------------------------------------------------

PlotDwgData::PlotDwgData(int aIdDwg)
{
    mDwg = new DwgForSaveData(aIdDwg);
}

PlotDwgData::~PlotDwgData() {
    delete mDwg;
    qDeleteAll(mImages);
    qDeleteAll(mAddFiles);
}

//------------------------------------------------------------------------------------------------------------
PlotCommentData::PlotCommentData(int aId, const QDate &aDate, const QString &aUser, const QString &aComment) :
    InitParRO(Id), InitParRO(Date), InitParRO(User), InitParRO(Comment),
    mIsNew(false)
{
}

PlotCommentData::PlotCommentData(int aIdPlot, const QString &aComment) :
    mId(0), InitParRO(IdPlot), InitParRO(Comment), mIsNew(true)
{

}

// if true, the data is successfully saved (and can be added to list)
bool PlotCommentData::SaveData() {
    bool res = false;

    if (!mIsNew) {
        gLogger->ShowError(QObject::tr("Document comments"), QObject::tr("Document comments can't be changed"));
    } else {
        if (!mId) {
            if (!gOracle->GetSeqNextVal("plot_comments_id_seq", mId)) return false;
        }

        QSqlQuery qInsert(db);

        qInsert.prepare("insert into v_plot_comments (id, id_plot, comments) values (:id, :id_plot, :comments)");
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Document comments"), qInsert);
        } else {
            qInsert.bindValue(":id", mId);
            qInsert.bindValue(":id_plot", mIdPlot);
            qInsert.bindValue(":comments", mComment);
            if (!qInsert.exec()) {
                gLogger->ShowSqlError(QObject::tr("Document comments"), qInsert);
            } else {
                //res = RefreshData();
                res = true; // somitimes it is not required to refresh
                mIsNew = !res;
            }
        }
    }
    return res;
}

bool PlotCommentData::RefreshData() {
    bool res = false;

    if (mId) {
        QSqlQuery query(db);
        query.prepare("select comments_date, insert_user, comments from v_plot_comments a where id = :id");
        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Document comments"), query);
        } else {
            query.addBindValue(mId);
            if (!query.exec()) {
                gLogger->ShowSqlError(QObject::tr("Document comments"), query);
            } else {
                if (query.next()) {
                    mDate = query.value("comments_date").toDate();
                    mUser = query.value("insert_user").toString();
                    mComment = query.value("comments").toString();
                    res = true;
                } else {
                    gLogger->ShowError(QObject::tr("Document comments"), QObject::tr("Data not found") + "\nv_plot_comments: id = " + QString::number(mId));
                }
            }
        }
    }

    return res;
}

//------------------------------------------------------------------------------------------------------------
PlotHistoryData::PlotHistoryData(int aId, int aNum, int aIdPlot, int aType, const QString &aUser, const QDateTime &aWhen, const QString &aComputer, const QString &aIpAddr,
                                 const QDateTime &aStartTime, const QDateTime &aEndTime, int aIdleSec, int aEntChanged, int aEntAdded, int aEntDeleted,
                                 int aSaveCount, const QDateTime &aLastSave, const QDateTime &aFTime, qlonglong aDataLength, int aXrefsCnt,
                                 const QString &aExt, const QString &aWorkingFileName, const QString &aSavedFromFileName, qlonglong aFileSize, int aNeedNotProcess,
                                 int aFromIdPlot, int aFromVersion) :
    InitParRO(Id), InitParRO(Num), InitParRO(IdPlot), InitParRO(Type), InitParRO(User), InitParRO(When),
    InitParRO(Computer), InitParRO(IpAddr),
    InitParRO(StartTime), InitParRO(EndTime), InitParRO(IdleSec), InitParRO(EntChanged), InitParRO(EntAdded), InitParRO(EntDeleted),
    InitParRO(SaveCount), InitParRO(LastSave), InitParRO(FTime), InitParRO(DataLength), InitParRO(XrefsCnt),
    InitParRO(Ext), InitParRO(WorkingFileName), InitParRO(SavedFromFileName), InitParRO(FileSize), InitParRO(NeedNotProcess),
    InitParRO(FromIdPlot), InitParRO(FromVersion)
{

}

void PlotHistoryData::InitGrpData(int aGrpNum) {
    if (!mInited.contains(aGrpNum)) {
        switch (aGrpNum) {
        case 1:
            LoadAddFiles();
            break;
        case 2:
            InitAcadVer();
            break;
        }
    }
}

bool PlotHistoryData::IsPicture() const {
    return !mExt.compare("jpg", Qt::CaseInsensitive)
            || !mExt.compare("jpeg", Qt::CaseInsensitive)
            || !mExt.compare("png", Qt::CaseInsensitive)
            || !mExt.compare("bmp", Qt::CaseInsensitive)
            || !mExt.compare("tif", Qt::CaseInsensitive)
            || !mExt.compare("tiff", Qt::CaseInsensitive)
            || !mExt.compare("gif", Qt::CaseInsensitive);
}

QString PlotHistoryData::AcadVerStr() {
    InitGrpData(2);
    if (mAcadVer > 14) {
        return QString::number(mAcadVer);
    } else if (mAcadVer == 14) {
        return "R14";
    } else {
        return "";
    }
}

void PlotHistoryData::LoadAddFiles() {
    if (!mInited.contains(1)) mInited.append(1);

    qDeleteAll(mAddFiles);
    mAddFiles.clear();

    QSqlQuery queryAddFiles(db);

    queryAddFiles.prepare(QString() + "select /*+INDEX(b) */ a.id as id, b.id as id_lob, a.file_name as name, a.version as version,"
                          " a.id_xref as IdXref,"
                          + ((db.driverName()== "QPSQL")?" length(b.data) as DataLength,":" dbms_lob.getlength(b.data) DataLength,")
                          + " b.ftime as ftime"
                          " from xref a, v_dwg b where b.id = a.id_xref and a.deleted = 0 and a.id_dwg = :id_dwg");

    if (queryAddFiles.lastError().isValid()) {
        gLogger->ShowSqlError("PlotHistoryData::LoadAddFiles", queryAddFiles);
    } else {
        queryAddFiles.bindValue(":id_dwg", mId);
        if (!queryAddFiles.exec()) {
            gLogger->ShowSqlError("PlotHistoryData::LoadAddFiles", queryAddFiles);
        } else {
            while (queryAddFiles.next()) {
                PlotAddFileData * lPlotAddFileData =
                        new PlotAddFileData(queryAddFiles.value("id").toInt(), queryAddFiles.value("id_lob").toInt(), queryAddFiles.value("name").toString(),
                                            queryAddFiles.value("version").toInt(), queryAddFiles.value("DataLength").toLongLong(),
                                            queryAddFiles.value("ftime").toDateTime());
                mAddFiles.append(lPlotAddFileData);
            }
        }
    }
}

void PlotHistoryData::UninitAddFiles() {
    mInited.removeAll(1);
}

void PlotHistoryData::InitAcadVer() {
    if (!mInited.contains(2)) mInited.append(2);

    mAcadVer = InitAcadVerInternal(mId);
}

//------------------------------------------------------------------------------------------------------------
PlotAddFileData::PlotAddFileData(int aId, int aIdLob, const QString &aName, int aVersion, qlonglong aDataLength, const QDateTime &aFTime) :
    InitParRO(Id), InitParRO(IdLob), InitPar(Name), InitParRO(Version), InitParRO(DataLength), InitParRO(FTime)
{

}

bool PlotAddFileData::IsPicture() const {
    return mName.endsWith(".jpg", Qt::CaseInsensitive)
            || mName.endsWith(".jpeg", Qt::CaseInsensitive)
            || mName.endsWith(".png", Qt::CaseInsensitive)
            || mName.endsWith(".bmp", Qt::CaseInsensitive)
            || mName.endsWith(".tif", Qt::CaseInsensitive)
            || mName.endsWith(".tiff", Qt::CaseInsensitive)
            || mName.endsWith(".gif", Qt::CaseInsensitive);
}

//------------------------------------------------------------------------------------------------------------
PlotNamedListData::PlotNamedListData(int aId, const QString &aName, bool aGetList) :
    InitParRO(Id), InitPar(Name)
{
    if (!aGetList) return;

    mIdsCommon.clear();
    mIdsCommonOrig.clear();

    QSqlQuery query(db);

    //mIdPlots.clear();

    query.prepare("select id_common from v_pl_id where id_pl_name = ?");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Plot list data", query);
    } else {
        query.addBindValue(aId);
        if (!query.exec()) {
            gLogger->ShowSqlError("Plot list data", query);
        } else {
            while (query.next()) {
                mIdsCommon.append(query.value("id_common").toInt());
                mIdsCommonOrig.append(query.value("id_common").toInt());
            }
        }
    }
}

PlotNamedListData * PlotNamedListData::INSERT(int &aId, int aIdProject, const QString &aName) {
    PlotNamedListData * res = NULL;

    if (!aId && !gOracle->GetSeqNextVal("pl_name_id_seq", aId)) {
        return NULL;
    }

    QSqlQuery qInsert(db);

    qInsert.prepare("insert into v_pl_name (id, id_project, name)"
                    " values (:id, :id_project, :name)");
    if (qInsert.lastError().isValid()) {
        gLogger->ShowSqlError("Plot list data - prepare", qInsert);
    } else {
        qInsert.bindValue(":id", aId);
        qInsert.bindValue(":id_project", aIdProject);
        qInsert.bindValue(":name", aName);

        if (!qInsert.exec()) {
            gLogger->ShowSqlError("Plot list data - execute", qInsert);
        } else {
            res = new PlotNamedListData(aId, aName, false);
        }
    }

    return res;
}

bool PlotNamedListData::DODELETE(int aId) {
    bool res = false;

    QSqlQuery qDelete(db);

    qDelete.prepare("delete from v_pl_name where id = :id");
    if (qDelete.lastError().isValid()) {
        gLogger->ShowSqlError("Plot list data - prepare", qDelete);
    } else {
        qDelete.bindValue(":id", aId);

        if (!qDelete.exec()) {
            gLogger->ShowSqlError("Plot list data - execute", qDelete);
        } else {
            res = true;
        }
    }

    return res;
}

bool PlotNamedListData::SaveData() {
    QString lUpdStr;
    QList<QVariant> lUpdValues;

    bool res = false;

    CheckParChange(Name, name)
    if (!lUpdStr.isEmpty()) {
        lUpdStr = lUpdStr.left(lUpdStr.length() - 1);

        QSqlQuery qUpdate(db);

        qUpdate.prepare("update v_pl_name set " + lUpdStr + " where id = ?");
        if (qUpdate.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Named list data"), qUpdate);
        } else {
            for (int i = 0; i < lUpdValues.length(); i++) {
                qUpdate.addBindValue(lUpdValues.at(i));
            }
            qUpdate.addBindValue(mId);
            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(QObject::tr("Named list data"), qUpdate);
            } else {
                res = true;
            }
        }
    } else {
        res = true;
    }

    if (res) {
        if (mIdsCommon != mIdsCommonOrig) {
            //need update list
            bool lIsFirst = true;
            QSqlQuery query(db);
            foreach(int lIdNew, mIdsCommon) {
                if (!mIdsCommonOrig.contains(lIdNew)) {
                    if (lIsFirst) {
                        query.prepare("insert into v_pl_id (id_pl_name, id_common) values (:id_pl_name, :id_common)");
                        if (query.lastError().isValid()) {
                            gLogger->ShowSqlError("Plot list data insert list - prepare", query);
                            res = false;
                            break;
                        }
                        query.bindValue(":id_pl_name", mId);

                        lIsFirst = false;
                    }
                    query.bindValue(":id_common", lIdNew);
                    if (!query.exec()) {
                        gLogger->ShowSqlError("Plot list data insert list - execute", query);
                        res = false;
                        break;
                    }
                }
            }
            lIsFirst = true;
            foreach(int lIdOld, mIdsCommonOrig) {
                if (!mIdsCommon.contains(lIdOld)) {
                    if (lIsFirst) {
                        query.prepare("delete from v_pl_id where id_pl_name = :id_pl_name and id_common = :id_common");
                        if (query.lastError().isValid()) {
                            gLogger->ShowSqlError("Plot list data delete list - prepare", query);
                            res = false;
                            break;
                        }
                        query.bindValue(":id_pl_name", mId);

                        lIsFirst = false;
                    }
                    query.bindValue(":id_common", lIdOld);
                    if (!query.exec()) {
                        gLogger->ShowSqlError("Plot list data delete list - execute", query);
                        res = false;
                        break;
                    }
                }
            }
        }
    }

    return res;
}

void PlotNamedListData::RollbackEdit() {
    RollbackPar(Name)
    RollbackPar(IdsCommon)
}

void PlotNamedListData::CommitEdit() {
    CommitPar(Name)
    CommitPar(IdsCommon)
}

bool PlotData::SetPropWithVersions(bool aStartTransaction, bool aSaveData, PropType aType, QList<QVariant> aValues) {
    if (aStartTransaction)
        if (!db.transaction()) {
            gLogger->ShowSqlError(QObject::tr("Mark documents"), QObject::tr("Can't start transaction"), db);
            false;
        }

    bool lIsOk = true;
    int i;
    PlotData * lPlot, *lPlot2;
    if (mWorking || !aStartTransaction) {
        lPlot = this;
    } else {
        lPlot = gProjects->FindByIdProject(mIdProject)->GetPlotByIdCommon(mIdCommon);
    }

    if (aStartTransaction) {
        lIsOk = lPlot->LoadVersions();
    }

    if (lIsOk) {
        for (i = -1; i < lPlot->mVersions.length(); i++) {
            bool lNeedSaveData = false;
            if (i == -1) {
                lPlot2 = lPlot;
            } else {
                lPlot2 = lPlot->mVersions.at(i);
            }
            switch (aType) {
            case MATIdProject:
                if (lPlot2->IdProject() != aValues.at(0).toInt()) {
                    lPlot2->setIdProject(aValues.at(0).toInt());
                    lNeedSaveData = true;
                }
                break;
            case MATTreeType:
                if (lPlot2->TDArea() != aValues.at(0).toInt()
                        || lPlot2->TDId() != aValues.at(1).toInt()) {
                    lPlot2->setTDArea(aValues.at(0).toInt());
                    lPlot2->setTDId(aValues.at(1).toInt());
                    lNeedSaveData = true;
                }
                break;
            case MATComplect:
                if (lPlot2->SectionConst().trimmed() != aValues.at(0).toString()) {
                    lPlot2->SectionRef() = (aValues.at(0).toString());
                    lNeedSaveData = true;
                }
                break;
//            case MATCode:
//                if (lPlot2->CodeConst().trimmed() != aValues.at(0).toString()) {
//                    lPlot2->CodeRef() = (aValues.at(0).toString());
//                    lNeedSaveData = true;
//                }
//                break;
//            case MATSheet:
//                if (lPlot2->SheetConst().trimmed() != aValues.at(0).toString()) {
//                    lPlot2->SheetRef() = (aValues.at(0).toString());
//                    lNeedSaveData = true;
//                }
//                break;
            case MATBlockName:
                if (lPlot2->BlockNameConst().trimmed() != aValues.at(0).toString()) {
                    lPlot2->BlockNameRef() = (aValues.at(0).toString());
                    lNeedSaveData = true;
                }
                break;
            case MATNameTop:
                if (lPlot2->NameTopConst().trimmed() != aValues.at(0).toString()) {
                    lPlot2->NameTopRef() = (aValues.at(0).toString());
                    lNeedSaveData = true;
                }
                break;
            case MATNameBottom:
                if (lPlot2->NameConst().trimmed() != aValues.at(0).toString()) {
                    lPlot2->NameRef() = (aValues.at(0).toString());
                    lNeedSaveData = true;
                }
                break;
            case MATCancelled:
                if (lPlot2->Cancelled() != aValues.at(0).toInt()) {
                    lPlot2->setCancelled(aValues.at(0).toInt());
                    lNeedSaveData = true;
                }
                break;
            case MATDeleted:
                if (lPlot2->Deleted() != aValues.at(0).toInt()) {
                    lPlot2->setDeleted(aValues.at(0).toInt());
                    lNeedSaveData = true;
                }
                break;
            }
            if (lNeedSaveData && aSaveData) {
                if (!(lIsOk = lPlot2->SaveData())) break;
            }
        }
    }

    if (aStartTransaction) {
        if (lIsOk) {
            if (db.commit()) {
                lPlot->CommitEdit();
                for (i = 0; i < lPlot->mVersions.length(); i++) {
                    lPlot->mVersions.at(i)->CommitEdit();
                }
            } else {
                gLogger->ShowSqlError(QObject::tr("Mark documents"), QObject::tr("Can't commit"), db);
                lIsOk = false;
            }
        }

        if (!lIsOk) {
            db.rollback();
            lPlot->RollbackEdit();
            for (i = 0; i < lPlot->mVersions.length(); i++) {
                lPlot->mVersions.at(i)->RollbackEdit();
            }
        }
    }

    return lIsOk;
}


bool PlotData::SaveDataWithVersions() {
    bool lIsOk = true;

    int i;
    PlotData * lPlot;

    for (i = -1; i < mVersions.length(); i++) {
        if (i == -1) {
            lPlot = this;
        } else {
            lPlot = mVersions.at(i);
        }
        if (!lPlot->SaveData()) {
            lIsOk = false;
            break;
        }
    }
    return lIsOk;
}

bool PlotData::INSERT(int &aId, int &aIdCommon, int aIdProject, int aTypeArea, int aType, const QString &aVersionInt, const QString &aVersionExt,
                      const QString &aComplect, const QString &aStage,
                      const QString &aCode, const QString &aSheetNumber, const QString &aNameTop, const QString &aName,
                      const QString &aBlockName, const QString &aComments) {

    if (!aId && !gOracle->GetSeqNextVal("plot_id_seq", aId)) {
        return false;
    }
    if (!aIdCommon && !gOracle->GetSeqNextVal("seq_plot_id_common", aIdCommon)) {
        return false;
    }
    bool res = false;

    QSqlQuery qInsert(db);
    qInsert.prepare("insert into v_plot_simple (id, id_common, id_project, type_area, type, working, version, version_ext,"
                    " section, stage, code, sheet_number, NameTop, Name, block_name, comments)"
                    "  values (:id, :id_common, :id_project, :type_area, :type, 1, :version, :version_ext,"
                    " :section, :stage, :code, :sheet_number, :NameTop, :Name, :block_name, :comments)");
    if (qInsert.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("New document"), "Insert into v_plot_simple - prepare", qInsert);
    } else {
        qInsert.bindValue(":id", aId);
        qInsert.bindValue(":id_common", aIdCommon);
        qInsert.bindValue(":id_project", aIdProject);
        qInsert.bindValue(":type_area", aTypeArea);
        qInsert.bindValue(":type", aType);
        qInsert.bindValue(":version", aVersionInt);
        qInsert.bindValue(":version_ext", aVersionExt);
        qInsert.bindValue(":section", aComplect);
        qInsert.bindValue(":stage", aStage);
        qInsert.bindValue(":code", aCode);
        qInsert.bindValue(":sheet_number", aSheetNumber);
        qInsert.bindValue(":NameTop", aNameTop);
        qInsert.bindValue(":Name", aName);
        qInsert.bindValue(":block_name", aBlockName);
        qInsert.bindValue(":comments", aComments);

        if (!qInsert.exec()) {
            gLogger->ShowSqlError(QObject::tr("New document"), "Insert into v_plot_simple - execute", qInsert);
        } else {
            res = true;
        }
    }
    return res;
}

// aIdDWGMax - used as previous ID from dwg for coping xrefs;
// aDWGMaxVersion - really dwg max version (0 for new documents or new versions)
bool PlotData::LOADFROMFILE(bool aIsFile, int aIdPlot, quint64 &aIdDwgMain, quint64 aIdDWGMax, int aDWGMaxVersion, const QFileInfo &aOrigFileInfo, qint64 aOrigFileSize,
                            const QString &aOrigFileHash, const QByteArray &aMainFileData, const QString &aProcessedFileHash,
                            const QList<CDwgLayout *> &aDwgLayouts, XchgFileDataList &aAddFileData,
                            bool aDoNotInsertDwgFile, QWidget *aWaitOwner) {
    bool lIsOk = false;
    aIdDwgMain = 0;

    // in DWG - hash for data field; in DWG_FILE - hash for original file; it is different for Autocad drawings
    if ((aIsFile && DwgData::INSERT(aIdDwgMain, aIdPlot, aDWGMaxVersion + 1, aOrigFileInfo.suffix(), aProcessedFileHash.isEmpty()?aOrigFileHash:aProcessedFileHash,
                                    0, (aOrigFileInfo.suffix().toLower() == "dwg")?aDwgLayouts.length():-1, aOrigFileInfo.lastModified(), &aMainFileData)
            || !aIsFile && DwgData::INSERT(aIdDwgMain, aIdPlot, aDWGMaxVersion + 1, "", aProcessedFileHash.isEmpty()?aOrigFileHash:aProcessedFileHash,
                                           0, -1, aOrigFileInfo.lastModified(), NULL))
            && (aDoNotInsertDwgFile
                || gOracle->InsertDwgFile(aIdDwgMain, false, aOrigFileInfo.filePath(), aOrigFileSize, aOrigFileInfo.lastModified(), aOrigFileHash))) {
        lIsOk = true;


        quint64 lFoundIdDwgXref;
        int lXrefVersion;
        bool lShaIsEqual;
        bool lIsAny = false;

        if (!aAddFileData.isEmpty()) {
            WaitDlg lWaitDlg(aWaitOwner);
            lWaitDlg.show();
            lWaitDlg.SetMessage(QObject::tr("Loading add. files..."));
            for (int i = 0; i < aAddFileData.length(); i++) {
                lWaitDlg.SetMessage(QObject::tr("Loading add. files ") + QString::number(i + 1) + "/" + QString::number(aAddFileData.length()) + "...");
                if (aIdDWGMax) {
                    if (!gOracle->FindXref(aIdDWGMax, aAddFileData.at(i)->FileInfoPrcdConst().fileName(), aAddFileData.at(i)->HashPrcdConst(), lFoundIdDwgXref, lXrefVersion, lShaIsEqual)) {
                        lIsOk = false;
                        break;
                    }
                } else {
                    lFoundIdDwgXref = 0;
                    lXrefVersion = 1;
                }

                quint64 lIdDwg = 0;

                if (lFoundIdDwgXref && lShaIsEqual) {
                    // not changed from previous version, copy it
                    lIdDwg = lFoundIdDwgXref;
                } else {
                    lIsAny = true;

                    if (!gOracle->FindDwgBySha256(aAddFileData.at(i)->HashPrcdConst(), lIdDwg)) {
                        lIsOk = false;
                        break;
                    }

                    if (!lIdDwg
                            && !DwgData::INSERT(lIdDwg, 0, 0, aAddFileData.at(i)->FileInfoOrigConst().suffix(), aAddFileData.at(i)->HashPrcdConst(),
                                                0, -1, aAddFileData.at(i)->FileInfoOrigConst().lastModified(), aAddFileData.at(i)->BinaryDataConst())) {
                        lIsOk = false;
                        break;
                    }
                }

                if (!gOracle->InsertXref(aIdDwgMain, aAddFileData.at(i)->FileInfoPrcdConst().fileName(), lIdDwg, lXrefVersion, aAddFileData.at(i)->Group())) {
                    lIsOk = false;
                    break;
                }

                aAddFileData.at(i)->SetIdDwg(lIdDwg);
            }
            gOracle->Clean();
        }

        if (!aIsFile
                && !lIsAny) {
            QMessageBox::critical(gMainWindow, QObject::tr("Loading document"), QObject::tr("No changes, nothing to load"));
            lIsOk = false; // rollback
        }

        if (lIsOk && !aDwgLayouts.isEmpty()) {
            foreach (CDwgLayout * lDwgLayout, aDwgLayouts) {
                if (!lDwgLayout->InsertToBase(aIdDwgMain)) {
                    lIsOk = false;
                    break;
                }
            }
            CDwgLayoutFactory1::GetInstance()->Clean();
        }

        // coping of xrefs used after calling this function
    }
    return lIsOk;
}

bool PlotData::LOADFROMFILESIMPLE(int aIdPlot, int aDWGMaxVersion, const QString &aFileName) {
    QFile file(aFileName);
    QFileInfo lFileInfo(file);

    if (file.open(QFile::ReadOnly)) {
        QByteArray lFileData(file.readAll());
        file.close();

        QCryptographicHash hash1(QCryptographicHash::Sha256);
        hash1.addData(lFileData);

        quint64 lIdDwg = 0;
        if (DwgData::INSERT(lIdDwg, aIdPlot, aDWGMaxVersion + 1, lFileInfo.suffix(), QString(hash1.result().toHex()).toUpper(), 0, -1, lFileInfo.lastModified(), &lFileData)
                && gOracle->InsertDwgFile(lIdDwg, false, lFileInfo.filePath(), lFileInfo.size(), lFileInfo.lastModified(), QString(hash1.result().toHex()).toUpper())) {
            return true;
        } else {
            return false;
        }
    } else {
        gLogger->LogError("PlotData::LOADFROMFILESIMPLE");
        gLogger->ShowError(QObject::tr("Load document - opening file"),
                           QObject::tr("Error opening file") + ":\r\n" + file.fileName() + "\r\n" + QObject::tr("Error") +": " + file.errorString());
        return false;
    }
}

bool PlotData::LOCKIDPLOT(int aIdPlot) {
    bool res = false;
    QSqlQuery qLock("select pp.GetLock(" + QString::number(aIdPlot) + ") from dual", db);

    if (qLock.lastError().isValid()) {
        gLogger->ShowError(QObject::tr("Locking document"), QObject::tr("Error locking document") + " "
                           + QString::number(aIdPlot) + "\r\n"
                           + qLock.lastError().text());
    } else {
        if (qLock.next() && !qLock.value(0).toInt()) {
            res = true;
        } else {
            gLogger->ShowError(QObject::tr("Locking document"), QObject::tr("Can't lock document") + " "
                               + QString::number(aIdPlot) + "\r\n"
                               + qLock.lastError().text());
        }
    }
    return res;
}

bool PlotData::STARTEDIT(qulonglong &aNewIdDwgEdit, int aIdPlot, int aIdDwgIn, const QString &aFilename) {
    bool res = false;
    if (!db.transaction()) {
        gLogger->ShowSqlError(QObject::tr("Starting editing"), QObject::tr("Can't start transaction"), db);
    } else {
        bool lIsOk = false;

        if (LOCKIDPLOT(aIdPlot)) {
            if (aNewIdDwgEdit
                    || gOracle->GetSeqNextVal("dwg_edit_id_seq", aNewIdDwgEdit)) {
                QSqlQuery qInsert(db);

                qInsert.prepare(QString("insert into dwg_edit(id, id_dwgin, session_id, file_name, starttime)"
                                " values(:id, :id_dwgin,")
                                + ((db.driverName() == "QPSQL")
                                ?" cast(pg_backend_pid() as varchar)"
                                :" dbms_session.unique_session_id")
                                + ", :file_name, current_timestamp)");

                if (qInsert.lastError().isValid()) {
                    gLogger->ShowSqlError(QObject::tr("Starting editing") + " - prepare dwg_edit", qInsert);
                } else {
                    qInsert.bindValue(":id", aNewIdDwgEdit);
                    qInsert.bindValue(":id_dwgin", aIdDwgIn);
                    qInsert.bindValue(":file_name", aFilename);
                    if (qInsert.exec()) {
                        lIsOk = true;
                    } else {
                        gLogger->ShowSqlError(QObject::tr("Starting editing") + " - execute dwg_edit", qInsert);
                    }
                }
            }
        }

        if (lIsOk) {
            if (db.commit()) {
                res = true;
            } else {
                gLogger->ShowSqlError(QObject::tr("Starting editing"), QObject::tr("Can't commit"), db);
                lIsOk = false;
            }
        }
        if (!lIsOk) {
            db.rollback();
        }
    }

    return res;
}

bool PlotData::ENDEDIT(qulonglong aIdDwgEdit) {
    bool res = false;
    QSqlQuery qUpdate(db);

    qUpdate.prepare("update dwg_edit set endtime = current_timestamp where id = :id");

    if (qUpdate.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Ending editing"), qUpdate);
    } else {
        qUpdate.bindValue(":id", aIdDwgEdit);
        if (qUpdate.exec()) {
            res = true;
        } else {
            gLogger->ShowSqlError(QObject::tr("Ending editing"), qUpdate);
        }
    }
    return res;
}
