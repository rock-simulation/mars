project(log_console)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "The library provides a console module.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

execute_process(COMMAND git rev-parse
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE under_git_control
                ERROR_QUIET)
if(NOT under_git_control)
  execute_process(COMMAND git rev-parse HEAD 
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE git_hash
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git diff-index --quiet HEAD 
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE git_has_local_changes
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git remote -v
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE git_src
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "origin	([^ ]*) \\(fetch\\).*" "\\1" git_src ${git_src})
  if(git_has_local_changes)
    message(WARNING "git repository has local uncommitted changes!")
    set(git_revision "${git_hash} (with local changes)")
  else(git_has_local_changes)
    set(git_version ${git_hash})
  endif(git_has_local_changes)
  add_definitions("-DGIT_INFO" "-DGIT_INFO_REV=${git_revision}" "-DGIT_INFO_SRC=${git_src}")
endif(NOT under_git_control)

if(WIN32)
        # this fixes the error 998 from the LibManager
        SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import")
        SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--enable-auto-import")
else(WIN32)
        SET(CMAKE_CXX_FLAGS "-fPIC")
endif(WIN32)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

IF (APPLE)
  EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
  STRING(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
  MESSAGE(STATUS "DARWIN_VERSION=${DARWIN_VERSION}")
  IF (DARWIN_VERSION GREATER 10)
    MESSAGE("use gnu compiler")
    set(CMAKE_C_COMPILER "gcc-apple-4.2")
    set(CMAKE_CXX_COMPILER "g++-apple-4.2")
  ENDIF (DARWIN_VERSION GREATER 10)
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
