#include <string>
#include <vector>
#include <list>
#include <forward_list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_6, map) {
    {
        const std::vector<int> v{1,2,3};
        const std::list<int> l{4,5,6};
        const std::forward_list<int> fl{7,8,9};

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
        std::vector<int> v{1,2,3};
        std::list<int> l{4,5,6};
        std::forward_list<int> fl{7,8,9};

        // mapped Callables can accept references 
        auto add_and_reset = [](int& a, int& b, int& c) {
            auto sum = a+b+c;
            a = 0;
            b = 1;
            c = 2;
            return sum;
        };

        auto out = sca::map(add_and_reset, v, l, fl);

        EXPECT_EQ(12, out[0]);
        EXPECT_EQ(15, out[1]);
        EXPECT_EQ(18, out[2]);

        EXPECT_EQ(3, sca::size(v));

        for(auto& e : v) {
            EXPECT_EQ(0, e);
        }

        EXPECT_EQ(3, sca::size(l));

        for(auto& e : l) {
            EXPECT_EQ(1, e);
        }

        EXPECT_EQ(3, sca::size(fl));

        for(auto& e : fl) {
            EXPECT_EQ(2, e);
        }
    }
}

TEST(lesson_6, fold) {
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

TEST(lesson_6, all) {
    {
        const std::vector<int> v{1,2,3,4,5,6};
        const std::vector<int> v2{2,4,6};
        const std::vector<int> v3{1,3,5};

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
        
        out = sca::all(is_odd, v);
        EXPECT_FALSE(out);

        out = sca::all(is_odd, v2);
        EXPECT_FALSE(out);

        out = sca::all(is_odd, v3);
        EXPECT_TRUE(out);
    }

    {
        const std::forward_list<std::string> l{"I", " ", "am", " ", "a", " ", "stick!"};
        const std::list<std::string> fl{"I", " ", "am", "groot", ""};

        auto not_empty = [](const std::string& e) { return e != std::string(""); };

        auto out = sca::all(not_empty, l);
        EXPECT_TRUE(out);

        out = sca::all(not_empty, fl);
        EXPECT_FALSE(out);
    }
}

TEST(lesson_6, some) {
    {
        const std::vector<int> v{1,2,3,4,5,6};
        const std::vector<int> v2{2,4,6};
        const std::vector<int> v3{1,3,5};

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
        
        out = sca::some(is_odd, v);
        EXPECT_TRUE(out);

        out = sca::some(is_odd, v2);
        EXPECT_FALSE(out);

        out = sca::some(is_odd, v3);
        EXPECT_TRUE(out);
    }

    {
        const std::forward_list<std::string> l{"I", " ", "am", " ", "a", " ", "stick!"};
        const std::list<std::string> fl{"I", " ", "am", "groot", ""};

        auto not_empty = [](const std::string& e) { return e != std::string(""); };

        auto out = sca::some(not_empty, l);
        EXPECT_TRUE(out);

        out = sca::some(not_empty, fl);
        EXPECT_TRUE(out);
    }
}

namespace lesson_6_ns {

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
        return unique_lock{std::unique_lock<MUTEX>(m_mtx), m_t};
    }

private:
    T m_t;
    MUTEX m_mtx;
};

// simplify init_thread() by using value_guard<T>
template <typename InitFunction, typename Function, typename... OptionalArgs>
std::thread init_thread2(InitFunction&& init_f, Function&& f, OptionalArgs&&... args) {
    value_guard<bool> vg(false);
    std::condition_variable cv;

    std::thread thd([=, &vg, &cv]() mutable {
        init_f();
        vg.acquire().value = true;
        cv.notify_one();
        f(std::forward<OptionalArgs>(args)...);
    });

    {
        auto lk = vg.acquire();

        while(!lk.value) {
            cv.wait(lk);
        }
    }

    return thd;
}

};

TEST(lesson_6, extra_credit_init_thread) {
    using namespace lesson_6_ns;

    for(size_t cnt = 10000; cnt; --cnt) {
        bool flag = false;
        auto init = [&]{ flag = true; };
        auto do_nothing = []{};
        auto thd = init_thread(init, do_nothing);
        EXPECT_TRUE(flag);
        thd.join();
    }
    
    for(size_t cnt = 10000; cnt; --cnt) {
        bool flag = false;
        auto init = [&]{ flag = true; };
        auto do_nothing = []{};
        auto thd = init_thread2(init, do_nothing);
        EXPECT_TRUE(flag);
        thd.join();
    }
}
