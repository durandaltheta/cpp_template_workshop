#include <type_traits>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
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
        bool val = std::is_same<int,decltype(ref_i)>::value;
        EXPECT_FALSE(val);
    }

    {
        bool val = std::is_same<int,std::decay<decltype(ref_i)>::type>::value;
        EXPECT_TRUE(val);
    }
}

TEST(lesson_2, to) {
    std::list<int> l{1,2,3};
    std::vector<double> v{1.0,2.0,3.0};
    std::forward_list<std::string> fl{"hello"," world"};

    {
        auto out = sca::to<std::vector<int>>(l);
        auto val = std::is_same<std::vector<int>,decltype(out)>::value;
        EXPECT_TRUE(val);
    }

    {
        auto out = sca::to<std::list<double>>(v);
        auto val = std::is_same<std::list<double>,decltype(out)>::value;
        EXPECT_TRUE(val);
    }

    {
        auto out = sca::to<std::list<std::string>>(fl);
        auto val = std::is_same<std::list<std::string>,decltype(out)>::value;
        EXPECT_TRUE(val);
    }
}

TEST(lesson_2, reverse) {
    std::forward_list<int> fl{1,2,3};
    std::list<std::string> l{"hello", " my", " ragtime", " gal"};

    {
        auto out = sca::reverse(fl);
        auto val = std::is_same<std::vector<int>,decltype(out)>::value;
        EXPECT_TRUE(val);
        EXPECT_EQ(3, out[0]);
        EXPECT_EQ(2, out[1]);
        EXPECT_EQ(1, out[2]);
    }

    {
        auto out = sca::reverse(l);
        auto val = std::is_same<std::vector<std::string>,decltype(out)>::value;
        EXPECT_TRUE(val);
        EXPECT_EQ(std::string(" gal"), out[0]);
        EXPECT_EQ(std::string(" ragtime"), out[1]);
        EXPECT_EQ(std::string(" my"), out[2]);
        EXPECT_EQ(std::string("hello"), out[3]);
    }
}

TEST(lesson_2, group) {
    {
        std::vector<int> v{1,2,};
        std::list<int> l{3,4};
        std::forward_list<int> fl{5,6};

        auto out = sca::group(l, fl, v);
        auto val = std::is_same<std::vector<int>,decltype(out)>::value;
        EXPECT_TRUE(val);
        auto cmp = std::vector<int>{3,4,5,6,1,2};
        EXPECT_EQ(cmp, out);
    }

    {
        std::list<std::string> l{"hello"," my"};
        std::forward_list<std::string> fl{" name", " is"};
        std::vector<std::string> v{" foo","faa"};

        auto out = sca::group(l, fl, v);
        auto val = std::is_same<std::vector<std::string>,decltype(out)>::value;
        EXPECT_TRUE(val);
        auto cmp = std::vector<std::string>{"hello", " my", " name", " is", " foo", "faa"};
        EXPECT_EQ(cmp, out);
    }
}
