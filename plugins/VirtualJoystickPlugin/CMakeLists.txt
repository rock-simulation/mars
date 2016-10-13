project(VirtualJoystickPlugin)
set(PROJECT_VERSION 1.0)

find_package(lib_manager)
lib_defaults()
define_module_info()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")
#set(CMAKE_BUILD_TYPE Debug)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)

pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    data_broker
          cfg_manager
          main_gui
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

setup_qt()

set(HEADER 
	CrossWidget.h
 	JoystickWidget.h
	VirtualJoystickPlugin.h
)

set(SOURCES
	CrossWidget.cpp
 	JoystickWidget.cpp
	VirtualJoystickPlugin.cpp
)

set(SOURCES_UI
    CrossWidget.ui
    JoystickWidget.ui
)

if (${Qt5Widgets_FOUND})
qt5_add_resources(RESOURCES CrossWidget.qrc)
qt5_wrap_cpp( SOURCES_H_MOC ${HEADER} )
qt5_wrap_ui( SOURCES_UI_ ${SOURCES_UI} )
else()
qt4_add_resources(RESOURCES CrossWidget.qrc)
qt4_wrap_cpp( SOURCES_H_MOC ${HEADER} )
qt4_wrap_ui( SOURCES_UI_ ${SOURCES_UI} )
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${SOURCES_H_MOC} ${RESOURCES} ${SOURCES_UI_})

if (${Qt5Widgets_FOUND})
  qt5_use_modules(${PROJECT_NAME} Widgets)
endif()

target_link_libraries(${PROJECT_NAME}
          ${QT_LIBRARIES}
          ${PKGCONFIG_LIBRARIES}
)

IF (WIN32)
  SET (POSTLIB ".dll")
  SET (PRELIB "lib")
  SET (LIBPATH "bin")
ELSE (WIN32)
  IF (APPLE)
    SET (POSTLIB ".dylib")
    SET (PRELIB "lib")
    SET (LIBPATH "lib")
  ELSE (APPLE)
    SET (POSTLIB ".so")
    SET (PRELIB "lib")
    SET (LIBPATH "lib")
  ENDIF (APPLE)
ENDIF (WIN32)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIBPATH}
    ARCHIVE DESTINATION lib)


