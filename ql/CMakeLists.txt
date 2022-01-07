set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/ql.h
)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/ql_manager.cpp
)

add_library(ql ${HEADER} ${SOURCE})
link_libraries(pf rm ix sm)
set(CTEST_OUTPUT_ON_FAILURE True)
#add_executable(SM_TEST_1 sm_test1.cpp ${HEADER} ${SOURCE})
#add_executable(SM_TEST_2 sm_test2.cpp ${HEADER} ${SOURCE})
#target_link_libraries(SM_TEST_1)
#target_link_libraries(pf_TEST_2 SP)
#enable_testing()
#add_test(NAME sm_test_1 COMMAND SM_TEST_1)
#add_test(NAME sm_test_2 COMMAND SM_TEST_2)
