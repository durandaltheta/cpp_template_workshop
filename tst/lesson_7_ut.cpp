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
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_7, fold) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_7, all) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_7, some) {
    EXPECT_TRUE(false); //TODO: implement test from main
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
