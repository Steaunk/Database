set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_buffermgr.h
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_hashtable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_internal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/pf.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_buffermgr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_error.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_filehandle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_hashtable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pf_pagehandle.cpp)
add_library(pf ${HEADER} ${SOURCE})
#add_executable(pf_test test1.cpp ${HEADER} ${SOURCE})
#add_executable(pf_TEST_2 test2.cpp ${HEADER} ${SOURCE})
#add_executable(pf_TEST_3 test3.cpp ${HEADER} ${SOURCE})
#target_link_libraries(pf_TEST_1 SP)
#target_link_libraries(pf_TEST_2 SP)
#target_link_libraries(pf_TEST_3 SP)
#target_link_libraries(pf_TEST_4 SP)
