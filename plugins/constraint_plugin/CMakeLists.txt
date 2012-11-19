project(constraints_plugin)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "Plugin to create constraints between Nodes.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

execute_process(COMMAND git rev-parse HEAD 
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE res
                OUTPUT_VARIABLE git_hash
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git diff-index --quiet HEAD 
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE git_has_local_changes
                OUTPUT_VARIABLE out
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if(git_has_local_changes)
  message(WARNING "git repository has local uncommitted changes!")
  set(git_version "${git_hash} (with local changes)")
else(git_has_local_changes)
  set(git_version ${git_hash})
endif(git_has_local_changes)
add_definitions("-DGIT_INFO=${git_version}")

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

pkg_check_modules(CFG_MANAGER "cfg_manager")
include_directories(${CFG_MANAGER_INCLUDE_DIRS})
link_directories(${CFG_MANAGER_LIBRARY_DIRS})

pkg_check_modules(MAIN_GUI "main_gui")
include_directories(${MAIN_GUI_INCLUDE_DIRS})
link_directories(${MAIN_GUI_LIBRARY_DIRS})

pkg_check_modules(MARS_BASE REQUIRED "mars_interfaces")
include_directories(${MARS_BASE_INCLUDE_DIRS})
link_directories(${MARS_BASE_LIBRARY_DIRS})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})

include_directories(
	src
)

set(SOURCES 
	src/ConstraintsPlugin.cpp
	src/NodeConstraint.cpp
)

set(HEADERS
	src/ConstraintsPlugin.h
)



add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
                      ${LIB_MANAGER_LIBRARIES}
                      ${CFG_MANAGER_LIBRARIES}
                      ${MAIN_GUI_LIBRARIES}
                      ${MARS_BASE_LIBRARIES}
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
install(FILES ${HEADERS} DESTINATION include/mars/pluigns/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)


