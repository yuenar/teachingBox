/**
  *@brief       编辑器主题实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统及界面修改
  *@copyright   siasun.co
  */
#include <QtWidgets>
#include "Sial/CodeEdit.h"
#include "RobotEditor.h"
#include "TextStyleDialog.h"
#include "ui_textstyledialog.h"
#include "unit.h"
#include <log/QsLog.h>


TextStyleDialog::TextStyleDialog(const CodeEditThemes &code_edit_theme, WorkSpace *work_space, RobotEditor *parent) :
	QDialog(parent),
	ui(new Ui::TextStyleDialog),
    example_edit_(new CodeEdit(new DataEdit(TextTheme(), SIAL, this), this)),
	work_space_(work_space),
	main_window_(parent),
	code_edit_themes_(code_edit_theme),
	cur_theme_(code_edit_themes_.CurrentTheme()),
    highlighter_(new SyntaxHighlighter(example_edit_->document(), SIAL)),
	color_widget_(new QListWidget(this))
{
	ui->setupUi(this);

	//Init example text
	ui->text_layout->addWidget(example_edit_);
	QFile example(":/example_text.txt");
	example.open(QFile::ReadOnly | QFile::Text);
	QTextStream in(&example);
	example_edit_->setPlainText(in.readAll());
	example.close();
	example_edit_->setReadOnly(true);
	example_edit_->setVisible(true);

	highlighter_->resetFormat(code_edit_themes_.CurrentTheme());

	ReadColorItems();
	InitTheme();
	InitBox();
	ui->item_widget->setItemDelegate(new NoFocusFrameDelegate());
	ui->item_widget->setCurrentRow(0);

	connect(ui->theme_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &TextStyleDialog::UpdateCurrentTheme);
	connect(ui->item_widget, &QListWidget::currentRowChanged, this, &TextStyleDialog::UpdateCurrentItem);
	connect(ui->foreground_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &TextStyleDialog::UpdateForeground);
	connect(ui->background_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &TextStyleDialog::UpdateBackground);
	connect(ui->font_family_box, &QComboBox::currentTextChanged, this, &TextStyleDialog::UpdateFontFamily);
	connect(ui->font_size_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &TextStyleDialog::UpdateFontSize);
	connect(ui->bold, &QCheckBox::stateChanged, this, &TextStyleDialog::UpdateBold);
	connect(ui->italic, &QCheckBox::stateChanged, this, &TextStyleDialog::UpdateItalic);
	connect(ui->underline, &QCheckBox::stateChanged, this, &TextStyleDialog::UpdateUnderline);

	connect(ui->add_theme, &QPushButton::clicked, this, &TextStyleDialog::AddTheme);
	connect(ui->remove_theme, &QPushButton::clicked, this, &TextStyleDialog::RemoveTheme);
	connect(ui->OK_button, &QPushButton::clicked, this, &TextStyleDialog::Finish);
	connect(ui->apply_button, &QPushButton::clicked, this, &TextStyleDialog::Apply);
	connect(ui->cancel_button, &QPushButton::clicked, this, &TextStyleDialog::close);
	connect(ui->add_color, &QPushButton::clicked, this, &TextStyleDialog::AddColorItem);
	connect(ui->remove_color, &QPushButton::clicked, this, &TextStyleDialog::RemoveColorItem);

	if(ui->theme_box->currentIndex() != code_edit_themes_.cur_num)
		ui->theme_box->setCurrentIndex(code_edit_themes_.cur_num);
	else
		UpdateCurrentTheme(code_edit_themes_.cur_num);
	ui->font_size_box->setValue(cur_theme_.formats[NORMAL_FORMAT].font().pixelSize());
}

TextStyleDialog::~TextStyleDialog()
{
	disconnect(ui->theme_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &TextStyleDialog::UpdateCurrentTheme);
	disconnect(ui->item_widget, &QListWidget::currentRowChanged, this, &TextStyleDialog::UpdateCurrentItem);
	disconnect(ui->foreground_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &TextStyleDialog::UpdateForeground);
	disconnect(ui->background_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &TextStyleDialog::UpdateBackground);
	disconnect(ui->bold, &QCheckBox::stateChanged, this, &TextStyleDialog::UpdateBold);
	disconnect(ui->italic, &QCheckBox::stateChanged, this, &TextStyleDialog::UpdateItalic);
	disconnect(ui->underline, &QCheckBox::stateChanged, this, &TextStyleDialog::UpdateUnderline);
	delete ui;
}

void TextStyleDialog::chooseFormatItem(QTextCharFormat format)
{
	for(int i = 0; i != FORMAT_COUNT; ++i)
		if(format == cur_theme_.formats[i]){
			UpdateCurrentItem(i);
			break;
		}
}

