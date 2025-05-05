qt_add_library(kdeconnect_sftp SHARED
    kdeconnect_sftp.json
    sftpplugin-win.h
    sftpplugin-win.cpp
    plugin_sftp_debug.h
    plugin_sftp_debug.cpp
)

target_link_libraries(kdeconnect_sftp PRIVATE Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_sftp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
