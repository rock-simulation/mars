project(cfg_manager)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "A small library to read, write, and share software parameter using yaml configuration files.")
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


pkg_check_modules(PKGCONFIG
	lib_manager
	yaml-cpp
	mars_utils
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CLFAGS_OTHER})  #flags excluding the ones with -I

include_directories(
	src
)


set(SOURCES 
	src/CFGManager.cpp
	src/CFGParam.cpp
	src/CFGParamBool.cpp
	src/CFGParamDouble.cpp
	src/CFGParamInt.cpp
	src/CFGParamString.cpp
	src/CFGProperty.cpp
)

set(HEADERS
	src/CFGClient.h
	src/CFGDefs.h
	src/CFGManager.h
	src/CFGManagerInterface.h
	src/CFGParam.h
	src/CFGProperty.h
)



add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
                      ${PKGCONFIG_LIBRARIES}
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


