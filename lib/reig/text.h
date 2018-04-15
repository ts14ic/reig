#ifndef REIG_TEXT_H
#define REIG_TEXT_H

namespace reig::text {
    enum class Alignment : unsigned {
        CENTER_HORIZONTAL = 0x1u,
        LEFT = 0x2u,
        RIGHT = 0x4u,
        CENTER_VERTICAL = 0x10,
        TOP = 0x20,
        BOTTOM = 0x40,
        CENTER = CENTER_HORIZONTAL | CENTER_VERTICAL,
        TOP_LEFT = TOP | LEFT,
        TOP_RIGHT = TOP | RIGHT,
        BOTTOM_LEFT = BOTTOM | LEFT,
        BOTTOM_RIGHT = BOTTOM | RIGHT
    };
}

#endif //REIG_TEXT_H
