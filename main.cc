#include <unistd.h>
#include "main.hh"

PrivateDialog::PrivateDialog(QString origin) {
    this->origin = origin;
    this->setWindowTitle("Private Dialog: " + origin);
    this->resize(340, 250);
    groupBox = new QGroupBox(this);
    groupBox->setGeometry(QRect(10, 10, 321, 231));
    groupBox->setAlignment(Qt::AlignCenter);
    groupBox->setTitle("Send Private Message");
    textEdit = new QTextEdit(groupBox);
    textEdit->setGeometry(QRect(10, 20, 301, 161));
    sendBtn = new QPushButton(groupBox);
    sendBtn->setText("Send");
    sendBtn->setStyleSheet(
            "QPushButton { border: 1px solid darkGrey; background-color: rgb(0, 153, 255); color: white};");
    sendBtn->setGeometry(QRect(10, 180, 301, 41));
    connect(sendBtn, SIGNAL(clicked()), this, SLOT(gotSendPressed()));
}

void PrivateDialog::gotSendPressed() {
    qDebug() << "emit " << textEdit->toPlainText() << " from " << origin;
    emit privateMsgSent(origin, textEdit->toPlainText());
    close();
}

ChatDialog::ChatDialog() {
    isShift = false;
    noForward = false;
    // Set random seed
    QTime time(0, 0);
    qsrand(static_cast<uint>(time.secsTo(QTime::currentTime())));

    // Create a UDP network socket
    sock = new NetSocket();
    if (!sock->bind())
        exit(1);

    //origin = QString::number(sock->localPort());
    origin = (QHostInfo::localHostName().split('.')[0] + "-" +
              (QUuid::createUuid().toString()).remove(QRegExp("\\{|\\}|\\-")));
    setWindowTitle("Peerster " + origin);
    //messageMap.insert(origin, QMap<quint16, QString>());
    messageMap.insert(origin, QMap<quint16, QVariantMap>());

    //resize(608, );
    this->setFixedSize(615, 460);
    horizontalLayoutWidget = new QWidget(this);
    horizontalLayoutWidget->setGeometry(QRect(10, 10, 590, 440));
    horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    leftPanel = new QGroupBox(horizontalLayoutWidget);
    leftPanel->setMinimumSize(QSize(300, 0));
    textview = new QTextEdit(leftPanel);
    textview->setGeometry(QRect(6, 23, 285, 321));
    textview->setReadOnly(true);
    textline = new QTextEdit(leftPanel);
    textline->setGeometry(QRect(6, 340, 285, 93));
    textline->setFocus(); // Auto focus
    textview->raise();
    textline->raise();
    horizontalLayout->addWidget(leftPanel);
    rightPanel = new QTabWidget(horizontalLayoutWidget);
    rightPanel->setStyleSheet("QTabWidget::pane { border: 0; }");

    peerTab = new QWidget();
    peerEdit = new QLineEdit(peerTab);
    peerEdit->setGeometry(QRect(0, 370, 211, 41));
    peerDisplay = new QListWidget(peerTab);
    peerDisplay->setGeometry(QRect(0, 0, 281, 371));
    peerBtn = new QPushButton("Add Peer", peerTab);
    peerBtn->setGeometry(QRect(210, 371, 71, 40));
    rightPanel->addTab(peerTab, QString());

    recentTab = new QWidget();
    recentDisplay = new QListWidget(recentTab);
    recentDisplay->setGeometry(QRect(0, 0, 281, 411));
    rightPanel->addTab(recentTab, QString());

    leftPanel->setTitle("Chat");
    peerBtn->setText("Add Peer");
    peerBtn->setStyleSheet(
            "QPushButton { border: 1px solid darkGrey; background-color: rgb(0, 153, 255); color: white};");
    rightPanel->setTabText(rightPanel->indexOf(peerTab), "Peer");
    rightPanel->setTabText(rightPanel->indexOf(recentTab), "Recent");
    horizontalLayout->addWidget(rightPanel);
    rightPanel->setCurrentIndex(0);

    //textview = new QTextEdit(this);
    //textview->setReadOnly(true);
    //textline = new QTextEdit(this);
    //textline->setFixedHeight(75);
    //textline->setFocus(); // Auto focus
    // Lay out the widgets to appear in the main window.
    // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
    //QVBoxLayout *layout = new QVBoxLayout();
    //layout->addWidget(textview);
    //layout->addWidget(textline);
    //setLayout(layout);

    QString localPublicIP = QHostInfo::fromName(QHostInfo::localHostName()).addresses().first().toString();
    int portMin = 32768 + (getuid() % 4096) * 4;
    int i = 3; // Initial neighbour number
    for (quint16 port = portMin; port < portMin + 4; port++) {
        if (port != sock->localPort()) {
            i--;
            addPeer(QHostAddress(localPublicIP), port, QHostInfo::localHostName());
        }
        if (i <= 0) break;
    }
    //qDebug() << "We have " << peerList.size() << " neighbours";

    QStringList args = QCoreApplication::arguments();
    for (i = 1; i < args.size(); i++) {
        if (args.at(i) == "-noforward") {
            noForward = true;
        } else {
            peerEdit->setText(args.at(i));
            gotPeerReturnPressed();
        }
    }

    // Prime the pump
    QTimer::singleShot(0, this, SLOT(sendRoutingMessage()));

    // Register a callback on the textline's returnPressed signal
    // so that we can send the message entered by the user.
    textline->installEventFilter(this); // Bind return press event.
    connect(sock, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    antiEntropyTimer = new QTimer(this);
    connect(antiEntropyTimer, SIGNAL(timeout()), this, SLOT(antiEntropyTimeout()));
    antiEntropyTimer->start(5000); // Anti-Entropy every 5s.
    //connect(peerEdit, SIGNAL(enterPressed()), this, SLOT(gotPeerReturnPressed()));
    connect(peerBtn, SIGNAL(clicked()), this, SLOT(gotPeerReturnPressed()));
    routingRumorTimer = new QTimer(this);
    connect(routingRumorTimer, SIGNAL(timeout()), this, SLOT(sendRoutingMessage()));
    routingRumorTimer->start(60000);
    connect(recentDisplay, SIGNAL(itemDoubleClicked(QListWidgetItem * )), this,
            SLOT(openPrivateDialog(QListWidgetItem * )));
}

bool ChatDialog::eventFilter(QObject *obj, QEvent *event) {
    // http://doc.qt.io/qt-4.8/qobject.html#installEventFilter
    if (obj == textline && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Shift) {
            ChatDialog::isShift = true;
        } else if (keyEvent->key() == Qt::Key_Return) {
            if (textline->toPlainText() != "" && !ChatDialog::isShift) {
                gotChatReturnPressed();
            }
            return !ChatDialog::isShift;
        }
    } else if (obj == textline && event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Shift) ChatDialog::isShift = false;
    }
    return false; // stop event being handled further, return true; otherwise return false.
}

