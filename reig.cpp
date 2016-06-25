#include "reig.h"

namespace reig::detail {
    template <typename T>
    bool between(T val, T min, T max) {
        return val > min && val < max;
    }
    
    bool in_box(Point const& pt, Rectangle const& box) {
        return between(pt.x, box.x, box.x + box.w) && between(pt.y, box.y, box.y + box.h);
    }
    
    template <typename T>
    T clamp(T val, T min, T max) {
        if(val < min) val = min;
        else if(val > max) val = max;
        return val;
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
        if(a < 0) return -a;
        return a;
    }
    
    template <typename T>
    int_t sign(T a) {
        if(a > 0) return  1;
        if(a < 0) return -1;
        return 0;
    }
    
    Color get_yiq_contrast(Color color) {
        uint_t y = (299 * color.r + 587 * color.g + 114 * color.b) / 1000;
        return y >= 128 ? Color{0u, 0u, 0u} : Color{255u, 255u, 255u};
    }
    
    Color lighten_color_by(Color color, ubyte_t delta) {
        ubyte_t max = 255u;
        color.r < max - delta ? color.r += delta : color.r = max;
        color.g < max - delta ? color.g += delta : color.g = max;
        color.b < max - delta ? color.b += delta : color.b = max;
        return color;
    }
    
    Rectangle decrease(Rectangle aRect, int_t by) {
        int_t moveBy = by / 2;
        aRect.x += moveBy;
        aRect.y += moveBy;
        aRect.w -= by;
        aRect.h -= by;
        return aRect;
    }
}

void reig::Context::set_render_handler(void (*renderHandler)(DrawData const& drawData)) {
    _renderHandler = renderHandler;
}

void reig::Context::render_all() const {
    if(!_renderHandler) return;
    _renderHandler(_drawData);
}

void reig::Context::start_new_frame() {
    _mouse.left.clicked = false;
    _drawData.clear();
}

void reig::Context::move_mouse(float_t difx, float_t dify) {
    _mouse.cursorPos.x += difx;
    _mouse.cursorPos.y += dify;
}

void reig::Context::place_mouse(float_t x, float_t y) {
    _mouse.cursorPos.x = x;
    _mouse.cursorPos.y = y;
}

void reig::Context::mouse_press_left(float_t x, float_t y) {
    if(!_mouse.left.pressed) {
        _mouse.left.pressed = true;
        _mouse.left.clicked = true;
        _mouse.left.clickedPos = {x, y};
    }
}

void reig::Context::mouse_release_left() {
    _mouse.left.pressed = false;
}

void reig::Context::Figure::form(std::vector<Vertex>& vertices, std::vector<uint_t>& indices) {
    vertices.swap(_vertices);
    indices.swap(_indices);
}

std::vector<reig::Vertex> const& reig::Context::Figure::vertices() const {
    return _vertices;
}

std::vector<reig::uint_t> const& reig::Context::Figure::indices() const {
    return _indices;
}

bool reig::Context::button(Rectangle aRect, Color aColor) {
    // Render button outline first
    Color outlineCol = detail::get_yiq_contrast(aColor);
    render_rectangle(aRect, outlineCol);
    
    // if cursor is over the button, highlight it
    if(detail::in_box(_mouse.cursorPos, aRect)) {
        aColor = detail::lighten_color_by(aColor, 50);
    }
    aRect = detail::decrease(aRect, 4);
    render_rectangle(aRect, aColor);
    
    return _mouse.left.clicked && detail::in_box(_mouse.left.clickedPos, aRect);
}

bool reig::Context::slider(
    Rectangle aBox, Color aColor, 
    float_t& aValue, float_t aMin, float_t aMax, float_t aStep
) {
    // Render slider's base
    Color cursorColor = detail::get_yiq_contrast(aColor);
    render_rectangle(aBox, cursorColor);
    aBox = detail::decrease(aBox, 4);
    render_rectangle(aBox, aColor);
    
    // Prepare the values
    float_t min = detail::min(aMin, aMax);
    float_t max = detail::max(aMin, aMax);
    float_t value = detail::clamp(aValue, min, max);
    int_t offset = static_cast<int_t>((value - min) / aStep);
    int_t valuesNum = (max - min) / aStep + 1;
    
    // Render the cursor
    auto cursorBox = detail::decrease(aBox, 4);
    cursorBox.w /= valuesNum;
    cursorBox.x += offset * cursorBox.w;
    if(detail::in_box(_mouse.cursorPos, cursorBox)) {
        cursorColor = detail::lighten_color_by(cursorColor, 50);
    }
    render_rectangle(cursorBox, cursorColor);
    
    if(_mouse.left.pressed && detail::in_box(_mouse.left.clickedPos, aBox)) {
        auto halfCursorW = cursorBox.w / 2;
        auto distance = _mouse.cursorPos.x - cursorBox.x - halfCursorW;
        
        if(detail::abs(distance) > halfCursorW) {
            value += static_cast<int_t>(distance / cursorBox.w) * aStep;
            value = detail::clamp(value, min, max);
        }
    }
    
    if(aValue != value) {
        aValue = value;
        return true;
    }
    else {
        return false;
    }
}

void reig::Context::render_triangle(Triangle const& aTri, Color const& aColor) {
    std::vector<Vertex> vertices (3);
    vertices[0].position = {aTri.pos0};
    vertices[1].position = {aTri.pos1};
    vertices[2].position = {aTri.pos2};
    
    for(auto& vert : vertices) {
        vert.color = aColor;
    }
    
    std::vector<uint_t> indices = {0, 1, 2};
    
    Figure fig;
    fig.form(vertices, indices);
    _drawData.push_back(fig);
}

void reig::Context::render_rectangle(Rectangle const& aRect, Color const& aColor) {
    std::vector<Vertex> vertices (4);
    vertices[0].position = Point{aRect.x          , aRect.y          };
    vertices[1].position = Point{aRect.x + aRect.w, aRect.y          };
    vertices[2].position = Point{aRect.x + aRect.w, aRect.y + aRect.h};
    vertices[3].position = Point{aRect.x          , aRect.y + aRect.h};
    
    for(auto& vert : vertices) {
        vert.color = aColor;
    }
    
    std::vector<uint_t> indices {0, 1, 2, 0, 2, 3};
    
    Figure fig;
    fig.form(vertices, indices);
    _drawData.push_back(fig);
}
