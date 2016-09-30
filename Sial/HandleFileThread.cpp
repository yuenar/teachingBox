/**
  *@brief       编辑器文件操作类线程实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "HandleFileThread.h"
#include <QtWidgets>
#include "unit.h"
#include "CodeEdit.h"
#include <log/QsLog.h>

#pragma execution_character_set("utf-8")

HandleFileThread::HandleFileThread(QFile *file):
	stop(true),theFile(file)
{
}

void
HandleFileThread::run()
{
	stop = false;
	QTextStream in(theFile);
	qint64 readNumberCnt = 0;
	if (!stop && theFile->size() < (1 << 20) ) //Less than 1MB
	{
		QString s = in.readAll();
		readNumberCnt = s.size();
		emit readData(s);
	}
	else
	{
		QString s;
		s.reserve(READ_LENGTH + 100);
		int cnt = 0;
		while(!stop && !in.atEnd()){
			s += in.readLine();
			if(s.size() >= READ_LENGTH || in.atEnd()){
				emit readData(s);
				readNumberCnt += s.size();
				s.clear();
				s.reserve(READ_LENGTH + 100);
				++cnt;
			}
			else s += '\n';
		}
		readNumberCnt += cnt - 1;
	}
	theFile->close();
	theFile->deleteLater();
	if(stop) return;
	emit readSuccess(readNumberCnt);
}
