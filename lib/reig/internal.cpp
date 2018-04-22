#include "internal.h"
#include "context.h"

namespace reig::internal {
    bool is_boxed_in(const Point& pt, const Rectangle& rect) {
        return is_between(pt.x, rect.x, get_x2(rect)) && is_between(pt.y, rect.y, get_y2(rect));
    }

    Color get_yiq_contrast(Color color) {
        using namespace primitive::colors::literals;

        uint32_t y = (299u * color.red.val + 587 * color.green.val + 114 * color.blue.val) / 1000;
        return y >= 128
               ? Color{0_r, 0_g, 0_b, 255_a}
               : Color{255_r, 255_g, 255_b};
    }

    Color lighten_color_by(Color color, uint8_t delta) {
        uint8_t max = 255u;
        color.red.val < max - delta ? color.red.val += delta : color.red.val = max;
        color.green.val < max - delta ? color.green.val += delta : color.green.val = max;
        color.blue.val < max - delta ? color.blue.val += delta : color.blue.val = max;
        return color;
    }

    Rectangle decrease_rect(Rectangle aRect, int by) {
        int moveBy = by / 2;
        aRect.x += moveBy;
        aRect.y += moveBy;
        aRect.width = max(aRect.width - by, 2.0f);
        aRect.height = max(aRect.height - by, 2.0f);
        return aRect;
    }

    void trim_rect_in_other(Rectangle& fitted, const Rectangle& container) {
        fitted.x = max(fitted.x, container.x);
        fitted.y = max(fitted.y, container.y);

        auto fit_size = [](float& fittedSize, const float& fittedCoord,
                           const float& containerSize, const float& containerCoord) {
            auto fittedEnd = fittedCoord + fittedSize;
            auto containerEnd = containerCoord + containerSize;
            auto end = min(fittedEnd, containerEnd);
            auto excess = fittedEnd - end;
            fittedSize = max(fittedSize - excess, 0.0f);
        };

        fit_size(fitted.width, fitted.x, container.width, container.x);
        fit_size(fitted.height, fitted.y, container.height, container.y);
    }

    std::array<Rectangle, 4> get_rect_frame(const Rectangle& rect, float thickness) {
        return {
                Rectangle{rect.x,                          rect.y,                           rect.width, thickness},
                Rectangle{rect.x,                          rect.y + rect.height - thickness, rect.width, thickness},
                Rectangle{rect.x,                          rect.y + thickness,               thickness,  rect.height},
                Rectangle{rect.x + rect.width - thickness, rect.y,                           thickness,  rect.height}
        };
    }
}
