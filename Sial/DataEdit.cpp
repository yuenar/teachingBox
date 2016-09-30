/**
  *@brief       编辑器作业文件数据操作类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "DataEdit.h"
#include <QtWidgets>
#include <log/QsLog.h>

#pragma execution_character_set("utf-8")

DataEdit::DataEdit(const TextTheme &theme, EditType type, QWidget *parent):
	QPlainTextEdit(parent),
	modified_(false),
	highlighter_(new SyntaxHighlighter(this->document(), type)),
	normal_format_(theme.formats[NORMAL_FORMAT]),
	reading_(false)
{
	setVisible(false);
	highlighter_->resetFormat(theme);
	connect(this, &DataEdit::blockCountChanged, this, &DataEdit::markLineChanged);
}
DataEdit::~DataEdit()
{
}

void
DataEdit::readText(const QString &s)
{
	reading_ = true;
	appendPlainText(s);
}

void
DataEdit::checkEditModified(qint64 read_size)
{
	document()->setModified(modified_ = toPlainText().size() != read_size);//
	reading_ = false;
}

void
DataEdit::rename()
{
	if(reading_) return;
	QFileInfo info(file_path_);
	QString dir = info.dir().absolutePath();
	QString suffix = info.suffix();
	int i;
	for(i = 0; i != g_file_types.size(); ++i)
		if(QString::compare(suffix, g_file_types[i].suffix, Qt::CaseInsensitive) == 0) break;
	QString *select_filter = i == g_file_types.size() ? NULL : &g_file_types[i].filter;
	QString new_path = QFileDialog::getSaveFileName(this, tr("Rename file"), dir, g_file_filter, select_filter);
	if(!new_path.isEmpty()){
		QFile file(file_path_);
		file.rename(new_path);
		file.close();
		setFileInfo(new_path);
	}
}

void
DataEdit::setMarkLine(int mark_line_num)
{
	if(reading_) return;
	QTextBlock block = document()->findBlockByNumber(mark_line_num);
	block.setUserState(block.userState() ^ NOT_MARK_LINE);
    QLOG_INFO() << block.userState();
	emit markLineChange(block, !(block.userState() & NOT_MARK_LINE));
}

void
DataEdit::setFileInfo(const QString &file_full_name)
{
	file_path_ = file_full_name;
	file_name_ = QFileInfo(file_full_name).fileName();
    QLOG_INFO() << "重命名为: " << file_full_name
             << "\n新的文件路径: " << file_path_
             << "\n新的文件名: " << file_name_;
	emit renameSuccess(file_name_);
}
