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
    sendBtn->setStyleSheet("QPushButton { border: 1px solid darkGrey; background-color: #0099ff; color: white};");
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
    this->setFixedSize(611, 810);
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
    bottomSection = new QGroupBox(this);
    bottomSection->setTitle("File Sharing and Search");
    bottomSection->setGeometry(QRect(10, 460, 591, 341));
    bottomPanel = new QTabWidget(bottomSection);
    bottomPanel->setGeometry(QRect(10, 20, 571, 311));

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

    fileTab = new QWidget();
    fileView = new QTableWidget(fileTab);
    auto *fileNameTab = new QTableWidgetItem();
    fileView->setColumnCount(5);
    fileNameTab->setText("Name");
    fileView->setHorizontalHeaderItem(0, fileNameTab);
    fileView->setColumnWidth(0, 120);
    auto *fileSizeTab = new QTableWidgetItem();
    fileSizeTab->setText("Size");
    fileView->setHorizontalHeaderItem(1, fileSizeTab);
    fileView->setColumnWidth(1, 77);
    auto *metafileTab = new QTableWidgetItem();
    metafileTab->setText("Hash");
    fileView->setHorizontalHeaderItem(2, metafileTab);
    fileView->setColumnWidth(2, 180);
    auto *statusTab = new QTableWidgetItem();
    statusTab->setText("Status");
    fileView->setHorizontalHeaderItem(3, statusTab);
    fileView->setColumnWidth(3, 77);
    auto *originTab = new QTableWidgetItem();
    originTab->setText("Origin");
    fileView->setHorizontalHeaderItem(4, originTab);
    fileView->setColumnWidth(4, 110);
    fileView->setGeometry(QRect(0, 0, 568, 251));
    fileView->verticalHeader()->setVisible(false);
    fileView->setRowCount(0);
    addFileBtn = new QPushButton("Add File", fileTab);
    addFileBtn->setGeometry(QRect(467, 251, 101, 40));
    bottomPanel->addTab(fileTab, QString());

    downloadTab = new QWidget();
    QLabel *label1 = new QLabel(downloadTab);
    label1->setGeometry(QRect(40, 40, 71, 31));
    label1->setText("File Name:");
    ddName = new QLineEdit(downloadTab);
    ddName->setGeometry(QRect(120, 40, 401, 31));
    ddName->setPlaceholderText("Enter file name ...");
    QLabel *label2 = new QLabel(downloadTab);
    label2->setGeometry(QRect(50, 90, 61, 31));
    label2->setText("MetaFile:");
    ddMeta = new QLineEdit(downloadTab);
    ddMeta->setGeometry(QRect(120, 90, 401, 31));
    ddMeta->setPlaceholderText("Enter metafile SHA-1 hash value ...");
    QLabel *label3 = new QLabel(downloadTab);
    label3->setGeometry(QRect(40, 140, 91, 31));
    label3->setText("File Origin:");
    ddFrom = new QLineEdit(downloadTab);
    ddFrom->setGeometry(QRect(120, 140, 401, 31));
    ddFrom->setPlaceholderText("Enter origin you want to download from ...");
    ddBtn = new QPushButton("Download", downloadTab);
    ddBtn->setGeometry(QRect(120, 190, 80, 36));
    bottomPanel->addTab(downloadTab, QString());

    searchTab = new QWidget();
    searchFile = new QLineEdit(searchTab);
    searchFile->setGeometry(QRect(20, 20, 441, 31));
    searchFile->setPlaceholderText("Enter file name to search ...");
    searchBtn = new QPushButton(searchTab);
    searchBtn->setText("Search");
    searchBtn->setGeometry(QRect(470, 20, 81, 31));
    searchResult = new QListWidget(searchTab);
    searchResult->setGeometry(QRect(20, 60, 531, 211));
    //searchResult->setGridSize(QSize(0, 55));
    bottomPanel->addTab(searchTab, QString());

    leftPanel->setTitle("Chat");
    peerBtn->setText("Add Peer");
    QString btnStyle = "QPushButton { border: 1px solid darkGrey; background-color: #0099ff; color: white; };";
    QString listItemStyle = "QListWidget:item{ height: 40px; border-bottom: 1px solid lightgrey; }";
    listItemStyle += "QListWidget::item:selected{background-color: #6598cb; color: white; }";
    recentDisplay->setStyleSheet(listItemStyle);
    peerDisplay->setStyleSheet(listItemStyle);
    QString searchItemStyle = "QListWidget:item{ height: 60px; border-bottom: 1px solid lightgrey; }";
    searchItemStyle += "QListWidget::item:selected{background-color: #6598cb; color: white; }";
    searchResult->setStyleSheet(searchItemStyle);
    peerBtn->setStyleSheet(btnStyle);
    addFileBtn->setStyleSheet(btnStyle);
    ddBtn->setStyleSheet(btnStyle);
    searchBtn->setStyleSheet(btnStyle);
    rightPanel->setTabText(rightPanel->indexOf(peerTab), "Peer List");
    rightPanel->setTabText(rightPanel->indexOf(recentTab), "Roting Table");
    bottomPanel->setTabText(bottomPanel->indexOf(fileTab), "File List");
    bottomPanel->setTabText(bottomPanel->indexOf(downloadTab), "Direct Download");
    bottomPanel->setTabText(bottomPanel->indexOf(searchTab), "Search && Download");
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
    searchTimer = new QTimer(this);
    connect(searchTimer, SIGNAL(timeout()), this, SLOT(floodPeriodically()));
    antiEntropyTimer->start(5000); // Anti-Entropy every 5s.
    //connect(peerEdit, SIGNAL(enterPressed()), this, SLOT(gotPeerReturnPressed()));
    connect(peerBtn, SIGNAL(clicked()), this, SLOT(gotPeerReturnPressed()));
    routingRumorTimer = new QTimer(this);
    connect(routingRumorTimer, SIGNAL(timeout()), this, SLOT(sendRoutingMessage()));
    routingRumorTimer->start(60000);
    connect(recentDisplay, SIGNAL(itemDoubleClicked(QListWidgetItem * )), this,
            SLOT(openPrivateDialog(QListWidgetItem * )));
    connect(addFileBtn, SIGNAL(clicked()), this, SLOT(gotAddFilePressed()));
    connect(ddBtn, SIGNAL(clicked()), this, SLOT(directDownloadFile()));
    connect(searchBtn, SIGNAL(clicked()), this, SLOT(gotSearchFilePressed()));
    connect(searchResult, SIGNAL(itemDoubleClicked(QListWidgetItem * )), this,
            SLOT(clickToDownload(QListWidgetItem * )));
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

