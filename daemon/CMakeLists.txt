qt_add_executable(kdeconnectd
    kdeconnectd.cpp
    desktopdaemon.cpp
    desktopdaemon.h
)

set_target_properties(kdeconnectd PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.kdeconnectd
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(kdeconnectd
    PRIVATE Qt::Core Qt::Gui Qt::Widgets Qt::Network Qt::DBus OpenSSL::Crypto kdeconnectcore
)

include(GNUInstallDirs)
install(TARGETS kdeconnectd
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET kdeconnectd
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

