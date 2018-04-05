#include "reig.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#include "stb_truetype.h"
#include <cstdio>

using std::vector;

namespace reig::detail {
    template<typename T>
    auto between(T val, T min, T max) -> bool {
        return val > min && val < max;
    }

    auto in_box(Point const& pt, Rectangle const& box) -> bool {
        return between(pt.x, box.x, box.x + box.width) && between(pt.y, box.y, box.y + box.height);
    }

    template<typename T>
    auto clamp(T val, T min, T max) -> T {
        if (val < min) val = min;
        else if (val > max) val = max;
        return val;
    }

    template<typename T>
    auto min(T a, T b) -> T {
        return a < b ? a : b;
    }

    template<typename T>
    auto max(T a, T b) -> T {
        return a > b ? a : b;
    }

    template<typename T>
    auto abs(T a) -> T {
        if (a < 0) return -a;
        return a;
    }

    template<typename T>
    auto sign(T a) -> int_t {
        if (a > 0) return 1;
        if (a < 0) return -1;
        return 0;
    }

    auto get_yiq_contrast(Color color) -> Color {
        using namespace Colors::literals;

        uint_t y = (299u * color.red.val + 587 * color.green.val + 114 * color.blue.val) / 1000;
        return y >= 128 ? Color{0_r, 0_g, 0_b} : Color{255_r, 255_g, 255_b};
    }

    auto lighten_color_by(Color color, ubyte_t delta) -> Color {
        ubyte_t max = 255u;
        color.red.val < max - delta ? color.red.val += delta : color.red.val = max;
        color.green.val < max - delta ? color.green.val += delta : color.green.val = max;
        color.blue.val < max - delta ? color.blue.val += delta : color.blue.val = max;
        return color;
    }

    auto decrease(Rectangle aRect, int_t by) -> Rectangle {
        int_t moveBy = by / 2;
        aRect.x += moveBy;
        aRect.y += moveBy;
        aRect.width -= by;
        aRect.height -= by;
        return aRect;
    }
}

auto reig::Colors::to_uint(Color const& color) -> uint_t {
    return (color.alpha.val << 24)
           + (color.blue.val << 16)
           + (color.green.val << 8)
           + color.red.val;
}

auto reig::Colors::from_uint(uint_t rgba) -> Color {
    return Color {
            Red{static_cast<ubyte_t>((rgba >> 24) & 0xFF)},
            Green{static_cast<ubyte_t>((rgba >> 16) & 0xFF)},
            Blue{static_cast<ubyte_t>((rgba >> 8) & 0xFF)},
            Alpha{static_cast<ubyte_t>(rgba & 0xFF)}
    };
}

auto reig::Context::set_render_handler(RenderHandler renderHandler) -> void {
    mRenderHandler = renderHandler;
}

auto reig::Context::set_user_ptr(void* ptr) -> void {
    mUserPtr = ptr;
}

auto reig::Context::get_user_ptr() const -> void const* {
    return mUserPtr;
}

auto reig::Context::set_font(char const* aPath, uint_t aTextureId, float aSize) -> FontData {
    // 0 is used as no texture
    if (aTextureId == 0) return {};
    if (aSize <= 0) return {};

    // C io plays nicely with bytes
    auto file = std::fopen(aPath, "rb");
    // If file can't be accessed
    if (!file) return {};
    // Find the amount of memory to allocate for filebuffer
    std::fseek(file, 0, SEEK_END);
    auto fileSize = ftell(file);
    std::rewind(file);
    auto fileContents = new unsigned char[fileSize];
    std::fread(fileContents, 1, fileSize, file);
    std::fclose(file);

    // We want all ASCII chars from space to square
    int const charsNum = 96;
    struct {
        int w, h;
    } constexpr bmp {512, 512};

    auto bakedChars = new stbtt_bakedchar[charsNum];
    auto bitmap = vector<ubyte_t>(bmp.w * bmp.h);
    auto height = stbtt_BakeFontBitmap(
            fileContents, 0, aSize, bitmap.data(), bmp.w, bmp.h, ' ', charsNum, bakedChars
    );
    // No longer need the file, after creating the bitmap
    delete[] fileContents;

    // If the bitmap was not large enough, free memory
    if (height < 0 || height > bmp.h) {
        return {};
    }

    // If all successfull, replace current font data
    if (mFont.bakedChars) delete[] mFont.bakedChars;
    mFont.bakedChars = bakedChars;
    mFont.texture = aTextureId;
    mFont.width = bmp.w;
    mFont.height = height;
    mFont.size = aSize;

    // Return texture creation info
    FontData ret;
    ret.bitmap = bitmap;
    ret.width = bmp.w;
    ret.height = height;

    return ret;
}

