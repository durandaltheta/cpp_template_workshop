#ifndef SIMPLE_CPLUSPLUS_ALGORITHM
#define SIMPLE_CPLUSPLUS_ALGORITHM

// cpp stl 
#include <type_traits>
#include <vector>
#include <functional>
#include <iterator>
#include <memory>
#include <tuple>
#include <optional>

// local
#include "detail/template.hpp"
#include "detail/algorithm.hpp"

/**
 * A NOTE ON API DESIGN
 * As a note, much of the complexity of these templates is caused by more
 * effort being put into usability for the user, rather than implementing 
 * minimalist algorithms. 
 *
 * For instance, the algorithms defined in the c++ standard library typically 
 * deal with iterators rather than the containers themselves. Instead, this 
 * library's algorithms accept containers as arguments and return containers, 
 * because this leaves the smallest amount of work for the user and reduces risk 
 * of exception throwing bugs. This also helps the user avoid making trivial 
 * efficiency mistakes when writing algorithm code.
 *
 * The choice to deal with containers instead of iterators causes a cascade of 
 * issues this library must address, such as what output container types to use 
 * or how to calculate the size of arbitrary output containers, efficiently, in 
 * advance. 
 * 
 * However it is my opinion that this is a valuable exercise because if the 
 * library doesn't do these calculations automatically then it is up to the 
 * user to do them manually, opening the door for unwanted bugs.
 *
 * PROVIDED ALGORITHMS 
 * The algorithms in this header library are intended for general usecases and 
 * composability (the results of one algorithm can often be used as an argument 
 * in another). They are not exhaustive, but should cover the majority of
 * simple data processing usecases.
 *
 * Algorithms and Objects provided by this header:
 * size() - return a container's size
 * to() - copy from an iterable object to a designated output container type
 * slice() - return a (potentially const) object capable of iterating a subset of a container
 * mslice() - return an object capable of iterating a mutable subset of a container
 * group() - return a container composed of all elements of all argument containers
 * reverse() - return a container whose elements are in reverse order of input container
 * filter() - return a container filled with only elements which return true when applied to a function
 * map() - return the results of applying all elements of argument containers to a function
 * fold() - calculate a result after iterating through all elements of argument containers
 * each() - apply a function to every element of a container
 * all() - return true if all elements return true when applied to a function
 * some() - return true if at least one element returns true when applie to a function
 */

namespace sca { // simple cpp algorithm

//------------------------------------------------------------------------------
// default container type 

/**
 * @brief vectors are almost always the correct default choice
 *
 * In C++, algorithms are typically fastest with vectors, so algorithms 
 * implemented in this library typically convert to them internally and return 
 * the result vectors. 
 * 
 * If the user requires a container of elements be converted to another 
 * container type after the data has been processed they can use the `to<T>()` 
 * algorithm defined below.
 */
template <typename T>
using vector = std::vector<T>;

//------------------------------------------------------------------------------
// size

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

//------------------------------------------------------------------------------
// to 

/**
 * @brief copy or move elements from a container of one type to a container of another
 * @param c container of elements
 * @return a container of the target type Result
 */
template <typename Result, typename C>
auto
to(C&& c) {
    Result ret(size(c));
    detail::algorithm::range_copy_or_move(typename std::is_lvalue_reference<C>::type(), ret.begin(), c.begin(), c.end());
    return ret;
}


//------------------------------------------------------------------------------
// slice 

/**
 * @brief the underlying type returned by `slice()` representing a subset of a container
 *
 * This object implements `begin()` and `end()`, returning iterators to the 
 * beginning and end of a range of values in the source container.
 *
 * There is no need to use this object directly. `slice()` methods will detect 
 * the necessary template information and return the proper `slice_of` object.
 */
template<typename C>
class slice_of {
    typedef detail::templates::unqualified<C> UC;
    typedef typename UC::iterator iterator;
    std::shared_ptr<UC> m_mem; // place to hold container memory if constructed with an rvalue 
    const size_t m_size;
    iterator m_begin;
    iterator m_end;

public:
    typedef typename UC::value_type value_type;
    typedef typename UC::size_type size_type;

    slice_of() = delete; // no default initialization
    slice_of(const C& c, size_t idx, size_t len) = delete; // must use const_slice_of

    // mutable lvalue constructor
    slice_of(C& c, size_t idx, size_t len) :
        m_size(len),
        m_begin(std::next(c.begin(), idx)),
        m_end(std::next(m_begin, len))
    { }

    // rvalue constructor
    slice_of(C&& c, size_t idx, size_t len) :
        m_mem(std::make_shared<UC>(std::move(c))), // keep container in memory
        m_size(len),
        m_begin(std::next(m_mem->begin(), idx)),
        m_end(std::next(m_begin, len))
    { }

    /// return the iterable length of the slice
    inline size_t size() const {
        return m_size;
    }

    /// return an iterator to the beginning of the container subset
    inline auto begin() {
        return m_begin;
    }

