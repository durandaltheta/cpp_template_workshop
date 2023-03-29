#ifndef CPP_TEMPLATE_WORKSHOP_DETAIL_TEMPLATE
#define CPP_TEMPLATE_WORKSHOP_DETAIL_TEMPLATE 

// cpp stl
#include <type_traits>
#include <functional>

/**
 * This where code goes which should *not* be directly included by a user, but
 * is needed by other code included by a user.
 */

namespace ctw { // cpp template workshop
namespace detail { 

// ----------------------------------------------------------------------------- 
// type traits

// get the unqualified base type of a type
template <typename T>
using unqualified = typename std::decay<T>::type;

template <typename T>
using enable_if_rvalue = typename std::enable_if
                         <
                             !std::is_lvalue_reference<T>::value
                         >::type;

template<typename T>
struct function_traits;

// get access to more type information about a function, IE:
//
// typedef std::function<R(A,B)> fun;
// 
// function_traits<fun>::arg<1>::type
template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    static const size_t nargs = sizeof...(Args);

    typedef std::function<R(Args...)> function_type;
    typedef R result_type;

    template <std::size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
    };
}; 

// handle pre and post c++17 
#if __cplusplus >= 201703L
template <typename F, typename... Ts>
using function_return_type = typename std::invoke_result<unqualified<F>,Ts...>::type;
#else 
template <typename F, typename... Ts>
using function_return_type = typename std::result_of<unqualified<F>(Ts...)>::type;
#endif

template <typename F, std::size_t i>
using function_arg_type = typename function_traits<F>::template arg<i>::type;

// -----------------------------------------------------------------------------
// size  

/* BEWARE: DEEP, EVIL, WICKED TEMPLATE MAGIC AHEAD 
This template structure allows for detecting if an object has a method `size()`. 
Almost all `std` containers have this method, but not all. Example:
`std::forward_list`.

This code requires a DEEP depth of template knowledge to fully grok. As such I 
will be providing the full code to it here. However, it may be extremely 
beneficial to understand this code because it will improve your template skills.
Therefore, I have provided an in-depth explanation of how it works.

The following code uses (abuses) SFINAE (an acronym for Substitution 
Failure Is Not An Error). SFINAE is a C++ concept that even if a template does 
not produce valid C++ code with a given type the compiler will silently 
continue onward. This allows for the template author to write *several* 
competing templates with the same name, and the compiler will select the first 
one that produces valid code with the given type.

The code also abuses compile time constants. Essentially, if a value is `const` 
then the compiler must be able to know its value *during* compilation. The c++ 
standards allow values known at compile time to be used inside of template 
declarations (IE, within the `< >` brackets). In practice, this means we can 
write limited `if/else` logic to determine *which* template gets used at certain 
times.

I will go through this struct line by line to explain everything that's going on:

```
template<typename T>
struct has_size {
```

We are defining a placeholder struct named `has_size<T>`. We need this struct 
because we need to use SFINAE to determine if type `T` has a method named 
`size()` which returns an unsigned number.

```
    template<typename U, U::size_type (U::*)() const> struct SFINAE {};
```

We are declaring a template struct *inside* of the template struct `has_size` 
with potentially a different type `U`. In a normal context this is necessary 
when your template object has need of further template methods or sub-objects. 
In our case, it is because we don't want the compiler to try to produce 
valid code *yet*, so we create a template that will only be created *IF* the 
type `U` can produce valid code.

The code `U::size_type` is specifying that whatever `U` is *must* have some type 
defined called `size_type`. Every `std` container defines this type. `size_type`
is always some number type, typically an unsigned integer.

The code `U::size_type (U::*)() const` is a function pointer. The `U::*` is the 
template mechanism for specifying "function pointer in the `U` namespace". 
Function pointers typically look like: `return_type (*)(arg_1_type, arg_2_type, 
etc...)`.In this case, we are specifying that for our struct `SFINAE` to be 
valid, `U` must have some const method which has no arguments and returns 
`U::size_type`.

```
    template<typename U> static char test(SFINAE<U, &U::size>*);
```

This is a forward declaration of a template method belonging to `has_size`. As 
the name of this method implies, we are using it to test something, in this case 
we are testing *at compilation time* (not runtime!) if we can create a valid 
`SFINAE` object. 

This `SFINAE` object must be able to declare a function pointer to a method 
`U::size` as its second template argument. If this template succeeds, this test 
will be selected and `has_size` will have a method named `test` which returns a 
`char`. If this is the case, then we know type `U` has a method `size()`!

```
    template<typename U> static int test(...);
```

This is a forward declration of a template method as a fallback in case the 
previous test template cannot produce valid code (in which case `T` does *NOT* 
have a `size()` method!!). 

```
    static const bool has = sizeof(test<T>(0)) == sizeof(char);
```

`has` is created by comparing *theoretical* return value type sizes. Essentially 
*IF* the method `test<T>` was called (remember, `T` is the type of `has_size`), 
then `sizeof()` can determine the byte size of `test<T>`'s return value. Our 
first `test` template returns a `char`, while the second returns an `int`, which 
are guaranteed by the c++ standard to be of *different byte sizes*. 

This means we are assigning a value of `true` to `has` if the first `test<T>` is 
selected (meaning type `T` has a method `size()`!), otherwise we are assigning 
`false`.

Here is the final magic. All the above code has been careful to be valid at 
*compile time*. Because of this, we can use the value of `has_size<T>::has` 
inside *other* templates to determine whether an object has a `size()` method or 
not!
*/
template<typename T>
struct has_size {
    template<typename U, U::size_type (U::*)() const> struct SFINAE {};
    template<typename U> static char test(SFINAE<U, &U::size>*);
    template<typename U> static int test(...);
    static const bool has = sizeof(test<T>(0)) == sizeof(char);
};

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

// ----------------------------------------------------------------------------- 
// advance
void advance_group() { }

template <typename IT, typename... ITs>
void advance_group(IT& it, ITs&... its) {
    ++it;
    advance_group(std::forward<ITs>(its)...);
}

// ----------------------------------------------------------------------------- 
// map
template <typename RIT, typename F, typename IT, typename... ITs>
void map(RIT&& rit, F&& f, IT&& it, ITs&&... its) {
    *rit = f(*it, *its...);
    advance_group(it, its...);
}

}
}

#endif
