/**
  *@brief       示教盒实现文件
  *@author      yuenar
  *@date        16-09-30
  *@version     0.26
  *@note        @16-07-14 yuenar v0.10 初版，设计UI及基本函数
  *@note        @16-07-15 yuenar v0.11 增加TCP通信部分基础函数
  *@note        @16-07-19 yuenar v0.12 修改通信部分，测试一个简单命令
  *@note        @16-08-08 yuenar v0.13 新增连续示教加减函数及格式化协议指令
  *@note        @16-08-15 yuenar v0.14 新增位置更新不同模式下的解析函数
  *@note        @16-08-28 yuenar v0.15 新增XML线程处理数据更新函数
  *@note        @16-08-29 yuenar v0.16 新增XML读取指令的文本编辑器功能
  *@note        @16-08-30 yuenar v0.17 新增XML文本编辑器保存XML文件功能
  *@note        @16-08-31 yuenar v0.18 新增XML文本编辑器删除自动保存功能
  *@note        @16-09-10 yuenar v0.19 提取XML文本编辑器到任务管理器类中
  *@note        @16-09-20 yuenar v0.20 修改线程调用函数
  *@note        @16-09-22 yuenar v0.21 新增各函数伺服电机使能条件判断及示教运动参数下发
  *@note        @16-09-23 yuenar v0.22 新增系统信息界面与下位机状态同步功能
  *@note        @16-09-27 yuenar v0.23 修改界面样式，修复子线程未正常结束BUG
  *@note        @16-09-28 yuenar v0.24 增加下发作业文件功能全部读为字节流
  *@note        @16-09-29 yuenar v0.25 增加回读机器人上次作业文件功能
  *@note        @16-09-30 yuenar v0.26 回读下发历史作业文件功能测试成功
  *@copyright   siasun.co
  */
#include "TeachingBox.h"
#include "ui_TeachingBox.h"
#include <log/QsLog.h>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QTimer>

#pragma execution_character_set("utf-8")

/**
 *!   内部指令分别用U-Z指代1-6轴
 *!   以下常用指令用于格式化补全
 */
const QString AxisStr="X-AXis,Y-AXis,Z-AXis,RX-AXis,RY-AXis,RZ-AXis";//坐标体系
const QString ControlDemand="ctd2%1";//控制指令
const QString RcfDemand="rcf2%1";//指令文件指令，%1为作业文件内容，由主socket下发及回传,回读要早于下发！
const QString MoveDemand="mov2%1";//运动方式（加速或者减速）

/*
！！运动参数，%1尝试为动作类型，2%为速度
*/
const QString CfgInfo="cfg2%1%2";
const QString ClientServerLineDemd="csl2%1";//CS关联指令，此类命令由主线程发往服务端
const QString TaskManageDemd="tkm2%1";//作业管理指令
const int mainPort=5000;//主端口
const int sendPort=5001;//下发端口
const int recvPort=4999;//接收端口