//Init Theme
//----------------------------------------------------------------------------------------------------------------------------
void TextStyleDialog::InitTheme(){
	ui->theme_box->setStyleSheet("QComboBox QAbstractItemView::item {height: "
									  + QString::number(ITEM_HEIGHT)
									  + "px; }");
	QListWidget *theme_widget = SetBoxWidget(ui->theme_box);
	for(int i = 0;i != code_edit_themes_.themes.size(); ++i){
		theme_widget->addItem(code_edit_themes_.themes[i].theme_name);
	}
	ui->theme_box->setEditable(false);
}


//Init Box
//----------------------------------------------------------------------------------------------------------------------------
void TextStyleDialog::InitBox(){
	color_widget_->setVisible(false);
	color_widget_->setItemDelegate(new NoFocusFrameDelegate());
	QPixmap pix(ITEM_HEIGHT, ITEM_HEIGHT);
	for(int i = 0; i != color_list.size(); ++i){
		pix.fill(color_list[i].color);
		color_widget_->addItem(new QListWidgetItem(QIcon(pix), color_list[i].name));
	}
	//Foreground

	ui->foreground_box->setModel(color_widget_->model());
	QListView *foreground = new QListView(ui->foreground_box);
	foreground->setModel(color_widget_->model());
	ui->foreground_box->setView(foreground);

	ui->foreground_box->setEditable(false);
	//Background

	ui->background_box->setModel(color_widget_->model());
	QListView *background = new QListView(ui->background_box);
	background->setModel(color_widget_->model());
	ui->background_box->setView(background);

	ui->background_box->setEditable(false);

	//Font Family
	QListWidget *font_family_widget = new QListWidget(ui->font_family_box);
	font_family_widget->setItemDelegate(new NoFocusFrameDelegate());
	ui->font_family_box->setModel(font_family_widget->model());
	ui->font_family_box->setView(font_family_widget);
	//Add Font Family
	QFontDatabase data;
	ui->font_family_box->addItems(data.families());
}

//Slots
//----------------------------------------------------------------------------------------------------------------------------

//Theme
void TextStyleDialog::UpdateCurrentTheme(int theme_row){
#ifdef _MY_DEBUG_
    QLOG_INFO() << "UpdateCurrentTheme!";
#endif
	code_edit_themes_.cur_num = theme_row;
	cur_theme_ = code_edit_themes_.CurrentTheme();
	example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
    QLOG_INFO() << "\nCurrentTheme"
			 << "\nForeground: " << cur_theme_.formats[NORMAL_FORMAT].foreground().color()
			 << "\nBackground: " << cur_theme_.formats[NORMAL_FORMAT].background().color();
	highlighter_->resetFormat(cur_theme_);
	UpdateCurrentItem(ui->item_widget->currentRow());
	ui->font_family_box->setCurrentText(cur_theme_.formats[NORMAL_FORMAT].fontFamily());
	ui->font_size_box->setValue(cur_theme_.formats[NORMAL_FORMAT].font().pixelSize());
}

//Item
void TextStyleDialog::UpdateCurrentItem(int row){
#ifdef _MY_DEBUG_
    QLOG_INFO() << "UpdateCurrentItem!";
#endif
	int pos;
	QColor search_color;

	//Foreground
	search_color = cur_theme_.formats[row].foreground().color();
	for(int i = 0; i != color_list.size(); ++i){
		if(color_list[i].color == search_color){
			pos = i;
			break;
		}
	}
	ui->foreground_box->setCurrentIndex(pos);

	//Background
	search_color = cur_theme_.formats[row].background().color();
	for(int i = 0; i != color_list.size(); ++i){
		if(color_list[i].color == search_color){
			pos = i;
			break;
		}
	}
	ui->background_box->setCurrentIndex(pos);

	//Bold
	if(cur_theme_.formats[row].fontWeight() == (int)QFont::Bold)
		ui->bold->setCheckState(Qt::Checked);
	else
		ui->bold->setCheckState(Qt::Unchecked);
	//Italic
	if(cur_theme_.formats[row].fontItalic())
		ui->italic->setCheckState(Qt::Checked);
	else
		ui->italic->setCheckState(Qt::Unchecked);

	//Underline
	if(cur_theme_.formats[row].fontUnderline())
		ui->underline->setCheckState(Qt::Checked);
	else
		ui->underline->setCheckState(Qt::Unchecked);
}


