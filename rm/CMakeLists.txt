set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/rm.h
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_internal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_rid.h
)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_filehandle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_filescan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_record.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rm_rid.cpp
)

set(CTEST_OUTPUT_ON_FAILURE True)
add_library(rm ${HEADER} ${SOURCE})
add_executable(rm_test_1 rm_test1.cpp ${HEADER} ${SOURCE})
add_executable(rm_test_2 rm_test2.cpp ${HEADER} ${SOURCE})
add_executable(rm_test_3 rm_test3.cpp ${HEADER} ${SOURCE})
#add_executable(pf_TEST_3 test3.cpp ${HEADER} ${SOURCE})
target_link_libraries(rm_test_1 pf)
target_link_libraries(rm_test_2 pf)
target_link_libraries(rm_test_3 pf)

enable_testing()
add_test(NAME rm_test_1 COMMAND rm_test_1)
add_test(NAME rm_test_2 COMMAND rm_test_2)
add_test(NAME rm_test_3 COMMAND rm_test_3)
