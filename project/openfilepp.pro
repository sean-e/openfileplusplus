QT += widgets

HEADERS       = AppWindow.h \
                DarkModeStyle.h \
                Model.h \
                DirectoryExclusionsEditor.h \
                SourceListEditorDlg.h
SOURCES       = main.cpp \
                AppWindow.cpp \
                DarkModeStyle.cpp \
                Model.cpp \
                DirectoryExclusionsEditor.cpp \
                SourceListEditorDlg.cpp
RESOURCES     = OpenFile.qrc

# Q_OS_WIN
LIBS += -lUser32
LIBS += -lOle32

# install
target.path = .
INSTALLS += target
