#include "QRoCheckBox.h"

QRoCheckBox::QRoCheckBox(QWidget *gb) :
    QCheckBox(gb), mReadOnly(true)
{
}

void QRoCheckBox::setReadOnly(bool aReadOnly) {
    mReadOnly = aReadOnly;
}

void QRoCheckBox::mousePressEvent(QMouseEvent *event) {
    if (!mReadOnly) QCheckBox::mousePressEvent(event);
}

void QRoCheckBox::mouseReleaseEvent(QMouseEvent *event) {
    if (!mReadOnly) QCheckBox::mouseReleaseEvent(event);
}