void ChatDialog::gotChatReturnPressed() {
    if (!statusMap.contains(origin)) statusMap.insert(origin, 1);
    quint16 seqNo = statusMap.value(origin).toUInt();
    mongeringMsg.clear();
    mongeringMsg.insert("ChatText", textline->toPlainText());
    mongeringMsg.insert("SeqNo", seqNo);
    mongeringMsg.insert("Origin", origin);
    sendRumorMessage(mongeringMsg);
    //mongerRumor(textline->toPlainText(), origin, seqNo);
    //QMap<quint16, QString> msg = messageMap.value(origin);
    QMap<quint16, QVariantMap> msg = messageMap.value(origin);
    msg[seqNo] = mongeringMsg;
    messageMap[origin] = msg;
    statusMap[origin] = seqNo + 1;
    textview->append(textline->toPlainText());
    textline->clear();
}

void ChatDialog::gotPeerReturnPressed() {
    qDebug() << "got Peer Return Pressed: " << peerEdit->text();
    if (peerEdit->text() != "") {
        QStringList list = peerEdit->text().split(':');
        if (list.size() != 2) return;
        bool isPort;
        list[1].toInt(&isPort);
        if (!isPort) return;
        peerInputQueue.enqueue(peerEdit->text());
        peerEdit->setDisabled(true);
        QHostInfo::lookupHost(list[0], this, SLOT(lookUpHost(QHostInfo)));
    }
}

