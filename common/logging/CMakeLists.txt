cmake_minimum_required(VERSION 2.6)

project(mars_logging)
set(PROJECT_VERSION 1.0)

include(FindPkgConfig)
include(${CMAKE_INSTALL_PREFIX}/cmake/mars.cmake)

define_module_info()

if(WIN32)
else()
    set(LIB_DEPS "base-lib")
endif()


set(HEADERS
    src/Logging.hpp
)

# Install headers into mars include directory
install(FILES ${HEADERS} DESTINATION include/mars/logging)

configure_file(${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)




