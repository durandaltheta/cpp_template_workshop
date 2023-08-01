#include <string>
#include <vector>
#include <list>
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_1, manual_type_specification) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_1, type_deduction) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_1, multiple_template_types) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_1, type_specialization) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_1, default_type_assignment) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

namespace lesson_1_ns {

template <typename C, typename C2>
bool compare_container_elements(C c, C2 c2) {
    return false;
}

}

/*
EXTRA CREDIT
Implement the body of template `lesson_1_ns::compare_container_elements` such 
that it returns `true` if both containers are the same size, and elements of the 
first container are equivalent to the other container at each index. Otherwise, 
`compare_container_elements` should return `false`.
 */
TEST(lesson_1, extra_credit_container_comparison) {
    using namespace lesson_1_ns;

    {
        const std::vector<int> c{1,2,3};
        const std::list<int> c2{1,2,3};
        const std::vector<int> c3{1,2,3,4};
        const std::list<int> c4{0,2,3};

        EXPECT_TRUE(compare_container_elements(c,c2));
        EXPECT_TRUE(compare_container_elements(c2,c));
        EXPECT_FALSE(compare_container_elements(c,c3));
        EXPECT_FALSE(compare_container_elements(c3,c));
        EXPECT_FALSE(compare_container_elements(c,c4));
    }

    {
        const std::vector<std::string> c{"1","2","3"};
        const std::list<std::string> c2{"1","2","3"};
        const std::vector<std::string> c3{"1","2","3","4"};
        const std::list<std::string> c4{"0","2","3"};

        EXPECT_TRUE(compare_container_elements(c,c2));
        EXPECT_TRUE(compare_container_elements(c2,c));
        EXPECT_FALSE(compare_container_elements(c,c3));
        EXPECT_FALSE(compare_container_elements(c3,c));
        EXPECT_FALSE(compare_container_elements(c,c4));
    }
}
