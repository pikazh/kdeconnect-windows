qt_add_library(kdeconnect_sms SHARED
    kdeconnect_sms.json
    smsplugin.h
    smsplugin.cpp
    plugin_sms_debug.h
    plugin_sms_debug.cpp
  )

target_link_libraries(kdeconnect_sms PRIVATE Qt::Core Qt::Network kdeconnectcore)

include(GNUInstallDirs)
install(TARGETS kdeconnect_sms
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
