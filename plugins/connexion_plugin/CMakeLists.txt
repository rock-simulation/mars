project(connexion_plugin)
SET(PROJECT_VERSION 1.0)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")
#set(CMAKE_BUILD_TYPE Debug)

if(WIN32)
    # this fixes the error 998 from the LibManager
    SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import")
    SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--enable-auto-import")
else(WIN32)
    SET(CMAKE_CXX_FLAGS "-fPIC")
endif(WIN32)

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)

pkg_check_modules(LIB_MANAGER REQUIRED "lib_manager")
include_directories(${LIB_MANAGER_INCLUDE_DIRS})
link_directories(${LIB_MANAGER_LIBRARY_DIRS})

pkg_check_modules(DATA_BROKER REQUIRED "data_broker")
include_directories(${DATA_BROKER_INCLUDE_DIRS})
link_directories(${DATA_BROKER_LIBRARY_DIRS})

pkg_check_modules(MAIN_GUI REQUIRED "main_gui")
include_directories(${MAIN_GUI_INCLUDE_DIRS})
link_directories(${MAIN_GUI_LIBRARY_DIRS})

pkg_check_modules(MARS_CORE REQUIRED "mars_interfaces")
include_directories(${MARS_CORE_INCLUDE_DIRS})
link_directories(${MARS_CORE_LIBRARY_DIRS})

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
          ${LIB_MANAGER_LIBRARIES}
          ${MAIN_GUI_LIBRARIES}
          ${MARS_CORE_LIBRARIES}
          ${DATA_BROKER_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

INSTALL(FILES resources/connexion.png DESTINATION share/mars/plugins/connexion_plugin)
