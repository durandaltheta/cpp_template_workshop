# Basic Templates Part 2
## Template References and Pointers
Templates can be written to accept references `&` and pointers `*` in the same way as normal functions:
```
#include <string>
#include <iostream>

template <typename T>
void modify_by_reference(T& t, T new_value) {
    t = new_value;
}

template <typename T>
void modify_by_pointer(T* t, T new_value) {
    *t = new_value;
}

print_vals(int& i, std::string& s) {
    std::cout << "i: " << i << ", s: " << s << std::endl;
}

int main() {
    int i = 3;
    std::string s("foo");

    print_vals(i, s);
    modify_by_reference(i, 4);
    modify_by_reference(s, "faa");
    print_vals(i, s);
    modify_by_pointere(&i, 5);
    modify_by_pointer(&s, "goodbye!");
    print_vals(i, s);
    return 0;
}
```

Executing this program:
```
$ ./a.out
i: 3, s: foo
i: 4, s: faa
i: 5, s: goodbye! 
$
``` 

## Lvalues and Rvalues
In `c++` there is actually more than one kind of reference, there are `lvalue` and `rvalue` references.

`lvalue`s and `rvalue`s are a concept in `c++` called [value categories](https://en.cppreference.com/w/cpp/language/value_category). However, that hyperlinked list is *extremely* confusing and contains many sub-categories which are commonly also called `lvalue`s or `rvalue`s but are in fact some other value type (`prvalue`, `xvalue`, etc.).

Historically, an `lvalue` might be described as the value on the left side of a statement, whereas an rvalue is on the right side. The classic example is an assignment:
```
int i_am_an_lvalue = 3; // 3 is an rvalue
```

However, the exceptions to this case are enormous, because the right hand side of a statement can *very easily* be an lvalue:
```
int i_am_an_lvalue = 3; 
int i_am_also_an_lvalue = i_am_an_lvalue; // both sides are still lvalues!
```

Here is a quote about the difference between lvalues and rvalues from computer scientist Scott Meyers (taken from https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers)
> A precise definition for these terms is difficult to develop (the C++11 standard generally specifies whether an expression is an lvalue or an rvalue on a case-by-case basis), but in practice, the following suffices:
> 
> If you can take the address of an expression, the expression is an lvalue.
> If the type of an expression is an lvalue reference (e.g., T& or const T&, etc.), that expression is an lvalue.
> Otherwise, the expression is an rvalue.  Conceptually (and typically also in fact), rvalues correspond to temporary objects, such as those returned from functions or created through implicit type conversions. Most literal values (e.g., 10 and 5.3) are also rvalues.

To further simplify what Scott described you can generalize the two value categories as such:

### rvalue
Any variable or object  which is going to go out of existence after the current statement. Examples:
```
int a_variable = 3; // 3 is an rvalue

struct my_struct { };

// the "my_struct()" on the right side of the assignment is an rvalue 
auto my_struct_instance = my_struct(); 

// std::move() forces a_variable to appear as an rvalue during the assignment
int another_variable = std::move(a_variable);

int add_1(int&& i) { // double ampersand reference to i is an rvalue
    return i + 1;
}
```

### lvalue
Every other kind of variable or object:
```
int a_variable; // lvalue

struct my_struct {
    int my_member_variable; // lvalue
};

my_struct my_struct_instance; // lvalue 

int add_3(int& i) { // reference to i is an lvalue
    return i + 3;
}

int sub_2(const int& i) { // const reference to i is an lvalue
    return i - 2;
}
```

This doesn't capture the *exact* meaning, you may need to refer to the [value categories](https://en.cppreference.com/w/cpp/language/value_category) page for greater detail, but in my experience this is a useful mental model, because the utility this concept brings to `c++` programming is tied to the idea of something "about to go out of existence" which in turn enables something called "rvalue move semantics".

## Why are lvalues and rvalues important?
In `c` there are two ways to store data, by-value (variables) and by-reference (pointers). Variables exist at a memory address, while pointers hold the value of *another* value's memory address.

Assigning values to variables is called a "deep copy", while assigning a variable to a pointer is a "shallow copy". All this means is that, if the variable is large (some kind of struct or array), deep copying to it is a more expensive operation than shallow copying, which only needs to assign a single value (the memory address).
```
char str[10]; // a character buffer to hold our string
memset(str, 0, sizeof(str));
strncpy(str,"hello world",sizeof(str)); // this is a deep copy
char* str_ptr = &str; // this is a shallow copy
printf("%s", str_ptr); // prints "hello world"
```

The introduction of `rvalue`s to `c++` enables a higher order "in-between copy" called "swapping" using "rvalue move semantics". 

An `std::string` contains an allocated buffer of characters. When the `std::string` is destroyed or reassigned it deallocates it's internal buffer of characters (and when reassigning allocates a *new* buffer to hold the new values). This means that a standard copy between named `std::string` variables is slower (often *much* slower) than copying only `char *` pointers. 
```
std::string s1 = "hello world"; // this is a deep copy, each char must be copied into s1's character buffer
std::string s2 = s1; // this is also a deep copy
```

However, with "rvalue move semantics", if the compiler detects that the `std::string` on the right hand side is about to go out of existence (it is an `rvalue`) then the compiler knows it can safely *steal* the underlying `char *` buffer from that string and give it to the `std::string` on the left (actually the two `std::string` will exchange buffer pointers). This operation is called "swapping". Swapping turns a deep copy into a shallow copy (for at least the string character buffer, other `std::string` members may need to be copied).
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

## Lvalue and Rvalue References
Most objects can be assigned or constructed using rvalues (operations which the compiler may write for you if you don't explicitly write them for your objects). But how can we write code which recognizes an argument as an `rvalue`?

In *non-templates* the solution is to denote an `rvalue` reference by a double ampersand `&&`, instead of a single ampersand `&` used in normal (lvalue!) references. In *templates* the double ampersand `&&` has special meaning, which we will address later.

A simple struct with a lvalue and rvalue constructors/assignment explicitly written:
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

As stated previously, the double ampersand `&&` has a special meaning in templates. `&&` usage in a template means the template should accept *either* `lvalue`s or `rvalue`s. This version of `&&` is colloqually known as a [universal reference](https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers). A universal reference is a way of telling the compiler that your template accepts *some* kind of reference as an argument, but it needs to deduce what the reference type actually is when the template is invoked.

When universal references are combined with `std::forward<T>()` you can write a template which can pass a reference without change to another function. Here's an example of forwarding an argument reference to an `std::string`'s assignment operator (which have different implementations for `rvalue`s and `lvalue`s!):
```
#include <string>

template <typename STRING>
void assign(std::string& lhs, STRING&& rhs) {
    // passes rhs to the assignment function as either an rvalue or lvalue depending on context
    lhs = std::forward<STRING>(rhs); 
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
    assign(s2, (const)s1); // also works!
    assign((const)s2, s1); // a compiler error, cannot assign a value to a const variable
    return 0;
}
```

The takeaway here is that to write efficient template code for all usecases the author may need to incorporate universal references and forwarding. However, it is often *much* easier to write and maintain template code utilizing universal references and forwarding rather than be forced to write `lvalue` AND `rvalue` implementations of methods (where they are required).

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
Type decay is the ability to take a template type and remove all references and const modifiers from it. This is useful when you are writing a template which which needs to accept a universal reference but will need to return the base form of that type. This can be done with `std::decay_t`, part of the `<type_traits>` standard library:
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
    std::cout << add_three(a) << std::endl; // returns `int`
    std::cout << add_three(b) << std::endl; // still returns `int`, not `const int`
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
[Inlining in c++](https://en.cppreference.com/w/cpp/language/inline) is the language feature for writing code which can be "copy pasted" by the compiler wherever it is called in code instead of actually triggering a new function call on the stack. Inlining is a useful compiler technique because setting up new function calls on the stack during runtime has its own computation cost, in addition to the cost of actually *executing* the function. 

Inlining is very similar to [macros in c](https://gcc.gnu.org/onlinedocs/cpp/Macros.html), with the distinction that inlining in `c++` is actually at the discretion of the compiler (the `inline` keyword is just a suggestion). Additionally, `inline` code is namespace aware, unlike macros which blindly paste text. There's also varous edgecases around efficiency and compilation which requires `inline` functions to be a true function on the stack rather than an effective text copy/paste, which the compiler will handle internally. 

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

It should be noted that if templates are required as part of a library's API, they are typically *required* to be implemented in a header, at least for any code where a type needs to be deduced by the user code's compiler.

### Takeway
For almost all developers, the best way to determine your usage of inlining and templates is *not* to consider the performance beforehand. Except cases where efficiency is a real bottleneck (and even there care must be taken to determine what, when, where and why the bottleneck is occurring), the main advantage to these tools is how much they improve the writability and readability of your code!

Being able to write libraries which don't require pre-compilation is very useful for distributing code to projects because it is both quicker and easier to install. Another advantage is you can write normal code alongside your templates allowing *all relevant* code to exist in the same file, rather than creating labyrinthine source and header dependencies and avoid populating your source with early function declarations just to get things to compile. The fact these libraries may add some startup overhead (reading slightly larger executable binary files from the disk) is unlikely to be the bottleneck to meet performance standards *in most cases*.

As a one wise man once said, "keep it simple stupid!" 