auto reig::Context::render_all() -> void {
    if (!mRenderHandler) return;
    if (mCurrentWindow.started) end_window();

    if (!mCurrentWindow.drawData.empty()) {
        mRenderHandler(mCurrentWindow.drawData, mUserPtr);
    }
    if (!mDrawData.empty()) {
        mRenderHandler(mDrawData, mUserPtr);
    }
}

auto reig::Context::start_new_frame() -> void {
    mCurrentWindow.drawData.clear();
    mDrawData.clear();

    mouse.leftButton.mIsClicked = false;
    mouse.mScrolled = 0.f;
}

auto reig::Mouse::move(float_t difx, float_t dify) -> void {
    mCursorPos.x += difx;
    mCursorPos.y += dify;
}

auto reig::Mouse::place(float_t x, float_t y) -> void {
    mCursorPos.x = x;
    mCursorPos.y = y;
}

auto reig::Mouse::scroll(float dy) -> void {
    mScrolled = dy;
}

auto reig::MouseButton::press(float_t x, float_t y) -> void {
    if (!mIsPressed) {
        mIsPressed = true;
        mIsClicked = true;
        mClickedPos = {x, y};
    }
}

auto reig::MouseButton::release() -> void {
    mIsPressed = false;
}

auto reig::Figure::form(vector<Vertex>& vertices, vector<uint_t>& indices, uint_t id) -> void {
    vertices.swap(mVertices);
    indices.swap(mIndices);
    mTextureId = id;
}

auto reig::Figure::vertices() const -> vector<Vertex> const& {
    return mVertices;
}

auto reig::Figure::indices() const -> vector<uint_t> const& {
    return mIndices;
}

auto reig::Figure::texture() const -> uint_t {
    return mTextureId;
}

void reig::Context::start_window(char const* aTitle, float& aX, float& aY) {
    if (mCurrentWindow.started) end_window();

    mCurrentWindow.started = true;
    mCurrentWindow.title = aTitle;
    mCurrentWindow.x = &aX;
    mCurrentWindow.y = &aY;
    mCurrentWindow.w = 0;
    mCurrentWindow.h = 0;
    mCurrentWindow.headerSize = 8 + mFont.size;
}

void reig::Context::end_window() {
    if (!mCurrentWindow.started) return;
    mCurrentWindow.started = false;

    mCurrentWindow.w += 4;
    mCurrentWindow.h += 4;

    Rectangle headerBox{
            *mCurrentWindow.x, *mCurrentWindow.y,
            mCurrentWindow.w, mCurrentWindow.headerSize
    };
    Triangle headerTriangle{
            *mCurrentWindow.x + 3.f, *mCurrentWindow.y + 3.f,
            *mCurrentWindow.x + 3.f + mCurrentWindow.headerSize, *mCurrentWindow.y + 3.f,
            *mCurrentWindow.x + 3.f + mCurrentWindow.headerSize / 2.f,
            *mCurrentWindow.y + mCurrentWindow.headerSize - 3.f
    };
    Rectangle titleBox{
            *mCurrentWindow.x + mCurrentWindow.headerSize + 4, *mCurrentWindow.y + 4,
            mCurrentWindow.w - mCurrentWindow.headerSize - 4, mCurrentWindow.headerSize - 4
    };
    Rectangle bodyBox{
            *mCurrentWindow.x, *mCurrentWindow.y + mCurrentWindow.headerSize,
            mCurrentWindow.w, mCurrentWindow.h - mCurrentWindow.headerSize
    };

    using namespace Colors::literals;
    using namespace Colors::mixing;

    render_rectangle(headerBox, Colors::mediumGrey | 200_a);
    render_triangle(headerTriangle, Colors::lightGrey);
    render_text(mCurrentWindow.title, titleBox);
    render_rectangle(bodyBox, Colors::mediumGrey | 100_a);

    if (mouse.leftButton.mIsPressed && ::reig::detail::in_box(mouse.leftButton.mClickedPos, headerBox)) {
        Point moved{
                mouse.mCursorPos.x - mouse.leftButton.mClickedPos.x,
                mouse.mCursorPos.y - mouse.leftButton.mClickedPos.y
        };

        *mCurrentWindow.x += moved.x;
        *mCurrentWindow.y += moved.y;
        mouse.leftButton.mClickedPos.x += moved.x;
        mouse.leftButton.mClickedPos.y += moved.y;
    }
}

void reig::detail::Window::expand(Rectangle& aBox) {
    if (started) {
        aBox.x += *x + 4;
        aBox.y += *y + headerSize + 4;

        if (*x + w < aBox.x + aBox.width) {
            w = aBox.x + aBox.width - *x;
        }
        if (*y + h < aBox.y + aBox.height) {
            h = aBox.y + aBox.height - *y;
        }
        if (aBox.x < *x) {
            auto d = *x - aBox.x;
            aBox.x += d + 4;
        }
        if (aBox.y < *y) {
            auto d = *y - aBox.y;
            aBox.y += d + 4;
        }
    }
}