TeachingBox::TeachingBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TeachingBox),isJointMode(true),isConnected(false),isExecuted(false),isWorking(false),mode(1),curMvSpeed("0.00"),oldFucCmd(""),curSpType("MOVJ")
{//以上为初始化参数列表
    ui->setupUi(this);

    tcpClient = new QTcpSocket(this);
    rThread =new ReciThread();
    rThread->DoSetup(listenerTd);
    rThread->moveToThread(&listenerTd);

    connect(this, SIGNAL(startListener()), rThread, SLOT(listener()));

    if(!listenerTd.isRunning()){
        listenerTd.start();
    }else{
        emit startListener();
    }
    sThread =new SendThread();

    sThread->DoSetup(writerTd);
    sThread->moveToThread(&writerTd);

    connect(this, SIGNAL(startWriter()), sThread, SLOT(writer()));
    connect(this,&TeachingBox::readyReview,[&]()
    {
        QFile file("lastTask.xml");
        if(false==file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
        {
            QMessageBox::critical(NULL, "提示", "无法创建文件");
            return;
        }else
        {
            QTextStream out(&file);
            out<<oldFucCmd<<endl;
            out.flush();
            file.close();
            oldFucCmd.clear();
            emit tipInfo(tr("回读机器人上次作业文件成功！"));
        }
    });

    if(!writerTd.isRunning()){
        writerTd.start();
    }else{
        emit startWriter();
    }
    tcpClient->abort();

    connect(tcpClient,&QTcpSocket::readyRead,
            [&]()
    {
        QString tring=QString(this->tcpClient->readAll());
        if(true==tring.contains("</Task_List>"))
        {//回读时候直接写入本地lastTask.xml，不是则打印信息
            oldFucCmd.append(tring);
            QLOG_INFO() << "开始回读历史作业文件!";
            emit readyReview();
        }else
            oldFucCmd.append(tring);
    });
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(ReadError(QAbstractSocket::SocketError)));
    connect(tcpClient,&QTcpSocket::disconnected,[]()
    {
        QLOG_ERROR() << "连接已断开!" ;
    });
    connect(rThread,&ReciThread::receiveData,this,&TeachingBox::upRecvData);
    connect(sThread,&SendThread::sentRdata,this,&TeachingBox::upSentData);
    //    connect(cEtor,&CodeEditor::deleText,xThread,&XmlThread::deleText);
    initJointAxisUI();
    QLOG_INFO() << "示教盒初始化完毕!"<<this->thread()->currentThreadId();
}

TeachingBox::~TeachingBox()
{//防止子线程未正常退出导致内存泄漏
    delete ui;
    delete tcpClient;
    if(listenerTd.isRunning())
    {
        QLOG_INFO() << "听者线程正在退出!";
        listenerTd.quit();
        listenerTd.wait(300);
    }else
        QLOG_INFO() << "听者线程已正常退出了!";
    if(writerTd.isRunning())
    {
        QLOG_INFO() << "听者线程正在退出!";
        writerTd.quit();
        writerTd.wait(300);
    }else
        QLOG_INFO() << "写手线程已正常退出了!";
    rThread->deleteLater();
    sThread->deleteLater();
    QLOG_INFO() << "界面线程正常结束!";
}
void
TeachingBox::initJointAxisUI()
{
    this->update();
    model = new QStandardItemModel(6,2);
    model->setHeaderData(0, Qt::Horizontal, tr("关节编号"));
    model->setHeaderData(1, Qt::Horizontal, tr("关节角度"));
    ui->tableView->setModel(model);
    QString Jname="    J%1";
    for (int col = 0; col < 6; col++) {
        QModelIndex index = model->index(col, 0, QModelIndex());
        model->setData(index,Jname.arg(col+1));
    }
    ui->XaxLb->setText("  J1");
    ui->YaxLb->setText("  J2");
    ui->ZaxLb->setText("  J3");
    ui->RXaxLb->setText("  J4");
    ui->RYaxLb->setText("  J5");
    ui->RZaxLb->setText("  J6");
}
void
TeachingBox::swapAxisUI()
{
    model = new QStandardItemModel(6,2);
    model->setHeaderData(0, Qt::Horizontal, tr("坐标轴"));
    model->setHeaderData(1, Qt::Horizontal, tr("末端点"));
    ui->tableView->setModel(model);
    QStringList axisList=AxisStr.split(",");
    QListIterator<QString> g(axisList);
    while (g.hasNext()) {
        for (int col = 0; col < axisList.count(); ++col) {
            QModelIndex index = model->index(col, 0, QModelIndex());
            model->setData(index, g.next());
        }
    }
    ui->XaxLb->setText(axisList.at(0));
    ui->YaxLb->setText(axisList.at(1));
    ui->ZaxLb->setText(axisList.at(2));
    ui->RXaxLb->setText(axisList.at(3));
    ui->RYaxLb->setText(axisList.at(4));
    ui->RZaxLb->setText(axisList.at(5));
}

