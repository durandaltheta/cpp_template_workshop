#include <string>
#include <vector>
#include <list>
#include "algorithm.hpp"
#include <gtest/gtest.h> 

TEST(lesson_1, manual_type_specification) {
    std::vector<int> v{1,2,3};

    EXPECT_EQ(v[0],1);
    EXPECT_EQ(v[1],2);
    EXPECT_EQ(v[2],3);
}

namespace nm_type_deduction {

template <typename T>
T add(T t1, T t2) {
    return t1 + t2;
}

}

TEST(lesson_1, type_deduction) {
    EXPECT_EQ(nm_type_deduction::add(1,2), 3);
    EXPECT_EQ(nm_type_deduction::add(3.0,5.5), 8.5);
    EXPECT_EQ(nm_type_deduction::add(std::string("hello "),std::string("world")), std::string("hello world"));
}

namespace nm_multiple_template_types {

template <typename T, typename T2>
T add(T t1, T2 t2) {
    return t1 + t2;
}

}

TEST(lesson_1, multiple_template_types) {
    EXPECT_EQ(nm_multiple_template_types::add(1,2), 3);
    EXPECT_EQ(nm_multiple_template_types::add(1,2.0), 3);
}

namespace nm_type_specialization {

template <typename T, typename T2>
T add(T t1, T2 t2) {
    return t1 + t2;
}

template <typename T>
std::string add(std::string s, T t) {
    return s + std::to_string(t);
}

template <typename T>
std::string add(T t, std::string s) {
    return std::to_string(t) + s;
}

}

TEST(lesson_1, type_specialization) {
    EXPECT_EQ(nm_type_specialization::add(1, 2), 3);
    EXPECT_EQ(nm_type_specialization::add(std::string("number: "), 3), std::string("number: 3"));
    EXPECT_EQ(nm_type_specialization::add(3, std::string(" is also a number")), std::string("3 is also a number"));
}

namespace nm_default_type_assignment {

template <typename T, typename Container = std::vector<T>>
Container construct_container_with_one_element(T t) {
    return Container{t};
}

}

TEST(lesson_1, default_type_assignment) {
    EXPECT_EQ(1, nm_default_type_assignment::construct_container_with_one_element(1).front());
    {
        auto l = nm_default_type_assignment::construct_container_with_one_element<int, std::list<int>>(2);

        EXPECT_EQ(2, l.front());
    }
}
