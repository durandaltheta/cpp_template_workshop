#ifndef CPP_TEMPLATE_WORKSHOP_TEMPLATE
#define CPP_TEMPLATE_WORKSHOP_TEMPLATE 

// cpp stl 
#include <type_traits>
#include <vector>
#include <functional>
#include <iterator>

// local
#include "detail/template.hpp"

/**
 * This is where code goes which can be included by a user, like this project's
 * unit tests. It defines the public API of our template library.
 */

namespace cta { // cpp template algorithm

/**
 * @brief call C::size() if member exists, else calculate the size using iterators
 * @param c a container 
 * @return the size of the container
 */
template <typename C>
inline size_t // size_t is generally convertable from all container `size_type`s
size(C&& c) { 
    return detail::template::size(
            c, 
            // if this template resolves to an `std::true_type`, then the 
            // template which calls `c.size()` will be selected. Otherwise, the 
            // size of `c` will be determined by iterating through it.
            std::integral_constant<
                    bool, 
                    detail::template::has_size<C>::has 
            >()
    ); 
}

/**
 * @brief call C::resize if member exists, else resize by reassignment
 * @param c a container 
 * @param n new target size 
 */
template <typename C>
void 
resize(C&& c, size_t n) 
{ 
    using UC = detail::templates::unqualified<C>;
    detail::algorithm::resize_(
            c, 
            n, 
            std::integral_constant
            <
                bool, 
                detail::algorithm::has_resize<UC>::has 
            >()); 
}

// map
template <typename R,
          typename F,
          typename C,
          typename... Cs>
std::vector<R>
map_to(F&& f, C& c, Cs&&... cs) {
    std::vector<R> ret(size(c));
    detail::map(ret.begin(), std::forward<F>(f), c.begin(), cs.begin()...);
    return ret;
}

template <typename F,
          typename C,
          typename... Cs>
std::vector<
   detail::templates::function_return_type<
       F,
       typename C::value_type,
       typename Cs::value_type...
   >
>
map(F&& f, C& c, Cs&&... cs) {
    using R = std::vector<
       detail::templates::function_return_type<
           F,
           typename C::value_type,
           typename Cs::value_type...
       >
    >;

    return map_to<R>(std::forward<F>(f), std::forward<C>(c), std::forward<Cs>(cs)...);
}

}

#endif
