qt_add_resources(kdeconnect_custom_icons_SRCS ${CMAKE_SOURCE_DIR}/icons/custom_icons.qrc)

qt_add_executable(kdeconnect-app
    main.cpp
    ${kdeconnect_custom_icons_SRCS}
)

set(kdeconnect_app_qml_FILES
    qml/Main.qml
    qml/Settings.qml
)

qt_add_qml_module(kdeconnect-app
    URI "org.kde.kdeconnect.app"
    VERSION 1.0
    QML_FILES
        ${kdeconnect_app_qml_FILES}

)

qt_add_resources(kdeconnect-app "qml"
    PREFIX "/qml"
    FILES
        ${kdeconnect_app_qml_FILES}
)

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
    PRIVATE
    Qt::Quick
    Qt::QuickWidgets
    Qt::QuickControls2
    Qt::Core
    Qt::DBus
    kdeconnectinterfaces
    kdeconnectcore
)

include(GNUInstallDirs)
install(TARGETS kdeconnect-app
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
 TARGET kdeconnect-app
 OUTPUT_SCRIPT deploy_script
 MACOS_BUNDLE_POST_BUILD
 NO_UNSUPPORTED_PLATFORM_ERROR
 DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)

install(SCRIPT ${deploy_script})
