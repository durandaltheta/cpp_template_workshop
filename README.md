# cpp_template_workshop_2023

## Introduction To Templates - building a toolbox

### When to use templates?
Shorthand Rule:
- library code should use templates in its API to improve its capabilities
- normal project code should often avoid writing templates

However, in cases where the code in question is any two of the following then templates are often a good solution:
- difficult
- dangerous
- frequently used algorithm follows a similar pattern with different types 
- frequently used algorithm follows a similar pattern with different executing code

An example of several of the above: you need to launch a child thread which does some initialization. However, the parent thread wants to wait till the child completes initialization before moving on. 

This can happen when using `std::thread`s where there is some operation on the underlying `pthread` that must be completed. Now you are in a pickle, `std::thread`s automatically launch their system `pthread` without allowing for pre-configuration of it's attribute values. You could directly use `pthread` operations but you want the runtime simplicity `std::thread` in the rest of your program, and would like to avoid poluting your code with a combination of both `std::thread` and `pthread`. Worse still, you have to do something similar (but different) on multiple threads throughout your program!

You decide that sticking with `std::thread` is worth it but now you have to do some scary `std::condition_variable` blocking to wait for your child `std::thread` to complete the necessary initialization. Wouldn't it be nice to write a pattern of code which could do this *dangerous* operation *multiple times* in *different ways*?

Example Solution:
```
// in some header 
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

template <typename InitFunction, typename Function, typename... OptionalArgs>
std::thread init_thread(InitFunction&& init_f, Function&& f, OptionalArgs&&... args) :
{
    // figure out the scary synchronization once
    std::mutex mtx;
    std::condition_variable cv;
    bool flag = false

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

    return std::move(thd);
}
```

Then you can use your new (and safe!) function throughout your program:
```
#include <pthread.h>
#include <unistd.h>
#include <thread>
#include "some_header_with_your_init_thread_template.hpp"

// set your thread affinity, from https://stackoverflow.com/a/11583550
int stick_this_thread_to_core(int core_id) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
   if (core_id < 0 || core_id >= num_cores)
      return EINVAL;

   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

void child_thread_function(int arg0, const char* arg1) {
    // do everything your thread needs to do...
}

std::thread g_my_child_0;
std::thread g_my_child_1;

void launch_my_child_threads() {
    // use some lambdas to call stick_this_thread_to_core() during thread initialization
    g_my_child_0 = init_thread([]{ stick_this_thread_to_core(0); }, child_thread_function, 42, "the meaning of life");
    g_my_child_1 = init_thread([]{ stick_this_thread_to_core(1); }, child_thread_function, 0, "hello world");
}
```

The power of templates!


## Lesson Notes
- Basic Templates
- Basic Templates - type deduction
- Basic Templates - default arguments
- Basic Templates - rvalues and lvalues
- Basic Templates - Techniques - forwarding
- Basic Templates - Techniques - type decay
- SFINAE - "Substitution Failure Is Not An Error"
- SFINAE - Techniques - SFINAE specialization
- SFINAE - Techniques - SFINAE method detection
- Variadics
- Variadics - Techniques - variadic rvalue iterator trampolines
- Callables
- Callables - Techniques - SFINAE Callable argument type detection 
- Callables - Techniques - SFINAE Callable return value type detection
- Putting it all together - map 
- Putting it all together - fold 

