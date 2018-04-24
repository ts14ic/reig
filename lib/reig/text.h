#ifndef REIG_TEXT_H
#define REIG_TEXT_H

namespace reig::text {
    enum class Alignment : unsigned {
        kCenterHorizontal = 0x1u,
        kLeft = 0x2u,
        kRight = 0x4u,
        kCenterVertical = 0x10,
        kTop = 0x20,
        kBottom = 0x40,
        kCenter = kCenterHorizontal | kCenterVertical,
        kTopLeft = kTop | kLeft,
        kTopRight = kTop | kRight,
        kBottomLeft = kBottom | kLeft,
        kBottomRight = kBottom | kRight
    };
}

#endif //REIG_TEXT_H
