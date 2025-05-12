qt_add_library(kdeconnect_remotecontrol SHARED
    remotecontrolplugin.h
    remotecontrolplugin.cpp
    kdeconnect_remotecontrol.json
)

target_link_libraries(kdeconnect_remotecontrol PRIVATE Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_remotecontrol
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
