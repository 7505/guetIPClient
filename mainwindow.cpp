#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setMenu();
    setShowStyle();//设置界面
    openPwdFile();

    openclient();//尝试对接

  //  writePwdFile();
    //openPwdFile();

    this->setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    //this->setFixedSize(this->width(), this->height());


}

void MainWindow::setMenu()
{
    QAction *about;
    about = new QAction(tr("&about"), this);
    about->setStatusTip(QStringLiteral("点击打开帮助文档"));

    QAction *statistics;
    statistics = new QAction(QStringLiteral("信息统计"), this);
    statistics->setStatusTip(QStringLiteral("点击进行数据统计"));
    QMenu *helpMenu;
    //this->addToolBar();
    helpMenu = this->menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(about);
    connect(about, SIGNAL(triggered()), this, SLOT(showHelp()));


}

void MainWindow::showHelp()
{
    QMessageBox::information(NULL,
                             QStringLiteral("帮助信息"),
                             QStringLiteral("该程序为桂林电子科技大大学IP拨号软件                  \
                                            较学校发布的版本，本版本支持记忆多个账号  \
                                            如需保存多个账号，仅需要点击连接之前勾选保存账号复选框  \
                                            如需删除多余账号，仅需要右键选择删除按钮  \
                                            后期将会不断的完善，修复BUG                            \
                                            软件就对安全、不会对信息进行泄露。账号保存在本地安装目录下programdata中（加密）\
                                            本软件完全开源，源代码下载地址：https://github.com/7505/guetIPClient     \
                                            对于软件使用的任何问题，可联系woshidahuaidan2011@hotmail.com"),
                                            QMessageBox::Yes );
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openPwdFile()
{
    QFile pwdFile("./programedata");
    if (!pwdFile.open(QIODevice::ReadWrite ))
    {
        return;

    }

    QTextStream out(&pwdFile);
    nameList.clear();


    while (!out.atEnd()) {
        QString line = out.readLine();
        struct namePwd tmp;
        tmp.uNane  = QByteArray::fromBase64(line.section(',',0,0).toLocal8Bit(),QByteArray::OmitTrailingEquals);
        tmp.passWD = QByteArray::fromBase64(line.section(',',1,1).toLocal8Bit(),QByteArray::OmitTrailingEquals);
        tmp.flag   =  line.section(',',2,2).toInt();
        qDebug()<<tmp.uNane <<tmp.passWD <<tmp.flag ;
        nameList.append(tmp);
        userName->addItem(tmp.uNane);
        if(tmp.flag == 1)
            userName->setCurrentText(tmp.uNane);

    }
    pwdFile.close();

}


void MainWindow::writePwdFile()
{
    //qDebug()<<userName->currentText()<<password->text();

    QByteArray name =userName->currentText().toLocal8Bit();
    QByteArray pwd  = password->text().toLocal8Bit();

    struct namePwd currrentName;
    currrentName.passWD = pwd;
    currrentName.uNane  = name;
    currrentName.flag   = 1;
    struct namePwd tmp;

    for (int i = 0; i < nameList.size(); ++i) {
        tmp = nameList.at(i);
        if(tmp.flag ==1)
        {
            nameList.removeAt(i);
            tmp.flag = 0;
            nameList.append(tmp);
        }


    }

   // for()
    int foundFlag = 0;
    for (int i = 0; i < nameList.size(); ++i) {
        tmp = nameList.at(i);
        if((tmp.uNane == name )&& (tmp.passWD == pwd )){//找到了
            if(tmp.flag ==0)
            {
                nameList.removeAt(i);
                tmp.flag = 1;
                nameList.append(tmp);
                foundFlag =1;
                break;
            }
            return;
        }
        if((tmp.uNane == name )&& (tmp.passWD != pwd )){//找到了,更正密码
            nameList.removeAt(i);
            break;
        }

     }

    if(foundFlag != 1)
    {
        nameList.append(currrentName);
        userName->addItem(currrentName.uNane);
    }


   // QByteArray nameSave =name.toBase64(QByteArray::OmitTrailingEquals);
    //QByteArray pwdSave =pwd.toBase64(QByteArray::OmitTrailingEquals);


    QFile pwdFile("./programedata");
    if (pwdFile.open(QIODevice::ReadWrite ))
    {
        pwdFile.remove();

    }
    pwdFile.reset();
   // QFile pwdFile("./programedata");
    if (!pwdFile.open(QIODevice::ReadWrite ))
    {
        return;

    }

    QTextStream in(&pwdFile);

    for (int i = 0; i < nameList.size(); ++i) {
        tmp = nameList.at(i);
        in<<tmp.uNane.toBase64(QByteArray::OmitTrailingEquals)
         <<","
        <<tmp.passWD.toBase64(QByteArray::OmitTrailingEquals)
         <<","
        <<tmp.flag
        <<endl;

     }
   // in<<nameSave<<","<<pwdSave<<endl;
   // pwdFile.write(nameSave.data());
   // pwdFile.write(",");
   // pwdFile.write(pwdSave.data());
    pwdFile.close();

}

//对接成功后需要申请TCP链接
//还需要申请发送密码的UDP链接
void MainWindow::connectSuccess()
{
    sendPWClient = new QTcpSocket(this);
    sendPWClient->close();
    sendPWClient->open(QIODevice::ReadWrite);
    sendPWClient->connectToHost(QHostAddress(hostIpAdd), SENDTCPPORT);//80端口

    sendPWServer = new QTcpServer();

    qDebug()<<sendPWServer->listen(QHostAddress::AnyIPv4, sendPWClient->localPort());

    connect(sendPWServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));



    sendUdpPW = new QUdpSocket(this); //发送密码client
    receiveUdpPW = new QUdpSocket(this);//接受密码server


     if (sendPWClient->waitForConnected())
     {
        currentIP = sendPWClient->localAddress();
        qDebug()<<currentIP;
     }





}


