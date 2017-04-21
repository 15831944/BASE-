#ifndef QROCHECKBOX_H
#define QROCHECKBOX_H

#include <QCheckBox>

class QRoCheckBox : public QCheckBox {
private:
    bool mReadOnly;
public:
    QRoCheckBox(QWidget *gb);

    void setReadOnly(bool aReadOnly);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // QROCHECKBOX_H
