#include "PlotListTree.h"

#include <QScrollBar>

#include "SelectColumnsDlg.h"

#include "../Logger/logger.h"


void PlotListTree::LoadFromSettings(QSettings &aSettings) {
    int i;
    QStringList lSelectedIdsStr;

    lSelectedIdsStr = aSettings.value("SelectedDocs").toString().split(';');
    mSelectedPlotIdsCommon.clear();
    for (i = 0; i < lSelectedIdsStr.length(); i++)
        mSelectedPlotIdsCommon.append(lSelectedIdsStr.at(i).toInt());

    lSelectedIdsStr = aSettings.value("SelectedLayouts").toString().split(';');
    mSelectedLayoutIds.clear();
    for (i = 0; i < lSelectedIdsStr.length(); i++)
        mSelectedLayoutIds.append(lSelectedIdsStr.at(i).toInt());

    lSelectedIdsStr = aSettings.value("SelectedHistory").toString().split(';');
    mSelectedHistoryIds.clear();
    for (i = 0; i < lSelectedIdsStr.length(); i++)
        mSelectedHistoryIds.append(lSelectedIdsStr.at(i).toInt());

    lSelectedIdsStr = aSettings.value("SelectedAddFiles").toString().split(';');
    mSelectedAddFilesIds.clear();
    for (i = 0; i < lSelectedIdsStr.length(); i++)
        mSelectedAddFilesIds.append(lSelectedIdsStr.at(i).toInt());

    lSelectedIdsStr = aSettings.value("ExpandedDocs").toString().split(';');
    mExpandedPlotIdsCommon.clear();
    for (i = 0; i < lSelectedIdsStr.length(); i++)
        mExpandedPlotIdsCommon.append(lSelectedIdsStr.at(i).toInt());

    mCurrentItemType = aSettings.value("CurrentItemType").toInt();
    mCurrentItemId = aSettings.value("CurrentItemId").toInt();
    mCurrentColumn = aSettings.value("CurrentColumn").toInt();
}

//void PlotListTree::OnPlotListBeforeUpdate(int aIdProject) {
//    int i;
//    QList<QTreeWidgetItem *> lSelected = selectedItems();

//    mSelectedPlotIds.clear();
//    mSelectedLayoutIds.clear();
//    mSelectedHistoryIds.clear();
//    mSelectedAddFilesIds.clear();
//    mExpandedPlotIds.clear();

//    for (i = 0; i < lSelected.length(); i++) {
//        if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()) {
//            if (!aIdProject || static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()->IdProject() == aIdProject) {
//                mSelectedPlotIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotConst()->Id());
//            }
//        } else {
//            if (!aIdProject
//                    || lSelected.at(i)->parent()
//                        && static_cast<PlotListTreeItem *>(lSelected.at(i)->parent())->PlotConst()
//                        && static_cast<PlotListTreeItem *>(lSelected.at(i)->parent())->PlotConst()->IdProject() == aIdProject) {
//                if (static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()) {
//                    mSelectedLayoutIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->DwgLayoutConst()->Id());
//                } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()) {
//                    mSelectedHistoryIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotHistoryConst()->Id());
//                } else if (static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotAddFileConst()) {
//                    mSelectedAddFilesIds.append(static_cast<PlotListTreeItem *>(lSelected.at(i))->PlotAddFileConst()->Id());
//                }
//            }
//        }
//    }

//    for (i = 0; i < topLevelItemCount(); i++) {
//        if (topLevelItem(i)->isExpanded()) {
//            mExpandedPlotIds.append(static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()->Id());
//        }
//    }

//    if (currentItem()) {
//        if (static_cast<PlotListTreeItem *>(currentItem())->PlotConst()) {
//            mCurrentItemType = 0;
//            mCurrentItemId = static_cast<PlotListTreeItem *>(currentItem())->PlotConst()->Id();
//        } else if (static_cast<PlotListTreeItem *>(currentItem())->DwgLayoutConst()) {
//            mCurrentItemType = GlobalSettings::DocumentTreeStruct::SLTLayouts;
//            mCurrentItemId = static_cast<PlotListTreeItem *>(currentItem())->DwgLayoutConst()->Id();
//        } else if (static_cast<PlotListTreeItem *>(currentItem())->PlotHistoryConst()) {
//            mCurrentItemType = GlobalSettings::DocumentTreeStruct::SLTHistory;
//            mCurrentItemId = static_cast<PlotListTreeItem *>(currentItem())->PlotHistoryConst()->Id();
//        } else if (static_cast<PlotListTreeItem *>(currentItem())->PlotAddFileConst()) {
//            mCurrentItemType = GlobalSettings::DocumentTreeStruct::SLTAddFiles;
//            mCurrentItemId = static_cast<PlotListTreeItem *>(currentItem())->PlotAddFileConst()->Id();
//        }
//        mCurrentColumn = currentColumn();
//    }