void MainWindow::acceptConnection()
{
    clientConnection = sendPWServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readClient()));
}
void MainWindow::readPWClient()
{
    qDebug()<<clientConnection->readAll();
}


void MainWindow::setShowStyle()
{

    //QStackedLayout campusInterface = new QStackedLayout;

    //设置布局
    QGridLayout *campusInterface = new QGridLayout();//网格布局

    QLabel *guetPic = new QLabel();//显示学校图标
    QLabel *newNotice = new  QLabel();//最新公告
    balance = new  QLabel();//余额
    QHBoxLayout *loginInformation =  new QHBoxLayout();//用户名等登录信息
    QHBoxLayout *selectInformation =  new QHBoxLayout();//运营商选择
    QHBoxLayout *clientButton =  new QHBoxLayout();//点击按钮

     //设置图标
    QImage  picture;
    picture.load(QStringLiteral("./ico/guet.png"));           // load picture
   // qDebug()<<picture.data();
    guetPic->setPixmap(QPixmap::fromImage(picture));

    newNotice->setText(QStringLiteral("最新公告："));
    balance->setText(QStringLiteral("剩余金额："));

    //设置登录条目
    QLabel *userNameLabel= new QLabel();
    QLabel *passwordLabel = new QLabel();
    userName = new QComboBox();
    password = new QLineEdit();
    savePW  = new QCheckBox();
    userNameLabel->setText(QStringLiteral("用户名"));
    passwordLabel->setText(QStringLiteral("密码"));
    savePW->setText(QStringLiteral("保存密码"));
    savePW->setCheckState(Qt::Checked);

    userName->setEditable(true);//设置用户名可编辑
    //userName->setT setPlaceholderText(QStringLiteral("请输入用户名"));
    password->setPlaceholderText(QStringLiteral("请输入密码"));
    password->setEchoMode(QLineEdit::Password);

    userNameLabel->setMinimumWidth(40);
    userNameLabel->setMaximumWidth(40);
    userName->setMinimumWidth(120);
    userName->setMaximumWidth(120);
    passwordLabel->setMinimumWidth(30);
    passwordLabel->setMaximumWidth(30);
    password->setMinimumWidth(120);
    password->setMaximumWidth(120);


    loginInformation->addWidget(userNameLabel);
    loginInformation->addWidget(userName);
    loginInformation->addWidget(passwordLabel);
    loginInformation->addWidget(password);
    loginInformation->addWidget(savePW);
    loginInformation->setSpacing(10);

    //设置选择条目
    QLabel *serviceProvider= new QLabel(); //运营商名字
    QLabel *connectName = new QLabel();
    serviceProvider->setText(QStringLiteral("运营商"));
    connectName->setText(QStringLiteral("连接"));

    QComboBox *Provider = new QComboBox();
    QComboBox *connect = new QComboBox();
    Provider->addItem(QStringLiteral("电信"));
    Provider->addItem(QStringLiteral("移动"));
    Provider->addItem(QStringLiteral("联通"));
    connect->setEditable(true);

    serviceProvider->setMinimumWidth(40);
    serviceProvider->setMaximumWidth(40);
    Provider->setMinimumWidth(120);
    Provider->setMaximumWidth(120);
    connectName->setMinimumWidth(30);
    connectName->setMaximumWidth(30);
    connect->setMinimumWidth(120);
    connect->setMaximumWidth(120);

    selectInformation->addWidget(serviceProvider);
    selectInformation->addWidget(Provider);
    selectInformation->addWidget(connectName);
    selectInformation->addWidget(connect);
    selectInformation->setSpacing(10);

    //底下的几个按钮
    connectButton = new QPushButton(QStringLiteral("连接"));
    breakButton = new QPushButton(QStringLiteral("断开"));
    queryButton = new QPushButton(QStringLiteral("查询"));
    chineseButton = new QPushButton(QStringLiteral("中文版"));
    englishButton = new QPushButton(QStringLiteral("English version"));
    clientButton->addWidget(connectButton);
    clientButton->addWidget(breakButton);
    clientButton->addWidget(queryButton);
    clientButton->addWidget(chineseButton);
    clientButton->addWidget(englishButton);

    campusInterface->addWidget(guetPic,0,0,1,5,Qt::AlignTop);
    campusInterface->addWidget(newNotice,1,0,1,5,Qt::AlignTop|Qt::AlignLeft);
    campusInterface->addWidget(balance,2,0,1,5,Qt::AlignTop|Qt::AlignLeft);
    campusInterface->addLayout(loginInformation,3,0,1,5,Qt::AlignTop|Qt::AlignLeft);
    campusInterface->addLayout(selectInformation,4,0,1,5,Qt::AlignTop|Qt::AlignLeft);
    campusInterface->addLayout(clientButton,5,0,1,5,Qt::AlignBottom|Qt::AlignLeft);


    QWidget *widget = new QWidget();
    //this->setLayout(gradLay);
    widget->setLayout(campusInterface);
   // MainWindow.setCentralWidget(widget);
    this->setCentralWidget(widget);
   // selectQuantity->setStyleSheet("{background-color:transparent}");

    //for test
  //  userName->addItem("1401420210");
    //password->setText("1234567");

    //用户名添加右键菜单
    userName->setContextMenuPolicy(Qt::CustomContextMenu);



    //设置一下槽函数
     QObject::connect(connectButton, SIGNAL(clicked()), this, SLOT(startConnect()));//心跳包
     QObject::connect(breakButton, SIGNAL(clicked()), this, SLOT(stopConnect()));//停止上网
     QObject::connect(userName, SIGNAL(currentTextChanged(const QString &)), this, SLOT(nameChange(const QString &)));//停止上网
     QObject::connect(userName,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(userRightmouse(const QPoint &)));
     
     
     
     //设置心跳包
     heartBeat = new QTimer;
     QObject::connect(heartBeat, SIGNAL(timeout()), this, SLOT(heartBeatSend()));
     

}

