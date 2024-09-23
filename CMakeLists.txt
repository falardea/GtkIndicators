cmake_minimum_required(VERSION 3.16)

project(GtkIndicators VERSION 0.0.1  LANGUAGES C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)
set(GRESOURCE_C     gtk_indicators.gresource.c)
set(GRESOURCE_XML   gtk_indicators.gresource.xml)

set(PROJECT_TARGET      gtk_indicators)
set(PROJECT_SOURCE_DIR  ./src)
set(PROJECT_BINARY_DIR  ./target)

set(CSS_TARGET gtk_indicators_style.css)

set(PROJECT_RESOURCE_DIR    ${CMAKE_SOURCE_DIR}/resources)
set(STYLE_DIR               ${PROJECT_RESOURCE_DIR}/style)
set(SCSS_SOURCE             ${STYLE_DIR}/scss/gtk_indicators_style.scss)
set(CSS_OUTFILE             ${STYLE_DIR}/${CSS_TARGET})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_compile_options(${CMAKE_C_FLAGS} -Wall -Wextra -rdynamic)
add_link_options(-rdynamic)

add_custom_command(OUTPUT ${CSS_OUTFILE} COMMAND sassc -t expanded ${SCSS_SOURCE} ${CSS_OUTFILE})

# This will build our resource bundle
# From: https://stackoverflow.com/questions/63697778/how-to-use-glib-compile-resources-with-cmake
add_custom_command(
      OUTPUT ${GRESOURCE_C}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND ${GLIB_COMPILE_RESOURCES}
      ARGS
      --generate-source
      --target=${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
      ${PROJECT_RESOURCE_DIR}/${GRESOURCE_XML}
      VERBATIM
      MAIN_DEPENDENCY ${PROJECT_RESOURCE_DIR}/${GRESOURCE_XML}
      DEPENDS
      ${PROJECT_RESOURCE_DIR}/gtk_indicators.glade
      ${CSS_OUTFILE}
)
add_custom_target(gtk_indicators-resource DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C})

set(MY_SOURCES
    src/main.c
    src/app_globals.h
    src/utils/parse_args.c
    src/utils/parse_args.h
    src/utils/logging.c
    src/utils/logging.h
    src/utils/sys_interface.c
    src/utils/sys_interface.h
    src/interfaces/app_interface.c
    src/interfaces/app_interface.h
    src/views/ui_builder.c
    src/views/ui_builder.h
    src/views/css_styler.c
    src/views/css_styler.h
    src/views/root_window.c
    src/views/root_window.h
    src/indicators/battery_indicator.c
    src/indicators/battery_indicator.h
    src/indicators/gtkdial.c
    src/indicators/gtkdial.h
    src/indicators/numeric_label.c
    src/indicators/numeric_label.h
    src/indicators/basic_level_indicator.c
    src/indicators/basic_level_indicator.h
    src/indicators/bluetooth_indicator.c
    src/indicators/bluetooth_indicator.h
)

include_directories(${PROJECT_SOURCE_DIR})
configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${PROJECT_SOURCE_DIR}/version.h")

add_executable(${PROJECT_TARGET} ${MY_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C})

target_include_directories(${PROJECT_TARGET} PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(${PROJECT_TARGET} ${GTK3_LIBRARIES} PkgConfig::deps m)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C} PROPERTIES GENERATED TRUE )
add_dependencies(${PROJECT_TARGET} gtk_indicators-resource)