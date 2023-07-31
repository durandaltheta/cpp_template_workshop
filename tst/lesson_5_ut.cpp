#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include "scalgorithm"
#include <gtest/gtest.h> 

namespace lesson_5_ns {

int function_1() {
    return 1;
}

struct Functor_1 { 
    int operator()() {
        return 2;
    }
};

template <typename F>
auto
execute_callable(F&& f) {
    return f();
}

};

TEST(lesson_5, callable) {
    using namespace lesson_5_ns;

    int (*function_ptr_1)() = function_1;
    auto lambda_1 = []{ return 3; };
    std::function<int()> wrapper_1(lambda_1);

    EXPECT_EQ(1, function_1());
    EXPECT_EQ(1, function_ptr_1());
    EXPECT_EQ(2, Functor_1()());
    EXPECT_EQ(3, lambda_1());
    EXPECT_EQ(3, wrapper_1());

    EXPECT_EQ(1, execute_callable(function_1));
    EXPECT_EQ(1, execute_callable(function_ptr_1));
    EXPECT_EQ(2, execute_callable(Functor_1()));
    EXPECT_EQ(3, execute_callable(lambda_1));
    EXPECT_EQ(3, execute_callable(wrapper_1));
}

namespace lesson_5_ns {

int function_2(int i) {
    return i + 1;
}

struct Functor_2 { 
    std::string operator()(std::string s) {
        return std::string("Functor_2") + s;
    }
};

template <typename F, typename T>
auto
execute_unary_callable(F&& f, T&& t) {
    return f(std::forward<T>(t));
}

};

TEST(lesson_5, callable_with_argument) {
    using namespace lesson_5_ns;

    int (*function_ptr_2)(int) = function_2;
    auto lambda_2 = [](int i){ return i + 3; };
    std::function<int(int)> wrapper_2(lambda_2);

    EXPECT_EQ(2, function_2(1));
    EXPECT_EQ(3, function_ptr_2(2));
    EXPECT_EQ(std::string("Functor_2 hello"), Functor_2()(std::string(" hello")));
    EXPECT_EQ(5, lambda_2(2));
    EXPECT_EQ(5, wrapper_2(2));

    EXPECT_EQ(2, execute_unary_callable(function_2,1));
    EXPECT_EQ(3, execute_unary_callable(function_ptr_2,2));
    EXPECT_EQ(std::string("Functor_2 hello"), execute_unary_callable(Functor_2(),std::string(" hello")));
    EXPECT_EQ(5, execute_unary_callable(lambda_2,2));
    EXPECT_EQ(5, execute_unary_callable(wrapper_2,2));
}

namespace lesson_5_ns {

std::string function_3(int i) {
    return std::to_string(i);
}

template <typename InputIt, typename OutputIt, typename UnaryOperation>
void transform(InputIt cur, InputIt end, OutputIt out, UnaryOperation f) {
    while(cur != end) {
        *out = f(*cur);
        ++cur;
        ++out;
    }
}

}

TEST(lesson_5, algorithms_and_callables) {
    const std::vector<int> v{1,2,3,4,5,6,7,8,9,10};

    // transform int via increment
    {
        std::vector<int> out(sca::size(v));
        lesson_5_ns::transform(v.begin(), v.end(), out.begin(), [](int i){ return i + 2; });
        std::vector<int> expect{3,4,5,6,7,8,9,10,11,12};
        EXPECT_EQ(expect, out);
    }

    // transform int to strings
    {
        std::vector<std::string> out(sca::size(v));
        lesson_5_ns::transform(v.begin(), v.end(), out.begin(), lesson_5_ns::function_3);
        std::vector<std::string> expect{
                std::to_string(1),
                std::to_string(2),
                std::to_string(3),
                std::to_string(4),
                std::to_string(5),
                std::to_string(6),
                std::to_string(7),
                std::to_string(8),
                std::to_string(9),
                std::to_string(10)};
        EXPECT_EQ(expect, out);
    }
}

TEST(lesson_5, filter) {
    const std::vector<int> v{1,2,3,4,5,6,7,8,9,10};

    // simple filter
    {
        auto out = sca::filter([](int i) { return i % 2 == 0; }, v);
        auto is_same = std::is_same<std::vector<int>,decltype(out)>::value;
        std::vector<int> expect{2,4,6,8,10};
        EXPECT_TRUE(is_same);
        EXPECT_EQ(expect, out);
    }
   
    // inline lambda filter
    {
        std::vector<int> expect{1,3,5,7,9};
        EXPECT_EQ(expect, sca::filter([](int i) { return i % 2 != 0; }, v));
    }

    // multiline filter function
    {
        int cnt = 0;
        auto skip_every_2 = [&cnt](int i) {
            if(cnt < 2) {
                ++cnt;
                return false;
            } else {
                cnt = 0;
                return true;
            }
        };

        std::vector<int> expect{3,6,9};
        EXPECT_EQ(expect, sca::filter(skip_every_2, v));
    }

    // filter slice
    {
        auto sl = sca::slice(v,4,4);
        auto out = sca::filter([](int i) { return i % 2 == 0; }, sl);
        std::vector<int> expect{6,8};
        EXPECT_EQ(expect, out);
    }

    // complex string filter
    {
        std::vector<std::string> s{"hello", "1", " my", "2", " name", "3", " is", "4", " regret", ""};

        auto ascii_filter_non_alphabet_or_space = [](std::string& s) {
            auto is_space = [](const char c){ return c == 32; };
            auto upper_case = [](const char c){ return 64 < c && c < 91; };
            auto lower_case = [](const char c){ return 96 < c && c < 123; };
            return s.size() && (is_space(s[0]) || upper_case(s[0]) || lower_case(s[0]));
        };

        auto out = sca::filter(ascii_filter_non_alphabet_or_space, s);
        auto is_same = std::is_same<std::vector<std::string>,decltype(out)>::value;
        std::vector<std::string> expect{"hello", " my", " name", " is", " regret"};
        EXPECT_TRUE(is_same);
        EXPECT_EQ(expect, out);
    }
}

#ifdef COMPILE_EXTRA_CREDIT
namespace lesson_5_ns {

}

TEST(lesson_5, extra_credit) {
}
#endif
