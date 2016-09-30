#-------------------------------------------------
#
# Project created by QtCreator 2016-07-14T08:34:36
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TeachingBox
TEMPLATE = app
CONFIG += c++11
include(QsLog.pri)

SOURCES += main.cpp\
    Sial/CodeEdit.cpp \
    Sial/DataEdit.cpp \
    Sial/DirView.cpp \
    Sial/EditorWindow.cpp \
    Sial/FileListWidget.cpp \
    Sial/FileManageWidget.cpp \
    Sial/HandleFileThread.cpp \
    Sial/NoFocusFrameDelegate.cpp \
    Sial/SyntaxHighlighter.cpp \
    Sial/WidgetHeader.cpp \
    Sial/WorkSpace.cpp \
    UI/LoginDlg.cpp \
    UI/MainWindow.cpp \
    UI/RobotEditor.cpp \
    UI/SearchWidget.cpp \
    UI/SysInfoBox.cpp \
    UI/TeachingBox.cpp \
    UI/TextStyleDialog.cpp \
    ReciThread.cpp \
    SendThread.cpp

HEADERS  += \
    Sial/CodeEdit.h \
    Sial/DataEdit.h \
    Sial/DirView.h \
    Sial/EditorWindow.h \
    Sial/FileListWidget.h \
    Sial/FileManageWidget.h \
    Sial/HandleFileThread.h \
    Sial/NoFocusFrameDelegate.h \
    Sial/SyntaxHighlighter.h \
    Sial/WidgetHeader.h \
    Sial/WorkSpace.h \
    UI/LoginDlg.h \
    UI/MainWindow.h \
    UI/RobotEditor.h \
    UI/SearchWidget.h \
    UI/SysInfoBox.h \
    UI/TeachingBox.h \
    UI/TextStyleDialog.h \
    ReciThread.h \
    SendThread.h \
    unit.h

RESOURCES += \
    Sial/editor.qrc

FORMS += \
    UI/LoginDlg.ui \
    UI/MainWindow.ui \
    UI/SysInfoBox.ui \
    UI/TeachingBox.ui \ 
    UI/RobotEditor.ui \
    UI/SearchWidget.ui \
    UI/TextStyleDialog.ui
