project(constraints_plugin)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "Plugin to create constraints between Nodes.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

find_package(lib_manager)
lib_defaults()
define_module_info()

pkg_check_modules(PKGCONFIG REQUIRED
        lib_manager
        cfg_manager
        main_gui
        mars_interfaces
        configmaps
)

include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

setup_qt()

include_directories(
    src
)

set(SOURCES
    src/ConstraintsPlugin.cpp
    src/NodeConstraint.cpp
)

set(HEADERS
    src/ConstraintsPlugin.h
    src/BaseConstraint.h
    src/NodeConstraint.h
)


add_library(${PROJECT_NAME} SHARED ${SOURCES})


if (${USE_QT5})
  qt5_use_modules(${PROJECT_NAME} Widgets)
endif (${USE_QT5})

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
install(FILES ${HEADERS} DESTINATION include/mars/plugins/${PROJECT_NAME})

# Prepare and install necessary files to support finding of the library 
# using pkg-config
configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)


