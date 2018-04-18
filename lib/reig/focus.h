#ifndef REIG_FOCUS_H
#define REIG_FOCUS_H

#include "primitive.h"
#include <functional>

namespace reig {
    enum class Focus {
        NONE,
        HOVER,
        CLICK,
        SELECT,
        HOLD,
    };

    using FocusAreaCallback_t = std::function<void(const Focus&)>;

    struct FocusCallback {
        FocusCallback(const primitive::Rectangle& zone, FocusAreaCallback_t callback);

        primitive::Rectangle rect;
        FocusAreaCallback_t callback;
    };
}

#endif //REIG_FOCUS_H
