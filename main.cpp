/**
  *@brief       主函数入口
  *@author      yuenar
  *@date        16-09-27
  *@version     0.12
  *@note        @16-07-14 yuenar v0.10 初版，添加日志系统及基本函数
  *@note        @16-09-23 yuenar v0.11 新增登录界面及调用函数
  *@note        @16-09-27 yuenar v0.12 增加开源编辑器相关容器
  *@copyright   siasun.co
  */

#include <QApplication>
#include <QDateTime>
#include <log/QsLog.h>
#include <log/QsLogDest.h>
#include <QString>
#include <QObject>
#include <QDir>
#include "UI/LoginDlg.h"
#include "UI/MainWindow.h"
#include "unit.h"

QVector<FileType> g_file_types;
QString g_file_filter;

#pragma execution_character_set("utf-8")
const QString logFile="%1.log";

int main(int argc, char *argv[])
{
    /**
     * 以下为实例化日志系统类
*/
    using namespace QsLogging;
    QString strBuffer;
    QDateTime time;
    time = QDateTime::currentDateTime();
    strBuffer = time.toString("yyyy-MM-dd");
    // init the logging mechanism
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    QApplication app(argc, argv);//日志系统在主程序之前
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString sLogPath(QDir(app.applicationDirPath()).filePath(logFile.arg(strBuffer)));
    QsLogging::DestinationPtr fileDestination(
                QsLogging::DestinationFactory::MakeFileDestination(sLogPath) );
    QsLogging::DestinationPtr debugDestination(
                QsLogging::DestinationFactory::MakeDebugOutputDestination() );
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);
    /**
     * 以上为实例化日志系统类
    */
    QLOG_INFO() << "Program started";
    LoginDlg login;//登陆窗口
    QLOG_INFO() << "请登录";
    MainWindow rootWin;
    login.show();
    QObject::connect(&login,&LoginDlg::curUser,&rootWin,&MainWindow::setUser);
    if(login.exec()==QDialog::Accepted){
        rootWin.show();
        return app.exec();
    }
    return 0;

}
