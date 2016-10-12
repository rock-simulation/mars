project(PythonMars)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "")
cmake_minimum_required(VERSION 2.8.11)
include(FindPkgConfig)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#find_package(Numpy REQUIRED)
find_package(lib_manager)
lib_defaults()
define_module_info()

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("tr1/functional" USE_TR1)
if(${USE_TR1})
    ADD_DEFINITIONS(-DUSE_TR1)
else(${USE_TR1})
    ADD_DEFINITIONS(-DNO_TR1)
endif()

pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    data_broker
			    mars_interfaces
          configmaps
          osg_points
          osg_lines
          osg_material_manager
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

include_directories(
	src
)

set(SOURCES 
	src/PythonMars.cpp
	src/PythonInterpreter.cpp
)

set(HEADERS
	src/PythonMars.h
	src/PythonInterpreter.hpp
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_definitions(-DPYTHON_SUPPORT)
add_definitions(-DUSE_MEMORYVIEWS)
if (APPLE)
  # here we assume to search for the Python libraries provided by macports
  set(PYTHON_LIBRARY "/opt/local/Library/Frameworks/Python.framework/Versions/2.7/Python")
  set(PYTHON_INCLUDE_DIR "/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7" "/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/numpy/core/include")
  find_package(PythonLibs REQUIRED)
else (APPLE)
  find_package(PythonLibs REQUIRED)
  #pkg_check_modules(PYTHON "python")
  #find_package(Numpy REQUIRED)
endif (APPLE)

find_package(OpenSceneGraph REQUIRED osgManipulator osgViewer osgFX osgShadow osgParticle osgTerrain osgDB osgGA osgWidget osgText osgUtil osgQt)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
link_directories(${OPENSCENEGRAPH_LIBRARY_DIRS})
include(CheckIncludeFileCXX)
set (CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${OPENSCENEGRAPH_INCLUDE_DIRS})
CHECK_INCLUDE_FILE_CXX("osg/Version" HAVE_OSG_VERSION_H)
if(${HAVE_OSG_VERSION_H})
    ADD_DEFINITIONS(-DHAVE_OSG_VERSION_H)
endif()

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${NUMPY_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARY_DIRS})

add_library(${PROJECT_NAME} SHARED ${SOURCES})

find_package(Qt5Widgets)
if (${Qt5Widgets_FOUND})
  qt5_use_modules(${PROJECT_NAME} Widgets)
  target_link_libraries(${PROJECT_NAME}
                        ${PKGCONFIG_LIBRARIES}
                        ${PYTHON_LIBRARIES}
                        ${OPENSCENEGRAPH_LIBRARIES}
   )
else(${Qt5Widgets_FOUND})
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  target_link_libraries(${PROJECT_NAME}
                        ${PKGCONFIG_LIBRARIES}
                        ${QT_LIBRARIES}
                        ${PYTHON_LIBRARIES}
                        ${OPENSCENEGRAPH_LIBRARIES}
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
install(DIRECTORY python DESTINATION share/PythonMars/)
