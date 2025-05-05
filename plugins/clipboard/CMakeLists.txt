qt_add_library(kdeconnect_clipboard SHARED
    kdeconnect_clipboard.json
    clipboardlistener.h
    clipboardlistener.cpp
    clipboardplugin.h
    clipboardplugin.cpp
    plugin_clipboard_debug.h
    plugin_clipboard_debug.cpp
)

target_link_libraries(kdeconnect_clipboard PRIVATE Qt::Gui Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_clipboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
