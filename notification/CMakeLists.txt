qt_add_library(notification SHARED
    notification_debug.h
    notification_debug.cpp
    notification.h
    notification.cpp
    notificationmanager.h
    notificationmanager.cpp

    notificationplugin.h
    notifybysnore.h
    notifybysnore.cpp
)

include(GenerateExportHeader)
generate_export_header(notification EXPORT_FILE_NAME notification_export.h)

target_include_directories(notification PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(notification PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(notification PRIVATE Qt::Core Qt::Gui Qt::Network SnoreToast::SnoreToastActions)

include(GNUInstallDirs)
install(TARGETS notification
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
