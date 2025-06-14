qt_add_library(kdeconnectcore SHARED
    networkpackettypes.h
    networkpacket.h
    networkpacket.cpp
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
    imageutil.h
    imageutil.cpp
    krandom.h
    krandom.cpp
    closure.h
    atexitmanager.h

    backends/lan/server.cpp
    backends/lan/server.h
    backends/lan/lanlinkprovider.cpp
    backends/lan/lanlinkprovider.h
    backends/lan/landevicelink.cpp
    backends/lan/landevicelink.h
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

    plugins/share/itransferhistorymanager.h
    plugins/share/transferhistoryrecord.h
    plugins/share/itransferhistorychangelistener.h

    task/task.h
    task/task.cpp
    task/taskstatus.h
    task/netaction.h
    task/netaction.cpp
    task/taskstatus.cpp
    task/taskscheduler.h
    task/taskscheduler.cpp
    task/peersslsockettask.h
    task/peersslsockettask.cpp
    task/peerbufferdownloadtask.h
    task/peerbufferdownloadtask.cpp
    task/peerfiledownloadtask.h
    task/peerfiledownloadtask.cpp
    task/peerfileuploadtask.h
    task/peerfileuploadtask.cpp

    sms/conversationmessage.h
    sms/conversationmessage.cpp

    locale/mapentry_p.h
    locale/isocodes_p.h
    locale/isocodescache_p.h
    locale/isocodescache.qrc
    locale/isocodescache.cpp
    locale/isocodes.cpp
    locale/kcountry.h
    locale/kcountry.cpp
)

include(GenerateExportHeader)
generate_export_header(kdeconnectcore EXPORT_FILE_NAME kdeconnectcore_export.h)

target_include_directories(kdeconnectcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(kdeconnectcore PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kdeconnectcore PRIVATE Qt::Core Qt::Gui Qt::Network OpenSSL::Crypto)

include(GNUInstallDirs)
install(TARGETS kdeconnectcore
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
