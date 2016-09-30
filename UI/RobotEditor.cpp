/**
  *@brief       编辑器实现文件
  *@author      yuenar
  *@date        16-09-29
  *@version     0.13
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加SIAL语言保留字解析及界面修改
  *@note        @16-09-28 yuenar v0.12 增加保存编辑函数到作业文件功能
  *@note        @16-09-29 yuenar v0.13 增加回读机器人上次作业文件功能
  *@copyright   siasun.co
  */
#include "RobotEditor.h"
#include <QtWidgets>
#include <QXmlStreamWriter>
#include "Sial/WorkSpace.h"
#include "Sial/CodeEdit.h"
#include "TextStyleDialog.h"
#include <QMessageBox>
#include "Sial/HandleFileThread.h"
#include "unit.h"
#include <log/QsLog.h>
#pragma execution_character_set("utf-8")

RobotEditor::RobotEditor(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle("SIAL编辑器");
    readSettings();
    initFileTypes();
    rwCmdList.clear();
    work_space_ = new WorkSpace(code_edit_themes_, this);
    setCentralWidget(work_space_);
    connect(work_space_->file_manage_widget_->folder_widget_, &DirView::openFile, this, &RobotEditor::loadFile);

    InitMenu();
    statusBar();
    checkLastXml();//回读历史作业文件，没有则载入三个必需函数文件
    setUnifiedTitleAndToolBarOnMac(true);
}
RobotEditor::~RobotEditor(){
    work_space_->deleteLater();

    rf_separator_->deleteLater();
    _newFile->deleteLater();
    _openFile->deleteLater();
    save_->deleteLater();
    save_as_->deleteLater();
    save_all_->deleteLater();
    _exit->deleteLater();
    cut_->deleteLater();
    copy_->deleteLater();
    paste_->deleteLater();
    style_->deleteLater();
    _about->deleteLater();
    file_menu_->deleteLater();
    edit_menu_->deleteLater();
    setting_menu_->deleteLater();
    _status->deleteLater();

}
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
void
RobotEditor::InitMenu(){
    file_menu_ = menuBar()->addMenu(tr("&文件"));
    //New File
    _newFile = new QAction(tr("新建"), this);
    _newFile->setShortcut(QKeySequence::New);
    _newFile->setStatusTip(tr("新建一个文件"));
	connect(_newFile, &QAction::triggered, this, &RobotEditor::newFile);
    file_menu_->addAction(_newFile);

    //Open File
    _openFile = new QAction(tr("打开"), this);
    _openFile->setShortcuts(QKeySequence::Open);
    _openFile->setStatusTip(tr("打开一个已经存在的文件"));
	connect(_openFile, &QAction::triggered, this, &RobotEditor::open);
    file_menu_->addAction(_openFile);

    //Save
    save_ = new QAction(tr("保存"), this);
    save_->setShortcuts(QKeySequence::Save);
    save_->setStatusTip(tr("保存为一个文件"));
	connect(save_, &QAction::triggered, this, &RobotEditor::Save);
    file_menu_->addAction(save_);

    //Save As
    save_as_ = new QAction(tr("另存为"), this);
    save_as_->setShortcuts(QKeySequence::SaveAs);
    save_as_->setStatusTip(tr("保存为另一个文件"));
	connect(save_as_, &QAction::triggered, this, &RobotEditor::SaveAs);
    file_menu_->addAction(save_as_);

    //Save All
    save_all_ = new QAction(tr("全部保存"), this);
    save_all_->setStatusTip(tr("保存全部文件"));
	connect(save_all_, &QAction::triggered, this, &RobotEditor::SaveAll);
    file_menu_->addAction(save_all_);

    rf_separator_ = file_menu_->addSeparator();
    for(int i = 0; i != RECENT_FILE_COUNT; ++i){
        file_menu_->addAction(recent_files_[i] = new QAction(this));
        recent_files_[i]->setVisible(false);
        connect(recent_files_[i], &QAction::triggered, this, &RobotEditor::openRecentFile);
    }
    updateRecentFiles();

    file_menu_->addSeparator();

    //Exit
    _exit = new QAction(tr("退出"), this);
    _exit->setShortcut(QKeySequence::Quit);
    _exit->setStatusTip(tr("关闭编辑器"));
    connect(_exit, &QAction::triggered, this, &RobotEditor::closeWin);
    file_menu_->addAction(_exit);

    //Edit Menu
    edit_menu_ = menuBar()->addMenu(tr("&编辑"));
    //Cut
    cut_ = new QAction(tr("&剪切"), this);
    cut_->setShortcuts(QKeySequence::Cut);
    connect(cut_, &QAction::triggered, work_space_, &WorkSpace::Cut);
    edit_menu_->addAction(cut_);

    //Copy
    copy_ = new QAction(tr("复制"), this);
    copy_->setShortcuts(QKeySequence::Copy);
    connect(copy_, &QAction::triggered, work_space_, &WorkSpace::Copy);
    edit_menu_->addAction(copy_);

    //Paste
    paste_ = new QAction(tr("粘贴"), this);
    paste_->setShortcuts(QKeySequence::Paste);
    connect(paste_, &QAction::triggered, work_space_, &WorkSpace::Paste);
    edit_menu_->addAction(paste_);

    //Search
    QAction *search = new QAction(tr("搜索"), this);
    search->setShortcut(QKeySequence::Find);
    connect(search, &QAction::triggered,
            [this](){EditorWindow *window = this->work_space_->cur_window_;
        if(window) window->search_widget_->setVisible(true);});
    edit_menu_->addAction(search);

    setting_menu_ = menuBar()->addMenu(tr("&设置"));
    //Style Setting
    style_ = new QAction(QIcon(""), tr("主题"), this);
    style_->setStatusTip(tr("自定义文本"));
    setting_menu_->addAction(style_);
    connect(style_, &QAction::triggered, this, &RobotEditor::SetTextStyle);

    //About
    _about = new QAction(tr("&关于"), this);
    _about->setStatusTip("显示本软件版本相关信息");
	connect(_about, &QAction::triggered, this, &RobotEditor::about);
    setting_menu_->addAction(_about);

    //Set Available
    save_->setEnabled(false);
    save_as_->setEnabled(false);
    cut_->setEnabled(false);
    copy_->setEnabled(false);
    paste_->setEnabled(false);

    connect(work_space_->file_manage_widget_->work_file_widget_, &FileListWidget::currentItemChanged, this, &RobotEditor::SetSaveEnable);
    connect(work_space_->file_manage_widget_->work_file_widget_, &FileListWidget::currentItemChanged, this, &RobotEditor::SetSaveAsEnable);
    connect(work_space_, &WorkSpace::addCurrentFile, this, &RobotEditor::setCurrentFile);
    connect(work_space_, &WorkSpace::currentEditorWindowChange, this, &RobotEditor::SetCutEnable);
    connect(work_space_, &WorkSpace::currentEditorWindowChange, this, &RobotEditor::SetCopyEnable);
    connect(work_space_, &WorkSpace::currentEditorWindowChange, this, &RobotEditor::SetPasteEnable);
}

