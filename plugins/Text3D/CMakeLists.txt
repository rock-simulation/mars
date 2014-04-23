project(Text3D)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This plugin loads text elements for the hud from a config file.")
cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)

include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)
#find_package(mars)

mars_defaults()
define_module_info()

include(CheckIncludeFileCXX)

pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    data_broker
			    mars_interfaces
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

pkg_check_modules(PKGCONFIG_HEADER REQUIRED
			    osg_text_factory
			    osg_text
)
include_directories(${PKGCONFIG_HEADER_INCLUDE_DIRS})


include_directories(
	src
)

set(SOURCES 
	src/Text3D.cpp
)

set(HEADERS
	src/Text3D.h
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
install(FILES ${HEADERS} DESTINATION include/mars/plugins/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)


