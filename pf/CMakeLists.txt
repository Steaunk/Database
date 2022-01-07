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

add_executable(pf_test_1 pf_test1.cpp ${HEADER} ${SOURCE})
add_executable(pf_test_2 pf_test2.cpp ${HEADER} ${SOURCE})
add_executable(pf_test_3 pf_test3.cpp ${HEADER} ${SOURCE})

enable_testing()

add_test(NAME pf_test_1 COMMAND pf_test_1)
add_test(NAME pf_test_2 COMMAND pf_test_2)
add_test(NAME pf_test_3 COMMAND pf_test_3)
