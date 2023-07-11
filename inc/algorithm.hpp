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
 * This decision to avoid iterators in algorithm API also removes some of the 
 * inherent control that direct usage of iterators enables, such as 
 * pre-advancing iterators to a desired index, or selecting a custom end 
 * iterator. To address such limitations, users can construct slices of 
 * containers with calls to `slice()` and `mutable_slie()`. The returned 
 * `slice_of` or `const_slice_of` objects return iterators over a subset range 
 * of the source container when their `begin()` and `end()` methods are called. 
 * `slice_of`/`const_slice_of` objects can be passed as arguments to any 
 * algorithm which accepts containers.
 *
 * PROVIDED ALGORITHMS 
 * The algorithms in this header library are intended for general usecases and 
 * composability (the results of one algorithm can often be used as an argument 
 * in another). They are not exhaustive, but should cover the majority of
 * simple data processing usecases.
 *
 * Algorithms provided by this header:
 * size() - return a container's size
 * resize() - resize a container
 * to() - copy from an iterable object to a designated output container type
 * slice() - return a (potentially const) object capable of iterating a subset of a container
 * mutable_slice() - return a mutable object capable of iterating a subset of a container
 * group() - return a container composed of all elements of all argument containers
 * split() - return partitions of a container
 * reverse() - return a container whose elements are in reverse order of input container
 * filter() - return a container filled with only elements which return true when applied to a function
 * map() - return the results of applying all elements of argument containers to a function
 * fold() - calculate a result after iterating through all elements of argument containers
 * each() - apply a function to every element of a container
 * all() - return true if all elements return true when applied to a function
 * some() - return true if at least one element returns true when applie to a function
 *
 * As stated in the note on design, this library specifically addresses the 
 * fact that the standard library algorithms don't deal directly with 
 * containers. 
 *
 * However, there are many algorithms in the standard library which don't 
 * *need* to deal directly with containers without impacting the user or
 * increasing risk of bugs. Those algorithms are good enough and are not 
 * addressed here.
 */

namespace sca { // simple cpp algorithm

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
using default_container = std::vector<T>;

//------------------------------------------------------------------------------
// size

/**
 * @brief call C::size() if member exists, else calculate the size using iterators
 * @param c a container 
 * @return the size of the container
 */
template <typename C>
size_t // size_t is generally convertable from all container `size_type`s
size(C&& c) { 
    return detail::algorithm::size(c, std::integral_constant<bool, detail::algorithm::has_size<C>::has>()); 
}


//------------------------------------------------------------------------------
// resize

/**
 * @brief on each argument container call resize() if member exists, else resize by reassignment
 * @param c a container 
 * @param sz new target size 
 */
template <typename C, typename... Cs>
void 
resize(C&& c, size_t sz) { 
    detail::algorithm::resize(c, sz, std::integral_constant<bool, detail::algorithm::has_resize<C>::has>());
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
    auto it = ret.begin();

    detail::algorithm::range_copy_or_move(std::is_lvalue_reference<C>(), ret.begin(), c.begin(), c.end());

    return ret;
}


//------------------------------------------------------------------------------
// slice 

namespace orientation {
    /// type hint struct for indicating forward iteration
    struct forward { };
    
    /// type hint struct for indicating reverse iteration
    struct reverse { };
    
    template <typename T>
    auto begin(orientation::forward, T& t, size_t idx) {
        return std::next(t.begin(), idx);
    }
    
    template <typename T>
    auto begin(orientation::reverse, T& t, size_t idx) {
        return std::next(t.rbegin(), idx);
    }
    
    template <typename T>
    auto cbegin(orientation::forward, T& t, size_t idx) {
        return std::next(t.cbegin(), idx);
    }
    
    template <typename T>
    auto cbegin(orientation::reverse, T& t, size_t idx) {
        return std::next(t.crbegin(), idx);
    }
}

/**
 * @brief the underlying type returned by `slice()` representing a subset of a container
 *
 * This object implements `begin()` and `end()`, returning iterators to the 
 * beginning and end of a range of values in the source container.
 *
 * There is no need to use this object directly. `slice()` methods will detect 
 * the necessary template information and return the proper `slice_of` object.
 */
template<typename ORIENTATION, typename C>
struct slice_of {
    typedef typename C::value_type value_type;
    typedef typename C::size_type size_type;

