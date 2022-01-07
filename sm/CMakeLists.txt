set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/sm.h
        ${CMAKE_CURRENT_SOURCE_DIR}/sm_internal.h
<<<<<<< HEAD

=======
>>>>>>> de089d84b74d92177a924a3dc54bdc3d52a4299b
)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/sm_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sm_error.cpp
)

add_library(sm ${HEADER} ${SOURCE})
link_libraries(pf rm ix)
set(CTEST_OUTPUT_ON_FAILURE True)
add_executable(SM_TEST_1 sm_test1.cpp ${HEADER} ${SOURCE})
add_executable(SM_TEST_2 sm_test2.cpp ${HEADER} ${SOURCE})
target_link_libraries(SM_TEST_1 pf rm ix)
target_link_libraries(SM_TEST_2 pf rm ix)
enable_testing()
add_test(NAME sm_test_1 COMMAND SM_TEST_1)
add_test(NAME sm_test_2 COMMAND SM_TEST_2)
