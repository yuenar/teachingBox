/**
  *@brief       系统信息类实现文件
  *@author      yuenar
  *@date        16-09-23
  *@version     0.10
  *@note        @16-09-23 yuenar v0.10 初版，增加基础布局及必要参数
  *@copyright   siasun.co
  */
#include "SysInfoBox.h"
#include "ui_SysInfoBox.h"
#include <log/QsLog.h>
#pragma execution_character_set("utf-8")

SysInfoBox::SysInfoBox(QWidget *parent) :
    QToolBox(parent),
    ui(new Ui::SysInfoBox)
{
    ui->setupUi(this);
}

SysInfoBox::~SysInfoBox()
{
    delete ui;
}

void
SysInfoBox::upIoInfo(const QString& curIoCfg,const QString &curIoSat)
{//IO配置信息更新
    ui->cfgDetLb->setText(curIoCfg);//配置信息
    ui->staDetLb->setText(curIoSat);//状态信息
}

void
SysInfoBox::upCtrCfgInfo(const QString& curConSat,const QString &curCtrSat,const QString& curTep,const QString &curErrInfo)
{//控制器状态信息更新
    ui->conDetLb->setText(curConSat);//通信状态
    ui->ctrDetLb->setText(curCtrSat);//设备状态
    ui->tepDetLb->setText(curTep);//温度状态
    ui->errDetLb->setText(curErrInfo);//异常信息
}

void
SysInfoBox::upCtrSetInfo(const QString& curVrn,const QString &curSoftLmt,const QString& curSptCfg,const QString &curZero,const QString &curDhCfg,const QString &curSubPct)
{//控制器设置信息更新
    ui->vrnDetLb->setText(curVrn);//版本
    ui->softLmtDetLb->setText(curSoftLmt);//软限位
    ui->sptDetLb->setText(curSptCfg);//运动参数限制
    ui->zeroDetLb->setText(curZero);//零位码盘值
    ui->dhDetLb->setText(curDhCfg);//DH参数
    ui->subPrecentLb->setText(curSubPct);//减速比
}

void
SysInfoBox::upUserInfo(const QString& curUser)
{//用户信息更新
    ui->usrDetLb->setText(curUser);//当前用户名
}
