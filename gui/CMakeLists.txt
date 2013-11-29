project(mars_gui)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This library is a GUI based in the main_gui for MARS.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

mars_defaults()
define_module_info()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")


pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    data_broker
			    main_gui
			    cfg_manager
			    mars_interfaces
			    opencv
)

include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #not the ones starting with -I

find_package(OpenSceneGraph REQUIRED osgManipulator osgViewer osgFX osgShadow osgParticle osgTerrain osgDB osgGA osgText osgUtil)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
link_directories(${OPENSCENEGRAPH_LIBRARY_DIRS})

find_package(Qt4 REQUIRED)
set(QT_USE_QTOPENGL TRUE)
include(${QT_USE_FILE})
INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})

QT4_ADD_RESOURCES(MARSGuiResources resources/mars.qrc)

include_directories(
        src 
        src/menus
        src/dialogs
        src/dialogs/CameraConfig
        src/dialogs/CaptureWindow
)


include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )


set(HEADERS 
        src/ControllerHandler.h
        src/JointHandler.h
        src/JoystickWidget.h
        src/LightHandler.h
        src/MainGUIDefs.h
        src/MarsGui.h
        #src/MarsStyle.h
        src/MotorHandler.h
        src/NodeHandler.h
        src/NodeSelectionTree.h
        src/SelectionDialog.h
        src/SensorHandler.h
        src/Widget_Joint_State.h 
        src/Widget_Node_Options_ODE.h 
        src/Widget_Node_State.h 

        src/dialogs/AboutDialog.h 
        src/dialogs/BlenderExportGUI.h
        src/dialogs/ControllerConfigGUI.h
        src/dialogs/Dialog_Add_Force.h 
        src/dialogs/Dialog_Add_Torque.h 
        src/dialogs/Dialog_Create_Material.h 
        src/dialogs/Dialog_Edit_Material.h
        src/dialogs/Dialog_Graphics_Options.h 
        src/dialogs/Dialog_Import_Mesh.h 
        src/dialogs/Dialog_Motor_Control.h 
        src/dialogs/Dialog_Rescale_Environment.h 
        src/dialogs/DialogControllers.h
        src/dialogs/DialogDistance.h
        src/dialogs/DialogJoints.h
        src/dialogs/DialogJoystick.h
        src/dialogs/DialogLights.h
        src/dialogs/DialogMotors.h
        src/dialogs/DialogNodes.h
        src/dialogs/DialogSensors.h
        
        src/dialogs/CameraConfig/CameraConfig.h
        src/dialogs/CameraConfig/CameraConfigurator.h
        src/dialogs/CameraConfig/CameraConfiguratorGUI.h
        
        src/dialogs/CaptureWindow/CaptureConfig.h
        src/dialogs/CaptureWindow/CaptureGUI.h
        src/dialogs/CaptureWindow/CaptureWindow.h
        src/dialogs/CaptureWindow/ImageProcess.h
        
        src/menus/MenuFile.h
        src/menus/MenuHelp.h
        src/menus/MenuSimulation.h
        src/menus/MenuWindow.h
)

set(QT_MOC_HEADER
        src/ControllerHandler.h
        src/JointHandler.h
        src/JoystickWidget.h
        src/LightHandler.h
        #src/MarsStyle.h
        src/MotorHandler.h
        src/NodeHandler.h
        src/NodeSelectionTree.h
        src/SelectionDialog.h
        src/SensorHandler.h
        src/Widget_Joint_State.h 
        src/Widget_Node_Options_ODE.h 
        src/Widget_Node_State.h 

        src/dialogs/AboutDialog.h 
        src/dialogs/BlenderExportGUI.h
        src/dialogs/ControllerConfigGUI.h
        src/dialogs/Dialog_Add_Force.h 
        src/dialogs/Dialog_Add_Torque.h 
        src/dialogs/Dialog_Create_Material.h 
        src/dialogs/Dialog_Edit_Material.h
        src/dialogs/Dialog_Import_Mesh.h 
        src/dialogs/Dialog_Motor_Control.h 
        src/dialogs/Dialog_Rescale_Environment.h 
        src/dialogs/Dialog_Graphics_Options.h 
        src/dialogs/DialogControllers.h
        src/dialogs/DialogDistance.h
        src/dialogs/DialogJoints.h
        src/dialogs/DialogJoystick.h
        src/dialogs/DialogLights.h
        src/dialogs/DialogMotors.h
        src/dialogs/DialogNodes.h
        src/dialogs/DialogSensors.h

        src/dialogs/CameraConfig/CameraConfigurator.h
        src/dialogs/CameraConfig/CameraConfiguratorGUI.h

        src/dialogs/CaptureWindow/CaptureConfig.h
        src/dialogs/CaptureWindow/CaptureGUI.h
        src/dialogs/CaptureWindow/CaptureWindow.h

        src/menus/MenuSimulation.h
        src/menus/MenuWindow.h
)
 
