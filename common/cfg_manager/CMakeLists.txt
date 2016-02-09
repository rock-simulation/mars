project(cfg_manager)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "A small library to read, write, and share software parameter using yaml configuration files.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

find_package(lib_manager)
lib_defaults()
define_module_info()

find_package(Rock)
rock_find_cmake(Boost)

pkg_check_modules(YAML REQUIRED
        yaml-cpp
)
include_directories(${YAML_INCLUDE_DIRS})
link_directories(${YAML_LIBRARY_DIRS})
add_definitions(${YAML_CFLAGS_OTHER})  #flags excluding the ones with -I

if(${YAML_VERSION} EQUAL "0.3.0")
  add_definitions(-DYAML_03_API)
  message("       WARNING: The use of yaml 0.3 is deprecated!")
endif()

pkg_check_modules(DEPS REQUIRED
	lib_manager
	mars_utils
)
include_directories(${DEPS_INCLUDE_DIRS})
link_directories(${DEPS_LIBRARY_DIRS})
add_definitions(${DEPS_CLFAGS_OTHER})  #flags excluding the ones with -I

include_directories(
	src
)


set(SOURCES 
	src/CFGManager.cpp
	src/CFGParam.cpp
	src/CFGParamBool.cpp
	src/CFGParamDouble.cpp
	src/CFGParamInt.cpp
	src/CFGParamString.cpp
	src/CFGProperty.cpp
)

set(HEADERS
	src/CFGClient.h
	src/CFGDefs.h
	src/CFGManager.h
	src/CFGManagerInterface.h
	src/CFGParam.h
	src/CFGProperty.h
)



add_library(${PROJECT_NAME} SHARED ${SOURCES})


MESSAGE("LIBS ${DEPS_LIBRARIES}")

target_link_libraries(${PROJECT_NAME}
                      ${DEPS_LIBRARIES}
                      ${YAML_LIBRARIES}
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


