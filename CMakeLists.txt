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

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Gui Sql LinguistTools)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(third_party/snoretoast)
add_subdirectory(third_party/singleapplication)
add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(notification)
add_subdirectory(kcodecs)
add_subdirectory(kcontacts)
add_subdirectory(icons)
add_subdirectory(plugins)

set(TRANSLATION_FILES
        ${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}_zh_CN.ts
    )

qt_add_translations(kdeconnect-app
    QM_FILES_OUTPUT_VARIABLE qm_files
    TS_FILES ${TRANSLATION_FILES}
    )

include(GNUInstallDirs)
install(FILES ${qm_files}
    DESTINATION "${CMAKE_INSTALL_BINDIR}/translations")

# copy WinSCP
install(DIRECTORY others/ DESTINATION ${CMAKE_INSTALL_BINDIR})

qt_generate_deploy_script(
    TARGET kdeconnect-app
    OUTPUT_SCRIPT deploy_script
    CONTENT "
qt_deploy_runtime_dependencies(
    EXECUTABLE \"$<TARGET_FILE:kdeconnect-app>\"
    ADDITIONAL_MODULES  \"$<TARGET_FILE:kdeconnect_mprisremote>\"
    NO_COMPILER_RUNTIME
    GENERATE_QT_CONF
    DEPLOY_TOOL_OPTIONS \"--no-opengl-sw\" \"--no-system-d3d-compiler\"
    VERBOSE
)")
install(SCRIPT ${deploy_script})
