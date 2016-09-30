/**
  *@brief       编辑器行编辑去焦效果实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "NoFocusFrameDelegate.h"
#include <QtWidgets>
#include <log/QsLog.h>

#pragma execution_character_set("utf-8")
NoFocusFrameDelegate::NoFocusFrameDelegate(QObject *parent):QStyledItemDelegate(parent)
{
}

void NoFocusFrameDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem itemOption(option);
	// remove the focus state
	if(itemOption.state & QStyle::State_HasFocus){
		itemOption.state ^= QStyle::State_HasFocus;
	}
	QStyledItemDelegate::paint(painter, itemOption, index);
}
