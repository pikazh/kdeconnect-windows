qt_add_library(kdeconnect_remotekeyboard SHARED
    plugin_remotekeyboard_debug.h
    plugin_remotekeyboard_debug.cpp
    remotekeyboardplugin.h
    remotekeyboardplugin.cpp
    kdeconnect_remotekeyboard.json
)

target_link_libraries(kdeconnect_remotekeyboard PRIVATE Qt::Core Qt::Gui Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_remotekeyboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
