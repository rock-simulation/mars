project(osg_material_manager)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "")
cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)
include(CheckIncludeFileCXX)

find_package(lib_manager)
lib_defaults()
define_module_info()

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
			    lib_manager
			    configmaps
			    mars_utils
          mars_interfaces
          cfg_manager
          opencv
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #cflags without -I


add_definitions(-DMARS_PREFERENCES_DEFAULT_RESOURCES_PATH=\"${CMAKE_INSTALL_PREFIX}/share\")

set(SOURCES 
	src/OsgMaterialManager.cpp
	src/OsgMaterial.cpp
	src/MaterialNode.cpp
	src/shader/shader-types.cpp
	src/shader/shader-generator.cpp
	src/shader/shader-function.cpp
	src/shader/bumpmapping.cpp
	src/shader/pixellight.cpp
)

set(HEADERS
	src/OsgMaterialManager.h
	src/OsgMaterial.h
	src/MaterialNode.h
	src/shader/shader-types.h
	src/shader/shader-generator.h
	src/shader/shader-function.h
	src/shader/bumpmapping.h
	src/shader/pixellight.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
                      ${OPENSCENEGRAPH_LIBRARIES}
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
install(FILES ${HEADERS} DESTINATION include/mars/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
INSTALL(DIRECTORY resources DESTINATION share/mars/osg_material_manager)


