set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/sm.h
)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/sm_manager.cpp
)

add_library(sm ${HEADER} ${SOURCE})
set(CTEST_OUTPUT_ON_FAILURE True)
add_executable(SM_TEST_1 sm_test1.cpp ${HEADER} ${SOURCE})
#target_link_libraries(SM_TEST_1)
#target_link_libraries(pf_TEST_2 SP)
#target_link_libraries(pf_TEST_3 SP)
#target_link_libraries(pf_TEST_4 SP)
enable_testing()
add_test(NAME sm_test_1 COMMAND SM_TEST_1)
