/**
  *@brief       编辑器高亮声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加SIAL语言保留字解析及日志系统
  *@copyright   siasun.co
  */
#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include "unit.h"

/*
 * 高亮编辑内容
*/
class QTextDocument;

class SyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT
private:
	struct HighlightingRule
	{
		QRegExp pattern_;
		QTextCharFormat format_;
	};
public:
	SyntaxHighlighter(QTextDocument *parent, EditType type);
	QTextCharFormat formatFor(int positionInBlock);
public slots:
	void resetFormat(const TextTheme &text_theme);
	void setEditType(EditType type){
		edit_type_ = type;
		rehighlight();
	}
public:
	EditType edit_type_;
protected:
	void highlightBlock(const QString &text) override;
private:
	QVector<HighlightingRule> highlightingRules;

	QRegExp singleCommentExp_;
	QRegExp mulCommentStartExp_;
	QRegExp mulCommentEndExp_;

	QTextCharFormat keywordFormat_;
	QTextCharFormat commentFormat_;
	QTextCharFormat preprocessorFormat_;
	QTextCharFormat functionFormat_;
	QTextCharFormat numberFormat_;
	QTextCharFormat symbolFormat_;
};

#endif // SYNTAXHIGHLIGHTER_H
