cmake_minimum_required(VERSION 3.12.4)

if($ENV{TRAVIS_BUILD_NUMBER})
  set(VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
  set(VERSION 0.0.0)
endif()
project(bulkmt VERSION ${VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### Dependencies ##############################################################

set(SRC_PATH "src/")
set(TEST_PATH "test/")
include_directories("inc/")

set(GTEST_SRCDIR "/usr/src/gtest/")
set(GTEST_FOUND Yes)
if(NOT EXISTS "${GTEST_SRCDIR}/CMakeLists.txt")
  set(GTEST_SRCDIR "/usr/src/googletest/")
endif()
if(NOT EXISTS "${GTEST_SRCDIR}/CMakeLists.txt")
  set(GTEST_FOUND No)
endif()

find_package(Threads REQUIRED)

### Main targets ##############################################################

add_executable(${CMAKE_PROJECT_NAME} ${SRC_PATH}/main.cpp)
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  ${CMAKE_THREAD_LIBS_INIT})

### Tests #####################################################################

if(GTEST_FOUND)
  add_subdirectory(${GTEST_SRCDIR}/ ${CMAKE_BINARY_DIR}/gtest/ EXCLUDE_FROM_ALL)

  add_executable(test_suite ${TEST_PATH}/test.cpp)
  target_link_libraries(
    test_suite
    PRIVATE
    gtest
    gtest_main)
  add_test(test_suite test_suite)
else()
  message(
    WARNING
    "Test suite will not be launched because GTest sources is not found at "
    "${GTEST_SRCDIR}.")
endif()

enable_testing()

### Packing ###################################################################

install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_CONTACT bergentroll@insiberia.net)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
