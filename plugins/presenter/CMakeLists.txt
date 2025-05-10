qt_add_library(kdeconnect_presenter SHARED
    assets.qrc
    kdeconnect_presenter.json
    plugin_presenter_debug.h
    plugin_presenter_debug.cpp
    presenterplugin.h
    presenterplugin.cpp
)

target_link_libraries(kdeconnect_presenter PRIVATE Qt::Widgets Qt::Gui Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_presenter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
