# Substitution Failure Is Not An Error - SFINAE 
The phrase "substitution failure is not an error" is a phrase coined by David Vandevoorde. It refers to the behavior of the compiler when processing templates. In essence: if an error occurs when the compiler is checking whether a template can return valid code it does *not* halt compilation and return an error. Instead it first checks the remaining available template candidates for an alternative which *is* valid. Only if no template is found which can produce valid code does the compiler error out.

This behavior is useful because it lets the user write multiple specialized templates to address different types. An example of this was provided in the first lesson:
```
#include <string>
#include <iostream>

template <typename T, typename T2>
T add(T t1, T2 t2) {
    return t1 + t2;
}

template <typename T>
std::string add(std::string s, T t) {
    return s + std::to_string(t);
}

template <typename T>
std::string add(T t, std::string s) {
    return std::to_string(t) + s;
}

int main() {
    std::cout << add(1, 2) << std::endl;
    std::cout << add(std::string("number: ", 3.0) << std::endl;
    std::cout << add(3, std::string("is also a number ") << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
3
number: 3.0
3 is also a number
$
```

As can be seen above, aside from the body of the functions, the templates for `add` are different in one of the
following ways:
- they have different `template` headers AND/OR
- they have different function signatures (the order/types of their arguments are unique)

Writing multiple versions of templates in this way is the simplest way to create special case handling. In certain cases this allows other users to write their *own* template specializations building on the ones written before. An example of this can be seen in global operator overloads:
```
struct MyClass {
    MyClass(int i) : m_i(i) { }

    template <typename T>
    int operator+(T&& rhs) {
        // allow compiler to select the appropriate addition function for adding to an int
        return m_i + rhs; 
    }

private:
    const int m_i;
};

// globally overloading the '+' operator in the case where MyClass is on the right hand side of the operation
template <typename T>
int operator+(T&& lhs, MyClass rhs) {
    // use MyClass's internal standard addition operator
    return rhs + lhs; 
}
```

## SFINAE template selection 
Compiler selection between competing templates in C++ is a bit stranger than you'd expect. When I first started studying this topic I assumed the "first valid definition" the compiler came across to be selected, to match behavior in other parts of the `C` programming language, like linking after compilation (if multiple identical function definitions are provided to the linker, the first valid one the linker comes across will be selected and the others discarded). 

