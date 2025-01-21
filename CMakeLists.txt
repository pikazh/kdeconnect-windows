cmake_minimum_required(VERSION 3.16)

project(kdeconnect VERSION 0.1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Qml Core Quick QuickWidgets QuickControls2 Network Gui Widgets DBus)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

qt_standard_project_setup(REQUIRES 6.5)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(declarativeplugin/org/kde/kdeconnect)
add_subdirectory(daemon)
add_subdirectory(interfaces)
add_subdirectory(kdeconnectplugins)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/dbus/"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PATTERN "${PROJECT_SOURCE_DIR}/dbus/*")
