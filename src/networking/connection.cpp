#include "networking/connection.h"

Connection::Connection(int ID, QObject *parent) :
    QThread(parent)
{
    this->socketDescriptor = ID;
}

void Connection::run()
{
    // make thread loop
    exec();
}

void Connection::readyRead(){
    QByteArray utfData = socket->readAll();
    // qDebug() << socketDescriptor << " RECV: " << utfData;
    QString data = QString::fromUtf8(utfData);
    emit recieved(data);
}

void Connection::send(QString data) {
    QByteArray utfData = data.toUtf8();
    // qDebug() << socketDescriptor << " SENDING: " << utfData;
    socket->write(utfData + "*");
    socket->flush();
}

void Connection::disconnected()
{
    qDebug() << socketDescriptor << " Disconnected";
    socket->deleteLater();
    exit(0);
}

void Connection::createSocket() {
    // thread starts here
    qDebug() << socketDescriptor << " Starting thread";
    socket = new QTcpSocket(this);
    if(!socket->setSocketDescriptor(this->socketDescriptor))
    {
        emit error(socket->error());
        qDebug() << "Socket ERROR";
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()),Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()),Qt::DirectConnection);

    qDebug() << socketDescriptor << " Client connected";

    this->send("NICK;");
}

void Connection::destroySocket() {
    qDebug() << "Destroying socket";
    socket->disconnect();
    socket->deleteLater();
    this->exit();
}
