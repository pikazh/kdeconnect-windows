add_subdirectory(breeze)
add_subdirectory(hicolor)

qt_add_library(icons SHARED
    $<TARGET_OBJECTS:breeze_icons>
    $<TARGET_OBJECTS:hicolor_icons>
    src/lib/icons.cpp
    src/lib/icons.h
)

include(GenerateExportHeader)
generate_export_header(icons EXPORT_FILE_NAME icon_export.h)

target_include_directories(icons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)
target_include_directories(icons PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(icons
    PRIVATE
        Qt::Core
        Qt::Gui
)
