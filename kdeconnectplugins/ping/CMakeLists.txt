qt_add_library(kdeconnect_ping SHARED
    pingplugin.h
    pingplugin.cpp
    plugin_ping_debug.h
    plugin_ping_debug.cpp
    kdeconnect_ping.json
)

target_link_libraries(kdeconnect_ping PRIVATE Qt::Core Qt::Network kdeconnectcore)
