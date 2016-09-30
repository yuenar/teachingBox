INCLUDEPATH += $$PWD
#DEFINES += QS_LOG_LINE_NUMBERS    # automatically writes the file and line for each log message
#DEFINES += QS_LOG_DISABLE         # logging code is replaced with a no-op
#DEFINES += QS_LOG_SEPARATE_THREAD # messages are queued and written from a separate thread
SOURCES += $$PWD/log/QsLogDest.cpp \
    $$PWD/log/QsLog.cpp \
    $$PWD/log/QsLogDestConsole.cpp \
    $$PWD/log/QsLogDestFile.cpp \
    $$PWD/log/QsLogDestFunctor.cpp

HEADERS += $$PWD/log/QsLogDest.h \
    $$PWD/log/QsLog.h \
    $$PWD/log/QsLogDestConsole.h \
    $$PWD/log/QsLogLevel.h \
    $$PWD/log/QsLogDestFile.h \
    $$PWD/log/QsLogDisableForThisFile.h \
    $$PWD/log/QsLogDestFunctor.h

OTHER_FILES += \
    $$PWD/log/QsLogChanges.txt \
    $$PWD/log/QsLogReadme.txt \
    $$PWD/log/LICENSE.txt
