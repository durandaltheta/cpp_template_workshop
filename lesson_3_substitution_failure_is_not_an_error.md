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

Writing multiple versions of templates in this way is the simplest way to create specialization. In certain cases this allows other users to write their *own* template specializations building on the ones written before. An example of this can be seen in global operator overloads:
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

// overloading the '+' operator in the case where MyClass is on the right hand side of the operation
template <typename T>
int operator+(T&& lhs, MyClass rhs) {
    // use MyClass's internal standard addition operator
    return rhs + lhs; 
}
```

## SFINAE specialization selection
## SFINAE method detection
A very advanced usage of SFINAE can occur when trying to write template specializations which are selected based on whether a templated type has a necessary method. The example I am about to show is [potentially no longer required as of c++20 using constraints](https://en.cppreference.com/w/cpp/language/constraints), however it is extremely useful as a measuring stick for a developer's ability to understand SFINAE in action. 

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

Almost all `std` containers have the method `size()`, but not all. Example: `std::forward_list<T>`.

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

/**
 * @brief call C::size() if member exists, else calculate the size using iterators
 * @param c a container 
 * @return the size of the container
 */
template <typename C>
size_t // size_t is convertable from all std:: container `size_type`s
size(C&& c) { 
    return detail::algorithm::size(c, std::integral_constant<bool, detail::algorithm::has_size<C>::has>()); 
}

}
```

When our `size()` function is called with an argument type `C`, it selects the proper `detail::size()` implementation based on the result of the compile time expression `std::integral_constant<bool, detail::algorithm::has_size<C>::has>()`.

## Exercises 
### Exercise - size()
### Exercise - rvalue slice()
### Extra Credit - const slice()
