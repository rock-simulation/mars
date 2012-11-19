project(mars_scene_loader)
set(PROJECT_VERSION 1.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 2.6)

#set(CMAKE_BUILD_TYPE Debug)

execute_process(COMMAND git rev-parse HEAD 
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE res
                OUTPUT_VARIABLE git_hash
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git diff-index --quiet HEAD 
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE git_has_local_changes
                OUTPUT_VARIABLE out
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if(git_has_local_changes)
  message(WARNING "git repository has local uncommitted changes!")
  set(git_version "${git_hash} (with local changes)")
else(git_has_local_changes)
  set(git_version ${git_hash})
endif(git_has_local_changes)
add_definitions("-DGIT_INFO=${git_version}")

if(WIN32)
        # this fixes the error 998 from the LibManager
        SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import")
        SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--enable-auto-import")
else(WIN32)
        SET(CMAKE_CXX_FLAGS "-fPIC")
endif(WIN32)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

cmake_minimum_required(VERSION 2.6)
include(FindPkgConfig)

#find_package(Doxygen)
#if(DOXYGEN_FOUND)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
#add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
#endif(DOXYGEN_FOUND)

find_package(Qt4 REQUIRED)
set(QT_USE_QTOPENGL TRUE)
include(${QT_USE_FILE})
INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})

#Get linker and compiler flags from pkg-config
pkg_check_modules(PKGCONFIG REQUIRED
			    lib_manager
			    minizip
			    mars_utils
			    mars_interfaces)

include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #flags excluding the ones with -I

set(ADD_INCLUDES "-I${QT_INCLUDE_DIR} -I${QT_QTXML_INCLUDE_DIR}")

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set(SOURCES_H
       src/Load.h
       src/SceneLoader.h
       src/Save.h
       src/SaveLoadStructs.h
       src/zipit.h
    )

set(TARGET_SRC ${SOURCES_H_MOC}
       src/SceneLoader.cpp
       src/Load.cpp
       src/Save.cpp
       src/zipit.cpp
)

add_library(${PROJECT_NAME} SHARED ${TARGET_SRC})

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
install(FILES ${SOURCES_H} DESTINATION include/mars/scene_loader)

configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
