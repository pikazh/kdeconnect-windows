qt_add_library(kdeconnectcore SHARED
    backends/lan/server.cpp
    backends/lan/server.h
    backends/lan/lanlinkprovider.cpp
    backends/lan/lanlinkprovider.h
    backends/lan/landevicelink.cpp
    backends/lan/landevicelink.h
    #    backends/lan/compositeuploadjob.cpp
    #	backends/lan/compositeuploadjob.h
    #   backends/lan/uploadjob.cpp
    #	backends/lan/uploadjob.h
    backends/lan/mdnsdiscovery.cpp
    backends/lan/mdnsdiscovery.h
    backends/lan/mdns_wrapper.cpp
    backends/lan/mdns_wrapper.h
    backends/linkprovider.cpp
    backends/linkprovider.h
    backends/devicelink.cpp
    backends/devicelink.h
    backends/pairinghandler.cpp
    backends/pairinghandler.h
    networkpackettypes.h
    networkpacket.h
    networkpacket.cpp
    #daemon.h
    #daemon.cpp
    core_debug.h
    core_debug.cpp
    deviceinfo.h
    device.h
    device.cpp
    kdeconnectconfig.h
    kdeconnectconfig.cpp
    jsonutils.h
    jsonutils.cpp
    sslhelper.h
    sslhelper.cpp
    plugins/pluginloader.h
    plugins/pluginloader.cpp
    plugins/kdeconnectpluginconfig.h
    plugins/kdeconnectpluginconfig.cpp
    plugins/kdeconnectplugin.h
    plugins/kdeconnectplugin.cpp
    plugins/pluginid.h
    plugins/pluginid.cpp
    plugins/pluginfactory.h
    plugins/pluginfactory.cpp
    plugins/pluginmetadata.h
    plugins/pluginmetadata.cpp
    task/task.h
    task/task.cpp
    task/netaction.h
    task/netaction.cpp
    task/taskscheduler.h
    task/taskscheduler.cpp
    task/sockettask.h
    task/sockettask.cpp
)

include(GenerateExportHeader)
generate_export_header(kdeconnectcore EXPORT_FILE_NAME kdeconnectcore_export.h)

target_include_directories(kdeconnectcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(kdeconnectcore PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kdeconnectcore PRIVATE Qt::Core Qt::Network OpenSSL::Crypto)

include(GNUInstallDirs)
install(TARGETS kdeconnectcore
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
