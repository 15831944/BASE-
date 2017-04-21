#include "PlotNewDlg.h"
#include "ui_PlotNewDlg.h"

#include "DwgData.h"

#include "../ProjectLib/ProjectTypeData.h"

#include "../VProject/PlotListDlg.h"
#include "../VProject/typetreeselect.h"

#include "../VProject/MainWindow.h"

#include "../SaveLoadLib/LoadImagesDlg.h"

#include <QMenu>

#include <QFileInfo>
#include <QFileDialog>
#include <QProcess>
#include <QTextCodec>

PlotNewDlg::PlotNewDlg(ProjectData *aProject, const TreeDataRecord * aTreeData, const QString &aComplect, PlotData * aPlotDataFrom,
                       PlotHistoryData * aPlotHistoryDataFrom, QWidget *parent) :
    QFCDialog(parent, false), AcadXchgDialog(),
    ui(new Ui::PlotNewDlg),
    mIdPlot(0), mIdDwg(0),
    mProject(aProject), mTreeData(aTreeData), mFileType(NULL), mPlotDataFrom(aPlotDataFrom), mPlotHistoryDataFrom(aPlotHistoryDataFrom),
    mSheetSetted(false), mCodeSetted(false),
    mAddFilesDlg(NULL),
    mOrigFileSize(0), mExistsListMenu(NULL)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    QPalette lPalette = ui->leTypeText->palette();
    // required
    lPalette.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    ui->leTypeText->setPalette(lPalette);
    ui->leVersionInt->setPalette(lPalette);
    ui->leVersionExt->setPalette(lPalette);
    ui->cbCode->setPalette(lPalette);
    ui->teBottomName->setPalette(lPalette);

    // read only
    lPalette.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);
    ui->leFileType->setPalette(lPalette);

    if (mPlotDataFrom) {
        ui->teTopName->setPlainText(mPlotDataFrom->NameTopConst());
    }

    if (mProject) {
        ProjectData *lProjectMain = mProject;
        while (lProjectMain->Parent()
               && lProjectMain->Parent()->Type() == ProjectData::PDProject) {
            lProjectMain = lProjectMain->Parent();
        }
        ui->cbComplect->blockSignals(true);
        ui->cbComplect->addItems(mProject->ComplectListConst());
        ui->cbComplect->setCurrentText(aComplect);
        ui->cbComplect->blockSignals(false);

        ui->lblSheet->setVisible(mProject->SheetDigitsActual() != -1);
        ui->cbSheet->setVisible(mProject->SheetDigitsActual() != -1);

        ui->cbStage->blockSignals(true);
        const QStringList &lStages = lProjectMain->ProjectType()->StagesConst();
        if (!lStages.isEmpty()) {
            ui->cbStage->addItem("");
        }
        ui->cbStage->addItems(lStages);
        ui->cbStage->setEditable(lStages.isEmpty());
        ui->cbStage->setCurrentText(lProjectMain->StageConst());
        ui->cbStage->blockSignals(false);
    } else {
        ui->lblSheet->setVisible(true);
        ui->cbSheet->setVisible(true);
    }

    TreeDataChanged();
    OtherPlotDataChanged();

    ui->lblAcadVersion->setVisible(false);
    ui->pbAlreadyInBase->setVisible(false);
}

PlotNewDlg::~PlotNewDlg()
{
    if (mAddFilesDlg) {
        delete mAddFilesDlg;
        mAddFilesDlg = NULL;
    }
    delete ui;
    gOracle->Clean();
}

void PlotNewDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    // always start with blank
    ui->cbCreateFrom->setCurrentIndex(0);
    ui->stCreateParams->setCurrentIndex(0);
}

void PlotNewDlg::TreeDataChanged() {
    bool lBlockNameVisible = false;
    if (mTreeData) {
        if (mTreeData->ActualIdGroup() == 2 /* Xrefs */
                || mTreeData->ActualIdGroup() == 9 /* Input data */) {
            // for xrefs (maybe change it in the future, use other types too; maybe use one flag in table "treedata")
            QString lDate = QDate::currentDate().toString("dd.MM.yy");
            ui->leVersionInt->setText(lDate);
            ui->leVersionExt->setText(lDate);
        } else {
            ui->leVersionInt->setText("0");
            if (mProject) {
                ui->leVersionExt->setText(mProject->ProjectType()->VerStartConst());
            } else {
                ui->leVersionExt->setText("1");
            }
        }
        ui->leTypeText->setText(mTreeData->FullName());
        if (mTreeData->CanExists()) {
            if (mFileType = gFileTypeList->FindById(mTreeData->ActualFileType())) {
                ui->leFileType->setText(tr(mFileType->DescriptionConst().toStdString().c_str()));
                lBlockNameVisible = mFileType->Id() == 0;
                switch (mFileType->LoadMode()) {
                case 0:
                    ui->lblSelectFile->setText(tr("Select file for load"));
                    break;
                case 1:
                case 3: // don't know the difference; i think 3 is not needed
                    ui->lblSelectFile->setText(tr("Select directory for load"));
                    break;
                case 2:
                    ui->lblSelectFile->setText(tr("Select files for load"));
                    break;
                }

            } else
                ui->leFileType->setText("");
        } else {
            ui->leFileType->setText(tr("Can't exists"));
        }
    } else {
        ui->leTypeText->setText("");
        ui->leFileType->setText("");
    }
    ui->leBlockName->setVisible(lBlockNameVisible);
    ui->lblBlockName->setVisible(lBlockNameVisible);

    RegenCode();
    //!!!???
//    if (mProjectData) {
//        ProjectData * lProjectData = mProjectData;
//        while (lProjectData->Parent() && lProjectData->Parent()->Type() == ProjectData::PDProject) {
//            lProjectData = lProjectData->Parent();
//        }
//        if (lProjectData->CodeTemplateConst().isEmpty()) RegenCode();
//    }
}