//    mDocTreeScrollPos = verticalScrollBar()->value();
//    mDocTreeScrollPosHoriz = horizontalScrollBar()->value();

//    for (i = topLevelItemCount() - 1; i>= 0; i--) {
//        if (!aIdProject) {
//            delete topLevelItem(i);
//        } else
//            if (static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()
//                    && static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()->IdProject() == aIdProject) {
//                delete topLevelItem(i);
//            } else {
//                if (topLevelItem(i)->parent()
//                        && static_cast<PlotListTreeItem *>(topLevelItem(i)->parent())->PlotConst()
//                        && static_cast<PlotListTreeItem *>(topLevelItem(i)->parent())->PlotConst()->IdProject() == aIdProject) {
//                    delete topLevelItem(i);
//                }
//            }
//    }

//}

//void PlotListTree::OnPlotListNeedUpdate(int aIdProject) {
//}

void PlotListTree::OnPlotBeforeUpdate(PlotData *aPlot, int aType) {
    // remove all child - it is unusable now
    //QMessageBox::critical(NULL, "OnPlotBeforeUpdate",  QString::number(aPlot->Id()) + ":" + QString::number(aType));
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst() == aPlot) {
            PlotListTreeItem *lMainItem = static_cast<PlotListTreeItem *>(topLevelItem(i));
            QList <int> lSelectedIds;
            for (int j = lMainItem->childCount() - 1; j >= 0; j--) {
                if (lMainItem->child(j)->isSelected()) {
                    switch (mSLT) {
                    case GlobalSettings::DocumentTreeStruct::SLTLayouts:
                        break;
                    case GlobalSettings::DocumentTreeStruct::SLTHistory:
                        lSelectedIds.append(static_cast<PlotListTreeItem *>(lMainItem->child(j))->PlotHistoryConst()->Id());
                        break;
                    }
                }
            }
            lMainItem->setData(1, Qt::UserRole + 0, QVariant::fromValue(lSelectedIds));
            lMainItem->setData(1, Qt::UserRole + 1, verticalScrollBar()->value());
            break;
        }
    }
    //    if (!aType
//            || aType == 1 && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts
//            || aType == 6 && mSLT == GlobalSettings::DocumentTreeStruct::SLTHistory) {
//        //QMessageBox::critical(this, QObject::tr("Project data"), "PlotListTree::OnPlotBeforeUpdate " + QString::number(aPlot->Id()));
//        for (int i = 0; i < topLevelItemCount(); i++) {
//            if (/*static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()
//                    && static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst()->Id() == aPlot->Id()*/
//                    static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst() == aPlot) {
//                PlotListTreeItem *lMainItem = static_cast<PlotListTreeItem *>(topLevelItem(i));
//                for (int j = lMainItem->childCount() - 1; j >= 0; j--) {
//                    delete lMainItem->child(j);
//                }
//            }
//        }
//    }
}

void PlotListTree::OnPlotNeedUpdate(PlotData *aPlot, int aType) {
    // if (!aType) - update all data and childs; if aType == somethingn then add list
    //QMessageBox::critical(NULL, "OnPlotNeedUpdate", QString::number(aPlot->Id()) + ":" + QString::number(aType));
    if (!aType
            || aType == 1 && mSLT == GlobalSettings::DocumentTreeStruct::SLTLayouts
            || aType == 6 && mSLT == GlobalSettings::DocumentTreeStruct::SLTHistory) {
        //QMessageBox::critical(this, QObject::tr("Project data"), "PlotListTree::OnPlotNeedUpdate " + QString::number(aPlot->Id()));
        bool lSignalsBlocked = blockSignals(true);
        for (int i = 0; i < topLevelItemCount(); i++) {
            if (static_cast<PlotListTreeItem *>(topLevelItem(i))->PlotConst() == aPlot) {

                PlotListTreeItem *lMainItem = static_cast<PlotListTreeItem *>(topLevelItem(i));
                lMainItem->ShowData(); // it is fast, do it anyway

                QList <int> lSelectedIds = lMainItem->data(1, Qt::UserRole + 0).value<QList <int>>();
                // update childs
                for (int j = lMainItem->childCount() - 1; j >= 0; j--) {
                    delete lMainItem->child(j);
                }

                switch (mSLT) {
                case GlobalSettings::DocumentTreeStruct::SLTLayouts:
                    PopulateLayoutsInternal(lMainItem, aPlot);
                    break;
//                case GlobalSettings::DocumentTreeStruct::SLTVersions:
//                    break;
                case GlobalSettings::DocumentTreeStruct::SLTHistory:
                    // history list
                    // it is - reiniting
                    if (lMainItem->PlotRef()->HistoryConst().length()) {
                        lMainItem->PlotRef()->SetIdDwgMax(lMainItem->PlotRef()->HistoryConst().at(0)->Id());
                        lMainItem->PlotRef()->SetDwgVersionMax(lMainItem->PlotRef()->HistoryConst().at(0)->Num());

                        for (int j = 0; j < lMainItem->PlotRef()->HistoryConst().length(); j++) {
                            PlotListTreeItem * lItem = new PlotListTreeItem(this, lMainItem->PlotRef()->HistoryConst().at(j));
                            lMainItem->addChild(lItem);
                            if (lSelectedIds.contains(lItem->PlotHistoryConst()->Id())) {
                                lItem->setSelected(true);
                            }
                        }
                    }
                    break;
                }

                verticalScrollBar()->setValue(lMainItem->data(1, Qt::UserRole + 1).toInt());

                lMainItem->setData(1, Qt::UserRole + 0, QVariant());
                lMainItem->setData(1, Qt::UserRole + 1, QVariant());
                break;
            }
        }
        blockSignals(lSignalsBlocked);
    }
}

