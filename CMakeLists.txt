cmake_minimum_required(VERSION 3.16)
project(WendyAr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 Qt 的 MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找所需的 Qt6 模块
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

# 设置链接器堆栈大小（对应原 .pro 的 -Wl,--stack,8388608）
# MinGW 下有效，MSVC 会忽略此选项
add_link_options(-Wl,--stack,8388608)

# 将所有源文件、头文件、UI 文件、资源文件添加到可执行目标
qt_add_executable(WendyAr WIN32
    bullet.cpp
    button.cpp
    enemy.cpp
    main.cpp
    mainwindow.cpp
    difficultywindow.cpp
    money.cpp
    obstacle.cpp
    radish.cpp
    startwindow.cpp
    tower.cpp
    towerpit.cpp

    bullet.h
    button.h
    enemy.h
    mainwindow.h
    difficultywindow.h
    money.h
    obstacle.h
    radish.h
    startwindow.h
    tower.h
    towerpit.h

    mainwindow.ui
    resources.qrc
)

# 链接 Qt 模块
target_link_libraries(WendyAr PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)