void PlotNewDlg::OtherPlotDataChanged() {
    ui->cbHistNum->clear();

    if (mPlotDataFrom) {
        QString lName;
        ProjectData * lProjectData = gProjects->FindByIdProject(mPlotDataFrom->IdProject());

        if (lProjectData) {
            ui->leIdProjectFrom->setText(QString::number(lProjectData->Id()));
            ui->leProjNameFrom->setText(lProjectData->FullShortName());
        } else {
            ui->leIdProjectFrom->setText("");
            ui->leProjNameFrom->setText("");
        }

        ui->leIdPlotFrom->setText(QString::number(mPlotDataFrom->Id()));

        lName = mPlotDataFrom->NameTopConst().trimmed();
        if (!lName.isEmpty()) {
            if (lName.left(1) != ".") lName += ".";
            lName += " ";
        }
        lName += mPlotDataFrom->NameConst().trimmed();
        ui->lePlotNameFrom->setText(lName);

        ui->cbHistNum->addItem(tr("Last"));
        for (int i = 0; i < mPlotDataFrom->HistoryConst().length(); i++) {
            ui->cbHistNum->addItem(QString::number(mPlotDataFrom->HistoryConst().at(i)->Num()), QVariant::fromValue(mPlotDataFrom->HistoryConst().at(i)));
        }

        if (mPlotHistoryDataFrom) {
            ui->cbHistNum->setCurrentText(QString::number(mPlotHistoryDataFrom->Num()));
        } else {
            ui->cbHistNum->setCurrentIndex(0);
        }
    } else {
        ui->leIdProjectFrom->setText("");
        ui->leProjNameFrom->setText("");

        ui->leIdPlotFrom->setText("");
        ui->lePlotNameFrom->setText("");
    }
}

