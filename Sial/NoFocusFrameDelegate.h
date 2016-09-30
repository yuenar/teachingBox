/**
  *@brief       编辑器行编辑去焦效果声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef NOFOCUSFRAMEDELEGATE_H
#define NOFOCUSFRAMEDELEGATE_H
#include <QObject>
#include <QStyledItemDelegate>

/*
 * 这个类用于取消当前列表项的对焦状态。
*/
class NoFocusFrameDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	NoFocusFrameDelegate(QObject *parent = 0);
private:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // NOFOCUSFRAMEDELEGATE_H
