cmake_minimum_required(VERSION 3.16)
project(HexCompare LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)




add_executable(HexCompare
    main.cpp
    src/ByteFormatter.cpp src/BinaryFile.cpp src/DiffEngine.cpp
    src/SearchEngine.cpp src/HexDiffModel.cpp src/HexDiffView.cpp
    src/FilePickerWidget.cpp src/RangeControlWidget.cpp src/SearchBar.cpp
    src/BookmarkManager.cpp src/BookmarkWidget.cpp
    src/SearchBookmarkConverter.cpp
    src/MainWindow.cpp
)

target_include_directories(HexCompare PRIVATE src)
target_link_libraries(HexCompare PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)