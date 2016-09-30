/**
  *@brief       编辑器核心类实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "CodeEdit.h"
#include <QtWidgets>
#include "CodeEdit.h"
#include "DataEdit.h"
#include "SyntaxHighlighter.h"
#include <log/QsLog.h>

#pragma execution_character_set("utf-8")

CodeEdit::CodeEdit(DataEdit *data_edit, QWidget *parent):
	QPlainTextEdit(parent),
	data_edit_(0),
	line_counter_(new LineCounter(this))
{
	setDataEdit(data_edit);
	setLineWrapMode(QPlainTextEdit::NoWrap);

	//LineCounter Init
	connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEdit::UpdateLineCounterWidth);
	connect(this, &QPlainTextEdit::updateRequest, this, &CodeEdit::UpdateLineCounterArea);
	connect(line_counter_, &LineCounter::lineCheck, this, &CodeEdit::setMark);
	UpdateLineCounterWidth(1);
}

int
CodeEdit::SetLineCounterWidth(int block_count){
	int digits = 2;
	while(block_count >= 100){
		block_count /= 10;
		++digits;
	}
	return line_counter_->line_width_ = (digits + 4) * fontMetrics().width(QLatin1Char('9'));
}

void
CodeEdit::UpdateLineCounterWidth(int block_count){
	setViewportMargins(SetLineCounterWidth(block_count), 0, 0, 0);
}

void
CodeEdit::UpdateLineCounterArea(QRect rect, int dy){
	if(dy)
		line_counter_->scroll(0, dy);
	else
		line_counter_->update(0, rect.y(), line_counter_->width(), rect.height());
	if (rect.contains(viewport()->rect()))				///confusing
			UpdateLineCounterWidth(blockCount());
}

void
CodeEdit::resizeEvent(QResizeEvent *e){
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	line_counter_->setGeometry(cr.left(), cr.top(), line_counter_->line_width_, cr.height());
}

void
CodeEdit::mousePressEvent(QMouseEvent *e)
{
	emit currentEdit();
	QPlainTextEdit::mousePressEvent(e);
}


void
CodeEdit::LineCounterPaintEvent(QPaintEvent *event){
	QRect rect = event->rect();
	QPainter painter(line_counter_);
	painter.fillRect(rect, Qt::lightGray);

	painter.setFont(font());
	painter.setPen(Qt::black);
	QTextBlock block = firstVisibleBlock();
	int block_num = block.blockNumber();
	int h = blockBoundingRect(block).height();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + h;
	while(block.isValid() && top < rect.bottom()){
		if(bottom > rect.top()){
			QString num = QString::number(block_num + 1);
			if(block == textCursor().block()){
				QFont f = font();
				f.setUnderline(true);
				painter.setFont(f);
				painter.drawText(0, top, line_counter_->width(), fontMetrics().height(),
								 Qt::AlignCenter, num);
				painter.setFont(font());
			}
			else painter.drawText(0, top, line_counter_->width(), fontMetrics().height(),
								 Qt::AlignCenter, num);
			if(!(block.userState() & NOT_MARK_LINE)){
                QLOG_INFO() << "MARK_LINE: " << block.blockNumber() << endl
						 << "UserState: " << block.userState() << endl;
				painter.save();
				painter.setRenderHint(QPainter::Antialiasing);
				QLinearGradient gradient(h, top, 0, top + h);
				gradient.setColorAt(0.1, QColor("#ffc0c0"));
				gradient.setColorAt(1, Qt::red);
				painter.setPen(Qt::NoPen);
				painter.setBrush(QBrush(gradient));
				painter.drawEllipse(0, top, h, h);
				painter.restore();
			}
		}
		block = block.next();
		top = bottom;
		bottom = top + blockBoundingRect(block).height();
		++block_num;
	}
}

void
CodeEdit::updateMarkLineSelections()
{
	setExtraSelections(mark_selections_);
	emit markLineSelectionsUpdated();
}
void
CodeEdit::SetNormalFormat(const QTextCharFormat &normal){
	setStyleSheet(tr("QPlainTextEdit{"
					 "color: %1;"
					 "background-color: %2;"
					 "font-weight: %3;"
					 "font-style: %4;"
					 "text-decoration: %5;"
					 "font-size: %6px;"
					 "font-family: %7;"
//					 "letter-spacing: %8px;"
					 "selection-background-color: #54b5ff;"
				 "}").arg(normal.foreground().color().name())
				  .arg(normal.background().color().name())
				  .arg((normal.fontWeight() == QFont::Bold) ? "bold" : "normal")
				  .arg(normal.fontItalic() ? "italic" : "normal")
				  .arg((normal.underlineStyle()== QTextCharFormat::SingleUnderline) ? "underline" : "none")
				  .arg(QString::number(normal.font().pixelSize()))
				  .arg('"' + normal.fontFamily() + '"')
//				  .arg(QString::number(normal.font().pixelSize() / CHAR_SPACING_DECREASE))
				  );
	setTabStopWidth(4 * fontMetrics().width(' '));
}

void
CodeEdit::setDataEdit(DataEdit *data_edit)
{
	if(data_edit_){
		disconnect(data_edit_, &DataEdit::markLineChange, this, &CodeEdit::changeMarkLine);
		disconnect(data_edit_, &DataEdit::markLineChanged, this, &CodeEdit::highlightMarkLine);
	}
	data_edit_ = data_edit;
	setDocument(data_edit_->document());
	SetNormalFormat(data_edit_->normal_format_);
	connect(data_edit_, &DataEdit::markLineChange, this, &CodeEdit::changeMarkLine);
	connect(data_edit_, &DataEdit::markLineChanged, this, &CodeEdit::highlightMarkLine);
	highlightMarkLine();
}

EditType CodeEdit::editType()
{
	return data_edit_->highlighter_->edit_type_;
}

void
CodeEdit::setMark(int mark_line_height)
{
	QTextBlock block = firstVisibleBlock();
	int line_num = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	line_num += (mark_line_height - top) / blockBoundingRect(block).height();
	data_edit_->setMarkLine(line_num);
	line_counter_->repaint();
}

void
CodeEdit::highlightMarkLine()
{
	mark_selections_.clear();
	QTextEdit::ExtraSelection selection;
	selection.format.setBackground(QColor(Qt::blue).lighter(170));
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	QTextDocument *doc = document();
	for(int i = 0; i != doc->blockCount(); ++i){
		QTextBlock block = doc->findBlockByNumber(i);
		if(!(block.userState() & NOT_MARK_LINE)){
			selection.cursor = QTextCursor(block);
			mark_selections_.append(selection);
		}
	}
	updateMarkLineSelections();
}

void
CodeEdit::changeMarkLine(QTextBlock mark_block, bool add)
{
	QTextCursor cursor(mark_block);
	if(add){
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::blue).lighter(170));
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = cursor;
		mark_selections_.append(selection);
	}
	else{
		for(auto it = mark_selections_.begin(); it != mark_selections_.end(); ++it)
			if((*it).cursor == cursor){
				mark_selections_.erase(it);
				break;
			}
	}
	updateMarkLineSelections();
}
void
CodeEdit::keyPressEvent(QKeyEvent *e){
	if(editType() == EditType::PLAIN){
		QPlainTextEdit::keyPressEvent(e);
		return;
	}

	QTextCursor cur = textCursor();
	QString pred = cur.block().text();
	QString addition;
	int pos = cur.positionInBlock();
	bool addBlock = false;
	bool keepPrePosi = true;
	if(e->key() == Qt::Key_Return){
		keepPrePosi = false;
		//Keep Tab Positon
		int tab_size = 0;
		while(tab_size < pred.size() && pred.at(tab_size) == '\t') ++tab_size;
		addition.append(QString(tab_size, '\t'));

		if(pred.size() > 0){
			while(--pos >= 0 && pred.at(pos).isSpace());//Move pos to Not space char
			if(pos >= 0){
				if(pred.at(pos) == '{' && pos + 1 < pred.size() && pred.at(pos + 1) == '}'){
					addition.append('\t');
					addBlock = true;
				}
				else if(!QString("};").contains(pred.at(pos)) && pred.at(0) != '#'/*No marco*/)//Not end indication or is a marco
					addition.append('\t');
			}
		}
	}
	//Complete parenthesis
	else if(pos >= pred.size() || pred.at(pos).isPunct()){
		switch (e->key()) {
		case Qt::Key_ParenLeft:
			addition = ")";
			break;
		case Qt::Key_BracketLeft:
			addition = "]";
			break;
		case Qt::Key_BraceLeft:
			addition = "}";
			break;
		case Qt::Key_QuoteDbl:
			addition.append('\"');
			break;
		case Qt::Key_Less:
			if(0 < pred.size() && pred.at(0) == '#') addition.append(">");
			break;
		default:
			break;
		}
	}

	QPlainTextEdit::keyPressEvent(e);

	if(!addition.isEmpty()){
		cur = textCursor();
		pos = cur.position();
		cur.beginEditBlock();
		cur.insertText(addition);
		cur.endEditBlock();
		if(keepPrePosi){
			cur.setPosition(pos);
			setTextCursor(cur);
		}
		else if(addBlock){
			cur = textCursor();
			pos = cur.position();
			cur.insertBlock();
			addition.chop(1);//Chop a tab
			cur.insertText(addition);
			cur.setPosition(pos);
			setTextCursor(cur);
		}
	}
}


void
LineCounter::mousePressEvent(QMouseEvent *e)
{
	if(QGuiApplication::mouseButtons() & Qt::LeftButton) emit lineCheck(e->y());
	QWidget::mousePressEvent(e);
}
