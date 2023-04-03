#ifndef CPP_TEMPLATE_WORKSHOP_ALGORITHM
#define CPP_TEMPLATE_WORKSHOP_ALGORITHM

// cpp stl 
#include <type_traits>
#include <vector>
#include <functional>
#include <iterator>

// local
#include "detail/template.hpp"
#include "detail/algorithm.hpp"

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


//------------------------------------------------------------------------------
// map 

/**
 * @brief evaluate function with the elements of containers grouped by index
 * @param f a function to call 
 * @param idx the first index to evaluated
 * @param len the count of indexes to evaluate
 * @param c the first container 
 * @param cs... the remaining containers
 * @return a container R of the results from calling f with elements in c and cs...
 */
template <typename R, typename F, typename C, typename... Cs>
R 
map_range_to(F&& f, std::size_t idx, std::size_t len, C&& c, Cs&&... cs) {
    R ret(len);
    detail::algorithm::map(ret.begin(),
                           len,
                           std::forward<F>(f),
                           std::next(c.begin(),idx),
                           std::next(cs.begin(),idx)...);
    return ret;
}

template <typename F, typename... Cs>
using map_default_return_type = std::vector<
    detail::templates::function_return_type<F,Cs...>>;

/**
 * @brief evaluate function with the elements of containers grouped by index 
 *
 * Evaluation begins at index 0, and ends when every element in container c has 
 * been iterated.
 *
 * @param f a function to call 
 * @param c the first container 
 * @param cs... the remaining containers
 * @return an std::vector of the results from calling f with elements in c and cs...
 */
template <typename F,
          typename C,
          typename... Cs>
map_default_return_type<F,C,Cs...>
map(F&& f, C& c, Cs&&... cs) {
    return map_range_to<map_default_return_type<F,C,Cs...>>(
            std::forward<F>(f), 
            0,
            size(c),
            std::forward<C>(c), 
            std::forward<Cs>(cs)...);
}

}

#endif
