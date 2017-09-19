#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QKeyEvent>
#include <QHostInfo>
#include <QUuid>
#include <QTimer>
#include <QGroupBox>
#include <QTabWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVariant>
#include <QAction>
#include <QButtonGroup>
#include <QHeaderView>
#include <QPushButton>
#include <QWidget>
#include <QVector>
#include <QQueue>

#include "peer.hh"

class NetSocket : public QUdpSocket {
Q_OBJECT

public:
    NetSocket();

    bool bind(); // Bind this socket to a Peerster-specific default port.

private:
    int myPortMin, myPortMax;
};

class ChatDialog : public QDialog {
Q_OBJECT

public:
    ChatDialog();

    NetSocket *sock;
    QString origin;
    //const QString origin = (QHostInfo::localHostName().split('.')[0] + "-" +
    //                        (QUuid::createUuid().toString()).remove(QRegExp("\\{|\\}|\\-")));
public slots:

    void receiveMessage();

    void gotChatReturnPressed();

    void gotPeerReturnPressed();

    void flipCoin(); // Flip coin or rumor mongering time out

    void antiEntropyTimeout();

    void lookUpHost(QHostInfo host);

private:

    // GUI
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *leftPanel;
    QTabWidget *rightPanel;
    QWidget *peerTab;
    QLineEdit *peerEdit;
    QListWidget *peerDisplay;
    QPushButton *peerBtn;
    QTextEdit *textview; // Msg Display
    QTextEdit *textline; // Msg Input

    // Local Data Structure
    QMap<QString, QMap<quint16, QString> > messageMap; // History Message Database, <Origin, <SeqNo, text>>
    QMap<QString, QVariant> statusMap; // Local Status Map, <Origin, SeqNo>
    QVariantMap mongeringMsg;// Most recent mongering rumor
    bool isShift; // If shift key is hold or not
    QTimer *antiEntropyTimer;
    //QVector<Peer *> peerList;
    QMap<QString, Peer *> peerMap;
    QQueue<QString> peerInputQueue; // To handle the concurrency of add peer actions.

    // Functions
    bool eventFilter(QObject *obj, QEvent *ev); // If press enter to send msg or not

    void receiveStatusMessage(QMap<QString, QVariant> senderStatus, QHostAddress address, quint16 port);

    void receiveRumorMessage(QMap<QString, QVariant> rumor, QHostAddress address, quint16 port);

    void mongerRumor(QString chatText, QString origin, quint16 seqNo, QHostAddress address = QHostAddress::LocalHost,
                     quint16 port = 0);

    void sendStatus(QMap<QString, QVariant> statusMap, QHostAddress address, quint16 port);

    Peer *pickNeighbors();

    void addPeer(QHostAddress address, quint16 port, QString host);
};


#endif // PEERSTER_MAIN_HH
