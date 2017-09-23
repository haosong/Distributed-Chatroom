#ifndef PEERSTER_PEER_HH
#define PEERSTER_PEER_HH

#include <QHostAddress>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QStringList>
#include <QHostInfo>


class Peer : public QObject {
Q_OBJECT

public:
    Peer() {};

    Peer(QHostAddress address, quint16 port);

    //explicit Peer(QString string);

    const QString &getHost() const;

    const QHostAddress &getAddress() const;

    quint16 getPort() const;

public slots:

    void process(QHostInfo q);

private:
    QString origin; // Origin
    QString host; // Host Name
    QHostAddress address; // IP Address
    quint16 port; //Port

};

#endif //PEERSTER_PEER_HH
