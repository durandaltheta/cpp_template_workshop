#include <string>
#include <vector>
#include <list>
#include <sstream>
#include "scalgorithm.hpp"
#include <gtest/gtest.h> 

namespace lesson_5_ns {

namespace detail {

// handle conversion of non-strings to `std::string`
template <typename T>
std::string convert_to_string(T&& t) {
    return std::to_string(std::forward<T>(t));
}

// overload to forward mutable c strings
std::string convert_to_string(char* s) {
    return std::string(s);
}

// overload to forward const c strings
std::string convert_to_string(const char* s) {
    return std::string(s);
}

// overload to forward `std::string`s without converting
std::string convert_to_string(std::string&& s) {
    return std::move(s);
}

// lvalue overloads explicitly return a reference to avoid copying
std::string& convert_to_string(std::string& s) {
    return s;
}

const std::string& convert_to_string(const std::string& s) {
    return s;
}

// Final invocation when no more arguments to concatenate. 
std::string concatenate(std::stringstream& ss) { 
    return ss.str();
}

// handle concatenating an element at a time
template <typename A, typename... As>
std::string concatenate(std::stringstream& ss, A&& a, As&&... as) {
    ss << convert_to_string(std::forward<A>(a));
    return concatenate(ss, std::forward<As>(as)...); // pass the rest to further calls
}

}

template <typename... As>
std::string concatenate(As&&... as) {
    std::stringstream ss; // create a stringstream to be used in detail calls
    return detail::concatenate(ss, std::forward<As>(as)...);
}

}

TEST(lesson_5, concatenate) {
    using namespace lesson_5_ns;

    // concatenate 3 rvalue `std::string`s
    {
        auto s = concatenate(std::string("foo"), std::string(" "), std::string("faa"));
        EXPECT_EQ(std::string("foo faa"), s);
    }
    
    // concatenate 2 rvalue `std::string`s and an lvalue `std::string`
    {
        std::string third("faa");
        auto s = concatenate(std::string("foo"), std::string(" "), third);
        EXPECT_EQ(std::string("foo faa"), s);
    }
    
    // concatenate 1 rvalue `std::string`s, an lvalue `std::string`, and
    // a const lvalue `std::string`
    {
        const std::string first("foo");
        std::string third("faa");
        auto s = concatenate(first, std::string(" "), third);
        EXPECT_EQ(std::string("foo faa"), s);
    }

    // concatenate 2 `std::string`s and a c-string 
    {
        auto s = concatenate(std::string("foo"), std::string(" "), "faa");
        EXPECT_EQ(std::string("foo faa"), s);
    }
    
    // concatenate 1 `std::string`s and 2 c-string
    {
        auto s = concatenate("foo", std::string(" "), "faa");
        EXPECT_EQ(std::string("foo faa"), s);
    }

    // concatenate 3 c-strings
    {
        auto s = concatenate("foo", " ", "faa");
        EXPECT_EQ(std::string("foo faa"), s);
    }

    // concatenate a mutable c string and a const c string
    {
        char mutable_c_str[10];
        memset(mutable_c_str, 0, sizeof(mutable_c_str));
        strncpy(mutable_c_str, "hello", sizeof(mutable_c_str) - 1);
        auto s = concatenate(mutable_c_str, " world");
        EXPECT_EQ(std::string("hello world"), s);
    }

    // concatenate an std::string and a number
    {
        auto s = concatenate(std::string("number "), 3);
        EXPECT_EQ(std::string("number 3"), s);
    }

    // concatenate a number and std::string
    {
        auto s = concatenate(3, std::string(" is a number"));
        EXPECT_EQ(std::string("3 is a number"), s);
    }

}
