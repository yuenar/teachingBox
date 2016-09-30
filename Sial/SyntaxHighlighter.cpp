/**
  *@brief       编辑器高亮实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加SIAL语言保留字解析及日志系统
  *@copyright   siasun.co
  */
#include "SyntaxHighlighter.h"
#include "unit.h"
#include <log/QsLog.h>

const int  keyWordNum=31;//保留字数目

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent, EditType type)
    : QSyntaxHighlighter(parent),
      edit_type_(type)
{
    HighlightingRule rule;

    //Function
    rule.pattern_ = QRegExp("\\b[A-Za-z0-9_]+(?=\\s?\\()");
    highlightingRules.append(rule);

    //Keyword保留字
    QStringList keywordPatterns;
    keywordPatterns << "\\bbegin\\b"		<< "\\bend\\b"		<< "\\bbool\\b"
                    << "\\bstring\\b"		<< "\\bsio\\b"		<< "\\bjoint\\b"
                    << "\\bdio\\b"			<< "\\bdouble\\b"   << "\\belse\\b"
                    << "\\bfalse\\b"		<< "\\bfloat\\b"	<< "\\bfor\\b"
                    << "\\bframe\\b"		<< "\\bif\\b"       << "\\bint\\b"
                    << "\\breturn\\b"		<< "\\bcall\\b"     << "\\bbreak\\b"
                    << "\\bdo until\\b"		<< "\\bswitch\\b"	<< "\\btrue\\b"
                    << "\\btool\\b"         << "\\bwhile\\b"	<< "\\bconfig\\b"
                    << "\\bpropty\\b"		<< "\\bpoint\\b"    << "\\bfunc\\b"
                    << "\\bMoveJ\\b"		<< "\\bMoveL\\b"    << "\\bMoveC\\b"
                    << "\\btransf\\b";	//Size 28
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern_ = QRegExp(pattern);
        highlightingRules.append(rule);
    }

    //Symbol运算符相关
    rule.pattern_ = QRegExp("[\\+\\|\\^\\*\\(\\)\\{\\}\\[\\]\\.\\?~!%;:,/-&=<>]");
    highlightingRules.append(rule);

    //Number
    rule.pattern_ = QRegExp("(-|\\b)\\d+(u|U)?(l{0,2}|L{0,2})\\b");							//Int
    highlightingRules.append(rule);
    rule.pattern_ = QRegExp("(-|\\b)((\\d+\\.)|(\\.\\d+)|(\\d+\\.\\d+))(f|F)?(l|L)?\\b");	//Normal float
    highlightingRules.append(rule);
    rule.pattern_ = QRegExp("(-|\\b)\\d+(e|E)\\d+(f|F)?(l|L)?\\b");							//Scientific notation
    highlightingRules.append(rule);

    //Preprocessor
    rule.pattern_ = QRegExp("^#[^\n]+$");
    highlightingRules.append(rule);

    //Comment注释
    singleCommentExp_ = QRegExp("//[^\n]*");
    mulCommentStartExp_ = QRegExp("/\\*");
    mulCommentEndExp_ = QRegExp("\\*/");
}

QTextCharFormat SyntaxHighlighter::formatFor(int positionInBlock)
{
    return format(positionInBlock);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    if(edit_type_ == PLAIN) return;

    foreach (const HighlightingRule &rule, highlightingRules){
        QRegExp expression(rule.pattern_);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format_);
            index = expression.indexIn(text, index + length);
        }
    }

    //The (/**/) type of comment
    setCurrentBlockState(currentBlockState() | COMMENT_IN_BLOCK);
    int startIndex = 0;
    if (previousBlockState() & COMMENT_IN_BLOCK)
        startIndex = mulCommentStartExp_.indexIn(text);//Find Begin/*
    while (startIndex >= 0) {
        int endIndex = startIndex;
        while(endIndex >= 0){
            if((endIndex = mulCommentEndExp_.indexIn(text, endIndex)) != startIndex + 1)//Not superposition like /*/
                break;
            endIndex += 2;
        }
        int commentLength;
        if (endIndex == -1) {//No End*/
            setCurrentBlockState(currentBlockState() & (~COMMENT_IN_BLOCK));
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + mulCommentEndExp_.matchedLength();
        }
        setFormat(startIndex, commentLength, commentFormat_);
        startIndex = mulCommentStartExp_.indexIn(text, startIndex + commentLength);
    }
    int index = singleCommentExp_.indexIn(text);
    if(index >= 0 && format(index) != commentFormat_){
        int length = singleCommentExp_.matchedLength();
        setFormat(index, length, commentFormat_);
    }
}

void SyntaxHighlighter::resetFormat(const TextTheme &text_theme){
    functionFormat_		= text_theme.formats[FUNCTION_FORMAT];
    keywordFormat_		= text_theme.formats[KEYWORD_FORMAT];
    preprocessorFormat_ = text_theme.formats[PREPROCESSOR_FORMAT];
    commentFormat_		= text_theme.formats[COMMENT_FORMAT];
    numberFormat_		= text_theme.formats[NUMBER_FORMAT];
    symbolFormat_		= text_theme.formats[SYMBOL_FORMAT];

    int i = 0;
    highlightingRules[i++].format_ = functionFormat_;
    for(int cnt = 0; cnt != keyWordNum; ++cnt)
        highlightingRules[i++].format_ = keywordFormat_;
    highlightingRules[i++].format_ = symbolFormat_;
    for(int cnt = 0; cnt != 3; ++cnt)
        highlightingRules[i++].format_ = numberFormat_;
    highlightingRules[i++].format_ = preprocessorFormat_;
    rehighlight();
}