void PlotNewDlg::RegenCodeOld() {
    int i;
    QString lStr1;

    if (!mProject) return;

    qulonglong lMinNumber = mProject->ProjectType()->SheetStart();

    ProjectData * lProjectMain = mProject;
    while (lProjectMain->Parent()
           && lProjectMain->Parent()->Type() == ProjectData::PDProject
           && lProjectMain->CodeTemplateConst().isEmpty()) {
        //if (lConstructNumber.isEmpty()) lConstructNumber = lProjectMain->ShortNumConst();
        lProjectMain = lProjectMain->Parent();
    }

    ui->cbCode->blockSignals(true);

    //const ProjectTypeData *lProjectType = gProjectTypes->GetById(lProjectMain->IdProjType());

    if (mTreeData &&  mTreeData->ActualIdGroup() > 1
            || lProjectMain->CodeTemplateConst().isEmpty()) {
        // it's simple code generating
        if (mTreeData) {
            lStr1 = mTreeData->ActualCode();
            lProjectMain->CodeTempleReplaceWithDataMain(lStr1);
            mProject->CodeTempleReplaceWithDataSub(lStr1);
            lStr1 = mProject->GenerateFixedCode(lStr1, 0, -1);
        }
    } else {
        bool lEndsWithVer;

        lStr1 = lProjectMain->CodeTemplateConst();
        lProjectMain->CodeTempleReplaceWithDataMain(lStr1);
        mProject->CodeTempleReplaceWithDataSub(lStr1);

        lStr1.replace("%SECT%", ui->cbComplect->currentText());
        lStr1.replace("%STAGE%", ui->cbStage->currentText());

        if (lStr1.endsWith("-%VER%")) {
            lEndsWithVer = true;
            lStr1 = lStr1.left(lStr1.length() - QString("-%VER%").length());
        } else {
            lEndsWithVer = false;
            lStr1.replace("%VER%", ui->leVersionExt->text());
        }

        while (lStr1.indexOf("--") != -1) lStr1.replace("--", "-");

        int lIndexStart, lIndexEnd;
        QList<qulonglong> lNums, lNewNums;
        QString lStart, lEnd;

        ui->cbCode->clear();

        // no number string
        QString lNoNumberCode = lProjectMain->ProjectType()->NoNumTemplConst();
        lProjectMain->CodeTempleReplaceWithDataMain(lNoNumberCode);
        mProject->CodeTempleReplaceWithDataSub(lNoNumberCode);
        lNoNumberCode = mProject->GenerateFixedCode(lNoNumberCode, 0, -1);
        ui->cbCode->addItem(lNoNumberCode);
        //

        // true - code change; false - sheet change
        bool aTrueForCodeChange = (lIndexStart = lStr1.indexOf("%N")) != -1;

        if (aTrueForCodeChange) {
            qulonglong lCurNum;

            lStart = lStr1.left(lIndexStart);
            lIndexEnd = lStr1.indexOf("N%") + 2;
            lEnd = lStr1.mid(lIndexEnd);

            for (i = 0; i < mProject->PlotListConst().length(); i++) {
                QString lCodeOther = mProject->PlotListConst().at(i)->CodeConst();
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
            for (i = 0; i < mProject->PlotListConst().length(); i++) {
                qulonglong lSheet;
                const QString & lCodeOther = mProject->PlotListConst().at(i)->CodeConst();
                if (lCodeOther == lStr1) {
                    lSheet = mProject->PlotListConst().at(i)->SheetConst().toULongLong();
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
                    if (lEndsWithVer) lStr1 += "-" + ui->leVersionExt->text();
                    ui->cbCode->addItem(lStr1);
                }
                ui->cbCode->setCurrentIndex(ui->cbCode->count() - 1);
            } else {
                QString lStrSheet;
                for (i = 0; i < lNewNums.length(); i++) {
                    lStrSheet = QString::number(lNewNums.at(i));
                    if (mProject && mProject->SheetDigits() > 0) {
                        while (lStrSheet.length() < mProject->SheetDigits()) lStrSheet.prepend('0');
                    }
                    ui->cbSheet->addItem(lStrSheet);
                }
                if (mSheetSetted) {
                } else {
                    if (ui->cbSheet->count())
                        ui->cbSheet->setCurrentIndex(ui->cbSheet->count() - 1);
                    //ui->cbSheet->setCurrentText(lStrSheet);
                }
            }
        } else {
            // lNums is empty
            if (aTrueForCodeChange) {
                lStr1 = lStart + QString::number(lMinNumber) + lEnd;
                if (lEndsWithVer) lStr1 += "-" + ui->leVersionExt->text();
                ui->cbCode->addItem(lStr1);
                ui->cbCode->setCurrentIndex(ui->cbCode->count() - 1);
            } else {
                QString lStrSheet, lStrSheetSetted;

                if (mSheetSetted) {
                    lStrSheetSetted = ui->cbSheet->currentText();
                }

                // add first number to list any way
                lStrSheet = QString::number(lProjectMain->ProjectType()->SheetStart());
                while (lStrSheet.length() < lProjectMain->ProjectType()->SheetLen()) lStrSheet.prepend('0');

                ui->cbSheet->blockSignals(true);
                ui->cbSheet->clear();
                ui->cbSheet->addItem(lStrSheet);

                if (mSheetSetted) {
                    lStrSheet = lStrSheetSetted;
                    ui->cbSheet->setCurrentText(lStrSheet);
                } else {
                    if (ui->cbSheet->count())
                        ui->cbSheet->setCurrentIndex(ui->cbSheet->count() - 1);
                }
                ui->cbSheet->blockSignals(false);

                // form code
                lStr1.replace("%SHEET%", lStrSheet);
                if (lEndsWithVer) lStr1 += "-" + ui->leVersionExt->text();
                ui->cbCode->addItem(lStr1);
                ui->cbCode->setCurrentIndex(ui->cbCode->count() - 1);
            }
        }
    }

    if (mPrevCode != lStr1) {
        ui->cbCode->setCurrentText(lStr1);
        mPrevCode = lStr1;
        if (mCodeSetted) {
            QMessageBox::critical(this, tr("New document"), tr("Code was modified!"));
            mCodeSetted = false;
        }
    }

    ui->cbCode->blockSignals(false);
}

void PlotNewDlg::RegenCode() {
    QStringList lCodeList, lSheetList;
    QString lCodeNew, lSheet;

    lSheet = ui->cbSheet->currentText();

    PlotData::RegenCodeStatic(mProject, mTreeData, ui->cbComplect->currentText(), ui->cbStage->currentText(), ui->leVersionExt->text(),
                              lCodeList, lCodeNew, lSheetList, lSheet, mSheetSetted, -1);

    ui->cbCode->blockSignals(true);
    ui->cbCode->clear();
    ui->cbCode->addItems(lCodeList);
    // index
    if (!lCodeList.isEmpty()
            && lCodeList.at(lCodeList.length() - 1) == lCodeNew)
        ui->cbCode->setCurrentIndex(lCodeList.length() - 1);
    // text
    if (mPrevCode != lCodeNew) {
        ui->cbCode->setCurrentText(lCodeNew);
        mPrevCode = lCodeNew;
        if (mCodeSetted) {
            QMessageBox::critical(this, tr("New document"), tr("Code was modified!"));
            mCodeSetted = false;
        }
    }
    ui->cbCode->blockSignals(false);

    ui->cbSheet->blockSignals(true);
    ui->cbSheet->clear();
    ui->cbSheet->addItems(lSheetList);
    if (mSheetSetted) {
        ui->cbSheet->setCurrentText(lSheet);
    } else {
        if (ui->cbSheet->count())
            ui->cbSheet->setCurrentIndex(ui->cbSheet->count() - 1);
    }
    ui->cbSheet->blockSignals(false);
}

bool PlotNewDlg::nativeEvent(const QByteArray & eventType, void * message, long * result) {
    if (AcadXchgDialog::DoNativeEvent(eventType, message, result)) return true;
    return QFCDialog::nativeEvent(eventType, message, result);
}

void PlotNewDlg::on_tbTreeSel_clicked() {
    TypeTreeSelect dSel(mTreeData, this);

    if (dSel.exec() == QDialog::Accepted) {
        mTreeData = dSel.GetSelected();
        TreeDataChanged();
    }
}

void PlotNewDlg::Accept() {
    bool lDoAccept = false;

    if (!mProject) {
        QMessageBox::critical(this, tr("New document"), tr("Project must be specified!"));
        return;
    }

    if (!mTreeData) {
        QMessageBox::critical(this, tr("New document"), tr("Document type must be specified!"));
        ui->leTypeText->setFocus();
        on_tbTreeSel_clicked();
        return;
    }

    if (!mTreeData->CanExists()) {
        QMessageBox::critical(this, tr("New document"), tr("Document type is wrong, documents can't exist on that leaf!"));
        ui->leTypeText->setFocus();
        on_tbTreeSel_clicked();
        return;
    }

    // something went wrong
    if (!mFileType) {
        QMessageBox::critical(this, tr("New document"), tr("Document type not specified!"));
        ui->leTypeText->setFocus();
        on_tbTreeSel_clicked();
        return;
    }

    if (mFileType->Id() == -1) {
        QMessageBox::critical(this, tr("New document"), tr("Document type is wrong, documents can't exist on that leaf!"));
        ui->leTypeText->setFocus();
        on_tbTreeSel_clicked();
        return;
    }

    if (ui->leVersionInt->text().isEmpty()) {
        QMessageBox::critical(this, tr("New document"), tr("Version internal must be specified!"));
        ui->leVersionInt->setFocus();
        return;
    }

    if (ui->leVersionExt->text().isEmpty()) {
        QMessageBox::critical(this, tr("New document"), tr("Version for customer must be specified!"));
        ui->leVersionExt->setFocus();
        return;
    }

    if (mTreeData->ActualIdGroup() < 2
            && mProject->ProjectType()->VerLenFixed()
            && ui->leVersionExt->text().length() != mProject->ProjectType()->VerLen()) {
        QMessageBox::critical(this, tr("New document"), tr("Version length must be equal to") + " " + QString::number(mProject->ProjectType()->VerLen()) + "!");
        ui->leVersionExt->setFocus();
        return;
    }

    if (ui->cbCode->currentText().isEmpty()) {
        QMessageBox::critical(this, tr("New document"), tr("Code must be specified!"));
        ui->cbCode->setFocus();
        return;
    }

    if (ui->teBottomName->toPlainText().isEmpty()) {
        QMessageBox::critical(this, tr("New document"), tr("Bottom name must be specified!"));
        ui->teBottomName->setFocus();
        return;
    }

    if (ui->cbCreateFrom->currentIndex() == 2) {
        if (ui->leFilename->text().isEmpty()) {
            QMessageBox::critical(this, tr("New document"), tr("File name must be specified!"));
            ui->leFilename->setFocus();
            return;
        }
        if (!QFile::exists(ui->leFilename->text())) {
            QMessageBox::critical(this, tr("New document"), tr("File doesn't exist!"));
            ui->leFilename->setFocus();
            return;
        }

        if (mFiles.isEmpty()) {
            QMessageBox::critical(this, tr("New document"), tr("Can't open file!"));
            ui->leFilename->setFocus();
            return;
        }
    } else if (ui->cbCreateFrom->currentIndex() == 3) {
        if (!mPlotDataFrom) {
            QMessageBox::critical(this, tr("New document"), tr("Original document must be specified!"));
            emit ui->tbPlotSel->click();
            return;
        }
    }


    int lDupRes = PlotData::CheckCodeDupStatic(mProject, ui->leVersionExt->text(), ui->cbCode->currentText(), ui->cbSheet->currentText(), ui->cbSheet->isVisible(),
                                               ui->teTopName->toPlainText(), ui->teBottomName->toPlainText(), -1);

    if (lDupRes == 1) {
        if (!ui->cbSheet->isVisible()) {
            QMessageBox::critical(this, tr("New document"), tr("Code is not unique!"));
        } else {
            QMessageBox::critical(this, tr("New document"), tr("Code and sheet number is not unique!"));
        }
        ui->cbCode->setFocus();
        return;
    } else if (lDupRes == 2) {
        QMessageBox::critical(this, tr("New document"), tr("Name is not unique!"));
        ui->teTopName->setFocus();
        return;
    }

//    ProjectData * lProjectMain = mProject;
//    while (lProjectMain->Parent()
//           && lProjectMain->Parent()->Type() == ProjectData::PDProject
//           && lProjectMain->CodeTemplateConst().isEmpty()) {
//        //if (lConstructNumber.isEmpty()) lConstructNumber = lProjectMain->ShortNumConst();
//        lProjectMain = lProjectMain->Parent();
//    }

//    bool lCutVersion = false;
//    QString lNewCodeForCompare = ui->cbCode->currentText().trimmed();

//    if (!lProjectMain->CodeTemplateConst().isEmpty()
//            && lProjectMain->CodeTemplateConst().endsWith("-%VER%")) {
//        lCutVersion = true;
//        if (lNewCodeForCompare.endsWith("-" + ui->leVersionExt->text().trimmed())) {
//            lNewCodeForCompare = lNewCodeForCompare.left(lNewCodeForCompare.length() - ui->leVersionExt->text().trimmed().length() - 1);
//        }
//    }

//    foreach (PlotData * lPlotAll, mProject->PlotListConst()) {
//        QString lCodeForCompare = lPlotAll->CodeConst().trimmed();

//        if (lCutVersion
//                && lCodeForCompare.endsWith("-" + lPlotAll->VersionExtConst())) {
//            lCodeForCompare = lCodeForCompare.left(lCodeForCompare.length() - lPlotAll->VersionExtConst().length() - 1);
//        }

//        if (lNewCodeForCompare == lCodeForCompare
//                && (/*mProject->SheetDigitsActual() == -1*/ !ui->cbSheet->isVisible()
//                    || ui->cbSheet->currentText().trimmed() == lPlotAll->SheetConst().trimmed())) {
//            if (/*mProject->SheetDigitsActual() == -1*/ !ui->cbSheet->isVisible()) {
//                QMessageBox::critical(this, tr("New document"), tr("Code is not unique!"));
//            } else {
//                QMessageBox::critical(this, tr("New document"), tr("Code and sheet number is not unique!"));
//            }
//            ui->cbCode->setFocus();
//            return;
//        } else if (ui->teTopName->toPlainText().trimmed() == lPlotAll->NameTopConst().trimmed()
//                   && ui->teBottomName->toPlainText().trimmed() == lPlotAll->NameConst().trimmed()) {
//            QMessageBox::critical(this, tr("New document"), tr("Name is not unique!"));
//            ui->teTopName->setFocus();
//            return;
//        }
//    }

    if (db.transaction()) {
        bool lIsOk = false;
        //bool lErr = false;

        XchgFileData *lXchgFileData = NULL;

        if (ui->cbCreateFrom->currentIndex() == 2) {
            lXchgFileData = mFiles.at(0);
        }
        quint64 lMainIdDwgFromFile;
        //QList<int> lAddIdDwg;
        int lIdDwgEdit;

        // need some checks here

        // end of checks, start inserting
        QString lBlockName;
        if (ui->cbCreateFrom->currentIndex() == 2
                && ui->leFilename->text().endsWith(".dwg", Qt::CaseInsensitive)) {
            lBlockName = ui->leFilename->text().left(ui->leFilename->text().length() - 4).mid(ui->leFilename->text().lastIndexOf('/') + 1);
        } else if (ui->leBlockName->isVisible()) {
            lBlockName = ui->leBlockName->text().trimmed();
        }
        int lIdCommonDummy = 0;
        if ((mIdPlot || gOracle->GetSeqNextVal("plot_id_seq", mIdPlot))
                && (mIdDwg || gOracle->GetSeqNextVal("dwg_id_seq", mIdDwg))
                && PlotData::INSERT(mIdPlot, lIdCommonDummy, mProject->Id(), mTreeData->Area(), mTreeData->Id(), ui->leVersionInt->text().trimmed(), ui->leVersionExt->text().trimmed(),
                                    ui->cbComplect->currentText().trimmed(), ui->cbStage->currentText().trimmed(), ui->cbCode->currentText().trimmed(), ui->cbSheet->currentText().trimmed(),
                                    ui->teTopName->toPlainText().trimmed(), ui->teBottomName->toPlainText().trimmed(), lBlockName, ui->teNote->toPlainText().trimmed())) {

            switch (ui->cbCreateFrom->currentIndex()) {
            case 0: // blank mother fucker
            {
                QCryptographicHash lHashBlank(QCryptographicHash::Sha256);
                if (DwgData::INSERT(mIdDwg, mIdPlot, 0, mFileType->ExtensionConst(), QString(lHashBlank.result().toHex()).toUpper(), 0, -1, QDateTime(), NULL)) {
                    QSqlQuery qInsert(db);
                    qInsert.prepare("insert into dwg_edit (id, id_dwgout) values (:id, :id_dwgout)");
                    if (qInsert.lastError().isValid()) {
                        gLogger->ShowSqlError(this, tr("New document"), "Insert into dwg_edit - prepare", qInsert);
                        //lErr = true;
                    } else {
                        if (gOracle->GetSeqNextVal("dwg_edit_id_seq", lIdDwgEdit)) {
                            qInsert.bindValue(":id", lIdDwgEdit);
                            qInsert.bindValue(":id_dwgout", mIdDwg);
                            if (!qInsert.exec()) {
                                gLogger->ShowSqlError(this, tr("New document"), "Insert into dwg_edit - execute", qInsert);
                                //lErr = true;
                            } else {
                                lIsOk = true;
                            }
                        }
                    }
                }
            }
                break;
            case 1: // template
                break;
            case 2: // file
            {
                bool b = false;

                if (lXchgFileData->FileInfoOrigConst().suffix().toLower() == "dwg") {
                    b = ProcessDwgsForLoad(lpClearAnnoScales | lpPurgeRegapps | lpExplodeAllProxies | lpRemoveAllProxies | lpAudit, 0, 0, 0, 0, "", "", winId());
                } else {
                    b = true;
                }

                if (b) {
                    if (PlotData::LOADFROMFILE(mFileType->LoadMode() != 3, mIdPlot, lMainIdDwgFromFile, 0, 0,
                                               lXchgFileData->FileInfoOrigConst(), mOrigFileSize, lXchgFileData->HashOrigConst(),
                                               *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst(),
                                               lXchgFileData->DwgLayoutsConst(),
                                               ui->pbAddFiles->isVisible()?mAddFilesDlg->FilesRef():lXchgFileData->AddFilesRef(),
                                               false, this)) {
                        lIsOk = true;
                    }
                }

                break;
            }
            case 3: // copy from other doc
                QSqlQuery qInsert(db);
                qInsert.prepare("insert into v_dwg (id, id_plot, extension, sha256, /*convert, */neednotprocess, nestedxrefmode, layout_cnt, ftime, InSubs, data)"
                                " (select :id, :id_plot, extension, sha256, /*convert, */neednotprocess, nestedxrefmode, layout_cnt, ftime, InSubs, data from v_dwg"
                                " where id = :id_dwg_orig)");
                if (qInsert.lastError().isValid()) {
                    gLogger->ShowSqlError(this, tr("New document"), "Insert into v_dwg - prepare", qInsert);
                    //lErr = true;
                } else {
                    int lIdDwgOrig;

                    qInsert.bindValue(":id", mIdDwg);
                    qInsert.bindValue(":id_plot", mIdPlot);
                    if (!ui->cbHistNum->currentIndex()) {
                        // last
                        mPlotDataFrom->InitIdDwgMax();
                        lIdDwgOrig = mPlotDataFrom->IdDwgMax();
                    } else {
                        lIdDwgOrig = ui->cbHistNum->currentData().value<PlotHistoryData *>()->Id();
                    }
                    qInsert.bindValue(":id_dwg_orig", lIdDwgOrig);
                    if (!qInsert.exec()) {
                        gLogger->ShowSqlError(this, tr("New document"), "Insert into v_dwg - execute", qInsert);
                        //lErr = true;
                    } else {
                        if (DwgData::CopyAllRefs(lIdDwgOrig, mIdDwg)) {
                            qInsert.prepare("insert into dwg_edit (id, id_dwgin, id_dwgout) values (:id, :id_dwgin, :id_dwgout)");
                            if (qInsert.lastError().isValid()) {
                                gLogger->ShowSqlError(this, tr("New document"), "Insert into dwg_edit - prepare", qInsert);
                                //lErr = true;
                            } else {
                                if (gOracle->GetSeqNextVal("dwg_edit_id_seq", lIdDwgEdit)) {
                                    qInsert.bindValue(":id", lIdDwgEdit);
                                    qInsert.bindValue(":id_dwgin", lIdDwgOrig);
                                    qInsert.bindValue(":id_dwgout", mIdDwg);
                                    if (!qInsert.exec()) {
                                        gLogger->ShowSqlError(this, tr("New document"), "Insert into dwg_edit - execute", qInsert);
                                        //lErr = true;
                                    } else {
                                        lIsOk = true;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
        }

        if (lIsOk) {
            if (!db.commit()) {
                gLogger->ShowSqlError(this, tr("New document"), tr("Can't commit"), db);
            } else {
                lDoAccept = true;
                switch (ui->cbCreateFrom->currentIndex()) {
                case 2: // from file
                    if (mFileType->LoadMode() != 3) {
                        gSettings->SaveToLocalCache(lMainIdDwgFromFile, *lXchgFileData->BinaryDataConst(), lXchgFileData->HashPrcdConst());
                    }
                    int i;
                    for (i = 0; i < lXchgFileData->AddFilesConst().length(); i++) {
                        gSettings->SaveToLocalCache(lXchgFileData->AddFilesConst().at(i)->IdDwg(),
                                                    *lXchgFileData->AddFilesConst().at(i)->BinaryDataConst(), lXchgFileData->AddFilesConst().at(i)->HashPrcdConst());
                    }
                    if (ui->pbAddFiles->isVisible()) {
                        for (i = 0; i < mAddFilesDlg->FilesConst().length(); i++) {
                            gSettings->SaveToLocalCache(mAddFilesDlg->FilesConst().at(i)->IdDwg(),
                                                        *mAddFilesDlg->FilesConst().at(i)->BinaryDataConst(), mAddFilesDlg->FilesConst().at(i)->HashPrcdConst());
                        }
                    }
                    break;
                case 0: // blank mother fucker
                case 3: // edit from other
                    if (mFileType->ExtensionConst().toLower() == "dwg") {
                        MainDataForCopyToAcad lDataForAcad(2);
                        lDataForAcad.ListRef().append(new RecordDataForCopyToAcad(RecordDataForCopyToAcad::eVE, mIdPlot, mIdDwg, lIdDwgEdit, false));
                        gSettings->DoOpenDwgNew(lDataForAcad);
                    } else {
                        gSettings->DoOpenNonDwg(lIdDwgEdit, 0, 1, "");
                    }
                    break;
                }
            }
        }
        if (!lIsOk){
            db.rollback();
        }
    } else {
        gLogger->ShowSqlError(this, tr("New document"), tr("Cant't start transaction"), db);
    }
    if (lDoAccept) accept();
}

void PlotNewDlg::on_tbPlotSel_clicked() {
    PlotListDlg w(PlotListDlg::DTShowSelectOne, mPlotDataFrom, NULL, this);

    w.SetProjectData(gProjects->FindByIdProject(ui->leIdProjectFrom->text().toInt()));

    if (w.exec() == QDialog::Accepted) {
        mPlotDataFrom = w.SelectedPlot();
        OtherPlotDataChanged();
    }
}

void PlotNewDlg::on_tbTreeSel_2_clicked() {
    QFileDialog dlg;

    ui->pbAddFiles->setVisible(false);
    if (mAddFilesDlg) {
        delete mAddFilesDlg;
        mAddFilesDlg = NULL;
    }

    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    if (mFileType) {
        if (!mFileType->FileMasks_QTConst().isEmpty())
            dlg.setNameFilters(mFileType->FileMasks_QTConst().split(';'));

        switch (mFileType->LoadMode()) {
        case 0:
            dlg.setFileMode(QFileDialog::ExistingFile);
            break;
        case 1:
        case 3: // don't know the difference; i think 3 is not needed, but 1 not used now
            dlg.setFileMode(QFileDialog::DirectoryOnly);

            // recommended - not work as usual
            //dlg.setFileMode(QFileDialog::Directory);
            //dlg.setOption(QFileDialog::ShowDirsOnly, true);
            break;
        case 2:
            //dlg.setFileMode(QFileDialog::ExistingFiles);
            dlg.setFileMode(QFileDialog::ExistingFile);
            break;
        }
    } else {
        dlg.setFileMode(QFileDialog::ExistingFile);
    }

    if (!ui->leFilename->text().isEmpty())
        dlg.selectFile(ui->leFilename->text());
    else
        dlg.setDirectory(gSettings->LoadFiles.LastDir);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.selectedFiles();
        if (files.length() == 1) {
            if (mFileType->LoadMode() != 3) {
                // file selected, save dir
                gSettings->LoadFiles.LastDir = files.at(0).left(files.at(0).lastIndexOf('/'));
            } else {
                // dir selected, save it
                gSettings->LoadFiles.LastDir = files.at(0);
            }

            ui->leFilename->setText(files.at(0));

            qDeleteAll(mFiles);
            mFiles.clear();

            mExistingIds.clear();
            ui->pbAlreadyInBase->setVisible(false);
            ui->lblAcadVersion->setVisible(false);

            XchgFileData *lXchgFileData = new XchgFileData(QFileInfo(files.at(0)));
            if (!gFileUtils->InitDataForLoad(mFileType->LoadMode() != 3, *lXchgFileData, mOrigFileSize)) {
                delete lXchgFileData;
                return;
            }

            if (mFileType->LoadMode() == 3) {
                if (lXchgFileData->AddFilesConst().isEmpty()) {
                    QMessageBox::critical(this, tr("Loading document"), tr("No files exist in this directory!"));
                    delete lXchgFileData;
                    return;
                }
            }

            if (!gOracle->CollectAlreadyLoaded(lXchgFileData->HashOrigConst(), mExistingIds)) {
                delete lXchgFileData;
                return;
            }

            mFiles.append(lXchgFileData);

            if (!mExistingIds.isEmpty()) {
                ui->pbAlreadyInBase->setVisible(true);
                if (mExistingIds.count() == 1) {
                    // single, just
                    ui->pbAlreadyInBase->setText(QString::number(mExistingIds.at(0).first.first) + "/" + QString::number(mExistingIds.at(0).first.second) + " - "
                                                 + mExistingIds.at(0).second);
                    ui->pbAlreadyInBase->setArrowType(Qt::NoArrow);

                } else {
                    // submenu
                    if (mExistsListMenu) delete mExistsListMenu;
                    mExistsListMenu = new QMenu(this);
                    foreach (tPairIntIntString mExistingId, mExistingIds) {
                        QAction *lAction;
                        lAction = mExistsListMenu->addAction(QString::number(mExistingId.first.first) + "/" + QString::number(mExistingId.first.second) + " - "
                                                             + mExistingId.second);
                        lAction->setCheckable(true);
                    }
                    mExistsListMenu->addSeparator();
                    ui->actionGo_to_selected->setEnabled(false);
                    mExistsListMenu->addAction(ui->actionGo_to_selected);

                    ui->pbAlreadyInBase->setText(tr("Already loaded"));
                    ui->pbAlreadyInBase->setArrowType(Qt::DownArrow);
                }

            } else {
                ui->pbAlreadyInBase->setVisible(false);
            }

            if (lXchgFileData->AcadVersionOrig()) {
                ui->lblAcadVersion->setText("AutoCAD: " + QString::number(lXchgFileData->AcadVersionOrig()));
                ui->lblAcadVersion->setVisible(true);
            } else {
                ui->lblAcadVersion->setVisible(false);
            }

            if (lXchgFileData->FileInfoOrigConst().fileName().endsWith(".xls", Qt::CaseInsensitive)
                    || lXchgFileData->FileInfoOrigConst().fileName().endsWith(".xlsx", Qt::CaseInsensitive)) {

                QString cmdLine, lTempFilename;
                cmdLine = QCoreApplication::applicationDirPath();
                cmdLine.resize(cmdLine.lastIndexOf(QChar('/')));
                cmdLine.resize(cmdLine.lastIndexOf(QChar('/')));
                lTempFilename = cmdLine + "/temp/data/v" + QDateTime::currentDateTime().toString("dd.MM.yy.hh.mm") + "/";

                QDir lDir(lTempFilename);
                lDir.mkpath(lTempFilename);

                lTempFilename += lXchgFileData->FileInfoOrigConst().fileName() + QDateTime::currentDateTime().toString("dd.MM.yy.hh.mm") + ".txt";

                cmdLine += "/common/vbs/ExcelGetLinks.vbs\" \"";
                cmdLine += lXchgFileData->FileInfoOrigConst().filePath() + "\"";
                cmdLine.prepend("\"");
                cmdLine += " \"" + lTempFilename + "\"";
                cmdLine.replace('/', '\\');

                cmdLine.prepend("cscript //NoLogo ");
                //QMessageBox::critical(NULL, "", cmdLine);
                //QMessageBox::critical(NULL, "", lTempFilename);

                QProcess proc1;
                proc1.start(cmdLine);
                if (!proc1.waitForStarted(-1)) {
                    gLogger->ShowError(this, tr("Excel script can't start"), proc1.errorString());
                } else {
                    if (!proc1.waitForFinished(-1)) {
                        gLogger->ShowError(this, tr("Excel script can't finish"), proc1.errorString());
                    } else {
                        //QMessageBox::critical(NULL, "", lTempFilename);
                        //QMessageBox::critical(NULL, "", cmdLine);

                        QTextCodec *lCodecForRead;
                        lCodecForRead = QTextCodec::codecForName("UTF-8");
                        QFile lFile(lTempFilename);
                        if (lFile.open(QFile::ReadOnly)) {
                            QString lLog = lCodecForRead->toUnicode(lFile.readAll());
                            lFile.close();
                            QStringList lLogList = lLog.split("\r\n");
                            lLogList.removeAll("");
                            if (!lLogList.isEmpty()) {
                                QString lPath = gSettings->LoadFiles.LastDir + "/";
                                lLogList.removeDuplicates();
                                for (int i = lLogList.length() - 1; i >= 0; i--) {
                                    if (lLogList.at(i).startsWith("http://")) {
                                        lLogList.removeAt(i);
                                    } else if (lLogList.at(i)[1] != ':'
                                            && !lLogList.at(i).startsWith("//")
                                            && !lLogList.at(i).startsWith("\\\\")) {
                                        lLogList[i].prepend(lPath);
                                    }

                                }

                                std::sort(lLogList.begin(), lLogList.end());
                                ui->pbAddFiles->setVisible(true);
                                mAddFilesDlg = new LoadImagesDlg(lLogList, this);
                            }
                        }
                    }
                }
                lDir.removeRecursively();
            }
        }
    }
}

void PlotNewDlg::on_cbComplect_editTextChanged(const QString &arg1) {
    RegenCode();
}

void PlotNewDlg::on_pbAlreadyInBase_clicked() {
    QList<int> lIndexes;

    if (mExistingIds.count() == 1
            && QMessageBox::question(this, tr("New document"), tr("Close this window and go to document")
                                     + " " + QString::number(mExistingIds.at(0).first.first) + "/" + QString::number(mExistingIds.at(0).first.second) + " - "
                                     + mExistingIds.at(0).second + "?") == QMessageBox::Yes) {
        lIndexes.append(0);
    } else {
        if (mExistsListMenu) {
            QAction *qActRes;
            do {
                qActRes = mExistsListMenu->exec(ui->pbAlreadyInBase->mapToGlobal(ui->pbAlreadyInBase->rect().bottomLeft()));

                bool b = false;
                foreach (QAction * lAction, mExistsListMenu->actions()) {
                    if (lAction->isChecked()) {
                        b = true;
                        break;
                    }
                }
                ui->actionGo_to_selected->setEnabled(b);

            } while (qActRes && qActRes != ui->actionGo_to_selected);
            if (qActRes == ui->actionGo_to_selected
                    && QMessageBox::question(this, tr("New document"), tr("Close this window and go to selected documents?")) == QMessageBox::Yes)
                for (int i = 0; i < mExistsListMenu->actions().count(); i++)
                    if (mExistsListMenu->actions().at(i)->isChecked())
                        lIndexes.append(i);
        }
    }
    foreach (int lIndex, lIndexes) {
        PlotData * lPlotGoto = gProjects->FindByIdPlot(mExistingIds.at(lIndex).first.first);
        if (lPlotGoto) {
            PlotHistoryData * lHistoryGoto = NULL;
            lPlotGoto->ReinitHistory(); // reinit, ya
            foreach (PlotHistoryData * lHistoryFound, lPlotGoto->HistoryConst()) {
                if (lHistoryFound->Num() == mExistingIds.at(lIndex).first.second) {
                    lHistoryGoto = lHistoryFound;
                    break;
                }
            }
            gMainWindow->ShowPlotList(gProjects->FindByIdProject(lPlotGoto->IdProject()), lPlotGoto, lHistoryGoto);
        } else {
            QMessageBox::critical(this, tr("Project data"), tr("Can't find document with ID = ") + QString::number(mExistingIds.at(lIndex).first.first));
        }
    }
    if (!lIndexes.isEmpty())
        done(QDialog::Rejected);
}

void PlotNewDlg::on_cbCreateFrom_currentIndexChanged(int index) {
    ui->stCreateParams->setCurrentIndex(index);
    if (index == 2
            && ui->leFilename->text().isEmpty()) {
        emit ui->tbTreeSel_2->click();
        //on_tbTreeSel_2_clicked();
    }
}

void PlotNewDlg::on_leIdPlotFrom_editingFinished() {
    mPlotDataFrom = gProjects->FindByIdPlot(ui->leIdPlotFrom->text().toULongLong());
    OtherPlotDataChanged();
}

void PlotNewDlg::on_cbStage_currentIndexChanged(int index) {
    // it is not duplicated with on_cbStage_editTextChanged
    RegenCode();
}

void PlotNewDlg::on_leVersionExt_textEdited(const QString &arg1) {
    RegenCode();
}

void PlotNewDlg::on_leVersionExt_editingFinished() {
    if (!mProject) return;

    if (mProject->ProjectType()->VerLenFixed()) {
        QString lVerExt = ui->leVersionExt->text();
        QString lVerExtOrig = lVerExt;
        while (lVerExt.length() <  mProject->ProjectType()->VerLen()) lVerExt.prepend('0');
        if (mProject->ProjectType()->VerLen())
            while (lVerExt.length() >  mProject->ProjectType()->VerLen() && lVerExt.at(0) == '0') lVerExt = lVerExt.mid(1);
        if (lVerExtOrig != lVerExt) {
            ui->leVersionExt->setText(lVerExt);
            RegenCode();
        }
    }
}

void PlotNewDlg::on_cbSheet_editTextChanged(const QString &arg1) {
    mSheetSetted = true;
    RegenCode();
}

void PlotNewDlg::on_cbCode_customContextMenuRequested(const QPoint &pos) {
    QMenu *lPopup = ui->cbCode->lineEdit()->createStandardContextMenu();

    lPopup->addSeparator();
    lPopup->addAction(ui->actionRegenerate_code);
    lPopup->exec(QCursor::pos());
}

void PlotNewDlg::on_cbCode_editTextChanged(const QString &arg1) {
    //QMessageBox::critical(NULL, "", "editTextChanged");
    if (mPrevCode != ui->cbCode->currentText()) {
        mPrevCode = ui->cbCode->currentText();
        mCodeSetted = true;
    }
}

//void PlotNewDlg::on_cbCode_currentTextChanged(const QString &arg1) {
//    QMessageBox::critical(NULL, "", "currentTextChanged");
//    mCodeSetted = false;
//}

void PlotNewDlg::on_cbStage_editTextChanged(const QString &arg1) {
    // it is fired when user text
    RegenCode();
}

void PlotNewDlg::on_pbAddFiles_clicked() {
    if (mAddFilesDlg) {
        mAddFilesDlg->exec();
    }
}
