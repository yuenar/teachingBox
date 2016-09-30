/**
  *@brief       编辑器作业文件编辑窗类声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QWidget>
#include <QHBoxLayout>
#include "unit.h"
#include <QTextEdit>
#include <QTextBlock>

class CodeEdit;
class DataEdit;
class QLabel;
class QComboBox;
class SearchWidget;
class QToolButton;
class QVBoxLayout;
class SyntaxHighlighter;

class EditorWindow : public QWidget
{
    Q_OBJECT
public:
	explicit EditorWindow(DataEdit *edit, QWidget *parent = 0);
    ~EditorWindow();
	void setEdit(DataEdit *edit);
	void updateFormat();
signals:
	void reAddWindow(EditorWindow *window);
	void updateFontSize(QFont);
	void currentWindow();
public slots:
	void updateLineHint();
	void updateEditorTypeHint();
	void updateTextName();
	void search(bool search_direction);
	bool replace(ReplaceType type);
	void highlightSearchWord();
	void updateExtraSelections();
	void highlightChangedText(int posi);
protected:
	void mousePressEvent(QMouseEvent *event)override;
	void keyPressEvent(QKeyEvent *event)override;
	void closeEvent(QCloseEvent *event)override;
	bool eventFilter(QObject *, QEvent *)override;
//成员
public:
	QLabel *title_;
	QToolButton *close_;
	CodeEdit *code_edit_;
	QLabel *line_hint_;
	QComboBox *edit_type_hint_;
	SearchWidget *search_widget_;
	QVBoxLayout *layout_;
	QList<QTextEdit::ExtraSelection> search_selections_;
	QList<QTextEdit::ExtraSelection> &mark_selections_;
};

#endif // EDITORWINDOW_H
