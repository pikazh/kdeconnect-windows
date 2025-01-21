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

qt_add_library(kdeconnectinterfaces STATIC
    ${device_interface_SRCS}
    ${daemon_interface_SRCS}
    devicedbusinterface.h
    devicedbusinterface.cpp
    daemondbusinterface.h
    daemondbusinterface.cpp
    interface_debug.h
    interface_debug.cpp
)

target_include_directories(kdeconnectinterfaces PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(kdeconnectinterfaces PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(kdeconnectinterfaces PRIVATE Qt::Core Qt::Gui Qt::DBus)

include(GNUInstallDirs)
install(TARGETS kdeconnectinterfaces
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
