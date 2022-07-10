#ifndef ILISTENER_H
#define ILISTENER_H

#include <QObject>

class IListener : public QObject
{
    Q_OBJECT
public:
    explicit IListener(QObject *parent = nullptr);

signals:

public slots:


};

#endif // ILISTENER_H
