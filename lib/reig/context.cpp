#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION

#include "context.h"
#include "internal.h"
#include "exception.h"
#include <memory>

using std::unique_ptr;
using std::vector;

using namespace reig::primitive;
namespace internal = reig::internal;

void reig::Context::set_render_handler(RenderHandler renderHandler) {
    mRenderHandler = renderHandler;
}

void reig::Context::set_user_ptr(std::any ptr) {
    using std::move;
    mUserPtr = move(ptr);
}

std::any const& reig::Context::get_user_ptr() const {
    return mUserPtr;
}

vector<reig::uint8_t> read_font_into_buffer(char const* fontFilePath) {
    using reig::exception::FailedToLoadFontException;

    auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(std::fopen(fontFilePath, "rb"), &std::fclose);
    if (!file) throw FailedToLoadFontException::couldNotOpenFile(fontFilePath);

    std::fseek(file.get(), 0, SEEK_END);
    long filePos = ftell(file.get());
    if (filePos < 0) throw FailedToLoadFontException::invalidFile(fontFilePath);

    auto fileSize = internal::integral_cast<size_t>(filePos);
    std::rewind(file.get());

    auto ttfBuffer = std::vector<unsigned char>(fileSize);
    std::fread(ttfBuffer.data(), 1, fileSize, file.get());
    return ttfBuffer;
}

reig::Context::FontData reig::Context::set_font(char const* fontFilePath, int textureId, float fontHeightPx) {
    using exception::FailedToLoadFontException;

    if (textureId == 0) throw FailedToLoadFontException::noTextureId(fontFilePath);
    if (fontHeightPx <= 0) throw FailedToLoadFontException::invalidSize(fontFilePath, fontHeightPx);

    auto ttfBuffer = read_font_into_buffer(fontFilePath);

    // We want all ASCII chars from space to square
    int const charsNum = 96;
    int bitmapWidth = 512;
    int bitmapHeight = 512;

    auto bakedChars = std::vector<stbtt_bakedchar>(charsNum);
    auto bitmap = vector<uint8_t>(internal::integral_cast<size_t>(bitmapWidth * bitmapHeight));
    auto height = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, fontHeightPx, bitmap.data(), bitmapWidth, bitmapHeight, ' ', charsNum, std::data(bakedChars)
    );

    if (height < 0 || height > bitmapHeight) {
        throw FailedToLoadFontException::couldNotFitCharacters(fontFilePath, fontHeightPx, bitmapWidth, bitmapHeight);
    }

    using std::move;
    // If all successfull, replace current font data
    mFont.mBakedChars = move(bakedChars);
    mFont.mTextureId = textureId;
    mFont.mWidth = bitmapWidth;
    mFont.mHeight = height;
    mFont.mSize = fontHeightPx;

    // Return texture creation info
    FontData ret;
    ret.bitmap = bitmap;
    ret.width = bitmapWidth;
    ret.height = height;

    return ret;
}

float reig::Context::get_font_size() const {
    return mFont.mSize;
}

const char* reig::exception::NoRenderHandlerException::what() const noexcept {
    return "No render handler specified";
}

void reig::Context::render_all() {
    if (!mRenderHandler) {
        throw exception::NoRenderHandlerException{};
    }
    if (mCurrentWindow.mIsStarted) end_window();

    if (!mCurrentWindow.mDrawData.empty()) {
        mRenderHandler(mCurrentWindow.mDrawData, mUserPtr);
    }
    if (!mDrawData.empty()) {
        mRenderHandler(mDrawData, mUserPtr);
    }
}

void reig::Context::start_new_frame() {
    mCurrentWindow.mDrawData.clear();
    mDrawData.clear();

    mouse.leftButton.mIsClicked = false;
    mouse.mScrolled = 0.f;

    keyboard.reset();
}

void reig::Context::start_window(char const* aTitle, float& aX, float& aY) {
    if (mCurrentWindow.mIsStarted) end_window();

    mCurrentWindow.mIsStarted = true;
    mCurrentWindow.mTitle = aTitle;
    mCurrentWindow.mX = &aX;
    mCurrentWindow.mY = &aY;
    mCurrentWindow.mWidth = 0;
    mCurrentWindow.mHeight = 0;
    mCurrentWindow.mTitleBarHeight = 8 + mFont.mSize;
}

