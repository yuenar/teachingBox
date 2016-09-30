/**
  *@brief       编辑器作业文件编辑窗类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "EditorWindow.h"
#include <QtWidgets>
#include "CodeEdit.h"
#include "SyntaxHighlighter.h"
#include "UI/SearchWidget.h"
#include "NoFocusFrameDelegate.h"
#include "DataEdit.h"
#include <log/QsLog.h>

#pragma execution_character_set("utf-8")

EditorWindow::EditorWindow(DataEdit *edit, QWidget *parent) :
	QWidget(parent),
	title_(new QLabel(this)),
	close_(new QToolButton(this)),
	code_edit_(new CodeEdit(edit, this)),
	line_hint_(new QLabel(this)),
	edit_type_hint_(new QComboBox(this)),
	search_widget_(new SearchWidget(this)),
	layout_(new QVBoxLayout),
	mark_selections_(code_edit_->mark_selections_)
{
	edit_type_hint_->setObjectName("edit_type");
	//Header
	close_->setIcon(QIcon(":/icon/close.png"));
	//Hint
	edit_type_hint_->setItemDelegate(new NoFocusFrameDelegate(edit_type_hint_));
    edit_type_hint_->addItem(tr("SIAL语言"));
    edit_type_hint_->addItem(tr("纯文本"));
	//Edit
	code_edit_->setAttribute(Qt::WA_DeleteOnClose);
	code_edit_->verticalScrollBar()->installEventFilter(this);//To watch the wheel event
	connect(edit, &DataEdit::renameSuccess, this, &EditorWindow::updateTextName);
	updateTextName();
	updateLineHint();
	updateEditorTypeHint();
	//Visible
	code_edit_->setVisible(true);
	search_widget_->setVisible(false);
	updateLineHint();
	updateEditorTypeHint();

	//Layout
	QHBoxLayout *header_layout = new QHBoxLayout;
	header_layout->addWidget(title_);
	header_layout->addWidget(line_hint_);
	header_layout->addWidget(edit_type_hint_);
	header_layout->addWidget(close_);

	layout_->addLayout(header_layout);
	layout_->addWidget(code_edit_, 10);
	layout_->addWidget(search_widget_);
	setLayout(layout_);

	//Connect
	connect(close_, &QToolButton::pressed, this, &EditorWindow::close);
	connect(code_edit_, &CodeEdit::currentEdit, this, &EditorWindow::currentWindow);
	connect(code_edit_, &QPlainTextEdit::cursorPositionChanged, this, &EditorWindow::updateLineHint);
	connect(edit_type_hint_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[this](int index){this->code_edit_->data_edit_->highlighter_->setEditType(static_cast<EditType>(index));});
	connect(search_widget_, &SearchWidget::searchDirection, this, &EditorWindow::search);
	connect(search_widget_, &SearchWidget::replaceType, this, &EditorWindow::replace);
	connect(search_widget_, &SearchWidget::updateSearch, this, &EditorWindow::highlightSearchWord);
	connect(edit->document(), &QTextDocument::contentsChange, this, &EditorWindow::highlightChangedText);
	connect(code_edit_, &CodeEdit::markLineSelectionsUpdated, this, &EditorWindow::updateExtraSelections);
	updateFormat();
}

EditorWindow::~EditorWindow()
{
}

void
EditorWindow::setEdit(DataEdit *edit)
{
	disconnect(code_edit_->data_edit_, &DataEdit::renameSuccess, this, &EditorWindow::updateTextName);
	disconnect(code_edit_->data_edit_->document(), &QTextDocument::contentsChange, this, &EditorWindow::highlightChangedText);
	code_edit_->setDataEdit(edit);
	connect(edit, &DataEdit::renameSuccess, this, &EditorWindow::updateTextName);
	connect(edit->document(), &QTextDocument::contentsChange, this, &EditorWindow::highlightChangedText);
	updateTextName();
	updateLineHint();
	updateEditorTypeHint();
	highlightSearchWord();
}

void
EditorWindow::updateFormat()
{
	code_edit_->SetNormalFormat(code_edit_->data_edit_->normal_format_);
}
void
EditorWindow::updateLineHint()
{
    line_hint_->setText(tr("行: %1,列: %2\t")
						.arg(code_edit_->textCursor().blockNumber() + 1).arg(code_edit_->textCursor().columnNumber()));
}

void
EditorWindow::updateEditorTypeHint()
{
	edit_type_hint_->setCurrentIndex(code_edit_->editType());
}

void
EditorWindow::updateTextName()
{
	title_->setText(code_edit_->data_edit_->file_name_);
}

//Search & Replace

void
EditorWindow::search(bool search_direction)
{
	QTextDocument::FindFlags flag;
	if(search_direction == SEARCH_PRED) flag |= QTextDocument::FindBackward;
	if(search_widget_->caseSensitive()) flag |= QTextDocument::FindCaseSensitively;
	if(search_widget_->searchWholeWord()) flag |= QTextDocument::FindWholeWords;
	QString find_word = search_widget_->findWord();
	if(search_widget_->searchWithRegExp()){
		QRegExp reg_exp(find_word);
		if(reg_exp.isValid()) searchForRegExp(code_edit_, reg_exp, search_direction);
	}
	else
		code_edit_->find(find_word, flag);
}

bool
EditorWindow::replace(ReplaceType type)
{
	QTextCursor cursor = code_edit_->textCursor();
	QString replace_word = search_widget_->replaceWord();
	if(type == ReplaceType::KEEP_POS){
		if(cursor.hasSelection()){
			if(search_widget_->searchWithRegExp()){
				QString select_word = cursor.selectedText();
				QRegExp reg_exp(search_widget_->findWord());
				if(reg_exp.isValid()) replace_word = select_word.replace(reg_exp, replace_word);
			}
			cursor.insertText(replace_word);
		}
		else return false;
	}
	else if(type == ReplaceType::MOVE_NEXT){
		if(replace(ReplaceType::KEEP_POS))
			search(SEARCH_NEXT);
		else
			return false;
	}
	else{
		QTextCursor t = code_edit_->textCursor();
		t.movePosition(QTextCursor::Start);
		code_edit_->setTextCursor(t);
		QTextDocument::FindFlags flag;
		if(search_widget_->caseSensitive()) flag |= QTextDocument::FindCaseSensitively;
		if(search_widget_->searchWholeWord()) flag |= QTextDocument::FindWholeWords;
		QString find_word = search_widget_->findWord();
		if(search_widget_->searchWithRegExp()){
			if(find_word.isEmpty()) return true;
			QRegExp reg_exp(find_word);
			while(searchForRegExp(code_edit_, reg_exp, SEARCH_NEXT)){
				QTextCursor cur = code_edit_->textCursor();
				QString select_word = cur.selectedText();
				cur.insertText(select_word.replace(reg_exp, replace_word));
			}
		}
		else
			while(code_edit_->find(find_word, flag))
				code_edit_->textCursor().insertText(replace_word);
		code_edit_->setTextCursor(cursor);
	}
	return true;
}

void
EditorWindow::highlightSearchWord()
{
	if(!search_widget_->isVisible()) return;
	search_selections_.clear();
	QTextCursor cursor, init_cursor;
	init_cursor = cursor = code_edit_->textCursor();

	cursor.movePosition(QTextCursor::Start);//Move to the start of the document.
	code_edit_->setTextCursor(cursor);//Because we want to highlight the word in the whole document.

	QTextEdit::ExtraSelection selection;
	selection.format.setBackground(QColor(Qt::yellow));
	//Get search condition
	QString find_word = search_widget_->findWord();
	QTextDocument::FindFlags flag;
	if(search_widget_->caseSensitive()) flag |= QTextDocument::FindCaseSensitively;
	if(search_widget_->searchWholeWord()) flag |= QTextDocument::FindWholeWords;
	//Search
	if(search_widget_->searchWithRegExp()){
		if(find_word.isEmpty())
			search_selections_.clear();
		else{
			QRegExp reg_exp(find_word);
			int cnt = 0;
			while(searchForRegExp(code_edit_, reg_exp, SEARCH_NEXT)){
                QLOG_INFO() << ++cnt;
				selection.cursor = code_edit_->textCursor();
				search_selections_.append(selection);
			}
		}
	}
	else{
		int cnt = 0;
		while(code_edit_->find(find_word, flag)){
            QLOG_INFO() << ++cnt;
			selection.cursor = code_edit_->textCursor();
			search_selections_.append(selection);
		}
	}
	updateExtraSelections();
	code_edit_->setTextCursor(init_cursor);
}

void
EditorWindow::updateExtraSelections()
{
	code_edit_->setExtraSelections(search_selections_ + mark_selections_);
}

void
EditorWindow::highlightChangedText(int posi)
{
	QList<QTextEdit::ExtraSelection>::iterator it = search_selections_.begin();
	QTextCursor init_cursor = code_edit_->textCursor();
	while(it != search_selections_.end())
		if(it->cursor.position() >= posi)break;
		else ++it;
	if(it != search_selections_.begin()){
		QTextCursor cursor = (it - 1)->cursor;
		cursor.setPosition(cursor.anchor());//The contentsChange may influence last selection
		code_edit_->setTextCursor(cursor);
	}
	else
		code_edit_->moveCursor(QTextCursor::Start);
	search_selections_.erase(it, search_selections_.end());
	QTextEdit::ExtraSelection selection;
	selection.format.setBackground(QColor(Qt::yellow));
	//Get search condition
	QString find_word = search_widget_->findWord();
	QTextDocument::FindFlags flag;
	if(search_widget_->caseSensitive()) flag |= QTextDocument::FindCaseSensitively;
	if(search_widget_->searchWholeWord()) flag |= QTextDocument::FindWholeWords;
	//Search
	if(search_widget_->searchWithRegExp()){
		if(find_word.isEmpty())
			search_selections_.clear();
		else{
			QRegExp reg_exp(find_word);
			int cnt = 0;
			while(searchForRegExp(code_edit_, reg_exp, SEARCH_NEXT)){
                QLOG_INFO() << ++cnt;
				selection.cursor = code_edit_->textCursor();
				search_selections_.append(selection);
			}
		}
	}
	else{
		int cnt = 0;
		while(code_edit_->find(find_word, flag)){
            QLOG_INFO() << ++cnt;
			selection.cursor = code_edit_->textCursor();
			search_selections_.append(selection);
		}
	}
	updateExtraSelections();
	code_edit_->setTextCursor(init_cursor);
}

void
EditorWindow::mousePressEvent(QMouseEvent *event)
{
	emit currentWindow();
	QWidget::mousePressEvent(event);
}

void
EditorWindow::closeEvent(QCloseEvent *event)
{
	code_edit_->close();
	QWidget::closeEvent(event);
}

void
EditorWindow::keyPressEvent(QKeyEvent *event)
{
	if(code_edit_->isReadOnly() && event->key() == Qt::Key_Escape){
		code_edit_->setReadOnly(false);
		setWindowFlags(Qt::SubWindow);
		emit reAddWindow(this);
		return;
	}
	QWidget::keyPressEvent(event);
}

//Zoom
bool
EditorWindow::eventFilter(QObject *obj, QEvent *event){
	if(obj == code_edit_->verticalScrollBar() && event->type() == QEvent::Wheel){
		QWheelEvent *wheel = static_cast<QWheelEvent*>(event);
			if(wheel->modifiers() == Qt::ControlModifier){
				QFont f = code_edit_->data_edit_->normal_format_.font();
				int size = f.pixelSize();
				if(wheel->delta() > 0)
					size = std::min(size + 1, 50);
				else
					size = std::max(size - 1, 9);
				f.setPixelSize(size);
				emit updateFontSize(f);
				return true;
			}
	}
	return QWidget::eventFilter(obj, event);
}
