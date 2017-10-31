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
#include <QSet>
#include <QtCrypto>
#include <QFileDialog>
#include <QTableWidget>
#include <QLabel>

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

    void gotSearchFilePressed();

    void directDownloadFile();

    void floodPeriodically();

    void clickToDownload(QListWidgetItem *item);

private:

    // GUI
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *leftPanel;
    QTabWidget *rightPanel;
    QWidget *peerTab;
    QWidget *recentTab;
    QWidget *fileTab;
    QWidget *downloadTab;
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
    QGroupBox *bottomSection;
    QTabWidget *bottomPanel;
    QLineEdit *ddMeta; // Direct download Metafile
    QLineEdit *ddName; // Direct download File name
    QLineEdit *ddFrom; // Direct download from origin
    QPushButton *ddBtn; // Direct download button
    QLineEdit *searchFile;
    QPushButton *searchBtn;
    QListWidget *searchResult;

    // Local Data
    //QMap<QString, QMap<quint16, QString> > messageMap; // History Message Database, <Origin, <SeqNo, text>>
    QMap<QString, QMap<quint16, QVariantMap> > messageMap; // History Message Database, <Origin, <SeqNo, text>>
    QMap<QString, QVariant> statusMap; // Local Status Map, <Origin, SeqNo>
    QVariantMap mongeringMsg;// Most recent mongering rumor
    bool isShift; // If shift key is hold or not
    QTimer *antiEntropyTimer;
    QTimer *routingRumorTimer;
    QTimer *searchTimer;
    QMap<QString, Peer *> peerMap;
    QQueue<QString> peerInputQueue; // To handle the concurrency of add peer actions.
    QHash<QString, QPair<QHostAddress, quint16> > routingTable; // Next-hop routing table
    bool noForward;
    QHash<QByteArray, QByteArray> fileBlockHash; // <Block Hash, Block File>
    QHash<QByteArray, QVariantMap> fileDisplay;
    QHash<QByteArray, QVariantMap> metafileList; // <Metafile Hash, Meta File Map>
    QHash<QByteArray, QSet<QByteArray>> downloadingFile;
    QHash<QByteArray, QVariantMap> downloadFileBlock; // <Block Hash, <"block": Block File, "belong": Metafile Hash>>
    QHash<QByteArray, QVariantMap> downloadMetafile; // <Metafile Hash, Meta File Map>
    QMap<QString, QVariant> searchStatus; // <<"Budget", uint>, <"Search", QString>
    QSet<QByteArray> searchResultSet;

    // Functions
    bool eventFilter(QObject *obj, QEvent *ev); // If press enter to send msg or not

    void receiveStatusMessage(QMap<QString, QVariant> senderStatus, QHostAddress address, quint16 port);

    void receiveRumorMessage(QMap<QString, QVariant> rumor, QHostAddress address, quint16 port);

    void receivePrivateMessage(QMap<QString, QVariant> privateMsg, QHostAddress address, quint16 port);

    void receiveBlockRequest(QMap<QString, QVariant> request);

    void receiveBlockReply(QMap<QString, QVariant> reply);

    //void mongerRumor(QString chatText, QString origin, quint16 seqNo, QHostAddress address = QHostAddress::LocalHost,
    //                 quint16 port = 0);

    void sendMessage(QMap<QString, QVariant> msg, QHostAddress address, quint16 port);

    void sendRumorMessage(QMap<QString, QVariant> rumor, QHostAddress address = QHostAddress::LocalHost,
                          quint16 port = 0);

    void sendStatus(QMap<QString, QVariant> statusMap, QHostAddress address, quint16 port);

    void sendPrivateMessage(QMap<QString, QVariant> privateMsg, QHostAddress address, quint16 port);

    Peer *pickNeighbors();

    void addPeer(QHostAddress address, quint16 port, QString host);

    void insertRoutingTable(QString origin, QHostAddress address, quint16 port);

    void receiveSearchRequest(QMap<QString, QVariant> reply);

    void receiveSearchReply(QMap<QString, QVariant> reply);

    void floodSearchRequest(QMap<QString, QVariant> request);

    void insertFileView(QString name, QString size, QByteArray hash, QString status, QString origin);
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
