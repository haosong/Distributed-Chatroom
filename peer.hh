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
    QString origin; // hippo-{UUID}
    QString host; // hippo.zoo.cs.yale.edu
    QHostAddress address; // 128.36.232.16
    quint16 port; //48264

};

#endif //PEERSTER_PEER_HH
