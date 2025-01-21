qt_add_qml_module(kdeconnectdeclarativeplugin
    URI "org.kde.kdeconnect"
    VERSION 1.0
    PLUGIN_TARGET kdeconnectdeclarativeplugin_imp
    SOURCES
    devicesmodel.h
    devicesmodel.cpp
)

target_include_directories(kdeconnectdeclarativeplugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(kdeconnectdeclarativeplugin
    PRIVATE Qt::Quick Qt::Core Qt::Qml Qt::Gui Qt::DBus kdeconnectinterfaces)

include(GNUInstallDirs)
install(TARGETS kdeconnectdeclarativeplugin
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/kdeconnectdeclarativeplugin_imp.dll"
    "${CMAKE_CURRENT_BINARY_DIR}/qmldir"
    DESTINATION "${CMAKE_INSTALL_BINDIR}/org/kde/kdeconnect")
