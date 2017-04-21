#ifndef ENTERSAVENAME_H
#define ENTERSAVENAME_H

#include <QtWidgets/QDialog>

namespace Ui {
class EnterSaveName;
}

class EnterSaveName : public QDialog
{
    Q_OBJECT
    
public:
    explicit EnterSaveName(QWidget *parent = 0);
    ~EnterSaveName();
    
    void SetFilename(QString aFilename);
    void SetCode(QString aCode);
    void SetNameTop(QString aNameTop);
    void SetName(QString aName);

    QString Filename() const;
    QString Code() const;
    QString NameTop() const;
    QString Name() const;
private slots:
    void on_pbOK_clicked();

    void on_pbCancel_clicked();

private:
    Ui::EnterSaveName *ui;
};

#endif // ENTERSAVENAME_H
