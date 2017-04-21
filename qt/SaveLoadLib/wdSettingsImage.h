#ifndef WDSETTINGSIMAGE_H
#define WDSETTINGSIMAGE_H

#include "../VProject/wdSettings.h"

namespace Ui {
class wdSettingsImage;
}

class wdSettingsImage : public wdSettings
{
    Q_OBJECT

public:
    explicit wdSettingsImage(QWidget *parent = 0);
    virtual ~wdSettingsImage();

    virtual bool DoSave();

protected:
    virtual void showEvent(QShowEvent* event);
private slots:
    void on_gbResizeForPreview_toggled(bool arg1);

    void on_rbXY_toggled(bool checked);

    void on_rbInternal_toggled(bool checked);

    void on_rbExtDefault_toggled(bool checked);

    void on_rbExtSpecified_toggled(bool checked);

    void on_tbSelectViewer_clicked();

    void on_cbExtViewer_currentIndexChanged(int index);

    void on_rbMSPaint_toggled(bool checked);

    void on_rbEditorSpecified_toggled(bool checked);

    void on_tbSelectEditor_clicked();

private:
    bool mJustStarted;
    Ui::wdSettingsImage *ui;
};

#endif // WDSETTINGSIMAGE_H
