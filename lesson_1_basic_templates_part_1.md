# Basic Templates Part 1
## What Are Templates?
In `c++` a template is a recipe for code that the compiler will finish writing for you *as needed* (and not before!). Templates always begin with the keyword `template`, followed by a template parameter list beginning with a `<` and ending with a `>`.  Within the `template` parameter list you can specify templated types with the keywords `typename` or `class`.

After the `template` header follows a function, a `struct`/`class` definition, or a `using typename =` type alias.

A psuedo-code example:
```
template <typename T>`
void my_template_function(T t) {
    // do things...
}

template <class T>
void my_template_function2(T t) {
    // do other things...
}
```

It is important to note that `c++` templates are *recipes* for final code, *NOT* the final code itself. This means that final `c++` code is *not generated by the compiler until a template is called in code*. This is extremely useful in some ways, but it can leave unexpected errors in the code where a template will not be able to generate valid code for some types if your templates are not thoroughly unit tested!

## Manual Type Specification 
The `typename`/`class` types defined in a `template` are known until the `template` is called in code. This means a template has the ability to *potentially* represent valid code for different types. 

The most basic way a template is called in code is to write the name of the `template` followed by `<`/`>` brackets with the types you want to use between them. You should immediately recognize this pattern with `std::vector`:
```
#include <vector>
#include <iostream>

int main() {
    // `std::vector` is a template class
    std::vector<int> v{1,2,3};

    for(auto& e : v) {
        std::cout << e << " ";
    }

    std::cout << endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
1 2 3 
$
```

Consider the fact that different objects have different implementations of the same function. Both `int` and `std::string` can be "added" together using the `+` operator. Under the hood, the `+` operator is just a function (and `c++` allows you to write these operators for your objects if you desire). 

Textually, the `+` is just a symbol that happens to have multiple meanings. Templates allow you to use this ambiguity to write 'code' which the compiler can use in multiple circumstances. Take this `template` example using `+`:
```
#include <string>
#include <iostream>

template <typename T>
T add(T t1, T t2) {
    return t1 + t2;
}

int main() {
    std::cout << add<int>(1, 2) << std::endl;
    std::cout << add<std::string>(std::string("hello "),  std::string("world")) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
3
hello world
$
```

## Type Deduction
One of the most useful aspects of `c++` `template`s is the fact that the compiler can often determine the final types *without* needing to use `<`/`>` brackets. Using our `add` template again:
```
#include <string>
#include <iostream>

template <typename T>
T add(T t1, T t2) {
    return t1 + t2;
}

int main() {
    // the compiler has enough information just by calling add() with integers
    // or std::strings that it can finish the template without <> brackets.
    std::cout << add(1, 2) << std::endl;
    std::cout << add(std::string("hello "),  std::string("world")) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
3
hello world
$
```

## Multiple Template Types
There's no limitation on the number of templated types a template can have. This allows us to write templates which can take more than one unknown argument type. Rewriting out `add` template:
```
#include <string>
#include <iostream>

template <typename T, typename T2>
T add(T t1, T2 t2) {
    return t1 + t2;
}

int main() {
    std::cout << add(1, 2) << std::endl;
    // compiler type coerces double to an integer inside the internal `+` function
    std::cout << add(1, 2.0) << std::endl;
    std::cout << add(1.0, 2.0) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out
3
3
3.0
$
```

## Type Specialization 
Sometimes you need explicitly write different behavior for specific subcases. This can be accomplished by writing more specific versions of your template alongside more general ones.
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

The order that template specializations are written can be quite in important for selecting the proper case. This is a whole topic in and of itself, but it should be noted that the compiler likes to take the *first* valid template it can use when it needs one, so you need to keep in mind your ordering so that you can ensure the proper template is selected.

## Default Type Assignment 
One final point is that templates types can have default values. This is often then case when a library template code supports a primary way of doing something in a template, but leaves the door open for custom overrides. When using a default type assignment the user does not have to specify the type in `<`/`>` brackets. Such is the case with `std::vector`, whose template class declaration is as follows:
```
template<
 class T,
 class Allocator = std::allocator<T>
> class vector;
```

This means that a `std::vector<int>` (where `T` is set to `int`) is more of a nickname for the actual `std::vector` type. The *actual* type is `std::vector<int, std::allocator<int>>`, but the compiler doesn't require you specify the `Allocator` type if you use the default allocator.

Here is an example where a template has a default type:
```
#include <vector> 
#include <list>
#include <iostream>

template <typename T, typename Container = std::vector<T>>
Container construct_container_with_one_element(T t) {
    return Container{t};
}

int main() {
    std::vector v = construct_container_with_one_element(1);
    std::list<int> l = construct_container_with_one_element<int, std::list<int>>(2);

    std::cout << v.front() << std::endl;
    std::cout << l.front() << std::endl;

    return 0;
}
```

Executing this program:
```
$ ./a.out
1
2
$
```