//Recent Files
//-----------------------------------------------------------------------------------------------
void
RobotEditor::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        loadFile(action->data().toString());
}
void
RobotEditor::checkLastXml()
{
    QString fileName = "lastTask.xml";
    QString taskName;
    QFile file(fileName);
    if (false==file.open(QFile::ReadOnly | QFile::Text))
    {
        QLOG_WARN()<<"没有回读历史作业文件！";
        loadFile("./main.rcf");
        loadFile("./start.rcf");
        loadFile("./stop.rcf");
        return;
    }
    else
    {
        QXmlStreamReader reader;
        reader.setDevice(&file);
        while (false==reader.atEnd())
        {
            reader.readNext();
            if(reader.isStartElement())
            {
                if(reader.attributes().hasAttribute("Name"))
                {
                    QStringRef Temp=reader.attributes().value("Name");
                    taskName=Temp.toString().simplified();
                    if(false==rwCmdList.contains(taskName))
                    {
                        rwCmdList.append(taskName);
                        QLOG_INFO()<<taskName<<"文件回读中...";
                    }else
                    {
                        QLOG_WARN()<<taskName<<"文件已回读过，可能作业有错误！";
                        break;
                    }
                }
                else if(reader.name()=="Statement")
                {
                    QFile cmdFile(taskName);
                    if (false==cmdFile.open(QFile::ReadWrite | QFile::Text |QIODevice::Truncate))
                    {//Truncate重写xml文件，以防有垃圾文件信息残留
                        QLOG_WARN()<<"没有读到文件！";
                        return;
                    }
                    else
                    {
                        QTextStream in(&cmdFile);
                        in<<reader.readElementText()<<endl;
                        in.flush();
                        cmdFile.close();
                    }
                }
                else
                {
                    QLOG_INFO()<<"机器人历史作业文件回读进行中...";
                }
            }
        }
        file.close();
        foreach (QString str, rwCmdList) {
            loadFile(str);
        }
    }
}

