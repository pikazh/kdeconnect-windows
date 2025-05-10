
qt_add_library(kdeconnect_mpriscontrol SHARED
    kdeconnect_mpriscontrol.json
    mpriscontrolplugin-win.h
    mpriscontrolplugin-win.cpp
    plugin_mpriscontrol_debug.h
    plugin_mpriscontrol_debug.cpp
    mediaplaybackcontroller.h
    mediaplaybackcontroller.cpp
  )

target_link_libraries(kdeconnect_mpriscontrol PRIVATE Qt::Core Qt::Network kdeconnectcore windowsapp)

include(GNUInstallDirs)
install(TARGETS kdeconnect_mpriscontrol
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
