#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION

#include "context.h"
#include "internal.h"
#include "exception.h"
#include <memory>
#include <algorithm>

using namespace reig::primitive;
using std::unique_ptr;
using std::reference_wrapper;
using std::vector;

namespace reig {
    Context::Context() : Context{Config::builder().build()} {}

    Context::Context(const Config& config)
            : mouse{*this}, mConfig{config} {}

    void Context::set_config(const Config& config) {
        mConfig = config;
    }

    void Context::set_render_handler(RenderHandler renderHandler) {
        mRenderHandler = renderHandler;
    }

    void Context::set_user_ptr(std::any userPtr) {
        using std::move;
        mUserPtr = move(userPtr);
    }

    vector<uint8_t> read_font_into_buffer(const char* fontFilePath) {
        using exception::FailedToLoadFontException;

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

    Context::FontBitmap Context::set_font(const char* fontFilePath, int textureId, float fontHeightPx) {
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

    float Context::get_font_size() const {
        return mFont.mHeight;
    }

    void Context::end_frame() {
        if (!mRenderHandler) {
            throw exception::NoRenderHandlerException{};
        }
        end_window();

        mRenderHandler(mFreeDrawData, mUserPtr);
        render_windows();

        //{{{ persist previous windows
        mPreviousWindows = move(mQueuedWindows);
        mQueuedWindows.clear();
        std::reverse(begin(mPreviousWindows), end(mPreviousWindows));
        //}}}
    }

    bool Context::handle_window_focus(const char* window, bool claiming) {
        if (claiming) {
            if (!mDraggedWindow) {
                mDraggedWindow = window;
            }
            return mDraggedWindow == window;
        } else {
            if (mDraggedWindow == window) {
                mDraggedWindow = nullptr;
            }
            return mDraggedWindow != window;
        }
    }

    void Context::start_frame() {
        mQueuedWindows.clear();
        mFreeDrawData.clear();

        mouse.leftButton.mIsClicked = false;
        mouse.mScrolled = 0.f;

        keyboard.reset();

        ++mFrameCounter;
    }

    unsigned Context::get_frame_counter() const {
        return mFrameCounter;
    }

    detail::Window* Context::get_current_window() {
        if (!mQueuedWindows.empty() && !mQueuedWindows.back().isFinished) {
            return &mQueuedWindows.back();
        } else {
            return nullptr;
        }
    }

    void Context::start_window(const char* aTitle, float& aX, float& aY) {
        if (!mQueuedWindows.empty()) end_window();

        mQueuedWindows.emplace_back(aTitle, aX, aY, 0, 0, mFont.mHeight + 8);
    }

    void Context::render_windows() {
        for(auto& currentWindow : mQueuedWindows) {
            auto currentWidgetData = move(currentWindow.drawData);
            currentWindow.drawData.clear();

            Rectangle headerBox{
                    *currentWindow.x, *currentWindow.y,
                    currentWindow.width, currentWindow.titleBarHeight
            };
            Triangle headerTriangle{
                    {*currentWindow.x + 3.f, *currentWindow.y + 3.f},
                    {*currentWindow.x + 3.f + currentWindow.titleBarHeight, *currentWindow.y + 3.f},
                    {*currentWindow.x + 3.f + currentWindow.titleBarHeight / 2.f,
                     *currentWindow.y + currentWindow.titleBarHeight - 3.f}
            };
            Rectangle titleBox{
                    *currentWindow.x + currentWindow.titleBarHeight + 4, *currentWindow.y + 4,
                    currentWindow.width - currentWindow.titleBarHeight - 4, currentWindow.titleBarHeight - 4
            };
            Rectangle bodyBox{
                    *currentWindow.x, *currentWindow.y + currentWindow.titleBarHeight,
                    currentWindow.width, currentWindow.height - currentWindow.titleBarHeight
            };

            if (mConfig.mWindowsTextured) {
                render_rectangle(currentWindow.drawData, headerBox, mConfig.mTitleBackgroundTexture);
            } else {
                render_rectangle(currentWindow.drawData, headerBox, mConfig.mTitleBackgroundColor);
            }
            render_triangle(currentWindow.drawData, headerTriangle, colors::lightGrey);
            render_text(currentWindow.drawData, currentWindow.title, titleBox);
            if (mConfig.mWindowsTextured) {
                render_rectangle(currentWindow.drawData, bodyBox, mConfig.mWindowBackgroundTexture);
            } else {
                render_rectangle(currentWindow.drawData, bodyBox, mConfig.mWindowBackgroundColor);
            }

            mRenderHandler(currentWindow.drawData, mUserPtr);
            currentWindow.drawData.clear();

            mRenderHandler(currentWidgetData, mUserPtr);
        }
    }

    void Context::end_window() {
        if (mQueuedWindows.empty()) return;

        auto& currentWindow = *get_current_window();

        currentWindow.isFinished = true;
        currentWindow.width += 4;
        currentWindow.height += 4;

        handle_window_input(currentWindow);
    }

    void Context::handle_window_input(detail::Window& window) {
        Rectangle headerBox{
                *window.x, *window.y,
                window.width, window.titleBarHeight
        };

        if (mouse.leftButton.is_held()
            && internal::is_boxed_in(mouse.leftButton.get_clicked_pos(), headerBox)
            && handle_window_focus(window.title, true)) {
            Point moved{
                    mouse.get_cursor_pos().x - mouse.leftButton.get_clicked_pos().x,
                    mouse.get_cursor_pos().y - mouse.leftButton.get_clicked_pos().y
            };

            *window.x += moved.x;
            *window.y += moved.y;
            mouse.leftButton.mClickedPos.x += moved.x;
            mouse.leftButton.mClickedPos.y += moved.y;
        } else {
            handle_window_focus(window.title, false);
        }
    }

    void detail::fit_rect_in_window(reig::primitive::Rectangle& rect, reig::detail::Window& window) {
        rect.x += *window.x + 4;
        rect.y += *window.y + window.titleBarHeight + 4;

        if (*window.x + window.width < get_x2(rect)) {
            window.width = get_x2(rect) - *window.x;
        }
        if (*window.y + window.height < get_y2(rect)) {
            window.height = get_y2(rect) - *window.y;
        }
        if (rect.x < *window.x) {
            rect.x = *window.x + 4;
        }
        if (rect.y < *window.y) {
            rect.y = *window.y + 4;
        }
    }

    primitive::Rectangle detail::as_rect(const Window& window) {
        return {*window.x, *window.y, window.width, window.height};
    }

    void Context::fit_rect_in_window(Rectangle& rect) {
        if (!mQueuedWindows.empty()) {
            detail::fit_rect_in_window(rect, mQueuedWindows.back());
        }
    }

    bool has_alignment(text::Alignment container, text::Alignment alignment) {
        auto containerU = static_cast<unsigned>(container);
        auto alignmentU = static_cast<unsigned>(alignment);
        return (alignmentU & containerU) == alignmentU;
    }

    inline stbtt_aligned_quad get_char_quad(int charIndex, float& x, float& y, const detail::Font& font) {
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

    float Context::render_text(const char* text, const Rectangle rect, text::Alignment alignment, float scale) {
        return render_text(get_current_draw_data_buffer(), text, rect, alignment, scale);
    }

    float Context::render_text(DrawData& drawData, const char* text, Rectangle rect, text::Alignment alignment,
                               float scale) {
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
//            quad.y0 = internal::max(quad.y0, rect.y);
//            quad.y1 = internal::min(quad.y1, get_y2(rect));

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

        render_text_quads(drawData, quads, horizontalAlignment, verticalAlignment, mFont.mTextureId);

        return x;
    }

    DrawData& Context::get_current_draw_data_buffer() {
        auto* currentWindow = get_current_window();
        return currentWindow ? currentWindow->drawData : mFreeDrawData;
    }

    void Context::render_rectangle(const Rectangle& rect, const Color& color) {
        render_rectangle(get_current_draw_data_buffer(), rect, color);
    }

    void Context::render_rectangle(const Rectangle& rect, int textureId) {
        render_rectangle(get_current_draw_data_buffer(), rect, textureId);
    }

    void Context::render_triangle(const Triangle& triangle, const Color& color) {
        render_triangle(get_current_draw_data_buffer(), triangle, color);
    }

    void Context::render_rectangle(DrawData& drawData, const Rectangle& rect, const Color& color) {
        vector<Vertex> vertices{
                {{rect.x,       rect.y},       {}, color},
                {{get_x2(rect), rect.y},       {}, color},
                {{get_x2(rect), get_y2(rect)}, {}, color},
                {{rect.x,       get_y2(rect)}, {}, color}
        };
        vector<int> indices{0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices);
        drawData.push_back(fig);
    }

    void Context::render_rectangle(DrawData& drawData, const Rectangle& rect, int textureId) {
        vector<Vertex> vertices{
                {{rect.x,       rect.y},       {0.f, 0.f}, {}},
                {{get_x2(rect), rect.y},       {1.f, 0.f}, {}},
                {{get_x2(rect), get_y2(rect)}, {1.f, 1.f}, {}},
                {{rect.x,       get_y2(rect)}, {0.f, 1.f}, {}}
        };
        vector<int> indices{0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices, textureId);
        drawData.push_back(fig);
    }

    void Context::render_triangle(DrawData& drawData, const Triangle& triangle, const Color& color) {
        vector<Vertex> vertices{
                {{triangle.pos0}, {}, color},
                {{triangle.pos1}, {}, color},
                {{triangle.pos2}, {}, color}
        };
        vector<int> indices = {0, 1, 2};

        Figure fig;
        fig.form(vertices, indices);
        drawData.push_back(fig);
    }

    void Context::render_text_quads(DrawData& drawData, const std::vector<stbtt_aligned_quad>& quads,
                                    float horizontalAlignment, float verticalAlignment, int fontTextureId) {
        for (auto& q : quads) {
            vector<Vertex> vertices{
                    {{q.x0 + horizontalAlignment, q.y0 + verticalAlignment}, {q.s0, q.t0}, {}},
                    {{q.x1 + horizontalAlignment, q.y0 + verticalAlignment}, {q.s1, q.t0}, {}},
                    {{q.x1 + horizontalAlignment, q.y1 + verticalAlignment}, {q.s1, q.t1}, {}},
                    {{q.x0 + horizontalAlignment, q.y1 + verticalAlignment}, {q.s0, q.t1}, {}}
            };
            vector<int> indices{0, 1, 2, 2, 3, 0};

            Figure fig;
            fig.form(vertices, indices, fontTextureId);
            drawData.push_back(fig);
        }
    }
}