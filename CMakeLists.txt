cmake_minimum_required(VERSION 3.21)
project(Database)

ADD_DEFINITIONS("-Wall -Werror -Wstrict-prototypes -Wmissing-prototypes")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(HEADER
	${HEADER}
	${CMAKE_CURRENT_SOURCE_DIR}/base.h)

add_subdirectory(pf)
add_subdirectory(rm)

add_executable(database main.cpp)

target_link_libraries(database pf rm)


