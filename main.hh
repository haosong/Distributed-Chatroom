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
#include <QTime>
#include <QHash>
#include <QtCrypto>
#include <QFileDialog>
#include <QTableWidget>

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

    void sendRoutingMessage(); // send Routing Message to all neighbor periodically

    void lookUpHost(QHostInfo host);

    void gotPrivateMsgEntered(QString, QString);

    void openPrivateDialog(QListWidgetItem *item);

    void gotAddFilePressed();

private:

    // GUI
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *leftPanel;
    QTabWidget *rightPanel;
    QWidget *peerTab;
    QWidget *recentTab;
    QWidget *fileTab;
    QWidget *searchTab;
    QLineEdit *peerEdit;
    QListWidget *peerDisplay;
    QListWidget *recentDisplay;
    QPushButton *peerBtn;
    QPushButton *addFileBtn;
    QFileDialog *fileDialog;
    QTextEdit *textview; // Msg Display
    QTextEdit *textline; // Msg Input
    QTableWidget *fileView; // Msg Input

    // Local Data
    //QMap<QString, QMap<quint16, QString> > messageMap; // History Message Database, <Origin, <SeqNo, text>>
    QMap<QString, QMap<quint16, QVariantMap> > messageMap; // History Message Database, <Origin, <SeqNo, text>>
    QMap<QString, QVariant> statusMap; // Local Status Map, <Origin, SeqNo>
    QVariantMap mongeringMsg;// Most recent mongering rumor
    bool isShift; // If shift key is hold or not
    QTimer *antiEntropyTimer;
    QTimer *routingRumorTimer;
    QMap<QString, Peer *> peerMap;
    QQueue<QString> peerInputQueue; // To handle the concurrency of add peer actions.
    QHash<QString, QPair<QHostAddress, quint16> > routingTable; // Next-hop routing table
    bool noForward;
    QHash<QByteArray,QByteArray> fileBlockHash; // <Block Hash, Block File>
    QHash<QByteArray, QVariantMap> metafileList;

    // Functions
    bool eventFilter(QObject *obj, QEvent *ev); // If press enter to send msg or not

    void receiveStatusMessage(QMap<QString, QVariant> senderStatus, QHostAddress address, quint16 port);

    void receiveRumorMessage(QMap<QString, QVariant> rumor, QHostAddress address, quint16 port);

    void receivePrivateMessage(QMap<QString, QVariant> privateMsg);

    //void mongerRumor(QString chatText, QString origin, quint16 seqNo, QHostAddress address = QHostAddress::LocalHost,
    //                 quint16 port = 0);

    void sendRumorMessage(QMap<QString, QVariant> rumor, QHostAddress address = QHostAddress::LocalHost,
                          quint16 port = 0);

    void sendStatus(QMap<QString, QVariant> statusMap, QHostAddress address, quint16 port);

    void sendPrivateMessage(QMap<QString, QVariant> privateMsg, QHostAddress address, quint16 port);

    Peer *pickNeighbors();

    void addPeer(QHostAddress address, quint16 port, QString host);

    void insertRoutingTable(QString origin, QHostAddress address, quint16 port);
};

class PrivateDialog : public QDialog {
Q_OBJECT

    friend class ChatDialog;

public:
    PrivateDialog() = default;

    explicit PrivateDialog(QString origin);

public slots:

    void gotSendPressed();

signals:

    void privateMsgSent(QString origin, QString text);

private:
    QString origin;
    QGroupBox *groupBox;
    QTextEdit *textEdit;
    QPushButton *sendBtn;
};

#endif // PEERSTER_MAIN_HH
