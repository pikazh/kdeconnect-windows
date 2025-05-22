qt_add_library(kdeconnect_contacts SHARED
    kdeconnect_contacts.json
    contactsplugin.h
    contactsplugin.cpp
    plugin_contacts_debug.h
    plugin_contacts_debug.cpp
    contactsdb.h
    contactsdb.cpp
)

target_link_libraries(kdeconnect_contacts PRIVATE Qt::Core Qt::Sql Qt::Gui Qt::Network kdeconnectcore KF6Contacts)

include(GNUInstallDirs)
install(TARGETS kdeconnect_contacts
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/kdeconnect_plugins"
)
