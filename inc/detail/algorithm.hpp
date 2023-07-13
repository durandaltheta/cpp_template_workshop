#ifndef SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_ALGORITHM
#define SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_ALGORITHM

// cpp stl
#include <type_traits>
#include <functional>
#include <tuple>
#include <any>

// local 
#include "template.hpp"

/**
 * This where code goes which should *not* be directly included by a user, but
 * is needed by other code included by a user.
 */

namespace sca { // simple cpp algorithm
namespace detail { 
namespace algorithm {

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

YOU WILL PROBABLY HAVE TO READ THIS SEVERAL TIMES (AND PRACTICE IN BETWEEN) 
TO FULLY UNDERSTAND. 

It took me quite awhile :).

The following code uses (abuses) SFINAE (an acronym for Substitution 
Failure Is Not An Error). SFINAE is a C++ concept that even if a template does 
not produce valid C++ code with a given type the compiler will silently 
continue onward without erroring. This allows for the template author to write 
 *several* competing templates with the same name, and the compiler will select 
 the first one that produces valid code with the given type.

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
    template<typename U, typename U::size_type (U::*)() const> struct SFINAE {};
```

We are declaring a template struct *inside* of the template struct `has_size` 
with potentially a different type `U`. 

In normal uses this is necessary when your template object has need of internal 
template methods or sub-objects.

In our case, we are creating this templated sub-struct because we don't want the 
compiler to try to produce the final code *yet*. Instead, we want to wait to 
make the final code once the compiler has figured out what type of container `U` 
we are dealing with.

The code `U::size_type` is specifying that whatever `U` is *must* have some type 
defined called `size_type`. Every `std` container defines this type. `size_type`
is always some number type, typically an unsigned integer.

The code `U::size_type (U::*)() const` is a function pointer. The `U::*` is the 
template mechanism for specifying "function pointer in the `U` namespace". 

Function pointers typically look like: `return_type (*)(arg_1_type, arg_2_type, 
etc...)`. In this case, we are specifying that for our struct `SFINAE` to be 
valid, `U` must have some const method which has no arguments and returns 
`U::size_type`.

That is, we're really looking for if `U` has a function 
`size_type size() const`, and we are identifying if it has a member function 
which matches that pattern.

```
    template<typename U> static char test(SFINAE<U, &U::size>*);
```

This is a forward declaration of a template method belonging to `has_size`. As 
the name of this method implies, we are using it to test something, in this case 
we are testing *at compilation time* (not runtime!) if we can create a valid 
`SFINAE` object based on the characteristics of the object we are checking a 
`size()` function for. 

This `SFINAE` object must be able to declare a function pointer to a method 
`U::size` as its second template argument. If this template succeeds, this test 
will be selected and `has_size` will have a method named `test` which returns a 
`char`. If this is the case, then we know type `U` has a method `size()`!

```
    template<typename U> static int test(...);
```

This is a forward declaration of a template method as a fallback in case the 
previous test template cannot produce valid code (in which case `T` does *NOT* 
have a `size()` method!!). 

```
    static const bool has = sizeof(test<T>(0)) == sizeof(char);
```

`has` is created by comparing *theoretical* return value type sizes. Essentially 
*IF* the method `test<T>` was called (remember, `T` is the type of `has_size`, 
the object we checking for a `size()` method), then `sizeof()` can determine the 
byte size of `test<T>`'s return value. 

Our first `test()` template returns a `char`, while the second returns an `int`, 
which are guaranteed by the c++ standard to be of *different byte sizes*. 

If `test()` returns a `char`, then `T` has the function `size()`.

Otherwise if `test()` returns an `int`, then `T` does *NOT* have the function 
`size()`.

This means we are assigning a value of `true` to `has` if the first `test<T>` is 
selected (meaning type `T` has a method `size()`!), otherwise we are assigning 
`false`.

Here is the final magic. All the above code has been careful to be valid at 
*compile time*. Because of this, we can use the value of `has_size<T>::has` 
inside *other* templates to determine whether an object has a `size()` method or 
not!

See implementation of `sca::size()` one level up in `inc/algorithm.hpp` for the 
code which calls `has_size` and selects which of the final 
`std::true_type`/`std::false_type` `ct::detail::size()` implementations to call.
*/
template<typename T>
struct has_size {
    template<typename U, typename U::size_type (U::*)() const> struct SFINAE {};
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
// copy_or_move  

// Copy or move only one value. Keeping this as a separate template removes 
// having to rewrite templates which need to forward value categories based on 
// different types than their own type. IE, use the value category of a 
// `container<T>` to determine the copy/move operation when assigning values 
// between `container::<T>::iterator`s.
template <typename DEST, typename SRC>
void copy_or_move(std::true_type, DEST& dst, SRC& src) {
    dst = src;
}

template <typename DEST, typename SRC>
void copy_or_move(std::false_type, DEST& dst, SRC& src) {
    dst = std::move(src);
}


// ----------------------------------------------------------------------------- 
// range_copy_or_move 

// Don't use `std::copy()` or `std::move()` because we want to ensure that 
// side effects of incrementing iterators are preserved.
//
// Think of this as a container and value category aware memcpy() :).
template <typename DIT, typename IT>
void range_copy_or_move(std::true_type, DIT& dst_cur, IT& src_cur, IT& src_end) {
    for(; src_cur != src_end; ++src_cur, ++dst_cur) {
        *dst_cur = *src_cur;
    }
}

template <typename DIT, typename IT>
void range_copy_or_move(std::false_type, DIT& dst_cur, IT& src_cur, IT& src_end) {
    for(; src_cur != src_end; ++src_cur, ++dst_cur) {
        *dst_cur = std::move(*src_cur);
    }
}


// ----------------------------------------------------------------------------- 
// sum 

template <typename V>
size_t sum(V sum, V v) {
    return sum + v;
}

template <typename V, typename... Values>
size_t sum(V sum, V v, V v2, Values... vs) {
    return sum(sum + v, v2, vs...);
}


// ----------------------------------------------------------------------------- 
// group operations 

template <typename IT>
void group(IT&& cur) {
}

template <typename IT, typename C, typename... Cs>
void group(IT&& cur, C&& c, Cs&&... cs) {
    detail::algorithm::range_copy_or_move(std::is_lvalue_reference<C>(), cur, c.begin(), c.end());
    group(cur, std::forward<Cs>(cs)...);
}


// ----------------------------------------------------------------------------- 
// advance group 

/*
 * The purpose of this algorithm is to increment any number of iterators by reference
 */
template <typename IT>
void advance_group(IT& it) { 
    ++it;
}

template <typename IT, typename IT2, typename... ITs>
void advance_group(IT& it, IT2& it2, ITs&... its) {
    ++it;
    advance_group(it2, its...);
}


// ----------------------------------------------------------------------------- 
// map
template <typename F, typename RIT, typename IT, typename... ITs>
void map(F&& f, size_t len, RIT&& rit, IT&& it, ITs&&... its) {
    while(len) {
        --len;
        *rit = f(*it, *its...);
        advance_group(rit, it, its...);
    }
}


// ----------------------------------------------------------------------------
// fold
template <typename F, 
          typename R,
          typename... ITs>
R
fold(size_t len, F& f, R&& init, ITs&&... its) {
    using M = std::decay_t<R>;
    M mutable_state(std::forward<R>(init));

    for(size_t i=0; i<len; ++i) {
        mutable_state = f(std::move(mutable_state), *its...);
        advance_group(++its...);
    }

    return mutable_state;
}


// ----------------------------------------------------------------------------- 
// each
template <typename F, typename IT, typename... ITs>
void each(F&& f, size_t len, IT&& it, ITs&&... its) {
    for(; len; --len) {
        f(*it, *its...);
        advance_group(it, its...);
    }
}


// ----------------------------------------------------------------------------
// all
template <typename F, typename... CITs>
inline bool
all(size_t len, F&& f, CITs&&... cits) {
    bool ret = true;

    for(size_t i=0; i<len; ++i) {
        if(!f(*cits...)) {
            ret = false;
            break;
        }

        advance_group(++cits...);
    }

    return ret;
}


// ----------------------------------------------------------------------------
// some
template <typename F, typename... CITs>
inline bool
some(size_t len, F&& f, CITs&&... cits) {
    bool ret = false;

    for(size_t i=0; i<len; ++i) {
        if(f(*cits...)) {
            ret = true;
            break;
        }

        advance_group(++cits...);
    }

    return ret;
}

}
}
}

#endif
