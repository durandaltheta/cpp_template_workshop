#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include <functional>
#include "scalgorithm.hpp"
#include <gtest/gtest.h> 

namespace lesson_4_ns {

int function_1() {
    return 1;
}

struct Functor_1 { 
    int operator()() {
        return 2;
    }
};

template <typename F>
auto
execute_callable(F&& f) {
    return f();
}

};

TEST(lesson_4, callable) {
    using namespace lesson_4_ns;

    int (*function_ptr_1)() = function_1;
    auto lambda_1 = []{ return 3; };
    std::function<int()> wrapper_1(lambda_1);

    EXPECT_EQ(1, function_1());
    EXPECT_EQ(1, function_ptr_1());
    EXPECT_EQ(2, Functor_1()());
    EXPECT_EQ(3, lambda_1());
    EXPECT_EQ(3, wrapper_1());

    EXPECT_EQ(1, execute_callable(function_1));
    EXPECT_EQ(1, execute_callable(function_ptr_1));
    EXPECT_EQ(2, execute_callable(Functor_1()));
    EXPECT_EQ(3, execute_callable(lambda_1));
    EXPECT_EQ(3, execute_callable(wrapper_1));
}

namespace lesson_4_ns {

int function_2(int i) {
    return i + 1;
}

struct Functor_2 { 
    std::string operator()(std::string s) {
        return std::string("Functor_2") + s;
    }
};

template <typename F, typename T>
auto
execute_unary_callable(F&& f, T&& t) {
    return f(std::forward<T>(t));
}

};

TEST(lesson_4, callable_with_argument) {
    using namespace lesson_4_ns;

    int (*function_ptr_2)(int) = function_2;
    auto lambda_2 = [](int i){ return i + 3; };
    std::function<int(int)> wrapper_2(lambda_2);

    EXPECT_EQ(2, function_2(1));
    EXPECT_EQ(3, function_ptr_2(2));
    EXPECT_EQ(std::string("Functor_2 hello"), Functor_2()(std::string(" hello")));
    EXPECT_EQ(5, lambda_2(2));
    EXPECT_EQ(5, wrapper_2(2));

    EXPECT_EQ(2, execute_unary_callable(function_2,1));
    EXPECT_EQ(3, execute_unary_callable(function_ptr_2,2));
    EXPECT_EQ(std::string("Functor_2 hello"), execute_unary_callable(Functor_2(),std::string(" hello")));
    EXPECT_EQ(5, execute_unary_callable(lambda_2,2));
    EXPECT_EQ(5, execute_unary_callable(wrapper_2,2));
}
