/**
  *@brief       主窗口类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.13
  *@note        @16-09-10 yuenar v0.10 初版，UI布局
  *@note        @16-09-11 yuenar v0.11 合并编辑器和示教盒部分
  *@note        @16-09-13 yuenar v0.12 去除边框添加菜单栏及函数
  *@note        @16-09-27 yuenar v0.13 修改界面样式
  *@copyright   siasun.co
  */
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <log/QsLog.h>
#include "unit.h"
#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),isEditing(false)
{
    ui->setupUi(this);
    //    this->setWindowFlags(Qt::FramelessWindowHint);//无边框化
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
    te->deleteLater();
    rtr->deleteLater();
    sys->deleteLater();
}
void
MainWindow::init()
{
    msgLabel = new QLabel;
    msgLabel->setMinimumSize(msgLabel->sizeHint());
    msgLabel->setAlignment(Qt::AlignHCenter);
    statusBar()->addWidget(msgLabel);
    stack=new QStackedWidget(this);
    stack->setGeometry(100,30,540,380);
    te=new TeachingBox();
    sys=new SysInfoBox();
    stack->addWidget(te);
    stack->addWidget(sys);
    connect(te,&TeachingBox::tipInfo,msgLabel,&QLabel::setText);
    connect(te,&TeachingBox::upIoInfo,sys,&SysInfoBox::upIoInfo);
    connect(te,&TeachingBox::upCtrCfgInfo,sys,&SysInfoBox::upCtrCfgInfo);
    connect(te,&TeachingBox::upCtrSetInfo,sys,&SysInfoBox::upCtrSetInfo);
}
void
MainWindow::setUser(const QString &userType)
{//设置登录用户信息
    sys->upUserInfo(userType);
}

void
MainWindow::on_codeBtn_clicked()
{
    if(false==isEditing)
    {
    rtr=new RobotEditor();
    rtr->show();
    isEditing=true;
    }else
    QLOG_WARN()<<"已打开过一个编辑器";
    connect(rtr,&RobotEditor::ChangeEditState,[&](){
       isEditing=false;
    });
    connect(te,&TeachingBox::createTask,rtr,&RobotEditor::createTask);
}

void
MainWindow::on_teBtn_clicked()
{
    stack->setCurrentIndex(0);
}

void MainWindow::on_sysBtn_clicked()
{
    stack->setCurrentIndex(1);
}
