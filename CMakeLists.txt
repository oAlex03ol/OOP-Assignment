set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.16)
project(HyperFS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

set(SOURCES
    main.cpp
    MainWindow.cpp
    CanvasComponents.cpp
)

set(HEADERS
    MainWindow.h
    FSNode.h
    VisualNode.h
    CanvasEdge.h
)

add_executable(HyperFS ${SOURCES} ${HEADERS})
target_link_libraries(HyperFS PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
)