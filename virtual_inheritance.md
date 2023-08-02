# type erasure using virtual inheritance 
Another mechanism to implement type erasure is through the usage of interface classes. The `virtual` keyword is used to denote a method in a `class` (or `struct`, which are also `class`es under the hood) which can be "overwritten" will at runtime by a child `class` which implements their own version of them. Under the hood, the compiler is using function pointers to determine what *actual* function to call: 
```
#include <iostream>

struct interface {
    virtual void foo() {
        std::cout << "foo" << std::endl;
    }
};

struct child : public interface {
    virtual void foo() { // overwrite the interface implementation
        std::cout << "faa" << std::endl;
    }
};

int main() {
    child c;
    c.foo();
    return 0;
}
```

Executing this program:
```
$ ./a.out 
faa
$
```

Additionally, if a `virtual` method is declaration is post-pended with a ` = 0;` then that method is called a "pure virtual" method. A pure virtual method makes the `class` "abstract". An abstract `class` is what is known as an "interface" in other programming languages, essentially a specification of required methods that an inheriting class can fulfill. 

An abstract `class` cannot be created directly, but it *can* be inherited and if the child `class` fully implements the missing "pure virtual" methods of the abstract class then the child `class` *can* be constructed. 
```
#include <iostream>

struct interface {
    virtual void foo() = 0; // no implementation in a pure virtual function
};

struct child : public interface {
    virtual void foo() { // implements the pure virtual interface method
        std::cout << "faa" << std::endl;
    }
};

int main() {
    child c;
    c.foo();
    return 0;
}
```

Executing this program:
```
$ ./a.out 
faa
$
```

One of the advantages of `virtual` methods and interfaces is that references/pointers to a class which implements the base class/interface can be `dynamic_cast<>()` to a reference/pointer of the base class/interface. This casting process is often done implicitly by the compiler, although it can be explicitly when necessary. When the methods of the cast base class reference/pointer are called, the child classes methods will *actually* be called instead:
```
#include <iostream>

struct interface {
    virtual void foo() = 0;
};

struct child : public interface {
    virtual void foo() { 
        std::cout << "faa" << std::endl;
    }
};

void call_foo(interface& i) {
    i.foo();
}

int main() {
    child c;
    call_foo(c); // implicitly dynamically cast by the compiler to `interface`
    return 0;
}
```

Executing this program:
```
$ ./a.out 
faa
$
```

## Combining virtual inheritance with type erasure 
Here is an example on how type erasure could be implemented using virtual inheritance in an object which can accept any type by-value:
```
#include <type_info>
#include <type_traits>
#include <memory>
#include <exception>
#include <iostream>
#include <string>

struct any_interface {
    virtual void* data() = 0;
    virtual const std::type_info* type() = 0;
    friend cast_interface;
};

template <typename T>
struct any_impl : protected any_interface {
    template <typename T>
    any_impl(T&& t) : m_t(std::forward<T>(t)) { }

    // interface implementations
    virtual void* data() { return (void*)&m_t; }
    virtual const std::type_info* type() { return m_type; }

private:
    T m_t;
    const std::type_info* m_type = &(typeid(T));
};

struct bad_any_cast : public std::exception {
    const char* what() const {
        return "the cast type is incorrect you dummy!";
    }
};

struct any {
    // select specific constructors when assigning from another any object
    any(const any& rhs) : m_any_int(rhs.m_any_int) { }
    any(any& rhs) : m_any_int(rhs.m_any_int) { }
    any(any&& rhs) : m_any_int(rhs.m_any_int) { }

    // template constructor for assignment from some value T
    template <typename T>
    any(T&& t) : 
        m_any_int(allocate(std::forward<T>(t)) )
    { }

    // can implement assignment operators in a similar fashion to the constructors 
    // ...
    
    // return `true` if this object contains a value, else `false`
    operator bool() {
        return m_any_int; // calls `std::shared_ptr<any_interface>::operator bool()`
    }

private:
    template <typename T>
    any_interface* allocate(T&& t) {
        return new any_impl<std::decay_t<T>>(std::forward<T>(t));
    }

    std::shared_ptr<any_interface> m_any_int;

    // allow friend function to access private members
    friend template <typename T> T any_cast(any&);
};

template <typename T>
T any_cast(any& a) {
    typedef std::decay_t<T> DT;

    if(a.m_any_int && *(a.m_any_int->type()) != typeid(DT)) {
        throw std::bad_any_cast();
    }

    return *((DT*)(m_any_int->data()));
}

int main() {
    any a(std::string("faa"));
    std::cout << std::any_cast<std::string>(a) << std::endl;
    return 0;
}
```

Executing this program:
```
$ ./a.out 
faa
$
```
