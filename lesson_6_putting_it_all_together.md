# Putting it all together 
## What now?
The techniques covered in the previous lessons are all I'm going to explicitly teach in this course, they are already a pretty good foundation. There are many minor sub-features of templates, many helper structs and functions in the standard library, and many composite techniques which build on the essentials. All of these are nice but they don't matter until you explicitly need them, and they can be discovered as necessary when you need them.

What's more important now is to consider the question: *how* should I write *my* templates?

## Keep it simple stupid
In my opinion, good templates are not about what's possible, they are about what's helpful. Therefore, you should [follow the principle of least surprise](https://en.wikipedia.org/wiki/Principle_of_least_astonishment).

A programmer using your templates has very many things they need to do in their day, learning the ins and outs of your particular flavor of template library code doesn't need to be one of them. In almost every case, a developer's most pressing concern is getting their code working correctly, not forming the perfect algorithm. 

API is the most important aspect of template code, because it is the part that has to interface with the knowledge of the user. If the API is kept simple and understandable, users will be both more willing to use your code and more likely to use it correctly.

When attempting to address a problem with a template consider these factors:
- Could my API be made more similar to other API that the user likely understands which solves similar problems?
- Do the names of objects and functions clearly describe what they do?
- Are the names of objects and functions no longer than necessary?
- Can my overall design be simplified without omitting necessary functionality? 

Think back to the `init_thread` example in the README.md:
```
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

With regards to the above questions:
- The API is intentionally similar to `std::thread::thread()` to make it easier to grasp
- The name `init_thread` gives a rough idea of what the function is capable of
- The name `init_thread` and types `InitFunction`, `Function`, `OptionalArgs` succinctly describe their purpose
- The overall design only adds one new argument above `std::thread::thread()`, it would be difficult to simplify it further

## The clockwork internals
With all that said about simplicity of API, my experience has simultaneously shown that the *internals* of template code can as complicated as necessary in order to better solve problems. As long as the API is thoroughly tested, the user doesn't have to learn *how* it works, only be confident it *will* work. 

This doesn't mean that complexity should be blindly accepted within template internals, as over-complexity may be a sign of bad design. Only, the template author shouldn't feel inhibited by adding complexity where it is warranted.

## Expanding on the foundations of the language
Templates allow the creation of new `c++` language features that may be entirely outside of the design scope of the base language and standard library. While not always necessary or desirable, in some cases it may be worth considering using new ideas.

For example, in the Rust programming language, they have a different philosophy of how mutexes work. The [rust std::sync::Mutex object](https://doc.rust-lang.org/std/sync/struct.Mutex.html) is both a value and a lower level mutex. In order to read or write to the stored value, the user must first lock the `std::sync::Mutex` object which returns another object that contains the (Rust equivalent to a) reference to the value. This design eleminates an entire class of issues arising from accessing shared values because it forces users to correctly lock their data while also helping to scope the lifetime of that lock.

With templates, implementing a similar lock in `c++` is very possible. While considering some of the naming in the standard library for similar objects like `std::unique_lock` and `std::lock_guard`, our lock might look like:
```
#include <mutex>

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
    static value_guard(As&&... as) : 
        m_t(std::forward<As>(as)...)
    { }

    // Acquire a locked unique_lock containing a reference to stored value T.
    inline value_guard::unique_lock acquire() {
        return unique_lock{std::unique_lock<MUTEX>(m_mtx), *m_t};
    }

private:
    T m_t;
    MUTEX m_mtx;
};
```

This design combines the necessity of atomically protecting shared data, with the high level features of `c++` synchronization mechanisms. For instance, because `value_guard::unique_lock` inherits from `std::unique_lock`, you should be able to use it with `std::condition_variable`, even if it requires casting.

Usage of the above might look something like this in a trivial case:
```
// ...
#include <iostream>
#include "value_guard_header.hpp"
// ...
value_guard<int> g_shared_value;
// ...

void user_thread_func() {
    // ... 

    {
        auto lk = g_shared_value.lock();

        // can interact safely with value
        std::cout << lk.value << std::endl;
    }

    // lk goes out of scope releasing the underlying std::mutex

    // ... 
}

int main() {
    // ...

    std::thread user_thread(user_thread_func);

    // ...
}
```

All of this illustrates the point that templates expand what is possible in `c++`, potentially improving your code. Therefore, if a feature in a different language seems useful in your `c++` library or project, it may be worth considering how efficiently the concept could be ported.

## Further examples of well designed API
I'm a fan of the [scheme](https://www.scheme.com/tspl4/) functional programming language. `scheme` is a `LISP` variant with a focus on efficient, minimal API. It is also one of those languages (alongside `LISP` variants more generally) which incorporate very flexible data processing algorithms. In fact many of the powerful features that pop up in other languages find their roots or were pioneered in practice in the `LISP` family of programming languages. These include:
- generics, templates, and macros
- closures (lambda captures)
- coroutines (built from continuations)

In fact, these algorithms seem to find their way in some form into most high level programming languages. It should also be noted that `scheme` primarily operates on linked lists instead of arrays or vectors, and their algorithms are normally written against said lists. A short list of some of the most important algorithms: 
- [map](https://www.scheme.com/tspl4/control.html#./control:s30): apply a function to elements of one or more lists, returning the results in a list 
- [for-each](https://www.scheme.com/tspl4/control.html#./control:s33): similar to `map` but does not return a list of values.
- [fold-left](https://www.scheme.com/tspl4/control.html#./control:s38): perform a calculation on one or more lists by evaluating it with a function with an init value, evaluating from left to right 
- [exists](https://www.scheme.com/tspl4/control.html#./control:s36): apply a function to elements of one or more lists until function returns `true`, causing `exists` to return `true`. If function never returns `true`, `exists` returns `false`
- [for-all](https://www.scheme.com/tspl4/control.html#./control:s37): apply a function to elements of one or more lists, until function returns `false`, causing `for-all` to return `false`. If function never returns `false`, `for-all` returns `true`.

The above functions are extremely flexible, as well as frequently [composable](https://en.wikipedia.org/wiki/Composability). It's actually astounding the number of other algorithms that can be generated using only those above. For this very reason, I have provided implementations of each of these in [scalgorithm header](inc/scalgorithm.hpp) for both the education and utility of the reader.
