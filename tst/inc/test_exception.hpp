#ifndef CPP_TEMPLATE_WORKSHOP_TEST_EXCEPTION
#define CPP_TEMPLATE_WORKSHOP_TEST_EXCEPTION

// cpp stl
#include <exception>

// other libs
#include <gtest/gtest.h> 

namespace ctwt { // cpp template workshop test

struct todo_exception : public std::exception {
    inline virtual char const* what() const throw() {
        return "some code is unimplemented"
    }
};

template <typename F, typename... As>
void run_test(F&& f, As&&...) {
    try {
    } catch(const todo_exception& e) {
        std::cout << e.what() << std::endl;;
        ASSERT_TRUE(false);
    }
}

}

#endif
