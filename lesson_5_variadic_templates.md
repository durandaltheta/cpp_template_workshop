# Variadic Templates
## What does variadic mean?
In classic `c` a [variadic function](https://en.cppreference.com/w/cpp/utility/variadic) is a function which takes a variable number of arguments. 

Classic `c` variadic functions have several limitations. The first is that they aren't templated, but hard-coded, limiting their flexibility when dealing with arbitrary types. The second is that they are evaluated at runtime rather than compile time, preventing the compiler from inlining operations.

## Parameter packs
`c++11` expanded upon the concept of classic variadic functions by introducing a *very* powerful feature called [parameter packs](https://en.cppreference.com/w/cpp/language/parameter_pack) which enable the writing of templates with variable number of arguments. 

A parameter pack uses 3 periods `...` to distinguish itself from standard template types. A template type can be defined which can represent a variable number of templated types by using the keyword `typename...`:
```
template <typename... As>
```

When using a parameter pack in a function signature the `...` is placed after the type and before the argument name which will represent the parameter pack:
```
template <typename... As>
void foo(As... as) { // parameter pack of type 'As' is named 'as' 
    ...
}
```

References and universal references can be used:
```
template <typename... As>
void foo(As&&... as) { // parameter pack 'as' are references
    ...
}
```

A parameter pack can be used similarly to other arguments, except that `...` should be postpended to the end to inform the compiler to repeat any operation done on all arguments it represents:
```
template <typename... As>
void wrap_printf(As... as) { 
    printf(as...); 
}
```

Parameter packs can be forwarded/moved as normal:
```
template <typename... As>
void foo(As&&... as) { // parameter pack 'as' are references 
    // each reference represented by 'as' is individually forwarded to 'faa()'
    faa(std::forward<As>(as)...); 
}
```

## Processing parameter packs with recursive templates
A common pattern to deal with parameter packs is to write templates so they process one argument at a time in a recursive fashion. As a reminder, recursion is a programming technique where a function calls itself instead of using a loop construct:
```
#include <vector>

// sum_vector recursively calls itself
int sum_vector(int cur, std::vector<int>::iterator it, std::vector<int>::iterator end) {
    if(it != end) {
        return sum_vector(sum + *it, ++it, end);
    } else {
        return cur;
    }
}

int main() {
    std::vector<int> v{1,5,17,3};
    std::cout << sum_vector(0, v.begin(), v.end()) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out 
26
$
```

Variadic template arguments are often processed in a similar fashion, handling one concrete argument at a time:
```
#include <iostream>

// final function when no arguments are left to process
void print() { 
    std::cout << std::endl;
}

// handle one argument at a time
template <typename A, typename... As>
void print(A&& a, As&&... as) {
    std::cout << std::forward<A>(a); // process one argument
    print(std::forward<As>(as)...); // pass the rest to another call to print for processing
}

int main() {
    print("hello", " ", "world");
    return 0;
}
```

Executing this program:
```
$ ./a.out 
hello world
$
```

The compiler actually writes a function definition to handle every usage and generated usage of a template. The naive generated code from the above templates might look something like:
```
#include <iostream>

void print() {
    std::cout << std::endl;
}

void print(const char* a) {
    std::cout << a;
    print();
}

void print(const char* a, const char* b) {
    std::cout << a;
    print(b);
}

void print(const char* a, const char* b, const char* c) {
    std::cout << a;
    print(b, c);
}

int main() {
    print("hello", " ", "world");
    return 0;
}
```

However, in reality the compiler will probably inline much of the above to be something more like:
```
include <iostream>

int main() {
    std::cout << "hello";
    std::cout << " ";
    std::cout << "world";
    std::cout << endl;
    return 0;
}
```

## Callables and parameter packs
Callables can be easily combined with parameter packs:
```
#include <string>
#include <iostream>
#include <functional>

template <typename F, typename... As> // declare a template parameter pack 
auto // allow the compiler to deduce the return type 
execute_callable_with_args(F&& f, As&&... as) {
    return f(std::forward<As>(as)...);
}

int foo() {
    return 3;
}

struct s {
    std::string
}

int main() {
    return 0;
}
```
