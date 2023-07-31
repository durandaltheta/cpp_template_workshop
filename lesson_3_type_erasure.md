# Type Erasure 
## C Type Erasure
A `void*` is a basic `c` pointer type which can represent any other type of pointer. This allows the passing of data between points in a program in a type agnostic fashion. This is called [type erasure](https://en.wikipedia.org/wiki/Type_erasure):
```
#include <iostream>

print_string_from_void(void* str) {
    // cast void* back to string 
    std::cout << (const char*)str << std::endl;
}

int main() {
    const char* s = "hello!";
    void* v = (void*)s; // cast string to void*
    print_string_from_void(v);
    return 0;
}
```

Executing this program:
```
$ ./a.out
hello!
$
```

However, the above is not very useful if the code that handles the eventual `void*` doesn't know how to cast the pointer back to a known type. This problem is solved in one of two ways, with function pointers or with paired identifier values.

Function pointers are just what they sound like: pointers to a function. See subsection "Pointers to functions" in [this link](https://en.cppreference.com/w/cpp/language/pointer) for details. It is possible to package a `void*` with function pointer which knows how to handle said `void*`:
```
#include <iostream>

struct packaged_void_pointer {
    void (*handler)(void*);
    void* data;
};

print_string_from_void(void* str) {
    // cast void* back to const char*
    std::cout << (const char*)str << std::endl;
}

int main() {
    packaged_void_pointer pvp{ print_string_from_void, "hello" };
    pvp.handler(pvp.data);
    return 0;
}
```

Executing this program:
```
$ ./a.out
hello!
$
```

Function pointers only handle the usecase where origin code knows what behavior needs to executed on a piece of data ahead of time. To interact with our `void*` in ways unknown to the sender of the data we can associate the `void*`'s original type with some other value so we can "decode" it on the other side. 

This technique is often done with enumerations or strings as the associated identifier value:
```
#include <iostream>

enum types {
    is_unknown,
    is_int,
    is_string
};

struct void_pointer_with_id {
    size_t id = types::is_unknown;
    void* data = std::nullptr;
};

void print_void_pointer_with_id(void_pointer_with_id avp) {
    switch(avd.id) {
        case types::is_int:
            std::cout << "void pointer is an int: " << *((int*)(wv.data)) << std::endl;
        case types::is_string:
            std::cout << "void pointer is a std::string: " << (const char*)(wv.data) << std::endl;
        case types::is_unknown:
        default:
            std::cout << "void pointer is of an unknown type" << std::endl;
    }
}

int main() {
    int i = 3;
    const char* s = "foo"
    void_pointer_with_id avp;

    print_void_pointer_with_id(avp);

    avp.id = types::is_int;
    avp.data = &i;
    print_void_pointer_with_id(avp);

    avp.id = types::is_string;
    avp.data = s;
    print_void_pointer_with_id(avp);

    return 0;
}
```

Executing this program:
```
$ ./a.out 
void pointer is of an unknown types
void pointer is an int: 3
void pointer is a std::string: foo
$
```

## C++ Templated Type Erasure
Clever combination of templates with `void*` allows for safer and easier type erasure. For instance, [std::type_info](https://en.cppreference.com/w/cpp/types/type_info) is a convenient standard library utility type which allows us to acquire a unique unsigned integer value associated by the compiler with a given type at compile time returned by the function `std::type_info::hash_code()`, allowing for compiler enforced type safety. An `std::type_info` object for a given type can be acquired with the `c++` expression `typeid(my_type_or_variable_here)`:
```
#include <iostream>

void compare_hash_codes(std::type_info lhs, std::type_info rhs) {
    if(lhs.hash_code() == rhs.hash_code()) {
        std::cout << "lhs type code matches rhs type code" << std::endl;

    } else {
        std::cout << "lhs type code does not match rhs type code" << std::endl;
    }
}

int main() {
    int i = 0;
    const char* s = "foo";

    compare_hash_codes(typeid(i), typeid(int));
    compare_hash_codes(typeid(i), typeid(s));
    compare_hash_codes(typeid(s), typeid("faa"));
    return 0;
}
```

Executing this program:
```
$ ./a.out
lhs type code matches rhs type code
lhs type code does not match rhs type code
lhs type code matches rhs type code
$
```

It should be noted that the above technique is only safe when used on values that are part of the same program, you cannot safely use this technique when sending values via inter-process mechanims.

With templates we can improve this process, it is possible to write an object which "wraps" a value of any type using template functions and then can use further templates to unwrap it at runtime:
```
#include <iostream>
#include <string>

// A struct which can hold a pointer to any value
struct wrapped_value {
    // assign a value to this value wrapper
    template <typename T>
    void assign(T& t) {
        ptr = &t;
        // std::decay<> removes const and references from a type
        code = typeid(typename std::decay<T>).hash_code();
    }

    // return `true` if a value is assigned and the value type matches `T`, else return `false`
    template <typename T>
    bool is() {
        return ptr != std::nullptr && code == typeid(typename std::decay<T>).hash_code();
    }

    // return a reference to the assigned value
    template <typename T>
    T& to() {
        return *(static_cast<T*>(ptr));
    }

private:
    void* ptr = std::nullptr; // default point to nothing
    size_t code = 0;
};

void print_wrapped_value(wrapped_value wv) {
    if(wv.is<int>()) {
        std::cout << "wrapped value is an int: " << wv.to<std::string>() << std::endl;
    } else if(wv.is<std::string>()) {
        std::cout << "wrapped value is a std::string: " << wv.to<std::string>() << std::endl;
    } else {
        std::cout << "wrapped value is of an unknown type" << std::endl;
    }
}

int main() {
    bool b = true;
    int i = 3;
    std::string s = "foo";
    wrapped_value wv;

    wv.assign(b);
    print_wrapped_value(wv);
    wv.assign(i);
    print_wrapped_value(wv);
    wv.assign(s);
    print_wrapped_value(wv);

    return 0;
}
```

Executing this program:
```
$ ./a.out 
wrapped value is of an unknown types
wrapped value is an int: 3
wrapped value is a std::string: foo
$
```

[std::any](https://en.cppreference.com/w/cpp/utility/any) is a `c++17` object which can store any type using type erasure. Some of the techniques show above are possibly how the `c++17` object `std::any` internally encapsulates its data. However, `std::any` extends the above functionality by implementing enforced type checking via `std::bad_any_cast` exception throws as well as being able to store an actual value (not just a pointer to a value) and properly destroy said value when the `std::any` goes out of scope, as necessary. I will leave it to the reader to guess at or research how this is done (hint: function pointers to destructors are involved).
