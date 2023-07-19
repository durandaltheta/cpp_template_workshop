# Callables 
## What is a Callable?
According to [cppreference](https://en.cppreference.com/w/cpp/named_req/Callable) a Callable is:
> A Callable type is a type for which the INVOKE and INVOKE<R> operations (used by, e.g., std::function, std::bind, and std::thread::thread) are applicable.

What this basically means is if you can "execute" anything with `()` in a similar fashion to calling a normal function, then that object, function, or lambda is a `Callable`.

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

A function which accepts another function is known as a [higher order function](https://en.wikipedia.org/wiki/Higher-order_function). Higher order functions are very powerful tools (especially when written as templates!) for encapsulating executable behavior. For instance, a library can be written when accepts a user Callable that it promises to call when certain conditions are met (a callback). With templated Callables said API can be very flexible instead of rigorously strict.

## Callable arguments

## Wrapping Callables as finalized types
When a defined type is needed, user Callables can be wrapped inside the [std::function polymorphic function wrapper](https://en.cppreference.com/w/cpp/utility/functional/function). `std::function` instances use a templating technique called [type erasure](https://en.wikipedia.org/wiki/Type_erasure) to remove the *actual* type from being visible to the holder of an `std::function` so that only the `std::function` itself knows what it truly contains.

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
1
2
3
$
```

A wrapper like `std::function` is often necessary when writing parts of code which *need* to be precompiled (IE, code which is not header only). Combining the concepts of template Callables with `std::function` allows for all kinds of interesting code.
