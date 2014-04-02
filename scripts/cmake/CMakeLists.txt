project(mars-cmake)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This will install cmake extensions for mars")
cmake_minimum_required(VERSION 2.6)

configure_file(mars-config.cmake.in ${CMAKE_BINARY_DIR}/mars-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/mars-config.cmake DESTINATION share/mars/cmake)
install(FILES mars.cmake DESTINATION share/mars/cmake)
