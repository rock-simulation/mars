project(log_console)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "The library provides a console module.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

IF (APPLE)
  EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
  STRING(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
  MESSAGE(STATUS "DARWIN_VERSION=${DARWIN_VERSION}")
  IF (DARWIN_VERSION EQUAL 11)
    MESSAGE("use gnu compiler")
    set(CMAKE_C_COMPILER "gcc-apple-4.2")
    set(CMAKE_CXX_COMPILER "g++-apple-4.2")
  ENDIF (DARWIN_VERSION EQUAL 11)
ENDIF(APPLE)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})

pkg_check_modules(LIB_MANAGER "lib_manager")
include_directories(${LIB_MANAGER_INCLUDE_DIRS})
link_directories(${LIB_MANAGER_LIBRARY_DIRS})

pkg_check_modules(DATA_BROKER "data_broker")
include_directories(${DATA_BROKER_INCLUDE_DIRS})
link_directories(${DATA_BROKER_LIBRARY_DIRS})

pkg_check_modules(CFG_MANAGER "cfg_manager")
include_directories(${CFG_MANAGER_INCLUDE_DIRS})
link_directories(${CFG_MANAGER_LIBRARY_DIRS})

pkg_check_modules(MAIN_GUI "main_gui")
include_directories(${MAIN_GUI_INCLUDE_DIRS})
link_directories(${MAIN_GUI_LIBRARY_DIRS})

link_directories(${QT_LIBRARY_DIR})

include_directories(src)


set(SOURCES 
	src/ConsoleGUI.cpp
	src/MainConsole.cpp
)

set(HEADERS
	src/ConsoleGUI.h
	src/ConsoleInterface.h
	src/MainConsole.h
)

set(QT_MOC_HEADER
	src/ConsoleGUI.h
	src/MainConsole.h
)


qt4_wrap_cpp( QT_MOC_HEADER_SRC ${QT_MOC_HEADER} )


add_library(${PROJECT_NAME} SHARED ${SOURCES} ${QT_MOC_HEADER_SRC} )

target_link_libraries(${PROJECT_NAME}
                      ${LIB_MANAGER_LIBRARIES}
                      ${DATA_BROKER_LIBRARIES}
                      ${CFG_MANAGER_LIBRARIES}
                      ${MAIN_GUI_LIBRARIES}
                      ${QT_LIBRARIES}
)

if(WIN32)
  set(LIB_INSTALL_DIR bin) # .dll are in PATH, like executables
else(WIN32)
  set(LIB_INSTALL_DIR lib)
endif(WIN32)


set(_INSTALL_DESTINATIONS
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION lib
)


# Install the library into the lib folder
install(TARGETS ${PROJECT_NAME} ${_INSTALL_DESTINATIONS})

# Install headers into mars include directory
install(FILES ${HEADERS} DESTINATION include/mars/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

INSTALL(DIRECTORY resources DESTINATION share/mars/${PROJECT_NAME})
