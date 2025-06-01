qt_add_library(kdeconnect_remotecommands SHARED
    remotecommandsplugin.h
    remotecommandsplugin.cpp
    kdeconnect_remotecommands.json
)

target_link_libraries(kdeconnect_remotecommands PRIVATE Qt::Gui Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_remotecommands
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
