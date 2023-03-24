cmake_minimum_required(VERSION 3.0)

project(libcpp_template_workshop)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug) 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CPP_TEMPLATE_WORKSHOP_DETAIL_HEADER_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/inc/detail/template.hpp
)

set(CPP_TEMPLATE_WORKSHOP_HEADER_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/inc/template.hpp
)

add_library(cpp_template INTERFACE)
target_include_directories(cpp_template_workshop INTERFACE inc)

install(TARGETS cpp_template_workshop DESTINATION lib)
install(FILES ${CPP_TEMPLATE_WORKSHOP_HEADER_FILES} DESTINATION include/cpp_template_workshop)

install(FILES ${CPP_TEMPLATE_WORKSHOP_DETAIL_HEADER_FILES} DESTINATION include/cpp_template_workshop/detail) 

add_subdirectory(tst EXCLUDE_FROM_ALL)
