project(osg_frames)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "Implementation to visualize 3D frames.")
cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)

find_package(OpenSceneGraph REQUIRED osgManipulator osgViewer osgFX osgShadow osgParticle osgTerrain osgDB osgGA osgWidget osgText osgUtil)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
link_directories(${OPENSCENEGRAPH_LIBRARY_DIRS})

set(SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/osg_frames" CACHE STRING "The Default share dir to load")
add_definitions(-DSHARE_DIR=\"${SHARE_DIR}\")

include_directories(src)

set(SOURCES 
	src/FramesFactory.cpp
	src/FrameP.cpp
)

set(HEADERS
	src/FramesFactory.hpp
	src/FrameP.hpp
	src/Frame.hpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
                      ${OPENSCENEGRAPH_LIBRARIES}
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

# Install headers into include directory
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})
INSTALL(DIRECTORY resources DESTINATION share/osg_frames)

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)


