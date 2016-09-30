/**
  *@brief       接收线程实现文件
  *@author      yuenar
  *@date        16-09-20
  *@version     0.11
  *@note        @16-07-15 yuenar v0.10 初版，增加TCP通信部分基础函数
  *@note        @16-09-20 yuenar v0.11 修改线程调用函数
  *@copyright   siasun.co
  */
#include "ReciThread.h"
#include <log/QsLog.h>
#pragma execution_character_set("utf-8")

ReciThread::ReciThread(QObject *parent) : QObject(parent)
{


}

ReciThread::~ReciThread()
{
    receiveSocket->deleteLater();
}
void
ReciThread::DoSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(listener()));
}
void
ReciThread::listener()
{
    QLOG_INFO() << "客户端接收线程已运行..."<<this->thread()->currentThreadId();
    receiveSocket=new QTcpSocket(this);
    connect(receiveSocket,&QTcpSocket::readyRead,[&]()
    {
        QByteArray tmp=receiveSocket->readAll();
        QString str(tmp);
        QLOG_INFO()<<str;
        emit this->receiveData(str);});
    connect(receiveSocket,&QTcpSocket::connected,[&]()
    {
        QLOG_INFO() <<"接收线程已连接到服务器！";});
    connect(receiveSocket,&QTcpSocket::disconnected,[&]()
    {
        QLOG_INFO() <<"接收线程已与服务器断开！";});
}
void
ReciThread::connectHost(const QString &host, int port)
{
    receiveSocket->abort();
    receiveSocket->connectToHost(host,port);
}
void
ReciThread::disconnectHost()
{
    receiveSocket->disconnectFromHost();
}
