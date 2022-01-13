set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/MyVisitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLBaseVisitor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLLexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLVisitor.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLBaseVisitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLLexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SQLVisitor.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src
)
file(GLOB antlr4-cpp-src
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/atn/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/dfa/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/misc/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/support/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/tree/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/tree/pattern/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr4-cpp-runtime-4.9.3-source/runtime/src/tree/xpath/*.cpp
)
add_library (antlr4-cpp-runtime ${antlr4-cpp-src})
add_library(ps ${HEADER} ${SOURCE})
