qt_generate_dbus_interface(${CMAKE_SOURCE_DIR}/core/daemon.h ${CMAKE_CURRENT_BINARY_DIR}/generated/daemoninterface.xml)
qt_generate_dbus_interface(${CMAKE_SOURCE_DIR}/core/device.h ${CMAKE_CURRENT_BINARY_DIR}/generated/deviceinterface.xml)

set(device_interface_SRCS)
qt_add_dbus_interface(device_interface_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/generated/deviceinterface.xml
    deviceinterface
)

set(daemon_interface_SRCS)
qt_add_dbus_interface(daemon_interface_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/generated/daemoninterface.xml
    daemoninterface
)

qt_add_library(kdeconnectinterfaces SHARED)

include(GenerateExportHeader)
generate_export_header(kdeconnectinterfaces EXPORT_FILE_NAME kdeconnectinterfaces_export.h)


set(kdeconnectinterfaces_qml_FILES
    qml/DBusProperty.qml
)

qt_add_resources(kdeconnectinterfaces "qml"
    PREFIX "/qml"
    FILES
        ${kdeconnectinterfaces_qml_FILES}
)

qt_add_qml_module(kdeconnectinterfaces
    URI "org.kde.kdeconnect"
    VERSION 1.0
    PLUGIN_TARGET kdeconnectinterfaces_impl
    SOURCES
    ${device_interface_SRCS}
    ${daemon_interface_SRCS}
    devicedbusinterface.h
    devicedbusinterface.cpp
    daemondbusinterface.h
    daemondbusinterface.cpp
    devicesmodel.h
    devicesmodel.cpp
    devicessortproxymodel.h
    devicessortproxymodel.cpp
    responsewaiter.h
    responsewaiter.cpp
    interfaces_debug.h
    interfaces_debug.cpp
    QML_FILES
        ${kdeconnectinterfaces_qml_FILES}
)

target_include_directories(kdeconnectinterfaces PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(kdeconnectinterfaces PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kdeconnectinterfaces
    PRIVATE Qt::Quick Qt::Core Qt::Qml Qt::Gui Qt::DBus)

include(GNUInstallDirs)
install(TARGETS kdeconnectinterfaces
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/kdeconnectinterfaces_impl.dll"
    "${CMAKE_CURRENT_BINARY_DIR}/qmldir"
    DESTINATION "${CMAKE_INSTALL_BINDIR}/org/kde/kdeconnect")
