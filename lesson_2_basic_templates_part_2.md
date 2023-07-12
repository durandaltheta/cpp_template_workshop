# Basic Templates Part 2
## Lvalues and Rvalues
### What are they?
`lvalue`s and `rvalue`s are a concept in c++ called [value categories](https://en.cppreference.com/w/cpp/language/value_category). However, that hyperlinked list is *extremely* confusing and contains many sub-categories which are commonly also called `lvalue`s or `rvalue`s but are in fact some other value type (`prvalue`, `xvalue`, etc.).

Historically, an `lvalue` might be described as the value on the left side of a statement, whereas an rvalue is on the right side. The classic example is an assignment:
```
int i_am_an_lvalue = 3; // 3 is an rvalue
```

However, the exceptions to this case are enormous, because the right hand side of a statement can *very easily* be an lvalue:
```
int i_am_an_lvalue = 3; 
int i_am_also_an_lvalue = i_am_an_lvalue; // both sides are still lvalues!
```

The best way that I've found to generalize the two value categories for general use is:

#### rvalue
Any variable or object  which is going to go out of existence after the current statement. Examples:
```
int a_variable = 3; // 3 is an rvalue

struct my_struct { };

int my_struct_instance = my_struct(); // the "my_struct()" on the right side of the assignment is an rvalue 

int another_variable = std::move(a_variable); // std::move() forces a_variable to appear as an rvalue during the assignment
```

#### lvalue
Every other kind of variable or object:
```
int a_variable; // lvalue

struct my_struct {
    int my_member_variable; // lvalue
};

my_struct my_struct_instance; // lvalue
```

This doesn't capture the *exact* meaning, you may need to refer to the link I posted for greater detail, but in my experience this is a useful mental model, because the utility this concept brings to `c++` programming is tied to the idea of something "about to go out of existence", enabling "rvalue move semantics".

### Why are lvalues and rvalues important?
In `c` there are two ways to store data, by-value (variables) and by-reference (pointers). Variables exist at a memory address, while pointers hold the value of *another* value's memory address.

Assigning values to variables is called a "deep copy", while assigning a variable to a pointer is a "shallow copy". All this means is that, if the variable is large (some kind of struct or array), deep copying to it is a more expensive operation than shallow copying, which only needs to assign a single value (the memory address).
```
char str[10]; // a character buffer to hold our string
strncpy(&str,"hello world",sizeof(str)); // this is a deep copy
char* str_ptr = &str; // this is a shallow copy
printf("%s", str_ptr); // prints "hello world"
```

The introduction of `rvalue`s to `c++` enables a higher order "in-between copy" called "swapping" using "rvalue move semantics". 

An `std::string` contains an allocated buffer of characters. When the `std::string` is destroyed or reassigned it deallocates it's internal buffer of characters (and when reassigning allocates a *new* buffer to hold the new values). This means that a standard copy between named string variables is slower (often *much* slower) than copying using `char *` pointers. 
```
std::string s1 = "hello world"; // this is a deep copy, each char must be copied into s1's character buffer
std::string s2 = s1; // this is also a deep copy
```

However, with "rvalue move semantics", if the compiler detects that the `std::string` on the right hand side is about to go out of existence (it is an `rvalue`) then the compiler knows it can safely *steal* the underlying `char *` buffer from that string and give it to the `std::string` on the left (actually the two `std::string` will exchange buffer pointers (this is called swaping)). This turns a deep copy into a very shallow copy (for at least that buffer, though other values may need to be copied).
```
std::string s1 = std::string("hello world"); // rvalue makes this a shallow copy!
std::string s2 = std::move(s1); // std::move() makes s1 an rvalue also making this a shallow copy!
```

The `c++` method `std::move()` (part of the `<utility>` standard library) turns an `lvalue` statement into an `rvalue` statement. It (alongside `std::forward<T>()`) are fairly standard tools when writing efficient templates.

A `swap` operation is at least 3 steps rather than one step that a true shallow copy provides. Example implementation of a swap:
```
void swap(void* a, void* b) {
    void* temp = a;
    *a = b;
    *b = temp; 
}
```

### Lvalue and Rvalue References
Objects can often be assigned or constructed using rvalues (which is behavior that the compiler may write for you if you don't explicitly write one for your custom objects). 

`rvalue`s are always a special kind of c++ reference when writing code that uses them. In *non-templates* an `rvalue` reference is denoted by a double ampersand `&&`, instead of a single ampersand `&` used in normal (lvalue!) references. In *templates* the double ampersand `&&` has special meaning, which we will address later.

A simple struct with a lvalue and rvalue constructors/assignment:
```
struct my_struct() {
    // rhs is common shortand for "right hand side"

    my_struct(const my_struct& rhs) // lvalue constructor
        : m_str(rhs.m_str)
    { }

    my_struct(my_struct&& rhs) // rvalue constructor
        : m_str(std::move(rhs.m_str))
    { }

    inline my_struct& operator=(const my_struct& rhs) { // lvalue assignment
        m_str = rhs.m_str;
    }

    inline my_struct& operator=(my_struct&& rhs) { // rvalue assignment
        m_str = std::move(rhs.m_str);
    }

    std::string m_str;
};
```

## Forwarding
### Moving vs Forwarding
`rvalues`, unless explicitly `std::move`ed or `std::forward`ed, will be automatically converted to `lvalues` when passed to subsequent functions in order to limit unintended side effects.

Forwarding is the method of taking either an `rvalue` or `lvalue` passed to a method/function and calling
*another* method/function in such a way as to preserve the `rvalue`ness/`lvalue`ness in the subsequent call.

A call to `std::move()` will transform it's argument into an `rvalue` reference. A call to `std::forward<T>()` will instead *persist* the current value category of its argument, rather than allowing it to be blindly converted to an `lvalue` reference (this means that `rvalue`s will stay `rvalue`s and `lvalue`s will stay `lvalue`s).

As stated previously, the double ampersand `&&` has a special meaning in templates. `&&` usage in a template means the template should accept *either* `lvalue`s or `rvalue`s. When combined with `std::forward<T>()` you can allow the template to pass a reference without change to another function. Here's an example of forwarding an argument reference to an `std::string`'s assignment operator (which have different implementations for `rvalue`s and `lvalue`s!):
```
#include <string>

template <typename STRING>
void assign(std::string& lhs, STRING&& rhs) {
    lhs = std::forward<STRING>(rhs); // us
}

void foo() {
    std::string s;
    std::string s2("foo");
    assign(s, s2); // a deep copy is performed because s2 remains an lvalue
    
    // a swap is performed because the rvalue-ness created by std::move of is preserved inside assign()
    assign(s, std::move(s2)); 
}
```

It should be noted that universal references also accept const references:
```
#include <string>

template <typename T, typename T2>
void assign(T&& lhs, T2&& rhs) {
    lhs = std::forward<T2>(rhs);
}

int main() {
    std::string s1 = "foo";
    std::string s2;
    assign(s2, s1); // works!
    assign(s2, (const)s1); // will still function!
    assign((const)s2, s1) // a compiler error, cannot assign to const
    return 0;
}
```

The takeaway here is that to write efficient template code for all usecases the author may need to incorporate universal references and forwarding. However, it is often *much* easier to write and maintain template code with universal references and forwarding which does this than be forced to write `lvalue` AND `rvalue` implementations of methods (where they are required).

### Universal References in Template Objects
An important note here is that the universal reference treatment of the double ampersand `&&` *ONLY* takes place in top level templates. If you are writing a template `struct` or `class`, and you want to write a template method that uses universal references, you must add an additional `template` header to your method:
```
template <typename T>
struct my_struct {
    void my_lvalue_assignment(T& t) {
        m_value = t;
    }

    void my_rvalue_assignment(T&& t) {
        m_value = std::move(t);
    }

    template <typename T2>
    void my_universal_reference_assignment(T2&& t2) {
        m_value = std::forward<T2>(t2);
    }

private:
    T m_value;
};
```

This is behavior may seem strange initially, but it has to do with what stage of compilation the code is being examined by the compiler. Once a template struct has been been constructed by the compiler (like if you wrote the statement `my_struct<int> ms;`), this forces the compiler to actually generate the final code for that struct (making `&&` represent `rvalue` references in calls to `my_rvalue_assignment()` because that's what the double ampersand means in non-template `c++` code!). The exception to this bevavior is in sub-templates (like in `my_universal_reference_assignment()`) which can be left un-generated by the compiler until they are actually called in the code.

## Type Decay 
Type decay is the ability to take a template type and remove all references and const modifiers from it. This is useful when you are writing a template which which needs to accept a universal reference but will need to return the that same type. This can be done with `std::decay_t`, part of the `<type_traits>` standard library:
```
#include <utility>
#include <type_traits>
#include <iostream>

// std::decay_t<T> removes constness and references from type T
template <typename T>
std::decay_t<T> add_three(T&& t) {
    return t+3
}

int main() {
    int a = 3;
    const int b = 2;
    std::cout << add_three(a) << std::endl;
    std::cout << add_three(b) << std::endl; // still works because the final return type is just 'int'
    return 0; 
}
```

Executing this program:
```
$ ./a.out
6
5
$
``` 

## Compiler Behavior with Templates and Inlining
[Inlining in c++](https://en.cppreference.com/w/cpp/language/inline) is the language feature for writing code which will can have it's body be "copy pasted" by the compiler wherever it is called in code instead of actually triggering a new function call to be put on the stack. It is very similar to a [macro in c](https://gcc.gnu.org/onlinedocs/cpp/Macros.html), with the distinction that inlining in `c++` is actually at the discretion of the compiler (the `inline` keyword is just a suggestion). There's also varous edgecases around efficiency and compilation which requires functions to be a true function on the stack rather than an effective text copy/paste, which the compiler will handle internally. 

Inlining is a useful compiler technique because setting up new function calls on the stack during runtime has its own computation cost, in addition to the cost of actually *executing* the function. 

Inline functions and templates are very similar concepts. Their main distinction to the developer is that templates have the ability to be generated for different types (`std::string`, `int`, `my_object`, etc.), while inline functions are just normal functions with the `inline` keyword prepended to their definition:
```
// compiler can choose to copy paste the function body logic into other functions
inline int add(int a, int b) {
    return a + b;
}
```

There is another important trait shared by inline functions and templates, that they don't generate final code until they are called somewhere. As a consequence valid `c++` libraries can be written which are *header only* (no compiled `.cpp` files!). Header only `c++` libraries are very convenient, because they can be "installed" into a system as source code without pre-compiling them, similar to higher level languages like `python`. 

In header somewhere:
```
#ifndef MY_HEADER_ONLY_LIBRARY
#define MY_HEADER_ONLY_LIBRARY

#include <iostream>

inline int real_main() {
    std::cout << "hello from the real slim shady" << std::endl;
    return 0;
}

#endif
```

In a `.cpp` somewhere:
```
#include "my_header_only_library.hpp"

int main() {
    return real_main(); // real_main() is compiled because it is called here
}
```

Executing this program:
```
$ ./a.out
hello from the real slim shady
$
``` 

There are some limitations around writing code like this. For instance, header only libraries tend to produce larger binaries because the compiler copy pastes so much code (which also increases program start time as more data has to be loaded into memory to begin execution, even though execution may be faster once started). The fact that portions of a program are not pre-compiled also means that compilation speed can be slower as more code needs to be compiled every time. In the average case this is not going to be a problem when used in a project (remember, templates are used all the time by the standard library, see `std::string` which is a type alias to an underlying type `std::basic_string<CharT>` or `std::vector<T>`), but it is worth considering. 

As a side note, [c++ lambdas](https://en.cppreference.com/w/cpp/language/lambda) will also be inlined by the compiler where possible, making them very efficient by default.

### Takeway
For almost all developers, the best way to determine your usage of inlining and templates is *not* to consider the performance beforehand. Except cases where efficiency is a real bottleneck (and even there care must be taken to determine what, when, where and why the bottleneck is occurring), the main advantage to these tools is how much they improve the writing and readability of your code!

For instance, being able to write libraries which exist as source code and don't require pre-compilation is very useful for distributing code to projects because it is both quicker and easier to install. Another advantage is you can write normal code alongside your templates allowing your code to exist in the same file, rather than creating labyrinthine source and header dependencies and avoids populating your source with early function declarations just to get things to compile.

As a one wise man once said, "keep it simple stupid!"
