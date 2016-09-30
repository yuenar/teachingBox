/**
  *@brief       编辑器作业文件管理类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "FileManageWidget.h"
#include <QtWidgets>
#include "WorkSpace.h"
#include "NoFocusFrameDelegate.h"
#include "DataEdit.h"
#include <log/QsLog.h>
#pragma execution_character_set("utf-8")

FileManageWidget::FileManageWidget(WorkSpace *parent):
    QWidget(parent),
    work_space_(parent),
    work_file_widget_(new FileListWidget(this)),
    folder_widget_(new DirView(this))
{
    setObjectName("FileManageWidget");
    work_file_widget_->setItemDelegate(new NoFocusFrameDelegate());
    work_file_widget_header_ = new WidgetHeader(tr("作业列表"), work_file_widget_, this);
    folder_widget_header_ = new WidgetHeader(tr("本地目录"), folder_widget_, this);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);
    vlayout->addWidget(work_file_widget_header_, 0, Qt::AlignTop);
    vlayout->addWidget(work_file_widget_, 10, Qt::AlignTop);
    vlayout->addWidget(folder_widget_header_, 0, Qt::AlignTop);
    vlayout->addWidget(folder_widget_, 10, Qt::AlignTop);
    setLayout(vlayout);
    //Connect
    connect(folder_widget_header_->clean_action_, &QAction::triggered, folder_widget_, &DirView::closeFolder);
	connect(work_file_widget_header_->clean_action_, &QAction::triggered, this, &FileManageWidget::cleanWorkingFiles);

    setMaximumWidth(220);
}

FileManageWidget::~FileManageWidget()
{
}

void
FileManageWidget::addWorkFile(QString file_path, DataEdit *edit)
{
    //Add Real File Name
    QString file_name;
    int v;
    if(file_path.isEmpty()){
        file_name = "func1.rcf";
        for(int i = 0; i != work_file_widget_->count(); ++i)
            if(work_file_widget_->item(i)->text() == file_name){
                v=work_file_widget_->count()+1;
                file_name.replace(4, 1,
                                  QString::number(v));
            }
    }
    else
        file_name = QFileInfo(file_path).fileName();
    edit->file_name_ = file_name;
    QListWidgetItem *item = new QListWidgetItem(file_name);
    connect(edit, &DataEdit::renameSuccess, [item](const QString &file_name){item->setText(file_name);});
    item->setData(Qt::UserRole, QVariant::fromValue<DataEdit*>(edit));
    work_file_widget_->addItem(item);
    //	item->setSizeHint (QSize(file_widget->rect().width(),FILE_ITEM_HEIGHT/*file_widget->rect().height()*/));
    work_file_widget_->setCurrentItem(item);
    work_space_->setCurrentEdit(item);
}

QListWidgetItem* FileManageWidget::currentItem()
{
    return work_file_widget_->currentItem();
}

void
FileManageWidget::removeWorkFile(QListWidgetItem *item)
{
    delete item->data(Qt::UserRole).value<DataEdit*>();

    item->setData(Qt::UserRole, QVariant::fromValue<DataEdit*>(NULL));
    delete item;
    work_file_widget_->setCurrentItem(NULL);
}
