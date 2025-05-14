qt_add_library(kdeconnect_battery SHARED
    kdeconnect_battery.json
    batteryplugin-win.h
    batteryplugin-win.cpp
    plugin_battery_debug.h
    plugin_battery_debug.cpp
    batterymonitor.h
    batterymonitor.cpp
)

target_link_libraries(kdeconnect_battery PRIVATE Qt::Gui Qt::Core Qt::Network kdeconnectcore notification)

include(GNUInstallDirs)
install(TARGETS kdeconnect_battery
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
