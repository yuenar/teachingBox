/**
  *@brief       编辑器作业文件操作线程类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef HANDLEFILETHREAD_H
#define HANDLEFILETHREAD_H
#include <QThread>
#include <QFile>
#define READ_LENGTH (1LL << 14)

/*
 * 载入文件线程类
*/
class HandleFileThread : public QThread
{
	Q_OBJECT
public:
	HandleFileThread(QFile *File);
	bool stop;
signals:
	void readData(const QString &s);
	void readSuccess(qint64 read_size);
protected:
	void run();
private:
	QFile *theFile;
};

#endif // HANDLEFILETHREAD_H
