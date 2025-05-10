cmake_minimum_required(VERSION 3.16)

project(kdeconnect VERSION 1.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

# SingleApplication base class
set(QAPPLICATION_CLASS FreeStandingSingleApplication)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Gui LinguistTools)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(third_party/snoretoast)
add_subdirectory(third_party/singleapplication)
add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(notification)
add_subdirectory(icons)
add_subdirectory(plugins)
