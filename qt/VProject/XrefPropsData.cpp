#include "XrefPropsData.h"

XrefOnePropData::XrefOnePropData() {
    s1.mIdMain = 0;
    s1.mIdXref = 0;
    s1.mIdXrefPlot = 0;
    s1.mName[0] = 0;
    s1.mObjType = -1;

    s1.mNeedProcess = false; // doesn't matter, dummy

}

bool XrefOnePropData::operator==(const XrefOnePropData &other) const {
    if (s1.mIdMain != other.s1.mIdMain
            || s1.mIdXref != other.s1.mIdXref
            || s1.mIdXrefPlot != other.s1.mIdXrefPlot
            || s1.mObjType != other.s1.mObjType
            || wcscmp(s1.mName, other.s1.mName)
            || s1.mDisabled != other.s1.mDisabled) return false;
    switch (s1.mObjType) {
    case 0:
        return s1.mAllBlocksColor == other.s1.mAllBlocksColor
                && s1.mAllEntitiesColor == other.s1.mAllEntitiesColor
                && s1.mAllBlocksLineweight == other.s1.mAllBlocksLineweight
                && s1.mAllEntitiesLineweight == other.s1.mAllEntitiesLineweight
                && !wcscmp(s1.mLayer0Name, other.s1.mLayer0Name);
        break;
    case 1:
        return s1.mLayerPrintColor == other.s1.mLayerPrintColor
                && s1.mLayerEntitiesColor == other.s1.mLayerEntitiesColor
                && s1.mLayerEntitiesLineweight == other.s1.mLayerEntitiesLineweight
                && !wcscmp(s1.mLayerNewName, other.s1.mLayerNewName);
        break;
    default:
        return s1.mObjType == other.s1.mObjType;
    }
}

void XrefOnePropData::SetObjType(long aObjType) {
    s1.mObjType = aObjType;
    s1.mName[0] = 0;
    s1.mDisabled = 0;
    switch (aObjType) {
        case 0:
            s1.mAllBlocksColor = -1;
            s1.mAllEntitiesColor = -1;
            s1.mAllBlocksLineweight = -100;
            s1.mAllEntitiesLineweight = -100;
            s1.mLayer0Name[0] = 0;

            break;
        case 1:
            s1.mLayerPrintColor = -1;
            s1.mLayerEntitiesColor = -1;
            s1.mLayerEntitiesLineweight = -100;
            s1.mLayerNewName[0] = 0;

            break;
    }
}

bool XrefOnePropData::IsDefault() const {
    return s1.mObjType == -1
        || s1.mObjType == 0 && !s1.mDisabled
            && s1.mAllBlocksColor == -1 && s1.mAllEntitiesColor == -1
            && s1.mAllBlocksLineweight == -100 && s1.mAllEntitiesLineweight == -100
            && !wcslen(s1.mLayer0Name)
        || s1.mObjType == 1 && !s1.mDisabled
            && s1.mLayerPrintColor == -1 && s1.mLayerEntitiesColor == -1
            && s1.mLayerEntitiesLineweight == -100
            && !wcslen(s1.mLayerNewName);
}

QString XrefOnePropData::GetAbrv() const {
    if (IsDefault()) return "";
    if (s1.mObjType == 0 && s1.mDisabled) return "Disabled;";

    QString res;

    switch (s1.mObjType) {
    case 0:
        res += "<All>;";
        if (s1.mAllBlocksColor != -1) res += "AllBlockColors:" + QString::number(s1.mAllBlocksColor) + ";";
        if (s1.mAllEntitiesColor != -1) res += "AllEntitiesColor:" + QString::number(s1.mAllEntitiesColor) + ";";
        if (s1.mAllBlocksLineweight != -100) res += "AllBlocksLineweight:" + QString::number(s1.mAllBlocksLineweight) + ";";
        if (s1.mAllEntitiesLineweight != -100) res += "AllEntitiesLineweight:" + QString::number(s1.mAllEntitiesLineweight) + ";";
        if (s1.mLayer0Name[0] != 0) res += "Layer0Name:" + QString::fromWCharArray(s1.mLayer0Name) + ";";

        break;
    case 1:
        res += "<Layer:" + QString::fromWCharArray(s1.mName) + ">;";
        if (s1.mDisabled) res += "Disabled;";
        if (s1.mLayerPrintColor != -1) res += "LayerPrintColor:" + QString::number(s1.mLayerPrintColor) + ";";
        if (s1.mLayerEntitiesColor != -1) res += "LayerEntitiesColor:" + QString::number(s1.mLayerEntitiesColor) + ";";
        if (s1.mLayerEntitiesLineweight != -100) res += "LayerEntitiesLineweight:" + QString::number(s1.mLayerEntitiesLineweight) + ";";
        if (s1.mLayerNewName[0] != 0) res += "LayerNewName:" + QString::fromWCharArray(s1.mLayerNewName) + ";";
        break;
    }
    return res;
}
