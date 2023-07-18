#ifndef SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_ALGORITHM
#define SIMPLE_CPLUSPLUS_ALGORITHM_DETAIL_ALGORITHM

// cpp stl
#include <type_traits>
#include <functional>
#include <tuple>
#include <any>

// local 
#include "template.hpp"

/**
 * This where code goes which should *not* be directly included by a user, but
 * is needed by other code included by a user.
 */

namespace sca { // simple cpp algorithm
namespace detail { 
namespace algorithm {

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
inline size_t 
size(C& c, std::true_type) { 
    return c.size(); 
}

// Get the size of an object the *slow* way by iterating through it
template <typename C>
inline size_t 
size(C& c, std::false_type) {
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
    detail::algorithm::range_copy_or_move(typename std::is_lvalue_reference<C>::type(), cur, c.begin(), c.end());
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
R
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
inline bool
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
}
}

#endif
