# Lambdas
## A Gentle Introduction
Lambdas (or "closures" or "anonymous functions") are functions that are created initially without a name and can be created within other functions. This is very different from classic C/C++, where all functions have names and functions cannot normally be defined inside another function.

Practically speaking, this just means that lambdas are functions that are also variables, and can be manipulated like any other variable. Lambdas follow this pattern in c++:

```
[capture0, ..., captureN](arg0, ..., argN){ statement0; ...; statmentN; };
```

Lambdas can be stored as a variable using their real type (specified by the compiler) using auto:
```
auto my_lambda = [capture0, ..., captureN](arg0, ..., argN){ statement0; ...; statmentN; };
```

Lambdas can also be stored in a std::function object, which looks like this:
```
std::function<function_return_type(arg_0_type, ..., arg_N_type)> object_name;
object_name = [capture0, ..., captureN](arg0, ..., argN){ statement0; ...; statmentN; };
```

Lambdas can be invoked using parenthesis '()' with their required arguments. What this might look like in example code:
```
#include <functional>
#include <iostream>

int main(int argc, char**argv)
{
    // the "auto" keyword can be used here instead for my_lambda's type
    std::function<int(int,int)> my_lambda = [](int a, int b){ return a + b; };

    // the following prints "my_lambda: 5"
    std::cout << "my_lambda: " << my_lambda(2,3) << std::endl; 
}
```

Whitespace is arbitrary in lambdas, and can be written in different ways if the function has many statements:

```
void func(int a, int b, int c, int d)
{
    auto l = [](int a, int b, int c, int d)
    {
        const int e = c + a;
        const int f = e + b - d;
        return f;
    }

    std::cout << "lambda output: " << l(1,2,3,4) << std::endl;
}
```

## Lambda Captures
Lambdas can also "capture" values or references from the enclosing scope. This is similar to the behavior of functions with global variables or other normal functions. Example:

```
#include <functional>
#include <iostream>

int main(int argc, char**argv)
{
    int a = 2;
    int b = 3;
    auto my_lambda = [a,b](){ return a+b };

    // the following still prints "my_lambda: 5". Notice how it takes no 
    // arguments.
    std::cout << "my_lambda: " << my_lambda() << std::endl; 
}
```

Lambdas can capture by value: 
```
[my_variable,...](...){...};
```

Or by reference: 
```
[&my_variable,...](..){...};
```

Capturing by reference allows the user to refer back to the original variable, and potentially modify it. 

Values or references can be automatically captured as needed if given a '=' or '&' at the beginning of the capture clause. IE, If the first "variable" in the capture clause is a "=" it will implicitly capture any variable not specified in the clause by value as required. If the first "variable" in the capture clause is instead a "&" values will implicitly be captured by reference instead:
```
[=](..){...}; //default automatic const value capture
[&](..){...}; //default automatic reference capture
```

Even if a default, automatic capture behavior is specified, additional captures can still be specified that use the same or opposite behavior:
```
[=,&a_local_reference,a_local_value,...](..){...};
[&,a_local_value,&a_local_reference,...](..){...};
```

Lambda captures by value are const by default. This allows lambdas to be dynamically constructed on the stack and to pass around state without modifying it.

```
int high_level_func()
{
    int i = 5;
    int u = 6;

    // i and u cannot be modified by the lambda even though they are not 
    // defined const, making this lambda adhere to the functional paradigm
    auto l = [=]{ return i + u; };

    return l(); // return 11
}
```

## Returning Lambdas
Lambdas can also be returned. Functions that take *other* functions as arguments or return *other* functions are called "higher order functions":

```
std::function<int()> high_level_func()
{
    int i = 5;
    int u = 6;

    auto l = [=]{ return i + u; };

    return l; // return the lambda 'l' to calling context
}
```

The main purpose of passing lambdas around is as a method of passing state. Instead of passing around 10 variables from 5 functions, some of the calculation can be done in one place and passed to another function that knows what is needed to complete the calculation.

This is effectively a form of data hiding that is even more private than structs and objects. That is, when passing data around via structures all parties that read the data must include relevant headers for that structure that describes its members, typically public and private.

In the case of lambda captures, what is being passed is much more specialized, and the recipient doesn't need to know *anything* about the internals of the function other than its return type and argument types, making message passing and interfaces much easier and even more of black box. Thus:

```
std::function<int(int)> give_closure(int a, int b, int c)
{
    return [=](int d)
    {
        // The eventual user of this function knows *nothing* about a, b, and 
        // c. It doesn't need to know anything other than it is a 
        // std::function<int(int)> and what the return value is supposed to 
        // represent.
        return (a+b-c)*d;
    }
}
```

A function generated by give_closure() will function even if passed to another thread!
