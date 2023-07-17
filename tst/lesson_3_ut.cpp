#include <string>
#include <iostream>
#include <vector>
#include "algorithm.hpp"
#include <gtest/gtest.h> 

struct lesson_3 : public ::testing::Test {
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
        ++lesson_3::counts()[0];
        return t1 + t2;
    }

    template <typename T>
    static std::string add(std::string s, T t) {
        ++lesson_3::counts()[1];
        return s + std::to_string(t);
    }

    template <typename T>
    static std::string add(T t, std::string s) {
        ++lesson_3::counts()[2];
        return std::to_string(t) + s;
    }

    static std::string add(std::string s, std::string s2) { 
        ++lesson_3::counts()[3];
        return s + s2;
    }
};


TEST_F(lesson_3, sfinae) {
    for(auto& e : lesson_3::counts()) {
        EXPECT_EQ(0, e);
    }

    EXPECT_EQ(5, lesson_3::add(2,3));
    EXPECT_EQ(1, lesson_3::counts()[0]);
    EXPECT_EQ(0, lesson_3::counts()[1]);
    EXPECT_EQ(0, lesson_3::counts()[2]);
    EXPECT_EQ(0, lesson_3::counts()[3]);

    EXPECT_EQ(
        std::string("hello world"), 
        lesson_3::add(
            std::string("hello"), 
            std::string(" world")));
    EXPECT_EQ(1, lesson_3::counts()[0]);
    EXPECT_EQ(0, lesson_3::counts()[1]);
    EXPECT_EQ(0, lesson_3::counts()[2]);
    EXPECT_EQ(1, lesson_3::counts()[3]);

    EXPECT_EQ(
        std::string("3 world"), 
        lesson_3::add(
            3, 
            std::string(" world")));
    EXPECT_EQ(1, lesson_3::counts()[0]);
    EXPECT_EQ(0, lesson_3::counts()[1]);
    EXPECT_EQ(1, lesson_3::counts()[2]);
    EXPECT_EQ(1, lesson_3::counts()[3]);

    EXPECT_EQ(
        std::string("world 3"), 
        lesson_3::add(
            std::string("world "),
            3));
    EXPECT_EQ(1, lesson_3::counts()[0]);
    EXPECT_EQ(1, lesson_3::counts()[1]);
    EXPECT_EQ(1, lesson_3::counts()[2]);
    EXPECT_EQ(1, lesson_3::counts()[3]);
}
