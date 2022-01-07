cmake_minimum_required(VERSION 3.21)
project(Database)

if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
	You should create a separete directory for build files.
")
endif()

#ADD_DEFINITIONS("-pedantic -Wno-unused -Wall -Wextra -Werror -Wstrict-prototypes -Wmissing-prototypes -O2")
ADD_DEFINITIONS("-Wall -g -DDEBUG")
# set(CTEST_TEST_TIMEOUT 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
#list(APPEND CMAKE_CTEST_ARGUMENTS "--timeout 10")

set(HEADER
	${HEADER}
	${CMAKE_CURRENT_SOURCE_DIR}/base.h)

add_subdirectory(pf)
add_subdirectory(rm)
add_subdirectory(ix)
add_subdirectory(sm)
add_subdirectory(ql)
add_subdirectory(ps)
include_directories(
        ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src
)

add_executable(database main.cpp)

target_link_libraries(database pf rm sm ix ql ps antlr4-cpp-runtime)

enable_testing()


