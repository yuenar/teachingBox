/**
  *@brief       编辑器主题声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统及界面修改
  *@copyright   siasun.co
  */
#ifndef TEXTSTYLEDIALOG_H
#define TEXTSTYLEDIALOG_H

#include <QDialog>
#include <QString>
#include <QComboBox>
#include "Sial/WorkSpace.h"
#include "Sial/NoFocusFrameDelegate.h"
#include "Sial/SyntaxHighlighter.h"
#include "unit.h"

/*
 * 编辑风格设置类.
*/
class RobotEditor;
class QAction;
class QLabel;
class QStyledItemDelegate;
class CodeEdit;

namespace Ui {
class TextStyleDialog;
}

struct ColorItem
{
	QString name;
	QColor color;
};

class TextStyleDialog : public QDialog
{
	Q_OBJECT

public:
    explicit TextStyleDialog(const CodeEditThemes &code_edit_theme, WorkSpace *work_space, RobotEditor *parent = 0);
	~TextStyleDialog();
public slots:
	void chooseFormatItem(QTextCharFormat format);
private slots:
    void UpdateCurrentItem(int row);//更新当前项目行
    void UpdateCurrentTheme(int theme_row);//更新当前行主题
	void UpdateForeground(int row);
	void UpdateBackground(int row);
	void UpdateFontFamily(const QString &font_family);
	void UpdateFontSize(int size);
	void UpdateBold(int state);
	void UpdateItalic(int state);
	void UpdateUnderline(int state);

	void AddColorItem();
	void RemoveColorItem();
	void AddTheme();
	void RemoveTheme();
	void Apply();
    inline void Finish(){
		Apply();
		close();
	}
protected:
	void closeEvent(QCloseEvent *e)override;
private:
	struct ColorItem
	{
		ColorItem(const QString &n = QString(), const QColor &c = QColor()):name(n), color(c){}
		QString name;
		QColor color;
	};
private:
	void ReadColorItems();
	void WriteColorItems();
	void InitTheme();
	void InitBox();
	QListWidget* SetBoxWidget(QComboBox *box){
		QListWidget *widget = new QListWidget(this);
		widget->setItemDelegate(new NoFocusFrameDelegate(this));
		box->setModel(widget->model());
		box->setView(widget);
		return widget;
	}

private:
	Ui::TextStyleDialog *ui;
	DataEdit *example_data_;
	CodeEdit *example_edit_;
	WorkSpace *work_space_;
    RobotEditor *main_window_;
	QList<ColorItem> color_list;
	CodeEditThemes code_edit_themes_;
	TextTheme cur_theme_;
	SyntaxHighlighter *highlighter_;
	QListWidget *color_widget_;
};

#endif // TEXTSTYLEDIALOG_H
