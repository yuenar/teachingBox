/**
  *@brief       编辑器作业文件列表框声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef FILELABELWIDGET_H
#define FILELABELWIDGET_H
#include <QWidget>

class QLabel;
class QToolButton;
/*
 * A class to handle the visuality of manage_widget.
*/
class WidgetHeader : public QWidget
{
	Q_OBJECT
public:
	WidgetHeader(QString widget_name, QWidget *manage_widget, QWidget *parent = 0);
protected:
	void paintEvent(QPaintEvent *e)override;
private:
	void hide();
	void display();
public:
	QWidget *manage_widget_;
	QToolButton *visible_button_;
	QLabel *widget_label;
	QAction *clean_action_;
};

#endif // FILELABELWIDGET_H
