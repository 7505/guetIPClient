#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>

#include <QProcess>
#include <QMainWindow>
#include<QLabel>
#include<QGridLayout>
#include<QPicture>
#include<QPushButton>
#include<QGridLayout>
#include<QLineEdit>
#include<QMessageBox>

#include <QThread>
#include<QPixmap>
#include<QStackedLayout>
#include <QFileDialog>
#include <QDebug>
#include <QSlider>
#include <QPicture>
#include<QCheckBox>
#include<QComboBox>
#include<QUdpSocket>
#include<QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QList>
#include <QClipboard>
#include <QCloseEvent>
#include <QSystemTrayIcon>


#define CONNECTSENDPORT (5301)
#define CONNECTRECEIVEPORT (5201)
#define SENDTCPPORT (80)
#define SENDPWPORT (5300)
#define RECEIVEPWPORT (5200)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void setShowStyle();

private:
    QComboBox *userName; //用户名
    QLineEdit *password ;//密码
    QCheckBox  *savePW ;
    QLabel *balance; //剩余余额

    QPushButton *connectButton;
    QPushButton *breakButton;
    QPushButton *queryButton;
    QPushButton *chineseButton;
    QPushButton *englishButton;

    QUdpSocket *udpSocket;


    int connectFlag;//0没有与服务器连接成功，1与服务器连接成功



    QTimer *checkTimer ;//坚持是否连接成功

    QTcpSocket *sendPWClient;//TCP链接，是否没用
    QTcpServer *sendPWServer;//接受tcp返回信息
    QTcpSocket *clientConnection;

    QUdpSocket *sendUdpPW;//发送密码UDP客户端
    QUdpSocket *receiveUdpPW;//发送密码UDP客户端

    QTimer  *heartBeat;
    
    
    unsigned short keyNumber;
    QString currentName;//当前用户名


private:
    void openclient();

    void connectSuccess();

    QStringList get_mac();
    void openPwdFile();
    void writePwdFile();
    void setMenu();
private slots:
    void readPendingDatagrams();
    void checkConnect();
    void acceptConnection();
    void readPWClient();


    void startConnect();

    void receivePWudp();
    
    void heartBeatSend();//心跳发送包

    void stopConnect();//心跳发送包

    void nameChange(const QString &);
   // void removeUser(bool);
    
    void userRightmouse(const QPoint &p);
    void userRightmouseRemove(bool);
    void userRightmouseCopy(bool);
    void userRightmousePaste(bool);
    void showHelp();
    

protected:
    QString hostIpAdd="172.16.1.1";

    QHostAddress   currentIP;



    struct namePwd{
        QByteArray uNane;
        QByteArray passWD;
        int flag;
    };
    QList<struct namePwd> nameList;

protected:
      void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
