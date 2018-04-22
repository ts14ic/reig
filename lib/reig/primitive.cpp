#include "primitive.h"
#include "maths.h"

using std::vector;

namespace reig::primitive {
    bool is_point_in_rect(const Point& pt, const Rectangle& rect) {
        return math::is_between(pt.x, rect.x, get_x2(rect)) && math::is_between(pt.y, rect.y, get_y2(rect));
    }

    Rectangle decrease_rect(Rectangle aRect, int by) {
        int moveBy = by / 2;
        aRect.x += moveBy;
        aRect.y += moveBy;
        aRect.width = math::max(aRect.width - by, 2.0f);
        aRect.height = math::max(aRect.height - by, 2.0f);
        return aRect;
    }

    void trim_rect_in_other(Rectangle& fitted, const Rectangle& container) {
        fitted.x = math::max(fitted.x, container.x);
        fitted.y = math::max(fitted.y, container.y);

        auto fit_size = [](float& fittedSize, const float& fittedCoord,
                           const float& containerSize, const float& containerCoord) {
            auto fittedEnd = fittedCoord + fittedSize;
            auto containerEnd = containerCoord + containerSize;
            auto end = math::min(fittedEnd, containerEnd);
            auto excess = fittedEnd - end;
            fittedSize = math::max(fittedSize - excess, 0.0f);
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

    uint32_t colors::to_uint(const Color& color) {
        return (color.alpha << 24u)
               + (color.blue << 16u)
               + (color.green << 8u)
               + color.red;
    }

    Color colors::from_uint(uint32_t rgba) {
        return Color{
                static_cast<uint8_t>((rgba >> 24u) & 0xFFu),
                static_cast<uint8_t>((rgba >> 16u) & 0xFFu),
                static_cast<uint8_t>((rgba >> 8u) & 0xFFu),
                static_cast<uint8_t>(rgba & 0xFFu)
        };
    }

    Color colors::get_yiq_contrast(Color color) {
        using namespace colors::literals;

        uint32_t y = (299u * color.red + 587 * color.green + 114 * color.blue) / 1000;
        return y >= 128
               ? Color{0_r, 0_g, 0_b, 255_a}
               : Color{255_r, 255_g, 255_b};
    }

    Color colors::lighten_color_by(Color color, uint8_t delta) {
        uint8_t max = 255u;
        color.red < max - delta ? color.red += delta : color.red = max;
        color.green < max - delta ? color.green += delta : color.green = max;
        color.blue < max - delta ? color.blue += delta : color.blue = max;
        return color;
    }

    Color colors::dim_color_by(Color color, uint8_t delta) {
        uint8_t min = 0u;
        color.red > min + delta ? color.red -= delta : color.red = min;
        color.green > min + delta ? color.green -= delta : color.green = min;
        color.blue > min + delta ? color.blue -= delta : color.blue = min;
        return color;
    }

    void Figure::form(vector<Vertex>& vertices, vector<int>& indices, int id) {
        vertices.swap(mVertices);
        indices.swap(mIndices);
        mTextureId = id;
    }

    const vector<Vertex>& Figure::vertices() const {
        return mVertices;
    }

    const vector<int>& Figure::indices() const {
        return mIndices;
    }

    int Figure::texture() const {
        return mTextureId;
    }
}