    /// return an iterator to the end of the container subset
    inline auto end() { 
        return m_end;
    }
};

/// const variation of slice_of
template <typename C>
class const_slice_of {
    typedef detail::templates::unqualified<C> UC;
    typedef typename UC::const_iterator const_iterator;
    const size_t m_size;
    const_iterator m_cbegin;
    const_iterator m_cend;

public:
    typedef typename UC::value_type value_type;
    typedef typename UC::size_type size_type;

    const_slice_of() = delete; // no default initialization

    // const lvalue constructor
    const_slice_of(const C& c, size_t idx, size_t len) :
        m_size(len),
        m_cbegin(std::next(c.cbegin(), idx)),
        m_cend(std::next(m_cbegin, len))
    { }

    /// return the iterable length of the slice
    inline size_t size() const {
        return m_size;
    }

    /// return a const_iterator to the beginning of the container subset
    inline auto begin() const {
        return m_cbegin;
    }

    /// return a const_iterator to the end of the container subset
    inline auto end() const {
        return m_cend;
    }
};

/**
 * @brief create a `slice_of` object from a container which allows iteration over a subset of another container
 *
 * This implementation only gets selected when the input container is an rvalue.
 * The `slice_of` object will keep the original container in memory as long as 
 * the `slice_of` object exists.
 *
 * Typical usecase is to use `auto` as the returned variable's type:
 * ```
 * auto my_slice = sca::slice(my_container, 0, 13);
 * auto my_result = sca::map(my_function, my_slice);
 * ```
 *
 * Or to use the slice inline:
 * ```
 * auto my_result = sca::map(my_function, sca::slice(my_container, 0, 13));
 * ```
 *
 * @param idx starting index of the range of values 
 * @param len ending index of the range of values
 * @param c container to take slice of
 * @return a tuple of slices from one or more input containers
 */
template <typename C>
auto
slice(C&& c, size_t idx, size_t len) {
    return slice_of<C>(std::forward<C>(c), idx, len);
}

template <typename C>
auto
slice(const C& c, size_t idx, size_t len) {
    return const_slice_of<C>(c, idx, len);
}

// explicitly catch mutable lvalue reference so compiler doesn't convert to `const C&`
template <typename C>
auto
slice(C& c, size_t idx, size_t len) {
    return const_slice_of<C>(c, idx, len);
}

/**
 * @brief create a mutable `slice_of` object which allows iteration of a subset of another container
 *
 * This is the mutable reference implementation of the algorithm, returning a 
 * `slice_of`. This can be dangerous when used inline carelessly, as it will be 
 * treated as an rvalue by algorithms causing unexpected swaps. Best usage is to 
 * explicitly save the result of this method as an lvalue before usage:
 * ```
 * auto my_lvalue_slice = sca::mslice(my_container, 0, 13);
 * auto my_result = sca::map(my_function, my_lvalue_slice));
 * ```
 *
 * @param idx starting index of the range of values 
 * @param len ending index of the range of values
 * @param c container to take slice of
 * @return a tuple of slices from one or more input containers
 */
template <typename C>
auto
mslice(C&& c, size_t idx, size_t len) {
    return slice_of<C>(std::forward<C>(c), idx, len);
}

// explicitly catch mutable lvalue reference so compiler doesn't convert to `const C&`
template <typename C>
auto
mslice(C& c, size_t idx, size_t len) {
    return slice_of<C>(c, idx, len);
}


//------------------------------------------------------------------------------
// group

/**
 * @brief assemble a container containing all elements of two or more containers 
 *
 * @param c the first container whose elements should be grouped together with the others
 * @param c2 the second container whose elements should be grouped together with the others
 * @param cs optional, additional containers whose elements should be grouped together with the others
 * @return a container containing all elements of the arguments
 */
template <typename C, typename C2, typename... Cs>
auto
group(C&& c, C2&& c2, Cs&&... cs) {
    using UC = detail::templates::unqualified<C>;
    sca::vector<typename UC::value_type> ret(detail::algorithm::sum(size(c), size(c2), size(cs)...));
    detail::algorithm::group(
            ret.begin(), 
            std::forward<C>(c), 
            std::forward<C2>(c2), 
            std::forward<Cs>(cs)...);

    return ret;
}


//------------------------------------------------------------------------------
// reverse

/** 
 * @brief return a container where the order of elements is the reverse of the input container
 * @param container an input container 
 * @return a new container with elements reversed from the input container
 */
template <typename C>
auto
reverse(C&& container) {
    using UC = detail::templates::unqualified<C>;
    sca::vector<typename UC::value_type> res(size(container));
    detail::algorithm::range_copy_or_move(typename std::is_lvalue_reference<C>::type(), res.begin(), container.begin(), container.end());
    std::reverse(res.begin(), res.end());
    return res; 
}


//------------------------------------------------------------------------------
// filter

/**
 * @brief return a container of results for which applying the predicate function returns true on each element of the input container
 *
 * Return container defaults to `std::vector<T>`, where `T` is the type
 * contained in the input container
 *
 * @param f a predicate function which gets applied to each element of the input container
 * @param container the input container
 */
template <typename F, typename C>
auto
filter(F&& f, C&& container) {
    sca::vector<typename C::value_type> ret(size(container));
    size_t cur = 0;

    for(auto& e : container) {
        if(f(e)) {
            copy_or_move(typename std::is_lvalue_reference<C>::type(), ret[cur], e);
            ++cur;
        }
    }

    ret.resize(cur);
    return ret;
}


//------------------------------------------------------------------------------
// map 

/**
 * @brief evaluate function with the elements of containers grouped by index and return a container filled with the results of each function call
 *
 * Evaluation begins at index 0, and ends when every traversible element in 
 * container c has been iterated. It returns an `sca::vector<T>` (where `T` is 
 * the deduced return value of user function `f`) containing the result of 
 * every invocation of user function `f`.
 *
 * Each container can contain a different value type as long as the value type 
 * can be passed to the function.
 *
 * @param f a function to call 
 * @param c the first container 
 * @param cs... the remaining containers
 * @return a container R of the results from calling f with elements in c and cs...
 */
template <typename F, typename C, typename... Cs>
auto
map(F&& f, C&& c, Cs&&... cs) {
    using Result = sca::vector<detail::templates::function_return_type<F,typename C::value_type, typename Cs::value_type...>>;
    Result ret(size(c));
    detail::algorithm::map(ret.begin(), c.begin(), c.end(), cs.begin()...);
    return ret;

}


//------------------------------------------------------------------------------
// fold 

/**
 * @brief perform a calculation on the values stored in a range of indices across one or more containers 
 *
 * Evaluation ends when traversible element in container c has been iterated. 
 *
 * The argument function must accept the current value as its first argument, 
 * and one or more elements stored in the current index (the count of elements 
 * passed to the function is identical to the number of containers).
 *
 * The value returned by the function becomes the new current value. When all 
 * indices have been processed `fold()` will return the final return value of 
 * the function.
 *
 * Each container can contain a different value type as long as the value type 
 * can be passed to the function.
 *
 * @param f the calculation function 
 * @param init the initial value of the calculation being performed 
 * @param c the first container whose elements will be calculated 
 * @param cs optional additional containers whose elements will also be calculated one
 * @return the final calculated value returned from function f
 */
template <typename F, typename Result, typename C, typename... Cs>
auto
fold(F&& f, Result&& init, C&& c, Cs&&... cs) {
    return detail::algorithm::fold(f, std::forward<Result>(init), c.begin(), c.end(), cs.begin()...);
}


//------------------------------------------------------------------------------
// for_each

/**
 * @brief evaluate function with the elements of containers grouped by index 
 *
 * Evaluation ends when traversible element in container c has been iterated. 
 *
 * No value is returned from this function, any changes are side effects of 
 * executing the function.
 *
 * Each container can contain a different value type as long as the value type 
 * can be passed to the function.
 *
 * @param f a function to call 
 * @param c the first container 
 * @param cs... the remaining containers
 */
template <typename F, typename C, typename... Cs>
void
each(F&& f, C&& c, Cs&&... cs) {
    detail::algorithm::each(f, c.begin(), c.end(), cs.begin()...);
}


//------------------------------------------------------------------------------
// all

/**
 * @brief verify if all input elements cause the argument function to return `true`
 *
 * The set of elements from all input containers at the current index are
 * simultaneously passed to the predicate function. Element argument ordering 
 * matches the order their source containers appear in the function invocation.
 *
 * Each container can contain a different value type as long as the value type 
 * can be passed to the function.
 *
 * @param f a predicate function applied to elements of input containers
 * @param c the first container whose elements will have f applied to 
 * @param cs the optional remaining containers whose elements will have
 * f applied to
 * @return `true` if `f` returns `true` for all iterated elements, else `false`
 */
template <typename F, typename C, typename... Cs>
bool 
all(F&& f, C&& c, Cs&&... cs) {
    return detail::algorithm::all(f, c.begin(), c.end(), cs.begin()...);
}


//------------------------------------------------------------------------------
// some

/**
 * @brief verify if at least one input elements cause an argument function to return `true`
 *
 * The set of elements from all input containers at the current index are
 * simultaneously passed to the predicate function. Element argument ordering 
 * matches the order their source containers appear in the function invocation.
 *
 * Each container can contain a different value type as long as the value type 
 * can be passed to the function.
 *
 * @param f a predicate function applied to elements of input containers
 * @param c the first container whose elements will have f applied to 
 * @param cs the optional remaining containers whose elements will have
 * f applied to
 * @return `true` if `f` returns `true` for at least one iterated elements, else `false`
 */
template <typename F, typename C, typename... Cs>
bool 
some(F&& f, C&& c, Cs&&... cs) {
    return detail::algorithm::some(f, c.begin(), c.end(), cs.begin()...);
}

}

#endif