void MainWindow::userRightmouse(const QPoint &p)
{
    QMenu *cmenu = new QMenu( userName);
    QAction *action1 = cmenu->addAction("copy");
    QAction *action2 = cmenu->addAction("paste");
    QAction *action3 = cmenu->addAction("delete");
    QObject::connect(action1, SIGNAL(triggered(bool)), this, SLOT(userRightmouseCopy(bool)));
    QObject::connect(action2, SIGNAL(triggered(bool)), this, SLOT(userRightmousePaste(bool)));
    QObject::connect(action3, SIGNAL(triggered(bool)), this, SLOT(userRightmouseRemove(bool)));
    cmenu->exec(QCursor::pos());
}

void MainWindow::userRightmouseCopy(bool ok)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = userName->currentText();
    clipboard->setText(originalText);
}

void MainWindow::userRightmousePaste(bool ok)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();
    userName->setCurrentText(originalText);
    //clipboard->setText(newText);

}
void MainWindow::userRightmouseRemove(bool ok)
{
    QByteArray name =userName->currentText().toLocal8Bit();
    QByteArray pwd  = password->text().toLocal8Bit();


    struct namePwd tmp;
    int isfound =0;

    for (int i = 0; i < nameList.size(); ++i) {
        tmp = nameList.at(i);
        if(tmp.uNane == name )//找到了
         {
                nameList.removeAt(i);
                userName->removeItem(userName->currentIndex());
                isfound =1;
                break;
         }


     }

    if(isfound == 0)
    {
        return;
    }




   // QByteArray nameSave =name.toBase64(QByteArray::OmitTrailingEquals);
    //QByteArray pwdSave =pwd.toBase64(QByteArray::OmitTrailingEquals);



    QFile pwdFile("./programedata");
    if (pwdFile.open(QIODevice::ReadWrite ))
    {
        pwdFile.remove();

    }
    pwdFile.reset();
   // QFile pwdFile("./programedata");
    if (!pwdFile.open(QIODevice::ReadWrite ))
    {
        return;

    }

    QTextStream in(&pwdFile);

    for (int i = 0; i < nameList.size(); ++i) {

        tmp = nameList.at(i);
        qDebug()<<tmp.uNane;
        in<<tmp.uNane.toBase64(QByteArray::OmitTrailingEquals)
         <<","
        <<tmp.passWD.toBase64(QByteArray::OmitTrailingEquals)
         <<","
        <<tmp.flag
        <<endl;

     }
    pwdFile.close();

}

