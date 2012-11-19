project(data_broker_plotter)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "Plot implementation for the DataBroker by using QCustomPlot.")
cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIR})
link_directories(${QT_LIBRARY_DIR})
add_definitions("-D_REENTRANT -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED")
remove_definitions(-DQT_DLL)

pkg_check_modules(LIB_MANAGER "lib_manager")
include_directories(${LIB_MANAGER_INCLUDE_DIRS})
link_directories(${LIB_MANAGER_LIBRARY_DIRS})

pkg_check_modules(GUI_CORE "main_gui")
include_directories(${GUI_CORE_INCLUDE_DIRS})
link_directories(${GUI_CORE_LIBRARY_DIRS})

pkg_check_modules(DATA_BROKER "data_broker")
include_directories(${DATA_BROKER_INCLUDE_DIRS})
link_directories(${DATA_BROKER_LIBRARY_DIRS})

pkg_check_modules(CFG "cfg_manager")
include_directories(${CFG_INCLUDE_DIRS})
link_directories(${CFG_LIBRARY_DIRS})

pkg_check_modules(EIGEN "eigen3")
include_directories(${EIGEN_INCLUDE_DIRS})

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

qt4_wrap_ui( SOURCES_UI_H ${SOURCES_UI})

qt4_wrap_cpp ( QT_MOC_HEADER_SRC ${QT_MOC_HEADER} )

cmake_minimum_required(VERSION 2.6)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${QT_MOC_HEADER_SRC} ${SOURCES_UI_H} )

target_link_libraries(${PROJECT_NAME}
                      ${LIB_MANAGER_LIBRARIES}
                      ${GUI_CORE_LIBRARIES}
                      ${DATA_BROKER_LIBRARIES}
                      ${CFG_LIBRARIES}
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


