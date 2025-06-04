qt_add_library(kdeconnect_systemvolume SHARED
    kdeconnect_systemvolume.json
    PolicyConfig.h
    plugin_systemvolume_debug.h
    plugin_systemvolume_debug.cpp
    systemvolumeplugin-win.cpp
    systemvolumeplugin-win.h
  )

target_link_libraries(kdeconnect_systemvolume PRIVATE Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_systemvolume
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
 
