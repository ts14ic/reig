#include "reig.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
//#include <iostream>
#include <cstdio>
#include <iterator>

using std::vector;

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

void reig::Context::set_render_handler(CallbackType renderHandler) {
    _renderHandler = renderHandler;
}

void reig::Context::set_user_ptr(void* ptr) {
    _userPtr = ptr;
}

void const* reig::Context::get_user_ptr() const {
    return _userPtr;
}

namespace {
    
}

void reig::FontData::auto_free() {
    _free = true;
}

reig::FontData::~FontData() {
    if(_free && bitmap) delete[] bitmap;
}

reig::FontData reig::Context::set_font(char const* aPath, uint_t aTextureId, float aSize) {
    auto file = std::fopen("/usr/share/fonts/TTF/monof55.ttf", "rb");
    std::fseek(file, 0, SEEK_END);
    auto size = ftell(file);
    std::rewind(file);
    auto fileContents = new unsigned char[size];
    std::fread(fileContents, 1, size, file);
    std::fclose(file);
    
    int const charsNum = 96;
    
    auto bitmap = new unsigned char[256 * 256];
    auto bakedChars = new stbtt_bakedchar[charsNum];
    stbtt_BakeFontBitmap(fileContents, 0, 20.f, bitmap, 256, 256, ' ', charsNum, bakedChars);
    
    FontData ret;
    ret.width = 256;
    ret.height = 256;
    ret.bitmap = new ubyte_t[ret.width * ret.height];
    
    if(_font.bakedChars) delete[] _font.bakedChars;
    _font.bakedChars = new stbtt_bakedchar[charsNum];
    _font.texture = aTextureId;
    _font.width = ret.width;
    _font.height = ret.height;
    _font.size = aSize;
    
    stbtt_BakeFontBitmap(
        fileContents, 0, aSize, 
        ret.bitmap, ret.width, ret.height, 
        ' ', 96, 
        _font.bakedChars
    );
    
    // DEBUG-PRINT
//    for(auto j = 0u; j < _font.height; ++j) {
//        for(auto i = 0u; i < _font.width; ++i) {
//            std::cout << " .:-=+*#"[ret.bitmap[j * _font.width + i] >> 5];
//        }   std::cout << std::endl;
//    }
    // DEBUG-END
    
    return ret;
}

void reig::Context::render_all() const {
    if(!_renderHandler) return;
    _renderHandler(_drawData, _userPtr);
}

void reig::Context::start_new_frame() {
    _drawData.clear();
    
    mouse.left._clicked = false;
    mouse._scroll = 0.f;
}

void reig::Mouse::move(float_t difx, float_t dify) {
    _cursorPos.x += difx;
    _cursorPos.y += dify;
}

void reig::Mouse::place(float_t x, float_t y) {
    _cursorPos.x = x;
    _cursorPos.y = y;
}

void reig::Mouse::scroll(float dy) {
    _scroll = dy;
}

void reig::MouseButton::press(float_t x, float_t y) {
    if(!_pressed) {
        _pressed = true;
        _clicked = true;
        _clickedPos = {x, y};
    }
}

void reig::MouseButton::release() {
    _pressed = false;
}

void reig::Figure::form(vector<Vertex>& vertices, vector<uint_t>& indices, uint_t id) {
    vertices.swap(_vertices);
    indices.swap(_indices);
    _texture = id;
}

vector<reig::Vertex> const&
reig::Figure::vertices() const {
    return _vertices;
}

vector<reig::uint_t> const&
reig::Figure::indices() const {
    return _indices;
}

reig::uint_t
reig::Figure::texture() const {
    return _texture;
}

bool reig::Context::button(Rectangle aBox, Color aColor) {
    // Render button outline first
    Color outlineCol = detail::get_yiq_contrast(aColor);
    render_rectangle(aBox, outlineCol);
    
    // if cursor is over the button, highlight it
    if(detail::in_box(mouse._cursorPos, aBox)) {
        aColor = detail::lighten_color_by(aColor, 50);
    }
    aBox = detail::decrease(aBox, 4);
    
    bool clickedButton = detail::in_box(mouse.left._clickedPos, aBox);
    
    if(mouse.left._pressed && clickedButton) {
        aColor = detail::lighten_color_by(aColor, 50);
    }
    render_rectangle(aBox, aColor);
    
    return mouse.left._clicked && clickedButton;
}