void MainWindow::nameChange(const QString &s)
{
    //qDebug()<<s;
    struct namePwd tmp;
    for (int i = 0; i < nameList.size(); ++i) {
        tmp = nameList.at(i);
        if(tmp.uNane == s ){
            password->setText(tmp.passWD);
            return;
        }

     }
    password->clear();

}

void MainWindow::heartBeatSend()
{


    heartBeat->setInterval(40000);
    char charStore[500]= {0x82,0x23,0x1e};
    charStore[0x03] = keyNumber&0x00ff;
    charStore[0x04] = (keyNumber>>8)&0x00ff;
    charStore[0x1b] = currentName.size();


     //qDebug()<<(15+currentName.size());
     for(int i=0x1f;i< (0x1f+currentName.size()); i++)
        charStore[i] = currentName.at(i-0x1f).cell();
     charStore[0x1f+currentName.size()] = 1;
     charStore[0x1f+currentName.size()+4] = 0x62;
     charStore[0x1f+currentName.size()+4+1] = 0x0c;
     charStore[0x1f+currentName.size()+4+1+4] = 0xbd;
     charStore[0x1f+currentName.size()+4+1+5] = 0xf1;
     charStore[0x1f+currentName.size()+4+1+6] = 0xcc;
     charStore[0x1f+currentName.size()+4+1+7] = 0xec;
     charStore[0x1f+currentName.size()+4+1+8] = 0xcc;
     charStore[0x1f+currentName.size()+4+1+9] = 0xec;
     charStore[0x1f+currentName.size()+4+1+10] = 0x6c;
     charStore[0x1f+currentName.size()+4+1+11] = 0xf8;
     charStore[0x1f+currentName.size()+4+1+12] = 0xd5;
     charStore[0x1f+currentName.size()+4+1+13] = 0xe6;
     charStore[0x1f+currentName.size()+4+1+14] = 0xba;
     charStore[0x1f+currentName.size()+4+1+15] = 0xc3;

     charStore[0x1f+currentName.size()+4+1+15+1] = 1;
     charStore[0x1f+currentName.size()+4+1+15+1+4] = 0x20;

     ::memset(charStore+0x1f+currentName.size()+4+1+15+1+4+1,0xff,
             500-(0x1f+currentName.size()+4+1+15+1+4+1));

    // qDebug()<<"asssssssssssss";

     udpSocket->writeDatagram(charStore,500,QHostAddress(hostIpAdd),CONNECTSENDPORT);
    //qDebug()<<"bbbbbbbbbbbbbb";


/**/
}


