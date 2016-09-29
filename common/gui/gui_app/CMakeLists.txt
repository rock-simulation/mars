project(gui_app)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This is ...")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

find_package(lib_manager)
lib_defaults()
define_module_info()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

pkg_check_modules(PKGCONFIG
  lib_manager
  cfg_manager
  main_gui
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CLFAGS_OTHER})  #flags excluding the ones with -I

setup_qt()

set(SOURCES
    src/GuiApp.cpp
)

set(HEADERS
    src/GuiApp.hpp
    src/MyApp.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} src/main.cpp)

add_library(${PROJECT_NAME}_lib SHARED ${SOURCES})

if (${USE_QT5})
qt5_use_modules(${PROJECT_NAME} Widgets)
qt5_use_modules(${PROJECT_NAME}_lib Widgets)
endif (${USE_QT5})

IF (WIN32)
  set(WIN_LIBS
        -lwinmm
  )
  SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS -Wl,--stack,0x1000000)
ENDIF (WIN32)

target_link_libraries(${PROJECT_NAME}_lib
            ${QT_LIBRARIES}
            ${WIN_LIBS}
            ${PKGCONFIG_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

INSTALL(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(FILES ${HEADERS} DESTINATION include/mars/gui_app/)
configure_file(gui_app.pc.in ${CMAKE_BINARY_DIR}/gui_app.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/gui_app.pc DESTINATION lib/pkgconfig/)
