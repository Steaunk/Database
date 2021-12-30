set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/sm.h
)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/sm_manager.cpp
)

add_library(sm ${HEADER} ${SOURCE})
#add_executable(pf_test test1.cpp ${HEADER} ${SOURCE})
#add_executable(pf_TEST_2 test2.cpp ${HEADER} ${SOURCE})
#add_executable(pf_TEST_3 test3.cpp ${HEADER} ${SOURCE})
#target_link_libraries(pf_TEST_1 SP)
#target_link_libraries(pf_TEST_2 SP)
#target_link_libraries(pf_TEST_3 SP)
#target_link_libraries(pf_TEST_4 SP)