void
RobotEditor::createTask()
{//读入各作业列表文件写入Task.xml
    int ct=work_space_->file_manage_widget_->work_file_widget_->count();
    QString rcfName="";
    QString strHtml ="";

    if(ct>0)
    {//列表文件数目不为0开始读入
        QString fileName = "Task.xml";
        QFile file(fileName);
        if (false==file.open(QFile::ReadWrite | QFile::Text |QIODevice::Truncate))
        {//Truncate重写xml文件，以防有垃圾文件信息残留
            QLOG_WARN()<<"没有读到文件！";
            return;
        }
        else
        {
            QXmlStreamWriter writer(&file);
            writer.setAutoFormatting(true);
            writer.writeStartDocument();// 写文档头
            writer.writeStartElement("Task_List");
            for (int i = 0; i <ct; i++)
            {
                rcfName=work_space_->file_manage_widget_->work_file_widget_->item(i)->text();
                QFile rcfFile(rcfName);
                if(rcfFile.open(QFile::ReadOnly | QFile::Text)){
                    QTextStream in(&rcfFile);
                    //                    QTextCodec *codec=QTextCodec::codecForName("GBK");//防止中文乱码
                    //                    in.setCodec(codec);
                    strHtml = in.readAll();
                    if(strHtml.contains("//")||strHtml.contains("/*"))
                    {
                        QMessageBox::warning(this, tr("警告"),
                                             tr("在编辑的 %1中发现未去除的注释，\n请去除注释后保存该编辑文件重试！")
                                             .arg(rcfName));
                        continue;
                    }
                    else
                    {
                        writer.writeStartElement("Func");
                        writer.writeAttribute("Name", rcfName);
                        writer.writeTextElement("Statement",strHtml);
                        writer.writeEndElement();
                        QLOG_INFO()<<"本地文件："<<rcfName<<"写入作业文件完成！";
                        rcfFile.close();
                    }
                }else
                    QLOG_WARN()<<"当前编辑的文件："<<rcfName<<"未保存，请保存该编辑文件后重试！";
            }
            writer.writeEndDocument();
            statusBar()->showMessage(tr("作业文件创建完成，请检查内容!"), 20000);
            file.close();
        }
    }else
        QLOG_WARN() << "作业列表为空，请检查后重试！";
}

void
RobotEditor::setCurrentFile(const QString &full_file_name)
{
    QSettings settings("SIASUN", "TEXT");
    QStringList files = settings.value("recent_files").toStringList();
    files.removeAll(full_file_name);
    files.prepend(full_file_name);
    while (files.size() > RECENT_FILE_COUNT)
        files.removeLast();

    settings.setValue("recent_files", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        RobotEditor *mainWin = qobject_cast<RobotEditor *>(widget);
        if (mainWin)
            mainWin->updateRecentFiles();
    }
}

void
RobotEditor::updateRecentFiles(){
    QSettings settings("SIASUN", "TEXT");
    QStringList files = settings.value("recent_files").toStringList();

    int fileCount = qMin(files.size(), RECENT_FILE_COUNT);
    int i;
    for(i = 0; i < fileCount; ++i){
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recent_files_[i]->setText(text);
        recent_files_[i]->setData(files[i]);
        recent_files_[i]->setVisible(true);
    }
    for(; i < RECENT_FILE_COUNT; ++i)
        recent_files_[i]->setVisible(false);
    rf_separator_->setVisible(fileCount > 0);
}

