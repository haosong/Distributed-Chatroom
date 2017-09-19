#include <unistd.h>

#include "main.hh"

ChatDialog::ChatDialog() {
    // Create a UDP network socket

    sock = new NetSocket();
    if (!sock->bind())
        exit(1);

    //origin = QString::number(sock->localPort());
    origin = (QHostInfo::localHostName().split('.')[0] + "-" +
              (QUuid::createUuid().toString()).remove(QRegExp("\\{|\\}|\\-")));
    setWindowTitle("Peerster " + origin);
    messageMap.insert(origin, QMap<quint16, QString>());

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
    rightPanel->setEnabled(true);
    rightPanel->setMaximumSize(QSize(283, 16777215));
    rightPanel->setElideMode(Qt::ElideNone);
    rightPanel->setUsesScrollButtons(true);
    peerTab = new QWidget();
    peerEdit = new QLineEdit(peerTab);
    peerEdit->setGeometry(QRect(0, 370, 211, 41));
    peerDisplay = new QListWidget(peerTab);
    peerDisplay->setGeometry(QRect(0, 0, 281, 371));
    peerBtn = new QPushButton("Add Peer", peerTab);
    peerBtn->setGeometry(QRect(210, 371, 71, 40));
    rightPanel->addTab(peerTab, QString());
    leftPanel->setTitle("Chat");
    peerBtn->setText("Add Peer");
    peerBtn->setStyleSheet(
            "QPushButton { border: 1px solid darkGrey; background-color: rgb(0, 153, 255); color: white}; border-radius: 90px;");
    rightPanel->setTabText(rightPanel->indexOf(peerTab), "Peer");
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
    for (int i = 1; i < args.size(); i++) {
        peerEdit->setText(args.at(i));
        gotPeerReturnPressed();
    }

    // Register a callback on the textline's returnPressed signal
    // so that we can send the message entered by the user.
    textline->installEventFilter(this); // Bind return press event.
    connect(sock, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    antiEntropyTimer = new QTimer(this);
    connect(antiEntropyTimer, SIGNAL(timeout()), this, SLOT(antiEntropyTimeout()));
    antiEntropyTimer->start(5000); // Anti-Entropy every 5s.
    //connect(peerEdit, SIGNAL(enterPressed()), this, SLOT(gotPeerReturnPressed()));
    connect(peerBtn, SIGNAL(clicked()), this, SLOT(gotPeerReturnPressed()));
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
    // Initially, just echo the string locally.
    // Insert some networking code here...
    //QVariantMap map;
    //QByteArray mapData;
    //map.insert("ChatText", textline->toPlainText());
    //QDataStream outStream(&mapData, QIODevice::WriteOnly);
    //outStream << map;
    if (!statusMap.contains(origin)) statusMap.insert(origin, 1);

    quint16 seqNo = statusMap.value(origin).toUInt();
    mongeringMsg.clear();
    mongeringMsg.insert("ChatText", textline->toPlainText());
    mongeringMsg.insert("SeqNo", seqNo);
    mongeringMsg.insert("Origin", origin);
    mongerRumor(textline->toPlainText(), origin, seqNo);
    QMap<quint16, QString> msg = messageMap.value(origin);
    msg[seqNo] = textline->toPlainText();
    messageMap[origin] = msg;
    statusMap[origin] = seqNo + 1;
    textview->append(textline->toPlainText());
    //quint16 myPortMin = 32768 + (getuid() % 4096) * 4;
    //quint16 myPortMax = myPortMin + 3;
    //for (quint16 p = myPortMin; p <= myPortMax; p++) {
    //    sock->writeDatagram(mapData, QHostAddress::LocalHost, p);
    //}
    //textview->append(textline->toPlainText());

    // Clear the textline to get ready for the next input message.
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
        qDebug() << "Rceive msg from neighbor: " << key;
        qDebug() << "We have " << peerMap.size() << " neighbours: " << peerMap.keys();

        if (port == sock->localPort()) return;
        //qDebug() << map;
        //if (map.contains(QString("ChatText"))) textview->append(map.value("ChatText").toString());
        if (map.contains("Origin")) {
            // Receive Rumor Message
            receiveRumorMessage(map, address, port);
        } else if (map.contains("Want")) {
            // Receive Status Message
            receiveStatusMessage(map, address, port);
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
            QString origin = i.key();
            quint16 seqNo = senderStatus.contains(origin) ? senderStatus.value(origin).toUInt() : 1;
            QMap<quint16, QString> msgs = messageMap.value(i.key());
            mongerRumor(msgs.value(seqNo), origin, seqNo, address, port);
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
    quint16 seqNo = rumor.value("SeqNo").toUInt();
    QString text = rumor.value("ChatText").toString();
    qDebug() << "received rumor from " << port << " : " << text << " SeqNo =" << seqNo;
    if ((!statusMap.contains(senderOrigin) && (seqNo == 1 && text != "")) || statusMap.value(senderOrigin) == seqNo) {
        QMap<quint16, QString> msg;
        int status = 2;
        if (statusMap.value(senderOrigin) == seqNo) {
            msg = messageMap.value(senderOrigin);
            status = seqNo + 1;
        }
        msg.insert(seqNo, text);
        messageMap.insert(senderOrigin, msg);
        statusMap[senderOrigin] = status;
        textview->append(text);
        qDebug() << "my statusMap: " << statusMap;

        sendStatus(statusMap, address, port);
        mongeringMsg.insert("ChatText", text);
        mongeringMsg.insert("SeqNo", seqNo);
        mongeringMsg.insert("Origin", senderOrigin);
        mongerRumor(text, senderOrigin, seqNo);
    }
    //if (statusMap.value(senderOrigin) == seqNo) {
    //    QMap<quint16, QString> msg = messageMap.value(senderOrigin);
    //    msg.insert(seqNo, text);
    //    messageMap.insert(senderOrigin, msg);
    //    statusMap[senderOrigin] = seqNo + 1;
    //    textview->append(text);
    //}
    //sendStatus(statusMap, address, port);
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

void ChatDialog::mongerRumor(QString chatText, QString origin, quint16 seqNo, QHostAddress address, quint16 port) {
    //qDebug() << "Start rumor monger text: " << chatText << "origin = " << origin << " seqNo = " << seqNo;
    QVariantMap map;
    map.insert("ChatText", chatText);
    map.insert("Origin", origin);
    map.insert("SeqNo", seqNo);
    QByteArray mapData;
    QDataStream stream(&mapData, QIODevice::WriteOnly);
    stream << map;
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
    qDebug() << "Flip Coin";
    //if (qrand() % 2) {
    QByteArray mapData;
    QDataStream stream(&mapData, QIODevice::WriteOnly);
    stream << mongeringMsg;
    Peer *randNeighbor = pickNeighbors();
    sock->writeDatagram(mapData, randNeighbor->getAddress(), randNeighbor->getPort());
    //}
}

Peer *ChatDialog::pickNeighbors() {

    QString randomKey = peerMap.keys().takeAt(qrand() % peerMap.size());
    return peerMap.value(randomKey);


    //quint16 rand = qrand() % peerList.size();
    //return peerList.value(rand);

    //while (pick == sock->localPort() || pick == 0) {
    //    qDebug() << pick;
    //    pick = 32768 + (getuid() % 4096) * 4 + qrand() % 4;
    //}
    //quint16 pick = 32768 + (getuid() % 4096) * 4 + 1;
    //qDebug() << "Pick Neighbor: " << pick;
    //return pick;
}

void ChatDialog::antiEntropyTimeout() {
    //quint16 rand = qrand() % peerList.size();
    //Peer *randPeer = peerList.value(rand);
    QString randomKey = peerMap.keys().takeAt(qrand() % peerMap.size());
    Peer *randPeer = peerMap.value(randomKey);
    sendStatus(statusMap, randPeer->getAddress(), randPeer->getPort());
    //quint16 pick = sock->localPort();
    //while (pick == sock->localPort()) {
    //    pick = 32768 + (getuid() % 4096) * 4 + qrand() % 4;
    //}
    //sendStatus(statusMap, QHostAddress::LocalHost, pick);
}

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
