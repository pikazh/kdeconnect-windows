qt_add_library(kdeconnect_telephony SHARED
    telephonyplugin.h
    telephonyplugin.cpp
    kdeconnect_telephony.json
    plugin_telephony_debug.h
    plugin_telephony_debug.cpp
)

target_link_libraries(kdeconnect_telephony PRIVATE Qt::Gui Qt::Core Qt::Network kdeconnectcore notification)

include(GNUInstallDirs)
install(TARGETS kdeconnect_telephony
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