void reig::Context::label(char const* ch, Rectangle aBox) {
    mCurrentWindow.expand(aBox);
    render_text(ch, aBox);
}

auto reig::Context::button(char const* aTitle, Rectangle aBox, Color aColor) -> bool {
    mCurrentWindow.expand(aBox);

    // Render button outline first
    Color outlineCol = detail::get_yiq_contrast(aColor);
    render_rectangle(aBox, outlineCol);

    // if cursor is over the button, highlight it
    if (detail::in_box(mouse.mCursorPos, aBox)) {
        aColor = detail::lighten_color_by(aColor, 50);
    }

    // see, if clicked the inner part of button
    aBox = detail::decrease(aBox, 4);
    bool clickedButton = detail::in_box(mouse.leftButton.mClickedPos, aBox);

    // highlight even more, if clicked
    if (mouse.leftButton.mIsPressed && clickedButton) {
        aColor = detail::lighten_color_by(aColor, 50);
    }
    // render the inner part of button
    render_rectangle(aBox, aColor);
    // render button's title
    render_text(aTitle, aBox);

    return mouse.leftButton.mIsClicked && clickedButton;
}

auto reig::Context::button(char const* aTitle, Rectangle aBox, int aBaseTexture, int aHoverTexture) -> bool {
    mCurrentWindow.expand(aBox);

    bool clickedButton = detail::in_box(mouse.leftButton.mClickedPos, aBox);
    bool hoveredButton = detail::in_box(mouse.mCursorPos, aBox);

    if ((mouse.leftButton.mIsPressed && clickedButton) || hoveredButton) {
        render_rectangle(aBox, aBaseTexture);
    } else {
        render_rectangle(aBox, aHoverTexture);
    }

    aBox = detail::decrease(aBox, 8);

    render_text(aTitle, aBox);

    return mouse.leftButton.mIsClicked && clickedButton;
}

auto reig::Context::slider(Rectangle aBox,
                           Color aColor,
                           float_t& aValue,
                           float_t aMin,
                           float_t aMax,
                           float_t aStep) -> bool {
    mCurrentWindow.expand(aBox);

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
    cursorBox.width /= valuesNum;
    if (cursorBox.width < 1) cursorBox.width = 1;
    cursorBox.x += offset * cursorBox.width;
    if (detail::in_box(mouse.mCursorPos, cursorBox)) {
        cursorColor = detail::lighten_color_by(cursorColor, 50);
    }
    render_rectangle(cursorBox, cursorColor);

    if (mouse.leftButton.mIsPressed && detail::in_box(mouse.leftButton.mClickedPos, aBox)) {
        auto halfCursorW = cursorBox.width / 2;
        auto distance = mouse.mCursorPos.x - cursorBox.x - halfCursorW;

        if (detail::abs(distance) > halfCursorW) {
            value += static_cast<int_t>(distance / cursorBox.width) * aStep;
            value = detail::clamp(value, min, max);
        }
    } else if (mouse.mScrolled != 0 && detail::in_box(mouse.mCursorPos, aBox)) {
        value += static_cast<int_t>(mouse.mScrolled) * aStep;
        value = detail::clamp(value, min, max);
    }

    if (aValue != value) {
        aValue = value;
        return true;
    } else {
        return false;
    }
}

auto reig::Context::slider(
        Rectangle aBox,
        int aBaseTexture,
        int aCursorTexture,
        float_t& aValue,
        float_t aMin,
        float_t aMax,
        float_t aStep) -> bool {
    mCurrentWindow.expand(aBox);

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
    cursorBox.width /= valuesNum;
    cursorBox.x += offset * cursorBox.width;
    render_rectangle(cursorBox, aCursorTexture);

    if (mouse.leftButton.mIsPressed && detail::in_box(mouse.leftButton.mClickedPos, aBox)) {
        auto halfCursorW = cursorBox.width / 2;
        auto distance = mouse.mCursorPos.x - cursorBox.x - halfCursorW;

        if (detail::abs(distance) > halfCursorW) {
            value += static_cast<int_t>(distance / cursorBox.width) * aStep;
            value = detail::clamp(value, min, max);
        }
    } else if (mouse.mScrolled != 0 && detail::in_box(mouse.mCursorPos, aBox)) {
        value += static_cast<int_t>(mouse.mScrolled) * aStep;
        value = detail::clamp(value, min, max);
    }

    if (aValue != value) {
        aValue = value;
        return true;
    } else {
        return false;
    }
}

