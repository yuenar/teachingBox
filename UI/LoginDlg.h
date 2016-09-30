/**
  *@brief       登录界面类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-23 yuenar v0.10  初版，大致设计界面及基础函数
  *@note        @16-09-27 yuenar v0.11 修改界面样式
  *@copyright   siasun.co
  */
#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>
#include <QMap>
#include <QString>

namespace Ui {
class LoginDlg;
}

class LoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDlg(QWidget *parent = 0);
    ~LoginDlg();


    QString curUserType;//当前输入的用户名
    QString curPassWord;//当前输入的密码
    bool isLogin;//登录状态标志位
    QMap<QString,QString>  userModeMap;//储存用户模式信息

signals:
    /**
    *@brief		当前用户类型信号量
    *@remark	用于传值设定相应权限
    *@param[in]	userType输入的登录名
    */
    void curUser(const QString &userType);
private slots:
    void on_quitBtn_clicked();//退出按钮触发事件
    void loadUserMode();//加载本地用户模式配置
    void on_loginBtn_clicked();//登录按钮触发事件

private:
    Ui::LoginDlg *ui;
};

#endif // LOGINDLG_H