void ChatDialog::gotSearchFilePressed() {
    if (searchStatus.contains("Budget") && searchStatus.value("Budget").toUInt() > 0) return;
    searchResult->clear();
    searchResultSet.clear();
    QString keywords = searchFile->text().trimmed();
    if (keywords.length() == 0) return;
    int budget = 2;
    searchStatus["Budget"] = budget;
    searchStatus["Search"] = keywords;
    QSet<QByteArray> received;
    searchFile->setText("Searching \"" + keywords + "\" ...");
    searchFile->setEnabled(false);
    searchTimer->start(1000);
}

void ChatDialog::floodPeriodically() {
    uint budget = searchStatus.value("Budget").toUInt();
    qDebug() << budget;
    qDebug() << searchResultSet.size();
    // check if already got 10 request or budget larger than 100
    if (searchResultSet.size() >= 10 || (budget * 2) > 100) {
        searchStatus["Budget"] = 0;
        searchFile->clear();
        searchFile->setEnabled(true);
        searchTimer->stop();
    } else {
        budget *= 2;
        searchStatus["Budget"] = budget;
        floodSearchRequest(searchStatus);
    }
}

void ChatDialog::floodSearchRequest(QMap<QString, QVariant> request) {
    int budget = request.value("Budget").toUInt();
    if (budget <= 0) return;
    int peerNum = peerMap.size();
    int peerBudget = 0;
    QVariantMap requestMap;
    requestMap["Origin"] = request.contains("Origin") ? request["Origin"].toString() : this->origin;
    requestMap["Search"] = request.value("Search").toString();
    if (budget <= peerMap.size()) {
        int peerCandidate[peerNum];
        for (int i = 0; i < peerNum; i++) peerCandidate[i] = i;
        for (int i = 0; i < budget; i++) {
            int rand = qrand() % (peerNum - i);
            Peer *p = peerMap.values().takeAt(peerCandidate[rand]);
            peerCandidate[rand] = peerCandidate[peerNum - 1 - i];
            requestMap["Budget"] = 1;
            sendMessage(requestMap, p->getAddress(), p->getPort());
        }
    } else {
        peerBudget = budget / peerNum;
        int budgetRemain = budget - peerBudget * peerNum;
        int peerCandidate[peerNum];
        QSet<int> extraBudgetPeer;
        if (budgetRemain > 0) {
            for (int i = 0; i < peerNum; i++) peerCandidate[i] = i;
            for (int i = 0; i < budgetRemain; i++) {
                int rand = qrand() % (peerNum - i);
                extraBudgetPeer.insert(rand);
                peerCandidate[rand] = peerCandidate[peerNum - i];
            }
        }
        for (int i = 0; i < peerNum; i++) {
            Peer *p = peerMap.values().takeAt(i);
            requestMap["Budget"] = peerBudget + extraBudgetPeer.contains(i) ? 1 : 0;
            sendMessage(requestMap, p->getAddress(), p->getPort());
        }
    }
}

