#include "peer.hh"

Peer::Peer(QHostAddress address, quint16 port) {
    this->address = address;
    this->port = port;
}
//
//Peer::Peer(QString input) {
//    QStringList list = input.split(':');
//    if (list.length() == 2) {
//        QHostInfo::lookupHost(list[0], this, SLOT(process(QHostInfo)));
//        port = static_cast<quint16>(list[1].toUInt());
//    }
//
//}

void Peer::process(QHostInfo q) {
    if (q.error() != QHostInfo::NoError) {
        this->status = 0;
        qDebug() << "Lookup failed:" << q.errorString();
    } else {
        this->host = q.hostName();
        this->address = q.addresses().first();
        this->status = 1;
        qDebug() << q.hostName();
        qDebug() << q.addresses().first().toString();
    }
}

const QString &Peer::getHost() const {
    return host;
}

const QHostAddress &Peer::getAddress() const {
    return address;
}

quint16 Peer::getPort() const {
    return port;
}
