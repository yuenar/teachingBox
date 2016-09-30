/**
  *@brief       主窗口类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.13
  *@note        @16-09-10 yuenar v0.10 初版，UI布局
  *@note        @16-09-11 yuenar v0.11 合并编辑器和示教盒部分
  *@note        @16-09-13 yuenar v0.12 去除边框添加菜单栏及函数
  *@note        @16-09-27 yuenar v0.13 修改界面样式
  *@copyright   siasun.co
  */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QStackedWidget>
#include "TeachingBox.h"
#include "RobotEditor.h"
#include "SysInfoBox.h"
#include <QToolBar>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel* msgLabel;
    QStackedWidget *stack;
    TeachingBox *te;
    RobotEditor *rtr;
    SysInfoBox *sys;
    bool isEditing;//是否已经在编辑状态

private slots:
    void init();//初始化相关函数
    void on_codeBtn_clicked();

    void on_teBtn_clicked();
    void on_sysBtn_clicked();
public slots:
    void setUser(const QString &userType);
};

#endif // MAINWINDOW_H