void TextStyleDialog::UpdateForeground(int row){
#ifdef _MY_DEBUG_
    QLOG_INFO() << "UpdateForeground!";
#endif
	int cur_item_row = ui->item_widget->currentRow();
	QColor color = color_list[row].color;
	cur_theme_.formats[cur_item_row].setForeground(QBrush(color));
	if(cur_item_row)
		highlighter_->resetFormat(cur_theme_);
	else
		example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
}
void TextStyleDialog::UpdateBackground(int row){
#ifdef _MY_DEBUG_
    QLOG_INFO() << "UpdateBackground!";
#endif
	int cur_item_row = ui->item_widget->currentRow();
	QColor color = color_list[row].color;
	cur_theme_.formats[cur_item_row].setBackground(QBrush(color));
	if(cur_item_row)
		highlighter_->resetFormat(cur_theme_);
	else
		example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
}

void TextStyleDialog::UpdateBold(int state){
#ifdef _MY_DEBUG_
    QLOG_INFO() << "UpdateBold!";
#endif
	cur_theme_.formats[ui->item_widget->currentRow()].setFontWeight(state == 2 ? QFont::Bold : QFont::Normal);
	if(ui->item_widget->currentRow())
		highlighter_->resetFormat(cur_theme_);
	else
		example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
}
void TextStyleDialog::UpdateItalic(int state){
#ifdef _MY_DEBUG_
    QLOG_INFO() << "UpdateItalic!";
#endif
	cur_theme_.formats[ui->item_widget->currentRow()].setFontItalic(state == 2);
	if(ui->item_widget->currentRow())
		highlighter_->resetFormat(cur_theme_);
	else
		example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
}

void TextStyleDialog::UpdateUnderline(int state){
	cur_theme_.formats[ui->item_widget->currentRow()].setFontUnderline(state == 2);
	if(ui->item_widget->currentRow())
		highlighter_->resetFormat(cur_theme_);
	else
		example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
}

void TextStyleDialog::UpdateFontFamily(const QString &font_family){
	cur_theme_.formats[NORMAL_FORMAT].setFontFamily(font_family);
	example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
}
void TextStyleDialog::UpdateFontSize(int size){
	QFont f = cur_theme_.formats[NORMAL_FORMAT].font();
	f.setPixelSize(size);
	cur_theme_.formats[NORMAL_FORMAT].setFont(f);
	example_edit_->SetNormalFormat(cur_theme_.formats[NORMAL_FORMAT]);
}

