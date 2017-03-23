project(osg_terrain)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "")
cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)
include(CheckIncludeFileCXX)

find_package(OpenSceneGraph REQUIRED  osgDB osgText osgUtil)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
link_directories(${OPENSCENEGRAPH_LIBRARY_DIRS})

set (CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${OPENSCENEGRAPH_INCLUDE_DIRS})
CHECK_INCLUDE_FILE_CXX("osg/Version" HAVE_OSG_VERSION_H)
if(${HAVE_OSG_VERSION_H})
    ADD_DEFINITIONS(-DHAVE_OSG_VERSION_H)
endif()

include_directories(src)

pkg_check_modules(PKGCONFIG REQUIRED
			    mars_graphics
			    configmaps
			    mars_utils
          osg_material_manager
          opencv
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #cflags without -I


add_definitions(-DMARS_PREFERENCES_DEFAULT_RESOURCES_PATH=\"${CMAKE_INSTALL_PREFIX}/share\")

set(SOURCES 
	src/Terrain.cpp
	src/MultiResHeightMapRenderer.cpp
	src/VertexBufferTerrain.cpp
)

set(HEADERS
	src/Terrain.h
	src/MultiResHeightMapRenderer.h
	src/VertexBufferTerrain.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})


if(APPLE)
 FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
 SET(APPLE_LIBS ${OPENGL_LIBRARY} -L/opt/local/lib opencv_core opencv_highgui opencv_imgproc)
endif(APPLE)

target_link_libraries(${PROJECT_NAME}
                      ${OPENSCENEGRAPH_LIBRARIES}
                      ${PKGCONFIG_LIBRARIES}
                      ${APPLE_LIBS}
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
install(FILES ${HEADERS} DESTINATION include/mars/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)


