/**
  *@brief       编辑器作业文件列表类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H
#include <QListWidget>

/*
 * 存储和显示工作文件
*/
class FileListWidget : public QListWidget
{
	Q_OBJECT
public:
	FileListWidget(QWidget *parent = 0);
    ~FileListWidget();
signals:
	void tryCloseFile(QListWidgetItem *item);
	void currentFile(QListWidgetItem *item);
public slots:
	void emitCloseFileSignal(QListWidgetItem *item);
	void emitCurrentFileSignal(const QModelIndex &index);
protected:
	void contextMenuEvent(QContextMenuEvent * event)override;
private:
};

#endif // FILELISTWIDGET_H