qt4_wrap_cpp( QT_MOC_HEADER_SRC ${QT_MOC_HEADER} )

set(TARGET_SRC 
            ${QT_MOC_HEADER_SRC} 
            src/ControllerHandler.cpp
            src/JointHandler.cpp
            src/JoystickWidget.cpp
            src/SelectionDialog.cpp
            src/LightHandler.cpp
            src/MarsGui.cpp
            #src/MarsStyle.cpp
            src/MotorHandler.cpp
            src/NodeHandler.cpp
            src/NodeSelectionTree.cpp
            src/SensorHandler.cpp
            src/Widget_Joint_State.cpp 
            src/Widget_Node_Options_ODE.cpp 
            src/Widget_Node_State.cpp 

            src/dialogs/AboutDialog.cpp 
            src/dialogs/BlenderExportGUI.cpp
            src/dialogs/ControllerConfigGUI.cpp
            src/dialogs/Dialog_Add_Force.cpp 
            src/dialogs/Dialog_Add_Torque.cpp 
            src/dialogs/Dialog_Create_Material.cpp 
            src/dialogs/Dialog_Edit_Material.cpp
            src/dialogs/Dialog_Graphics_Options.cpp 
            src/dialogs/Dialog_Import_Mesh.cpp 
            src/dialogs/Dialog_Motor_Control.cpp 
            src/dialogs/Dialog_Rescale_Environment.cpp 
            src/dialogs/DialogControllers.cpp
            src/dialogs/DialogDistance.cpp
            src/dialogs/DialogJoints.cpp
            src/dialogs/DialogJoystick.cpp
            src/dialogs/DialogLights.cpp
            src/dialogs/DialogMotors.cpp
            src/dialogs/DialogNodes.cpp
            src/dialogs/DialogSensors.cpp

            src/dialogs/CameraConfig/CameraConfig.cpp
            src/dialogs/CameraConfig/CameraConfigurator.cpp
            src/dialogs/CameraConfig/CameraConfiguratorGUI.cpp

            src/dialogs/CaptureWindow/CaptureConfig.cpp
            src/dialogs/CaptureWindow/CaptureGUI.cpp
            src/dialogs/CaptureWindow/CaptureWindow.cpp
            src/dialogs/CaptureWindow/ImageProcess.cpp

            src/menus/MenuFile.cpp
            src/menus/MenuHelp.cpp
            src/menus/MenuWindow.cpp
            src/menus/MenuSimulation.cpp
)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${TARGET_SRC} ${MARSGuiResources})

IF (WIN32)
  set(WIN_LIBS -lwsock32 -lwinmm -lpthread)
#  SET_TARGET_PROPERTIES(mars PROPERTIES LINK_FLAGS -Wl,--stack,0x1000000)
ENDIF (WIN32)


IF(APPLE)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                        ${QT_LIBRARIES}
                        ${QT_QTXML_LIBRARY}
                        ${QT_QTOPENGL_LIBRARIES} 
                        ${PKGCONFIG_LIBRARIES}
                        ${WIN_LIBS}
                        ${OPENSCENEGRAPH_LIBRARIES}
                       -L/opt/local/lib opencv_core opencv_highgui
  )
ELSE(APPLE)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                        ${QT_LIBRARIES}
                        ${QT_QTXML_LIBRARY}
                        ${QT_QTOPENGL_LIBRARIES} 
                        ${PKGCONFIG_LIBRARIES} ${PKGCONFIG_LDFLAGS_OTHER}
                        ${WIN_LIBS}
                        ${OPENSCENEGRAPH_LIBRARIES}
  )
ENDIF(APPLE)

#add_subdirectory(src/dialogs)


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
install(FILES ${HEADERS} DESTINATION include/mars/gui)

# export cmake variables to c/c++
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)
include_directories("${CMAKE_BINARY_DIR}")

configure_file(configuration/${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

IF (WIN32)
  SET (POSTLIB ".dll")
  SET (PRELIB "lib")
  SET (LIBPATH "bin")
ELSE (WIN32)
  IF (APPLE)
    SET (POSTLIB ".dylib")
    SET (PRELIB "lib")
    SET (LIBPATH "lib")
  ELSE (APPLE)
    SET (POSTLIB ".so")
    SET (PRELIB "lib")
    SET (LIBPATH "lib")
  ENDIF (APPLE)
ENDIF (WIN32)


#CONFIGURE_FILE(configuration/dialog_config.in ${CMAKE_BINARY_DIR}/dialog_config.txt @ONLY)
#INSTALL(FILES ${CMAKE_BINARY_DIR}/dialog_config.txt DESTINATION configuration/mars-default/)
configure_file(configuration/mars_Gui.in ${CMAKE_BINARY_DIR}/mars_Gui.yaml @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/mars_Gui.yaml DESTINATION configuration/mars_default/)

INSTALL(DIRECTORY resources DESTINATION share/mars/gui)