bool reig::Context::button(char const* aTitle, Rectangle aBox, Color aColor) {
    bool ret = button(aBox, aColor);
    
    render_text(aTitle, aBox);
    
    return ret;
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
    if(detail::in_box(mouse._cursorPos, cursorBox)) {
        cursorColor = detail::lighten_color_by(cursorColor, 50);
    }
    render_rectangle(cursorBox, cursorColor);
    
    if(mouse.left._pressed && detail::in_box(mouse.left._clickedPos, aBox)) {
        auto halfCursorW = cursorBox.w / 2;
        auto distance = mouse._cursorPos.x - cursorBox.x - halfCursorW;
        
        if(detail::abs(distance) > halfCursorW) {
            value += static_cast<int_t>(distance / cursorBox.w) * aStep;
            value = detail::clamp(value, min, max);
        }
    }
    else if(mouse._scroll != 0 && detail::in_box(mouse._cursorPos, aBox)) {
        value += static_cast<int_t>(mouse._scroll) * aStep;
        value = detail::clamp(value, min, max);
    }
    
    if(aValue != value) {
        aValue = value;
        return true;
    }
    else {
        return false;
    }
}

bool reig::Context::checkbox(Rectangle aBox, Color aColor, bool& aValue) {
    // Render checkbox's base
    Color contrastColor = detail::get_yiq_contrast(aColor);
    render_rectangle(aBox, contrastColor);
    aBox = detail::decrease(aBox, 4);
    render_rectangle(aBox, aColor);
    
    // Render check
    if(aValue) {
        aBox = detail::decrease(aBox, 4);
        render_rectangle(aBox, contrastColor);
    }
    
    // True if state changed
    if(mouse.left._clicked && detail::in_box(mouse.left._clickedPos, aBox)) {
        aValue = !aValue;
        return true;
    }
    else {
        return false;
    }
}

void reig::Context::render_triangle(Triangle const& aTri, Color const& aColor) {
    vector<Vertex> vertices (3);
    vertices[0].position = {aTri.pos0};
    vertices[1].position = {aTri.pos1};
    vertices[2].position = {aTri.pos2};
    
    for(auto& vert : vertices) {
        vert.color = aColor;
    }
    
    vector<uint_t> indices = {0, 1, 2};
    
    Figure fig;
    fig.form(vertices, indices);
    _drawData.push_back(fig);
}

void reig::Context::render_text(char const* ch, Rectangle aBox) {
    if(!ch) return;
    
    aBox = detail::decrease(aBox, 8);
    float x = aBox.x;
    float y = aBox.y + aBox.h - _font.size / 2;
    
    for(; *ch; ++ch) {
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(
            _font.bakedChars,
            _font.width, _font.height, *ch - ' ', &x, &y, &q, 1
        );
        if(x > aBox.x + aBox.w) break;
        
        Color transparent {0, 0, 0, 0};
        
        vector<Vertex> vertices {
            {{q.x0, q.y0}, {q.s0, q.t0}, transparent},
            {{q.x1, q.y0}, {q.s1, q.t0}, transparent},
            {{q.x1, q.y1}, {q.s1, q.t1}, transparent},
            {{q.x0, q.y1}, {q.s0, q.t1}, transparent}
        };
        
        vector<uint_t> indices {0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices, _font.texture);
        _drawData.push_back(fig);
    }
}

void reig::Context::render_rectangle(Rectangle const& aRect, Color const& aColor) {
    vector<Vertex> vertices (4);
    vertices[0].position = Point{aRect.x          , aRect.y          };
    vertices[1].position = Point{aRect.x + aRect.w, aRect.y          };
    vertices[2].position = Point{aRect.x + aRect.w, aRect.y + aRect.h};
    vertices[3].position = Point{aRect.x          , aRect.y + aRect.h};
    
    for(auto& vert : vertices) {
        vert.color = aColor;
    }
    
    vector<uint_t> indices {0, 1, 2, 2, 3, 0};

    Figure fig;
    fig.form(vertices, indices);
    _drawData.push_back(fig);
}
