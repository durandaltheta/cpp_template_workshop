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
// this function is most specialized and will be selected when `add()`ing two `std::string`s
std::string add(std::string s, std::string s2) {
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

## SFINAE challenge 
Before heading into the unit tests, consider studying the [sfinae challenge section](sfinae_challenge_section.md). This section covers "method detection", an sfinae example too complicated to be required by this course for the non-extra credit unit tests. HOWEVER, it is a very useful example because of the challenge it presents, so take it as an opportunity to *really grok* some of the concepts presented in this lesson.
