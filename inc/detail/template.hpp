#ifndef SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_TEMPLATE
#define SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_TEMPLATE

#include <type_traits>
#include <functional>

namespace sca { // simple cpp algorithm
namespace detail { 
namespace templates {

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

}
}
}

#endif
