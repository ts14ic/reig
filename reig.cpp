#include "reig.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"
#include <cstdio>
#include <cassert>

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

reig::Color reig::transparent {};
reig::Color reig::red         {239,  41,  41};
reig::Color reig::orange      {252, 175,  62};
reig::Color reig::yellow      {252, 233,  79};
reig::Color reig::green       {138, 226,  52};
reig::Color reig::blue        {114, 159, 207};
reig::Color reig::violet      {173, 127, 168};
reig::Color reig::brown       {143,  89,   2};
reig::Color reig::white       {255, 255, 255};
reig::Color reig::lightGrey   {186, 189, 182};
reig::Color reig::mediumGrey  {136, 138, 133};
reig::Color reig::darkGrey    { 46,  52,  54};
reig::Color reig::black       {  0,   0,   0};

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
    auto file = std::fopen(aPath, "rb");
    std::fseek(file, 0, SEEK_END);
    auto size = ftell(file);
    std::rewind(file);
    auto fileContents = new unsigned char[size];
    std::fread(fileContents, 1, size, file);
    std::fclose(file);
    
    int const charsNum = 96;
    
    auto bakedChars = new stbtt_bakedchar[charsNum];
    
    auto bitmap = new ubyte_t[256 * 256];
    auto height = stbtt_BakeFontBitmap(
        fileContents, 0, aSize, bitmap, 256, 256, ' ', charsNum, bakedChars
    );
    assert(height <= 256);
    delete[] fileContents;
    
    if(_font.bakedChars) delete[] _font.bakedChars;
    _font.bakedChars = bakedChars;
    _font.texture = aTextureId;
    _font.width = 256;
    _font.height = height;
    _font.size = aSize;
    
    // DEBUG-PRINT
//    for(auto j = 0u; j < _font.height; ++j) {
//        for(auto i = 0u; i < _font.width; ++i) {
//            std::cout << " .:-=+*#"[ret.bitmap[j * _font.width + i] >> 5];
//        }   std::cout << std::endl;
//    }
    // DEBUG-END
    
    FontData ret;
    ret.bitmap = bitmap;
    ret.width = 256;
    ret.height = height;
    
    return ret;
}

void reig::Context::render_all() {
    if(!_renderHandler) return;
    if(_window.started) end_window();
    
    if(!_window.drawData.empty()) {
        _renderHandler(_window.drawData, _userPtr);
    }
    if(!_drawData.empty()) {
        _renderHandler(_drawData, _userPtr);
    }
}