auto reig::Context::checkbox(Rectangle aBox, Color aColor, bool& aValue) -> bool {
    mCurrentWindow.expand(aBox);

    // Render checkbox's base
    Color contrastColor = detail::get_yiq_contrast(aColor);
    render_rectangle(aBox, contrastColor);
    aBox = detail::decrease(aBox, 4);
    render_rectangle(aBox, aColor);

    // Render check
    if (aValue) {
        aBox = detail::decrease(aBox, 4);
        render_rectangle(aBox, contrastColor);
    }

    // True if state changed
    if (mouse.leftButton.mIsClicked && detail::in_box(mouse.leftButton.mClickedPos, aBox)) {
        aValue = !aValue;
        return true;
    } else {
        return false;
    }
}

auto reig::Context::checkbox(Rectangle aBox, int aBaseTexture, int aTickTexture, bool& aValue) -> bool {
    mCurrentWindow.expand(aBox);

    // Render checkbox's base
    render_rectangle(aBox, aBaseTexture);

    // Render check
    if (aValue) {
        aBox = detail::decrease(aBox, 8);
        render_rectangle(aBox, aTickTexture);
    }

    // True if state changed
    if (mouse.leftButton.mIsClicked && detail::in_box(mouse.leftButton.mClickedPos, aBox)) {
        aValue = !aValue;
        return true;
    } else {
        return false;
    }
}

auto reig::Context::render_text(char const* ch, Rectangle aBox) -> void {
    if (!mFont.bakedChars || !ch) return;

    aBox = detail::decrease(aBox, 8);
    float x = aBox.x;
    float y = aBox.y + aBox.height;

    vector<stbtt_aligned_quad> quads;
    quads.reserve(20);
    float textWidth = 0.f;

    char from = ' ';
    char to = ' ' + 95;
    char c;

    for (; *ch; ++ch) {
        c = *ch;
        if (c < from || c > to) c = to;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(
                mFont.bakedChars,
                mFont.width, mFont.height, c - ' ', &x, &y, &q, 1
        );
        if (q.x0 > aBox.x + aBox.width) {
            break;
        }
        if (q.x1 > aBox.x + aBox.width) {
            q.x1 = aBox.x + aBox.width;
        }
        if (q.y0 < aBox.y) {
            q.y0 = aBox.y;
        }

        textWidth += mFont.bakedChars[c - ' '].xadvance;

        quads.push_back(q);
    }

    auto deltax = (aBox.width - textWidth) / 2.f;

    for (auto& q : quads) {
        vector<Vertex> vertices{
                {{q.x0 + deltax, q.y0}, {q.s0, q.t0}, {}},
                {{q.x1 + deltax, q.y0}, {q.s1, q.t0}, {}},
                {{q.x1 + deltax, q.y1}, {q.s1, q.t1}, {}},
                {{q.x0 + deltax, q.y1}, {q.s0, q.t1}, {}}
        };
        vector<uint_t> indices{0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices, mFont.texture);
        mDrawData.push_back(fig);
    }
}

auto reig::Context::render_triangle(Triangle const& aTri, Color const& aColor) -> void {
    vector<Vertex> vertices{
            {{aTri.pos0}, {}, aColor},
            {{aTri.pos1}, {}, aColor},
            {{aTri.pos2}, {}, aColor}
    };
    vector<uint_t> indices = {0, 1, 2};

    Figure fig;
    fig.form(vertices, indices);
    mDrawData.push_back(fig);
}

auto reig::Context::render_rectangle(Rectangle const& aBox, int aTexture) -> void {
    vector<Vertex> vertices{
            {{aBox.x,              aBox.y},               {0.f, 0.f}, {}},
            {{aBox.x + aBox.width, aBox.y},               {1.f, 0.f}, {}},
            {{aBox.x + aBox.width, aBox.y + aBox.height}, {1.f, 1.f}, {}},
            {{aBox.x,              aBox.y + aBox.height}, {0.f, 1.f}, {}}
    };
    vector<uint_t> indices{0, 1, 2, 2, 3, 0};

    Figure fig;
    fig.form(vertices, indices, aTexture);
    mDrawData.push_back(fig);
}

auto reig::Context::render_rectangle(Rectangle const& aRect, Color const& aColor) -> void {
    vector<Vertex> vertices{
            {{aRect.x,               aRect.y},                {}, aColor},
            {{aRect.x + aRect.width, aRect.y},                {}, aColor},
            {{aRect.x + aRect.width, aRect.y + aRect.height}, {}, aColor},
            {{aRect.x,               aRect.y + aRect.height}, {}, aColor}
    };
    vector<uint_t> indices{0, 1, 2, 2, 3, 0};

    Figure fig;
    fig.form(vertices, indices);
    mDrawData.push_back(fig);
}