void ChatDialog::lookUpHost(QHostInfo q) {
    if (q.error() != QHostInfo::NoError) {
        peerEdit->setText("Invalid address !");
        peerEdit->selectAll();
        QTimer::singleShot(0, peerEdit, SLOT(setFocus()));
        //peerEdit->setFocus(Qt::OtherFocusReason);
    } else {
        QString input = peerInputQueue.dequeue();
        addPeer(q.addresses().first(), static_cast<quint16>(input.split(':')[1].toUInt()), q.hostName());
        peerEdit->clear();
    }
    peerEdit->setDisabled(false);
}

void ChatDialog::addPeer(QHostAddress addr, quint16 port, QString host) {
    QString key = addr.toString() + ":" + QString::number(port);
    if (!peerMap.contains(key)) {
        QListWidgetItem *newItem = new QListWidgetItem();
        newItem->setText(key + "\n@" + host);
        newItem->setSizeHint(QSize(250, 40));
        peerDisplay->addItem(newItem);
        Peer *p = new Peer(addr, port);
        //peerList.push_back(p);
        peerMap[key] = p;
        qDebug() << "We have " << peerMap.size() << " neighbours";
    }
}


void ChatDialog::receiveMessage() {
    while (sock->hasPendingDatagrams()) {
        QMap<QString, QVariant> map;
        QByteArray mapData;
        QHostAddress address;
        quint16 port;

        mapData.resize(sock->pendingDatagramSize());
        sock->readDatagram(mapData.data(), mapData.size(), &address, &port);
        QDataStream inStream(&mapData, QIODevice::ReadOnly);
        inStream >> map;

        QString key = address.toString() + ":" + QString::number(port);
        if (!peerMap.contains(key)) {
            peerEdit->setText(key);
            gotPeerReturnPressed();
        }
        qDebug() << "Rceive msg from neighbor: " << key << " " << map;
        qDebug() << "We have " << peerMap.size() << " neighbours: " << peerMap.keys();

        if (map.contains("Origin")) {
            // Receive Rumor Message
            receiveRumorMessage(map, address, port);
        } else if (map.contains("Want")) {
            // Receive Status Message
            receiveStatusMessage(map, address, port);
        } else if (map.contains("Dest")) {
            // Receive Private Message
            receivePrivateMessage(map);
        }
    }
}

void ChatDialog::receiveStatusMessage(QMap<QString, QVariant> dataGram, QHostAddress address, quint16 port) {
    QMap<QString, QVariant> senderStatus = dataGram.value("Want").toMap();
    qDebug() << "received status from " << port << " : " << senderStatus;
    QMap<QString, QVariant>::iterator i; // http://doc.qt.io/qt-4.8/qmap-iterator.html
    for (i = statusMap.begin(); i != statusMap.end(); i++) {
        if (!senderStatus.contains(i.key()) || senderStatus.value(i.key()).toUInt() < i.value().toUInt()) {
            qDebug() << "Status: " << i.key() << " " << i.value() << " " << senderStatus.value(i.key());
            // Do the rumor mongering
            quint16 seqNo = senderStatus.contains(i.key()) ? senderStatus.value(i.key()).toUInt() : 1;
            QMap<quint16, QVariantMap> msgs = messageMap.value(i.key());
            sendRumorMessage(msgs.value(seqNo), address, port);
            return;
        }
    }
    for (i = senderStatus.begin(); i != senderStatus.end(); i++) {
        qDebug() << "received status" << i.key() << ": " << i.value().toUInt();
        if (!statusMap.contains(i.key()) || statusMap.value(i.key()).toUInt() < i.value().toUInt()) {
            sendStatus(statusMap, address, port);
            return;
        }
    }
    // Flip-Coin and send the last message.
    //flipCoin();
}

