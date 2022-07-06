#include "src/networking/connection.h"

Connection::Connection(int ID, QObject *parent) :
    QThread(parent)
{
    this->socketDescriptor = ID;
}

void Connection::run()
{


    // make this thread a loop
    exec();
}

void Connection::readyRead(){
    QByteArray data = socket->readAll();
    qDebug() << socketDescriptor << " RECV: " << data;
    QString refined_data = QString::fromUtf8(data);
    emit recieved(refined_data);
}

void Connection::send(QString data) {
    QByteArray converted_data = data.toUtf8();
    qDebug() << socketDescriptor << " SENT: " << converted_data;
    socket->write(converted_data);
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
