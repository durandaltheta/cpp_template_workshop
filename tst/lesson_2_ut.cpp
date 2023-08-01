#include <type_traits>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include "scalgorithm"
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
        bool is_same = std::is_same<int,decltype(ref_i)>::value;
        EXPECT_FALSE(is_same);
    }

    {
        bool is_same = std::is_same<int,std::decay<decltype(ref_i)>::type>::value;
        EXPECT_TRUE(is_same);
    }
}

TEST(lesson_2, reverse) {
    std::forward_list<int> fl{1,2,3};
    std::list<std::string> l{"hello", " my", " ragtime", " gal"};

    {
        auto out = sca::reverse(fl);
        auto is_same = std::is_same<std::vector<int>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_EQ(3, out[0]);
        EXPECT_EQ(2, out[1]);
        EXPECT_EQ(1, out[2]);
    }

    {
        auto out = sca::reverse(l);
        auto is_same = std::is_same<std::vector<std::string>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
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
        auto is_same = std::is_same<std::vector<int>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        auto cmp = std::vector<int>{3,4,5,6,1,2};
        EXPECT_EQ(cmp, out);
    }

    {
        std::list<std::string> l{"hello"," my"};
        std::forward_list<std::string> fl{" name", " is"};
        std::vector<std::string> v{" foo","faa"};

        auto out = sca::group(l, fl, v);
        auto is_same = std::is_same<std::vector<std::string>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        auto cmp = std::vector<std::string>{"hello", " my", " name", " is", " foo", "faa"};
        EXPECT_EQ(cmp, out);
    }
}

namespace lesson_2_ns {

// compare elements of a container to the values pointed to by the elements of another container
template <typename Container, typename PointerContainer>
void
cmp_cont_to_pnt_cont(Container&& c, PointerContainer&& pc) {
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
        auto is_samev = std::is_same<std::vector<const int*>,decltype(outv)>::value;
        auto is_samel = std::is_same<std::vector<const int*>,decltype(outl)>::value;
        auto is_samefl = std::is_same<std::vector<const int*>,decltype(outfl)>::value;
        EXPECT_TRUE(is_samev);
        EXPECT_TRUE(is_samel);
        EXPECT_TRUE(is_samefl);
        cmp_cont_to_pnt_cont(cvr, outv);
        cmp_cont_to_pnt_cont(clr, outl);
        cmp_cont_to_pnt_cont(cflr, outfl);
    }

    {
        // copy elements into mutable containers
        auto cpv = v;
        auto cpl = l;
        auto cpfl = fl;

        auto outv = sca::pointers(cpv);
        auto outl = sca::pointers(cpl);
        auto outfl = sca::pointers(cpfl);
        auto is_samev = std::is_same<std::vector<int*>,decltype(outv)>::value;
        auto is_samel = std::is_same<std::vector<int*>,decltype(outl)>::value;
        auto is_samefl = std::is_same<std::vector<int*>,decltype(outfl)>::value;
        EXPECT_TRUE(is_samev);
        EXPECT_TRUE(is_samel);
        EXPECT_TRUE(is_samefl);
        cmp_cont_to_pnt_cont(cpv, outv);
        cmp_cont_to_pnt_cont(cpl, outl);
        cmp_cont_to_pnt_cont(cpfl, outfl);
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
}

TEST(lesson_2, values) {
    using namespace lesson_2_ns;

    const std::vector<int> v{1,2,3};

    // values() copies v
    {
        auto vv = sca::values(v);
        auto is_same = std::is_same<std::vector<int>,decltype(vv)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_EQ(v, vv);
    }

    // values() copies elements of pv, still equal to v
    {
        auto pv = sca::pointers(v);
        auto vv = sca::values(pv);
        auto is_same = std::is_same<std::vector<int>,decltype(vv)>::value;
        EXPECT_TRUE(is_same);
        cmp_cont_to_pnt_cont(vv, pv);
        EXPECT_EQ(v, vv);
    }
}

TEST(lesson_2, sort) {

    const std::vector<int> v{1,2,3};
    const std::list<int> l{4,5,6};
    const std::forward_list<int> fl{7,8,9};

    // sort elements
    {
        auto cpv = v;
        auto cpl = l;
        auto cpfl = fl;

        auto outgrp = sca::group(cpfl, cpl, cpv);

        {
            auto is_same = std::is_same<std::vector<int>,decltype(outgrp)>::value;
            EXPECT_TRUE(is_same);
        }

        EXPECT_EQ(7, outgrp[0]);
        EXPECT_EQ(8, outgrp[1]);
        EXPECT_EQ(9, outgrp[2]);
        EXPECT_EQ(4, outgrp[3]);
        EXPECT_EQ(5, outgrp[4]);
        EXPECT_EQ(6, outgrp[5]);
        EXPECT_EQ(1, outgrp[6]);
        EXPECT_EQ(2, outgrp[7]);
        EXPECT_EQ(3, outgrp[8]);

        // sort in ascending order
        auto outsort = sca::sort(outgrp, [](int a, int b) { return a < b; });

        {
            auto is_same = std::is_same<std::vector<int>,decltype(outsort)>::value;
            EXPECT_TRUE(is_same);
        }

        EXPECT_EQ(1, outsort[0]);
        EXPECT_EQ(2, outsort[1]);
        EXPECT_EQ(3, outsort[2]);
        EXPECT_EQ(4, outsort[3]);
        EXPECT_EQ(5, outsort[4]);
        EXPECT_EQ(6, outsort[5]);
        EXPECT_EQ(7, outsort[6]);
        EXPECT_EQ(8, outsort[7]);
        EXPECT_EQ(9, outsort[8]);
    }

    // sort pointers
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
        auto outsort = sca::sort(outgrp, [](int* a, int* b){ return *a < *b; });
        EXPECT_EQ(1, *(outsort[0]));
        EXPECT_EQ(2, *(outsort[1]));
        EXPECT_EQ(3, *(outsort[2]));
        EXPECT_EQ(4, *(outsort[3]));
        EXPECT_EQ(5, *(outsort[4]));
        EXPECT_EQ(6, *(outsort[5]));
        EXPECT_EQ(7, *(outsort[6]));
        EXPECT_EQ(8, *(outsort[7]));
        EXPECT_EQ(9, *(outsort[8]));

        // verify original mutable containers are unmodified 
        EXPECT_EQ(v, cpv);
        EXPECT_EQ(l, cpl);
        EXPECT_EQ(fl, cpfl);
    }
}
