cmake_minimum_required(VERSION 2.8)

project(osrmint)

set (osrmint_VERSION_MAJOR 0)
set (osrmint_VERSION_MINOR 1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#c++11
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11")
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE)
  MESSAGE(FATAL_ERROR "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory
`CMakeFiles'. Please delete them.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CheckCXXCompilerFlag)
include(FindPackageHandleStandardArgs)
include(FindPkgConfig)

# Packages
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Git
include(GetGitRevisionDescription)
git_describe(GIT_DESCRIPTION)

set(bitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(bitness 64)
  MESSAGE(STATUS "Building on a 64 bit system")
else()
  MESSAGE(WARNING "Building on a 32 bit system is unsupported")
endif()

### Find
## Boost
set(BOOST_COMPONENTS date_time filesystem iostreams program_options
regex system thread unit_test_framework)
#Check Boost
set(BOOST_MIN_VERSION "1.49.0")
find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
if(NOT Boost_FOUND)
  MESSAGE(FATAL_ERROR "Fatal error: Boost (version >= 1.49.0) required.\n")
endif()
##

## PostgreSQL
set(PostgreSQL_ADDITIONAL_VERSIONS "9.4")
set(PostgreSQL_ADDITIONAL_SEARCH_PATHS "/usr/include/postgresql/9.4/server")
find_package(PostgreSQL REQUIRED)
MESSAGE(STATUS "PostgreSQL_LIBRARIES: " ${PostgreSQL_LIBRARIES})
##

## Threads
find_package(Threads REQUIRED)
##

## OpenMP
#find_package(OpenMP REQUIRED)
#if ( OPENMP_FOUND )
#  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#  MESSAGE(STATUS "**OpenMP found")
#else()
#  MESSAGE(STATUS "**OpenMP NOT found")
#endif()
##

## rapidjson
find_package(rapidjson REQUIRED)
##

## CURL
find_package(CURL REQUIRED)
##

## CURLPP via PKG
if (UNIX)
  PKG_CHECK_MODULES(CURLPP curlpp)
  if (CURLPP_FOUND EQUAL 1)
    MESSAGE(STATUS "curlpp found" )
  else()
    MESSAGE(FATAL_ERROR "curlpp library NOT found (required)!")
  endif()
endif(UNIX)
###

## OSRM
# Office
#set(OSRM_SOURCE_PATH "/disco_local/fpacheco/im/osrm-backend")
# Home
#set(OSRM_SOURCE_PATH "/home/fpacheco/workspace/idm/osrm-backend")
#set(OSRM_LIBRARIES ${OSRM_SOURCE_PATH}/build/libOSRM.a)
##

### Includes
include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CURLPP_INCLUDE_DIRS})
include_directories(${RAPIDJSON_INCLUDE_DIRS})
include_directories(${OSRM_SOURCE_PATH}/Include ${OSRM_SOURCE_PATH}/third_party ${OSRM_SOURCE_PATH})
###

### CMAKE_CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RAPIDJSON_CXX_FLAGS}")
###

## cmake automatico GCC para C y GXX para cpp
## aux_source_directory(./src SRC_LIST)

### PostgreSQL module (shared library)
set(OSRMINT_SOURCES
    "./src/osrmintutils.c"
    "./src/osrmintroute.c"
    "./src/osrmintviaroute.c"
    "./src/osrmcurlpp.cpp"
    "./src/osrmint_wrapper.cpp"
    "./src/osrmint.c"
)


add_library(${PROJECT_NAME} SHARED ${OSRMINT_SOURCES})
# Sin prefijo lib!!
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
# Sin prefijo lib!!
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX "-${osrmint_VERSION_MAJOR}.${osrmint_VERSION_MINOR}.so")
# Se linkea con C!!
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
# Se agregan estas bibliotecas!!
target_link_libraries(${PROJECT_NAME} ${CURLPP_LIBRARIES})
###

### Test
#set(TESTS_SOURCE_FILES "./tests/ptest.cpp" "./src/osrmroute.cpp")
#add_executable(ptest ${TESTS_SOURCE_FILES})
#include_directories("${CMAKE_SOURCE_DIR}/src")
#target_link_libraries(ptest ${Boost_LIBRARIES} ${Threads_LIBRARIES} ${OSRM_LIBRARIES})

### Tests
set(TCPIP_SOURCE_FILES "./tests/tcpip.cpp")
add_executable(tcpip ${TCPIP_SOURCE_FILES})
include_directories("${CMAKE_SOURCE_DIR}/src" ${CURL_INCLUDE_DIRS} )
target_link_libraries(tcpip ${CURL_LIBRARIES} "curlpp")

### Tests
set(TEST1_SOURCE_FILES "./tests/test1.cpp" "./src/osrmcurlpp.cpp")
add_executable(test1 ${TEST1_SOURCE_FILES})
include_directories("${CMAKE_SOURCE_DIR}/src" ${CURL_INCLUDE_DIRS} )
target_link_libraries(test1 ${CURL_LIBRARIES} "curlpp")

set(TESTVR_SOURCE_FILES "./tests/testvr.cpp" "./src/osrmcurlpp.cpp")
add_executable(testvr ${TESTVR_SOURCE_FILES})
include_directories("${CMAKE_SOURCE_DIR}/src" ${CURL_INCLUDE_DIRS} )
target_link_libraries(testvr ${CURL_LIBRARIES} "curlpp")
