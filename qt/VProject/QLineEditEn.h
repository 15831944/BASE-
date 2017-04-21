#ifndef QLINEEDITEN_H
#define QLINEEDITEN_H

#include <QLineEdit>

class QLineEditEn : public QLineEdit
{
public:
    QLineEditEn(QWidget * parent = 0);
protected:
    virtual void focusInEvent(QFocusEvent * event);
};

#endif // QLINEEDITEN_H
