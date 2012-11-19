cmake_minimum_required(VERSION 2.6)

project(mars_utils)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This Library contains several useful utilities like a Mutex and Thread class.")

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
        SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import -Wall")
        SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--enable-auto-import -Wall")
else(WIN32)
        SET(CMAKE_CXX_FLAGS "-fPIC -Wall")
endif(WIN32)

pkg_check_modules(PKGCONFIG REQUIRED
        eigen3
        yaml-cpp
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CLFAGS_OTHER})  #flags excluding the ones with -I

if(NOT ${CMAKE_C_SIZEOF_DATA_PTR} EQUAL 8)
  set(MARS_CORE_EIGEN_DEFINITIONS -DEIGEN_DONT_ALIGN=1)
  add_definitions(${MARS_CORE_EIGEN_DEFINITIONS})
endif()

set(SOURCES 
    src/Color.cpp
    src/ConfigData.cpp
    src/Mutex.cpp
    src/MutexLocker.cpp
    src/ReadWriteLock.cpp
    src/ReadWriteLocker.cpp
    src/Thread.cpp
    src/WaitCondition.cpp
    src/mathUtils.cpp
    src/misc.cpp
)
set(HEADERS
    src/Color.h
    src/ConfigData.h
    src/FIFOMap.h
    src/Mutex.h
    src/MutexLocker.h
    src/Quaternion.h
    src/ReadWriteLock.h
    src/ReadWriteLocker.h
    src/Thread.h
    src/Vector.h
    src/WaitCondition.h
    src/mathUtils.h
    src/misc.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(
        ${PROJECT_NAME}
        ${PKGCONFIG_LIBRARIES} 
        -lpthread
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
install(FILES ${HEADERS} DESTINATION include/mars/utils)

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)