    slice_of() = delete; // no default initialization
    slice_of(size_t idx, size_t len, const C& c) = delete; // must use const_slice_of

    // rvalue constructor
    template <typename C2, class = detail::templates::enable_if_rvalue<C2>>
    slice_of(size_t idx, size_t len, C2&& c) :
        m_idx(idx),
        m_len(len),
        m_mem(std::make_shared<C2>(std::move(c))), // keep container in memory
        m_ref(*m_mem)
    { }

    // mutable lvalue constructor
    template <typename C2>
    slice_of(size_t idx, size_t len, C2& c) :
        m_idx(idx),
        m_len(len),
        m_ref(c)
    { }

    /// return the iterable length of the slice
    inline size_t size() const {
        return m_len - m_idx;
    }

    /// return an iterator to the beginning of the container subset
    inline auto begin() {
        return orientation::begin(ORIENTATION(), m_ref, m_idx);
    }

    /// return an iterator to the end of the container subset
    inline auto end() {
        return std::next(begin(), m_len);
    }

private:
    const size_t m_idx;
    const size_t m_len;
    // place to hold container memory if constructed with an rvalue 
    std::shared_ptr<C> m_mem; 
    C& m_ref; // reference to container 
};

/**
 * @brief the underlying (const variation) type returned by `slice()`
 *
 * This separate object is created to provide `const_iterator`s for non-rvalue
 * containers.
 */
template <typename ORIENTATION, typename C>
struct const_slice_of {
    typedef typename C::value_type value_type;
    typedef typename C::size_type size_type;

    const_slice_of() = delete; // no default initialization
   
    // const lvalue constructor
    const_slice_of(size_t idx, size_t len, const C& c) :
        m_idx(idx),
        m_len(len),
        m_cref(c)
    { }

    /// return the iterable length of the slice
    inline size_t size() const {
        return m_len - m_idx;
    }

    /// return a const_iterator to the beginning of the container subset
    inline auto begin() const {
        return orientation::cbegin(ORIENTATION(), m_cref, m_idx);
    }

