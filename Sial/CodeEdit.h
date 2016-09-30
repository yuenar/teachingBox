/**
  *@brief       编辑器核心类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef CODEEDIT_H
#define CODEEDIT_H

#include "DataEdit.h"
#include <QListWidget>
#include "unit.h"
#include <QDebug>

class LineCounter;
class QLabel;
class QComboBox;
/*CodeEdit
 *   这个类是在一个小部件来展示，它没有它自己的文件。
 *它有许多可见的元素，但高亮显示需要一定的数据源。
 * 缺省状态:
 *	可见
*/
class CodeEdit : public QPlainTextEdit
{
	Q_OBJECT
	friend class LineCounter;
public:
	CodeEdit(DataEdit *data_edit, QWidget *parent);
	void SetNormalFormat(const QTextCharFormat &normal);
	void setDataEdit(DataEdit *data_edit);
	EditType editType();
signals:
	void currentEdit();
	void markLineSelectionsUpdated();
public slots:
	void setMark(int mark_line_height);
	void highlightMarkLine();
	void changeMarkLine(QTextBlock mark_block, bool add);
protected:
	void resizeEvent(QResizeEvent *e)override;
	void mousePressEvent(QMouseEvent *e)override;
	void keyPressEvent(QKeyEvent *e)override;
private slots:
	void UpdateLineCounterWidth(int block_count);
	void UpdateLineCounterArea(QRect, int);
private:
	int SetLineCounterWidth(int block_count);
	void LineCounterPaintEvent(QPaintEvent *event);
	void updateMarkLineSelections();
public:
	DataEdit *data_edit_;
	QList<QTextEdit::ExtraSelection> mark_selections_;
private:
	LineCounter *line_counter_;
};

class LineCounter : public QWidget
{//行标号
	Q_OBJECT
	friend class CodeEdit;
signals:
	void lineCheck(int mark_line_height);
protected:
	void mousePressEvent(QMouseEvent *e);
private:
	LineCounter(CodeEdit *parent = 0):QWidget(parent){
		code_edit_ = parent;
	}
	QSize sizeHint() const override{
		return QSize(line_width_, 0);
	}
	void paintEvent(QPaintEvent *event)override{
		code_edit_->LineCounterPaintEvent(event);
	}
	CodeEdit *code_edit_;
	int line_width_;
};

#endif // CODEEDIT_H
