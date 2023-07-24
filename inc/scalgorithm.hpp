#ifndef SIMPLE_CPLUSPLUS_ALGORITHM
#define SIMPLE_CPLUSPLUS_ALGORITHM

// cpp stl 
#include <type_traits>
#include <vector>
#include <functional>
#include <iterator>
#include <memory>

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
 * size() - return a container's size, regardless if it implements a `::size()` method
 * to() - copy from an iterable object to a designated output container type 
 * pointers() - return container of the addresses of elements in another container
 * slice() - return a (potentially const) object capable of iterating a subset of a container
 * mslice() - return an object capable of iterating a mutable subset of a container
 * group() - return a container composed of all elements of all argument containers
 * reverse() - return a container whose elements are in reverse order of input container
 * filter() - return a container filled with only elements which return true when applied to a Callable
 * map() - return the results of applying all elements of argument containers to a Callable
 * fold() - calculate a result after iterating through all elements of argument containers
 * each() - apply a Callable to every element of a container
 * all() - return true if all elements return true when applied to a Callable
 * some() - return true if at least one element returns true when applied to a Callable
 */

namespace sca { // simple cpp algorithm
namespace detail {

// ----------------------------------------------------------------------------- 
// callable_return_type 

// handle pre and post c++17 
#if __cplusplus >= 201703L
template <typename F, typename... Ts>
using callable_return_type = typename std::invoke_result<std::decay_t<F>,Ts...>::type;
#else 
template <typename F, typename... Ts>
using callable_return_type = typename std::result_of<std::decay_t<F>(Ts...)>::type;
#endif

// -----------------------------------------------------------------------------
// size  

template<typename T>
struct has_size {
    template<typename U, typename U::size_type (U::*)() const> struct SFINAE {};
    template<typename U> static char test(SFINAE<U, &U::size>*);
    template<typename U> static int test(...);
    static const bool has = sizeof(test<T>(0)) == sizeof(char);
};

// Get the size of an object with its `size()` method
template <typename C>
size_t size(C& c, std::true_type) { 
    return c.size(); 
}

// Get the size of an object the *slow* way by iterating through it
template <typename C>
size_t size(C& c, std::false_type) {
    return std::distance(c.begin(), c.end());
}

// ----------------------------------------------------------------------------- 
// copy_or_move  

// Copy or move only one value. Keeping this as a separate template removes 
// having to rewrite templates which need to forward value categories based on 
// different types than their own type. IE, use the value category of a 
// `container<T>` to determine the copy/move operation when assigning values 
// between `container::<T>::iterator`s.
template <typename DEST, typename SRC>
void copy_or_move(std::true_type, DEST& dst, SRC& src) {
    dst = src;
}

template <typename DEST, typename SRC>
void copy_or_move(std::false_type, DEST& dst, SRC& src) {
    dst = std::move(src);
}


// ----------------------------------------------------------------------------- 
// range_copy_or_move 

// Don't use `std::copy()` or `std::move()` because we want to ensure that 
// side effects of incrementing iterators are preserved.
//
// Think of this as a container and value category aware memcpy() :).
template <typename DIT, typename IT>
void range_copy_or_move(std::true_type, DIT&& dst_cur, IT&& src_cur, IT&& src_end) {
    for(; src_cur != src_end; ++src_cur, ++dst_cur) {
        *dst_cur = *src_cur;
    }
}

template <typename DIT, typename IT>
void range_copy_or_move(std::false_type, DIT&& dst_cur, IT&& src_cur, IT&& src_end) {
    for(; src_cur != src_end; ++src_cur, ++dst_cur) {
        *dst_cur = std::move(*src_cur);
    }
}


// ----------------------------------------------------------------------------- 
// sum 

template <typename V>
size_t sum(V cur_sum, V v) {
    return cur_sum + v;
}

template <typename V, typename... Values>
size_t sum(V cur_sum, V v, V v2, Values... vs) {
    return sum(cur_sum + v, v2, vs...);
}


// ----------------------------------------------------------------------------- 
// group operations 

template <typename IT>
void group(IT&& cur) {
}

template <typename IT, typename C, typename... Cs>
void group(IT&& cur, C&& c, Cs&&... cs) {
    detail::range_copy_or_move(typename std::is_lvalue_reference<C>::type(), cur, c.begin(), c.end());
    group(cur, std::forward<Cs>(cs)...);
}


// ----------------------------------------------------------------------------- 
// advance group 

/*
 * The purpose of this algorithm is to increment any number of iterators by reference
 */
template <typename IT>
void advance_group(IT& it) { 
    ++it;
}

template <typename IT, typename IT2, typename... ITs>
void advance_group(IT& it, IT2& it2, ITs&... its) {
    ++it;
    advance_group(it2, its...);
}


// ----------------------------------------------------------------------------- 
// map
template <typename F, typename RIT, typename IT, typename... ITs>
void map(F&& f, RIT&& rit, IT&& it, IT&& it_end, ITs&&... its) {
    while(it != it_end) {
        *rit = f(*it, *its...);
        advance_group(rit, it, its...);
    }
}


// ----------------------------------------------------------------------------
// fold
template <typename F, 
          typename R,
          typename IT,
          typename... ITs>
std::decay_t<R>
fold(F& f, R&& init, IT&& it, IT&& it_end, ITs&&... its) {
    using M = std::decay_t<R>;
    M mutable_state(std::forward<R>(init));

    while(it != it_end) {
        mutable_state = f(std::move(mutable_state), *it, *its...);
        advance_group(it, its...);
    }

    return mutable_state;
}


// ----------------------------------------------------------------------------- 
// each
template <typename F, typename IT, typename... ITs>
void each(F&& f, IT&& it, IT&& it_end, ITs&&... its) {
    while(it != it_end) {
        f(*it, *its...);
        advance_group(it, its...);
    }
}


// ----------------------------------------------------------------------------
// all
template <typename F, typename IT, typename... ITs>
bool
all(F&& f, IT&& it, IT&& it_end, ITs&&... its) {
    bool ret = true;

    while(it != it_end) {
        if(!f(*it, *its...)) {
            ret = false;
            break;
        }

        advance_group(it, its...);
    }

    return ret;
}


// ----------------------------------------------------------------------------
// some
template <typename F, typename IT, typename... ITs>
bool
some(F&& f, IT&& it, IT&& it_end, ITs&&... its) {
    bool ret = false;

    while(it != it_end) {
        if(f(*it, *its...)) {
            ret = true;
            break;
        }

        advance_group(it, its...);
    }

    return ret;
}

}

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
    return detail::size(c, std::integral_constant<bool, detail::has_size<C>::has>()); 
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
    detail::range_copy_or_move(typename std::is_lvalue_reference<C>::type(), ret.begin(), c.begin(), c.end());
    return ret;
}