void reig::Context::end_window() {
    if (!mCurrentWindow.mIsStarted) return;
    mCurrentWindow.mIsStarted = false;

    mCurrentWindow.mWidth += 4;
    mCurrentWindow.mHeight += 4;

    Rectangle headerBox{
            *mCurrentWindow.mX, *mCurrentWindow.mY,
            mCurrentWindow.mWidth, mCurrentWindow.mTitleBarHeight
    };
    Triangle headerTriangle{
            *mCurrentWindow.mX + 3.f, *mCurrentWindow.mY + 3.f,
            *mCurrentWindow.mX + 3.f + mCurrentWindow.mTitleBarHeight, *mCurrentWindow.mY + 3.f,
            *mCurrentWindow.mX + 3.f + mCurrentWindow.mTitleBarHeight / 2.f,
            *mCurrentWindow.mY + mCurrentWindow.mTitleBarHeight - 3.f
    };
    Rectangle titleBox{
            *mCurrentWindow.mX + mCurrentWindow.mTitleBarHeight + 4, *mCurrentWindow.mY + 4,
            mCurrentWindow.mWidth - mCurrentWindow.mTitleBarHeight - 4, mCurrentWindow.mTitleBarHeight - 4
    };
    Rectangle bodyBox{
            *mCurrentWindow.mX, *mCurrentWindow.mY + mCurrentWindow.mTitleBarHeight,
            mCurrentWindow.mWidth, mCurrentWindow.mHeight - mCurrentWindow.mTitleBarHeight
    };

    using namespace colors::literals;
    using namespace colors::operators;

    render_rectangle(headerBox, colors::mediumGrey | 200_a);
    render_triangle(headerTriangle, colors::lightGrey);
    render_text(mCurrentWindow.mTitle, titleBox);
    render_rectangle(bodyBox, colors::mediumGrey | 100_a);

    if (mouse.leftButton.is_pressed() && internal::is_boxed_in(mouse.leftButton.get_clicked_pos(), headerBox)) {
        Point moved{
                mouse.get_cursor_pos().x - mouse.leftButton.get_clicked_pos().x,
                mouse.get_cursor_pos().y - mouse.leftButton.get_clicked_pos().y
        };

        *mCurrentWindow.mX += moved.x;
        *mCurrentWindow.mY += moved.y;
        mouse.leftButton.mClickedPos.x += moved.x;
        mouse.leftButton.mClickedPos.y += moved.y;
    }
}

void reig::detail::Window::fit_rect(Rectangle& rect) {
    if (mIsStarted) {
        rect.x += *mX + 4;
        rect.y += *mY + mTitleBarHeight + 4;

        if (*mX + mWidth < rect.x + rect.width) {
            mWidth = rect.x + rect.width - *mX;
        }
        if (*mY + mHeight < rect.y + rect.height) {
            mHeight = rect.y + rect.height - *mY;
        }
        if (rect.x < *mX) {
            rect.x = *mX + 4;
        }
        if (rect.y < *mY) {
            rect.y = *mY + 4;
        }
    }
}

void reig::Context::fit_rect_in_window(Rectangle& rect) {
    mCurrentWindow.fit_rect(rect);
}


float reig::Context::measure_text_width(const char* text) const {
    if (mFont.mBakedChars.empty() || !text) return 0.0f;

    float x = 0;
    float y = 0;

    float textWidth = 0.f;

    int from = ' ';
    int to = from + 95;
    int c;
    stbtt_aligned_quad q;

    for (; *text; ++text) {
        c = *text;
        if (c < from || c > to) c = to;
        stbtt_GetBakedQuad(
                mFont.mBakedChars.data(),
                mFont.mWidth, mFont.mHeight, c - from, &x, &y, &q, 1
        );
        textWidth += mFont.mBakedChars[c - ' '].xadvance;
    }

    return textWidth;
}


void reig::Context::render_text(char const* ch, Rectangle aBox) {
    if (mFont.mBakedChars.empty() || !ch) return;

    aBox = internal::decrease_rect(aBox, 8);
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
                mFont.mBakedChars.data(),
                mFont.mWidth, mFont.mHeight, c - ' ', &x, &y, &q, 1
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

        textWidth += mFont.mBakedChars[c - ' '].xadvance;

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
        fig.form(vertices, indices, mFont.mTextureId);
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
