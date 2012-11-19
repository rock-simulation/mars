project(lib_manager)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This is the simulation plugin for the RIMRES Reipos modules")
cmake_minimum_required(VERSION 2.6)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

set(SOURCES 
	src/LibManager.cpp
)
set(HEADERS
	src/LibInterface.h
	src/LibManager.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if(UNIX)
  target_link_libraries(${PROJECT_NAME} dl)
endif(UNIX)

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

