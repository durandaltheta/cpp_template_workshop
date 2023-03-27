# C++ template workshop 2023

## Introduction To Templates - building a toolbox

### When to use templates?
Shorthand Rule:
- library code should use templates in its API to improve its capabilities
- normal project code should often avoid writing custom templates

However, in cases where the code in question is any of the following then templates and inlining are often a good solution:
- frequently used algorithm follows a similar pattern with different types 
- frequently used algorithm follows a similar pattern with different executing code
- compiler maximum runtime speed optimization is required (at the expense of longer startup load times)

Furthermore, templates are ultimately just functions (that will been finalized by the compiler as needed), and can be used to also abstract difficult and/or dangerous code just like normal functions.

An example of several of the above issues: you need to launch a child thread which does some initialization but the parent thread wants to wait till the child completes initialization before moving on. 

This can happen when using `std::thread`s where a signal handler needs to be set on the child thread in a synchronized way to avoid a race condition. Unfortunately, `std::thread` automatically launches its system thread without allowing for pre-configuration of it's signal handlers. Worse still, you have to do something similar (but different) on multiple threads throughout your program!

Now you have to do some scary `std::condition_variable` blocking to wait for your child `std::thread` to complete the necessary initialization. Wouldn't it be nice to write a pattern of code which could do this *dangerous* operation *multiple times* in *different ways* that was maintainable from a *single function*?

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

std::thread g_my_child_0;
std::thread g_my_child_1;

void my_child_0_sighdl(int sig) {
    std::cout << "child 0 received signal[" << sig << "]" << std::endl;
}

void my_child_1_sighdl(int sig) {
    std::cout << "child 1 received signal[" << sig << "]" << std::endl;
}

// from https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
void set_handler(void(*sig_handler)(int)) {
    sigaction new_action, old_action;
    new_action.sa_handler = sig_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction (SIGINT, &new_action, NULL);
    }

    sigaction (SIGHUP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction (SIGHUP, &new_action, NULL);
    }

    sigaction (SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction (SIGTERM, &new_action, NULL);
    }
}

void child_func(int arg0, const char* arg1) {
    // do everything your thread needs to do...
}

void launch_my_child_threads() {
    // use some lambdas to easily call set_handler() during thread initialization
    g_my_child_0 = init_thread([]{ set_handler(my_child_0_sighdl); }, child_func, 42, "the meaning of life");
    g_my_child_1 = init_thread([]{ set_handler(my_child_1_sighdl); }, child_func, 0, "hello world");
}
```

The power of templates!

## Lesson Notes
### Basic Templates
- Basic Templates - manual type specification
- Basic Templates - type deduction
- Basic Templates - default type assignment
- Basic Templates - rvalues and lvalues
- Basic Templates - Techniques - forwarding
- Basic Templates - Techniques - type decay
- Basic Templates - inlining and compiler behavior
### SFINAE 
- SFINAE - Substitution Failure Is Not An Error
- SFINAE - Techniques - SFINAE specialization selection
- SFINAE - Techniques - SFINAE method detection
### Variadics
- Variadics - Handling any number of arguments
- Variadics - Techniques - variadic rvalue iterator trampolines
### Callables
- Callables - function pointers, functors, lambdas, std::function and you
- Callables - Techniques - SFINAE Callable argument type detection 
- Callables - Techniques - SFINAE Callable return value type detection
### Putting it all together 
- Putting it all together - map 
- Putting it all together - fold 

