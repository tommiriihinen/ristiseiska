#ifndef CONNECTION_H
#define CONNECTION_H

#include <QThread>
#include <QTcpSocket>
#include <QObject>


class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(int iID, QObject *parent = nullptr);

    void send(QString data);

signals:
    void error(QTcpSocket::SocketError socketerror);
    void recieved(QString data);

public slots:
    void readyRead();
    void disconnected();
    void createSocket();
    void destroySocket();

private:
    QString MSG_DELIMITER { "/" };
    QByteArray MSG_DELIMITER_UTF8 { MSG_DELIMITER.toUtf8() };
    QTcpSocket *socket;
    int socketDescriptor;

};
#endif // CONNECTION_H
