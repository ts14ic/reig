#include "internal.h"

bool reig::internal::is_boxed_in(Point const& pt, Rectangle const& box) {
    return is_between(pt.x, box.x, box.x + box.width) && is_between(pt.y, box.y, box.y + box.height);
}

Color reig::internal::get_yiq_contrast(Color color) {
    using namespace primitive::colors::literals;

    uint32_t y = (299u * color.red.val + 587 * color.green.val + 114 * color.blue.val) / 1000;
    return y >= 128
           ? Color{0_r, 0_g, 0_b, 255_a}
           : Color{255_r, 255_g, 255_b};
}

Color reig::internal::lighten_color_by(Color color, uint8_t delta) {
    uint8_t max = 255u;
    color.red.val < max - delta ? color.red.val += delta : color.red.val = max;
    color.green.val < max - delta ? color.green.val += delta : color.green.val = max;
    color.blue.val < max - delta ? color.blue.val += delta : color.blue.val = max;
    return color;
}

Rectangle reig::internal::decrease_box(Rectangle aRect, int by) {
    int moveBy = by / 2;
    aRect.x += moveBy;
    aRect.y += moveBy;
    aRect.width -= by;
    aRect.height -= by;
    return aRect;
}
