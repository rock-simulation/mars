project(mars_sim)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This library ...")
cmake_minimum_required(VERSION 2.6)

find_package(Rock)

include(FindPkgConfig)

find_package(lib_manager)
lib_defaults()
define_module_info()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")

set(DEFAULT_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/configuration/mars_default" CACHE STRING "The Default config dir to load")
add_definitions(-DDEFAULT_CONFIG_DIR=\"${DEFAULT_CONFIG_DIR}\")


MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)

rock_find_cmake(Boost)

#Get linker and compiler flags from pkg-config
pkg_check_modules(PKGCONFIG REQUIRED
			    ode
			    lib_manager
			    data_broker
			    cfg_manager
			    mars_interfaces
                mars_utils
                configmaps
)

include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

add_definitions(-DODE11=1 -DdDOUBLE)
add_definitions(-DFORWARD_DECL_ONLY=1)

foreach(DIR ${CFG_MANAGER_INCLUDE_DIRS})
    set(ADD_INCLUDES "${ADD_INCLUDES} -I${DIR}")
endforeach(DIR)

include_directories(
               src/core
               src/interfaces/sensors
               src/physics
               src/sensors
)


include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set(SOURCES_H
       src/core/Controller.h
       src/core/ControllerManager.h
       src/core/EntityManager.h
       src/core/JointManager.h
       src/core/MotorManager.h
       src/core/NodeManager.h
       src/core/PhysicsMapper.h
       src/core/SensorManager.h
       src/core/SimEntity.h
       src/core/SimJoint.h
       src/core/SimMotor.h
       src/core/SimNode.h
       src/core/Simulator.h
       src/sensors/RotatingRaySensor.h
       
       src/physics/JointPhysics.h
       src/physics/NodePhysics.h
       src/physics/WorldPhysics.h
       
       src/sensors/CameraSensor.h
       src/sensors/IDListConfig.h
       src/sensors/Joint6DOFSensor.h
       src/sensors/JointArraySensor.h
       src/sensors/JointAVGTorqueSensor.h
       src/sensors/JointLoadSensor.h
       src/sensors/JointPositionSensor.h
       src/sensors/JointTorqueSensor.h
       src/sensors/JointVelocitySensor.h
       src/sensors/MotorCurrentSensor.h
#       src/sensors/MotorPositionSensor.h
       src/sensors/NodeAngularVelocitySensor.h
       src/sensors/NodeArraySensor.h
       src/sensors/NodeCOMSensor.h
       src/sensors/NodeContactForceSensor.h
       src/sensors/NodeContactSensor.h
       src/sensors/NodePositionSensor.h
       src/sensors/NodeRotationSensor.h
       src/sensors/NodeVelocitySensor.h
#       src/sensors/RayGridSensor.h
       src/sensors/RaySensor.h
       src/sensors/MultiLevelLaserRangeFinder.h

       src/sensors/ScanningSonar.h
       
       src/interfaces/sensors/GridSensorInterface.h
    )

set(TARGET_SRC
       src/core/Controller.cpp
       src/core/ControllerManager.cpp
       src/core/EntityManager.cpp
       src/core/JointManager.cpp
       src/core/MotorManager.cpp
       src/core/NodeManager.cpp
       src/core/PhysicsMapper.cpp
       src/core/SensorManager.cpp
       src/core/SimEntity.cpp
       src/core/SimJoint.cpp
       src/core/SimMotor.cpp
       src/core/SimNode.cpp
       src/core/Simulator.cpp
       src/sensors/MultiLevelLaserRangeFinder.cpp
       src/sensors/RotatingRaySensor.cpp

       src/physics/JointPhysics.cpp
       src/physics/NodePhysics.cpp
       src/physics/WorldPhysics.cpp

       src/sensors/CameraSensor.cpp
       src/sensors/Joint6DOFSensor.cpp
       src/sensors/JointArraySensor.cpp
       src/sensors/JointAVGTorqueSensor.cpp
       src/sensors/JointLoadSensor.cpp
       src/sensors/JointPositionSensor.cpp
       src/sensors/JointTorqueSensor.cpp
       src/sensors/JointVelocitySensor.cpp
       src/sensors/MotorCurrentSensor.cpp
#       src/sensors/MotorPositionSensor.cpp
       src/sensors/NodeAngularVelocitySensor.cpp
       src/sensors/NodeArraySensor.cpp
       src/sensors/NodeCOMSensor.cpp
       src/sensors/NodeContactForceSensor.cpp
       src/sensors/NodeContactSensor.cpp
       src/sensors/NodePositionSensor.cpp
       src/sensors/NodeRotationSensor.cpp
       src/sensors/NodeVelocitySensor.cpp
#       src/sensors/RayGridSensor.cpp
       src/sensors/RaySensor.cpp

       src/sensors/ScanningSonar.cpp
)

#cmake variables
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)
include_directories("${CMAKE_BINARY_DIR}")
add_library(${PROJECT_NAME} SHARED ${TARGET_SRC})

IF (WIN32)
  set(WIN_LIBS -lwsock32 -lwinmm -lpthread)
#  SET_TARGET_PROPERTIES(mars PROPERTIES LINK_FLAGS -Wl,--stack,0x1000000)
ENDIF (WIN32)

set(_INSTALL_DESTINATIONS
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION lib
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            ${PKGCONFIG_LIBRARIES}
            ${WIN_LIBS}
)


#------------------------------------------------------------------------------
set(MARS_HDRS_DIRS
  src/interfaces/core/
  src/interfaces/plugins/
  src/interfaces/sensors/
  src/interfaces/physics/
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
install(FILES ${SOURCES_H} DESTINATION include/mars/sim)

install(DIRECTORY ${MARS_HDRS_DIRS} DESTINATION include/mars/sim FILES_MATCHING PATTERN "*.h")

configure_file(configuration/${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

#configure_file(configuration/mars_Preferences.in ${CMAKE_BINARY_DIR}/mars_Preferences.yaml @ONLY)
#install(FILES ${CMAKE_BINARY_DIR}/mars_Preferences.yaml DESTINATION configuration/mars_default/)
