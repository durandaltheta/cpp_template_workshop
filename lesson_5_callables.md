# Callables 
## What is a Callable?
According to [cppreference](https://en.cppreference.com/w/cpp/named_req/Callable) a Callable is:
> A Callable type is a type for which the INVOKE and INVOKE<R> operations (used by, e.g., `std::function`, `std::bind`, and `std::thread::thread`) are applicable.

What this means is if you can "execute" anything with `()` (internally the compiler transforms this to an `INVOKE` or `INVOKE<R>`) in a similar fashion to calling a normal function, then that object, function, or lambda is a `Callable`.

Examples:
```
#include <functional>

int f() { // function f is a Callable
    return 1;
}

struct s { 
    // struct s is a Callable because it has an operator() overload. This type 
    // of object is also known as a Functor.
    int operator()() { 
        return 2;
    }
};

int main() {
    int (*fptr)() = f; // function pointer fptr is a Callable 
    auto l = []{ return 3; }; // lambda l is a Callable 
    std::function<int()> w(l); // function wrapper w is a Callable
    return 0;
}
```

## Callables and templates
Because of SFINAE it is possible to write generic templates which accept Callables as an argument, allowing a broad range of possible argument types:
```
int f() { // function f is a Callable
    return 1;
}

struct s { 
    int operator()() { 
        return 2;
    }
};

template <typename F>
int execute_callable(F&& f) {
    f();
}

int main() {
    struct s;
    int (*fptr)() = f; 
    auto l = []{ return 3; }; 
    std::function<int()> w(l); 

    std::cout << execute_callable(f) << std::endl;
    std::cout << execute_callable(s) << std::endl;
    std::cout << execute_callable(fptr) << std::endl;
    std::cout << execute_callable(l) << std::endl;
    std::cout << execute_callable(w) << std::endl;

    return 0;
}
```

Executing this program:
```
$ ./a.out
1
2
1
3
3
$
```

A function which accepts another function is known as a [higher order function](https://en.wikipedia.org/wiki/Higher-order_function). Higher order functions are very powerful tools (especially when written as templates!) for encapsulating executable behavior. For instance, a library function can be written that accepts a user Callable as an argument that the library function promises to call when certain conditions are met (a callback). With templated Callables said API can be very flexible instead of rigorously strict.

## Callable arguments
SFINAE also allows the ability to pass arguments to an argument function:
```
int foo(int i) {
    return i + 1;
}

struct s {
    int operator()(int i) {
        return i + 2;
    }
}

template <typename F>
int execute_callable(F&& f, int i) {
    return f(i);
}

int main() {
    auto l = [](int i){ return i + 3; };
    std::cout << execute_callable(foo, 10) << std::endl;
    std::cout << execute_callable(s(), 10) << std::endl;
    std::cout << execute_callable(l, 10) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
11
12
13
$
```

There are no standard defined limits on the number or types of arguments that are passable to Callables. In fact, the standard library provides a very commonly used template which allows for any number of potential arguments, [std::thread](https://en.cppreference.com/w/cpp/thread/thread):

```
#include <thread>

void foo(int i) {
   std::cout << i << std::endl;
}

struct s {
    void operator()(std::string s) {
        std::cout << s << std::endl;
    }
}

int main() {
    auto l = [](std::string s, int i) { std::cout << s + std::to_string(i) << std::endl; };

    std::thread th0(foo, 15);
    th0.join();

    std::thread th1(s(), "hello world");
    th1.join();

    std::thread th2(l, "this is a number: ", 7);
    th2.join();

    return 0;
}
```

Executing this program:
```
$ ./a.out 
15
hello world
this is a number: 7
$
```

## Wrapping Callables as finalized types
When a defined type is needed, user Callables can be wrapped inside the [std::function polymorphic function wrapper](https://en.cppreference.com/w/cpp/utility/functional/function) `std::function` instances using type erasure (see lesson 1).

`std::function`s are defined with the return value and argument types inside of the template `<>` brackets following the pattern `std::function<ReturnType(Arg1Type, Arg2Type, ..., ArgNType)>`:
```
#include <functional>

int foo(int i) {
    return i + 1;
}

struct s {
    int operator()(int i) {
        return i + 2;
    }
}

int main() {
    std::function<int(int)> w;

    w = foo;
    std::cout << w(1) << std::endl;
    w = s();
    std::cout << w(1) << std::endl;
    w = [](int i){ return i + 3; };
    std::cout << w(1) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
2
3
4
$
```

A wrapper like `std::function` is often necessary when writing parts of code which *need* to be precompiled (IE, code which is not header only). Combining the concepts of template Callables with `std::function` allows for all kinds of interesting code.

## Examining lambdas 
Lambdas are a type of callable introduced in `c++11` that I've found both extremely useful and often ill-understood by developers generally. To address this deficit of knowledge I have written this optional [lambda primer](lambda_primer.md) as an educational aid for anyone who wants to know more about them. 

Be warned, I will not hold back from using lambdas in future examples, so refer to this primer as necessary.

## Algorithms and Callables 
It is possible to write data processing algorithms which incorporate user provided Callables to provide a portion of the implementation. An example of this in the standard library is [std::transform](https://en.cppreference.com/w/cpp/algorithm/transform), which accepts a start `cur` iterator, an `end` iterator, an output `out` iterator, and a Callable. `std::transform` will iterate from `cur` to `end`, callng the Callable with the value pointed to by `cur` and storing the result in the `out` iterator. An implemenation of said algorithm might look like:
```
namespace std {

template <typename InputIt, typename OutputIt, typename UnaryOperation>
void transform(InputIt cur, InputIt end, OutputIt out, UnaryOperation f) {
    while(cur != end) {
        *out = f(*cur);
        ++cur;
        ++out;
    }
}

}
```

The above `UnaryOperation` is any Callable which accepts the type stored in the input iterators and whose output can be stored in the output iterator:
```
#include <algorithm>
#include <iostream>
#include <string>

int add_2(int i) {
    return i + 2;
}

struct add_1 {
    unsigned int operator()(int i) {
        return i + 1;
    }
};

int main() {
    const std::vector<int> inp{1,2,3};

    {
        std::vector<int> out(inp.size());
        std::transform(inp.begin(), inp.end(), out.begin(), add_2);

        for(auto& e : out) {
            std::cout << "int: " << e << std::endl;
        }
    }

    {
        std::vector<size_t> out(inp.size());
        std::transform(inp.begin(), inp.end(), out.begin(), add_1());

        for(auto& e : out) {
            std::cout << "unsigned int: " << e << std::endl;
        }
    }

    {
        std::vector<std::string> out(inp.size());

        auto add_1_and_to_string = [](int i) {
            return std::to_string(i + 1);
        };

        std::transform(inp.begin(), inp.end(), out.begin(), add_1_and_to_string);

        for(auto& e : out) {
            std::cout << "string: " << e << std::endl;
        }
    }

    return 0;
}
```

Executing this program:
```
$ ./a.out 
int: 3
int: 4
int: 5
unsigned int: 2
unsigned int: 3
unsigned int: 4
string: 2
string: 3
string: 4
$
``` 
