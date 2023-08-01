#include <string>
#include <gtest/gtest.h> 

TEST(lesson_3, packaged_void_pointer) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_3, void_pointer_with_id) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_3, wrapped_value) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

namespace lesson_3_ns {

struct safe_wrapped_value {
    template <typename T>
    void set(T& t) { }

    template <typename T>
    bool get(T& t) {
        return false;
    }
};

bool expect_bool(safe_wrapped_value swv, bool expected) {
    bool actual = false;
    bool success = swv.get(actual);
    return success && expected == actual;
};

bool expect_int(safe_wrapped_value swv, int expected) {
    int actual = 0;
    bool success = swv.get(actual);
    return success && expected == actual;
};

bool expect_string(safe_wrapped_value swv, std::string expected) {
    std::string actual;
    bool success = swv.get(actual);
    return success && expected == actual;
};

};

/*
EXTRA CREDIT
Implement the lesson_3_ns::safe_wrapped_value such that: 
void lesson_3_ns::safe_wrapped_value::set(T&): assign argument reference to an internal wrapped_value
bool lesson_3_ns::safe_wrapped_value::get(T&): if type T matches stored value assign stored value to argument reference and return `true`, else returns `false`
 */
TEST(lesson_3, extra_credit_safe_wrapped_value) {
    using namespace lesson_3_ns;

    safe_wrapped_value swv;
    bool b = true;
    int i = 31;
    std::string s("foo");
    
    EXPECT_FALSE(expect_bool(swv, false));
    EXPECT_FALSE(expect_bool(swv, true));
    EXPECT_FALSE(expect_int(swv, 0));
    EXPECT_FALSE(expect_int(swv, 31));
    EXPECT_FALSE(expect_string(swv, std::string("faa")));
    EXPECT_FALSE(expect_string(swv, std::string("foo")));

    swv.set(b);
    EXPECT_FALSE(expect_bool(swv, false));
    EXPECT_TRUE(expect_bool(swv, true));
    EXPECT_FALSE(expect_int(swv, 0));
    EXPECT_FALSE(expect_int(swv, 31));
    EXPECT_FALSE(expect_string(swv, std::string("faa")));
    EXPECT_FALSE(expect_string(swv, std::string("foo")));

    swv.set(i);
    EXPECT_FALSE(expect_bool(swv, false));
    EXPECT_FALSE(expect_bool(swv, true));
    EXPECT_FALSE(expect_int(swv, 0));
    EXPECT_TRUE(expect_int(swv, 31));
    EXPECT_FALSE(expect_string(swv, std::string("faa")));
    EXPECT_FALSE(expect_string(swv, std::string("foo")));

    swv.set(s);
    EXPECT_FALSE(expect_bool(swv, false));
    EXPECT_FALSE(expect_bool(swv, true));
    EXPECT_FALSE(expect_int(swv, 0));
    EXPECT_FALSE(expect_int(swv, 31));
    EXPECT_FALSE(expect_string(swv, std::string("faa")));
    EXPECT_TRUE(expect_string(swv, std::string("foo")));
}
