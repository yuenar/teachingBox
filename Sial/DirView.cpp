/**
  *@brief       编辑器作业文件夹类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include <QtWidgets>
#include "DirView.h"
#include "WidgetHeader.h"
#include "NoFocusFrameDelegate.h"
#include <log/QsLog.h>

#pragma execution_character_set("utf-8")

DirView::DirView(QWidget *parent):
	QWidget(parent),
	dir_model_(new QFileSystemModel(this)),
	dir_view_(new QTreeView(this)),
    open_folder_button_(new QPushButton(tr("打开文件夹"), this))
{
	dir_view_->setItemDelegate(new NoFocusFrameDelegate());
    setObjectName(tr("DirView"));
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(dir_view_);
	layout->addWidget(open_folder_button_);
	setLayout(layout);
    QSettings settings("SIASUN", "TEXT");
	QString folder_dir = settings.value("folder_dir", QString()).toString();
	if(folder_dir.isEmpty()){
		closeFolder();
	}
	else setFolder(folder_dir);
    connect(open_folder_button_, &QPushButton::pressed, this, &DirView::openFolder);
    connect(dir_view_, &QTreeView::doubleClicked, this, &DirView::DoubleClickedItem);
}

void
DirView::setFolder(const QString &directory)
{
    QLOG_INFO() << directory;
    QSettings settings("SIASUN", "TEXT");
	settings.setValue("folder_dir", directory);
	dir_view_->setModel(dir_model_);
	dir_view_->setRootIndex(dir_model_->setRootPath(directory));
	dir_view_->setSortingEnabled(true);
	dir_view_->setHeaderHidden(true);
	dir_view_->hideColumn(1);
	dir_view_->hideColumn(2);
	dir_view_->hideColumn(3);
	open_folder_button_->setVisible(false);
	dir_view_->setVisible(true);
}

void
DirView::openFolder()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("选择目录"));
	if(!directory.isEmpty()) setFolder(directory);
}

void
DirView::closeFolder()
{
	open_folder_button_->setVisible(true);
	dir_view_->setVisible(false);
    QSettings settings("SIASUN", "TEXT");
	settings.setValue("folder_dir", QString());
}

void
DirView::DoubleClickedItem(const QModelIndex &index)
{
	if(QGuiApplication::mouseButtons() & Qt::LeftButton){
		QFileInfo info = dir_model_->fileInfo(index);
		if(info.isFile()) emit openFile(info.filePath());
	}
}
