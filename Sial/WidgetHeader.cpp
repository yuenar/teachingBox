/**
  *@brief       编辑器作业文件列表框实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "WidgetHeader.h"
#include "unit.h"
#include <QtWidgets>
#include <log/QsLog.h>

WidgetHeader::WidgetHeader(QString widget_name, QWidget *manage_widget, QWidget *parent):
	QWidget(parent),
	manage_widget_(manage_widget),
	visible_button_(new QToolButton(this)),
	widget_label(new QLabel(widget_name, this)),
	clean_action_(new QAction(tr("清空"), this))
{
	visible_button_->setStyleSheet("QToolButton{background-color: #b0b0b0;border: 0px;}"
									"QToolButton:hover{background-color: rgb(150, 150, 150);}");

	QFont font;
	font.setPixelSize(15);
	widget_label->setFont(font);
	addAction(clean_action_);
	setContextMenuPolicy(Qt::ActionsContextMenu);

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(visible_button_);
	layout->addWidget(widget_label);
//	layout->setContentsMargins(3, 0, 3, 0);
	setLayout(layout);
	setMaximumHeight(40);

	//Connect
	connect(visible_button_, &QToolButton::clicked,
			[this](){if(this->manage_widget_->isVisible())this->hide();else this->display();});
	display();
}

void WidgetHeader::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	QRect rect = e->rect();
	rect.setY(rect.y() + 7);
	rect.setHeight(rect.height() - 7);
	painter.fillRect(rect, QColor("#b0b0b0"));
}

void WidgetHeader::hide()
{
	visible_button_->setIcon(QIcon(":/icon/hide.png"));
	manage_widget_->setVisible(false);
    QSettings settings("SIASUN", "TEXT");
	settings.setValue(manage_widget_->objectName() + "_visible", false);
}

void WidgetHeader::display()
{
	visible_button_->setIcon(QIcon(":/icon/display.png"));
	manage_widget_->setVisible(true);
    QSettings settings("SIASUN", "TEXT");
	settings.setValue(manage_widget_->objectName() + "_visible", true);
}

