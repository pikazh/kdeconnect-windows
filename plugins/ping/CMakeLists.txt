qt_add_library(kdeconnect_ping SHARED
    pingplugin.h
    pingplugin.cpp
    plugin_ping_debug.h
    plugin_ping_debug.cpp
    kdeconnect_ping.json
)

target_link_libraries(kdeconnect_ping PRIVATE Qt::Gui Qt::Core Qt::Network kdeconnectcore notification)

include(GNUInstallDirs)
install(TARGETS kdeconnect_ping
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
