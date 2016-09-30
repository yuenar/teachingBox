/**
  *@brief       系统信息类声明文件
  *@author      yuenar
  *@date        16-09-23
  *@version     0.10
  *@note        @16-09-23 yuenar v0.10 初版，增加基础布局及必要参数
  *@copyright   siasun.co
  */
#ifndef SYSINFOBOX_H
#define SYSINFOBOX_H

#include <QToolBox>

namespace Ui {
class SysInfoBox;
}

class SysInfoBox : public QToolBox
{
    Q_OBJECT

public:
    explicit SysInfoBox(QWidget *parent = 0);
    ~SysInfoBox();

private:
    Ui::SysInfoBox *ui;
public slots:
    void upIoInfo(const QString& curIoCfg,const QString &curIoSat);
    void upCtrCfgInfo(const QString& curConSat,const QString &curCtrSat,const QString& curTep,const QString &curErrInfo);
    void upCtrSetInfo(const QString& curVrn,const QString &curSoftLmt,const QString& curSptCfg,const QString &curZero,const QString &curDhCfg,const QString &curSubPct);
    void upUserInfo(const QString& curUser);

};

#endif // SYSINFOBOX_H
