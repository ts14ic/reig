#include "reig.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#include "stb_truetype.h"
#include <sstream>
#include <memory>

using std::unique_ptr;
using std::vector;

namespace internal {
    using namespace reig;
    using namespace primitive;

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
    int sign(T a) {
        return a < 0 ? -1 :
               a > 0 ? 1 :
               0;
    }

    primitive::Color get_yiq_contrast(primitive::Color color) {
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

    Rectangle decrease_box(Rectangle aRect, int by) {
        int moveBy = by / 2;
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
            ss << "Bad integral cast from " << typeid(T).name() << "(" << t << ") to type " << typeid(R).name();
            throw std::range_error(ss.str());
        }
        return r;
    };
}

using namespace reig::primitive;

reig::uint32_t colors::to_uint(Color const& color) {
    return (color.alpha.val << 24)
           + (color.blue.val << 16)
           + (color.green.val << 8)
           + color.red.val;
}

Color colors::from_uint(uint32_t rgba) {
    return Color {
            Color::Red{static_cast<uint8_t>((rgba >> 24) & 0xFF)},
            Color::Green{static_cast<uint8_t>((rgba >> 16) & 0xFF)},
            Color::Blue{static_cast<uint8_t>((rgba >> 8) & 0xFF)},
            Color::Alpha{static_cast<uint8_t>(rgba & 0xFF)}
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

reig::except::FailedToLoadFontException::FailedToLoadFontException(std::string message)
        : message{std::move(message)} {}

const char* reig::except::FailedToLoadFontException::what() const noexcept {
    return message.c_str();
}

reig::except::FailedToLoadFontException reig::except::FailedToLoadFontException::noTextureId(const char* filePath) {
    std::ostringstream ss;
    ss << "No texture id was specified for font: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

reig::except::FailedToLoadFontException reig::except::FailedToLoadFontException::invalidSize(const char* filePath, float fontSize) {
    std::ostringstream ss;
    ss << "Invalid size specified for font: [" << filePath << "], size: [" << fontSize << "]";
    return FailedToLoadFontException(ss.str());
}

reig::except::FailedToLoadFontException reig::except::FailedToLoadFontException::couldNotOpenFile(const char* filePath) {
    std::ostringstream ss;
    ss << "Could not open font file: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

reig::except::FailedToLoadFontException reig::except::FailedToLoadFontException::couldNotFitCharacters(
        const char* filePath, float fontSize, int width, int height) {
    std::ostringstream ss;
    ss << "Could not fit characters for font: ["
       << filePath << "], size: [" << fontSize << "], atlas size: ["
       << width << "x" << height << "]";
    return FailedToLoadFontException(ss.str());
}

reig::except::FailedToLoadFontException reig::except::FailedToLoadFontException::invalidFile(const char* filePath) {
    std::ostringstream ss;
    ss << "Invalid file for font: [" << filePath << "]";
    return FailedToLoadFontException(ss.str());
}

vector<reig::uint8_t> read_font_into_buffer(char const* fontFilePath) {
    auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(std::fopen(fontFilePath, "rb"), &std::fclose);
    if (!file) throw reig::except::FailedToLoadFontException::couldNotOpenFile(fontFilePath);

    std::fseek(file.get(), 0, SEEK_END);
    long filePos = ftell(file.get());
    if (filePos < 0) throw reig::except::FailedToLoadFontException::invalidFile(fontFilePath);

    auto fileSize = internal::integral_cast<size_t>(filePos);
    std::rewind(file.get());

    auto ttfBuffer = std::vector<unsigned char>(fileSize);
    std::fread(ttfBuffer.data(), 1, fileSize, file.get());
    return ttfBuffer;
}

reig::FontData reig::Context::set_font(char const* fontFilePath, int textureId, float fontHeightPx) {
    if (textureId == 0) throw except::FailedToLoadFontException::noTextureId(fontFilePath);
    if (fontHeightPx <= 0) throw except::FailedToLoadFontException::invalidSize(fontFilePath, fontHeightPx);

    auto ttfBuffer = read_font_into_buffer(fontFilePath);

    // We want all ASCII chars from space to square
    int const charsNum = 96;
    struct {
        int w, h;
    } constexpr bmp {512, 512};

    auto bakedChars = std::vector<stbtt_bakedchar>(charsNum);
    auto bitmap = vector<uint8_t>(internal::integral_cast<size_t>(bmp.w * bmp.h));
    auto height = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, fontHeightPx, bitmap.data(), bmp.w, bmp.h, ' ', charsNum, std::data(bakedChars)
    );

    if (height < 0 || height > bmp.h) {
        throw except::FailedToLoadFontException::couldNotFitCharacters(fontFilePath, fontHeightPx, bmp.w, bmp.h);
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

const char* reig::except::NoRenderHandlerException::what() const noexcept {
    return "No render handler specified";
}

void reig::Context::render_all() {
    if (!mRenderHandler) {
        throw except::NoRenderHandlerException{};
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

void reig::detail::Mouse::move(float_t difx, float_t dify) {
    mCursorPos.x += difx;
    mCursorPos.y += dify;
}

void reig::detail::Mouse::place(float_t x, float_t y) {
    mCursorPos.x = x;
    mCursorPos.y = y;
}

void reig::detail::Mouse::scroll(float dy) {
    mScrolled = dy;
}

const Point& reig::detail::Mouse::get_cursor_pos() const {
    return mCursorPos;
}

float reig::detail::Mouse::get_scrolled() const {
    return mScrolled;
}

void reig::detail::MouseButton::press(float_t x, float_t y) {
    if (!mIsPressed) {
        mIsPressed = true;
        mIsClicked = true;
        mClickedPos = {x, y};
    }
}

void reig::detail::MouseButton::release() {
    mIsPressed = false;
}

const Point& reig::detail::MouseButton::get_clicked_pos() const {
    return mClickedPos;
}

bool reig::detail::MouseButton::is_pressed() const {
    return mIsPressed;
}

bool reig::detail::MouseButton::is_clicked() const {
    return mIsClicked;
}

void Figure::form(vector<Vertex>& vertices, vector<int>& indices, int id) {
    vertices.swap(mVertices);
    indices.swap(mIndices);
    mTextureId = id;
}

vector<Vertex> const& Figure::vertices() const {
    return mVertices;
}

vector<int> const& Figure::indices() const {
    return mIndices;
}

int Figure::texture() const {
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

    using namespace colors::literals;
    using namespace colors::operators;

    render_rectangle(headerBox, colors::mediumGrey | 200_a);
    render_triangle(headerTriangle, colors::lightGrey);
    render_text(mCurrentWindow.title, titleBox);
    render_rectangle(bodyBox, colors::mediumGrey | 100_a);

    if (mouse.leftButton.is_pressed() && internal::is_boxed_in(mouse.leftButton.get_clicked_pos(), headerBox)) {
        Point moved{
                mouse.get_cursor_pos().x - mouse.leftButton.get_clicked_pos().x,
                mouse.get_cursor_pos().y - mouse.leftButton.get_clicked_pos().y
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

bool reig::reference_widgets::button::draw(reig::Context& ctx) const {
    Rectangle box = this->mBoundingBox;
    ctx.fit_rect_in_window(box);

    // Render button outline first
    Color outlineCol = internal::get_yiq_contrast(mBaseColor);
    ctx.render_rectangle(box, outlineCol);

    Color color = this->mBaseColor;
    // if cursor is over the button, highlight it
    if (internal::is_boxed_in(ctx.mouse.get_cursor_pos(), box)) {
        color = internal::lighten_color_by(color, 50);
    }

    // see, if clicked the inner part of button
    box = internal::decrease_box(box, 4);
    bool clickedInBox = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), box);

    // highlight even more, if clicked
    if (ctx.mouse.leftButton.is_pressed() && clickedInBox) {
        color = internal::lighten_color_by(color, 50);
    }

    // render the inner part of button
    ctx.render_rectangle(box, color);
    // render button's title
    ctx.render_text(mTitle, box);

    return ctx.mouse.leftButton.is_clicked() && clickedInBox;
}


bool reig::reference_widgets::textured_button::draw(reig::Context& ctx) const {
    Rectangle box = this->mBoundingBox;
    ctx.fit_rect_in_window(box);

    bool clickedInBox = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), box);
    bool hoveredOnBox = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), box);

    if((ctx.mouse.leftButton.is_pressed() && clickedInBox) || hoveredOnBox) {
        ctx.render_rectangle(box, mBaseTexture);
    } else {
        ctx.render_rectangle(box, mHoverTexture);
    }

    box = internal::decrease_box(box, 8);
    ctx.render_text(mTitle, box);

    return ctx.mouse.leftButton.is_clicked() && clickedInBox;
}

void reig::reference_widgets::label::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);
    ctx.render_text(mTitle, boundingBox);
}

bool reig::reference_widgets::slider::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);

    // Render slider's base
    Color cursorColor = internal::get_yiq_contrast(mBaseColor);
    ctx.render_rectangle(boundingBox, cursorColor);
    boundingBox = internal::decrease_box(boundingBox, 4);
    ctx.render_rectangle(boundingBox, mBaseColor);

    // Prepare the values
    float_t min = internal::min(mMin, mMax);
    float_t max = internal::max(mMin, mMax);
    float_t value = internal::clamp(mValueRef, min, max);
    int offset = static_cast<int>((value - min) / mStep);
    int valuesNum = static_cast<int>((max - min) / mStep + 1);

    // Render the cursor
    auto cursorBox = internal::decrease_box(boundingBox, 4);
    cursorBox.width /= valuesNum;
    if (cursorBox.width < 1) cursorBox.width = 1;
    cursorBox.x += offset * cursorBox.width;
    if (internal::is_boxed_in(ctx.mouse.get_cursor_pos(), cursorBox)) {
        cursorColor = internal::lighten_color_by(cursorColor, 50);
    }
    ctx.render_rectangle(cursorBox, cursorColor);

    if (ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        auto halfCursorW = cursorBox.width / 2;
        auto distance = ctx.mouse.get_cursor_pos().x - cursorBox.x - halfCursorW;

        if (internal::abs(distance) > halfCursorW) {
            value += static_cast<int>(distance / cursorBox.width) * mStep;
            value = internal::clamp(value, min, max);
        }
    } else if (ctx.mouse.get_scrolled() != 0 && internal::is_boxed_in(ctx.mouse.get_cursor_pos(), boundingBox)) {
        value += static_cast<int>(ctx.mouse.get_scrolled()) * mStep;
        value = internal::clamp(value, min, max);
    }

    if (mValueRef != value) {
        mValueRef = value;
        return true;
    } else {
        return false;
    }
}

