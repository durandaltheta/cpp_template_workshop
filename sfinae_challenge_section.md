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

This code abuses compile time constants. Essentially, if a variable is `const` or a function is a `constexpr` then the compiler must be able to know its value *during* compilation. The c++ standard allows values known at compile time to be used inside of template declarations (IE, within the `< >` brackets) and other places. In practice, this means we can write limited `if/else` logic to determine *which* template gets used at certain times.

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

The code `U::size_type (U::*)() const` is a function pointer. The `U::*` is the template mechanism for specifying "function pointer in the `U` namespace". Function pointers typically look like: `return_type (*)(arg_1_type, arg_2_type, etc...)`. See [cppreference's pointer section, subsection "Pointers to functions"](https://en.cppreference.com/w/cpp/language/pointer) for a basic primer on function pointers.

In this case, we are specifying that for our struct `SFINAE` to be valid, `U` must have some const method (a class function which will not modify member variables) which has no arguments and returns a `U::size_type`. That is, we're really looking for if `U` has a function `size_type size() const`, and we are identifying if it has at least one member function which matches that pattern.

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
size_t size(C& c, std::true_type) { 
    return c.size(); 
}

// Get the size of an object the *slow* way by iterating through it
template <typename C>
size_t size(C& c, std::false_type) {
    return std::distance(c.begin(), c.end());
}

}
```

Above I have defined the two function overloads we can call to return the size of a container. The first simply calls `C::size()`, while the second iterates through the container `C` and increments a count. They accept special empty objects `std::true_type` and `std::false_type`, two different types that can be returned by some `c++` standard library template utility functions. 

Finally, to make the compiler choose one the two implementations, we wrap the call to `detail::size()` in another function:
```
template <typename C>
size_t // size_t is convertable from all std:: container `size_type`s
size(C&& c) { 
    return detail::size(c, std::integral_constant<bool, detail::has_size<C>::has>()); 
}
```

When our `size()` function is called with an argument type `C`, it selects the proper `detail::size()` implementation based on the result of the compile time expression `std::integral_constant<bool, detail::has_size<C>::has>()`. 

`std::integral_constant<T,T2>` is a `c++` helper struct which has a `constexpr` method `std::integral_constant<T,T2>::operator()()` which returns either a `std::true_type` if the types `T` and `T2` are identical or `std::false_type` otherwise.

Since said method `std::integral_constant<T,T2>::operator()()` is a `constexpr`, the compiler *always* knows what this return type will be during compilation, allowing us select one of the two implementations of `detail::size()`:

If `std::true_type` (object has a `size()` method):
```
template <typename C>
size_t size(C& c, std::true_type);
```

Otherwise `std::false_type` (object has no `size()` method):
```
template <typename C>
size_t size(C& c, std::false_type);
```
