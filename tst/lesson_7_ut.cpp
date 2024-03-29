#include <string>
#include <vector>
#include <list>
#include <forward_list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_7, map) {
    const std::vector<int> v{1,2,3};
    const std::list<int> l{4,5,6};
    const std::forward_list<int> fl{7,8,9};

    {
        auto out = sca::map([](int a, int b, int c) { return a+b+c; }, v, l, fl);
        auto is_same = std::is_same<std::vector<int>,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_EQ(12, out[0]);
        EXPECT_EQ(15, out[1]);
        EXPECT_EQ(18, out[2]);

        auto pv = sca::pointers(v);
        auto get_pointer = [](const int& e) { return &e; };
        auto gen_pv = sca::map(get_pointer, v);
        EXPECT_EQ(pv, gen_pv);
    }

    {
        // copies
        auto cpv = v;
        auto cpl = l;
        auto cpfl = fl;

        // mapped Callables can accept references 
        auto add_and_reset = [](int& a, int& b, int& c) {
            auto sum = a+b+c;
            a = 0;
            b = 1;
            c = 2;
            return sum;
        };

        auto out = sca::map(add_and_reset, cpv, cpl, cpfl);

        EXPECT_EQ(12, out[0]);
        EXPECT_EQ(15, out[1]);
        EXPECT_EQ(18, out[2]);

        EXPECT_EQ(3, sca::size(cpv));

        for(auto& e : cpv) {
            EXPECT_EQ(0, e);
        }

        EXPECT_EQ(3, sca::size(cpl));

        for(auto& e : cpl) {
            EXPECT_EQ(1, e);
        }

        EXPECT_EQ(3, sca::size(cpfl));

        for(auto& e : cpfl) {
            EXPECT_EQ(2, e);
        }
    }
}

TEST(lesson_7, fold) {
    {
        const std::vector<int> v{1,2,3};
        const std::list<int> l{4,5,6};
        const std::forward_list<int> fl{7,8,9};

        auto sum = [](int cur, int a, int b, int c) {
            return cur + a + b + c;
        };

        auto out = sca::fold(sum, 0, v, l, fl);
        auto is_same = std::is_same<int,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_EQ(45, out);
    }

    {
        const std::vector<std::string> v{"I", "am", "a", "stick"};
        const std::forward_list<std::string> fl{" ", " ", " ", "!"};

        auto concatenate = [](std::string cur, const std::string& ve, const std::string& fle) {
            return cur + ve + fle;
        };

        auto out = sca::fold(concatenate, std::string(""), v, fl);
        auto is_same = std::is_same<std::string,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_EQ(std::string("I am a stick!"), out);
    }
}

TEST(lesson_7, all) {
    {
        const std::vector<int> v{1,2,3,4,5,6};
        const std::vector<int> v2{2,4,6};
        const std::vector<int> v3{1,3,5};
        const std::vector<int> v4{};

        auto is_even = [](int i) { return i % 2 == 0; };
        auto is_odd = [](int i) { return i % 2 != 0; };

        auto out = sca::all(is_even, v);
        auto is_same = std::is_same<bool,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_FALSE(out);

        out = sca::all(is_even, v2);
        EXPECT_TRUE(out);

        out = sca::all(is_even, v3);
        EXPECT_FALSE(out);

        out = sca::all(is_even, v4);
        EXPECT_TRUE(out);
        
        out = sca::all(is_odd, v);
        EXPECT_FALSE(out);

        out = sca::all(is_odd, v2);
        EXPECT_FALSE(out);

        out = sca::all(is_odd, v3);
        EXPECT_TRUE(out);

        out = sca::all(is_odd, v4);
        EXPECT_TRUE(out);
    }

    {
        const std::forward_list<std::string> fl{"I", " ", "am", " ", "a", " ", "stick!"};
        const std::list<std::string> l{"I", " ", "am", "groot", ""};
        const std::vector<std::string> v{"","",""};
        const std::vector<std::string> ve{};

        auto not_empty = [](const std::string& e) { return e != std::string(""); };

        auto out = sca::all(not_empty, fl);
        EXPECT_TRUE(out);

        out = sca::all(not_empty, l);
        EXPECT_FALSE(out);
        
        out = sca::all(not_empty, v);
        EXPECT_FALSE(out);
        
        out = sca::all(not_empty, ve);
        EXPECT_TRUE(out);
    }
}

TEST(lesson_7, some) {
    {
        const std::vector<int> v{1,2,3,4,5,6};
        const std::vector<int> v2{2,4,6};
        const std::vector<int> v3{1,3,5};
        const std::vector<int> v4{};

        auto is_even = [](int i) { return i % 2 == 0; };
        auto is_odd = [](int i) { return i % 2 != 0; };

        auto out = sca::some(is_even, v);
        auto is_same = std::is_same<bool,decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_TRUE(out);

        out = sca::some(is_even, v2);
        EXPECT_TRUE(out);

        out = sca::some(is_even, v3);
        EXPECT_FALSE(out);

        out = sca::some(is_even, v4);
        EXPECT_FALSE(out);
        
        out = sca::some(is_odd, v);
        EXPECT_TRUE(out);

        out = sca::some(is_odd, v2);
        EXPECT_FALSE(out);

        out = sca::some(is_odd, v3);
        EXPECT_TRUE(out);

        out = sca::some(is_odd, v4);
        EXPECT_FALSE(out);
    }

    {
        const std::forward_list<std::string> fl{"I", " ", "am", " ", "a", " ", "stick!"};
        const std::list<std::string> l{"I", " ", "am", "groot", ""};
        const std::vector<std::string> v{"","",""};
        const std::vector<std::string> ve{};

        auto not_empty = [](const std::string& e) { return e != std::string(""); };

        auto out = sca::some(not_empty, fl);
        EXPECT_TRUE(out);

        out = sca::some(not_empty, l);
        EXPECT_TRUE(out);
        
        out = sca::some(not_empty, v);
        EXPECT_FALSE(out);

        out = sca::some(not_empty, ve);
        EXPECT_FALSE(out);
    }
}
