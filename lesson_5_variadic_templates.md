# Variadic Templates
## What does variadic mean?
In classic `c` a [variadic function](https://en.cppreference.com/w/cpp/utility/variadic) is a function which takes a variable number of arguments. 

Classic `c` variadic functions have several limitations. The first is that they aren't templated, but hard-coded, limiting their flexibility when dealing with arbitrary types. The second is that variadic functions operate on raw pointers, which is less ideal than references. The third is that they are evaluated at runtime rather than compile time, preventing the compiler from inlining operations.

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
void wrap_printf(As... as) { // 'as...' resolves to 'const char*, const char*'
    printf(as...); 
}

int main() {
    wrap_printf("hello %s\n", "world");
    return 0;
}
```

Executing this program:
```
$ ./a.out 
hello world
$
```

Parameter packs can be forwarded/moved as normal:
```
template <typename... As>
void foo(As&&... as) { // parameter pack 'as' are references 
    // each reference represented by 'as' is individually forwarded to 'faa()'
    faa(std::forward<As>(as)...); 
}
```

If a template uses a normal `template` `typename` as well as a parameter pack `typename...` then the parameter pack should typically come last:
```
template <typneame A, typename... As>
void faa(A&& a, As&&... as) {
    // can use a and as... as normal
}
```

## Processing parameter pack elements with recursive templates
In the ideal case the developer will not have to process the individual elements of a parameter pack, because the `as...` variable will resolve to the actual arguments that the compiler can understand.

However, sometimes you run into situations where the template itself needs to process through the arguments in a parameter pack. A common pattern to deal with parameter pack iteration is to write templates so they process one argument at a time in a recursive fashion. As a reminder, recursion is a programming technique where a function calls itself instead of using a loop construct:
```
#include <vector>
#include <iostream>

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

Variadic template arguments are often processed in a similar fashion, handling one concrete argument at a time. They break out of recursion by calling an explicitly defined template or function with the same function name that does not recursively call itself:
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
    print(std::forward<As>(as)...); // pass the rest to another print call
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

The compiler actually writes a function definition to handle every generated usage of a template. The naive generated code from the above templates might look something like:
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
Callables can be easily combined with parameter packs to pass arbitrary number of arguments to them:
```
#include <string>
#include <iostream>

template <typename F, typename... As> // declare a template parameter pack 
auto // allow the compiler to deduce the return type 
execute_callable_with_args(F&& f, As&&... as) {
    return f(std::forward<As>(as)...);
}

int foo() {
    return 3;
}

int faa(int i, int i2, int i3) {
    return i + i2 + i3;
}

struct s {
    std::string operator()(std::string s, std::string s2) {
        return s + s2;
    }
}

int main() {
    std::cout << execute_callable_with_args(foo) << std::endl;
    std::cout << execute_callable_with_args(faa, 1, 2, 3) << std::endl;
    std::cout << execute_callable_with_args(s(), "hello", " world") << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out 
3
6
hello world
$
```

The above pattern allows for very flexible algorithm construction.
