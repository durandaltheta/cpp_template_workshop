cmake_minimum_required(VERSION 3.0)

project(libcpp_stl_algorithm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CPP_STL_ALGORITHM_DETAIL_HEADER_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/inc/detail/template.hpp
)

set(CPP_STL_ALGORITHM_HEADER_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/inc/template.hpp
)

add_library(cpp_template INTERFACE)
target_include_directories(cpp_stl_algorithm INTERFACE inc)

install(TARGETS cpp_stl_algorithm DESTINATION lib)
install(FILES ${CPP_STL_ALGORITHM_HEADER_FILES} DESTINATION include/cpp_stl_algorithm)

install(FILES ${CPP_STL_ALGORITHM_DETAIL_HEADER_FILES} DESTINATION include/cpp_stl_algorithm/detail) 

add_subdirectory(tst EXCLUDE_FROM_ALL)