//------------------------------------------------------------------------------
// pointers

/**
 * @brief copy the addresses of elements in a container  
 *
 * This a helper mechanism for ensuring all calculations on data are by 
 * reference to a specific set of values. 
 * 
 * This can be useful when operating on large sets of data so that downstream 
 * calculations like `filter()` and `map()` are forced to be efficient even 
 * if user code doesn't properly use references.
 *
 * This algorithm also useful when sorting data without modifying the source 
 * data's container positions.
 *
 * It may be beneficial to apply `pointers()` to the result of `slice()`, to 
 * only operate on the necessary subset of elements.
 *
 * @param c container of elements
 * @return a container of pointers to elements in the argument container
 */
template <typename C>
auto
pointers(C& c) {
    typedef typename std::decay_t<C>::value_type CV;
    sca::vector<CV*> ret(size(c));
    std::transform(c.begin(), c.end(), ret.begin(), [](CV& e){ return &e; });
    return ret;
}

template <typename C>
auto
pointers(const C& c) {
    typedef typename std::decay_t<C>::value_type CV;
    sca::vector<const CV*> ret(size(c));
    std::transform(c.begin(), c.end(), ret.begin(), [](const CV& e){ return &e; });
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
    typedef std::decay_t<C> DC;

public:
    typedef typename DC::iterator iterator;
    typedef typename DC::value_type value_type;
    typedef typename DC::size_type size_type;

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
        m_mem(std::make_shared<DC>(std::move(c))), // keep container in memory
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

private:
    std::shared_ptr<DC> m_mem; // place to hold container memory if constructed with an rvalue 
    const size_t m_size;
    iterator m_begin;
    iterator m_end;
};

/// const variation of slice_of
template <typename C>
class const_slice_of {
    typedef std::decay_t<C> DC;

public:
    typedef typename DC::const_iterator const_iterator;
    typedef typename DC::value_type value_type;
    typedef typename DC::size_type size_type;

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

private:
    const size_t m_size;
    const_iterator m_cbegin;
    const_iterator m_cend;
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
 * @return a slice object capable of iterating a given container
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
 * @return a slice object capable of iterating a given container
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
    using DC = std::decay_t<C>;
    sca::vector<typename DC::value_type> ret(detail::sum(size(c), size(c2), size(cs)...));
    detail::group(
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
    using DC = std::decay_t<C>;
    sca::vector<typename DC::value_type> res(size(container));
    detail::range_copy_or_move(typename std::is_lvalue_reference<C>::type(), res.begin(), container.begin(), container.end());
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
    typedef std::decay_t<C> DC;
    sca::vector<typename DC::value_type> ret(size(container));
    size_t cur = 0;

    for(auto& e : container) {
        if(f(e)) {
            detail::copy_or_move(typename std::is_lvalue_reference<C>::type(), ret[cur], e);
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
    // get the return type of `f`
    using FReturnType = detail::callable_return_type<
        F,
        typename C::value_type, 
        typename Cs::value_type...>;

    // determine the return type of this function
    using Result = sca::vector<FReturnType>;
    Result ret(size(c));
    detail::map(ret.begin(), c.begin(), c.end(), cs.begin()...);
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
    return detail::fold(f, std::forward<Result>(init), c.begin(), c.end(), cs.begin()...);
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
    detail::each(f, c.begin(), c.end(), cs.begin()...);
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
 * @param cs the optional remaining containers whose elements will have f applied to
 * @return `true` if `f` returns `true` for all iterated elements, else `false`
 */
template <typename F, typename C, typename... Cs>
bool 
all(F&& f, C&& c, Cs&&... cs) {
    return detail::all(f, c.begin(), c.end(), cs.begin()...);
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
 * @param cs the optional remaining containers whose elements will have f applied to
 * @return `true` if `f` returns `true` for at least one iterated elements, else `false`
 */
template <typename F, typename C, typename... Cs>
bool 
some(F&& f, C&& c, Cs&&... cs) {
    return detail::some(f, c.begin(), c.end(), cs.begin()...);
}

}

#endif
