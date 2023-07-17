project(cpp_template_workshop_ut)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug) 
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O3 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

add_subdirectory(googletest)

add_executable(cpp_template_workshop_ut
    lesson_1_ut.cpp 
    lesson_3_ut.cpp 
)

target_compile_features(cpp_template_workshop_ut PRIVATE cxx_std_17)
target_include_directories(cpp_template_workshop_ut INTERFACE inc)

target_link_libraries(cpp_template_workshop_ut
    gtest 
    gtest_main
    sca
)
