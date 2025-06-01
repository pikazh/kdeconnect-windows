qt_add_library(kdeconnect_share SHARED
    kdeconnect_share.json
    shareplugin.h
    shareplugin.cpp
    plugin_share_debug.h
    plugin_share_debug.cpp
)

target_link_libraries(kdeconnect_share PRIVATE Qt::Gui Qt::Core Qt::Network kdeconnectcore notification)

include(GNUInstallDirs)
install(TARGETS kdeconnect_share
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
