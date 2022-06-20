project(mars_smurf_loader)
set(PROJECT_VERSION 1.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 2.6)

#set(CMAKE_BUILD_TYPE Debug)

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)

find_package(lib_manager)
lib_defaults()
define_module_info()

#find_package(Doxygen)
#if(DOXYGEN_FOUND)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
#add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
#endif(DOXYGEN_FOUND)

find_package(Boost REQUIRED system thread)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

setup_qt()

#Get linker and compiler flags from pkg-config
pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    minizip
			    configmaps
			    mars_interfaces
			    urdfdom
			    tinyxml
			    mars_entity_factory
			    mars_sim
)

include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

set(ADD_INCLUDES "-I${QT_INCLUDE_DIR} -I${QT_QTXML_INCLUDE_DIR}")

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set(SOURCES_H
       src/SMURFLoader.h
       src/zipit.h
    )

set(TARGET_SRC ${SOURCES_H_MOC}
       src/SMURFLoader.cpp
       src/zipit.cpp
)

add_library(${PROJECT_NAME} SHARED ${TARGET_SRC})

if (${USE_QT5})
#qt5_use_modules(${PROJECT_NAME} Xml)
find_package(Qt5Xml)
include_directories(${PROJECT_NAME} ${Qt5XML_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Qt5::Xml)
endif (${USE_QT5})

set(_INSTALL_DESTINATIONS
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION lib
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            ${QT_LIBRARIES}
            ${QT_QTXML_LIBRARY}
            ${PKGCONFIG_LIBRARIES}
            ${WIN_LIBS}
            z
)


#------------------------------------------------------------------------------

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
install(FILES ${SOURCES_H} DESTINATION include/mars/smurf_loader)

configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