    /// return a const_iterator to the end of the container subset
    inline auto end() const {
        return std::next(begin(), m_len);
    }

private:
    const size_t m_idx;
    const size_t m_len;
    const C&  m_cref; // reference to const container
};

/**
 * @brief create a `slice_of` object from an rvalue container which allows iteration over a subset of another container
 *
 * This implementation only gets selected when the input container is an rvalue.
 * The `slice_of` object will keep the original container in memory as long as 
 * the `slice_of` object exists.
 *
 * Typical usecase is to use `auto` as the returned variable's type:
 * ```
 * auto my_slice = sca::slice(0, 13, std::move(my_container));
 * auto my_result = sca::map(my_function, my_slice);
 * ```
 *
 * Or to use the slice inline:
 * ```
 * auto my_result = sca::map(my_function, sca::slice(0, 13, std::move(my_container)));
 * ```
 *
 * The slice can instead provide reverse iterators by specifying the 
 * `orientation` in the template:
 * ```
 * auto my_reversed_results = sca::map(my_function, sca::slice<orientation::reverse>(0, 13, std::move(my_container)))
 * ```
 *
 * @param idx starting index of the range of values 
 * @param len ending index of the range of values
 * @param c container to take slice of
 * @return a tuple of slices from one or more input containers
 */
template <typename ORIENTATION = orientation::forward, typename C, class = detail::templates::enable_if_rvalue<C>>
auto
slice(size_t idx, size_t len, C&& c) {
    return slice_of<C, ORIENTATION>(idx, len, std::forward<C>(c));
}

/**
 * @brief create a `const_slice_of` object which allows iteration of a subset of another container
 *
 * This is the lvalue implementation of the algorithm, returning a 
 * `const_slice_of` instead of a `slice_of`. This prevents modification of the 
 * original container, enforcing const references or deep copies.
 *
 * A typical usecase is to use `auto` as the returned variable's type:
 * ```
 * auto my_slice = sca::slice(0, 13, my_container);
 * auto my_result = sca::map(my_function, my_slice);
 * ```
 *
 * Or to use the slice inline:
 * ```
 * auto my_result = sca::map(my_function, sca::slice(0, 13, my_container));
 * ```
 *
 * Since values returned from iterators managed by `const_slice_of` are `const`,
 * functions which accept values from a `const_slice_of` need to accept either 
 * const references to the stored value (`const T&`, where `T` is the value type 
 * stored in the original container) or in the worst case a deep copy `T`.
 *
 * The slice can instead provide reverse iterators by specifying the 
 * `orientation` in the template:
 * ```
 * auto my_reversed_results = sca::map(my_function, sca::slice<orientation::reverse>(0, 13, my_container))
 * ```
 *
 * @param idx starting index of the range of values 
 * @param len ending index of the range of values
 * @param c container to take slice of
 * @return a tuple of slices from one or more input containers
 */
template <typename ORIENTATION = orientation::forward, typename C>
auto
slice(size_t idx, size_t len, const C& c) {
    return const_slice_of<C, ORIENTATION>(idx, len, c);
}

/**
 * @brief create a mutable `slice_of` object from a container which allows iteration over a subset of another container
 *
 * This implementation enforces the creation of a `slice_of` instead of a 
 * `const_slice_of`, which is more dangerous than normal calls to `slice()`.
 * This function should only be used (carefully) if normal calls to `slice()` 
 * won't work, such as cases where in-place container modification is necessary.
 *
 * Specifically, if the returned `slice_of` object is passed inline to an 
 * algorithm, the algorithm will see the `slice_of` object as an rvalue and 
 * attempt to `std::move()` values from it, which can unexpectedly modify the 
 * original source container.
 *
 * WARNING: BAD THINGS CAN HAPPEN TO ORIGINAL CONTAINER IF USED INLINE
 * ```
 * sca::each(my_function, sca::mutable_slice(0, 13, my_container));
 * ```
 *
 * Safe usecase is to use `auto` as the returned variable's type on a separate 
 * line from the call that uses the slice, which should avoid unexpected 
 * `std::swap()`s:
 * ```
 * auto my_slice = sca::mutable_slice(0, 13, my_container);
 * sca::each(my_function, my_slice);
 * ```
 *
 * @param idx starting index of the range of values 
 * @param len ending index of the range of values
 * @param c container to take slice of
 * @return a tuple of slices from one or more input containers
 */
template <typename ORIENTATION = orientation::forward, typename C>
auto
mutable_slice(size_t idx, size_t len, C&& c) {
    return slice_of<ORIENTATION, C>(idx, len, std::forward<C>(c));
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
    default_container<typename C::value_type> ret(detail::algorithm::sum(size(c), size(c2), size(cs)...));
    auto cur = ret.begin();
    auto end = ret.end();

    detail::algorithm::group(cur, end, std::forward<C>(c), std::forward<C2>(c2), std::forward<Cs>(cs)...);

    return ret;
}


//------------------------------------------------------------------------------
// split 

/**
 * @brief split a container into two or more partitions 
 *
 * The size of the final partition is determined by the aggregate sizes of the 
 * preceding partitions. 
 *
 * If the aggregate partition lengths are greater than or equal to the size of 
 * the source container the resulting `std::optional<...>` will be empty.
 *
 * @param len size of partition1
 * @param lens optional sizes of more partitions
 * @return an `std::optional<container<container<T>>>` of the resulting partitions 
 */
template <typename C, typename... size_ts>
auto
split(C&& c, size_t part1len, size_ts... partlens) {
    using R = std::optional<default_container<default_container<typename C::value_type>>>;

    // only partition if we can guarantee all partitions have space to exist
    if(size(c) > detail::algorithm::sum(part1len, partlens...)) {
        R res(std::in_place_t, 1 + sizeof...(partlens));
        detail::algorithm::split(
                std::is_lvalue_reference<C>(),
                res.begin(), 
                c.begin(), 
                c.end(), 
                part1len, 
                partlens...);
        return res;
    } else {
        return R();
    }
}


//------------------------------------------------------------------------------
// reverse

/** 
 * @brief return a container where the order of elements is the reverse of the input container
 *
 * This implementation of the algorithm allows specification of the returned 
 * container type. 
 *
 * @param container an input container 
 * @return a new container with elements reversed from the input container
 */
template <typename C>
auto
reverse(C&& container) {
    default_container<typename C::value_type> res(sz);
    
    range_copy_or_move(std::is_lvalue_reference<C>(), res.begin(), container.begin(), container.end());
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
    default_container<typename C::value_type> ret(size(container));

    size_t actual_size = 0;
    auto first = container.begin();
    auto last = container.end();
    auto ret_first = ret.begin();

    for(; first != last; ++first) {
        if(f(*first)) {
            copy_or_move(std::is_lvalue_reference<C>(), *ret_first, *first);
            ++ret_first;
            ++actual_size;
        }
    }

    resize(ret, actual_size);
    return ret;
}


//------------------------------------------------------------------------------
// map 

/**
 * @brief evaluate function with the elements of containers grouped by index and return a container filled with the results of each function call
 *
 * Evaluation begins at index 0, and ends when every element in container c has 
 * been iterated. It returns a container<T> (default `std::vector<T>`) where 
 * `T` is the deduced return value of user function `F`.
 *
 * @param f a function to call 
 * @param c the first container 
 * @param cs... the remaining containers
 * @return a container R of the results from calling f with elements in c and cs...
 */
template <typename F, typename C, typename... Cs>
auto
map(F&& f, C&& c, Cs&&... cs) {
    using Result = default_container<detail::templates::function_return_type<F,typename C::value_type>>;
    size_t len = size(c);
    Result ret(len);
    detail::algorithm::map(len, ret.begin(), c.begin(), cs.begin()...);
    return ret;

}


//------------------------------------------------------------------------------
// fold 

/**
 * @brief perform a calculation on the values stored in a range of indices across one or more containers 
 *
 * The argument function must accept the current value as its first argument, 
 * and one or more elements stored in the current index (the count of elements 
 * passed to the function is identical to the number of containers).
 *
 * The value returned by the function becomes the new current value. When all 
 * indices have been processed `fold()` will return the final return value of 
 * the function.
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
    size_t len = size(c);
    return detail::algorithm::fold(f, init, c.begin(), cs.begin()...);
}


//------------------------------------------------------------------------------
// for_each

/**
 * @brief evaluate function with the elements of containers grouped by index 
 *
 * Evaluation begins at index 0, and ends when every element in container c has 
 * been iterated. 
 *
 * No value is returned from this function, any changes are side effects of 
 * executing the function.
 *
 * @param f a function to call 
 * @param c the first container 
 * @param cs... the remaining containers
 * @return a container R of the results from calling f with elements in c and cs...
 */
template <typename F, typename C, typename... Cs>
void
each(F&& f, C&& c, Cs&&... cs) {
    size_t len = size(c);
    default_container<typename C::value_type> ret(len);

    detail::algorithm::each(len, c.begin(), cs.begin()...);
    return ret;

}


//------------------------------------------------------------------------------
// all

/**
 * @brief verify if all input elements cause the argument function to return `true`
 *
 * The set of elements from all input containers at the current index are
 * simultaneously passed to the predicate function. Element argument ordering 
 * matches the order they appear in the function invocation.
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
    return detail::algorithm::all(size(c), f, c.begin(), cs.begin()...);
}


//------------------------------------------------------------------------------
// some

/**
 * @brief verify if at least one input elements cause an argument function to return `true`
 *
 * The set of elements from all input containers at the current index are
 * simultaneously passed to the predicate function. Element argument ordering 
 * matches the order they appear in the function invocation.
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
    return detail::algorithm::some(size(c), f, c.begin(), cs.begin()...);
}

}

#endif
