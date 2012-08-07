project(mars_utils)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This Library contains several useful utilities like a Mutex and Thread class.")

include(FindPkgConfig)

if(WIN32)
        # this fixes the error 998 from the LibManager
        SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import -Wall")
        SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--enable-auto-import -Wall")
else(WIN32)
        SET(CMAKE_CXX_FLAGS "-fPIC -Wall")
endif(WIN32)

pkg_check_modules(EIGEN REQUIRED "eigen3")
include_directories(${EIGEN_INCLUDE_DIRS})
link_directories(${EIGEN_LIBRARY_DIRS})

if(NOT ${CMAKE_C_SIZEOF_DATA_PTR} EQUAL 8)
  set(MARS_CORE_EIGEN_DEFINITIONS -DEIGEN_DONT_ALIGN=1)
  add_definitions(${MARS_CORE_EIGEN_DEFINITIONS})
endif()

set(SOURCES 
    src/Color.cpp
    src/Mutex.cpp
    src/MutexLocker.cpp
    src/ReadWriteLock.cpp
    src/ReadWriteLocker.cpp
    src/Thread.cpp
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
    src/mathUtils.h
    src/misc.h
)

cmake_minimum_required(VERSION 2.6)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(
        ${PROJECT_NAME}
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




