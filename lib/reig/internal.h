#ifndef REIG_INTERNAL_H
#define REIG_INTERNAL_H

#include "primitive.h"
#include <sstream>

using namespace reig::primitive;

namespace reig::internal {
    template <typename T>
    bool is_between(T val, T min, T max) {
        return val > min && val < max;
    }

    bool is_boxed_in(const Point& pt, const Rectangle& rect);

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

    primitive::Color get_yiq_contrast(primitive::Color color);

    Color lighten_color_by(Color color, uint8_t delta);

    Rectangle decrease_rect(Rectangle aRect, int by);

    template <typename R, typename T, typename = std::enable_if_t<std::is_integral_v<R> && std::is_integral_v<R>>>
    R integral_cast(T t) {
        auto r = static_cast<R>(t);
        if (r != t || (std::is_signed_v<T> != std::is_signed_v<R> && ((t < T{}) != r < R{}))) {
            std::stringstream ss;
            ss << "Bad integral cast from " << typeid(T).name() << "(" << t << ") to type " << typeid(R).name();
            throw std::range_error(ss.str());
        }
        return r;
    };

    void trim_rect_in_other(Rectangle& fitted, const Rectangle& container);
}

#endif //REIG_INTERNAL_H