void ChatDialog::receiveRumorMessage(QMap<QString, QVariant> rumor, QHostAddress address, quint16 port) {
    QString senderOrigin = rumor.value("Origin").toString();
    quint16 seqNo = static_cast<quint16>(rumor.value("SeqNo").toUInt());
    qDebug() << "received rumor: " << rumor << "from " << port << " :  SeqNo =" << seqNo;
    //if ((!statusMap.contains(senderOrigin) && (seqNo == 1 && text != "")) || statusMap.value(senderOrigin) == seqNo) {
    if ((!statusMap.contains(senderOrigin) && (seqNo == 1)) || statusMap.value(senderOrigin) == seqNo) {
        // Update Routing Table
        insertRoutingTable(senderOrigin, address, port);
        QMap<quint16, QVariantMap> msg;
        int status = 2;
        if (statusMap.value(senderOrigin) == seqNo) {
            msg = messageMap.value(senderOrigin);
            status = seqNo + 1;
        }
        msg.insert(seqNo, rumor);
        messageMap.insert(senderOrigin, msg);
        statusMap[senderOrigin] = status;
        if (rumor.contains("ChatText")) textview->append(rumor.value("ChatText").toString());
        qDebug() << "my statusMap: " << statusMap;
        sendRumorMessage(rumor);
    } else {
        sendStatus(statusMap, address, port);
    }
}

void ChatDialog::receivePrivateMessage(QMap<QString, QVariant> privateMsg) {
    QString dest = privateMsg.value("Dest").toString();
    quint32 hopLimit = privateMsg.value("HopLimit").toUInt();
    if (dest == this->origin) {
        textview->append(privateMsg.value("ChatText").toString());
        return;
    }
    if (!noForward && hopLimit > 0 && routingTable.contains(dest)) {
        privateMsg["HopLimit"] = hopLimit - 1;
        sendPrivateMessage(privateMsg, routingTable.value(dest).first, routingTable.value(dest).second);
    }
}

void ChatDialog::sendStatus(QMap<QString, QVariant> statusMap, QHostAddress address, quint16 port) {
    QMap<QString, QVariant> map;
    map.insert("Want", statusMap);
    QByteArray mapData;
    QDataStream stream(&mapData, QIODevice::WriteOnly);
    stream << map;
    qDebug() << "Sending status: " << map;
    sock->writeDatagram(mapData, address, port);
}

void ChatDialog::sendRumorMessage(QMap<QString, QVariant> rumor, QHostAddress address, quint16 port) {
    if(noForward && rumor.contains("ChatText")) return;
    //qDebug() << "Start rumor monger text: " << chatText << "origin = " << origin << " seqNo = " << seqNo;
    QByteArray mapData;
    QDataStream stream(&mapData, QIODevice::WriteOnly);
    stream << rumor;
    if (port == 0) {
        Peer *randNeighbor = pickNeighbors();
        sock->writeDatagram(mapData, randNeighbor->getAddress(), randNeighbor->getPort());
    } else {
        sock->writeDatagram(mapData, address, port);
    }
    QTimer::singleShot(1000, this, SLOT(flipCoin()));
}

void ChatDialog::flipCoin() {
    // Flip-Coin and send the last message.
    if (qrand() % 3 != 0) { // 66.66% Possibility
        qDebug() << "Flip Coin";
        sendRumorMessage(mongeringMsg);
    }
}

Peer *ChatDialog::pickNeighbors() {
    QString randomKey = peerMap.keys().takeAt(qrand() % peerMap.size());
    return peerMap.value(randomKey);
}

void ChatDialog::antiEntropyTimeout() {
    QString randomKey = peerMap.keys().takeAt(qrand() % peerMap.size());
    Peer *randPeer = peerMap.value(randomKey);
    sendStatus(statusMap, randPeer->getAddress(), randPeer->getPort());
}

void ChatDialog::insertRoutingTable(QString origin, QHostAddress address, quint16 port) {
    routingTable.insert(origin, QPair<QHostAddress, quint16>(address, port));
    QHash<QString, QPair<QHostAddress, quint16> >::iterator i;
    recentDisplay->clear();
    for (i = routingTable.begin(); i != routingTable.end(); ++i) {
        auto *newItem = new QListWidgetItem();
        newItem->setData(Qt::UserRole, i.key());
        qDebug() << "insert table : " << i.key();
        newItem->setText(i.key() + "\n" + i.value().first.toString() + ":" + QString::number(i.value().second));
        //newItem->setSizeHint(QSize(newItem->sizeHint().width(), 40));
        recentDisplay->addItem(newItem);
    }
}

