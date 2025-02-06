file(GLOB_RECURSE icon_files FOLLOW_SYMLINKS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.svg)
file(GLOB_RECURSE index_theme_files FOLLOW_SYMLINKS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} index.theme)
file(GLOB_RECURSE scaled_dir_files FOLLOW_SYMLINKS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *@[2-4]x)

set(target_name "hicolor_icons")
set(icon_theme_name "hicolor")

set(icon_qrc_file_in ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.qrc.in)
set(icon_qrc_file ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.qrc)

file(WRITE ${icon_qrc_file_in}
    "<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n<qresource>\n")

foreach(icon_file ${icon_files})
    file(APPEND ${icon_qrc_file_in}
        "<file alias=\"${icon_file}\">${CMAKE_CURRENT_SOURCE_DIR}/${icon_file}</file>\n")
endforeach()

foreach(scaled_dir_file ${scaled_dir_files})
    file(APPEND ${icon_qrc_file_in}
        "<file alias=\"${scaled_dir_file}\">${CMAKE_CURRENT_SOURCE_DIR}/${scaled_dir_file}</file>\n")
endforeach()

foreach(index_theme_file ${index_theme_files})
    file(APPEND ${icon_qrc_file_in}
        "<file alias=\"${index_theme_file}\">${CMAKE_CURRENT_SOURCE_DIR}/${index_theme_file}</file>\n")
endforeach()

file(APPEND ${icon_qrc_file_in}
    "</qresource>\n</RCC>")

configure_file(${icon_qrc_file_in} ${icon_qrc_file} COPYONLY)

# use normal resource adding, big resource variant has issue with LTO
qt_add_resources(target_resource_SRCS
    ${icon_qrc_file}
    OPTIONS --root /icons/${icon_theme_name}
)

qt_add_library(${target_name} OBJECT ${target_resource_SRCS})
