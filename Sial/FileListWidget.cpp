/**
  *@brief       编辑器作业文件列表类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "FileListWidget.h"
#include <QtWidgets>
#include "WorkSpace.h"
#include <log/QsLog.h>

#pragma execution_character_set("utf-8")

FileListWidget::FileListWidget(QWidget *parent):
	QListWidget(parent)
{
	connect(this, &FileListWidget::pressed, this, &FileListWidget::emitCurrentFileSignal);
}

FileListWidget::~FileListWidget()
{
}

void
FileListWidget::emitCloseFileSignal(QListWidgetItem *item)
{
	emit tryCloseFile(item);
}

void
FileListWidget::emitCurrentFileSignal(const QModelIndex &index)
{
	if(QGuiApplication::mouseButtons() & Qt::LeftButton) emit currentFile(itemFromIndex(index));
}

void
FileListWidget::contextMenuEvent(QContextMenuEvent *event)
{
	QListWidgetItem *item;
	if((item = itemAt(event->pos())) != NULL){
		QMenu *menu = new QMenu(this);
                QAction *rename = new QAction(tr("重命名"), menu);
                QAction *close = new QAction(tr("关闭"), menu);
		connect(rename, &QAction::triggered, item->data(Qt::UserRole).value<DataEdit*>(), &DataEdit::rename);
		connect(close, &QAction::triggered, [this, item](){this->emitCloseFileSignal(item);});
		menu->addAction(rename);
		menu->addAction(close);
		menu->setAttribute(Qt::WA_DeleteOnClose);
		menu->exec(QCursor::pos());
	}
}

