/**
  *@brief       示教盒声明文件
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
#ifndef TEACHINGBOX_H
#define TEACHINGBOX_H

#include <QWidget>
#include <QTimer>
#include <QStandardItemModel>
#include <QtNetwork>
#include <QLabel>
#include <QThread>
#include <QStringList>
#include "ReciThread.h"
#include "SendThread.h"


namespace Ui {
class TeachingBox;
}

class TeachingBox : public QWidget
{
    Q_OBJECT

public:
    explicit TeachingBox(QWidget *parent = 0);
    ~TeachingBox();

private:
    Ui::TeachingBox *ui;
    QTimer tm;//运行时间计数器
    QStringList list;//用来存放坐标值
    QStandardItemModel *model;//表格模型
    QTcpSocket * tcpClient;
    ReciThread *rThread;
    SendThread *sThread;
    QThread listenerTd;
    QThread writerTd;


    bool isConnected;//是否连接flag
    bool isExecuted;//作业指令是否在执行状态
    bool isWorking;//是否上电flag
    bool isJointMode;//是否是关节模式flag

    /*以下为全局参数变量*/
    int mode;//示教模式值/×1=JINT 2=RECT 3=TOOL 4=USER×/
//    QString AddMinsTr;//加减各轴判定字串（预留）
    QString curMvSpeed;//当前速度
    QString curSpType;//当前运动类型
    /*与编辑器同步函数信息*/
    QString curFucCmd;//用于保存Task.xml中文本信息，用于下发
    QString oldFucCmd;//用于保存回读的Task.xml中文本信息，用于下发

    /*以下为系统信息界面相关参数，
     * 通过接收线程进行接收及解析
     * 建议统一转换成字串
    */
    QString ioCfg;//io配置
    QString ioSat;//io状态
    QString conSat;//通信状态
    QString ctrSat;//控制器设备状态
    QString tep;//控制器温度
    QString errInfo;//控制器异常信息
    QString vrn;//控制器版本信息
    QString softLmt;//软限位
    QString sptCfg;//运动参数限值
    QString zero;//零位码盘值
    QString dhCfg;//DH参数
    QString subPct;//减速比
    //    QString userName;//当前登录用户名称
signals:
    void createTask();//新建任务文件
    void readyReview();
    void conHost();
    void disConHost();
    void tipInfo(const QString& str);//状态栏提示
    void startListener();
    void startWriter();

    void upIoInfo(const QString& curIoCfg,const QString &curIoSat);
    void upCtrCfgInfo(const QString& curConSat,const QString &curCtrSat,const QString& curTep,const QString &curErrInfo);
    void upCtrSetInfo(const QString& curVrn,const QString &curSoftLmt,const QString& curSptCfg,const QString &curZero,const QString &curDhCfg,const QString &curSubPct);
    //    void upUserInfo(const QString& curUser);
public slots:
    void ReadError(QAbstractSocket::SocketError);
    void initJointAxisUI();//初始化Joint模式下的UI
    void swapAxisUI();//切换Joint模式下的UI
    void listen();//开始监听
    void stopAll();//停止所有线程
private slots:
    void on_WdRdBtn_clicked();
    void on_ToRdBtn_clicked();
    void on_UrRdBtn_clicked();
    void on_JtRdBtn_clicked();

    void uMvStop();
    void vMvStop();
    void wMvStop();
    void xMvStop();
    void yMvStop();
    void zMvStop();
    void sendConfig();//运动前需要将当前的速度、运动方式先下发
    void setCheckBtn(const int mnum);//设置check属性


    void on_ContRdBtn_clicked();
    void on_SvmtRdBtn_clicked();
    void upTableData(const QString & str);
    void upRecvData(const QString & str);
    void upSentData(const QString & str);
    void on_crtBtn_clicked();
    void on_strBtn_clicked();
    void on_stpBtn_clicked();
    void on_getPsBtn_clicked();
    void on_Xplus_pressed();
    void on_Xplus_released();
    void on_Xsub_pressed();
    void on_Xsub_released();
    void on_Yplus_pressed();
    void on_Yplus_released();
    void on_Ysub_pressed();
    void on_Ysub_released();
    void on_Zplus_pressed();
    void on_Zplus_released();
    void on_Zsub_pressed();
    void on_Zsub_released();
    void on_RXplus_pressed();
    void on_RXplus_released();
    void on_RXsub_pressed();
    void on_RXsub_released();
    void on_RYplus_pressed();
    void on_RYplus_released();
    void on_RYsub_pressed();
    void on_RYsub_released();
    void on_RZplus_pressed();
    void on_RZplus_released();
    void on_RZsub_pressed();
    void on_RZsub_released();
    void on_rewBtn_clicked();
};

#endif // TEACHINGBOX_H
