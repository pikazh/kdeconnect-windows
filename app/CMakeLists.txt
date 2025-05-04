qt_add_executable(kdeconnect-app
    main.cpp
    kdeconnect.rc
    kdeconnect.qrc
    application.h
    application.cpp
    devicemanager.h
    devicemanager.cpp
    app_debug.h
    app_debug.cpp

    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    ui/devicewindow.h
    ui/devicewindow.cpp
    ui/devicelistmodel.h
    ui/devicelistmodel.cpp
	
    ui/widgets/PageContainer.h
    ui/widgets/PageContainer_p.h
    ui/widgets/PageContainer.cpp
    ui/widgets/IconLabel.h
    ui/widgets/IconLabel.cpp
    ui/widgets/imagewidget.h
    ui/widgets/imagewidget.cpp
    
    ui/pages/devicepairpageprovider.h
    ui/pages/devicepairpageprovider.cpp
    ui/pages/pairpage.h
    ui/pages/pairpage.cpp
    ui/pages/pairpage.ui
    ui/pages/devicepluginpagesprovider.h
    ui/pages/devicepluginpagesprovider.cpp
    ui/pages/mprisremotepage.ui
    ui/pages/mprisremotepage.h
    ui/pages/mprisremotepage.cpp
    ui/pages/BasePage.h
    ui/pages/BasePageContainer.h
    ui/pages/BasePageProvider.h

    plugin/pluginwrapperbase.h
    plugin/pluginwrapperbase.cpp
    plugin/mprisremotepluginwrapper.h
    plugin/mprisremotepluginwrapper.cpp
    plugin/batterypluginwrapper.h
    plugin/batterypluginwrapper.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(kdeconnect-app PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appuntitled4
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(kdeconnect-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(kdeconnect-app
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    SingleApplication::SingleApplication
    kdeconnectcore
    notification
    icons
)

include(GNUInstallDirs)
install(TARGETS kdeconnect-app
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