void ChatDialog::clickToDownload(QListWidgetItem *item) {
    QStringList itemContent = item->text().split("\n");
    QString fileName = itemContent[0];
    fileName = fileName.mid(11, fileName.length());
    QString metaFile = itemContent[1];
    metaFile = metaFile.mid(10, metaFile.length());
    QString origin = itemContent[2];
    origin = origin.mid(6, origin.length());
    qDebug() << fileName;
    qDebug() << metaFile;
    qDebug() << origin;
    ddFrom->setText(origin);
    ddMeta->setText(metaFile);
    ddName->setText(fileName);
    directDownloadFile();
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
        //qDebug() << "Rceive msg from neighbor: " << key << " " << map;
        //qDebug() << "We have " << peerMap.size() << " neighbours: " << peerMap.keys();

        if (map.contains("Search")) {
            receiveSearchRequest(map);
        } else if (map.contains("Dest")) {
            // Receive Private Message
            receivePrivateMessage(map, address, port);
        } else if (map.contains("Origin")) {
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
        if (!rumor.contains("LastIP") || !routingTable.contains(senderOrigin)) {
            insertRoutingTable(senderOrigin, address, port);
        }
        if (rumor.contains("LastIP")) {
            peerEdit->setText(
                    QHostAddress(rumor.value("LastIP").toUInt()).toString() + ":" + rumor.value("LastPort").toString());
            gotPeerReturnPressed();
        }
        rumor.insert("LastIP", address.toIPv4Address());
        rumor.insert("LastPort", port);
        QMap<quint16, QVariantMap> msg;
        int status = 2;
        if (statusMap.value(senderOrigin) == seqNo) {
            msg = messageMap.value(senderOrigin);
            status = seqNo + 1;
        }
        msg.insert(seqNo, rumor);
        messageMap.insert(senderOrigin, msg);
        statusMap[senderOrigin] = status;
        if (rumor.contains("ChatText")) {
            textview->append(rumor.value("ChatText").toString());
            sendRumorMessage(rumor);
        } else {
            // Forwarding routing message to all neighbour
            Peer *p = pickNeighbors();
            sendRumorMessage(rumor, p->getAddress(), p->getPort());
        }

    } else {
        sendStatus(statusMap, address, port);
    }
}

void ChatDialog::receivePrivateMessage(QMap<QString, QVariant> privateMsg, QHostAddress address, quint16 port) {
    QString dest = privateMsg.value("Dest").toString();
    quint32 hopLimit = privateMsg.value("HopLimit").toUInt();
    QString origin = privateMsg.value("Origin").toString();
    if (!routingTable.contains(origin)) {
        insertRoutingTable(origin, address, port);
    }
    if (dest == this->origin) {
        if (privateMsg.contains("ChatText")) {
            textview->append(privateMsg.value("ChatText").toString());
        } else if (privateMsg.contains("BlockRequest")) {
            receiveBlockRequest(privateMsg);
        } else if (privateMsg.contains("BlockReply")) {
            receiveBlockReply(privateMsg);
        } else if (privateMsg.contains("SearchReply")) {
            receiveSearchReply(privateMsg);
        }
        return;
    }
    if (!noForward && hopLimit > 0 && routingTable.contains(dest)) {
        privateMsg["HopLimit"] = hopLimit - 1;
        sendPrivateMessage(privateMsg, routingTable.value(dest).first, routingTable.value(dest).second);
    }
}

