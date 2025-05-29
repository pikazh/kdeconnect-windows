qt_add_library(kdeconnect_runcommand SHARED
    runcommandplugin.h
    runcommandplugin.cpp
    plugin_runcommand_debug.h
    plugin_runcommand_debug.cpp
    kdeconnect_runcommand.json
)

target_link_libraries(kdeconnect_runcommand PRIVATE Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_runcommand
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