void ChatDialog::gotPrivateMsgEntered(QString origin, QString text) {
    qDebug() << "ChatDialog get" << text << " from " << origin;
    QMap<QString, QVariant> privateMsg;
    privateMsg.insert("Dest", origin);
    privateMsg.insert("ChatText", text);
    privateMsg.insert("HopLimit", 10);
    textview->append(text);
    sendPrivateMessage(privateMsg, routingTable.value(origin).first, routingTable.value(origin).second);
}

void ChatDialog::sendPrivateMessage(QMap<QString, QVariant> privateMsg, QHostAddress address, quint16 port) {
    QByteArray mapData;
    QDataStream stream(&mapData, QIODevice::WriteOnly);
    stream << privateMsg;
    qDebug() << "Sending private msg: " << privateMsg;
    sock->writeDatagram(mapData, address, port);
}

void ChatDialog::sendRoutingMessage() {
    // Send routing to all neighbor per minute
    for (Peer *p : peerMap) {
        qDebug() << p->getHost() << " : " << p->getPort();
        qDebug() << "rumor route msg to" << p->getPort();
        if (!statusMap.contains(origin)) statusMap.insert(origin, 1);
        quint16 seqNo = static_cast<quint16>(statusMap.value(origin).toUInt());
        mongeringMsg.clear();
        mongeringMsg.insert("SeqNo", seqNo);
        mongeringMsg.insert("Origin", origin);
        sendRumorMessage(mongeringMsg, p->getAddress(), p->getPort());
        QMap<quint16, QVariantMap> msg = messageMap.value(origin);
        msg[seqNo] = mongeringMsg;
        messageMap[origin] = msg;
        statusMap[origin] = seqNo + 1;
    }
}

void ChatDialog::openPrivateDialog(QListWidgetItem *item) {
    QString itemOrigin = item->data(Qt::UserRole).toString();
    PrivateDialog *subDialog = new PrivateDialog(itemOrigin);
    subDialog->show();
    connect(subDialog, SIGNAL(privateMsgSent(QString, QString)), this, SLOT(gotPrivateMsgEntered(QString, QString)));
}


//void ChatDialog::sendRoutingMessage(QString origin, quint16 seqNo, QHostAddress address, quint16 port) {
//    QVariantMap map;
//    map.insert("Origin", origin);
//    map.insert("SeqNo", seqNo);
//    QByteArray mapData;
//    QDataStream stream(&mapData, QIODevice::WriteOnly);
//    stream << map;
//    if (port == 0) {
//        Peer *randNeighbor = pickNeighbors();
//        sock->writeDatagram(mapData, randNeighbor->getAddress(), randNeighbor->getPort());
//    } else {
//        sock->writeDatagram(mapData, address, port);
//    }
//}

NetSocket::NetSocket() {
    // Pick a range of four UDP ports to try to allocate by default,
    // computed based on my Unix user ID.
    // This makes it trivial for up to four Peerster instances per user
    // to find each other on the same host,
    // barring UDP port conflicts with other applications
    // (which are quite possible).
    // We use the range from 32768 to 49151 for this purpose.
    myPortMin = 32768 + (getuid() % 4096) * 4;
    myPortMax = myPortMin + 3;
}

bool NetSocket::bind() {
    // Try to bind to each of the range myPortMin..myPortMax in turn.
    for (int p = myPortMin; p <= myPortMax; p++) {
        if (QUdpSocket::bind(p)) {
            qDebug() << "bound to UDP port " << p;
            return true;
        }
    }
    qDebug() << "Oops, no ports in my default range " << myPortMin
             << "-" << myPortMax << " available";
    return false;
}

int main(int argc, char **argv) {
    // Initialize Qt toolkit
    QApplication app(argc, argv);

    // Create an initial chat dialog window
    ChatDialog dialog;
    dialog.show();

    // Enter the Qt main loop; everything else is event driven
    return app.exec();
}