void ChatDialog::receiveBlockRequest(QMap<QString, QVariant> request) {
    qDebug() << "Receive Block Request !!!!!!!!!!!!!!!!!!!!!!!!!";
    QString origin = request.value("Origin").toString();
    QByteArray requestBlock = request.value("BlockRequest").toByteArray();
    QVariantMap reply;
    reply["Dest"] = origin;
    reply["Origin"] = this->origin;
    reply["HopLimit"] = 10;
    reply["BlockReply"] = requestBlock;
    if (fileBlockHash.contains(requestBlock)) {
        // Request regular file data block from uploaded file
        reply["Data"] = fileBlockHash.value(requestBlock);
    } else if (metafileList.contains(requestBlock)) {
        // Request hash of a blocklist metafile from uploaded file
        reply["Data"] = metafileList.value(requestBlock)["block"];
    } else if (downloadFileBlock.contains(requestBlock)) {
        // Request regular file data block from downloaded file
        reply["Data"] = downloadFileBlock.value(requestBlock);
    } else if (downloadMetafile.contains(requestBlock)) {
        // Request hash of a blocklist metafile from downloaded file
        reply["Data"] = downloadMetafile.value(requestBlock)["block"];
    } else {
        qDebug() << "No Hit Hash for " << requestBlock;
        return;
    }
    if (QCA::Hash("sha1").hash(reply.value("Data").toByteArray()).toByteArray() != requestBlock) return;
    sendPrivateMessage(reply, routingTable.value(origin).first, routingTable.value(origin).second);
}

void ChatDialog::receiveBlockReply(QMap<QString, QVariant> reply) {
    qDebug() << "Receive Block Reply !!!!!!!!!!!!!!!!!!!!!!!!!";
    QString origin = reply.value("Origin").toString();
    QByteArray replyData = reply.value("Data").toByteArray();
    QByteArray replyHash = reply.value("BlockReply").toByteArray();
    // Validate Block Reply
    if (QCA::Hash("sha1").hash(replyData).toByteArray() == replyHash) {
        qDebug() << "Valid Block!!!!!!";
        if (downloadFileBlock.contains(replyHash)) {
            // Receive a normal file block, store it
            QByteArray metaFile = downloadFileBlock.value(replyHash).value("belong").toByteArray();
            QSet<QByteArray> t = downloadingFile.value(metaFile);
            downloadFileBlock[replyHash]["block"] = replyData;
            t.remove(replyHash);
            if (!t.empty()) {
                if (t.size() == 1) {
                    QVariantMap request;
                    request["Dest"] = origin;
                    request["Origin"] = this->origin;
                    request["HopLimit"] = 10;
                    request["BlockRequest"] = t.toList().value(0);
                    sendPrivateMessage(request, routingTable.value(origin).first, routingTable.value(origin).second);
                }
                downloadingFile[metaFile] = t;
            } else {
                qDebug() << "start writing!";
                downloadingFile.remove(metaFile);
                QByteArray toWrite;
                QByteArray allBlocks = downloadMetafile[metaFile]["block"].toByteArray();
                int blockSize = allBlocks.size();
                for (int i = 0; i < blockSize; i += 20) {
                    QByteArray oneBlock = allBlocks.mid(i, 20);
                    toWrite.append(downloadFileBlock.value(oneBlock).value("block").toByteArray());
                }
                QFile writeFile(downloadMetafile[metaFile]["name"].toString());
                writeFile.open(QIODevice::WriteOnly);
                writeFile.write(toWrite);
                qint64 fileSize = writeFile.size();
                writeFile.close();
                insertFileView(downloadMetafile[metaFile]["name"].toString(),
                               QString::number(fileSize) + "B", metaFile.toHex(), "Downloaded", origin);
            }
        } else if (downloadMetafile.contains(replyHash)) {
            qDebug() << "Receive Metafile block !!!!";
            // Receive a metafile block, store it, and request all blocks of these metafile
            int totalBlock = replyData.size();
            downloadMetafile[replyHash]["block"] = replyData;
            for (int i = 0; i < totalBlock; i += 20) {
                // read the metafile block 20byte by 20byte
                QByteArray oneBlock = replyData.mid(i, 20);
                downloadFileBlock[oneBlock]["belong"] = replyHash;
                QSet<QByteArray> t = downloadingFile.value(replyHash);
                t.insert(oneBlock);
                downloadingFile[replyHash] = t;
                // send request for each block to get real file data.
                QVariantMap request;
                request["Dest"] = origin;
                request["Origin"] = this->origin;
                request["HopLimit"] = 10;
                request["BlockRequest"] = oneBlock;
                sendPrivateMessage(request, routingTable.value(origin).first, routingTable.value(origin).second);
            }
        } else {
            qDebug() << "Receive unknown block !!!";
        }
    } else {
        qDebug() << "Invalid Block!!!!!!!!!";
    }
}

