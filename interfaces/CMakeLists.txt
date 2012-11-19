project(mars_interfaces)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This library is the base library for the MARS prject and includes the general defines, structs and classes.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()


pkg_check_modules(MARS_UTILS REQUIRED "mars_utils")
include_directories(${MARS_UTILS_INCLUDE_DIRS})
link_directories(${MARS_UTILS_LIBRARY_DIRS})

pkg_check_modules(DATA_BROKER REQUIRED "data_broker")
include_directories(${DATA_BROKER_INCLUDE_DIRS})
link_directories(${DATA_BROKER_LIBRARY_DIRS})

include_directories(src)


set(HEADERS
    src/cameraStruct.h
    src/contact_params.h
    src/ControllerData.h
    src/core_objects_exchange.h
    src/GraphicData.h
    src/JointData.h
    src/LightData.h
    src/MARSDefs.h
    src/MaterialData.h
    src/MotorData.h
    src/nodeState.h
    src/NodeData.h
    src/sensor_bases.h
    src/sim_common.h
    src/snmesh.h
    src/terrainStruct.h
    src/utils.h

    src/exceptions/SceneParseException.h
)

set(SOURCES 
    src/sim/ControlCenter.cpp
    src/MaterialData.cpp
    src/NodeData.cpp
    src/JointData.cpp
    src/MotorData.cpp
    src/LightData.cpp
    src/GraphicData.cpp
    src/ControllerData.cpp
    src/utils.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(
        ${PROJECT_NAME}
        ${MARS_UTILS_LIBRARIES}
        ${DATA_BROKER_LIBRARIES}
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

# Install the library
install(TARGETS ${PROJECT_NAME} ${_INSTALL_DESTINATIONS})

# Install headers into mars include directory
install(FILES ${HEADERS} DESTINATION include/mars/interfaces)

FILE(GLOB files "src/graphics/*.h")
install(FILES ${files} DESTINATION include/mars/interfaces/graphics)
FILE(GLOB files "src/sim/*.h")
install(FILES ${files} DESTINATION include/mars/interfaces/sim)
FILE(GLOB files "src/gui/*.h")
install(FILES ${files} DESTINATION include/mars/interfaces/gui)

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
