#ifndef UNIT
#define UNIT
#include <QApplication>
#include <QTextCharFormat>
#include <QList>
/*
 * This header file is to define common property.
*/
#define _MY_DEBUG_
#define _CLEAN_SETTINGS_

#define PREDEFINE_THEME_COUNT (1)
#define PREDEFINED_FONT_SIZE (17)
#define ITEM_HEIGHT (25)

#define SEARCH_PRED	false
#define SEARCH_NEXT	true

#define CHAR_SPACING (f.pixelSize()/5)

enum EditType{ SIAL,PLAIN};
enum ReplaceType{KEEP_POS, MOVE_NEXT, ALLWORD};
enum TextBlockState{COMMENT_IN_BLOCK = 1, NOT_MARK_LINE = 1 << 1};//Bit 1 represent True!!! -1 is BlockUserData's default.

#define FORMAT_COUNT		(7)
enum HighlightFormat
{NORMAL_FORMAT, FUNCTION_FORMAT, KEYWORD_FORMAT, NUMBER_FORMAT, SYMBOL_FORMAT, PREPROCESSOR_FORMAT, COMMENT_FORMAT};

struct TextTheme{//主题
	QTextCharFormat formats[FORMAT_COUNT];
	QString theme_name;
};

struct CodeEditThemes{
	TextTheme& CurrentTheme(){
		return themes[cur_num];
	}

//Member
	QList<TextTheme> themes;
	int cur_num;
};

struct FileType{
	QString suffix;
	QString filter;
};

extern QVector<FileType> g_file_types;
extern QString g_file_filter;
#endif // UNIT