//Read&Write Setting
//-----------------------------------------------------------------------------------------------
void
RobotEditor::readSettings(){
    //Position & Size
    QSettings settings("SIASUN", "TEXT");
    QPoint window_pos = settings.value("pos", QPoint(0, 0)).toPoint();
    QSize window_size = settings.value("size").toSize();
#ifdef _CLEAN_SETTINGS_
    window_size = QSize(0, 0);
#endif
    if(window_size.isEmpty())
        setWindowState(Qt::WindowMaximized);
    else
        resize(window_size);
    move(window_pos);

    //Application Theme
    QFile style_sheet(":/DarkTheme.qss");
    style_sheet.open(QIODevice::ReadOnly);
    setStyleSheet(style_sheet.readAll());
    style_sheet.close();

    //CodeEdit Theme
    int isize = settings.beginReadArray("code_edit_themes");
#ifdef _CLEAN_SETTINGS_
    isize = 0;
#endif
    if(isize > 0){
        code_edit_themes_.cur_num = settings.value("cur_num").toInt();
        QLOG_INFO() << "cur_num: " << code_edit_themes_.cur_num;
        for(int i = 0; i < isize; ++i){
            settings.setArrayIndex(i);
            TextTheme t;
            t.theme_name = settings.value("theme_name").toString();
            QFont f = t.formats[NORMAL_FORMAT].font();
            f.setPixelSize(settings.value("font_size").toInt());
            t.formats[NORMAL_FORMAT].setFont(f);
            t.formats[NORMAL_FORMAT].setFontFamily(settings.value("font_family").toString());
            int jsize = settings.beginReadArray("text_theme");
            for(int j = 0; j < jsize; ++j){
                settings.setArrayIndex(j);
                t.formats[j].setForeground(QBrush(settings.value("foreground_color").value<QColor>()));
                t.formats[j].setBackground(QBrush(settings.value("background_color").value<QColor>()));
                t.formats[j].setFontWeight(settings.value("bold").toBool() ? QFont::Bold : QFont::Normal);
                t.formats[j].setFontItalic(settings.value("italic").toBool());
                t.formats[j].setFontUnderline(settings.value("underline").toBool());
            }
            settings.endArray();
            code_edit_themes_.themes.append(t);
        }
    }
    else{//Predefine
        code_edit_themes_.cur_num = 0;
        TextTheme t;
        //Normal
        t.formats[NORMAL_FORMAT].setForeground(QColor("#1e1e1e"));
        t.formats[NORMAL_FORMAT].setBackground(QColor("#f0f0f0"));
        QFont f = t.formats[NORMAL_FORMAT].font();
        f.setPixelSize(PREDEFINED_FONT_SIZE);
        t.formats[NORMAL_FORMAT].setFont(f);
        t.formats[NORMAL_FORMAT].setFontFamily("Consolas");

        //Function
        t.formats[FUNCTION_FORMAT].setForeground(QColor("#aa55ff"));
        t.formats[FUNCTION_FORMAT].setBackground(QColor(0, 0, 0, 0));

        //Keywork
        t.formats[KEYWORD_FORMAT].setForeground(QColor("#0087cb"));
        t.formats[KEYWORD_FORMAT].setBackground(QColor(0, 0, 0, 0));
        t.formats[KEYWORD_FORMAT].setFontWeight(QFont::Bold);

        //Preprocessor
        t.formats[PREPROCESSOR_FORMAT].setForeground(QColor("#ff1493"));
        t.formats[PREPROCESSOR_FORMAT].setBackground(QColor("#ffb6c1"));

        //Comment
        t.formats[COMMENT_FORMAT].setForeground(Qt::darkGreen);
        t.formats[COMMENT_FORMAT].setBackground(QColor(0, 0, 0, 0));

        //Number
        t.formats[NUMBER_FORMAT].setForeground(QColor("#ff1493"));
        t.formats[NUMBER_FORMAT].setBackground(QColor(0, 0, 0, 0));

        //Symbol
        t.formats[SYMBOL_FORMAT].setForeground(Qt::darkYellow);
        t.formats[SYMBOL_FORMAT].setBackground(QColor(0, 0, 0, 0));

        t.theme_name = tr("经典");
        code_edit_themes_.themes.append(t);
        t.theme_name = tr("自定义");
        code_edit_themes_.themes.append(t);
    }
    settings.endArray();
}

void
RobotEditor::initFileTypes()
{
    FileType file_type;
    file_type.suffix = "txt";
    file_type.filter = "Text files (*.txt)";
    g_file_types << file_type;
    file_type.suffix = "rcf";
    file_type.filter = "Robot controller files (*.rcf)";
    g_file_types << file_type;
    g_file_filter = "All files (*.*);;";
    for(auto &i : g_file_types)
        g_file_filter += i.filter + ";;";
    g_file_filter.chop(2);
}
void
RobotEditor::writeSettings(){
    QSettings settings("SIASUN", "TEXT");
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    //Write Themes
    //	code_edit_themes_ = work_space_->code_edit_themes_;
    settings.beginWriteArray("code_edit_themes");
    settings.setValue("cur_num", code_edit_themes_.cur_num);
    QLOG_INFO() << "cur_num: " << code_edit_themes_.cur_num;
    for(int i = 0; i != code_edit_themes_.themes.size(); ++i){
        settings.setArrayIndex(i);
        settings.setValue("theme_name", code_edit_themes_.themes[i].theme_name);
        settings.setValue("font_size", code_edit_themes_.themes[i].formats[NORMAL_FORMAT].font().pixelSize());
        settings.setValue("font_family", code_edit_themes_.themes[i].formats[NORMAL_FORMAT].fontFamily());
        settings.beginWriteArray("text_theme");
        for(int j = 0; j != FORMAT_COUNT; ++j){
            settings.setArrayIndex(j);
            settings.setValue("foreground_color", code_edit_themes_.themes[i].formats[j].foreground().color());
            settings.setValue("background_color", code_edit_themes_.themes[i].formats[j].background().color());
            settings.setValue("bold", code_edit_themes_.themes[i].formats[j].fontWeight() == QFont::Bold);
            settings.setValue("italic", code_edit_themes_.themes[i].formats[j].fontItalic());
            settings.setValue("underline", code_edit_themes_.themes[i].formats[j].fontUnderline());
        }
        settings.endArray();
    }
    settings.endArray();
}

