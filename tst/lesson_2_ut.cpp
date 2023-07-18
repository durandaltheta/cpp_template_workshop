#include <type_traits>
#include <string>
#include <iostream>
#include "algorithm.hpp"
#include <gtest/gtest.h> 

namespace lesson_2_ns {

struct my_struct { };

/*
`std::is_lvalue_reference` is a utility struct defined by the standard library:
https://en.cppreference.com/w/cpp/types/is_lvalue_reference
 */
template <typename T>
bool is_lvalue(T&& t) {
    // call operator() to return the `bool` representation of the type `T` 
    // being an lvalue or not
    return std::is_lvalue_reference<T>();
}

template <typename T>
bool is_lvalue_pass_argument(T&& t) {
    return is_lvalue(t);
}


template <typename T>
bool is_lvalue_move(T&& t) {
    return is_lvalue(std::move(t));
}

template <typename T>
bool is_lvalue_forward(T&& t) {
    return is_lvalue(std::forward<T>(t));
}

};

TEST(lesson_2, lvalues_and_rvalues) {
    using namespace lesson_2_ns;

    int a_variable = 3;
    EXPECT_FALSE(is_lvalue(3));
    EXPECT_TRUE(is_lvalue(a_variable));
    EXPECT_TRUE(is_lvalue(static_cast<const int&>(a_variable)));
    
    auto ms = my_struct();
    EXPECT_FALSE(is_lvalue(my_struct()));
    EXPECT_TRUE(is_lvalue(ms));
    EXPECT_TRUE(is_lvalue(static_cast<const my_struct&>(ms)));

    int another_variable = std::move(a_variable);
    EXPECT_FALSE(is_lvalue(std::move(a_variable)));
    EXPECT_TRUE(is_lvalue(another_variable));
    EXPECT_TRUE(is_lvalue(static_cast<const int&>(another_variable)));
}

TEST(lesson_2, moving_vs_forwarding) {
    using namespace lesson_2_ns;

    int i = 3;
    EXPECT_TRUE(is_lvalue_pass_argument(i));
    EXPECT_TRUE(is_lvalue_pass_argument(std::move(i)));
    EXPECT_FALSE(is_lvalue_move(i));
    EXPECT_FALSE(is_lvalue_move(std::move(i)));
    EXPECT_TRUE(is_lvalue_forward(i));
    EXPECT_FALSE(is_lvalue_forward(std::move(i)));
}

TEST(lesson_2, type_decay) {
    using namespace lesson_2_ns;

    int i = 3;
    const int& ref_i = i;
    {
        bool val = std::is_same<int,typeof(ref_i)>::value;
        EXPECT_FALSE(val);
    }

    {
        bool val = std::is_same<int,std::decay<typeof(ref_i)>::type>::value;
        EXPECT_TRUE(val);
    }
}
