add_subdirectory(backends/lan)
#add_subdirectory(backends/loopback)

qt_add_library(kdeconnectcore SHARED
    ${backends_kdeconnect_SRCS}
    backends/linkprovider.cpp
    backends/linkprovider.h
    backends/devicelink.cpp
    backends/devicelink.h
    backends/pairinghandler.cpp
    backends/pairinghandler.h
    networkpackettypes.h
    networkpacket.h
    networkpacket.cpp
    daemon.h
    daemon.cpp
    core_debug.h
    core_debug.cpp
    deviceinfo.h
    device.h
    device.cpp
    pairstate.h
    kdeconnectconfig.h
    kdeconnectconfig.cpp
    kdeconnectpluginconfig.h
    kdeconnectpluginconfig.cpp
    kdeconnectplugin.h
    kdeconnectplugin.cpp
    sslhelper.h
    sslhelper.cpp
    dbushelper.h
    dbushelper.cpp

    #pluginloader.h
    #pluginloader.cpp
)

include(GenerateExportHeader)
generate_export_header(kdeconnectcore EXPORT_FILE_NAME kdeconnectcore_export.h)

#target_include_directories(kdeconnectcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(kdeconnectcore PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kdeconnectcore PRIVATE Qt::Core Qt::Network Qt::DBus OpenSSL::Crypto)

include(GNUInstallDirs)
install(TARGETS kdeconnectcore
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