//Save-------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
void
RobotEditor::Save(){
    DataEdit *edit = work_space_->currentEdit();
    if(edit && work_space_->Save(edit))
        statusBar()->showMessage(tr("文件已保存"), 2000);
}

void
RobotEditor::SaveAs(){
    DataEdit *edit = work_space_->currentEdit();
    if(edit && work_space_->SaveAs(edit))
        statusBar()->showMessage(tr("文件已保存"), 2000);
}

void
RobotEditor::SaveAll(){
    int cnt = work_space_->file_manage_widget_->work_file_widget_->count();
    for(int i = 0; i != cnt; ++i){
        DataEdit *edit = work_space_->file_manage_widget_->work_file_widget_->item(i)->data(Qt::UserRole).value<DataEdit*>();
        if(edit->modified_)
            work_space_->Save(edit);
    }
}

//Open And Read
//-----------------------------------------------------------------------------------------------
void
RobotEditor::open(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), tr(""), g_file_filter);
    if(!fileName.isEmpty())
        loadFile(fileName);
}
void
RobotEditor::loadFile(const QString &file_path){
    if(work_space_->fileExist(file_path)) return;
    QFile *file = new QFile(file_path);
    if(!file->open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this, tr("出错了！"),
                             tr("无法读入文件： %1\n%2.")
                             .arg(file_path)
                             .arg(file->errorString()));
        return ;
    }
    work_space_->AddFile(file_path);
    DataEdit *edit = work_space_->currentEdit();
    HandleFileThread *thread = new HandleFileThread(file);
    connect(thread, &HandleFileThread::readData, edit, &DataEdit::readText, Qt::BlockingQueuedConnection);
    connect(thread, &HandleFileThread::readSuccess, edit, &DataEdit::checkEditModified, Qt::BlockingQueuedConnection);
    connect(edit, &DataEdit::stopReading, [=](){thread->stop = true;});
    connect(thread, &HandleFileThread::readSuccess, this, &RobotEditor::loadFileSuccess);
    connect(thread, &HandleFileThread::finished, thread, &HandleFileThread::deleteLater);
    thread->start();
    setCurrentFile(file_path);
    QLOG_INFO() << "成功载入文件 " << file_path;
}

//New File
//----------------------------------------------------------------------------------------------------------------------------
void
RobotEditor::newFile(){
    work_space_->AddFile("");
}

void
RobotEditor::closeEvent(QCloseEvent *event){
    if(!work_space_->closeAllFile()){
        event->accept();
        return;
    }
    writeSettings();
    closeWin();
}
void
RobotEditor::closeWin()
{
    emit ChangeEditState();
    this->close();
}

//
void
RobotEditor::about(){
    QMessageBox::about(this, tr("关于"), tr("<b>SIASUN</b>机器语言编辑器"));
}


//
void
RobotEditor::SetSaveEnable(QListWidgetItem *cur, QListWidgetItem *){
    save_->setEnabled(cur != NULL);
}
void
RobotEditor::SetSaveAsEnable(QListWidgetItem *cur, QListWidgetItem *){
    save_as_->setEnabled(cur != NULL);
}
void
RobotEditor::SetCutEnable(EditorWindow *cur, EditorWindow *){
    cut_->setEnabled(cur && !cur->code_edit_->isReadOnly());
}
void RobotEditor::SetCopyEnable(EditorWindow *cur, EditorWindow *){
    copy_->setEnabled(cur && !cur->code_edit_->isReadOnly());
}
void
RobotEditor::SetPasteEnable(EditorWindow *cur, EditorWindow *){
    paste_->setEnabled(cur && !cur->code_edit_->isReadOnly());
}
void RobotEditor::SetTextStyle(){
    TextStyleDialog *text_style_dialog = new TextStyleDialog(code_edit_themes_, work_space_, this);
    text_style_dialog->setAttribute(Qt::WA_DeleteOnClose);
    text_style_dialog->exec();
}
void
RobotEditor::loadFileSuccess()
{
    statusBar()->showMessage(tr("读取文件成功!"), 2000);
}
