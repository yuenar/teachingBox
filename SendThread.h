/**
  *@brief       发送线程声明文件
  *@author      yuenar
  *@date        16-09-20
  *@version     0.11
  *@note        @16-07-15 yuenar v0.10 初版，增加TCP通信部分基础函数
  *@note        @16-09-20 yuenar v0.11 修改线程调用函数
  *@copyright   siasun.co
  */
#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QtNetwork>

class SendThread : public QObject
{
    Q_OBJECT
public:
    explicit SendThread(QObject *parent = 0);
    ~SendThread();
    void DoSetup(QThread &cThread);
private:
    QTcpSocket *sentSocket;
signals:
    void sentRdata(const QString & str);//发送的指令
public slots:
    void writer();//写手线程事件循环
    void connectHost(const QString &host, int port);//建立连接
    void disconnectHost();//关闭连接
    void sentCmd(const QString & cmd);//发送命令
};

#endif // SENDTHREAD_H
