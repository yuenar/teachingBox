/**
  *@brief       编辑器工作目录实现文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#include "WorkSpace.h"
#include <QtWidgets>
#include <QDebug>
#include <QFileSystemModel>
#include "UI/RobotEditor.h"
#include "NoFocusFrameDelegate.h"
#include "SyntaxHighlighter.h"
#include "FileManageWidget.h"
#include "EditorWindow.h"
#include "DataEdit.h"
#include <log/QsLog.h>


WorkSpace::WorkSpace(CodeEditThemes &code_edit_theme, QWidget *parent):
	QWidget(parent),
	file_manage_widget_(new FileManageWidget(this)),
	code_edit_themes_(code_edit_theme),
	cur_window_(NULL),
	editor_layout_(new QHBoxLayout)
{	
	InitFilesWidget();

	//Init Whole Layout
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(file_manage_widget_, 0, Qt::AlignLeft);
	layout->addLayout(editor_layout_, 20);
	setLayout(layout);
}

//QFilesWidget
//----------------------------------------------------------------------------------------------------------------------------
void
WorkSpace::InitFilesWidget(){
	connect(file_manage_widget_->work_file_widget_, &FileListWidget::currentFile,
			this, &WorkSpace::setCurrentEdit);
	connect(file_manage_widget_->work_file_widget_, &FileListWidget::tryCloseFile, this, &WorkSpace::closeFile);
	connect(this, &WorkSpace::signal_closeFile, file_manage_widget_, &FileManageWidget::removeWorkFile);
	connect(file_manage_widget_, &FileManageWidget::cleanWorkingFiles, this, &WorkSpace::closeAllFile);
}


//AddFile
//----------------------------------------------------------------------------------------------------------------------------
void
WorkSpace::AddFile(QString file_path){
	//Add CodeEdit
	DataEdit* edit = new DataEdit(code_edit_themes_.themes[code_edit_themes_.cur_num],
            QFileInfo(file_path).suffix().compare("rcf", Qt::CaseInsensitive) == 0 ? SIAL : PLAIN, this);
	edit->file_path_ = file_path;
	file_manage_widget_->addWorkFile(file_path, edit);//Register file
	//Connect
	connect(edit, &QPlainTextEdit::modificationChanged, [edit](bool changed){edit->modified_ = changed;});
}

//Close File
//----------------------------------------------------------------------------------------------------------------------------
bool
WorkSpace::closeFile(QListWidgetItem *item){
	DataEdit *edit = item->data(Qt::UserRole).value<DataEdit*>();
	if(edit->modified_ && !saveModified(edit)) return false;//Ask whether close
	//To Close
	edit->stopLoading();//To stop loading(if loading)
	for(auto it = editor_queue_.begin(); it != editor_queue_.end(); ++it)
		if((*it)->code_edit_->data_edit_ == edit){
			(*it)->close();
		}
	emit signal_closeFile(item);//Remove edit
	return true;
}

//EditorWindow
//----------------------------------------------------------------------------------------------------------------------------
void
WorkSpace::addEditorWindow()
{
	if(editor_queue_.size() >= 3) return ;
	EditorWindow *window = new EditorWindow(currentEdit(), this);
	window->setAttribute(Qt::WA_DeleteOnClose);
	editor_layout_->addWidget(window);
	editor_queue_.append(window);
	connect(window, &EditorWindow::updateFontSize, this, &WorkSpace::setFontSize);
	connect(window, &EditorWindow::currentWindow, this, &WorkSpace::setCurrentEditorWindow);
	connect(window, &EditorWindow::destroyed, this, &WorkSpace::closeEditorWindow);
	connect(window, &EditorWindow::reAddWindow, [this](EditorWindow *window){this->editor_layout_->addWidget(window);});
	emit currentEditorWindowChange(window, cur_window_);
	cur_window_ = window;
}

void
WorkSpace::setCurrentEditorWindow()
{
	/*Whenever the EditorWindow was pressed, this function would be call to update the cur_window.
	 * It alse will locate the currrent item-DataEidt.
	 * Called by:
	 *	EditorWindow::currentWindow
	*/
	EditorWindow *window = static_cast<EditorWindow*>(sender());
	if(window != cur_window_) emit currentEditorWindowChange(window, cur_window_);
	cur_window_ = window;
	updateCurrentFile();
}

