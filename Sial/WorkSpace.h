/**
  *@brief       编辑器工作目录声明文件
  *@author      yuenar
  *@date        16-09-27
  *@version     0.11
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加日志系统
  *@copyright   siasun.co
  */
#ifndef WORKSPACE_H
#define WORKSPACE_H
#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>
#include "DirView.h"
#include "SyntaxHighlighter.h"
#include "WidgetHeader.h"
#include "CodeEdit.h"
#include "DataEdit.h"
#include "EditorWindow.h"
#include "FileManageWidget.h"
#include "unit.h"
#include "UI/SearchWidget.h"

class MainWindow;
class QComboBox;
/*
*  这个类处理工作部件之间的协作。
*  这将是主窗口的中央控件。
*/
class WorkSpace : public QWidget
{
    Q_OBJECT
    //Function
public:
    WorkSpace(CodeEditThemes &code_edit_theme, QWidget *parent = 0);
    bool Save(DataEdit *edit);
    bool SaveAs(DataEdit *edit);
    bool SaveFile(DataEdit *edit);
    bool SaveAll();
    DataEdit *currentEdit();
    void updateCurrentFile();
    bool fileExist(QString file_full_name);
signals:
    void signal_closeFile(QListWidgetItem *item);
    void currentEditorWindowChange(EditorWindow *current, EditorWindow *previous);
    void addCurrentFile(const QString &full_file_name);
public slots:
    void AddFile(QString file_path);
    void setCurrentEdit(QListWidgetItem *cur);
    bool closeFile(QListWidgetItem *item);
    void addEditorWindow();
    void setCurrentEditorWindow();
    void closeEditorWindow(QObject *obj);
    void setFontSize(QFont f);
    bool closeAllFile();

    void Cut(){
        if(cur_window_)cur_window_->code_edit_->cut();
    }
    void Copy(){
        if(cur_window_)cur_window_->code_edit_->copy();
    }
    void Paste(){
        if(cur_window_)cur_window_->code_edit_->paste();
    }
protected:
    bool saveModified(DataEdit *edit);
    void InitSyntaxHighlighter();
    void InitFilesWidget();
    void InitSearchWidget();
    //Data
public:
    FileManageWidget *file_manage_widget_;
    CodeEditThemes &code_edit_themes_;
    EditorWindow *cur_window_;
    QHBoxLayout *editor_layout_;
    QList<EditorWindow*> editor_queue_;
    DataEdit *deleteEdit;
};

#endif // WORKSPACE_H
