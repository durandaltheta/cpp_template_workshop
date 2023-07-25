#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include "scalgorithm.hpp"
#include <gtest/gtest.h> 

struct lesson_3_f : public ::testing::Test {
protected:
    void SetUp() override {
        for(auto& e : counts()) {
            e = false;
        }
    }

public:
    static std::vector<int>& counts() {
        static std::vector<int> cnts(4);
        return cnts;
    }

    template <typename T, typename T2>
    static auto add(T t1, T2 t2) {
        ++lesson_3_f::counts()[0];
        return t1 + t2;
    }

    template <typename T>
    static std::string add(std::string s, T t) {
        ++lesson_3_f::counts()[1];
        return s + std::to_string(t);
    }

    template <typename T>
    static std::string add(T t, std::string s) {
        ++lesson_3_f::counts()[2];
        return std::to_string(t) + s;
    }

    static std::string add(std::string s, std::string s2) { 
        ++lesson_3_f::counts()[3];
        return s + s2;
    }
};


TEST_F(lesson_3_f, sfinae) {
    for(auto& e : lesson_3_f::counts()) {
        EXPECT_EQ(0, e);
    }

    EXPECT_EQ(5, lesson_3_f::add(2,3));
    EXPECT_EQ(1, lesson_3_f::counts()[0]);
    EXPECT_EQ(0, lesson_3_f::counts()[1]);
    EXPECT_EQ(0, lesson_3_f::counts()[2]);
    EXPECT_EQ(0, lesson_3_f::counts()[3]);

    EXPECT_EQ(
        std::string("hello world"), 
        lesson_3_f::add(
            std::string("hello"), 
            std::string(" world")));
    EXPECT_EQ(1, lesson_3_f::counts()[0]);
    EXPECT_EQ(0, lesson_3_f::counts()[1]);
    EXPECT_EQ(0, lesson_3_f::counts()[2]);
    EXPECT_EQ(1, lesson_3_f::counts()[3]);

    EXPECT_EQ(
        std::string("3 world"), 
        lesson_3_f::add(
            3, 
            std::string(" world")));
    EXPECT_EQ(1, lesson_3_f::counts()[0]);
    EXPECT_EQ(0, lesson_3_f::counts()[1]);
    EXPECT_EQ(1, lesson_3_f::counts()[2]);
    EXPECT_EQ(1, lesson_3_f::counts()[3]);

    EXPECT_EQ(
        std::string("world 3"), 
        lesson_3_f::add(
            std::string("world "),
            3));
    EXPECT_EQ(1, lesson_3_f::counts()[0]);
    EXPECT_EQ(1, lesson_3_f::counts()[1]);
    EXPECT_EQ(1, lesson_3_f::counts()[2]);
    EXPECT_EQ(1, lesson_3_f::counts()[3]);
}

TEST(lesson_3, size) {
    std::vector<int> v{1,2,3,4,5,6,7,8};
    std::list<std::string> l{"one", "two", "three"};
    std::forward_list<double> fl{1.0,2.0,3.0,4.0,5.0};

    EXPECT_EQ(8, sca::size(v));
    EXPECT_EQ(3, sca::size(l));
    EXPECT_EQ(5, sca::size(fl));
}

TEST(lesson_3, const_lvalue_slice) {
    const std::vector<int> v_base{1,13,5,78132,7,8};

    {
        auto v = v_base;
        auto out = sca::slice(v, 0, 2);
        auto is_same = std::is_same<sca::const_slice_of<std::vector<int>>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
    }

    {
        auto v = v_base;
        auto begin = v.begin();
        auto end = std::next(begin, 2);
        EXPECT_TRUE(std::equal(begin, end, sca::slice(v, 0, 2).begin()));
    }

    {
        auto v = v_base;
        auto begin = std::next(v.begin(), 2);
        auto end = std::next(begin, 3);
        EXPECT_TRUE(std::equal(begin, end, sca::slice(v, 2, 3).begin()));
    }

    {
        auto v = v_base;
        auto begin = std::next(v.begin(), 2);
        auto end = std::next(begin, 3);
        EXPECT_FALSE(std::equal(begin, end, sca::slice(v, 3, 3).begin()));
    }
    
    {
        auto v = v_base;
        auto csl = sca::slice(v, 2, 3);

        EXPECT_EQ(3, sca::size(csl));

        auto it = csl.begin();
        EXPECT_EQ(5, *it);
        ++it;
        EXPECT_EQ(78132, *it);
        ++it;
        EXPECT_EQ(7,*it);
        ++it;
        EXPECT_EQ(csl.end(), it);
    }
}

TEST(lesson_3, rvalue_slice) {
    const std::vector<int> v_base{1,13,5,78132,7,8};

    {
        auto v = v_base;
        auto out = sca::slice(std::move(v), 0, 2);
        auto is_same = std::is_same<sca::slice_of<std::vector<int>>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
    }

    {
        auto v = v_base;
        auto begin = v.begin();
        auto end = std::next(begin, 2);

        // must ensure a copy of the slice exists because slice holds rvalue
        // moved memory of its source container
        auto sl = sca::slice(std::move(v), 0, 2); 
        EXPECT_TRUE(std::equal(begin, end, sl.begin()));
    }

    {
        auto v = v_base;
        auto begin = std::next(v.begin(), 2);
        auto end = std::next(begin, 3);
        auto sl = sca::slice(std::move(v), 2, 3);
        EXPECT_TRUE(std::equal(begin, end, sl.begin()));
    }

    {
        auto v = v_base;
        auto begin = std::next(v.begin(), 2);
        auto end = std::next(begin, 3);
        auto sl = sca::slice(std::move(v), 3, 3);
        EXPECT_FALSE(std::equal(begin, end, sl.begin()));
    }
    
    {
        auto v = v_base;
        auto sl = sca::slice(std::move(v), 2, 3);

        for(auto& e : sl) {
            e = e + 1;
        }

        EXPECT_EQ(3, sca::size(sl));

        auto it = sl.begin();
        EXPECT_EQ(6, *it);
        ++it;
        EXPECT_EQ(78133, *it);
        ++it;
        EXPECT_EQ(8,*it);
        ++it;
        EXPECT_EQ(sl.end(), it);
    }
}

TEST(lesson_3, mutable_slice) {
    const std::vector<int> v_base{1,13,5,78132,7,8};

    {
        auto v = v_base;
        auto out = sca::mslice(v, 0, 2);
        auto is_same = std::is_same<sca::slice_of<std::vector<int>>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
    }

    {
        auto v = v_base;
        auto begin = v.begin();
        auto end = std::next(begin, 2);
        EXPECT_TRUE(std::equal(begin, end, sca::mslice(v, 0, 2).begin()));
    }

    {
        auto v = v_base;
        auto begin = std::next(v.begin(), 2);
        auto end = std::next(begin, 3);
        EXPECT_TRUE(std::equal(begin, end, sca::mslice(v, 2, 3).begin()));
    }

    {
        auto v = v_base;
        auto begin = std::next(v.begin(), 2);
        auto end = std::next(begin, 3);
        EXPECT_FALSE(std::equal(begin, end, sca::mslice(v, 3, 3).begin()));
    }

    {
        auto v = v_base;
        auto sl = sca::mslice(v, 2, 3);

        for(auto& e : sl) {
            e = e + 1;
        }

        EXPECT_EQ(3, sca::size(sl));

        auto it = sl.begin();
        EXPECT_EQ(6, *it);
        ++it;
        EXPECT_EQ(78133, *it);
        ++it;
        EXPECT_EQ(8, *it);
        ++it;
        EXPECT_EQ(sl.end(), it);
    }
}
