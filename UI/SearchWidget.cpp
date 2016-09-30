/**
  *@brief       编辑器搜索框实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统及界面修改
  *@copyright   siasun.co
  */
#include "SearchWidget.h"
#include "ui_searchwidget.h"
#include <QPlainTextEdit>
#include <log/QsLog.h>


SearchWidget::SearchWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SearchWidget)
{
	ui->setupUi(this);
	connect(ui->pred_, &QToolButton::pressed, this, &SearchWidget::searchPred);
	connect(ui->next_, &QToolButton::pressed, this, &SearchWidget::searchNext);
	connect(ui->colose_buttom_, &QToolButton::pressed, [=](){this->setVisible(false);});
	connect(ui->replace_, &QToolButton::pressed, this, &SearchWidget::replaceKeepPos);
	connect(ui->replace_move_next_, &QToolButton::pressed, this, &SearchWidget::replaceMoveNext);
	connect(ui->replace_all_, &QToolButton::pressed, this, &SearchWidget::replaceAll);
	connect(ui->find_word_, &QLineEdit::textChanged, this, &SearchWidget::updateSearch);
	connect(ui->whole_word_, &QCheckBox::stateChanged, this, &SearchWidget::updateSearch);
	connect(ui->case_sensitive_, &QCheckBox::stateChanged, this, &SearchWidget::updateSearch);
	connect(ui->reg_exp_, &QCheckBox::stateChanged, this, &SearchWidget::updateSearch);

}

SearchWidget::~SearchWidget()
{
	delete ui;
}

QString
SearchWidget::findWord()
{
	return ui->find_word_->text();
}

QString
SearchWidget::replaceWord()
{
	return ui->replace_word_->text();
}

bool
SearchWidget::caseSensitive()
{
	return ui->case_sensitive_->checkState() == Qt::Checked;
}

bool
SearchWidget::searchWholeWord()
{
	return ui->whole_word_->checkState() == Qt::Checked;
}

bool
SearchWidget::searchWithRegExp()
{
	return ui->reg_exp_->checkState() == Qt::Checked;
}

void
SearchWidget::searchPred()
{
	emit searchDirection(SEARCH_PRED);
}

void
SearchWidget::searchNext()
{
	emit searchDirection(SEARCH_NEXT);
}

void
SearchWidget::replaceKeepPos()
{
	emit replaceType(ReplaceType::KEEP_POS);
}

void
SearchWidget::replaceMoveNext()
{
	emit replaceType(ReplaceType::MOVE_NEXT);
}

void
SearchWidget::replaceAll()
{
	emit replaceType(ReplaceType::ALLWORD);
}



bool
searchForRegExp(QPlainTextEdit *edit, QRegExp &reg_exp, bool search_direction)
{
	QTextCursor cursor = edit->textCursor();
	int start_pos;
	QString text = edit->toPlainText();
	int pos;
	if(search_direction == SEARCH_NEXT){
		start_pos = cursor.position();
		if(start_pos >= text.size()) return false;
		pos = reg_exp.indexIn(text, start_pos);
	}
	else{
		if(cursor.hasSelection())
			start_pos = cursor.anchor();
		else
			start_pos = cursor.position();
		if(start_pos <= 0) return false;
		start_pos -= text.size() + 1;
		pos = reg_exp.lastIndexIn(text, start_pos);
	}
	if(pos >= 0){
		cursor.setPosition(pos);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, reg_exp.matchedLength());
		edit->setTextCursor(cursor);
		return true;
	}
	return false;
}