void MainWindow::checkConnect()
{
    checkTimer->stop();
    if(connectFlag == 0)
    {
        statusBar()->showMessage(QStringLiteral("与服务器对接失败，请确认网线是否正常连接，或重启软件重新尝试。"));
        connectSuccess();//配置TCP链接

    }


}
void MainWindow::openclient()
{

   connectFlag = 0;


   QUdpSocket  *client = new QUdpSocket(this);

   client->close();
   client->open(QIODevice::ReadWrite);
   client->bind(CONNECTRECEIVEPORT);
  // client->connectToHost(QHostAddress(hostIpAdd), CONNECTSENDPORT);





   qDebug()<<client->localPort();
   udpSocket = new QUdpSocket(this);

   qDebug()<<udpSocket->bind(QHostAddress::AnyIPv4,CONNECTRECEIVEPORT,
                             QAbstractSocket::ShareAddress);

   connect(udpSocket, SIGNAL(readyRead()),
                 this, SLOT(readPendingDatagrams()));


   //char charStore[500] = {0x82,0x23,0x05};
   char charStore[500] = {0x82,0x23,0x05};
   client->writeDatagram(charStore,500,QHostAddress(hostIpAdd),CONNECTSENDPORT);

  //写入成功后需要坚持
   checkTimer = new QTimer(this);
   connect(checkTimer, SIGNAL(timeout()), this, SLOT(checkConnect()));
   checkTimer->start(1000);


}

void MainWindow::readPendingDatagrams()
{

    while (udpSocket->hasPendingDatagrams()) {

                QByteArray datagram;
                datagram.resize(udpSocket->pendingDatagramSize());
                QHostAddress sender;
                quint16 senderPort;

                udpSocket->readDatagram(datagram.data(), datagram.size(),
                                        &sender, &senderPort);
                if(sender == QHostAddress(hostIpAdd) &&
                    senderPort == CONNECTSENDPORT    &&
                     (int)datagram.at(2)== 6 )
                {
                    statusBar()->showMessage(QStringLiteral("与服务器对接成功，可正常拨号连接"));
                    connectFlag = 1;
                    connectSuccess();//配置TCP链接
                }

                if(sender == QHostAddress(hostIpAdd) &&
                    senderPort == CONNECTSENDPORT    &&
                     (int)datagram.at(2)== 0x1f)//心跳接受
                {

                    QString balanceString;
                  //  QString flowString;


                    for(int i =(0x0B+8);i>=0x0B;i--)
                    {
                      //  flowString.append(datagram.at(i));
                        balanceString.append(datagram.at(i+8));
                        //qDebug()<<datagram.size() << datagram.at(i).toHex()<<(unsigned char )datagram.at(i+8);


                    }
                     // QString flowStringCopy = flowString.toLocal8Bit().toHex();
                      QString balanceStringCopy =balanceString.toLocal8Bit().toHex() ;
                      bool a;
                      //long long int fl= flowStringCopy.toLongLong(&a,16);
                      long long int bl= balanceStringCopy.toLongLong(&a,16);
                      //double *pf = (double *)&fl;
                      double *pb = (double *)&bl;
                      QString blenceMoney ;
                      blenceMoney.setNum(*pb);

                    //  qDebug()<<bl<<(double)*pb;
                      balance->setText(QStringLiteral("剩余金额：")+blenceMoney);
                }



        }

}


