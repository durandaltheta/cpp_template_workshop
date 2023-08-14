#include <string>
#include <vector>
#include <list>
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_1, manual_type_specification) {
    std::vector<int> v{1,2,3};

    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);

    std::list<std::string> l{"1", "2", "3"};
    auto cur = l.begin();

    EXPECT_EQ(std::string("1"), *cur);
    ++cur;
    EXPECT_EQ(std::string("2"), *cur);
    ++cur;
    EXPECT_EQ(std::string("3"), *cur);
    ++cur;
}

namespace lesson_1_ns {

template <typename T>
T add(T t1, T t2) {
    return t1 + t2;
}

}

TEST(lesson_1, type_deduction) {
    EXPECT_EQ(lesson_1_ns::add(1,2), 3);
    EXPECT_EQ(lesson_1_ns::add(3.0, 5.5), 8.5);
    EXPECT_EQ(lesson_1_ns::add(std::string("hello "), std::string("world")), std::string("hello world"));
}

namespace nm_multiple_template_types {

template <typename T, typename T2>
T add(T t1, T2 t2){
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

namespace lesson_1_ns {

template <typename T, typename Container = std::vector<T>>
Container construct_container_with_one_element(T t) {
    return Container{t};
}
}

TEST(lesson_1, default_type_assignment) {
    EXPECT_EQ(1, lesson_1_ns::construct_container_with_one_element(1).front());

    {
        auto l = lesson_1_ns::construct_container_with_one_element<int, std::list<int>>(2);

        EXPECT_EQ(2, l.front());
    }
}

// namespace lesson_1_ns {

// template <typename C, typename C2>
// bool compare_container_elements(C c, C2 c2) {
//     return false;
// }

// }

// /*
// EXTRA CREDIT
// Implement the body of template `lesson_1_ns::compare_container_elements` such 
// that it returns `true` if both containers are the same size, and elements of the 
// first container are equivalent to the other container at each index. Otherwise, 
// `compare_container_elements` should return `false`.
//  */
// TEST(lesson_1, extra_credit_container_comparison) {
//     using namespace lesson_1_ns;

//     {
//         const std::vector<int> c{1,2,3};
//         const std::list<int> c2{1,2,3};
//         const std::vector<int> c3{1,2,3,4};
//         const std::list<int> c4{0,2,3};

//         EXPECT_TRUE(compare_container_elements(c,c2));
//         EXPECT_TRUE(compare_container_elements(c2,c));
//         EXPECT_FALSE(compare_container_elements(c,c3));
//         EXPECT_FALSE(compare_container_elements(c3,c));
//         EXPECT_FALSE(compare_container_elements(c,c4));
//     }

//     {
//         const std::vector<std::string> c{"1","2","3"};
//         const std::list<std::string> c2{"1","2","3"};
//         const std::vector<std::string> c3{"1","2","3","4"};
//         const std::list<std::string> c4{"0","2","3"};

//         EXPECT_TRUE(compare_container_elements(c,c2));
//         EXPECT_TRUE(compare_container_elements(c2,c));
//         EXPECT_FALSE(compare_container_elements(c,c3));
//         EXPECT_FALSE(compare_container_elements(c3,c));
//         EXPECT_FALSE(compare_container_elements(c,c4));
//     }
// }
