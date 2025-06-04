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
    devicepairnotify.h
    devicepairnotify.cpp
    contactprovider.h
    contactprovider.cpp
    smshelper.h
    smshelper.cpp
    uicommon.h
    smsmanager.h
    smsmanager.cpp
    language.h
    language.cpp
    runcommandsetuplistener.h
    runcommandsetuplistener.cpp

    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    ui/devicewindow.h
    ui/devicewindow.cpp
    ui/devicelistmodel.h
    ui/devicelistmodel.cpp
    ui/smswindow.ui
    ui/smswindow.h
    ui/smswindow.cpp
    ui/smsconversationlistitemdelegate.h
    ui/smsconversationlistitemdelegate.cpp
    ui/smswidget.ui
    ui/smsconversationswidget.h
    ui/smsconversationswidget.cpp
    ui/smsconversationlistitemdelegate.h
    ui/smsconversationlistitemdelegate.cpp
    ui/smscontentitem.ui
    ui/smslistitemsortfilterproxymodel.h
    ui/smslistitemsortfilterproxymodel.cpp
    ui/smsconversationcontentwidget.ui
    ui/smsconversationcontentwidget.h
    ui/smsconversationcontentwidget.cpp
    ui/smscontentitem.h
    ui/smscontentitem.cpp
    ui/smstextbrowser.h
    ui/smstextbrowser.cpp
    ui/smsattachmentlistwidget.h
    ui/smsattachmentlistwidget.cpp
    ui/textlayoututils.h
    ui/textlayoututils.cpp

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
    ui/pages/remoteinputpage.h
    ui/pages/remoteinputpage.cpp
    ui/pages/remoteinputpage.ui
    ui/pages/devicepluginpagesprovider.h
    ui/pages/devicepluginpagesprovider.cpp
    ui/pages/mprisremotepage.ui
    ui/pages/mprisremotepage.h
    ui/pages/mprisremotepage.cpp
    ui/pages/remotecommandspage.ui
    ui/pages/remotecommandspage.h
    ui/pages/remotecommandspage.cpp
    ui/pages/remotecommandslistmodel.h
    ui/pages/remotecommandslistmodel.cpp
    ui/pages/BasePage.h
    ui/pages/BasePageContainer.h
    ui/pages/BasePageProvider.h
    ui/pages/remotecommandslistitemdelegate.h
    ui/pages/remotecommandslistitemdelegate.cpp
    ui/pages/volumecontrolpage.h
    ui/pages/volumecontrolpage.cpp
    ui/pages/volumecontrolpage.ui
    ui/pages/volumedeviceitem.ui
    ui/pages/volumedeviceitem.h
    ui/pages/volumedeviceitem.cpp
    ui/pages/filetransferpage.h
    ui/pages/filetransferpage.cpp
    ui/pages/filetransferpage.ui

    ui/dialogs/pluginsettingsdialog.h
    ui/dialogs/pluginsettingsdialog.cpp
    ui/dialogs/pluginsettingsdialog.ui
    ui/dialogs/clipboardpluginconfigdialog.h
    ui/dialogs/clipboardpluginconfigdialog.cpp
    ui/dialogs/clipboardpluginconfigdialog.ui
    ui/dialogs/batterypluginconfigdialog.h
    ui/dialogs/batterypluginconfigdialog.cpp
    ui/dialogs/batterypluginconfigdialog.ui
    ui/dialogs/runcommandpluginconfigdialog.h
    ui/dialogs/runcommandpluginconfigdialog.cpp
    ui/dialogs/runcommandpluginconfigdialog.ui
    ui/dialogs/appsettingsdialog.ui
    ui/dialogs/appsettingsdialog.h
    ui/dialogs/appsettingsdialog.cpp

    plugin/pluginwrapperbase.h
    plugin/pluginwrapperbase.cpp
    plugin/mprisremotepluginwrapper.h
    plugin/mprisremotepluginwrapper.cpp
    plugin/batterypluginwrapper.h
    plugin/batterypluginwrapper.cpp
    plugin/sftppluginwrapper.h
    plugin/sftppluginwrapper.cpp
    plugin/clipboardpluginwrapper.h
    plugin/clipboardpluginwrapper.cpp
    plugin/remotekeyboardpluginwrapper.h
    plugin/remotekeyboardpluginwrapper.cpp
    plugin/remotemousepadpluginwrapper.h
    plugin/remotemousepadpluginwrapper.cpp
    plugin/pingpluginwrapper.h
    plugin/pingpluginwrapper.cpp
    plugin/findmyphonepluginwrapper.h
    plugin/findmyphonepluginwrapper.cpp
    plugin/smspluginwrapper.h
    plugin/smspluginwrapper.cpp
    plugin/contactspluginwrapper.h
    plugin/contactspluginwrapper.cpp
    plugin/sharepluginwrapper.h
    plugin/sharepluginwrapper.cpp
    plugin/remotecommandspluginwrapper.h
    plugin/remotecommandspluginwrapper.cpp
    plugin/runcommandpluginwrapper.h
    plugin/runcommandpluginwrapper.cpp
    plugin/remotesystemvolumepluginwrapper.h
    plugin/remotesystemvolumepluginwrapper.cpp
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
    Qt::Core
    Qt::Gui
    Qt::Widgets
    SingleApplication::SingleApplication
    kdeconnectcore
    notification
    icons
    KF6Contacts
)

include(GNUInstallDirs)
install(TARGETS kdeconnect-app
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# qt_generate_deploy_app_script(
#     TARGET kdeconnect-app
#     OUTPUT_SCRIPT deploy_script
#     NO_UNSUPPORTED_PLATFORM_ERROR
# )
