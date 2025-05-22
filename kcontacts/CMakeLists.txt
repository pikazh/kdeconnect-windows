qt_add_library(KF6Contacts SHARED
    kcontacts_debug.h
    kcontacts_debug.cpp
    kcontacts.qrc
    address.cpp
    addressee.cpp
    addressee.h
    addresseehelper.cpp
    addresseehelper_p.h
    addresseelist.h
    addressformat.cpp
    addressformatparser.cpp
    addressformatscript.cpp
    addressformatter.cpp
    address.h
    calendarurl.cpp
    calendarurl.h
    clientpidmap.cpp
    clientpidmap.h
    contactgroup.cpp
    contactgroup.h
    contactgrouptool.cpp
    contactgrouptool.h
    email.cpp
    email.h
    fieldgroup.cpp
    fieldgroup.h
    gender.cpp
    gender.h
    geo.cpp
    geo.h
    impp.cpp
    impp.h
    key.cpp
    key.h
    lang.cpp
    lang.h
    ldif.cpp
    ldif_p.h
    namespace.cpp
    nickname.cpp
    nickname.h
    note.cpp
    note.h
    org.cpp
    org.h
    parametermap_p.h
    phonenumber.cpp
    phonenumber.h
    picture.cpp
    picture.h
    related.cpp
    related.h
    resourcelocatorurl.cpp
    resourcelocatorurl.h
    role.cpp
    role.h
    secrecy.cpp
    secrecy.h
    sound.cpp
    sound.h
    timezone.cpp
    timezone.h
    title.cpp
    title.h
    vcarddrag.cpp
    vcarddrag.h
    vcardparser/vcard.cpp
    vcardparser/vcardline.cpp
    vcardparser/vcardline_p.h
    vcardparser/vcardparser.cpp
    vcardparser/vcardparser_p.h
    vcardparser/vcard_p.h
    converter/ldifconverter.h
    converter/ldifconverter.cpp
    converter/vcardconverter.h
    converter/vcardconverter.cpp
    vcardtool.cpp
    vcardtool_p.h
    desktopfile.h
    desktopfile.cpp
)

qt_add_resources(KF6Contacts "improtocols"
    PREFIX "/org.kde.kcontacts/improtocols/"
    BASE "improtocols"
    FILES
        improtocols/aim.desktop
        improtocols/facebook.desktop
        improtocols/gg.desktop
        improtocols/googletalk.desktop
        improtocols/groupwise.desktop
        improtocols/icq.desktop
        improtocols/irc.desktop
        improtocols/meanwhile.desktop
        improtocols/msn.desktop
        improtocols/qq.desktop
        improtocols/sip.desktop
        improtocols/skype.desktop
        improtocols/sms.desktop
        improtocols/twitter.desktop
        improtocols/xmpp.desktop
        improtocols/ymsgr.desktop
        improtocols/matrix.desktop
        improtocols/telegram.desktop
        improtocols/rocketchat.desktop
        improtocols/signal.desktop
        improtocols/threema.desktop
)

include(GenerateExportHeader)
generate_export_header(KF6Contacts BASE_NAME kcontacts EXPORT_FILE_NAME kcontacts_export.h)

target_include_directories(KF6Contacts PUBLIC 
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vcardparser>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/converter>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>") 

target_link_libraries(KF6Contacts
PRIVATE
    Qt::Core
    Qt::Gui
    kdeconnectcore
    KF6Codecs
)

if (USE_QML)
  add_subdirectory(qml)
endif ()
#add_subdirectory(converter)

include(GNUInstallDirs)
install(TARGETS KF6Contacts
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
