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

reig::Context::Context() : mConfig{Config::builder().build()} {}

reig::Context::Context(const reig::Config& config)
        : mConfig{config} {}

void reig::Context::set_config(const reig::Config& config) {
    mConfig = config;
}

void reig::Context::set_render_handler(RenderHandler renderHandler) {
    mRenderHandler = renderHandler;
}

void reig::Context::set_user_ptr(std::any ptr) {
    using std::move;
    mUserPtr = move(ptr);
}

const std::any& reig::Context::get_user_ptr() const {
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

reig::Context::FontBitmap reig::Context::set_font(char const* fontFilePath, int textureId, float fontHeightPx) {
    using exception::FailedToLoadFontException;

    if (textureId == 0) throw FailedToLoadFontException::noTextureId(fontFilePath);
    if (fontHeightPx <= 0) throw FailedToLoadFontException::invalidSize(fontFilePath, fontHeightPx);

    auto ttfBuffer = read_font_into_buffer(fontFilePath);

    // We want all ASCII chars from space to backspace
    int const charsNum = 96;
    int bitmapWidth = mConfig.mFontBitmapWidth;
    int bitmapHeight = mConfig.mFontBitmapHeight;

    using std::data;
    auto bakedChars = std::vector<stbtt_bakedchar>(charsNum);
    auto bitmap = vector<uint8_t>(internal::integral_cast<size_t>(bitmapWidth * bitmapHeight));

    int bakedHeight = stbtt_BakeFontBitmap(ttfBuffer.data(), 0, fontHeightPx, bitmap.data(),
                                           bitmapWidth, bitmapHeight, ' ', charsNum, data(bakedChars));
    if (bakedHeight < 0 || bakedHeight > bitmapHeight) {
        throw FailedToLoadFontException::couldNotFitCharacters(fontFilePath, fontHeightPx, bitmapWidth, bitmapHeight);
    } else {
        bitmapHeight = bakedHeight;
    }

    using std::move;
    // If all successful, replace current font data
    mFont.mBakedChars = move(bakedChars);
    mFont.mTextureId = textureId;
    mFont.mBitmapWidth = bitmapWidth;
    mFont.mBitmapHeight = bitmapHeight;
    mFont.mHeight = fontHeightPx;

    return FontBitmap{bitmap, bitmapWidth, bitmapHeight};
}

float reig::Context::get_font_size() const {
    return mFont.mHeight;
}

void reig::Context::render_all() {
    if (!mRenderHandler) {
        throw exception::NoRenderHandlerException{};
    }
    end_window();

    if (!mDrawData.empty()) {
        using std::move;
        auto widgetDrawData = move(mDrawData);
        mDrawData.clear();
        render_windows();
        mRenderHandler(mDrawData, mUserPtr);
        mRenderHandler(widgetDrawData, mUserPtr);
    }
}

void reig::Context::start_new_frame() {
    mWindows.clear();
    mDrawData.clear();

    mouse.leftButton.mIsClicked = false;
    mouse.mScrolled = 0.f;

    keyboard.reset();

    focus.reset_counter();

    ++mFrameCounter;
}

unsigned reig::Context::get_frame_counter() const {
    return mFrameCounter;
}

void reig::Context::start_window(char const* aTitle, float& aX, float& aY) {
    if (!mWindows.empty()) end_window();

    detail::Window currentWindow;

    currentWindow.mTitle = aTitle;
    currentWindow.mX = &aX;
    currentWindow.mY = &aY;
    currentWindow.mWidth = 0;
    currentWindow.mHeight = 0;
    currentWindow.mTitleBarHeight = 8 + mFont.mHeight;

    mWindows.push_back(currentWindow);
}

void reig::Context::render_windows() {
    for(const auto& currentWindow : mWindows) {
        Rectangle headerBox{
                *currentWindow.mX, *currentWindow.mY,
                currentWindow.mWidth, currentWindow.mTitleBarHeight
        };
        Triangle headerTriangle{
                *currentWindow.mX + 3.f, *currentWindow.mY + 3.f,
                *currentWindow.mX + 3.f + currentWindow.mTitleBarHeight, *currentWindow.mY + 3.f,
                *currentWindow.mX + 3.f + currentWindow.mTitleBarHeight / 2.f,
                *currentWindow.mY + currentWindow.mTitleBarHeight - 3.f
        };
        Rectangle titleBox{
                *currentWindow.mX + currentWindow.mTitleBarHeight + 4, *currentWindow.mY + 4,
                currentWindow.mWidth - currentWindow.mTitleBarHeight - 4, currentWindow.mTitleBarHeight - 4
        };
        Rectangle bodyBox{
                *currentWindow.mX, *currentWindow.mY + currentWindow.mTitleBarHeight,
                currentWindow.mWidth, currentWindow.mHeight - currentWindow.mTitleBarHeight
        };

        if (mConfig.mWindowsTextured) {
            render_rectangle(headerBox, mConfig.mTitleBackgroundTexture);
        } else {
            render_rectangle(headerBox, mConfig.mTitleBackgroundColor);
        }
        render_triangle(headerTriangle, colors::lightGrey);
        render_text(currentWindow.mTitle, titleBox);
        if (mConfig.mWindowsTextured) {
            render_rectangle(bodyBox, mConfig.mWindowBackgroundTexture);
        } else {
            render_rectangle(bodyBox, mConfig.mWindowBackgroundColor);
        }
    }
}

void reig::Context::end_window() {
    if (mWindows.empty()) return;

    detail::Window currentWindow = mWindows.back();

    currentWindow.mWidth += 4;
    currentWindow.mHeight += 4;

    Rectangle headerBox{
            *currentWindow.mX, *currentWindow.mY,
            currentWindow.mWidth, currentWindow.mTitleBarHeight
    };

    if (mouse.leftButton.is_pressed()
        && internal::is_boxed_in(mouse.leftButton.get_clicked_pos(), headerBox)
        && focus.claim_for_window(currentWindow.mTitle)) {
        Point moved{
                mouse.get_cursor_pos().x - mouse.leftButton.get_clicked_pos().x,
                mouse.get_cursor_pos().y - mouse.leftButton.get_clicked_pos().y
        };

        *currentWindow.mX += moved.x;
        *currentWindow.mY += moved.y;
        mouse.leftButton.mClickedPos.x += moved.x;
        mouse.leftButton.mClickedPos.y += moved.y;
    } else {
        focus.release_from_window(currentWindow.mTitle);
    }
}

void reig::detail::Window::fit_rect(Rectangle& rect) {
    rect.x += *mX + 4;
    rect.y += *mY + mTitleBarHeight + 4;

    if (*mX + mWidth < get_x2(rect)) {
        mWidth = get_x2(rect) - *mX;
    }
    if (*mY + mHeight < get_y2(rect)) {
        mHeight = get_y2(rect) - *mY;
    }
    if (rect.x < *mX) {
        rect.x = *mX + 4;
    }
    if (rect.y < *mY) {
        rect.y = *mY + 4;
    }
}

void reig::Context::fit_rect_in_window(Rectangle& rect) {
    if (!mWindows.empty()) {
        mWindows.back().fit_rect(rect);
    }
}

bool has_alignment(reig::text::Alignment container, reig::text::Alignment alignment) {
    auto containerU = static_cast<unsigned>(container);
    auto alignmentU = static_cast<unsigned>(alignment);
    return (alignmentU & containerU) == alignmentU;
}

inline stbtt_aligned_quad get_char_quad(int charIndex, float& x, float& y, const reig::detail::Font& font) {
    stbtt_aligned_quad quad;
    stbtt_GetBakedQuad(data(font.mBakedChars), font.mBitmapWidth, font.mBitmapHeight, charIndex, &x, &y, &quad, true);
    return quad;
}

inline Point scale_quad(stbtt_aligned_quad& quad, float scale, float x, float previousX) {
    float antiScale = 1.0f - scale;

    float scalingHorizontalOffset = (x - previousX) * antiScale;
    quad.x1 -= scalingHorizontalOffset;
    float scalingVerticalOffset = (quad.y1 - quad.y0) * antiScale;
    quad.y0 += scalingVerticalOffset;

    return Point{scalingHorizontalOffset, scalingVerticalOffset};
}

float reig::Context::render_text(char const* text, const Rectangle rect, text::Alignment alignment, float scale) {
    if (mFont.mBakedChars.empty() || !text) return rect.x;

    float x = rect.x;
    float y = rect.y + rect.height;

    float minY = y;
    float maxY = y;

    vector<stbtt_aligned_quad> quads;
    quads.reserve(20);

    int fromChar = ' ';
    int toChar = fromChar + 95;
    int fallbackChar = toChar; // The backspace character
    for (int ch = *text; ch != '\0'; ch = *++text) {
        if (!(ch >= fromChar && ch <= toChar)) {
            ch = fallbackChar;
        }

        float previousX = x;
        auto quad = get_char_quad(ch - fromChar, x, y, mFont);

        auto scalingOffsets = scale_quad(quad, scale, x, previousX);
        x -= scalingOffsets.x;

        if (quad.x0 > get_x2(rect)) {
            break;
        }
        quad.x1 = internal::min(quad.x1, get_x2(rect));
        quad.y0 = internal::max(quad.y0, rect.y);
        quad.y1 = internal::min(quad.y1, get_y2(rect));

        minY = internal::min(minY, quad.y0);
        maxY = internal::max(maxY, quad.y1);

        quads.push_back(quad);
    }

    float textHeight = maxY - minY;
    float textWidth = 0.0f;
    if (!quads.empty()) {
        textWidth = quads.back().x1 - quads.front().x0;
    }

    float horizontalAlignment =
            has_alignment(alignment, text::Alignment::RIGHT) ? rect.width - textWidth :
            has_alignment(alignment, text::Alignment::LEFT) ? 0.0f :
            (rect.width - textWidth) * 0.5f;
    float verticalAlignment =
            has_alignment(alignment, text::Alignment::TOP) ? -(rect.height - textHeight) :
            has_alignment(alignment, text::Alignment::BOTTOM) ? 0.0f :
            (rect.height - textHeight) * -0.5f;

    for (auto& q : quads) {
        vector<Vertex> vertices{
                {{q.x0 + horizontalAlignment, q.y0 + verticalAlignment}, {q.s0, q.t0}, {}},
                {{q.x1 + horizontalAlignment, q.y0 + verticalAlignment}, {q.s1, q.t0}, {}},
                {{q.x1 + horizontalAlignment, q.y1 + verticalAlignment}, {q.s1, q.t1}, {}},
                {{q.x0 + horizontalAlignment, q.y1 + verticalAlignment}, {q.s0, q.t1}, {}}
        };
        vector<int> indices{0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices, mFont.mTextureId);
        mDrawData.push_back(fig);
    }

    return x;
}

void reig::Context::render_triangle(const Triangle& aTri, const Color& aColor) {
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

void reig::Context::render_rectangle(const Rectangle& rect, int textureId) {
    vector<Vertex> vertices{
            {{rect.x,       rect.y},       {0.f, 0.f}, {}},
            {{get_x2(rect), rect.y},       {1.f, 0.f}, {}},
            {{get_x2(rect), get_y2(rect)}, {1.f, 1.f}, {}},
            {{rect.x,       get_y2(rect)}, {0.f, 1.f}, {}}
    };
    vector<int> indices{0, 1, 2, 2, 3, 0};

    Figure fig;
    fig.form(vertices, indices, textureId);
    mDrawData.push_back(fig);
}

void reig::Context::render_rectangle(const Rectangle& rect, const Color& color) {
    vector<Vertex> vertices{
            {{rect.x,       rect.y},       {}, color},
            {{get_x2(rect), rect.y},       {}, color},
            {{get_x2(rect), get_y2(rect)}, {}, color},
            {{rect.x,       get_y2(rect)}, {}, color}
    };
    vector<int> indices{0, 1, 2, 2, 3, 0};

    Figure fig;
    fig.form(vertices, indices);
    mDrawData.push_back(fig);
}
