/**
  *@brief       编辑器作业文件数据操作类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef DATACODE_H
#define DATACODE_H
#include <QPlainTextEdit>
#include "SyntaxHighlighter.h"
/*
 * 保存数据.
 * 缺省状态:
 *	unvisible
*/

class DataEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	DataEdit(const TextTheme &theme, EditType type, QWidget *parent = 0);
	~DataEdit();
	void stopLoading(){emit stopReading();}
	void setMarkLine(int mark_line_num);
	void setFileInfo(const QString &file_full_name);
signals:
	void stopReading();
	void renameSuccess(const QString &file_name);
	void markLineChange(QTextBlock mark_block, bool add);
	void markLineChanged();
public slots:
	void readText(const QString &s);
	void checkEditModified(qint64 read_size);
	void rename();
//Data
public:
	bool modified_;
	QString file_path_;
	QString file_name_;
	SyntaxHighlighter *highlighter_;
	QTextCharFormat normal_format_;
private:
	bool reading_;
};

Q_DECLARE_METATYPE(DataEdit*)

#endif // DATACODE_H
