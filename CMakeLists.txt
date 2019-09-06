cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED ENV{TRAVIS_BUILD_NUMBER})
  set(Boost_ARCHITECTURE "-x32")
  set(ENV{TRAVIS_BUILD_NUMBER} "1")
endif()

project(bulkmt VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Threads)

add_executable(bulkmt main.cpp)
add_executable(unit_tests unit_tests.cpp)

set_target_properties(bulkmt unit_tests PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON 
  COMPILE_OPTIONS "-Wall;-Wpedantic;-Wextra"
  LINK_FLAGS "-lpthread"
)

set_target_properties(unit_tests bulkmt PROPERTIES
  COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
  INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_link_libraries(unit_tests 
  ${Boost_LIBRARIES}
  ${CMAKE_THREADS_LIB_INIT}
)

target_link_libraries(bulkmt 
  ${Boost_LIBRARIES}
  ${CMAKE_THREADS_LIB_INIT}
)

install(TARGETS bulkmt RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT lonfort@mail.ru)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

enable_testing()

add_test(NAME bulkmt_test COMMAND unit_tests)
