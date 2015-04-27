project(mars-cmake)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This will install cmake extensions for mars")
cmake_minimum_required(VERSION 2.6)

install(FILES mars.cmake DESTINATION share/mars/cmake)