void ChatDialog::receiveSearchReply(QMap<QString, QVariant> reply) {
    QVariantList fileNames = reply.value("MatchNames").toList();
    QVariantList fileIDs = reply.value("MatchIDs").toList();
    //QByteArray fileIDs = reply.value("MatchIDs").toByteArray();
    for (int i = 0; i < fileNames.length(); ++i) {
        QString fileName = fileNames.value(i).toString();
        auto *newItem = new QListWidgetItem();
        //newItem->setData(Qt::UserRole, i.key());
        QByteArray resultMetafile = fileIDs.value(i).toByteArray();
        //QByteArray resultMetafile = fileIDs.mid(20 * i, 20);
        if (!searchResultSet.contains(resultMetafile)) {
            searchResultSet.insert(resultMetafile);
            newItem->setText("File Name: " + fileName +
                             "\nMetafile: " + resultMetafile.toHex() +
                             "\nFrom: " + reply.value("Origin").toString());
            //newItem->setSizeHint(QSize(newItem->sizeHint().width(), 40));
            searchResult->addItem(newItem);
        }
    }
}

void ChatDialog::receiveSearchRequest(QMap<QString, QVariant> request) {
    QString origin = request.value("Origin").toString();
    if (origin == this->origin) return;
    QSet<QString> keywords = QSet<QString>::fromList(request.value("Search").toString().split(" "));
    qDebug() << "Search Keywords!!!!!" << keywords;
    QVariantList matchNames;
    QVariantList matchIDs;
    //QByteArray matchIDs;
    QHash<QByteArray, QVariantMap>::iterator i;
    for (i = metafileList.begin(); i != metafileList.end(); ++i) {
        QString oneName = i.value().value("name").toString();
        for (QString s : keywords) {
            if (oneName.contains(s)) {
                qDebug() << "!!!!!Found File " << oneName;
                matchNames.append(oneName);
                matchIDs.append(i.key());
            }
        }
    }
    for (i = downloadMetafile.begin(); i != downloadMetafile.end(); ++i) {
        QString oneName = i.value().value("name").toString();
        for (QString s : keywords) {
            if (oneName.contains(s)) {
                qDebug() << "!!!!!Found File " << oneName;
                matchNames.append(oneName);
                matchIDs.append(i.key());
            }
        }
    }
    QVariantMap searchReply;
    searchReply["Dest"] = origin;
    searchReply["Origin"] = this->origin;
    searchReply["SearchReply"] = request.value("Search").toString();
    searchReply["HopLimit"] = 10;
    searchReply["MatchNames"] = matchNames;
    searchReply["MatchIDs"] = matchIDs;
    sendPrivateMessage(searchReply, routingTable.value(origin).first, routingTable.value(origin).second);
    request["Budget"] = request.value("Budget").toUInt() - 1;
    floodSearchRequest(request);
}

