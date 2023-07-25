#include <string>
#include <vector>
#include <list>
#include <sstream>
#include "scalgorithm"
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

// lis_sameue overloads explicitly return a reference to avoid copying
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

    // concatenate 3 ris_sameue `std::string`s
    {
        auto s = concatenate(std::string("foo"), std::string(" "), std::string("faa"));
        EXPECT_EQ(std::string("foo faa"), s);
    }
    
    // concatenate 2 ris_sameue `std::string`s and an lis_sameue `std::string`
    {
        std::string third("faa");
        auto s = concatenate(std::string("foo"), std::string(" "), third);
        EXPECT_EQ(std::string("foo faa"), s);
    }
    
    // concatenate 1 ris_sameue `std::string`s, an lis_sameue `std::string`, and
    // a const lis_sameue `std::string`
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

TEST(lesson_5, detail_advance_group) {
    std::vector<int> v1{1,2,3};
    std::vector<int> v2{4,5,6};
    std::vector<int> v3{7,8,9};

    auto cur_v1 = v1.begin();
    auto cur_v2 = v2.begin();
    auto cur_v3 = v3.begin();

    EXPECT_EQ(1, *cur_v1);
    EXPECT_EQ(4, *cur_v2);
    EXPECT_EQ(7, *cur_v3);

    sca::detail::advance_group(cur_v1, cur_v2, cur_v3);

    EXPECT_EQ(2, *cur_v1);
    EXPECT_EQ(5, *cur_v2);
    EXPECT_EQ(8, *cur_v3);

    sca::detail::advance_group(cur_v1, cur_v2, cur_v3);

    EXPECT_EQ(3, *cur_v1);
    EXPECT_EQ(6, *cur_v2);
    EXPECT_EQ(9, *cur_v3);

    sca::detail::advance_group(cur_v1, cur_v2, cur_v3);

    EXPECT_EQ(v1.end(), cur_v1);
    EXPECT_EQ(v2.end(), cur_v2);
    EXPECT_EQ(v3.end(), cur_v3);
}

TEST(lesson_5, each) {
    std::vector<int> v1{1,2,3};
    std::vector<int> v2{4,5,6};
    
    const std::vector<int> expect{5,7,9};
    std::vector<int> out(sca::size(v1));
    auto out_it = out.begin();

    auto add = [&out_it](int a, int b) { 
        *out_it = a + b; 
        ++out_it;
    };

    sca::detail::each(add, v1.begin(), v1.end(), v2.begin());
    EXPECT_EQ(expect, out);

    out = std::vector<int>(sca::size(v1)); // reset our out vector
    out_it = out.begin(); // reset our iterator
    
    // completed algorithm `sca::each()` abstracts the argument iterators
    sca::each(add, v1, v2);
    EXPECT_EQ(expect, out);
    
    out = std::vector<int>(); // reset and resize our out vector
   
    // don't use iterator in this case
    auto add_v2 = [&out](int a, int b) { 
        out.push_back(a + b); 
    };
    
    sca::each(add_v2, v1, v2);
    EXPECT_EQ(expect, out);
}

TEST(lesson_5, detail_map) {
    std::vector<int> v1{1,2,3};
    std::vector<int> v2{4,5,6};

    {
        std::vector<int> out(sca::size(v1));
        auto add = [](int a, int b) { return a + b; };

        // internal algorithm `sca::detail::map` is similar to 
        // `std::transform()` except that it can accept iterators to more than 1 
        // container
        sca::detail::map(add, out.begin(), v1.begin(), v1.end(), v2.begin());

        std::vector<int> expect{5,7,9};
        EXPECT_EQ(expect, out);
    }

    {
        std::vector<std::string> out(sca::size(v1));
        auto add_and_stringify = [](int a, int b) { return std::to_string(a + b); };

        // internal algorithm `sca::detail::map` is similar to 
        // `std::transform()` except that it can accept iterators to more than 1 
        // container
        sca::detail::map(add_and_stringify, out.begin(), v1.begin(), v1.end(), v2.begin());

        std::vector<std::string> expect{"5","7","9"};
        EXPECT_EQ(expect, out);
    }
}

TEST(lesson_5, detail_fold) {
    std::vector<int> v1{1,2,3};
    std::vector<int> v2{4,5,6};

    {
        // sum 1 vector at a time
        auto sum = [](int cur_sum, int new_is_sameue) { 
            return cur_sum + new_is_sameue; 
        };
        
        auto out = sca::detail::fold(sum, 0, v1.begin(), v1.end());
        out = sca::detail::fold(sum, out, v2.begin(), v2.end());
        EXPECT_EQ(21, out);
    }

    {
        // sum 2 vectors simultaneously
        auto sum = [](int cur_sum, int new_is_sameue_1, int new_is_sameue_2) { 
            return cur_sum + new_is_sameue_1 + new_is_sameue_2; 
        };

        auto out = sca::detail::fold(sum, 0, v1.begin(), v1.end(), v2.begin());
        EXPECT_EQ(21, out);
    }
}
