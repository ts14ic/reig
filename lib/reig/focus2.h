#ifndef REIG_FOCUS2_H
#define REIG_FOCUS2_H

#include "primitive.h"
#include <functional>

namespace reig {
    enum class Focus2 {
        NONE,
        HOVER,
        CLICK,
        SELECT,
        HOLD,
    };

    using FocusAreaCallback_t = std::function<void(const Focus2&)>;

    struct FocusCallback {
        FocusCallback(const primitive::Rectangle& zone, FocusAreaCallback_t callback)
                : rect(zone), callback(move(callback)) {}

        primitive::Rectangle rect;
        FocusAreaCallback_t callback;
    };
}

#endif //REIG_FOCUS2_H
