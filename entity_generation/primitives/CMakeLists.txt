project(mars_primitives)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "entity factory for primitives")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 2.6)

#set(CMAKE_BUILD_TYPE Debug)

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)
#include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

find_package(lib_manager)
define_module_info()

find_package(Boost REQUIRED system thread)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})


#Get linker and compiler flags from pkg-config
pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    configmaps
			    mars_interfaces
			    mars_entity_factory
			    mars_sim
)

include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I


include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set(SOURCES
	src/primitives.cpp
)

set(HEADERS
	src/primitives.h
)



add_library(${PROJECT_NAME} SHARED ${SOURCES})


target_link_libraries(${PROJECT_NAME}
                      ${PKGCONFIG_LIBRARIES}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            ${PKGCONFIG_LIBRARIES}
            ${WIN_LIBS}
            z
)


#------------------------------------------------------------------------------

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
install(FILES ${HEADERS} DESTINATION include/mars/entity_generation/primitives)

# Prepare and install necessary files to support finding of the library
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
