project(__project__)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "__description__")
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
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import -Wall")
        set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--enable-auto-import -Wall")
else(WIN32)
        set(CMAKE_CXX_FLAGS "-fPIC -Wall")
endif(WIN32)

cmake_minimum_required(VERSION 2.6)

pkg_check_modules(LIB_MANAGER "lib_manager")
include_directories(${LIB_MANAGER_INCLUDE_DIRS})
link_directories(${LIB_MANAGER_LIBRARY_DIRS})

pkg_check_modules(DATA_BROKER "data_broker")
include_directories(${DATA_BROKER_INCLUDE_DIRS})
link_directories(${DATA_BROKER_LIBRARY_DIRS})

pkg_check_modules(MARS_INTERFACES REQUIRED "mars_interfaces")
include_directories(${MARS_INTERFACES_INCLUDE_DIRS})
link_directories(${MARS_INTERFACES_LIBRARY_DIRS})

include_directories(
	src
)

set(SOURCES 
	src/__classname__.cpp
)

set(HEADERS
	src/__classname__.h
)



add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
                      ${LIB_MANAGER_LIBRARIES}
                      ${DATA_BROKER_LIBRARIES}
                      ${MARS_INTERFACES_LIBRARIES}
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
install(FILES ${HEADERS} DESTINATION include/mars/plugins/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)


