qt_add_library(kdeconnect_findmyphone SHARED
    findmyphoneplugin.h
    findmyphoneplugin.cpp
    kdeconnect_findmyphone.json
)

target_link_libraries(kdeconnect_findmyphone PRIVATE Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_findmyphone
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)