Instead, [as described in this answer](https://stackoverflow.com/questions/15497004/c-template-selection) when the compiler has several valid template candidates available it selects the one that is *most specific*. 

A template which is *more* specific, can be used in *fewer* circumstances. 

This means that templates with more deduced types (like `typename T, typename T2, ..., typename TN`) in it's `template < >` header the *less specialized* (and more generalized) the template is. 

Given the previously defined `add()` template function signatures:
```
template <typename T, typename T2> // informally naming this "template 1", it is the *least* specialized
T add(T t1, T2 t2);

template <typename T>
std::string add(std::string s, T t); // this is "template 2", a little more specialized because it has only one deduced type `T`

template <typename T>
std::string add(T t, std::string s); // "template 3" is similar to template 2 but different arguments are deduced
```

Template 1 is the least specialized, because the compiler has to deduce two types `T` and `T2` rather than a single type or less. Template 2 and 3 are equally specialized from that perspective and will be selected by the compiler whenever it can.

```
add(1,2); // template 1 selected 
add(3,std::string(foo)); // template 3 selected
add(std::string(faa),17); // template 2 selected
```

However, which one would the compiler think is the most specialized in a scenario where both arguents `a1` and `a2` are `std::string`s?:
```
std::cout << add(std::string("hello "), std::string("world!")) << std::endl;
```

I have not found any documentation on this case. However, in my case the compiler will error because the `error: call of overloaded ‘add(std::string, std::string)’ is ambiguous`.

Instead we can remove ambiguity by creating a non-template, overloaded definition of our function, where *no* types are deduced by the compiler:
```
// no template header here, as this is a full function definition
std::string add(std::string s, std::string s2) { // this function is most specialized and will be selected when `add()`ing two `std::string`s
    std::cout << "ADD 4" << std::endl;
    return s + s2;
}
```

With this additional definition, the 4th version of `add()` would be selected when `add()`ing two `std::string` arguments:
```
#include <iostream>
#include "my_add_template_header.hpp"

int main() {
    std::cout << add(std::string("hello "), std::string("world!")) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
ADD 4
$
```

### Full Template Specialization
There is a concept in `c++` called [full template specialization](https://en.cppreference.com/w/cpp/language/template_specialization) that is very similar to writing a fully specified non-template version of a function. In practice, a full specialization *is* a fully specified non-template version of a function (just like `std::string add(std::string, std::string)` in the previous section), but it does it in a different way.

Instead of simply writing a new template definition, template specializations write versions of `template` code with the template `<>` types hard coded:
```
#include <iostream>

template <typename T, typename T2> // this is the primary template the specializations refer to
auto // auto keyword can be used to allow the return type to be deduced
add(T&& t, T2&& t2) {
    std::cout << "add(T1,T2)" << std::endl;
    return t + t2;
}

template <> // This is still a template! Brackets are empty because we are going to hardcode them!
int add<int,int>(int i, int i2) {
    std::cout << "add(int,int)" << std::endl;
    return i + i2;
}

template <> 
double add<double,double>(double i, double i2) {
    std::cout << "add(double,double)" << std::endl;
    return i + i2;
}

int main() {
    std::cout << add(1,2) << std::endl;
    std::cout << add(3.0,4.0) << std::endl;
    std::cout << add(3.0,7) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out 
add(int,int)
3
add(double,double)
7.0
add(T1,T2)
10 
$
```

Explicit/full template specialization is a complicated topic which is not very useful most of the time, so I will leave further exploration of this topic to the reader.

### Template and Overload Selection
It should be noted that when all other template rules are accounted for, the compiler follows [overload resolution rules](https://en.cppreference.com/w/cpp/language/overload_resolution), such as when dealing with versions of functions accepting objects with different positions in an inheritence tree. For instance:
```
#include <string> 

class MyString : std::string {
    // just inherit and use everything from parent std::string as-is
};

MyString add(MyString s1, Mystring s2) { // this is just as specialized as 'std::string add(std::string,std::string)'
    return s1 + s2;
}
```

In the above scenario, because MyString can be cast to a it's parent type `std::string`, the above implementation of `Mystring add(MyString s1, Mystring s2)` is probably unnecessary. However, it will probably be selected by the compiler over `std::string add(std::string,std::string)` should said function be defined.

## SFINAE CHALLENGE SECTION - method detection
A very advanced usage of SFINAE can occur when trying to write template specializations which are selected based on whether a templated type has a necessary method. 

The following information is not required to complete the course, however it is extremely useful as a measuring stick for a developer's ability to understand SFINAE in action. 

The example I am about to show is [potentially no longer required as of c++20 using constraints](https://en.cppreference.com/w/cpp/language/constraints), but again, is useful for understanding SFINAE.

YOU WILL PROBABLY HAVE TO READ THIS SEVERAL TIMES (AND PRACTICE IN BETWEEN) TO FULLY UNDERSTAND. YOU HAVE BEEN WARNED.

Here is the complete code for determining if a given object `T` has the method `size()`:
```
// put this in a sub-namespace because it should not be used directly by usercode
namespace detail { 

template<typename T>
struct has_size {
    template<typename U, typename U::size_type (U::*)() const> struct SFINAE {};
    template<typename U> static char test(SFINAE<U, &U::size>*);
    template<typename U> static int test(...);
    static const bool has = sizeof(test<T>(0)) == sizeof(char);
};

}
```

Almost all `std` containers have the method `size()`, but not all. Example: `std::forward_list<T>`. Since, it might be nice (or necessary) to be able to write an edgecase which can handle containers without the `size()` method we can use the above struct to do detect when the edgecase needs to be selected.

This code abuses compile time constants. Essentially, if a value is `const` then the compiler must be able to know its value *during* compilation. The c++ standards allow values known at compile time to be used inside of template declarations (IE, within the `< >` brackets). In practice, this means we can write limited `if/else` logic to determine *which* template gets used at certain times.

I will go through this struct line by line to explain everything that's going on:

```
template<typename T>
struct has_size {
```

We are defining a placeholder struct named `has_size<T>`. We need this struct 
because we need to use SFINAE to determine if type `T` has a method named 
`size()` which returns an unsigned number.

```
    template<typename U, typename U::size_type (U::*)() const> struct SFINAE {};
```

We are declaring a template struct *inside* of the template struct `has_size` with potentially a different type `U` because we don't want the compiler to try to produce the final code *yet*. Instead, we want to wait to make the final code once the compiler has figured out what type of container `U` we are dealing with.

The code `U::size_type` is specifying that whatever `U` is *must* have some type defined called `size_type`. Every `std` container defines this type. `size_type` is always some number type, typically an unsigned integer.

The code `U::size_type (U::*)() const` is a function pointer. The `U::*` is the template mechanism for specifying "function pointer in the `U` namespace". 

Function pointers typically look like: `return_type (*)(arg_1_type, arg_2_type, etc...)`. In this case, we are specifying that for our struct `SFINAE` to be valid, `U` must have some const method which has no arguments and returns `U::size_type`.

That is, we're really looking for if `U` has a function `size_type size() const`, and we are identifying if it has a member function which matches that pattern.

```
    template<typename U> static char test(SFINAE<U, &U::size>*);
```

This is a forward declaration of a template method belonging to `has_size`. As the name of this method implies, we are using it to test something, in this case we are testing *at compilation time* (not runtime!) if we can create a valid `SFINAE` object based on the characteristics of the object we are checking a `size()` function for. 

This `SFINAE` object must be able to declare a function pointer to a method `U::size` as its second template argument. If this template succeeds, this test will be selected and `has_size` will have a method named `test` which returns a `char`. If this is the case, then we know type `U` has a method `size()`!

```
    template<typename U> static int test(...);
```

This is a forward declaration of a template method as a fallback in case the previous test template cannot produce valid code (in which case `T` does *NOT* have a `size()` method!). 

```
    static const bool has = sizeof(test<T>(0)) == sizeof(char);
```

`has` is created by comparing *theoretical* return value type sizes. Essentially *IF* the method `test<T>` was called (remember, `T` is the type of `has_size`, the object we checking for a `size()` method), then `sizeof()` can determine the byte size of `test<T>`'s return value. 

Our first `test()` template returns a `char`, while the second returns an `int`, which are guaranteed by the c++ standard to be of *different byte sizes*. 

If `test()` returns a `char`, then `T` has the function `size()`. Otherwise if `test()` returns an `int`, then `T` does *NOT* have the function `size()`.

This means we are assigning a value of `true` to `has` if the first `test<T>` is selected (meaning type `T` has a method `size()`!), otherwise we are assigning `false`.

To use the `has_size` `struct`, we will need a couple more functions:
```
namespace detail {

// Get the size of an object with its `size()` method
template <typename C>
inline size_t 
size(C& c, std::true_type) { 
    return c.size(); 
}

// Get the size of an object the *slow* way by iterating through it
template <typename C>
inline size_t 
size(C& c, std::false_type) {
    size_t cnt = 0;
    auto first = c.begin();
    auto last = c.end();

    for(; first != last; ++first) { 
        ++cnt; 
    }

    return cnt;
}

}
```

Above I have defined the two function overloads we can call to  return the size of a container. The first simply calls `C::size()`, while the second iterates through the container `C` and increments a count. They accept special empty objects `std::true_type` and `std::false_type`, two different types that can be returned by some `c++` standard library template utility functions. 

Finally, to make the compiler choose one the two implementations, we wrap the call to `detail::size()` in another function:
```
template <typename C>
size_t // size_t is convertable from all std:: container `size_type`s
size(C&& c) { 
    return detail::size(c, std::integral_constant<bool, detail::algorithm::has_size<C>::has>()); 
}
```

When our `size()` function is called with an argument type `C`, it selects the proper `detail::size()` implementation based on the result of the compile time expression `std::integral_constant<bool, detail::algorithm::has_size<C>::has>()`. 

`std::integral_constant<T,T2>` is a `c++` helper struct which has a constexpr method `std::integral_constant<T,T2>::operator()()` which returns either a `std::true_type` if the types `T` and `T2` are identical or `std::false_type` otherwise.

Since said method `std::integral_constant<T,T2>::operator()()` is a `constexpr`, the compiler *always* knows what this return type will be during compilation, allowing us select one of the two implementations of `detail::size()`.

## Exercises 
### size()
### group()
### rvalue slice()
### const slice()
## Extra Credit  
### mutable slice()
