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

    using std::data;
    auto bakedChars = std::vector<stbtt_bakedchar>(charsNum);
    auto bitmap = vector<uint8_t>(internal::integral_cast<size_t>(bitmapWidth * bitmapHeight));
    auto height = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, fontHeightPx,
            bitmap.data(), bitmapWidth, bitmapHeight, ' ', charsNum, data(bakedChars)
    );

    if (height < 0 || height > bitmapHeight) {
        throw FailedToLoadFontException::couldNotFitCharacters(fontFilePath, fontHeightPx, bitmapWidth, bitmapHeight);
    }

    using std::move;
    // If all successfull, replace current font data
    mFont.mBakedChars = move(bakedChars);
    mFont.mTextureId = textureId;
    mFont.mBitmapWidth = bitmapWidth;
    mFont.mBitmapHeight = height;
    mFont.mHeight = fontHeightPx;

    // Return texture creation info
    FontData ret;
    ret.bitmap = bitmap;
    ret.width = bitmapWidth;
    ret.height = height;

    return ret;
}

float reig::Context::get_font_size() const {
    return mFont.mHeight;
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

    focus.reset_counter();

    ++mFrameCounter;
}

unsigned reig::Context::get_frame_counter() const {
    return mFrameCounter;
}

void reig::Context::start_window(char const* aTitle, float& aX, float& aY) {
    if (mCurrentWindow.mIsStarted) end_window();

    mCurrentWindow.mIsStarted = true;
    mCurrentWindow.mTitle = aTitle;
    mCurrentWindow.mX = &aX;
    mCurrentWindow.mY = &aY;
    mCurrentWindow.mWidth = 0;
    mCurrentWindow.mHeight = 0;
    mCurrentWindow.mTitleBarHeight = 8 + mFont.mHeight;
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

    if (mouse.leftButton.is_pressed()
        && internal::is_boxed_in(mouse.leftButton.get_clicked_pos(), headerBox)
        && focus.claim_for_window(mCurrentWindow.mTitle)) {
        Point moved{
                mouse.get_cursor_pos().x - mouse.leftButton.get_clicked_pos().x,
                mouse.get_cursor_pos().y - mouse.leftButton.get_clicked_pos().y
        };

        *mCurrentWindow.mX += moved.x;
        *mCurrentWindow.mY += moved.y;
        mouse.leftButton.mClickedPos.x += moved.x;
        mouse.leftButton.mClickedPos.y += moved.y;
    } else {
        focus.release_from_window(mCurrentWindow.mTitle);
    }
}

void reig::detail::Window::fit_rect(Rectangle& rect) {
    if (mIsStarted) {
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
}

void reig::Context::fit_rect_in_window(Rectangle& rect) {
    mCurrentWindow.fit_rect(rect);
}

bool has_alignment(reig::text::Alignment container, reig::text::Alignment alignment) {
    auto containerU = static_cast<unsigned>(container);
    auto alignmentU = static_cast<unsigned>(alignment);
    return (alignmentU & containerU) == alignmentU;
}

float reig::Context::render_text(char const* text, Rectangle rect, text::Alignment alignment) {
    if (mFont.mBakedChars.empty() || !text) return rect.x;

    float x = rect.x;
    float y = rect.y + rect.height;

    vector<stbtt_aligned_quad> quads;
    quads.reserve(20);

    stbtt_aligned_quad quad;
    int from = ' ';
    int to = from + 95; // The empty box character
    for (int ch = *text; *text; ch = *++text) {
        if (ch < from || ch > to) ch = to;

        float previousX = x;
        stbtt_GetBakedQuad(
                data(mFont.mBakedChars), mFont.mBitmapWidth, mFont.mBitmapHeight,
                ch - from, &x, &y, &quad, true
        );

        float scalingHorizontalOffset = (x - previousX) * (1 - scale);
        x -= scalingHorizontalOffset;
        quad.x1 = x;
        quad.y0 += (quad.y1 - quad.y0) * (1 - scale);

        if (quad.x0 > get_x2(rect)) {
            break;
        }
        quad.x1 = internal::min(quad.x1, get_x2(rect));
        quad.y0 = internal::max(quad.y0, rect.y);
        quad.y1 = internal::min(quad.y1, get_y2(rect));

        quads.push_back(quad);
    }

    float textWidth = 0.0f;
    if (!quads.empty()) {
        textWidth = quads.back().x1 - quads.front().x0;
    }

    float horizontalAlignment =
            has_alignment(alignment, text::Alignment::RIGHT) ? rect.width - textWidth :
            has_alignment(alignment, text::Alignment::LEFT) ? 0.0f :
            (rect.width - textWidth) * 0.5f;
    float verticalAlignment =
            has_alignment(alignment, text::Alignment::TOP) ? -(rect.height - fontHeight) :
            has_alignment(alignment, text::Alignment::BOTTOM) ? 0.0f :
            (rect.height - fontHeight) * -0.5f;

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

void reig::Context::render_rectangle(Rectangle const& rect, int textureId) {
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

void reig::Context::render_rectangle(Rectangle const& rect, Color const& color) {
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
