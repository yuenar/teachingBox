/**
  *@brief       编辑器作业文件管理类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef FILEMANAGEWIDGET_H
#define FILEMANAGEWIDGET_H
#include <QWidget>
#include "DataEdit.h"
#include "WidgetHeader.h"
#include "FilelistWidget.h"
#include "DirView.h"
/*
 * 管理工作文件和文件夹
*/

class WorkSpace;
class CodeEdit;
class QTextCharFormat;


class FileManageWidget :public QWidget
{
    Q_OBJECT
public:
    FileManageWidget(WorkSpace *parent);
    ~FileManageWidget();
    void addWorkFile(QString file_path, DataEdit *edit);
    QListWidgetItem *currentItem();
signals:
    void cleanWorkingFiles();
public slots:
    void removeWorkFile(QListWidgetItem *item);
private:
    void initFileWidget();
    //Data
public:
    WorkSpace *work_space_;
    FileListWidget *work_file_widget_;
    DirView *folder_widget_;
private:
    WidgetHeader *work_file_widget_header_;
    WidgetHeader *folder_widget_header_;
};

#endif // FILEMANAGEWIDGET_H
