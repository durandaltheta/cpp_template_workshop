#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <thread>
#include <mutex>
#include <memory>
#include <deque>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include "scalgorithm"
#include <gtest/gtest.h> 

namespace lesson_6_ns {

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

TEST(lesson_6, concatenate) {
    using namespace lesson_6_ns;

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

TEST(lesson_6, detail_advance_group) {
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

TEST(lesson_6, each) {
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

TEST(lesson_6, detail_map) {
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

TEST(lesson_6, detail_fold) {
    std::vector<int> v1{1,2,3};
    std::vector<int> v2{4,5,6};

    {
        // sum 1 vector at a time
        auto sum = [](int cur_sum, int new_value) { 
            return cur_sum + new_value; 
        };
        
        auto out = sca::detail::fold(sum, 0, v1.begin(), v1.end());
        out = sca::detail::fold(sum, out, v2.begin(), v2.end());
        EXPECT_EQ(21, out);
    }

    {
        // sum 2 vectors simultaneously
        auto sum = [](int cur_sum, int new_value_1, int new_value_2) { 
            return cur_sum + new_value_1 + new_value_2; 
        };

        auto out = sca::detail::fold(sum, 0, v1.begin(), v1.end(), v2.begin());
        EXPECT_EQ(21, out);
    }
}

#ifdef COMPILE_EXTRA_CREDIT
namespace lesson_6_ns {

/*
This worker thread implementation has more efficiency optimizations than are 
required to showcase the behavior of scheduling arbitrary thunks. These 
optimizations are provided because we are nearing the end of the workshop and 
applications to real code are more relevant. Some methods and implementation 
details may only exist for unit testing purposes.
 */
struct job_thread { 
    // only used for unit test introspection
    struct tester {
        enum thread_state_e {
            waiting,
            working
        };

        enum schedule_state_e {
            none,
            direct,
            parameter_pack
        };

        tester() = delete;
        tester(job_thread& wt) : m_wt(wt) { }

        thread_state_e thread_state() {
            std::lock_guard<std::mutex> lk(m_wt.m_mtx);
            return m_wt.m_thread_state;
        }

        schedule_state_e schedule_state() {
            std::lock_guard<std::mutex> lk(m_wt.m_mtx);
            return m_wt.m_schedule_state;
        }

    private:
        job_thread& m_wt;
    };

    // the generic Callable thunk wrapper for work to execute on this thread
    typedef std::function<void()> job;

    // Launch a worker thread. All work successfully scheduled on it is
    // guaranteed to be executed before the object is destroyed.
    job_thread() : 
        m_running(true), 
        m_thread_state(tester::thread_state_e::waiting),
        m_schedule_state(tester::schedule_state_e::none),
        // Lambda can use default by-reference capture because m_thread's 
        // lifetime is guaranteed to be tied to the job_thread's destructor. 
        // m_thread is initialized last to ensure other members are initialized.
        m_thread([&]{
            // access to job_thread members requires a critical section 
            std::unique_lock<std::mutex> lk(m_mtx);

            // always finish any remaining work
            while(m_jobs.size()) {
                do {
                    m_thread_state = tester::thread_state_e::working;

                    {
                        // remove a job from the queue
                        auto cur_job = std::move(m_jobs.front());
                        m_jobs.pop_front();

                        // exit critical section to allow more work to be 
                        // scheduled during work execution
                        lk.unlock(); 

                        // execute the scheduled job
                        (*cur_job)(); 
                    } // current job memory is freed

                    lk.lock(); // re-enter critical section
                } while(m_jobs.size());

                // Only process this statement when we are out of work. Wait 
                // until the thread is shutdown or more work arrives. 
                while(m_running && !m_jobs.size()) {
                    m_thread_state = tester::thread_state_e::waiting;
                    // While wait() blocks lk is unlocked allowing more work to be scheduled
                    m_cv.wait(lk);
                }
            }

        }) 
    { }

    // virtual destructor ensures anyone who inherits this class will properly 
    // shutdown m_thread
    virtual ~job_thread() {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            // inform m_thread to shutdown
            m_running = false;
        }

        // wake up m_thread
        m_cv.notify_one();
        
        // wait until underlying system thread ends
        m_thread.join();
    }
  
    // directly convert Callables to thunks where possible rather than 
    // double wrapping the code with a thunk and lambda 
    template <typename F>
    void schedule(F&& f) {
        m_schedule_state = tester::schedule_state_e::direct;
        schedule(std::unique_ptr<job>(new job(std::forward<F>(f))));
    }

    // wrap non-thunk Callables as a job via templated lambda capture
    template <typename F, typename A, typename... As>
    void schedule(F&& f, A&& a, As&&... as) {
        m_schedule_state = tester::schedule_state_e::parameter_pack;
        // lambda capture could potentially use perfect forwarding in c++14 via capture initializers
        schedule(std::unique_ptr<job>(new job([=]() mutable { 
            f(std::forward<A>(a), std::forward<As>(as)...); 
        })));
    }

private:
    // Schedule an allocated job to ensure that job copies are only deep 
    // copied once. This ensures queue pushes and pops trigger minimal overhead 
    // in the case that the wrapped Callable is large and/or does not implement 
    // an efficient rvalue constructor.
    void schedule(std::unique_ptr<job>&& j) {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_jobs.push_back(std::move(j));
        }

        m_cv.notify_one();
    }

    std::mutex m_mtx;
    bool m_running; 
    tester::thread_state_e m_thread_state; // used only for unit test introspection
    tester::schedule_state_e m_schedule_state; // used only for unit test introspection
    std::condition_variable m_cv;
    std::deque<std::unique_ptr<job>> m_jobs; 
    std::thread m_thread;
};

};

/*
EXTRA CREDIT 
Implement the missing body of the following methods:
- template <typename F> void lesson_6_ns::job_thread::schedule(F&&)
- template <typename F, typename A, typename... As> void lesson_6_ns::schedule(F&&, A&&, As&&...) 

So that each calls private method:
- void schedule(std::unique_ptr<job>&&) 

schedule(F&&) variation should also:
- assign tester::schedule_state_e::direct to m_schedule_state

schedule(F&&, A&&, As&&...) variation should also:
- assign tester::schedule_state_e::parameter_pack to m_schedule_state
 */
TEST(lesson_6, extra_credit_job_thread_scheduling) {
    using namespace lesson_6_ns;

    auto get_dur = [](size_t milli) {
        return std::chrono::milliseconds(milli);
    };

    std::mutex mtx;
    size_t count = 0;

    auto increment_count = [&]{ 
        std::lock_guard<std::mutex> lk(mtx);
        ++count;
    };

    auto sleep_for = [&](size_t milli) {
        increment_count();
        auto dur = get_dur(milli);
        std::this_thread::sleep_for(dur);
    };

    job_thread jthd;
    job_thread::tester tester(jthd);

    EXPECT_EQ(0, count);
    EXPECT_EQ(job_thread::tester::schedule_state_e::none, tester.schedule_state());
    EXPECT_EQ(job_thread::tester::thread_state_e::waiting, tester.thread_state());

    jthd.schedule(increment_count);
    auto dur = get_dur(500);
    std::this_thread::sleep_for(dur);
    EXPECT_EQ(job_thread::tester::schedule_state_e::direct, tester.schedule_state());
    EXPECT_EQ(job_thread::tester::thread_state_e::waiting, tester.thread_state());
    EXPECT_EQ(1, count);

    jthd.schedule(sleep_for, 1000);
    dur = get_dur(500);
    std::this_thread::sleep_for(dur);
    EXPECT_EQ(2, count);
    EXPECT_EQ(job_thread::tester::schedule_state_e::parameter_pack, tester.schedule_state());
    EXPECT_EQ(job_thread::tester::thread_state_e::working, tester.thread_state());

    dur = get_dur(2000);
    std::this_thread::sleep_for(dur);
    EXPECT_EQ(2, count);
    EXPECT_EQ(job_thread::tester::schedule_state_e::parameter_pack, tester.schedule_state());
    EXPECT_EQ(job_thread::tester::thread_state_e::waiting, tester.thread_state());

    jthd.schedule(increment_count);
    dur = get_dur(500);
    std::this_thread::sleep_for(dur);
    EXPECT_EQ(3, count);
    EXPECT_EQ(job_thread::tester::schedule_state_e::direct, tester.schedule_state());
    EXPECT_EQ(job_thread::tester::thread_state_e::waiting, tester.thread_state());
}
#endif