void reig::Context::start_new_frame() {
    _window.drawData.clear();
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

void reig::Context::start_window(char const* aTitle, float& aX, float& aY) {
    if(_window.started) end_window();
    
    _window.started = true;
    _window.title   = aTitle;
    _window.x = &aX;
    _window.y = &aY;
    _window.w = 0;
    _window.h = 0;
    _window.headerSize = 8 + _font.size;
}

void reig::Context::end_window() {
    if(!_window.started) return;
    _window.started = false;
    
    _window.w += 4;
    _window.h += 4;
    
    vector<Vertex> headerVertices {
        {{*_window.x,             *_window.y                     }, {}, darkGrey},
        {{*_window.x + _window.w, *_window.y                     }, {}, darkGrey},
        {{*_window.x + _window.w, *_window.y + _window.headerSize}, {}, darkGrey},
        {{*_window.x,             *_window.y + _window.headerSize}, {}, darkGrey}
    };
    vector<uint_t> headerIndices {0, 1, 2, 2, 3, 0};
    Figure header;
    header.form(headerVertices, headerIndices);
    
    vector<Vertex> bodyVertices {
        {{*_window.x,             *_window.y + _window.headerSize}, {}, mediumGrey},
        {{*_window.x + _window.w, *_window.y + _window.headerSize}, {}, mediumGrey},
        {{*_window.x + _window.w, *_window.y + _window.h         }, {}, mediumGrey},
        {{*_window.x,             *_window.y + _window.h         }, {}, mediumGrey},
        
    };
    vector<uint_t> bodyIndices {0, 1, 2, 2, 3, 0};
    Figure body;
    body.form(bodyVertices, bodyIndices);
    
    Triangle arrowDown {
        *_window.x + 3.f,                            *_window.y + 3.f,
        *_window.x + 3.f + _window.headerSize,       *_window.y + 3.f,
        *_window.x + 3.f + _window.headerSize / 2.f, *_window.y + _window.headerSize - 3.f
    };
    
    _window.drawData.push_back(header);
    render_text(_window.title, {
        *_window.x + _window.headerSize, *_window.y, _window.w - _window.headerSize, _window.headerSize
    });
    render_triangle(arrowDown, lightGrey);
    _window.drawData.push_back(body);
    
    Rectangle headerBox {*_window.x, *_window.y, _window.w, _window.headerSize};
    if(mouse.left._pressed && detail::in_box(mouse.left._clickedPos, headerBox)) {
        Point moved  {
            mouse._cursorPos.x - mouse.left._clickedPos.x,
            mouse._cursorPos.y - mouse.left._clickedPos.y
        };
        
        *_window.x += moved.x;
        *_window.y += moved.y;
        mouse.left._clickedPos.x += moved.x;
        mouse.left._clickedPos.y += moved.y;
    }
}

void reig::Context::Window::expand(Rectangle& aBox) {
    if(started) {
        aBox.x += *x + 4;
        aBox.y += *y + headerSize + 4;
        
        if(*x + w < aBox.x + aBox.w) {
            w = aBox.x + aBox.w - *x;
        }
        if(*y + h < aBox.y + aBox.h) {
            h = aBox.y + aBox.h - *y;
        }
    }
}

void reig::Context::label(char const* ch, Rectangle aBox) {
    _window.expand(aBox);
    render_text(ch, aBox);
}

bool reig::Context::button(char const* aTitle, Rectangle aBox, Color aColor) {
    _window.expand(aBox);
    
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
    render_text(aTitle, aBox);
    
    return mouse.left._clicked && clickedButton;
}

bool reig::Context::button(char const* aTitle, Rectangle aBox, int aBaseTexture, int aHoverTexture) {
    _window.expand(aBox);
    
    bool clickedButton = detail::in_box(mouse.left._clickedPos, aBox);
    bool hoveredButton = detail::in_box(mouse._cursorPos, aBox);
    
    if((mouse.left._pressed && clickedButton) || hoveredButton) {
        render_rectangle(aBox, aBaseTexture);
    }
    else {
        render_rectangle(aBox, aHoverTexture);
    }
    
    aBox = detail::decrease(aBox, 8);
    
    render_text(aTitle, aBox);
    
    return mouse.left._clicked && clickedButton;
}

bool reig::Context::slider(
    Rectangle aBox, Color aColor, 
    float_t& aValue, float_t aMin, float_t aMax, float_t aStep
) {
    _window.expand(aBox);
    
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

bool reig::Context::slider(
    Rectangle aBox, int aBaseTexture, int aCursorTexture, 
    float& aValue, float_t aMin, float_t aMax, float_t aStep
) {
    _window.expand(aBox);
    
    // Render slider's base
    render_rectangle(aBox, aBaseTexture);
    
    // Prepare the values
    float_t min = detail::min(aMin, aMax);
    float_t max = detail::max(aMin, aMax);
    float_t value = detail::clamp(aValue, min, max);
    int_t offset = static_cast<int_t>((value - min) / aStep);
    int_t valuesNum = (max - min) / aStep + 1;
    
    // Render the cursor
    auto cursorBox = detail::decrease(aBox, 8);
    cursorBox.w /= valuesNum;
    cursorBox.x += offset * cursorBox.w;
    render_rectangle(cursorBox, aCursorTexture);
    
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
    _window.expand(aBox);
    
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

bool reig::Context::checkbox(Rectangle aBox, int aBaseTexture, int aTickTexture, bool& aValue) {
    _window.expand(aBox);
    
    // Render checkbox's base
    render_rectangle(aBox, aBaseTexture);
    
    // Render check
    if(aValue) {
        aBox = detail::decrease(aBox, 8);
        render_rectangle(aBox, aTickTexture);
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

void reig::Context::render_text(char const* ch, Rectangle aBox) {
    if(_font.bakedChars == nullptr) return;
    if(ch == nullptr) return;
    
    aBox = detail::decrease(aBox, 8);
    float x = aBox.x;
    float y = aBox.y + aBox.h / 2 + _font.size / 2;
    
    for(; *ch; ++ch) {
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(
            _font.bakedChars,
            _font.width, _font.height, *ch - ' ', &x, &y, &q, 1
        );
        if(q.x0 > aBox.x + aBox.w) {
            break;
        }
        if(q.x1 > aBox.x + aBox.w) {
            q.x1 = aBox.x + aBox.w;
        }
        
        vector<Vertex> vertices {
            {{q.x0, q.y0}, {q.s0, q.t0}, {}},
            {{q.x1, q.y0}, {q.s1, q.t0}, {}},
            {{q.x1, q.y1}, {q.s1, q.t1}, {}},
            {{q.x0, q.y1}, {q.s0, q.t1}, {}}
        };
        
        vector<uint_t> indices {0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices, _font.texture);
        _drawData.push_back(fig);
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

void reig::Context::render_rectangle(Rectangle const& aBox, int aTexture) {
    vector<Vertex> vertices {
        {{aBox.x,          aBox.y         }, {0.f, 0.f}, {}},
        {{aBox.x + aBox.w, aBox.y         }, {1.f, 0.f}, {}},
        {{aBox.x + aBox.w, aBox.y + aBox.h}, {1.f, 1.f}, {}},
        {{aBox.x,          aBox.y + aBox.h}, {0.f, 1.f}, {}}
    };
    vector<uint_t> indices {0, 1, 2, 2, 3, 0};
    
    Figure fig;
    fig.form(vertices, indices, aTexture);
    _drawData.push_back(fig);
}

void reig::Context::render_rectangle(Rectangle const& aRect, Color const& aColor) {
    vector<Vertex> vertices {
        {{aRect.x,           aRect.y          }, {}, aColor},
        {{aRect.x + aRect.w, aRect.y          }, {}, aColor},
        {{aRect.x + aRect.w, aRect.y + aRect.h}, {}, aColor},
        {{aRect.x,           aRect.y + aRect.h}, {}, aColor}
    };
    vector<uint_t> indices {0, 1, 2, 2, 3, 0};

    Figure fig;
    fig.form(vertices, indices);
    _drawData.push_back(fig);
}
