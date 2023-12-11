#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include <functional>
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_5, callable) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_5, callable_with_argument) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_5, algorithms_and_callables) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_5, filter) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

namespace lesson_5_ns {

template <typename C>
void sort(C&& c) {
    //TODO: implement missing body
}

template <typename C, typename Compare>
void sort(C&& c, Compare&& cmp) {
    //TODO: implement missing body
}

template <typename C, typename C2>
bool equal(C&& c, C2&& c2) {
    return false; //TODO: implement missing body
}

template <typename C, typename C2, typename BinaryPredicate>
bool equal(C&& c, C2&& c2, BinaryPredicate&& pred) {
    return false; //TODO: implement missing body
}

template <typename C, typename UnaryPredicate>
bool all(C&& c, UnaryPredicate&& pred) {
    return false; //TODO: implement missing body
}

template <typename C, typename UnaryPredicate>
bool any(C&& c, UnaryPredicate&& pred) {
    return false; //TODO: implement missing body
}

template <typename C, typename UnaryPredicate>
bool none(C&& c, UnaryPredicate&& pred) {
    return false; //TODO: implement missing body
}

}

// namespace sca {

// //TODO: remove this algorithm implementation and implement real one in scalgorithm.hpp
// template <typename C>
// auto pointers(C&& c) {
//     typedef typename std::decay_t<C>::value_type CV;
//     return std::vector<CV*>(std::distance(c.begin(), c.end()));
// }

// }

/*
EXTRA CREDIT 
Implement the bodies of these algorithms in lesson_5_ns namespace which accept container universal references and potentially a callable:
template <typename C> void sort(C&& c)
template <typename C, typename Compare> void sort(C&& c, Compare&& cmp)
template <typename C, typename C2> bool equal(C&& c, C2&& c2)
template <typename C, typename C2, typename BinaryPredicate> bool equal(C&& c, C2&& c2, BinaryPredicate&& pred)
template <typename C, typename UnaryPredicate> bool all(C&& c, UnaryPredicate&& pred)
template <typename C, typename UnaryPredicate> bool any(C&& c, UnaryPredicate&& pred)
template <typename C, typename UnaryPredicate> bool none(C&& c, UnaryPredicate&& pred)

Using standard library algorithms `std::sort`, `std::equal`, `std::all_of`, 
`std::any_of`, and `std::none_of`:
- https://en.cppreference.com/w/cpp/algorithm/sort 
- https://en.cppreference.com/w/cpp/algorithm/equal
- https://en.cppreference.com/w/cpp/algorithm/all_any_none_of
 */
TEST(lesson_5, extra_credit_algorithms_and_callables) {
    const std::vector<int> cv{1,2,3,4,5};
    const std::list<int> cl{1,3,5};
    const std::forward_list<int> cfl{2,4};

    {
        auto v = cv;
        lesson_5_ns::sort(v, std::greater<int>());

        std::vector<int> expect{5,4,3,2,1};
        EXPECT_TRUE(lesson_5_ns::equal(expect, v));
        lesson_5_ns::sort(v);
        EXPECT_TRUE(lesson_5_ns::equal(cv, v));
       
    }

    {
        auto v = cv;
        auto pv = sca::pointers(v);
        auto cpv = sca::pointers(cv);

        EXPECT_FALSE(lesson_5_ns::equal(pv, cpv));
        EXPECT_TRUE(lesson_5_ns::equal(pv, pv));
        EXPECT_TRUE(lesson_5_ns::equal(cpv, cpv));
        
        auto cmp = [](const int* p1, const int* p2) {
            return *p1 == *p2;
        };

        std::vector<int> v2{6,7,8,9,10};
        auto pv2 = sca::pointers(v2);

        EXPECT_TRUE(lesson_5_ns::equal(pv, cpv, cmp));
        EXPECT_FALSE(lesson_5_ns::equal(pv, pv2));
        EXPECT_FALSE(lesson_5_ns::equal(pv, pv2,cmp));
    }

    {
        auto is_even = [](int i) { return i % 2 == 0; };
        auto is_odd = [](int i) { return i % 2 != 0; };

        EXPECT_FALSE(lesson_5_ns::all(cv, is_even));
        EXPECT_FALSE(lesson_5_ns::all(cv, is_odd));
        EXPECT_FALSE(lesson_5_ns::all(cl, is_even));
        EXPECT_TRUE(lesson_5_ns::all(cl, is_odd));
        EXPECT_TRUE(lesson_5_ns::all(cfl, is_even));
        EXPECT_FALSE(lesson_5_ns::all(cfl, is_odd));

        EXPECT_TRUE(lesson_5_ns::any(cv, is_even));
        EXPECT_TRUE(lesson_5_ns::any(cv, is_odd));
        EXPECT_FALSE(lesson_5_ns::any(cl, is_even));
        EXPECT_TRUE(lesson_5_ns::any(cl, is_odd));
        EXPECT_TRUE(lesson_5_ns::any(cfl, is_even));
        EXPECT_FALSE(lesson_5_ns::any(cfl, is_odd));

        EXPECT_FALSE(lesson_5_ns::none(cv, is_even));
        EXPECT_FALSE(lesson_5_ns::none(cv, is_odd));
        EXPECT_TRUE(lesson_5_ns::none(cl, is_even));
        EXPECT_FALSE(lesson_5_ns::none(cl, is_odd));
        EXPECT_FALSE(lesson_5_ns::none(cfl, is_even));
        EXPECT_TRUE(lesson_5_ns::none(cfl, is_odd));
    }
}