void MainWindow::startConnect()
{
    if(userName->currentText().isEmpty() ||password->text().isEmpty())
    {


        QMessageBox::information(NULL,
                                 QStringLiteral("提示信息"),
                                 QStringLiteral("密码或用户名不许为空。"),
                                                QMessageBox::Yes );
      return;
    }


    if(savePW->checkState()== Qt::Checked)
        writePwdFile();

    sendUdpPW->close();
    sendUdpPW->open(QIODevice::ReadWrite);
    sendUdpPW->bind(RECEIVEPWPORT);//绑定发送端口

    qDebug()<<sendUdpPW->localPort();



    receiveUdpPW->close();
    qDebug()<<receiveUdpPW->bind(QHostAddress::AnyIPv4,RECEIVEPWPORT,
                              QAbstractSocket::ShareAddress);

    connect(receiveUdpPW, SIGNAL(readyRead()),
                  this, SLOT(receivePWudp()));


    //char charStore[500] = {0x82,0x23,0x05};

    //读取用户输入信息

     currentName = userName->currentText();
    //QString currentPW   = password->text();
    //qDebug()<<currentName<<currentPW<<currentName.size()<<currentName.at(0).cell()- 0xA;



    char charStore[300]= {0x82,0x23,0x1f,0,0,0,0,0,0,0,0,
                           currentName.size(),0,0,0};

     qDebug()<<(15+currentName.size());
     for(int i=15;i< (15+currentName.size()); i++)
        charStore[i] = currentName.at(i-15).cell()- 0xA;
     charStore[15+currentName.size()] = 0x13;
     charStore[15+currentName.size()+23] = 1;
     charStore[15+currentName.size()+23+4] = 0x61;
     charStore[15+currentName.size()+23+4+11] = 0xf0;
     charStore[15+currentName.size()+23+4+12] = 0x3f;
     charStore[15+currentName.size()+23+4+13] = 1;
     charStore[15+currentName.size()+23+4+13+4] = 0x62;
     ::memset(charStore+(15+currentName.size()+23+4+13+4+1),0xff,
             300-(15+currentName.size()+23+4+13+4+1));


     sendUdpPW->writeDatagram(charStore,300,QHostAddress(hostIpAdd),SENDPWPORT);
     //qDebug()<<sendUdpPW->localAddress();



}

