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
#include "scalgorithm"
#include <gtest/gtest.h> 

TEST(lesson_6, concatenate) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_6, detail_advance_group) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_6, each) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_6, detail_map) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

TEST(lesson_6, detail_fold) {
    EXPECT_TRUE(false); //TODO: implement test from main
}

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
        //TODO: implement missing body
    }

    // wrap non-thunk Callables as a job via templated lambda capture
    template <typename F, typename A, typename... As>
    void schedule(F&& f, A&& a, As&&... as) {
        //TODO: implement missing body
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