void
WorkSpace::closeEditorWindow(QObject *obj)
{
#ifdef _MY_DEBUG_
    QLOG_INFO() << "Destory window: " << obj;
#endif
	Q_ASSERT(editor_queue_.removeOne(static_cast<EditorWindow*>(obj)));
	if(cur_window_ == obj){
		cur_window_ = NULL;//Always set to NULL
		emit currentEditorWindowChange(cur_window_, static_cast<EditorWindow*>(obj));
	}
}

//Save
//----------------------------------------------------------------------------------------------------------------------------
bool
WorkSpace::Save(DataEdit *edit){
	if(edit->file_path_.isEmpty()){
		return SaveAs(edit);
    }
	else
		return SaveFile(edit);
}
bool
WorkSpace::SaveAs(DataEdit *edit){
    QString file_path = QFileDialog::getSaveFileName(this, tr("Save File"), tr(""),
								tr("All files (*.*);;"
								   "Text files (*.txt);;"
                                   "Robot controller files (*.rcf)"));
	if(file_path.isEmpty()) return false;
	edit->setFileInfo(file_path);
	if(SaveFile(edit)){
		emit addCurrentFile(file_path);
		return true;
	}
	return false;
}
bool
WorkSpace::SaveFile(DataEdit *edit){
	QFile file(edit->file_path_);
	if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, tr("糟糕了！"),
                             tr("无法写入文件 %1:\n%2.")
							 .arg(edit->file_path_)
							 .arg(file.errorString()));
		return false;
	}
	QTextStream out(&file);
	#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
	#endif
	out << edit->toPlainText();
	#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
	#endif
	edit->document()->setModified(edit->modified_ = false);
	return true;
}

bool
WorkSpace::saveModified(DataEdit *edit){
	QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("文件:%1中信息已修改").arg(edit->file_name_),
        "你希望保存修改的内容吗?",
		QMessageBox::Save |
		QMessageBox::Discard |
		QMessageBox::Cancel);
	if(ret == QMessageBox::Save)
		return Save(edit);
	else if(ret == QMessageBox::Cancel)
		return false;
	return true;
}

void
WorkSpace::setFontSize(QFont f)
{
	code_edit_themes_.CurrentTheme().formats[NORMAL_FORMAT].setFont(f);
    QLOG_INFO() << "In WorkSpace the font size is: " << code_edit_themes_.CurrentTheme().formats[NORMAL_FORMAT].font().pixelSize();
	FileListWidget *list = file_manage_widget_->work_file_widget_;
	for(int i = 0; i != list->count(); ++i){
		list->item(i)->data(Qt::UserRole).value<DataEdit*>()->normal_format_ = code_edit_themes_.CurrentTheme().formats[NORMAL_FORMAT];
	}
	for(auto i : editor_queue_)
		i->updateFormat();
}

bool
WorkSpace::closeAllFile()
{
	FileListWidget *list = file_manage_widget_->work_file_widget_;
	while(list->count())
		if(!closeFile(list->item(0))) return false;
	return true;
}

//Current Info
//----------------------------------------------------------------------------------------------------------------------------
void
WorkSpace::setCurrentEdit(QListWidgetItem *cur){
	/*Whenever the item was pressed or to add a new file, this function would be call to set the newest edit to the current EditorWindow.
	 * Called by:
	 *	FileManageWidget::addWorkFile
	 *	FileListWidget::currentFile
	*/
	DataEdit *cur_edit = cur->data(Qt::UserRole).value<DataEdit*>();
	if(cur_window_)//Normal condition
		cur_window_->setEdit(cur_edit);//Change the current edit
	else//Add file condition
		addEditorWindow();
}

DataEdit *WorkSpace::currentEdit()
{
	QListWidgetItem *item = file_manage_widget_->currentItem();
	return item ? item->data(Qt::UserRole).value<DataEdit*>() : NULL;
}

void
WorkSpace::updateCurrentFile()
{
	FileListWidget *list = file_manage_widget_->work_file_widget_;
	int i;
	for(i = 0; i != list->count(); ++i){
		DataEdit *edit = list->item(i)->data(Qt::UserRole).value<DataEdit*>();
		if(edit == cur_window_->code_edit_->data_edit_){//Request cur_window != NULL !!!
			list->setCurrentRow(i);
			break;
		}
	}
	Q_ASSERT(i != list->count());
}

bool
WorkSpace::fileExist(QString file_full_name)
{
	FileListWidget *list = file_manage_widget_->work_file_widget_;
	for(int i = 0; i < list->count(); ++i)
		if(list->item(i)->data(Qt::UserRole).value<DataEdit*>()->file_path_ == file_full_name) return true;
	return false;
}
