#include <type_traits>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include "scalgorithm.hpp"
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

namespace lesson_2_ns {

// compare elements of a container to the values pointed to by the elements of
// another container
template <typename Container, typename PointerContainer>
void
compare_container_to_pointer_container(Container&& c, PointerContainer&& pc) {
    auto cit = c.begin();
    auto pcit = pc.begin();

    for(; cit != c.end(); ++cit, ++pcit) {
        EXPECT_EQ(*cit, **pcit);
    }
}

};


TEST(lesson_2, pointers) {
    using namespace lesson_2_ns;

    const std::vector<int> v{1,2,3};
    const std::list<int> l{4,5,6};
    const std::forward_list<int> fl{7,8,9};

    {
        const auto& cvr = v;
        const auto& clr = l;
        const auto& cflr = fl;
        auto outv = sca::pointers(cvr);
        auto outl = sca::pointers(clr);
        auto outfl = sca::pointers(cflr);
        auto valv = std::is_same<std::vector<const int*>,decltype(outv)>::value;
        auto vall = std::is_same<std::vector<const int*>,decltype(outl)>::value;
        auto valfl = std::is_same<std::vector<const int*>,decltype(outfl)>::value;
        EXPECT_TRUE(valv);
        EXPECT_TRUE(vall);
        EXPECT_TRUE(valfl);
        compare_container_to_pointer_container(cvr, outv);
        compare_container_to_pointer_container(clr, outl);
        compare_container_to_pointer_container(cflr, outfl);
    }

    {
        // copy elements into mutable containers
        auto cpv = v;
        auto cpl = l;
        auto cpfl = fl;

        auto outv = sca::pointers(cpv);
        auto outl = sca::pointers(cpl);
        auto outfl = sca::pointers(cpfl);
        auto valv = std::is_same<std::vector<int*>,decltype(outv)>::value;
        auto vall = std::is_same<std::vector<int*>,decltype(outl)>::value;
        auto valfl = std::is_same<std::vector<int*>,decltype(outfl)>::value;
        EXPECT_TRUE(valv);
        EXPECT_TRUE(vall);
        EXPECT_TRUE(valfl);
        compare_container_to_pointer_container(cpv, outv);
        compare_container_to_pointer_container(cpl, outl);
        compare_container_to_pointer_container(cpfl, outfl);
    }

    {
        auto cpv = v; // deep copy v
        auto out = sca::pointers(cpv);
        
        for(auto e : out) {
            *e = *e + 2; // increment v2 elements by 2
        }

        std::vector<int> expect{3,4,5};
        EXPECT_EQ(expect, cpv);
    }

    {
        // get a container of pointers to const values
        auto pv = sca::pointers(v);
        auto rpv = sca::reverse(pv);

        EXPECT_EQ(3, *(rpv[0]));
        EXPECT_EQ(2, *(rpv[1]));
        EXPECT_EQ(1, *(rpv[2]));
    }

    {
        // copy elements from source containers
        auto cpv = v;
        auto cpl = l;
        auto cpfl = fl;

        // convert to containers of pointers
        auto outv = sca::pointers(cpv);
        auto outl = sca::pointers(cpl);
        auto outfl = sca::pointers(cpfl);

        // group containers of pointers together in an arbitrary order
        auto outgrp = sca::group(outfl, outv, outl);

        // verify pointed values are as expected
        EXPECT_EQ(7, *(outgrp[0]));
        EXPECT_EQ(8, *(outgrp[1]));
        EXPECT_EQ(9, *(outgrp[2]));
        EXPECT_EQ(1, *(outgrp[3]));
        EXPECT_EQ(2, *(outgrp[4]));
        EXPECT_EQ(3, *(outgrp[5]));
        EXPECT_EQ(4, *(outgrp[6]));
        EXPECT_EQ(5, *(outgrp[7]));
        EXPECT_EQ(6, *(outgrp[8]));

        // sort pointers by pointed values in ascending order
        std::sort(outgrp.begin(), outgrp.end(), [](int* a, int* b){ return *a < *b; });
        EXPECT_EQ(1, *(outgrp[0]));
        EXPECT_EQ(2, *(outgrp[1]));
        EXPECT_EQ(3, *(outgrp[2]));
        EXPECT_EQ(4, *(outgrp[3]));
        EXPECT_EQ(5, *(outgrp[4]));
        EXPECT_EQ(6, *(outgrp[5]));
        EXPECT_EQ(7, *(outgrp[6]));
        EXPECT_EQ(8, *(outgrp[7]));
        EXPECT_EQ(9, *(outgrp[8]));

        // verify original mutable containers are unmodified 
        EXPECT_EQ(v, cpv);
        EXPECT_EQ(l, cpl);
        EXPECT_EQ(fl, cpfl);
    }
}
