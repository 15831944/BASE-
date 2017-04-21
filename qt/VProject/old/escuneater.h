#ifndef ESCUNEATER_H
#define ESCUNEATER_H

#include <QObject>

class EscUneater : public QObject
{
    Q_OBJECT
public:
    explicit EscUneater(QObject *parent = 0);
    
signals:
    
public slots:
    
protected:
     bool eventFilter(QObject *obj, QEvent *event);
};

#endif // ESCUNEATER_H
