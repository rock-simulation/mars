project(mars_app)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This is ...")
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

pkg_check_modules(PKGCONFIG
        lib_manager
        main_gui
        mars_interfaces
)
include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CLFAGS_OTHER})  #flags excluding the ones with -I


find_package(Qt5Widgets)
if (${Qt5Widgets_FOUND})
  set(USE_QT5)
  find_package(Qt5Widgets REQUIRED)
  include_directories(${Qt5Widgets_INCLUDES})
  add_definitions(-DUSE_QT5)
  add_definitions(${Qt5Widgets_DEFINITIONS})
  set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
else (${Qt5Widgets_FOUND})
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  include_directories(${QT_INCLUDE_DIR})
endif (${Qt5Widgets_FOUND})

set(SOURCES
    src/GraphicsTimer.cpp
    src/MARS.cpp
)

set(QT_MOC_HEADER
    src/GraphicsTimer.h
    #src/MARS.h
)

if (${USE_QT5})
qt5_wrap_cpp ( QT_MOC_HEADER_SRC ${QT_MOC_HEADER} )
else (${USE_QT5})
qt4_wrap_cpp ( QT_MOC_HEADER_SRC ${QT_MOC_HEADER} )
endif (${USE_QT5})

set(TARGET_SRC
    ${QT_MOC_HEADER_SRC}
    ${SOURCES}
)

IF( MINGW )
  # resource compilation for MinGW
  ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mars_icon.o
                      COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.rc
                             -o ${CMAKE_CURRENT_BINARY_DIR}/mars_icon.o )
  SET(TARGET_SRC ${TARGET_SRC} ${CMAKE_CURRENT_BINARY_DIR}/mars_icon.o)
#ELSE( MINGW )
#  SET(TARGET_SRC ${TARGET_SRC} resources/mars_icon.ico)
ENDIF( MINGW )

add_executable(${PROJECT_NAME} src/main.cpp ${TARGET_SRC})
add_library(mars SHARED ${TARGET_SRC})

if (${USE_QT5})
qt5_use_modules(${PROJECT_NAME} Widgets)
qt5_use_modules(mars Widgets)
endif (${USE_QT5})

IF (WIN32)
  set(WIN_LIBS
        -lwinmm
  )
  SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS -Wl,--stack,0x1000000)
ENDIF (WIN32)

target_link_libraries(mars
            ${QT_LIBRARIES}
            ${WIN_LIBS}
            ${PKGCONFIG_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} mars)

INSTALL(TARGETS ${PROJECT_NAME} mars
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

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

set(ADDITIONAL_LIBS "" CACHE INTERNAL "")

function(add_plugin_if_availible LIBNAME)
  find_library(${LIBNAME}_VAR "${LIBNAME}" HINTS ${CMAKE_INSTALL_PREFIX}/${LIBPATH})
  if(${LIBNAME}_VAR)
    MESSAGE("Found ${LIBNAME} Plugin: ${${LIBNAME}_VAR}")
    set(ADDITIONAL_LIBS "${ADDITIONAL_LIBS}${${LIBNAME}_VAR} " CACHE INTERNAL "")
  else()
    MESSAGE("Not found: ${LIBNAME}")
  endif()
endfunction()


add_plugin_if_availible("connexion_plugin")
add_plugin_if_availible("data_broker_gui")
add_plugin_if_availible("cfg_manager_gui")
add_plugin_if_availible("lib_manager_gui")


if(ADDITIONAL_LIBS)
string(REGEX REPLACE " " "
" ADDITIONAL_LIBS ${ADDITIONAL_LIBS})
endif()

configure_file(configuration/core_libs.in ${CMAKE_BINARY_DIR}/core_libs.txt @ONLY)
configure_file(configuration/other_libs.in ${CMAKE_BINARY_DIR}/other_libs.txt @ONLY)
configure_file(configuration/core_libs-nogui.in ${CMAKE_BINARY_DIR}/core_libs-nogui.txt @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/core_libs.txt ${CMAKE_BINARY_DIR}/other_libs.txt ${CMAKE_BINARY_DIR}/core_libs-nogui.txt DESTINATION configuration/mars_default/)

configure_file(mars_app.pc.in ${CMAKE_BINARY_DIR}/mars_app.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/mars_app.pc DESTINATION lib/pkgconfig/)
install(FILES ${CMAKE_SOURCE_DIR}/src/GraphicsTimer.h DESTINATION include/mars/app/)
install(FILES ${CMAKE_SOURCE_DIR}/src/MARS.h DESTINATION include/mars/app/)
install(FILES ${CMAKE_SOURCE_DIR}/src/MyApp.h DESTINATION include/mars/app/)
