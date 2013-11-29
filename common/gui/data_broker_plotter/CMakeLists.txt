project(data_broker_plotter)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "Plot implementation for the DataBroker by using QCustomPlot.")
cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

find_package(Qt5Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

#find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})
#include_directories(${QT_INCLUDE_DIR})
#link_directories(${QT_LIBRARY_DIR})
#add_definitions("-D_REENTRANT -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED")
#remove_definitions(-DQT_DLL)

pkg_check_modules(PKGCONFIG REQUIRED
        lib_manager
        cfg_manager
        data_broker
        main_gui
        eigen3
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CLFAGS_OTHER})  #flags excluding the ones with -I

include_directories(src)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/qcustomplot )

#user interface compilation
#set(SOURCES_UI src/init_spaceclimber.ui)

set(SOURCES 
	src/DataBrokerPlotterLib.cpp
	src/DataBrokerPlotter.cpp
	src/qcustomplot/qcustomplot.cpp
)

set(HEADERS
	src/DataBrokerPlotterLib.h
	src/DataBrokerPlotter.h
	src/qcustomplot/qcustomplot.h
)

set (QT_MOC_HEADER
	src/DataBrokerPlotterLib.h
	src/DataBrokerPlotter.h
	src/qcustomplot/qcustomplot.h
)

qt5_wrap_ui( SOURCES_UI_H ${SOURCES_UI})

qt5_wrap_cpp ( QT_MOC_HEADER_SRC ${QT_MOC_HEADER} )

cmake_minimum_required(VERSION 2.6)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${QT_MOC_HEADER_SRC} ${SOURCES_UI_H} )

qt5_use_modules(${PROJECT_NAME} Widgets)

target_link_libraries(${PROJECT_NAME}
                      ${PKGCONFIG_LIBRARIES}
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
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})

#INSTALL(DIRECTORY resources DESTINATION share/data_broker_widget)

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)


