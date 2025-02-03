qt_add_library(coreaddons SHARED
    lib/text/jsonutils.h
    lib/text/jsonutils.cpp
    plugin/pluginmetadata.h
    plugin/pluginmetadata.cpp
    plugin/pluginfactory.h
    plugin/pluginfactory.cpp
    coreaddons_debug.h
    coreaddons_debug.cpp
)

include(GenerateExportHeader)
generate_export_header(coreaddons EXPORT_FILE_NAME coreaddons_export.h)

set(coreaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/text/
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/
)

target_include_directories(coreaddons PUBLIC ${coreaddons_INCLUDE_DIRS})
target_include_directories(coreaddons PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(coreaddons PRIVATE Qt::Core)

include(GNUInstallDirs)
install(TARGETS coreaddons
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