bool reig::reference_widgets::slider_textured::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);

    // Render slider's base
    ctx.render_rectangle(boundingBox, mBaseTexture);

    // Prepare the values
    float_t min = internal::min(mMin, mMax);
    float_t max = internal::max(mMin, mMax);
    float_t value = internal::clamp(mValueRef, min, max);
    int offset = static_cast<int>((value - min) / mStep);
    int valuesNum = static_cast<int>((max - min) / mStep + 1);

    // Render the cursor
    auto cursorBox = internal::decrease_box(boundingBox, 8);
    cursorBox.width /= valuesNum;
    cursorBox.x += offset * cursorBox.width;
    ctx.render_rectangle(cursorBox, mCursorTexture);

    if (ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        auto halfCursorW = cursorBox.width / 2;
        auto distance = ctx.mouse.get_cursor_pos().x - cursorBox.x - halfCursorW;

        if (internal::abs(distance) > halfCursorW) {
            value += static_cast<int>(distance / cursorBox.width) * mStep;
            value = internal::clamp(value, min, max);
        }
    } else if (ctx.mouse.get_scrolled() != 0 && internal::is_boxed_in(ctx.mouse.get_cursor_pos(), boundingBox)) {
        value += static_cast<int>(ctx.mouse.get_scrolled()) * mStep;
        value = internal::clamp(value, min, max);
    }

    if (mValueRef != value) {
        mValueRef = value;
        return true;
    } else {
        return false;
    }
}

