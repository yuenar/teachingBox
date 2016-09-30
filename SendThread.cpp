/**
  *@brief       发送线程实现文件
  *@author      yuenar
  *@date        16-09-20
  *@version     0.11
  *@note        @16-07-15 yuenar v0.10 初版，增加TCP通信部分基础函数
  *@note        @16-09-20 yuenar v0.11 修改线程调用函数
  *@copyright   siasun.co
  */
#include "SendThread.h"
#include <log/QsLog.h>
#pragma execution_character_set("utf-8")

SendThread::SendThread(QObject *parent) : QObject(parent)
{

}

SendThread::~SendThread()
{
    sentSocket->deleteLater();
}
void
SendThread::DoSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(writer()));
}
void
SendThread::writer()
{
    QLOG_INFO() << "客户端下发线程已运行..."<<this->thread()->currentThreadId();;
    sentSocket=new QTcpSocket(this);
    connect(sentSocket,&QTcpSocket::connected,[&]()
    {
        QLOG_INFO() <<"下发线程已连接到服务器！";});
    connect(sentSocket,&QTcpSocket::disconnected,[&]()
    {
        QLOG_INFO() <<"下发线程已断开服务器！";});
}
void
SendThread::connectHost(const QString &host, int port)
{
    sentSocket->abort();
    sentSocket->connectToHost(host,port);
}
void
SendThread::disconnectHost()
{
    sentSocket->disconnectFromHost();
}
void
SendThread::sentCmd(const QString & cmd)
{
    QByteArray lcmd=cmd.toLocal8Bit();
    if(sentSocket->state() == QAbstractSocket::ConnectedState)
    {//只有联通的时候才会下发
        sentSocket->write(lcmd);
        QLOG_INFO() <<"写入操作：" <<lcmd;
        emit sentRdata(cmd);
    }
    return;
}
