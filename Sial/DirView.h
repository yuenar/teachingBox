/**
  *@brief       编辑器作业文件夹类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef DIRVIEW_H
#define DIRVIEW_H
#include <QWidget>

class QFileSystemModel;
class QTreeView;
class WidgetHeader;
class QPushButton;

/*
 * 文件夹管理类
*/
class DirView : public QWidget
{
	Q_OBJECT
public:
    DirView(QWidget *parent = 0);
	void setFolder(const QString &directory);
signals:
	void openFile(const QString &file_name);
public slots:
	void openFolder();
	void closeFolder();
private:
	void Display();
	void HideDirview();
private slots:
	void DoubleClickedItem(const QModelIndex &index);
private:
	QFileSystemModel *dir_model_;
	QTreeView *dir_view_;
	QPushButton *open_folder_button_;
};

#endif // DIRVIEW_H
