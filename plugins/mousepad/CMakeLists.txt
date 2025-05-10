qt_add_library(kdeconnect_mousepad SHARED
    kdeconnect_mousepad.json
    abstractremoteinput.h
    mousepadplugin.h
    mousepadplugin.cpp
    plugin_mousepad_debug.h
    plugin_mousepad_debug.cpp
    windowsremoteinput.h
    windowsremoteinput.cpp
)

target_link_libraries(kdeconnect_mousepad PRIVATE Qt::Core Qt::Gui Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_mousepad
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
