cmake_minimum_required(VERSION 3.16)
project(CrosshairGUI)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS 
    Core
    Gui
    Widgets
REQUIRED)

add_executable(CrosshairGUI
    main.cpp
    MainWindow.h
    MainWindow.cpp
)

target_link_libraries(CrosshairGUI
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    user32
)

target_compile_options(CrosshairGUI PRIVATE /std:c++17)
