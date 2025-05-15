if(WIN32)
    set(kdeconnect_mprisremote_SRCS
        mprisremoteplayer.h
        mprisremoteplayer.cpp
        mprisremoteplugin.h
        mprisremoteplugin.cpp
        plugin_mprisremote_debug.h
        plugin_mprisremote_debug.cpp
        albumartdownloadtask.h
        albumartdownloadtask.cpp
        albumartmanager.h
        albumartmanager.cpp
        albumartdb.h
        albumartdb.cpp
        kdeconnect_mprisremote.json
    )
endif()

qt_add_library(kdeconnect_mprisremote SHARED ${kdeconnect_mprisremote_SRCS})

target_link_libraries(kdeconnect_mprisremote PRIVATE Qt::Core Qt::Network Qt::Sql kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_mprisremote
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