bool reig::reference_widgets::checkbox::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);

    // Render checkbox's base
    Color contrastColor = internal::get_yiq_contrast(mBaseColor);
    ctx.render_rectangle(boundingBox, contrastColor);
    boundingBox = internal::decrease_box(boundingBox, 4);
    ctx.render_rectangle(boundingBox, mBaseColor);

    // Render check
    if (mValueRef) {
        boundingBox = internal::decrease_box(boundingBox, 4);
        ctx.render_rectangle(boundingBox, contrastColor);
    }

    // True if state changed
    if (ctx.mouse.leftButton.is_clicked() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        mValueRef = !mValueRef;
        return true;
    } else {
        return false;
    }
}

bool reig::reference_widgets::textured_checkbox::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);

    // Render checkbox's base
    ctx.render_rectangle(boundingBox, mBaseTexture);

    // Render check
    if (mValueRef) {
        boundingBox = internal::decrease_box(boundingBox, 8);
        ctx.render_rectangle(boundingBox, mCheckTexture);
    }

    // True if state changed
    if (ctx.mouse.leftButton.is_clicked() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        mValueRef = !mValueRef;
        return true;
    } else {
        return false;
    }
}

void reig::Context::render_text(char const* ch, Rectangle aBox) {
    if (mFont.bakedChars.empty() || !ch) return;

    aBox = internal::decrease_box(aBox, 8);
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
        vector<int> indices{0, 1, 2, 2, 3, 0};

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
    vector<int> indices = {0, 1, 2};

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
    vector<int> indices{0, 1, 2, 2, 3, 0};

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
    vector<int> indices{0, 1, 2, 2, 3, 0};

    Figure fig;
    fig.form(vertices, indices);
    mDrawData.push_back(fig);
}
