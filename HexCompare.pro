QT       += core gui widgets
CONFIG   += c++17
TEMPLATE  = app
TARGET    = HexCompare

INCLUDEPATH += src

SOURCES += \
    main.cpp \
    src/ByteFormatter.cpp \
    src/BinaryFile.cpp \
    src/DiffEngine.cpp \
    src/SearchEngine.cpp \
    src/HexDiffModel.cpp \
    src/HexDiffView.cpp \
    src/FilePickerWidget.cpp \
    src/RangeControlWidget.cpp \
    src/SearchBar.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/HexColors.h \
    src/ByteFormatter.h \
    src/BinaryFile.h \
    src/DiffEngine.h \
    src/SearchEngine.h \
    src/HexDiffModel.h \
    src/HexDiffView.h \
    src/FilePickerWidget.h \
    src/RangeControlWidget.h \
    src/SearchBar.h \
    src/MainWindow.h