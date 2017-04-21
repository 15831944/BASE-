#ifndef COMMONSETTINGSDLG_H
#define COMMONSETTINGSDLG_H

#include <QDialog>
#include <QTabWidget>

namespace Ui {
class CommonSettingsDlg;
}

class CommonSettingsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CommonSettingsDlg(QWidget *parent = 0);
    ~CommonSettingsDlg();

    QString VisualStyle() const;
    void SetVisualStyle(const QString &aStyle);

    bool ConfirmQuit() const;
    void SetConfirmQuit(bool aConfirmQuit);

    bool UseTabbedView() const;
    void SetUseTabbedView(bool aUseTabbedView);

    QTabWidget::TabPosition TabPos() const;
    void SetTabPos(QTabWidget::TabPosition aTabPos);

    bool SaveWinState() const;
    void SetSaveWinState(bool aSaveWinState);

    QFont Font() const;
    void SetFont(const QFont &lFont);

    int AddRowHeight() const;
    void SetAddRowHeight(int aAddRowHeight);

    bool ShowAfterCopy() const;
    void SetShowAfterCopy(bool aShowAfterCopy);

private slots:
    void on_cbUseTabbedView_toggled(bool checked);

    void on_pbSelectFont_clicked();

private:
    QFont mFont;
    Ui::CommonSettingsDlg *ui;
};

#endif // COMMONSETTINGSDLG_H
