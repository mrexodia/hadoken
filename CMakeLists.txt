project(hadoken)
cmake_minimum_required(VERSION 2.8)

enable_testing()

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")

option(UNIT_TESTS "Enable or disable unit tests execution" TRUE)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)


file(GLOB HADOKEN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/hadoken/*)
file(GLOB HADOKEN_HEADERS_1 ${CMAKE_CURRENT_SOURCE_DIR}/include/hadoken/*/*)


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${BOOST_INCLUDE_DIR} )

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/hadoken DESTINATION ${INCLUDE_INSTALL_DIR}/)


add_subdirectory(tests/unit)
