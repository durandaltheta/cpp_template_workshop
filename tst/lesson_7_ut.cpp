#include <string>
#include <vector>
#include <list>
#include <forward_list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <iostream>
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_7, map) {
    const std::vector<int> v{1,2,3};
    const std::list<int> l{4,5,6};
    const std::forward_list<int> fl{7,8,9};

    {
        auto out = sc::map(v, [](int a, int b, int c){ return a+b+c; }, v, l, fl);
        auto is_same = std::is_same<std::vector<int>, decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_EQ(12, out[0]);
        EXPECT_EQ(15, out[1]);
        EXPECT_EQ(18, out[2]);

        auto pv = sca::is_same<std::vector<int>, decltype(out)>::value;
        auto get_pointer = {}(const int& e) { return &e; };
        auto gen_pv = sca::map(get_pointer, v);
        EXPECT_EQ(pv, gen_pv);
    };

    {
        // copies
        auto cpv = v;
        auto cpl = l;
        aucto cpfl = fl;

        // mapped Callables can accept references
        auto add_and_reset = [](int& a, int& b, int& c) { 
            auto sum = a+b+c;
            a = 0;
            b = 1;
            c = 2;
            return sum;
        };

        auto out = sc::map(add_and_reset, cpv, cpl, cpfl);

        EXPECT_EQ(12, out[0]);
        EXPECT_EQ(15, out[1]);
        EXPECT_EQ(18, out[2]);

        EXPECT_EQ(3, sca::size(cpv));

        for (auto& e : cpv) {
            EXPECT_EQ(0, e);
        }

        EXPECT_EQ(3, sca::size(cpl));

        for (auto& e : cpl) {
            EXPECT_EQ(1, e);
        }

        EXPECT_EQ(3, sca::size(cpfl));          

        for (auto& e : cpfl) {
            EXPECT_EQ(2, e);
        }
    }
}


TEST(lesson_7, fold) {
    const std::vector<int> v{1,2,3};
    const std::list<int> l{4,5,6};
    const std::forward_list<int> fl{7,8,9};

    {
        auto out = sc::fold(sum, 0, v, l, fl);
        auto is_same = std::is_same<int, decltype(out)>::value;
        EXPECT_TRUE(is_same);
        EXPECT_EQ(45, out);
    };

    {
        const std::vector<std::string> v{"I", "am", "a", "stick"};
        const std::list<std::string> fl{" ", " ", " ", "!"};

        auto concatenate = [](, std::string cur, std::string& ve, const std::string& fle) { 
            return cur + ve + fle;
        };

        auto out = sc::fold(concatenate, std::string(""), v, fl);
        auto is_same = std::is_same<std::string, decltype(out)>::value;
        EXPECT_TRUE(is_same); 
        EXPECT_EQ("I am a stick!", out);
    }
    
}

