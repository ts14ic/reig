#include "reig.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#include "stb_truetype.h"
#include <sstream>
#include <memory>

using std::unique_ptr;
using std::vector;

namespace reig::detail {
    template<typename T>
    bool is_between(T val, T min, T max) {
        return val > min && val < max;
    }

    bool is_boxed_in(Point const& pt, Rectangle const& box) {
        return is_between(pt.x, box.x, box.x + box.width) && is_between(pt.y, box.y, box.y + box.height);
    }

    template<typename T>
    T clamp(T val, T min, T max) {
        return val < min ? min :
               val > max ? max :
               val;
    }

    template<typename T>
    T min(T a, T b) {
        return a < b ? a : b;
    }

    template<typename T>
    T max(T a, T b) {
        return a > b ? a : b;
    }

    template<typename T>
    T abs(T a) {
        return a < 0 ? -a : a;
    }

    template<typename T>
    int_t sign(T a) {
        return a < 0 ? -1 :
               a > 0 ? 1 :
               0;
    }

    Color get_yiq_contrast(Color color) {
        using namespace Colors::literals;

        uint_t y = (299u * color.red.val + 587 * color.green.val + 114 * color.blue.val) / 1000;
        return y >= 128
               ? Color{0_r, 0_g, 0_b, 255_a}
               : Color{255_r, 255_g, 255_b};
    }

    Color lighten_color_by(Color color, ubyte_t delta) {
        ubyte_t max = 255u;
        color.red.val < max - delta ? color.red.val += delta : color.red.val = max;
        color.green.val < max - delta ? color.green.val += delta : color.green.val = max;
        color.blue.val < max - delta ? color.blue.val += delta : color.blue.val = max;
        return color;
    }

    Rectangle decrease_box(Rectangle aRect, int_t by) {
        int_t moveBy = by / 2;
        aRect.x += moveBy;
        aRect.y += moveBy;
        aRect.width -= by;
        aRect.height -= by;
        return aRect;
    }

    template <typename R, typename T, typename = std::enable_if_t<std::is_integral_v<R> && std::is_integral_v<R>>>
    R integral_cast(T t) {
        auto r = static_cast<R>(t);
        if (r != t || (std::is_signed_v<T> != std::is_signed_v<R> && ((t < T{}) != r < R{}))) {
            std::stringstream ss;
            ss << "Bad narrow cast from " << typeid(T).name() << "(" << t << ") to type " << typeid(R).name();
            throw std::range_error(ss.str());
        }
        return r;
    };
}

reig::uint_t reig::Colors::to_uint(Color const& color) {
    return (color.alpha.val << 24)
           + (color.blue.val << 16)
           + (color.green.val << 8)
           + color.red.val;
}

reig::Color reig::Colors::from_uint(uint_t rgba) {
    return Color {
            Red{static_cast<ubyte_t>((rgba >> 24) & 0xFF)},
            Green{static_cast<ubyte_t>((rgba >> 16) & 0xFF)},
            Blue{static_cast<ubyte_t>((rgba >> 8) & 0xFF)},
            Alpha{static_cast<ubyte_t>(rgba & 0xFF)}
    };
}

void reig::Context::set_render_handler(RenderHandler renderHandler) {
    mRenderHandler = renderHandler;
}

void reig::Context::set_user_ptr(std::any ptr) {
    mUserPtr = std::move(ptr);
}

std::any const& reig::Context::get_user_ptr() const {
    return mUserPtr;
}

reig::FailedToLoadFontException::FailedToLoadFontException(std::string message)
        : message{std::move(message)} {}

const char* reig::FailedToLoadFontException::what() const noexcept {
    return message.c_str();
}

