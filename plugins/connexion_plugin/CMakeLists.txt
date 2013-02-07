project(connexion_plugin)
SET(PROJECT_VERSION 1.0)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 2.6)

#set(CMAKE_BUILD_TYPE Debug)
include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    data_broker
                            main_gui
			    mars_interfaces
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I


find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})

set(HEADER 
	ConnexionPlugin.h
 	ConnexionWidget.h	
 	ConnexionHID.h	
)

set(QT_HEADER 
	ConnexionPlugin.h
 	ConnexionWidget.h	
)

set(SOURCES
	ConnexionPlugin.cpp
 	ConnexionWidget.cpp
)

if(WIN32)
  set(SOURCES
      ${SOURCES}
      win32/ConnexionHID.cpp
  )
elseif(UNIX)
  if(APPLE)
    set(SOURCES
        ${SOURCES}
        osx/ConnexionHID.cpp
    )
    find_library(CARBON NAMES Carbon)
    find_library(IOKIT NAMES IOKit)
    add_definitions(-D__OSX__)
  else(APLLE)
    set(SOURCES
        ${SOURCES}
        linux/ConnexionHID.cpp
    )
  endif(APPLE)
endif(WIN32)

qt4_wrap_cpp(HEADER_MOC ${QT_HEADER})

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADER_MOC})

target_link_libraries(${PROJECT_NAME}
          ${QT_LIBRARIES}
          ${CARBON}
          ${IOKIT}
          ${PKGCONFIG_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

INSTALL(FILES resources/connexion.png DESTINATION share/mars/plugins/connexion_plugin)
