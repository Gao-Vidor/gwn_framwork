cmake_minimum_required(VERSION 3.0)
project(gwn)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(..)
include_directories(/apps/gwn/include)
link_directories(/apps/gwn/lib/libyaml-cpp)

set(LIB_SRC 
    gwn/log.cc
    gwn/util.cc
    gwn/config.cc
    gwn/thread.cc
    gwn/mutex.cc
    gwn/util.cc
    gwn/fiber.cc
    gwn/scheduler.cc
    )

find_library(PTHREAD pthread)

set(LIBS
    gwn
    yaml-cpp
    pthread
    )

add_library(gwn SHARED ${LIB_SRC})

add_executable(config_test tests/config_test.cc)
add_dependencies(config_test gwn)
#target_link_libraries(config_test gwn -L/apps/gwn/lib/libyaml-cpp -lyaml-cpp)
target_link_libraries(config_test gwn ${LIBS})

add_executable(thread_test tests/thread_test.cc)
add_dependencies(thread_test gwn)
target_link_libraries(thread_test gwn ${LIBS})


add_executable(util_test tests/util_test.cc)
add_dependencies(util_test gwn)
target_link_libraries(util_test gwn ${LIBS})

add_executable(fiber_test tests/fiber_test.cc)
add_dependencies(fiber_test gwn)
target_link_libraries(fiber_test gwn ${LIBS})

add_executable(schedul_test tests/schedul_test.cc)
add_dependencies(schedul_test gwn)
target_link_libraries(schedul_test gwn ${LIBS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

