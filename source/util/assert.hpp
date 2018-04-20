#pragma once
// Better assert
#include <stdexcept>
#include <string>

#define SS_ASSERT_STRINGIFY_IMPL(X) #X
#define SS_ASSERT_STRINGIFY(X) SS_ASSERT_STRINGIFY_IMPL(X)

#define SS_DESCRIPTIVE_ASSERT(DESCRIPTION, ...) do { \
    auto ok = (__VA_ARGS__); \
    if (!static_cast<bool>(ok)) { \
        std::string message{"assertion failed in function `"}; \
        message += __func__; \
        message += "` in file " __FILE__ ":" SS_ASSERT_STRINGIFY(__LINE__) "\n  "; \
        message += (DESCRIPTION); \
        message += "\n"; \
        throw std::logic_error{std::move(message)}; \
    } \
} while (false)

#define SS_ASSERT(...) SS_DESCRIPTIVE_ASSERT("Assertion: " #__VA_ARGS__, __VA_ARGS__)

// Override system assert
#ifdef assert
    #undef assert
#endif

#define assert(ASSERTION) SS_ASSERT(ASSERTION)
#define assert_with(ASSERTION, DESCRIPTION) SS_DESCRIPTIVE_ASSERT(DESCRIPTION, ASSERTION)