void ChatDialog::sendMessage(QMap<QString, QVariant> msg, QHostAddress address, quint16 port) {
    QByteArray mapData;
    QDataStream stream(&mapData, QIODevice::WriteOnly);
    stream << msg;
    sock->writeDatagram(mapData, address, port);
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
    if (noForward && rumor.contains("ChatText")) return;
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
    if (qrand() % 2 != 0) { // 50% Possibility
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
    privateMsg.insert("Origin", this->origin);
    privateMsg.insert("ChatText", text);
    privateMsg.insert("HopLimit", 10);
    textview->append(text);
    sendPrivateMessage(privateMsg, routingTable.value(origin).first, routingTable.value(origin).second);
}

void ChatDialog::sendPrivateMessage(QMap<QString, QVariant> privateMsg, QHostAddress address, quint16 port) {
    QByteArray mapData;
    QDataStream stream(&mapData, QIODevice::WriteOnly);
    stream << privateMsg;
    //qDebug() << "Sending private msg: " << privateMsg;
    sock->writeDatagram(mapData, address, port);
}

void ChatDialog::sendRoutingMessage() {
    // Send routing to all neighbor per minute
    Peer *p = pickNeighbors();
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

void ChatDialog::openPrivateDialog(QListWidgetItem *item) {
    QString itemOrigin = item->data(Qt::UserRole).toString();
    PrivateDialog *subDialog = new PrivateDialog(itemOrigin);
    subDialog->show();
    connect(subDialog, SIGNAL(privateMsgSent(QString, QString)), this, SLOT(gotPrivateMsgEntered(QString, QString)));
}


void ChatDialog::gotAddFilePressed() {
    fileDialog = new QFileDialog();
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->show();
    if (!fileDialog->exec()) return;
    QStringList fileNames = fileDialog->selectedFiles();
    qDebug() << fileNames;
    for (int i = 0; i < fileNames.length(); i++) {
        QFile file(fileNames.at(i));
        QFileInfo fileInfo(file.fileName());
        if (!file.open(QIODevice::ReadOnly)) continue;
        QDataStream in(&file);
        QByteArray blockHashList;
        qint64 fileSize = file.size();
        qint64 blockNum = fileSize / 8192;
        blockNum += (fileSize % 8192) == 0 ? 0 : 1;
        for (qint64 index = 0; index < blockNum; index++) {
            QByteArray block = file.read(8192);
            QByteArray blockHash = QCA::Hash("sha1").hash(block).toByteArray();
            blockHashList.push_back(blockHash);
            fileBlockHash.insert(blockHash, block);
        }
        QByteArray metafileHash = QCA::Hash("sha1").hash(blockHashList).toByteArray();
        if (metafileList.contains(metafileHash)) continue; // File already existed
        // Update GUI:
        insertFileView(fileInfo.fileName(), QString::number(fileSize) + "B", metafileHash.toHex(), "Sharing", "Self");
        QVariantMap metafileMap;
        metafileMap.insert("name", fileInfo.fileName());
        metafileMap.insert("size", file.size());
        metafileMap.insert("block", blockHashList);
        metafileList.insert(metafileHash, metafileMap);
    }
}

void ChatDialog::insertFileView(QString name, QString size, QByteArray hash, QString status, QString origin) {
    QVariantMap file;
    file["name"] = name;
    file["size"] = size + "B";
    file["status"] = status;
    file["origin"] = origin;
    fileDisplay.insert(hash, file);
    QHash<QByteArray, QVariantMap>::iterator i;
    //fileView->clear();
    fileView->clearContents();
    fileView->setRowCount(0);
    for (i = fileDisplay.begin(); i != fileDisplay.end(); ++i) {
        int tableIndex = fileView->rowCount();
        fileView->setRowCount(tableIndex + 1);
        auto *nameItem = new QLineEdit();
        nameItem->setText(i.value().value("name").toString());
        nameItem->setReadOnly(true);
        fileView->setCellWidget(tableIndex, 0, nameItem);
        auto *sizeItem = new QLineEdit();
        sizeItem->setText(i.value().value("size").toString());
        sizeItem->setReadOnly(true);
        fileView->setCellWidget(tableIndex, 1, sizeItem);
        auto *metaItem = new QLineEdit();
        metaItem->setText(i.key().toHex());
        metaItem->setReadOnly(true);
        fileView->setCellWidget(tableIndex, 2, metaItem);
        auto *statusItem = new QLineEdit();
        statusItem->setText(i.value().value("status").toString());
        statusItem->setReadOnly(true);
        fileView->setCellWidget(tableIndex, 3, statusItem);
        auto *originItem = new QLineEdit();
        originItem->setText(i.value().value("origin").toString());
        originItem->setReadOnly(true);
        fileView->setCellWidget(tableIndex, 4, originItem);
    }
}

void ChatDialog::directDownloadFile() {
    QString fromOrigin = this->ddFrom->text();
    if (!routingTable.contains(fromOrigin)) {
        this->ddFrom->setText("Origin not existed in routing table yet!");
        return;
    }
    QByteArray requestMeta = QByteArray::fromHex(this->ddMeta->text().toLatin1());
    //if (downloadMetafile.contains(requestMeta) || metafileList.contains(requestMeta)) return;
    QVariantMap temp;
    temp["name"] = this->ddName->text();
    downloadMetafile[requestMeta] = temp;
    QVariantMap request;
    request["Dest"] = fromOrigin;
    request["Origin"] = this->origin;
    request["HopLimit"] = 10;
    request["BlockRequest"] = requestMeta;
    sendPrivateMessage(request, routingTable.value(fromOrigin).first, routingTable.value(fromOrigin).second);
    ddFrom->clear();
    ddMeta->clear();
    ddName->clear();
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
    QCA::Initializer initializer;

    // Create an initial chat dialog window
    ChatDialog dialog;
    dialog.show();

    // Enter the Qt main loop; everything else is event driven
    return app.exec();
}
