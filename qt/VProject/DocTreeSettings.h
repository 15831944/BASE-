#ifndef DOCTREESETTINGS_H
#define DOCTREESETTINGS_H

#include <QDialog>
#include "GlobalSettings.h"

namespace Ui {
class DocTreeSettings;
}

class DocTreeSettings : public QDialog
{
    Q_OBJECT

protected:
    QColor mDocColor, mLayoutColor;
public:
    explicit DocTreeSettings(QWidget *parent = 0);
    ~DocTreeSettings();

    int TTExpandLevel() const;
    void SetTTExpandLevel(int aTTExpandLevel);

    bool TTFontPlusOne() const;
    void SetTTFontPlusOne(bool aTTFontPlusOne);

    bool TTFontBold() const;
    void SetTTFontBold(bool aTTFontBold);

    //-----------------
    int WindowTitleType() const;
    void SetWindowTitleType(int aWindowTitleType);

    bool OpenSingleDocument() const;
    void SetOpenSingleDocument(bool aOpenSingleDocument);

    bool ShowGridLines() const;
    void SetShowGridLines(bool aShowGridLines);

//    bool UniformRowHeights() const;
//    void SetUniformRowHeights(bool aUniformRowHeights);

    bool AutoWidth() const;
    void SetAutoWidth(bool aAutoWidth);

    bool DocFontPlusOne() const;
    void SetDocFontPlusOne(bool aDocFontPlusOne);

    bool DocFontBold() const;
    void SetDocFontBold(bool aDocFontBold);

    int AddRowHeight() const;
    void SetAddRowHeight(int aAddRowHeight);

    bool DragDrop() const;
    void SetDragDrop(bool aDragDrop);

    GlobalSettings::DocumentTreeStruct::DBLDOC OnDocDblClick() const;
    void SetOnDocDblClick(GlobalSettings::DocumentTreeStruct::DBLDOC aOnDocDblClick);

    GlobalSettings::DocumentTreeStruct::SLT SecondLevel() const;
    void SetSecondLevel(GlobalSettings::DocumentTreeStruct::SLT aSecondLevel);

    bool ExpandOnShow() const;
    void SetExpandOnShow(bool aExpandOnShow);

    void GetColors(bool &aUseDocColor, QColor &aDocColor, bool &aUseLayoutColor, QColor &aLayoutColor);
    void SetColors(bool aUseDocColor, const QColor &aDocColor, bool aUseLayoutColor, const QColor &aLayoutColor);

private slots:
    void on_pbDocument_clicked();

    void on_pbLayout_clicked();

private:
    Ui::DocTreeSettings *ui;
};

#endif // DOCTREESETTINGS_H