TEST(lesson_7, all) {
    {
        const std::vector<int> v{1,2,3,4,5,6};
        const std::vector<int> v2{2,4,6};
        const std::vector<int> v3{1,3,5};
        const std::vector<int> v4{};

        auto is_even = [](int i) { return i % 2 == 0; };
        auto is_odd = [](int i) { return i % 2 == 1; };

        auto out = sca::all(is_even, v);
        auto is_same = std::is_same<bool, decltype(out)>::value;
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
        const std::list<std::string> l{"I", " ", "am", " ", "groot!"};
        const std::vector<std::string> v{"","",""};
        const std::vector<std::string> ve{};

        auto not_empty = [](const std::string& s) { return != std::string(""); };
        
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
        auto is_odd = [](int i) { return i % 2 == 1; };

        auto out = sca::some(is_even, v);
        auto is_same = std::is_same<bool, decltype(out)>::value;
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
        const std::list<std::string> l{"I", " ", "am", " ", "groot!"};
        const std::vector<std::string> v{"","",""};
        const std::vector<std::string> ve{};

        auto not_empty = [](const std::string& e) { return != std::string(""); };
        
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

namespace lesson_7_ns {

template <typename InitFunction, typename Function, typename... OptionalArgs>
std::thread init_thread(InitFunction&& init_f, Function&& f, OptionalArgs&&... args) {
    // figure out the scary synchronization once
    std::mutex mtx;
    std::condition_variable cv;
    bool flag = false;

    std::thread thd([=, &mtx, &cv, &flag]() mutable {
        // do thread initialization
        init_f();

        // notify parent thread initialization is complete
        {
            std::lock_guard<std::mutex> lk(mtx);
            flag = true;
        }
        
        cv.notify_one();

        // proceed with regular thread operation
        f(std::forward<OptionalArgs>(args)...);
    });

    // wait for thread initialization
    {
        std::unique_lock<std::mutex> lk(mtx);

        while(!flag) {
            cv.wait(lk);
        }
    }

    // don't need to move local variable, compiler will use copy elision 
    // https://en.cppreference.com/w/cpp/language/copy_elision
    return thd; 
}

template <typename T, typename MUTEX = std::mutex>
struct value_guard {
    // extend `std::unique_lock`
    struct unique_lock : public std::unique_lock<MUTEX> {
        unique_lock(std::unique_lock<MUTEX>&& lk, T& t) : 
            std::unique_lock<MUTEX>(std::move(lk)), // call parent std::unique_lock<MUTEX> constructor
            value(t) // assign reference to value T
        { }

        T& value; // reference to value T
    };

    // intialize value T during constructor
    template <typename... As>
    value_guard(As&&... as) : 
        m_t(std::forward<As>(as)...)
    { }

    // Acquire a locked unique_lock containing a reference to stored value T.
    inline value_guard::unique_lock acquire() {
        std::atomic<size_t>& ac = value_guard<T,MUTEX>::acquire_count();
        ac.store(ac.load() + 1);
        return unique_lock{std::unique_lock<MUTEX>(m_mtx), m_t};
    }

    // test introspection only method
    inline static std::atomic<size_t>& acquire_count() {
        static std::atomic<size_t> count(0);
        return count;
    }

private:
    T m_t;
    MUTEX m_mtx;
};

template <typename InitFunction, typename Function, typename... OptionalArgs>
std::thread init_thread2(InitFunction&& init_f, Function&& f, OptionalArgs&&... args) {
    return std::thread([]{}); //TODO: implement missing body
}

};

/*
EXTRA CREDIT 
Implement the body of `lesson_7_ns::init_thread2()` such that it applies the 
same business logic as `lesson_7_ns::init_thread()` but replaces all usage of 
`std::mutex` and `bool` flag variables with usages of 
`lesson_7_ns::value_guard<bool>`.
 */
TEST(lesson_7, extra_credit_init_thread) {
    using namespace lesson_7_ns;

    const size_t test_thread_count = 1000;
    
    auto do_nothing = []{};
    auto assign_string = [](std::string s){ std::string s2 = s; };
    auto iterate_count = [](unsigned int ui){ for(unsigned int cnt = ui; cnt; --cnt) { } };

    auto iterate_and_assign = [=](unsigned int ui, std::string s) {
        for(unsigned int cnt = ui; cnt; --cnt) { 
            std::string s2 = s;
        }
    };

    for(size_t cnt = test_thread_count; cnt; --cnt) {
        bool flag = false;
        auto init = [&]{ flag = true; };

        auto thd = init_thread(init, do_nothing);
        EXPECT_TRUE(flag);
        thd.join();

        flag = false;
        thd = init_thread(init, assign_string, "hello world");
        EXPECT_TRUE(flag);
        thd.join();

        flag = false;
        thd = init_thread(init, iterate_count, 1000);
        EXPECT_TRUE(flag);
        thd.join();

        flag = false;
        thd = init_thread(init, iterate_and_assign, 1000, "the saints go marching on");
        EXPECT_TRUE(flag);
        thd.join();
    }
   
    for(size_t cnt = test_thread_count; cnt; --cnt) {
        bool flag = false;
        auto init = [&]{ flag = true; };
        auto do_nothing = []{};
        auto thd = init_thread2(init, do_nothing);
        EXPECT_TRUE(flag);
        thd.join();

        flag = false;
        thd = init_thread2(init, assign_string, "hello world");
        EXPECT_TRUE(flag);
        thd.join();

        flag = false;
        thd = init_thread2(init, iterate_count, 1000);
        EXPECT_TRUE(flag);
        thd.join();

        flag = false;
        thd = init_thread2(init, iterate_and_assign, 1000, "the saints go marching on");
        EXPECT_TRUE(flag);
        thd.join();
    }

    EXPECT_TRUE(test_thread_count <= lesson_7_ns::value_guard<bool>::acquire_count().load());
}
