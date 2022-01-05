set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/ix.h
)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/ix_handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ix_indexscan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ix_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
)


set(CTEST_OUTPUT_ON_FAILURE True)

add_library(ix ${HEADER} ${SOURCE})

add_executable(ix_test_1 test1.cpp ${HEADER} ${SOURCE})
add_executable(ix_test_2 test2.cpp ${HEADER} ${SOURCE})
add_executable(ix_test_3 test3.cpp ${HEADER} ${SOURCE})

target_link_libraries(ix_test_1 pf rm)
target_link_libraries(ix_test_2 pf rm)
target_link_libraries(ix_test_3 pf rm)

enable_testing()
add_test(NAME ix_test_1 COMMAND ix_test_1)
add_test(NAME ix_test_2 COMMAND ix_test_2)
add_test(NAME ix_test_3 COMMAND ix_test_3)