void MainWindow::receivePWudp()
{
    int statusFlag = 0;
    QByteArray datagram;

    while (receiveUdpPW->hasPendingDatagrams()) {


                datagram.resize(receiveUdpPW->pendingDatagramSize());
                QHostAddress sender;
                quint16 senderPort;

                receiveUdpPW->readDatagram(datagram.data(), datagram.size(),
                                        &sender, &senderPort);
                if(sender == QHostAddress(hostIpAdd) &&
                    senderPort == SENDPWPORT    &&
                     (int)datagram.at(2)== 0x20)//密码第一次连接
                {

                    statusFlag = 1;
                   // qDebug()<<sender;

                }

                if(sender == QHostAddress(hostIpAdd) &&
                    senderPort == SENDPWPORT    &&
                     (int)datagram.at(2)== 0x22 )//密码第二次连接
                {
                   //QString info;
                  // info =  data.append();

                   int getCode = (int)datagram.at(3);
                   switch(getCode)
                   {
                        case 0:
                            statusBar()->showMessage(QStringLiteral("开放IP成功。"));
                            heartBeat->start(4);//40s的心跳包
                         break;

                        case 0x63:
                            statusBar()->showMessage(QStringLiteral("用户名或密码错误，请重新输入。"));
                            break;
                        case 0x14:
                            statusBar()->showMessage(QStringLiteral("您的余额不足，请及时充值。"));
                       break;
                        default:
                            QString tmp;
                            tmp.setNum(getCode,16);
                            statusBar()->showMessage(QStringLiteral("连接错误，错误编码:0x")+tmp);
                            break;


                   }
                   //成功后需要设置开启心跳包

                   statusFlag = 2;
                }


                if(sender == QHostAddress(hostIpAdd) &&
                    senderPort == SENDPWPORT    &&
                     (int)datagram.at(2)== 0x24 )//关闭客户端
                {
                   //QString info;
                  // info =  data.append();
                   statusBar()->showMessage(QStringLiteral("IP已关闭成功。"));


                   statusFlag = 3;
                }







        }

        if(statusFlag == 1)//需要发送第二次申请
        {


            //数据提取
            QString data;
            data.append(datagram.at(15+userName->currentText().size()+23+4+2));
            data.append(datagram.at(15+userName->currentText().size()+23+4+1));
            QString datagramCopy = data.toLocal8Bit().toHex();

            bool a;
            unsigned short number= datagramCopy.toInt(&a,16)-0x0d10;
            keyNumber = number+0x05DC;
            qDebug()<<number<<keyNumber;
           // number = 0x1415-0xd10;
            data.clear();
            data.setNum(number,10);
            data += password->text();
            data = data.toUpper();
            qDebug()<<data;

            //第一次MD5

           // QString md5;
           // QString pwd="F54721000360117";
            QByteArray firstMD5;
            firstMD5 = QCryptographicHash::hash ( data.toLocal8Bit(), QCryptographicHash::Md5 );
           // md5.append(bb.toHex());
            qDebug()<<firstMD5.toHex();
            data.clear();

            //第二次MD5
            qDebug()<<firstMD5.toHex().left(5);
            data = (firstMD5.toHex().left(5)+userName->currentText());
            data = data.toUpper();
            qDebug()<<data.size()<<data.toLocal8Bit();
           // data= "f54721000360117";
            QByteArray secondMD5;
            secondMD5 = QCryptographicHash::hash ( data.toLocal8Bit(), QCryptographicHash::Md5 );
            qDebug()<<secondMD5.toHex();


            data.clear();

          //  QString test;
            data = secondMD5.toHex().left(30);
        //   test.toLocal8Bit()

            //开始发送第二次密码
            char charStore[300]= {0x82,0x23,0x21,0,0,0,0,0,0,0,0,
                                   0x13,0,0,0};

            //密匙
            charStore[0x22]=0x1e;
            for(int i =0x26; i<=0x43;i++)
            {

                if(data.at(i-0x26).toLatin1()>='a' && data.at(i-0x26).toLatin1()<='z')
                  charStore[i] =data.at(i-0x26).toUpper().cell();
                else
                  charStore[i] =data.at(i-0x26).cell();



            }
            //mac
            charStore[0x44] = 0x11;

            QStringList macList = get_mac();
            QString myMac = macList.at(0);//随机选取一个MAC
            qDebug()<< myMac;

            for(int i =0x48; i<=0x58;i++)
            {

                if(myMac.at(i-0x48).toLatin1()>='a' && myMac.at(i-0x48).toLatin1()<='z')
                  charStore[i] =myMac.at(i-0x48).toUpper().cell();
                else
                  charStore[i] =myMac.at(i-0x48).cell();

            }

             charStore[0x63]=0xf0;
             charStore[0x64]=0x3f;
             charStore[0x65]=1;
             charStore[0x69]=0x62;
             ::memset(charStore+0x6a,0xff,300-0x6a);
             sendUdpPW->writeDatagram(charStore,300,QHostAddress(hostIpAdd),SENDPWPORT);

        }

}

