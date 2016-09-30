/**
  *@brief       编辑器声明文件
  *@author      yuenar
  *@date        16-09-29
  *@version     0.13
  *@note        @16-09-26 yuenar v0.10 初版，重写编辑器，开源修改
  *@note        @16-09-27 yuenar v0.11 增加SIAL语言保留字解析及界面修改
  *@note        @16-09-28 yuenar v0.12 增加保存编辑函数到作业文件功能
  *@note        @16-09-29 yuenar v0.13 增加回读机器人上次作业文件功能
  *@copyright   siasun.co
  */
#ifndef ROBOTEDITOR_H
#define ROBOTEDITOR_H

#define RECENT_FILE_COUNT 5

#include <QMainWindow>
#include "unit.h"

class CodeEdit;
class WorkSpace;
class EditorWindow;
class TextStyleDialog;
class QListWidgetItem;

class RobotEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit RobotEditor(QWidget *parent = 0);
    ~RobotEditor();
public slots:
    void setCurrentFile(const QString &full_file_name);
    void loadFileSuccess();//载入文件成功
    void createTask();//创建/重写作业文件task.xml
    void checkLastXml();//读取回读的作业文件
protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
signals:
    void ChangeEditState();//编辑器是否打开状态
private:
    bool SaveChangedFiles();
    void textChanged();
    void writeSettings();
    void readSettings();
    void initFileTypes();

    //文件操作函数
    void newFile();
    void open();
    void loadFile(const QString&file_path);
    void Save();
    void SaveAs();
    void SaveAll();
    void about();
    //初始化函数
    void InitMenu();
    void createMenu();
    void createStatus();

private slots:
    void closeWin();
    void openRecentFile();
    void updateRecentFiles();
    void SetTextStyle();
    void SetSaveEnable(QListWidgetItem *cur, QListWidgetItem *);
    void SetSaveAsEnable(QListWidgetItem *cur, QListWidgetItem *);
    void SetCutEnable(EditorWindow *cur, EditorWindow *);
    void SetCopyEnable(EditorWindow *cur, EditorWindow *);
    void SetPasteEnable(EditorWindow *cur, EditorWindow *);
public:
    CodeEditThemes code_edit_themes_;
    QAction* recent_files_[RECENT_FILE_COUNT];
private:
    WorkSpace *work_space_;
    QStringList rwCmdList;//回读作业文件名称列表
    QAction *rf_separator_;
    QAction *_newFile;
    QAction *_openFile;
    QAction *save_;
    QAction *save_as_;
    QAction *save_all_;
    QAction *_exit;
    QAction *cut_;
    QAction *copy_;
    QAction *paste_;
    QAction *style_;
    QAction *_about;
    QMenu *file_menu_;
    QMenu *edit_menu_;
    QMenu *setting_menu_;
    QStatusBar *_status;
};

#endif // MAINWINDOW_H
