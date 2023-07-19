# C++ template workshop 2023

## Introduction to templates - building a toolbox 
Templates are a very powerful tool for writing code in c++. They allow you to do just about everything a C macro would allow (and more!), but with type safety and namespace awareness. My goal in this workshop is to give my knowledge of c++ templates away so that others might benefiti and significantly advance their understanding of c++. 

Even if you never write a single template in production code (though you probably will), this knowledge is *extremely* useful for reading and understanding existing template code, including standard library code. I did not become an expert in the standard library until I learned templates, for the simple reason that the standard library uses them all over the place :D.

Be warned, this is *not* a templates basics course. This course will cover advanced templating, though the first couple of lessons will cover the basics. The intended audience for this course is *advanced c++ developers*. 

### A note on course design 
I modeled this course after my favorite programming book [Learn Python the Hard Way](https://www.webpages.uidaho.edu/~stevel/504/LearnPythonTheHardWay.pdf). Said book is for very inexperienced students looking to learn the Python programming language. However, I was so impressed by how *successful* the book was at educating readers that I decided to steal his ideas :).

It's true the intended audience of this workshop is not c++ beginners. However, many (or all) will be beginners when it comes to c++ templating, which is a very niche skill :). Here's the opening paragraph from his book, which contains the core ideas I'm seeking to emulate:

> The Hard Way Is Easier
> 
> This simple book is meant to get you started in programming. The title says it’s the hard way to learn to write code;
> but it’s actually not. It’s only the “hard” way because it’s the way people used to teach things. With the help of this
> book, you will do the incredibly simple things that all programmers need to do to learn a language:
> 
> 1. Go through each unit test (written using [googletest](https://google.github.io/googletest/primer.html))
> 2. Type in each sample exactly.
> 3. Make it run.
> 
> That’s it. This will be very difficult at first, but stick with it. If you go through this book, and do each unit test for
> one or two hours a night, you will have a good foundation for moving onto another book. You might not really learn
> “programming” from this book, but you will learn the foundation skills you need to start learning the language.
> This book’s job is to teach you the three most essential skills that a beginning programmer needs to know: Reading
> and Writing, Attention to Detail, Spotting Differences.

### What is expected of participants 
Each participant will be expected to get a series of unit tests passing on a (remote) branch. The `main` branch of this code repository contains *the complete working algorithm code and implemented unit tests*. There is a secondary branch named `student` with much of the algorithm and unit test code either absent or commented out, create your branch from that branch with: 
```
git clone --recurse-submodules git@github.com:durandaltheta/cpp_template_workshop_2023.git && git checkout student && git checkout -b your_branch_name && git push --set-upstream origin your_branch_name
```

NOTE: as implied by the `git` argument `--recurse-submodules`, this repo uses submodules, so clones of this repo will always require `--recurse-submodules` argument if you want unit tests to work :D.

This repository has github actions setup to automatically build and run unit tests on each branch. Therefore, no local editor and tooling is technically necessary, you can simply edit your branch in the browser and commit most of the time and see the results. It will probably be faster to develop, build and test locally though.

I have provided the solutions for all code in the `main` branch. It is *intended* that you use this branch as reference. I do not want you to write your own solutions (though I have no problem if you do that on your own time). I literally expect participants to do the following:
- open `main` branch in their browser on right side of your screen (or other monitor)
- open your branch in your code editor on the left side of your screen (or other monitor)
- type *BY HAND* each unit test solution (and relevant algorithm implementation in `inc/algorithm.hpp` or `inc/detail/algorithm.hpp`), into your branch
    - *NO COPY PASTE*. The point of this unit test is to force all parts of your brain to engage with the learning process
- optionally, compile and run the unit tests locally with `cmake . && make cpp_template_2023_ut && tst/cpp_template_workshop_ut`
    - a single unit test can be run with `tst/cpp_template_workshop_ut unit_test_name`
- `git add`, `commit`, and `push` your changes to your remote branch 
- see if the github action succeeds in compiling and the relevant unit tests pass


unit tests can be run an built locally assuming you have `cmake` and a `c++` compiler installed which supports `c++17`. To configure build:
```
cd /path/to/checkout/directory
cmake .
```

To build and run unit tests:
```
make cpp_template_workshop_ut 
./tst/cpp_template_workshop_ut
```

### Grading
Grading for this workshop reflects Elektrobit's employee yearly performance goals scale of 1-5 (1 is bad, 3 is you did your job, 5 is perfect). As such, your score, if used by your manager to influence any of your yearly goals, can easily reason about how well you did and how they should adjust your end of year scores.

- a branch checkout in their user name exists at end of workshop: 1 point
    - if no branch checkout in their user name exists at the end of the workshop I will assume the user has not participated. This means no score, good or bad, will be forwarded to any line manager
- implement solution provided unit tests
    - every 25% of unit tests passing grants 1 point (total of 4 points)

Possible point total: 5

### Template theory - when to use templates? 
Here are my opinions on the topic:

Shorthand Rule:
- library code should use templates in its API to improve its capabilities
- normal project code should often avoid writing custom templates

However, in cases where the code in question is any of the following then templates and inlining are often a good solution, even in normal project code:
- function API can handle a variable number of arguments
- frequently used algorithm follows a similar pattern with different types 
- frequently used algorithm follows a similar pattern which wraps executing code inside another function
- compiler maximum runtime speed optimization is required (at the expense of potentially longer startup load times)

Additionally, templates are just functions (that will been finalized by the compiler as needed), and can be used to write difficult and/or dangerous code just like normal functions. This means, more generally, that *templates expand your ability to abstract code* by leveraging their expanded featureset.

An example of several of the above issues: you need to launch a child thread which does some initialization but the parent thread wants to wait till the child completes initialization before moving on. 

This can happen when using `std::thread`s where a signal handler needs to be set on the child thread in a synchronized way to avoid a race condition. Unfortunately, `std::thread` automatically launches its system thread without allowing for pre-configuration of it's signal handlers. Worse still, you have to do something similar (but different) on multiple threads throughout your program!

Now you have to do some scary `std::condition_variable` blocking to wait for your child `std::thread` to complete the necessary initialization. Wouldn't it be nice to write a pattern of code which could do this *dangerous* operation *multiple times* in *different ways* with an *undefined number of arguments* that was maintainable from a *single template function*?

Example Solution (if this doesn't make sense right away, consider coming back here throughout the workshop to re-examine with your new knowledge):
```
// in some header 
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

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

    return std::move(thd);
}
```

Then you can use your new (and safe!) function throughout your program:
```
#include <pthread.h>
#include <unistd.h>
#include <thread>
#include "some_header_with_your_init_thread_template.hpp"

std::thread g_my_child_1;
std::thread g_my_child_2;

void set_handler(void(*sig_handler)(int)) {
    sigaction new_action, old_action;
    new_action.sa_handler = sig_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction (SIGINT, &new_action, NULL);
    }

    //set other handlers...
}

void my_child_1_sighdl(int sig) {
    std::cout << "child 1 received signal[" << sig << "]" << std::endl;
}

void my_child_2_sighdl(int sig) {
    std::cout << "child 2 received signal[" << sig << "]" << std::endl;
}

void child_func_1(const char* arg0, const char* arg1) {
    // use arg0 and arg1 to rule the world
}

void child_func_2() {
    // rule the world without relying on the crutch of arguments
}

void launch_my_child_threads() {
    // use some lambdas to easily call set_handler() during thread initialization
    g_my_child_1 = init_thread([]{ set_handler(my_child_1_sighdl); }, child_func_1, "everybody wants to rule the", "world");
    g_my_child_2 = init_thread([]{ set_handler(my_child_2_sighdl); }, child_func_2);
}
```

## Lesson Notes
Here are the links to all the lesson notes. We will meet regularly (probably every 2 weeks or so) where I will go through the lesson notes and have extended question/answer session (if necessary). We can live debug together if necessary, though I can't promise I can immediately solve every problem on the spot :D. The goal is you should do your unit tests sometime in the intervening days, though I won't track your progress (and I don't care! I only care about helping you learn this skill).

- [Lesson 1: Basic Templates Part 1](lesson_1_basic_templates_part_1.md) -- [unit tests](tst/lesson_1_ut.cpp)
- [Lesson 2: Basic Templates Part 2](lesson_2_basic_templates_part_2.md) -- [unit tests](tst/lesson_2_ut.cpp)
- [Lesson 3: Substitution Failure Is Not An Error](lesson_3_substitution_failure_is_not_an_error.md) --
  [unit tests](tst/lesson_3_ut.cpp)
- [Lesson 4: Callables](lesson_4_callables.md) -- [unit tests](tst/lesson_4_ut.cpp)
- [Lesson 5: Variadic Templates](lesson_5_variadic_templates.md) -- [unit tests](tst/lesson_5_ut.cpp)

### Callables
#### Callables - function pointers, functors, lambdas, std::function and you
#### Callables - Techniques - SFINAE Callable argument type detection 
#### Callables - Techniques - SFINAE Callable return value type detection 
#### unit tests 
##### unit test - psuedo-std::function functor
##### unit test - lambda argument capturing 
##### Extra Credit - detail::filter(func, ItBegin, ItEnd)
##### Extra Credit - filter(func, container)

### Variadics
#### Variadics - Handling any number of arguments
#### Variadics - Techniques - variadic rvalue iterator trampolines
#### unit tests
##### unit test - detail::advance_group(It0, ..., ItN)
##### unit test - detail::map(func, ItOut, It0Begin, It0End, It1, It2, ..., ItN)
##### unit test - detail::fold(func, out, It0Begin, It0End, It1, It2, ..., ItN)
##### unit test - each
##### Extra Credit - detail::all(func, ItOut, It0Begin, It0End, It1, It2, ..., ItN)
##### Extra Credit - detail::some(func, ItOut, It0Begin, It0End, It1, It2, ..., ItN)

### Putting it all together 
#### Building API - broad functionality versus selecting sane defaults
#### unit tests
##### unit test - map(func, container0, ..., containerN)
##### unit test - fold(func, container0, ..., containerN)
##### Extra Credit - all(func, container0, ..., containerN)
##### Extra Credit - some(func, container0, ..., containerN)