void MainWindow::stopConnect()
{
    char charStore[300]= {0x82,0x23,0x23};
    charStore[11] = 0x13;
    charStore[34] = 0x13;
    charStore[57] = 1;
    charStore[61] = 0x61;
    charStore[72] = 0xf0;
    charStore[73] = 0x3f;
    charStore[74] = 1;
    charStore[78] = 0x62;

    ::memset(charStore+79,0xff,
             300-79);

    // qDebug()<<"asssssssssssss";

     sendUdpPW->writeDatagram(charStore,300,QHostAddress(hostIpAdd),SENDPWPORT);


}
QStringList MainWindow::get_mac()
{
    QStringList mac_list;
    QString strMac;
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for (int i=0; i<ifaces.count(); i++)
    {
        QNetworkInterface iface = ifaces.at(i);
        //过滤掉本地回环地址、没有开启的地址
        if (iface.flags().testFlag(QNetworkInterface::IsUp)
                && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)
                && iface.flags().testFlag(QNetworkInterface::IsRunning) )
        {
            //过滤掉虚拟地址
         //   qDebug()<<iface.humanReadableName()<<iface.allAddresses();

            if (!(iface.humanReadableName().contains("VMware",Qt::CaseInsensitive)))
            {

                 strMac = iface.hardwareAddress();
                 mac_list.append(strMac);

            }
        }
    }
    return mac_list;
}



//关闭是最小化到托盘
void MainWindow::closeEvent(QCloseEvent *event)
{
    //TODO: 在退出窗口之前，实现希望做的操作
    this->hide();
    system_tray = new QSystemTrayIcon();
         //放在托盘提示信息、托盘图标
    system_tray ->setToolTip(QStringLiteral("双击打开界面,右键可选择关闭程序，单击可查询余额"));
    system_tray ->setIcon(QIcon("./ico/icon.png"));


    event->ignore();

   //// if(system_tray->isVisible())
  ///      return;

    system_tray->show();

    system_tray->showMessage(QStringLiteral("温馨提示"), QStringLiteral("guetIPClient已隐藏到托盘，\
                                                                            双击打开界面,右键可选择关闭程序，单击可查询余额"));

    //设置右键
    QMenu   *menu = new QMenu(this);
    QAction *action_show = new QAction(this);
    QAction *action_quit = new QAction(this);
    action_show->setText(QStringLiteral("显示"));
    action_quit->setText(QStringLiteral("退出"));
    menu->addAction(action_show);
    menu->addAction(action_quit);

    system_tray->setContextMenu(menu);

    connect(action_show, SIGNAL(triggered(bool)), this, SLOT(showWidget(bool)));
    connect(action_quit, SIGNAL(triggered(bool)), this, SLOT(quitWidget(bool)));

    connect(system_tray , SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconIsActived(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::showWidget(bool ok)
{
    system_tray->hide();
    this->show();
}
void MainWindow::quitWidget(bool ok)
{
      QApplication::exit();
}

void MainWindow::iconIsActived(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        //点击托盘显示窗口
        case QSystemTrayIcon::Trigger:
        {
         //、、showNormal();
            system_tray ->showMessage(QStringLiteral("余额查询"), balance->text());
            break;
        }
         //双击托盘显示窗口
        case QSystemTrayIcon::DoubleClick:
        {

             system_tray->hide();
             this->show();
            break;
         }
         //右键
         case QSystemTrayIcon::Context:
         {
            // showNormal();
            break;
         }
         default:
            break;
     }
}

