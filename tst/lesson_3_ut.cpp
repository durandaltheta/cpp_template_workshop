#include <string>
#include <gtest/gtest.h> 

namespace lesson_3_ns {

const int INT_3 = 3;
const char* STR_FOO = "foo";
size_t g_function_hit = 0;

struct packaged_void_pointer {
    void (*handler)(void*) = nullptr;
    void* data = nullptr;
};

// agnostically handle data
void execute_packaged_void_pointer_handler(packaged_void_pointer pvp) {
    pvp.handler(pvp.data);
}

void expect_int_3(void* v) {
    EXPECT_EQ(0, g_function_hit);
    g_function_hit = 1;
    EXPECT_EQ((void*)&INT_3, v); // pointer comparison
    EXPECT_EQ(INT_3, *((const int*)v)); // value comparison
}

void expect_string_foo(void* v) {
    EXPECT_EQ(0, g_function_hit);
    g_function_hit = 2;
    EXPECT_EQ((void*)STR_FOO, v); // pointer comparison
    EXPECT_EQ(std::string(STR_FOO), std::string((const char*)v)); // value comparison
}

}

TEST(lesson_3, packaged_void_pointer) {
    using namespace lesson_3_ns;

    const int* i = &INT_3;
    const char* s = STR_FOO;
    packaged_void_pointer pvp;

    EXPECT_EQ(nullptr, pvp.handler);
    EXPECT_EQ(nullptr, pvp.data);
    EXPECT_EQ(0, g_function_hit);
    g_function_hit = 0;

    pvp.handler = expect_int_3;
    pvp.data = (void*)i;
    execute_packaged_void_pointer_handler(pvp);
    EXPECT_EQ(1, g_function_hit);
    g_function_hit = 0;

    pvp.handler = expect_string_foo;
    pvp.data = (void*)s;
    execute_packaged_void_pointer_handler(pvp);
    EXPECT_EQ(2, g_function_hit);
    g_function_hit = 0;
}

namespace lesson_3_ns {

enum types {
    is_unknown,
    is_int,
    is_string
};

struct void_pointer_with_id {
    size_t id = types::is_unknown;
    void* data = nullptr;
};

size_t g_switch_hit = 0;

void unwrap_void_pointer_with_id(void_pointer_with_id vpwi) {
    EXPECT_EQ(0, g_switch_hit);

    switch(vpwi.id) {
        case types::is_int:
            g_switch_hit = 1;
            EXPECT_EQ((void*)&INT_3, vpwi.data); // pointer comparison
            EXPECT_EQ(INT_3, *((const int*)(vpwi.data))); // value comparison 
            break;
        case types::is_string:
            g_switch_hit = 2;
            EXPECT_EQ((void*)STR_FOO, vpwi.data); // pointer comparison
            EXPECT_EQ(std::string(STR_FOO), std::string((const char*)(vpwi.data))); // value comparison
            break;
        case types::is_unknown:
        default:
            g_switch_hit = 3;
            EXPECT_EQ(nullptr, vpwi.data);
            break;
    }
}

}

TEST(lesson_3, void_pointer_with_id) {
    using namespace lesson_3_ns;

    const int* i = &INT_3;
    const char* s = STR_FOO;
    void_pointer_with_id vpwi;

    unwrap_void_pointer_with_id(vpwi);
    EXPECT_EQ(3, g_switch_hit);
    g_switch_hit = 0;

    vpwi.id = types::is_int;
    vpwi.data = (void*)i;
    unwrap_void_pointer_with_id(vpwi);
    EXPECT_EQ(1, g_switch_hit);
    g_switch_hit = 0;

    vpwi.id = types::is_string;
    vpwi.data = (void*)s;
    unwrap_void_pointer_with_id(vpwi);
    EXPECT_EQ(2, g_switch_hit);
    g_switch_hit = 0;

    vpwi.id = types::is_string+1;
    vpwi.data = nullptr;
    unwrap_void_pointer_with_id(vpwi);
    EXPECT_EQ(3, g_switch_hit);
    g_switch_hit = 0;
}

namespace lesson_3_ns {

// A struct which can hold a pointer to any value
struct wrapped_value {
    // assign a value to this value wrapper
    template <typename T>
    void set(T& t) {
        ptr = &t;
        tip = &typeid(typename std::decay<T>);
    }

    // return `true` if a value is assigned and the value type matches `T`, else return `false`
    template <typename T>
    bool is() {
        return ptr != nullptr && *tip == typeid(typename std::decay<T>);
    }

    // return a reference to the assigned value
    template <typename T>
    T& to() {
        return *(static_cast<T*>(ptr));
    }

private:
    void* ptr = nullptr;
    const std::type_info* tip;
};

bool expect_bool(wrapped_value wv, bool expected) {
    return wv.is<bool>() && expected == wv.to<bool>();
};

bool expect_int(wrapped_value wv, int expected) {
    return wv.is<int>() && expected == wv.to<int>();
};

bool expect_string(wrapped_value wv, std::string expected) {
    return wv.is<std::string>() && expected == wv.to<std::string>();
};

}

TEST(lesson_3, wrapped_value) {
    using namespace lesson_3_ns;

    wrapped_value wv;
    bool b = true;
    int i = 31;
    std::string s("foo");
    
    EXPECT_FALSE(expect_bool(wv, false));
    EXPECT_FALSE(expect_bool(wv, true));
    EXPECT_FALSE(expect_int(wv, 0));
    EXPECT_FALSE(expect_int(wv, 31));
    EXPECT_FALSE(expect_string(wv, std::string("faa")));
    EXPECT_FALSE(expect_string(wv, std::string("foo")));

    wv.set(b);
    EXPECT_FALSE(expect_bool(wv, false));
    EXPECT_TRUE(expect_bool(wv, true));
    EXPECT_FALSE(expect_int(wv, 0));
    EXPECT_FALSE(expect_int(wv, 31));
    EXPECT_FALSE(expect_string(wv, std::string("faa")));
    EXPECT_FALSE(expect_string(wv, std::string("foo")));

    wv.set(i);
    EXPECT_FALSE(expect_bool(wv, false));
    EXPECT_FALSE(expect_bool(wv, true));
    EXPECT_FALSE(expect_int(wv, 0));
    EXPECT_TRUE(expect_int(wv, 31));
    EXPECT_FALSE(expect_string(wv, std::string("faa")));
    EXPECT_FALSE(expect_string(wv, std::string("foo")));

    wv.set(s);
    EXPECT_FALSE(expect_bool(wv, false));
    EXPECT_FALSE(expect_bool(wv, true));
    EXPECT_FALSE(expect_int(wv, 0));
    EXPECT_FALSE(expect_int(wv, 31));
    EXPECT_FALSE(expect_string(wv, std::string("faa")));
    EXPECT_TRUE(expect_string(wv, std::string("foo")));
}