//Add & Remove
//----------------------------------------------------------------------------------------------------------------------------
void TextStyleDialog::AddColorItem(){
	QColor color;
	if(!(color = QColorDialog::getColor(Qt::white, this, tr("请选择添加的颜色")
									   , QColorDialog::DontUseNativeDialog)).isValid()) return;
	QString item_name;
	if((item_name = QInputDialog::getText(this, tr("颜色名称"), tr("请输入添加的颜色名称"))).isEmpty()) return;
	QPixmap pix(ITEM_HEIGHT, ITEM_HEIGHT);
	pix.fill(color);
	color_list.append(ColorItem(item_name, color));
	color_widget_->addItem(new QListWidgetItem(QIcon(pix), item_name));
}
void TextStyleDialog::RemoveColorItem(){
	QString item_name;
	if((item_name = QInputDialog::getText(this, tr("颜色名称"), tr("请输入添加的颜色名称"))).isEmpty()) return;
	int index;
	for(index = 0; index != color_list.size(); ++index){
		if(color_list[index].name == item_name)
			break;
	}
	if(index == color_list.size()){
		QMessageBox::warning(this, tr("错误！"), tr("找不到%1颜色").arg(item_name));
		return;
	}
	QColor color = color_list[index].color;
	bool warn = true;
	for(int i = 0; i != code_edit_themes_.themes.size(); ++i)
		for(int j = 0; j != FORMAT_COUNT; ++j){
			if(code_edit_themes_.themes[i].formats[j].foreground().color() == color){
				if(warn){
					if(QMessageBox::warning(this, tr("该颜色正在使用中"), tr("删除会将当前所有使用该颜色的部分替换成白色！\n是否继续删除"),
										 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
						warn = false;
						code_edit_themes_.themes[i].formats[j].setForeground(Qt::white);
					}
					else return;
				}
			}
			if(code_edit_themes_.themes[i].formats[j].background().color() == color){
				if(warn){
					if(QMessageBox::warning(this, tr("该颜色正在使用中"), tr("删除会将当前所有使用该颜色的部分替换成白色！\n是否继续删除"),
										 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
						warn = false;
						code_edit_themes_.themes[i].formats[j].setBackground(Qt::white);
					}
					else return;
				}
			}
		}
	color_list.removeAt(index);
	delete color_widget_->takeItem(index);
}


void TextStyleDialog::AddTheme(){
	QString theme_name;
	if((theme_name = QInputDialog::getText(this, tr("新建主题"), tr("请输入将新建的主题名称"))).isEmpty()) return;
	int insert_pos = code_edit_themes_.themes.size() - 1;
	TextTheme t = cur_theme_;
	t.theme_name = theme_name;
	code_edit_themes_.themes.insert(insert_pos, t);
	ui->theme_box->insertItem(insert_pos, theme_name);
	ui->theme_box->setCurrentIndex(insert_pos);
}
void TextStyleDialog::RemoveTheme(){
	int row = ui->theme_box->currentIndex();
	if(row > 0/*Predefine theme count*/ && row < code_edit_themes_.themes.size() - 1/*Custom theme*/){
		code_edit_themes_.themes.removeAt(row);
		ui->theme_box->removeItem(row);
		ui->theme_box->setCurrentIndex(row);
	}
}


void TextStyleDialog::Apply(){
	if(code_edit_themes_.cur_num >= PREDEFINE_THEME_COUNT)
		code_edit_themes_.CurrentTheme() = cur_theme_;
	else{
		code_edit_themes_.themes[code_edit_themes_.cur_num = code_edit_themes_.themes.size() - 1] = cur_theme_;
		code_edit_themes_.themes.last().theme_name = tr("自定义");
	}

	work_space_->code_edit_themes_ = code_edit_themes_;
	FileListWidget *list = work_space_->file_manage_widget_->work_file_widget_;
	for(int i = 0; i != list->count(); ++i){
		DataEdit *edit = list->item(i)->data(Qt::UserRole).value<DataEdit*>();
		edit->highlighter_->resetFormat(cur_theme_);
		edit->normal_format_ = cur_theme_.formats[NORMAL_FORMAT];
	}
	for(auto i : work_space_->editor_queue_){
		i->updateFormat();
	}
	ui->theme_box->setCurrentIndex(code_edit_themes_.cur_num);
}

void TextStyleDialog::closeEvent(QCloseEvent *e)
{
	WriteColorItems();
	QDialog::closeEvent(e);
}

//Read & Write
void TextStyleDialog::ReadColorItems(){
    QSettings settings("SIASUN", "TEXT");
	int size = settings.beginReadArray("color_items");
#ifdef _CLEAN_SETTINGS_
	size = 0;
#endif
	if(size > 0){
		ColorItem t;
		for(int i = 0; i != size; ++i){
			settings.setArrayIndex(i);
			t.color = settings.value("color").value<QColor>();
			t.name	= settings.value("name").toString();
			color_list.append(t);
		}

	}
	else{
		color_list.append(ColorItem("Transparent",	QColor(0, 0, 0, 0)));
		color_list.append(ColorItem("White",		QColor(Qt::white)));
		color_list.append(ColorItem("Black",		QColor(Qt::black)));
		color_list.append(ColorItem("Red",			QColor(Qt::red)));
		color_list.append(ColorItem("DarkRed",		QColor(Qt::darkRed)));
		color_list.append(ColorItem("Green",		QColor(Qt::green)));
		color_list.append(ColorItem("DarkGreen",	QColor(Qt::darkGreen)));
		color_list.append(ColorItem("Blue",			QColor(Qt::blue)));
		color_list.append(ColorItem("DarkBlue",		QColor(Qt::darkBlue)));
		color_list.append(ColorItem("Cyan",			QColor(Qt::cyan)));
		color_list.append(ColorItem("DarkCyan",		QColor(Qt::darkCyan)));
		color_list.append(ColorItem("Magenta",		QColor(Qt::magenta)));
		color_list.append(ColorItem("DarkMagenta",	QColor(Qt::darkMagenta)));
		color_list.append(ColorItem("Yellow",		QColor(Qt::yellow)));
		color_list.append(ColorItem("DarkYellow",	QColor(Qt::darkYellow)));
		color_list.append(ColorItem("Gray",			QColor(Qt::gray)));
		color_list.append(ColorItem("DarkGray",		QColor(Qt::darkGray)));
		color_list.append(ColorItem("LightGray",	QColor(Qt::lightGray)));
		color_list.append(ColorItem("Dark",			QColor("#1e1e1e")));
		color_list.append(ColorItem("LightBlue",	QColor("#0087cb")));
		color_list.append(ColorItem("Purple",		QColor("#aa55ff")));
		color_list.append(ColorItem("GreyishWhite", QColor("#f0f0f0")));
		color_list.append(ColorItem("DeepPink",		QColor("#ff1493")));
		color_list.append(ColorItem("LightPink",		QColor("#ffb6c1")));
	}
}
void TextStyleDialog::WriteColorItems(){
    QSettings settings("SIASUN", "TEXT");
	settings.beginWriteArray("color_items");
	for(int i = 0; i != color_list.size(); ++i){
		settings.setArrayIndex(i);
		settings.setValue("color", color_list[i].color);
		settings.setValue("name", color_list[i].name);
	}
	settings.endArray();
    QLOG_INFO() << "ColorItems'size: " << color_list.size();
}
