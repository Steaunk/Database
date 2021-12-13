cmake_minimum_required(VERSION 3.21)
project(Database)

#set(CMAKE_CXX_FLAGS_DEBUG_INIT "-DDEBUG")

#ADD_DEFINITIONS("-pedantic -Wno-unused -Wall -Wextra -Werror -Wstrict-prototypes -Wmissing-prototypes -O2")
ADD_DEFINITIONS("-Wall -Wstrict-prototypes -Wmissing-prototypes -O2")
set(CTEST_TEST_TIMEOUT 1)
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

add_executable(database main.cpp)

target_link_libraries(database pf rm ix)

enable_testing()


