/**
  *@brief       登录界面类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-23 yuenar v0.10  初版，大致设计界面及基础函数
  *@note        @16-09-27 yuenar v0.11 修改界面样式
  *@copyright   siasun.co
  */
#include "LoginDlg.h"
#include "ui_LoginDlg.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QTimer>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <log/QsLog.h>

LoginDlg::LoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    QLOG_INFO()<<"读取本地配置中...";
    loadUserMode();
}

LoginDlg::~LoginDlg()
{
    delete ui;
    QLOG_INFO()<<"Program exited";
}

void
LoginDlg::on_quitBtn_clicked()
{
    this->reject();
    QLOG_WARN()<<"登录已取消";
}
void
LoginDlg::loadUserMode()
{
    QString fileName = QDir::currentPath()+QString("/Config.xml");
    QFile file(fileName);

    if (false==file.open(QFile::ReadOnly | QFile::Text))
    {
        QLOG_ERROR()<<"本地配置读取失败"<<fileName;
        return;
    }
    else
    {
        QXmlStreamReader reader;
        reader.setDevice(&file);
        while (false==reader.atEnd())
        {
            reader.readNext();
            if(reader.isStartElement())
            {
                if((reader.name()=="user")&&(reader.attributes().hasAttribute("TYPE")))
                {
                    QStringRef Temp=reader.attributes().value("TYPE");
                    curUserType=Temp.toString();
                    if(false==curUserType.isEmpty())
                    {
                        QLOG_INFO()<<"读取"<<curUserType<<"用户配置";
                    }else
                    QLOG_ERROR()<<"IO error...";
                }else if(reader.name()=="password")
                {
                    curPassWord=reader.readElementText();
                    userModeMap[curUserType]=curPassWord;
                }
            }
        }
        QLOG_INFO()<<"本地配置读取完成";
        curUserType.clear();
        curPassWord.clear();
    }
}
void
LoginDlg::on_loginBtn_clicked()
{
    curUserType=this->ui->nameEdit->text();
    curPassWord=this->ui->pwdEdit->text();
    if((curUserType=="user")&&(curPassWord==userModeMap["user"]))
    {
        emit curUser("user");
        QLOG_INFO()<<"Loading UserMode...";
        isLogin=true;
        this->accept();
        this->hide();
    }
    else if((curUserType=="root")&&(curPassWord==userModeMap["root"]))
    {
        emit curUser("root");
        QLOG_INFO()<<"Loading RootMode...";
        isLogin=true;
        this->accept();
        this->hide();
    }
    else if((curUserType=="worker")&&(curPassWord==userModeMap["worker"]))
    {
        emit curUser("worker");
        QLOG_INFO()<<"Loading WorkerMode...";
        isLogin=true;
        this->accept();
        this->hide();
    }
    else
        QLOG_WARN()<<"账号或密码错误请重试！";
        return;
}
