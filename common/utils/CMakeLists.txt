cmake_minimum_required(VERSION 2.6)

project(mars_utils)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This Library contains several useful utilities like a Mutex and Thread class.")

include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

pkg_check_modules(PKGCONFIG REQUIRED
        eigen3
        yaml-cpp
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CLFAGS_OTHER})  #flags excluding the ones with -I

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
    src/Socket.cpp
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
    src/Socket.h
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




