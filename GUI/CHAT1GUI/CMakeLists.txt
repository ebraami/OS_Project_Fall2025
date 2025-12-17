cmake_minimum_required(VERSION 3.16)

project(Chat1ClientGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(Chat1ClientGUI
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    chatclient.cpp
    ChatClient.h
)

target_link_libraries(Chat1ClientGUI
    PRIVATE Qt6::Widgets
    ws2_32
)

set_target_properties(Chat1ClientGUI PROPERTIES
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(Chat1ClientGUI)
