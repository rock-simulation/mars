project(__project__)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "__description__")
cmake_minimum_required(VERSION 2.8.11)
include(FindPkgConfig)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(mars)
mars_defaults()
define_module_info()

pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    data_broker
			    mars_interfaces
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

include_directories(
	src
)

set(SOURCES 
	src/__classname__.cpp
	src/__classname___MainWin.cpp
)

set(HEADERS
	src/__classname__.h
	src/__classname___MainWin.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
find_package(Qt5Widgets)
if (${Qt5Widgets_FOUND})
  qt5_use_modules(${PROJECT_NAME} Widgets)
  target_link_libraries(${PROJECT_NAME}
                        ${PKGCONFIG_LIBRARIES}
  )
else(${Qt5Widgets_FOUND})
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  target_link_libraries(${PROJECT_NAME}
                        ${PKGCONFIG_LIBRARIES}
                        ${QT_LIBRARIES}
  )
endif(${Qt5Widgets_FOUND})


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