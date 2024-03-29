project(PythonMars)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "")
cmake_minimum_required(VERSION 2.8.11)
include(FindPkgConfig)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
          mars_sim
          mars_app
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

add_definitions(-DMARS_DEFAULT_RESOURCES_PATH=\"${CMAKE_INSTALL_PREFIX}/share\")


if("$ENV{PYTHON}" STREQUAL "")
    set(PYTHON "python")
    message(STATUS "Using default python.")
else()
    set(PYTHON $ENV{PYTHON})
endif()

execute_process(
    COMMAND ${PYTHON} -c "import sys;print(sys.version_info.major)"
    OUTPUT_VARIABLE PYTHON_VERSION_MAJOR
)
execute_process(
    COMMAND ${PYTHON} -c "import sys;print(sys.version_info.minor)"
    OUTPUT_VARIABLE PYTHON_VERSION_MINOR
)
string(STRIP ${PYTHON_VERSION_MAJOR} PYTHON_VERSION_MAJOR)
string(STRIP ${PYTHON_VERSION_MINOR} PYTHON_VERSION_MINOR)
message(STATUS "Python: ${PYTHON}")
message(STATUS "Python version: ${PYTHON_VERSION_MAJOR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_definitions(-DPYTHON_SUPPORT)
add_definitions(-DUSE_MEMORYVIEWS)
set(PYTHON_EXECUTABLE ${PYTHON})
find_package(Numpy REQUIRED)
find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} REQUIRED)
message(STATUS "Python include directories: ${PYTHON_INCLUDE_DIRS}")
include_directories(${PYTHON_INCLUDE_DIRS})
message(STATUS "Numpy include directories: ${NUMPY_INCLUDE_DIRS}")
include_directories(${NUMPY_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARY_DIRS})
# if python_libraries contains path to library fix it for linking:
if( NOT APPLE)
if(EXISTS ${PYTHON_LIBRARIES})
  string(REGEX MATCH "[^/]+$" VALUE ${PYTHON_LIBRARIES})
  string(REGEX REPLACE "\\.[^.]*$" "" VALUE2 ${VALUE})
  STRING(REGEX REPLACE "lib([^ ]*)" "\\1"  PYTHON_LIBRARIES ${VALUE2})
endif()
endif()
message(STATUS "Python lib: ${PYTHON_LIBRARIES}")
add_definitions(-DPYTHON_VERSION=${PYTHON_VERSION_MAJOR})
add_definitions(-DPYTHON_LIB="lib${PYTHON_LIBRARIES}.so")

find_package(OpenSceneGraph REQUIRED osgManipulator osgViewer osgFX osgShadow osgParticle osgTerrain osgDB osgGA osgWidget osgText osgUtil)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
link_directories(${OPENSCENEGRAPH_LIBRARY_DIRS})
include(CheckIncludeFileCXX)
set (CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${OPENSCENEGRAPH_INCLUDE_DIRS})
CHECK_INCLUDE_FILE_CXX("osg/Version" HAVE_OSG_VERSION_H)
if(${HAVE_OSG_VERSION_H})
    ADD_DEFINITIONS(-DHAVE_OSG_VERSION_H)
endif()

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
