#ifndef GAMEMESSAGING_H
#define GAMEMESSAGING_H

#include <QObject>

class GameMessaging
{
    Q_OBJECT
public:
    GameMessaging(QObject* parent = nullptr);

signals:
    virtual void sendGlobal(QString message);

public slots:
    virtual void recieveGlobal(QString message);

};

#endif // GAMEMESSAGING_H
