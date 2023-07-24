cmake_minimum_required(VERSION 3.0)

project(libsca)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_BUILD_TYPE Release) 
set(CMAKE_BUILD_TYPE Debug) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_HEADER_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/inc/detail/algorithm.hpp
)

set(SIMPLE_CPLUSPLUS_ALGORITHM_HEADER_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/inc/scalgorithm.hpp
)

add_library(sca INTERFACE)
target_include_directories(sca INTERFACE inc)

install(TARGETS sca DESTINATION lib)
install(FILES ${SIMPLE_CPLUSPLUS_ALGORITHM_HEADER_FILES} DESTINATION include/sca)
install(FILES ${SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_HEADER_FILES} DESTINATION include/sca/detail) 

add_subdirectory(tst EXCLUDE_FROM_ALL)
