project(gamepad_plugin)
SET(PROJECT_VERSION 1.0)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 2.6)

#set(CMAKE_BUILD_TYPE Debug)
include(FindPkgConfig)

find_package(lib_manager)
lib_defaults()
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
            configmaps
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

setup_qt()

set(HEADER
    src/GamepadPlugin.hpp
    src/GamepadHID.hpp
)

set(SOURCES
    src/GamepadPlugin.cpp
)

if(WIN32)
  set(SOURCES
      ${SOURCES}
      src/win32/GamepadHID.cpp
  )
elseif(UNIX)
  if(APPLE)
    set(SOURCES
        ${SOURCES}
        src/osx/GamepadHID.cpp
    )
    find_library(CARBON NAMES Carbon)
    find_library(IOKIT NAMES IOKit)
    add_definitions(-D__OSX__)
  else(APLLE)
    set(SOURCES
        ${SOURCES}
        src/linux/GamepadHID.cpp
    )
  endif(APPLE)
endif(WIN32)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
          ${PKGCONFIG_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
