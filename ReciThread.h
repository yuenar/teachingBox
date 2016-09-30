/**
  *@brief       接收线程声明文件
  *@author      yuenar
  *@date        16-09-20
  *@version     0.11
  *@note        @16-07-15 yuenar v0.10 初版，增加TCP通信部分基础函数
  *@note        @16-09-20 yuenar v0.11 修改线程调用函数
  *@copyright   siasun.co
  */
#ifndef RECITHREAD_H
#define RECITHREAD_H

#include <QThread>
#include <QtNetwork>

class ReciThread: public QObject
{
    Q_OBJECT
public:
    explicit ReciThread(QObject *parent = 0);
    ~ReciThread();
    void DoSetup(QThread &cThread);//与主线程分离
private:
    QTcpSocket *receiveSocket;
signals:
    void receiveData(const QString & data);//发送收到的数据
public slots:
    void listener();//听者线程事件循环
    void connectHost(const QString &host, int port);//建立连接
    void disconnectHost();//关闭连接
};

#endif // RECITHREAD_H