void
TeachingBox::on_WdRdBtn_clicked()
{
    if ((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        tcpClient->write(ControlDemand.arg("RECT").toLatin1());
        mode=2;
        QLOG_INFO() << "切换直角坐标系视角!";
        isJointMode=false;
        swapAxisUI();
        emit tipInfo("已切换直角坐标系视角!");
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
    setCheckBtn(mode);
}

void
TeachingBox::on_ToRdBtn_clicked()
{
    if ((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        tcpClient->write(ControlDemand.arg("TOOL").toLatin1());
        mode=3;
        QLOG_INFO() << "切换工具坐标系视角!";
        isJointMode=false;
        swapAxisUI();
        emit tipInfo("已切换工具坐标系视角!");
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
    setCheckBtn(mode);
}

void TeachingBox::on_UrRdBtn_clicked()
{
    if ((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        tcpClient->write(ControlDemand.arg("USER").toLatin1());
        mode=4;
        QLOG_INFO() << "切换用户坐标系视角!";
        isJointMode=false;
        swapAxisUI();
        emit tipInfo("已切换用户坐标系视角!");
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
    setCheckBtn(mode);
}

void
TeachingBox::on_JtRdBtn_clicked()
{
    if ((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        tcpClient->write(ControlDemand.arg("JINT").toLatin1());
        mode=1;
        QLOG_INFO() << "切换关节坐标系视角!";
        isJointMode=true;
        initJointAxisUI();
        emit tipInfo("已切换关节坐标系视角!");
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
    setCheckBtn(mode);
}
void
TeachingBox::setCheckBtn(const int mnum)
{
    switch (mnum) {
    case 1:
        ui->JtRdBtn->setChecked(true);
        break;
    case 2:
        ui->WdRdBtn->setChecked(true);
        break;
    case 3:
        ui->ToRdBtn->setChecked(true);
        break;
    case 4:
        ui->UrRdBtn->setChecked(true);
        break;
    default:
        QLOG_INFO() << "切换坐标系视角失败!";
        break;
    }

}

void
TeachingBox::ReadError(QAbstractSocket::SocketError)
{
    tcpClient->disconnectFromHost();
    emit tipInfo(tr("连接出错：%1").arg(tcpClient->errorString()));
}
void
TeachingBox::sendConfig()
{//每次会先把当前配置参数检查一遍是否修改，如果修改则发送新的参数然后发送控制指令未修改则只发送控制指令
    if((curMvSpeed!=ui->MvsdSpBox->text())||(curSpType!=ui->MvModeCbox->currentText()))
    {
        curMvSpeed=ui->MvsdSpBox->text();
        curSpType=ui->MvModeCbox->currentText();
    }
    else
    {
        QLOG_INFO() << "参数未曾修改！" ;
        return;//没有修改则直接返回，不往下进行
    }
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {//发送新的参数给下位机
        sThread->sentCmd(CfgInfo.arg(curSpType).arg(curMvSpeed).toLatin1());
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}

void
TeachingBox::on_ContRdBtn_clicked()
{
    if(false==isConnected)
    {
        QLOG_INFO() << "尝试建立连接>>>" ;
        listen();
    }
    else
    {
        QLOG_INFO() << "尝试断开连接>>>" ;
        sThread->sentCmd(ClientServerLineDemd.arg("QUIT").toLatin1());
        QTimer::singleShot(300, this, SLOT(stopAll()));
    }
}

void
TeachingBox::on_SvmtRdBtn_clicked()
{
    if((false==isWorking)&&(true==isConnected))
    {
        /*发送伺服电机上电指令*/
        isWorking=true;
        this->ui->SvmtRdBtn->setText("IsWorking");
        sThread->sentCmd(ControlDemand.arg("DMON").toLatin1());
        QLOG_INFO() << "正在给伺服电机上电>>>" ;
    }
    else
    {
        /*发送伺服电机断电指令*/
        isWorking=false;
        this->ui->SvmtRdBtn->setText("UnWorking");
        sThread->sentCmd(ControlDemand.arg("DMOF").toLatin1());
        QLOG_INFO() << "正在给伺服电机断电>>>" ;
    }
}
void
TeachingBox::listen()
{
    QString ipAdd(this->ui->HostInEdit->text());

    if (ipAdd.isEmpty())
    {
        emit tipInfo("请输入正确IP!");
        return;
    }
    else
    {
        tcpClient->connectToHost(ipAdd,mainPort);
        listenerTd.start();
        writerTd.start();
        rThread->connectHost(ipAdd,recvPort);
        sThread->connectHost(ipAdd,sendPort);
    }
    if(tcpClient->waitForConnected(1000))
    {
        ui->ContRdBtn->setText("IsConnected");
        emit tipInfo("连接服务器成功");
        QLOG_INFO() << "连接服务器成功!" ;
        tcpClient->write(ControlDemand.arg("JINT").toLatin1());//默认初始关节示教模式
        this->ui->JtRdBtn->setChecked(true);
        this->ui->HostInEdit->setEnabled(false);
        //        this->ui->PortLnEdit->setEnabled(false);
        isConnected=true;
    }
    else
    {
        isConnected=false;
        listenerTd.quit();
        listenerTd.wait(100);
        writerTd.quit();
        writerTd.wait(100);
        QLOG_INFO() << "连接服务器失败!" ;
    }
}
void
TeachingBox::stopAll()
{
    if(tcpClient->state() == QAbstractSocket::ConnectedState)
    {
        isWorking=false;
        tcpClient->disconnectFromHost();
        rThread->disconnectHost();
        listenerTd.quit();
        listenerTd.wait(100);
        sThread->disconnectHost();
        writerTd.quit();
        writerTd.wait(100);
    }
    if(tcpClient->state() == QAbstractSocket::UnconnectedState || tcpClient->waitForDisconnected(1000) )
    {
        ui->ContRdBtn->setText("Disconnect");
        emit tipInfo("已断开服务器");
        this->ui->HostInEdit->setEnabled(true);
        //        this->ui->PortLnEdit->setEnabled(true);
        isConnected=false;
    }
}
void
TeachingBox::upRecvData(const QString &str)
{
    if(str.contains("p"))
    {
        //从下位机接收的除了位置信息就是控制器IO相关信息
        upTableData(str);
        emit tipInfo("Pos is update!");
        QLOG_INFO()<<"Pos is update!";
    }/*else
        {
             emit tipInfo("Waiting data...");
        }*/
}
void
TeachingBox::upTableData(const QString &str)
{//Update Table Data
    QStringList posList=str.split("p");
    QListIterator<QString> p(posList);
    while (p.hasNext()) {
        for (int col = 0; col < posList.count(); ++col) {
            QModelIndex index = model->index(col, 1, QModelIndex());
            model->setData(index, p.next());
        }
    }

}

void
TeachingBox::upSentData(const QString &str)
{
    emit tipInfo(tr("%1 已发送到服务器：%2").arg(QTime::currentTime().toString("hh:mm:ss.zzz")).arg(str));
}

void
TeachingBox::on_crtBtn_clicked()
{//发送信号如果当前编辑正在打开会读取编辑器各作业文件名及内容进行写入task.xml文件
    emit createTask();
}

void
TeachingBox::on_strBtn_clicked()
{//扫描到本地作业文件在伺服电机上电状态下会下发
    QFile file("Task.xml");
    if(file.open(QFile::ReadOnly | QFile::Text)&&(true==isWorking))
    {
        QTextStream cmd(&file);
        curFucCmd=RcfDemand.arg(cmd.readAll());
        tcpClient->write(curFucCmd.toLatin1());//主socket下发作业文件
        isExecuted=true;
        QLOG_INFO()<<"已下发机器人作业文件！" ;
    }else
        QLOG_WARN()<<"请检查伺服电机或作业文件后重试！" ;
    file.close();
}

void
TeachingBox::on_stpBtn_clicked()
{
    if((true==isExecuted)&&(true==isWorking))
    {
        sThread->sentCmd(RcfDemand.arg("STOP").toLatin1());//停止当前作业
        isExecuted=false;
        QLOG_INFO()<<"机器人控制器作业正常中断！" ;
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}

void
TeachingBox::on_getPsBtn_clicked()
{
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("PSNW").toLatin1());//获取当前位置
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}


void
TeachingBox::uMvStop()
{
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("USTP").toLatin1());//1轴停止运动
        QLOG_INFO() << "1轴运动停止..." ;
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::vMvStop()
{
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("VSTP").toLatin1());//2轴停止运动
        QLOG_INFO() << "2轴运动停止..." ;
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::wMvStop()
{
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("WSTP").toLatin1());//3轴停止运动
        QLOG_INFO() << "3轴运动停止..." ;
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::xMvStop()
{
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("XSTP").toLatin1());//4轴停止运动
        QLOG_INFO() << "4轴运动停止..." ;
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::yMvStop()
{
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("YSTP").toLatin1());//5轴停止运动
        QLOG_INFO() << "5轴运动停止..." ;
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::zMvStop()
{
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("ZSTP").toLatin1());//6轴停止运动
        QLOG_INFO() << "6轴运动停止..." ;
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}


void
TeachingBox::on_Xplus_pressed()
{
    QLOG_INFO() << "使用1轴加速运动..." ;
    sendConfig();
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("UADD").toLatin1());//1轴加速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_Xplus_released()
{
    uMvStop();
}
void
TeachingBox::on_Xsub_pressed()
{
    QLOG_INFO() << "使用1轴减速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("USUB").toLatin1());//1轴减速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_Xsub_released()
{
    uMvStop();
}

void
TeachingBox::on_Yplus_pressed()
{
    QLOG_INFO() << "使用2轴加速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("VADD").toLatin1());//2轴加速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_Yplus_released()
{
    vMvStop();
}
void
TeachingBox::on_Ysub_pressed()
{
    QLOG_INFO() << "使用2轴减速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("VSUB").toLatin1());//2轴减速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_Ysub_released()
{
    vMvStop();
}

void
TeachingBox::on_Zplus_pressed()
{
    QLOG_INFO() << "使用3轴加速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("WADD").toLatin1());//3轴加速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_Zplus_released()
{
    wMvStop();
}
void
TeachingBox::on_Zsub_pressed()
{
    QLOG_INFO() << "使用3轴减速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("WSUB").toLatin1());//3轴减速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_Zsub_released()
{
    wMvStop();
}

void
TeachingBox::on_RXplus_pressed()
{
    QLOG_INFO() << "使用4轴加速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("XADD").toLatin1());//4轴加速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_RXplus_released()
{
    xMvStop();
}
void
TeachingBox::on_RXsub_pressed()
{
    QLOG_INFO() << "使用4轴减速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("XSUB").toLatin1());//4轴减速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_RXsub_released()
{
    xMvStop();
}

void
TeachingBox::on_RYplus_pressed()
{
    QLOG_INFO() << "使用5轴加速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("YADD").toLatin1());//5轴加速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_RYplus_released()
{
    yMvStop();
}
void
TeachingBox::on_RYsub_pressed()
{
    QLOG_INFO() << "使用5轴减速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("YSUB").toLatin1());//5轴减速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_RYsub_released()
{
    yMvStop();
}

void
TeachingBox::on_RZplus_pressed()
{
    QLOG_INFO() << "使用6轴加速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("ZADD").toLatin1());//6轴加速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_RZplus_released()
{
    zMvStop();
}
void
TeachingBox::on_RZsub_pressed()
{
    QLOG_INFO() << "使用6轴减速运动..." ;
    sendConfig();

    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        sThread->sentCmd(MoveDemand.arg("ZSUB").toLatin1());//6轴减速走
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
void
TeachingBox::on_RZsub_released()
{
    zMvStop();
}


void TeachingBox::on_rewBtn_clicked()
{//手动发送回读作业指令
    if((tcpClient->state() == QAbstractSocket::ConnectedState)&&(true==isWorking))
    {
        tcpClient->write(RcfDemand.arg("BCMD").toLatin1());//回读指令
    }else
        QLOG_WARN()<<"请检查伺服电机或链接状态后重试！" ;
}