void PlotListTree::SaveCurrentStateInternal() const {
    //gLogger->ShowErrorInList(NULL, QTime::currentTime().toString(), "SAVE STATE");

    if (mParentDlg) {
        QSettings settings;
        settings.beginGroup("Windows");
        settings.beginGroup(mParentDlg->metaObject()->className() + mParentDlg->AddToClassName());
        settings.beginGroup("DocTree");

        settings.setValue("Version", mVersionCurrent);
        settings.setValue("VerExtVisSLT" + QString::number(mSLT), mVerExtVisible);
        settings.setValue("VerDateVisSLT" + QString::number(mSLT), mVerDateVisible);
        settings.setValue("SentDateVisSLT" + QString::number(mSLT), mSentDateVisible);
        settings.setValue("SentUserVisSLT" + QString::number(mSLT), mSentUserVisible);
        settings.setValue("ComplectVisSLT" + QString::number(mSLT), mComplectVisible);
        settings.setValue("BlockNameVisSLT" + QString::number(mSLT), mBlockNameVisible);

        settings.setValue("SLT" + QString::number(mSLT), header()->saveState());

        settings.endGroup();
        settings.endGroup();
        settings.endGroup();
    }
}

void PlotListTree::OnSectionResizedTimer() {
    mInSaveTimer--;
    if (!mInSaveTimer) {
        SaveCurrentStateInternal();
    }
}

void PlotListTree::OnSectionResized(int logicalIndex, int oldSize, int newSize) {
    if (!mIgnoreSectionResize) {
        mInSaveTimer++;
        //gLogger->ShowErrorInList(NULL, QTime::currentTime().toString(), "OnSectionResized: " + QString::number(logicalIndex) + " - " + QString::number(oldSize) + " - " + QString::number(newSize));
        QTimer::singleShot(1500, this, SLOT(OnSectionResizedTimer()));
    }
}

void PlotListTree::OnSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex) {
    SaveCurrentStateInternal();
}

void PlotListTree::OnSortIndicatorChanged(int logicalIndex, Qt::SortOrder order) {
    SaveCurrentStateInternal();
}

void PlotListTree::OnSelectColumns(const QPoint &aPoint) {
    SelectColumnsDlg w(this);
    w.move(QCursor::pos());
    QList<int> lDis;
    lDis << mCols[colID]
         << mCols[colWorking]
         << mCols[colDeleteDate]
         << mCols[colDeleteUser]
         << mCols[colLayoutName]
         << mCols[colCode]
         << mCols[colSheet]
         << mCols[colNameBottom];

    w.SetHeaderView(header());
    w.SetDisabledIndexes(lDis);

    mIgnoreSectionResize = true;
    if (w.exec() == QDialog::Accepted) {
        if (gSettings->DocumentTree.AutoWidth) {
            for (int i = 0; i < columnCount(); i++) resizeColumnToContents(i);
        }

        mVerExtVisible = !header()->isSectionHidden(mCols[colVersionExt]);
        mVerDateVisible = !header()->isSectionHidden(mCols[colVersionDate]);
        mSentDateVisible = !header()->isSectionHidden(mCols[colSentDate]);
        mSentUserVisible = !header()->isSectionHidden(mCols[colSentBy]);
        mComplectVisible = !header()->isSectionHidden(mCols[colSection]);
        mBlockNameVisible = !header()->isSectionHidden(mCols[colBlockName]);

        SaveCurrentStateInternal();
    }
    mIgnoreSectionResize = false;
}