reig::FailedToLoadFontException reig::FailedToLoadFontException::noTextureId(const char* filePath) {
    std::ostringstream ss;
    ss << "No texture id was specified for font: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

reig::FailedToLoadFontException reig::FailedToLoadFontException::invalidSize(const char* filePath, float fontSize) {
    std::ostringstream ss;
    ss << "Invalid size specified for font: [" << filePath << "], size: [" << fontSize << "]";
    return FailedToLoadFontException(ss.str());
}

reig::FailedToLoadFontException reig::FailedToLoadFontException::couldNotOpenFile(const char* filePath) {
    std::ostringstream ss;
    ss << "Could not open font file: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

reig::FailedToLoadFontException reig::FailedToLoadFontException::couldNotFitCharacters(const char* filePath, float fontSize, reig::uint_t width,
                                                            reig::uint_t height) {
    std::ostringstream ss;
    ss << "Could not fit characters for font: ["
       << filePath << "], size: [" << fontSize << "], atlas size: ["
       << width << "x" << height << "]";
    return FailedToLoadFontException(ss.str());
}

reig::FailedToLoadFontException reig::FailedToLoadFontException::invalidFile(const char* filePath) {
    std::ostringstream ss;
    ss << "Invalid file for font: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

vector<reig::ubyte_t> read_font_into_buffer(char const* fontFilePath) {
    auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(std::fopen(fontFilePath, "rb"), &std::fclose);
    if (!file) throw reig::FailedToLoadFontException::couldNotOpenFile(fontFilePath);

    std::fseek(file.get(), 0, SEEK_END);
    long filePos = ftell(file.get());
    if (filePos < 0) throw reig::FailedToLoadFontException::invalidFile(fontFilePath);

    auto fileSize = reig::detail::integral_cast<reig::size_t>(filePos);
    std::rewind(file.get());

    auto ttfBuffer = std::vector<unsigned char>(fileSize);
    std::fread(ttfBuffer.data(), 1, fileSize, file.get());
    return ttfBuffer;
}

reig::FontData reig::Context::set_font(char const* fontFilePath, uint_t textureId, float fontHeightPx) {
    if (textureId == 0) throw FailedToLoadFontException::noTextureId(fontFilePath);
    if (fontHeightPx <= 0) throw FailedToLoadFontException::invalidSize(fontFilePath, fontHeightPx);

    auto ttfBuffer = read_font_into_buffer(fontFilePath);

    // We want all ASCII chars from space to square
    int const charsNum = 96;
    struct {
        uint_t w, h;
    } constexpr bmp {512, 512};

    auto bakedChars = std::vector<stbtt_bakedchar>(charsNum);
    auto bitmap = vector<ubyte_t>(bmp.w * bmp.h);
    auto height = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, fontHeightPx, bitmap.data(), bmp.w, bmp.h, ' ', charsNum, std::data(bakedChars)
    );

    if (height < 0 || height > bmp.h) {
        throw FailedToLoadFontException::couldNotFitCharacters(fontFilePath, fontHeightPx, bmp.w, bmp.h);
    }

    // If all successfull, replace current font data
    mFont.bakedChars = std::move(bakedChars);
    mFont.texture = textureId;
    mFont.width = bmp.w;
    mFont.height = height;
    mFont.size = fontHeightPx;

    // Return texture creation info
    FontData ret;
    ret.bitmap = bitmap;
    ret.width = bmp.w;
    ret.height = height;

    return ret;
}

const char* reig::NoRenderHandlerException::what() const noexcept {
    return "No render handler specified";
}

void reig::Context::render_all() {
    if (!mRenderHandler) {
        throw NoRenderHandlerException{};
    }
    if (mCurrentWindow.started) end_window();

    if (!mCurrentWindow.drawData.empty()) {
        mRenderHandler(mCurrentWindow.drawData, mUserPtr);
    }
    if (!mDrawData.empty()) {
        mRenderHandler(mDrawData, mUserPtr);
    }
}

void reig::Context::start_new_frame() {
    mCurrentWindow.drawData.clear();
    mDrawData.clear();

    mouse.leftButton.mIsClicked = false;
    mouse.mScrolled = 0.f;
}

void reig::Mouse::move(float_t difx, float_t dify) {
    mCursorPos.x += difx;
    mCursorPos.y += dify;
}

void reig::Mouse::place(float_t x, float_t y) {
    mCursorPos.x = x;
    mCursorPos.y = y;
}

void reig::Mouse::scroll(float dy) {
    mScrolled = dy;
}

const reig::Point& reig::Mouse::get_pos() {
    return mCursorPos;
}

void reig::MouseButton::press(float_t x, float_t y) {
    if (!mIsPressed) {
        mIsPressed = true;
        mIsClicked = true;
        mClickedPos = {x, y};
    }
}

void reig::MouseButton::release() {
    mIsPressed = false;
}

const reig::Point& reig::MouseButton::get_clicked_pos() {
    return mClickedPos;
}

bool reig::MouseButton::is_pressed() {
    return mIsPressed;
}

bool reig::MouseButton::is_clicked() {
    return mIsClicked;
}

void reig::Figure::form(vector<Vertex>& vertices, vector<uint_t>& indices, uint_t id) {
    vertices.swap(mVertices);
    indices.swap(mIndices);
    mTextureId = id;
}

vector<reig::Vertex> const& reig::Figure::vertices() const {
    return mVertices;
}

vector<reig::uint_t> const& reig::Figure::indices() const {
    return mIndices;
}

reig::uint_t reig::Figure::texture() const {
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

    if (mouse.leftButton.mIsPressed && ::reig::detail::is_boxed_in(mouse.leftButton.mClickedPos, headerBox)) {
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

void reig::Context::fit_rect_in_window(Rectangle& rect) {
    mCurrentWindow.expand(rect);
}

void reig::Context::label(char const* ch, Rectangle aBox) {
    mCurrentWindow.expand(aBox);
    render_text(ch, aBox);
}

bool reig::colored_button::draw(reig::Context& ctx) const {
    Rectangle rect = this->rect;
    ctx.fit_rect_in_window(rect);

    // Render button outline first
    Color outlineCol = detail::get_yiq_contrast(color);
    ctx.render_rectangle(rect, outlineCol);

    Color color = this->color;
    // if cursor is over the button, highlight it
    if (detail::is_boxed_in(ctx.mouse.get_pos(), rect)) {
        color = detail::lighten_color_by(color, 50);
    }

    // see, if clicked the inner part of button
    rect = detail::decrease_box(rect, 4);
    bool clickedInBox = detail::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), rect);

    // highlight even more, if clicked
    if (ctx.mouse.leftButton.is_pressed() && clickedInBox) {
        color = detail::lighten_color_by(color, 50);
    }

    // render the inner part of button
    ctx.render_rectangle(rect, color);
    // render button's title
    ctx.render_text(title, rect);

    return ctx.mouse.leftButton.is_clicked() && clickedInBox;
}

bool reig::Context::button(char const* aTitle, Rectangle aBox, int aBaseTexture, int aHoverTexture) {
    mCurrentWindow.expand(aBox);

    bool clickedButton = detail::is_boxed_in(mouse.leftButton.mClickedPos, aBox);
    bool hoveredButton = detail::is_boxed_in(mouse.mCursorPos, aBox);

    if ((mouse.leftButton.mIsPressed && clickedButton) || hoveredButton) {
        render_rectangle(aBox, aBaseTexture);
    } else {
        render_rectangle(aBox, aHoverTexture);
    }

    aBox = detail::decrease_box(aBox, 8);

    render_text(aTitle, aBox);

    return mouse.leftButton.mIsClicked && clickedButton;
}

bool reig::Context::slider(Rectangle aBox,
                           Color aColor,
                           float_t& aValue,
                           float_t aMin,
                           float_t aMax,
                           float_t aStep) {
    mCurrentWindow.expand(aBox);

    // Render slider's base
    Color cursorColor = detail::get_yiq_contrast(aColor);
    render_rectangle(aBox, cursorColor);
    aBox = detail::decrease_box(aBox, 4);
    render_rectangle(aBox, aColor);

    // Prepare the values
    float_t min = detail::min(aMin, aMax);
    float_t max = detail::max(aMin, aMax);
    float_t value = detail::clamp(aValue, min, max);
    int_t offset = static_cast<int_t>((value - min) / aStep);
    int_t valuesNum = (max - min) / aStep + 1;

    // Render the cursor
    auto cursorBox = detail::decrease_box(aBox, 4);
    cursorBox.width /= valuesNum;
    if (cursorBox.width < 1) cursorBox.width = 1;
    cursorBox.x += offset * cursorBox.width;
    if (detail::is_boxed_in(mouse.mCursorPos, cursorBox)) {
        cursorColor = detail::lighten_color_by(cursorColor, 50);
    }
    render_rectangle(cursorBox, cursorColor);

    if (mouse.leftButton.mIsPressed && detail::is_boxed_in(mouse.leftButton.mClickedPos, aBox)) {
        auto halfCursorW = cursorBox.width / 2;
        auto distance = mouse.mCursorPos.x - cursorBox.x - halfCursorW;

        if (detail::abs(distance) > halfCursorW) {
            value += static_cast<int_t>(distance / cursorBox.width) * aStep;
            value = detail::clamp(value, min, max);
        }
    } else if (mouse.mScrolled != 0 && detail::is_boxed_in(mouse.mCursorPos, aBox)) {
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

bool reig::Context::slider(
        Rectangle aBox,
        int aBaseTexture,
        int aCursorTexture,
        float_t& aValue,
        float_t aMin,
        float_t aMax,
        float_t aStep) {
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
    auto cursorBox = detail::decrease_box(aBox, 8);
    cursorBox.width /= valuesNum;
    cursorBox.x += offset * cursorBox.width;
    render_rectangle(cursorBox, aCursorTexture);

    if (mouse.leftButton.mIsPressed && detail::is_boxed_in(mouse.leftButton.mClickedPos, aBox)) {
        auto halfCursorW = cursorBox.width / 2;
        auto distance = mouse.mCursorPos.x - cursorBox.x - halfCursorW;

        if (detail::abs(distance) > halfCursorW) {
            value += static_cast<int_t>(distance / cursorBox.width) * aStep;
            value = detail::clamp(value, min, max);
        }
    } else if (mouse.mScrolled != 0 && detail::is_boxed_in(mouse.mCursorPos, aBox)) {
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

bool reig::Context::checkbox(Rectangle aBox, Color aColor, bool& aValue) {
    mCurrentWindow.expand(aBox);

    // Render checkbox's base
    Color contrastColor = detail::get_yiq_contrast(aColor);
    render_rectangle(aBox, contrastColor);
    aBox = detail::decrease_box(aBox, 4);
    render_rectangle(aBox, aColor);

    // Render check
    if (aValue) {
        aBox = detail::decrease_box(aBox, 4);
        render_rectangle(aBox, contrastColor);
    }

    // True if state changed
    if (mouse.leftButton.mIsClicked && detail::is_boxed_in(mouse.leftButton.mClickedPos, aBox)) {
        aValue = !aValue;
        return true;
    } else {
        return false;
    }
}

bool reig::Context::checkbox(Rectangle aBox, int aBaseTexture, int aTickTexture, bool& aValue) {
    mCurrentWindow.expand(aBox);

    // Render checkbox's base
    render_rectangle(aBox, aBaseTexture);

    // Render check
    if (aValue) {
        aBox = detail::decrease_box(aBox, 8);
        render_rectangle(aBox, aTickTexture);
    }

    // True if state changed
    if (mouse.leftButton.mIsClicked && detail::is_boxed_in(mouse.leftButton.mClickedPos, aBox)) {
        aValue = !aValue;
        return true;
    } else {
        return false;
    }
}

void reig::Context::render_text(char const* ch, Rectangle aBox) {
    if (mFont.bakedChars.empty() || !ch) return;

    aBox = detail::decrease_box(aBox, 8);
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
                mFont.bakedChars.data(),
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

void reig::Context::render_triangle(Triangle const& aTri, Color const& aColor) {
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

void reig::Context::render_rectangle(Rectangle const& aBox, int aTexture) {
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

void reig::Context::render_rectangle(Rectangle const& aRect, Color const& aColor) {
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
