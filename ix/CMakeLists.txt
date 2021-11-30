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

add_library(rm ${HEADER} ${SOURCE})
#add_executable(pf_test test1.cpp ${HEADER} ${SOURCE})
#add_executable(pf_TEST_2 test2.cpp ${HEADER} ${SOURCE})
#add_executable(pf_TEST_3 test3.cpp ${HEADER} ${SOURCE})
#target_link_libraries(pf_TEST_1 SP)
#target_link_libraries(pf_TEST_2 SP)
#target_link_libraries(pf_TEST_3 SP)
#target_link_libraries(pf_TEST_4 SP)
