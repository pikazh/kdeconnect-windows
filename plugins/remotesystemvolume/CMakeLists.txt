qt_add_library(kdeconnect_remotesystemvolume SHARED
    remotesystemvolumeplugin.h
    remotesystemvolumeplugin.cpp
    kdeconnect_remotesystemvolume.json
)

target_link_libraries(kdeconnect_remotesystemvolume PRIVATE Qt::Core Qt::Gui Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_remotesystemvolume
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
