cmake_minimum_required(VERSION 3.16)

project(KdeConnect VERSION 0.1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Qml Core Quick Network Gui Widgets DBus)
find_package(OpenSSL COMPONENTS Crypto)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(kdeconnect-app
    main.cpp
    Main.qml
)

include_directories(${PROJECT_SOURCE_DIR})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(kdeconnect-app PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appuntitled4
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(kdeconnect-app
    PRIVATE Qt6::Quick Qt6::Core Qt6::DBus kdeconnectdeclarativeplugin
)

add_subdirectory(core)
add_subdirectory(declarativeplugin/org/kde/kdeconnect)
add_subdirectory(daemon)
add_subdirectory(interfaces)
add_subdirectory(kdeconnectplugins)

qt_generate_deploy_qml_app_script(
 TARGET kdeconnect-app
 OUTPUT_SCRIPT deploy_script
 MACOS_BUNDLE_POST_BUILD
 NO_UNSUPPORTED_PLATFORM_ERROR
 DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)

install(SCRIPT ${deploy_script})



