#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include "scalgorithm"
#include <gtest/gtest.h> 

//TODO: implement test fixture from main
struct lesson_4_f : public ::testing::Test { };


TEST_F(lesson_4_f, sfinae) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_4, size) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_4, const_lvalue_slice) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_4, rvalue_slice) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_4, mutable_slice) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

namespace lesson_4_ns {
namespace detail {

template<typename T>
struct has_resize_struct {
    static const bool has = false; //TODO: implement missing resize detection struct
};

template <typename T>
using has_resize = std::false_type; //TODO: implement missing has_resize helper alias

// Resize the container via C::resize() method
template <typename C>
void resize(C& c, size_t new_size, std::true_type) { 
    c.resize(new_size);
}

// Resize the container via C(size) construction
template <typename C>
void resize(C& c, size_t new_size, std::false_type) {
    c = C(new_size);
}

}

template <typename C>
void resize(C& c, size_t new_size) {
    detail::resize(c, new_size, detail::has_resize<C>()); 
}

template <typename T>
struct no_resize {
    typedef size_t size_type;

    no_resize(size_type sz) : m_sz(sz) { }

    // let compiler generate other constructors

    size_type size() const {
        return m_sz;
    }

private:
    size_type m_sz;
};

}

namespace sca {

template <typename C> 
size_t size(C&& c) { //TODO: remove this implementation and implement real one in scalgorithm.hpp
    return 0;
}

}

/*
EXTRA CREDIT
Implement struct `lesson_4_ns::has_resize_struct` such that 
`has_resize_struct::has` property is equal to `true` when an object has a 
`void T::resize(T::size_type)` method, otherwise `has_resize::has` should equal 
`false`.

Implement stub `lesson_4_ns::has_resize` type alias to return a compile time 
`std::integral_constant` similar to `sca::detail::has_size`.
 */
TEST(lesson_4, extra_credit_resize_sfinae_detection) {
    using namespace lesson_4_ns;

    {
        auto is_same = std::is_same<
            detail::has_resize<std::vector<int>>,
            std::true_type>::value;
        EXPECT_TRUE(is_same);
    }

    {

        auto is_same = std::is_same<
            detail::has_resize<std::list<int>>,
            std::true_type>::value;
        EXPECT_TRUE(is_same);
    }

    {
        auto is_same = std::is_same<
            detail::has_resize<no_resize<int>>,
            std::false_type>::value;
        EXPECT_TRUE(is_same);
    }

    {
        std::vector<int> v(5);
        EXPECT_EQ(5, sca::size(v));
        resize(v, 500);
        EXPECT_EQ(500, sca::size(v));
    }

    {
        std::list<std::string> l(5);
        EXPECT_EQ(5, sca::size(l));
        resize(l, 50);
        EXPECT_EQ(50, sca::size(l));
    }

    {
        no_resize<double> nr(5);
        EXPECT_EQ(5, sca::size(nr));
        resize(nr, 5000);
        EXPECT_EQ(5000, sca::size(nr));
    }
}
