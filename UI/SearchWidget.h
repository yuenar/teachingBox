/**
  *@brief       编辑器搜索框声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统及界面修改
  *@copyright   siasun.co
  */
#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include "unit.h"

/*
 * 储存搜索信息类
*/

class QPlainTextEdit;
namespace Ui {
class SearchWidget;
}

class SearchWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SearchWidget(QWidget *parent = 0);
	~SearchWidget();
	QString findWord();
	QString replaceWord();
	bool caseSensitive();
	bool searchWholeWord();
	bool searchWithRegExp();
	void replaceKeepPos();
	void replaceMoveNext();
	void replaceAll();
public slots:
	void searchPred();
	void searchNext();
signals:
	void searchDirection(bool direction);
	void replaceType(ReplaceType replace_type);
	void updateSearch();
private:
	Ui::SearchWidget *ui;
};

bool searchForRegExp(QPlainTextEdit *edit, QRegExp &reg_exp, bool search_direction);

#endif // SEARCHWIDGET_H
