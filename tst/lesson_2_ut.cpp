#include <type_traits>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <algorithm>
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_2, lvalues_and_rvalues) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_2, moving_vs_forwarding) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_2, type_decay) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_2, reverse) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_2, group) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_2, pointers) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_2, values) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_2, sort) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

namespace lesson_2_ns {

struct value_category_aware {
    const bool lvalue_constructed;
    const bool rvalue_constructed;

    struct rvalue_override { };

    value_category_aware() : 
        lvalue_constructed(false),
        rvalue_constructed(false)
    { }

    value_category_aware(const value_category_aware& rhs) : 
        lvalue_constructed(true),
        rvalue_constructed(false)
    { }
    
    value_category_aware(value_category_aware&& rhs) : 
        lvalue_constructed(false),
        rvalue_constructed(true)
    { }

    value_category_aware(rvalue_override) :
        lvalue_constructed(false),
        rvalue_constructed(true)
    { }
};

template <typename T>
struct lvalue_allowed {
    lvalue_allowed(lvalue_allowed&&) = delete;

    lvalue_allowed() { }
    lvalue_allowed(const lvalue_allowed& rhs) { }
    lvalue_allowed(lvalue_allowed& rhs) { }

    template <typename T2>
    lvalue_allowed(T2&& t2) : 
        m_t(std::forward<T2>(t2))
    { }

    T& value() {
        return m_t;
    }

    const T& value() const {
        return m_t;
    }

private:
    T m_t;
};

template <typename T>
struct rvalue_allowed {
    rvalue_allowed(const rvalue_allowed&) = delete;

    rvalue_allowed(){}
    rvalue_allowed(rvalue_allowed&& rhs) { }

    template <typename T2>
    rvalue_allowed(T2&& t2) { }

    T& value() {
        return m_t;
    }

private:
    T m_t;
};

}

/*
EXTRA CREDIT
Implement the following constructors 
- lesson_2_ns::lvalue_allowed(const lvalue_allowed&): m_t is initialized via deep copy
- lesson_2_ns::lvalue_allowed(lvalue_allowed&): m_t is initialized via deep copy
- lesson_2_ns::lvalue_allowed(T2&&): m_t is initialized via perfect forwarding
- lesson_2_ns::rvalue_allowed(rvalue_allowed&&): m_t is initialized via move copy
- lesson_2_ns::rvalue_allowed(T2&&): m_t is initialized via perfect forwarding 
- forward_construct so that type A is perfect forwarded to new construct T
 */
TEST(lesson_2, extra_credit_value_categories) {
    using namespace lesson_2_ns;

    {
       lvalue_allowed<int> la(3);
       lvalue_allowed<int> la2(la);
       lvalue_allowed<value_category_aware> la3;
       lvalue_allowed<value_category_aware> la4(la3);
       const lvalue_allowed<value_category_aware> la5;
       lvalue_allowed<value_category_aware> la6(la5);
       EXPECT_EQ(3, la.value());
       EXPECT_EQ(3, la2.value());
       EXPECT_FALSE(la3.value().lvalue_constructed);
       EXPECT_FALSE(la3.value().rvalue_constructed);
       EXPECT_TRUE(la4.value().lvalue_constructed);
       EXPECT_FALSE(la4.value().rvalue_constructed);
       EXPECT_FALSE(la5.value().lvalue_constructed);
       EXPECT_FALSE(la5.value().rvalue_constructed);
       EXPECT_TRUE(la6.value().lvalue_constructed);
       EXPECT_FALSE(la6.value().rvalue_constructed);
    }

    {
       EXPECT_EQ(std::string("3"), rvalue_allowed<std::string>("3").value());
       EXPECT_EQ(std::string("3"), rvalue_allowed<std::string>(rvalue_allowed<std::string>("3")).value());
       EXPECT_FALSE(rvalue_allowed<value_category_aware>().value().lvalue_constructed);
       EXPECT_FALSE(rvalue_allowed<value_category_aware>().value().rvalue_constructed);
       EXPECT_FALSE(rvalue_allowed<value_category_aware>(rvalue_allowed<value_category_aware>()).value().lvalue_constructed);
       EXPECT_TRUE(rvalue_allowed<value_category_aware>(rvalue_allowed<value_category_aware>(value_category_aware::rvalue_override())).value().rvalue_constructed);
    }
}
