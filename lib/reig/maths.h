#ifndef REIG_MATH_H
#define REIG_MATH_H

#include <type_traits>
#include <sstream>

namespace reig::math {
    template <typename T>
    bool is_between(T val, T min, T max) {
        return val > min && val < max;
    }

    template <typename T>
    T clamp(T val, T min, T max) {
        return val < min ? min :
               val > max ? max :
               val;
    }

    template <typename T>
    T min(T a, T b) {
        return a < b ? a : b;
    }

    template <typename T>
    T max(T a, T b) {
        return a > b ? a : b;
    }

    template <typename T>
    T abs(T a) {
        return a < 0 ? -a : a;
    }

    template <typename T>
    int sign(T a) {
        return a < 0 ? -1 :
               a > 0 ? 1 :
               0;
    }

    template <typename R, typename T, typename = std::enable_if_t<std::is_integral_v<R> && std::is_integral_v<R>>>
    R integral_cast(T t) {
        auto r = static_cast<R>(t);
        if (r != t || (std::is_signed_v<T> != std::is_signed_v<R> && ((t < T{}) != r < R{}))) {
            // TODO: make a custom exception for this to hide the sstream dependency
            std::stringstream ss;
            ss << "Bad integral cast from " << typeid(T).name() << "(" << t << ") to type " << typeid(R).name();
            throw std::range_error(ss.str());
        }
        return r;
    };
}

#endif //REIG_MATH_H
