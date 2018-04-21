#ifndef REIG_CONTEXT_H
#define REIG_CONTEXT_H

#include "defs.h"
#include "mouse.h"
#include "keyboard.h"
#include "text.h"
#include "config.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "stb_truetype.h"
#pragma GCC diagnostic pop
#include <vector>
#include <any>
#include <string>

namespace reig {
    using DrawData = std::vector<primitive::Figure>;
    using RenderHandler = void(*)(const DrawData&, std::any userPtr);

    namespace detail {
        struct Font {
            std::vector<stbtt_bakedchar> mBakedChars;
            float mHeight = 0.f;
            int mTextureId = 0;
            int mBitmapWidth = 0;
            int mBitmapHeight = 0;
        };

        struct Window {
            const char* mTitle = "";
            float* mX = nullptr;
            float* mY = nullptr;
            float mWidth = 0.f;
            float mHeight = 0.f;
            float mTitleBarHeight = 0.f;

            /**
             * Increase the window's width and height to fit rect's bottom right point
             * Shift rect's y down to accommodate window's title bar
             * Reset rect's position if it's top left corner can't be fitted
             * @param rect The rectangle to accommodate
             */
            void fit_rect(primitive::Rectangle& rect);
        };

        primitive::Rectangle as_rect(const Window& window);
    }

    /**
     * @class Context
     * @brief Used to pump in input and request gui creation
     */
    class Context {
    public:
        Context();

        explicit Context(const Config& config);

        void set_config(const Config& config);

        /**
         * @brief Set's a user function, which will draw the gui, based
         * @param handler A C function pointer to a rendering callback
         * The handler should return void and take in const DrawData& and void*
         */
        void set_render_handler(RenderHandler handler);

        void set_user_ptr(std::any userPtr);

        struct FontBitmap {
            std::vector<uint8_t> bitmap;
            int width = 0;
            int height = 0;
        };

        /**
         * @brief Sets reig's font to be used for labels
         * @param fontFilePath The path to fonts .ttf file.
         * @param textureId This id will be passed by reig to render_handler with text vertices
         * @param fontHeightPx Font's pixel size
         * @return Returns the bitmap, which is used to create a texture by user.
         * Set returned bitmap field to nullptr, to avoid deletion
         */
        FontBitmap set_font(const char* fontFilePath, int textureId, float fontHeightPx);

        float get_font_size() const;

        /**
         * @brief Resets draw data and inputs
         */
        void start_frame();

        unsigned get_frame_counter() const;

        /**
         * @brief Uses stored drawData and draws everything using the user handler
         */
        void end_frame();

        // Inputs
        detail::Mouse mouse;
        detail::Keyboard keyboard;

        // Widget renders
        void start_window(const char* title, float& x, float& y);

        void end_window();

        void fit_rect_in_window(primitive::Rectangle& rect);

        // Primitive renders
        /**
         * @brief Render some text
         * @param text Text to be displayed
         * @param rect Text's bounding box
         * @return x coordinate after printing
         */
        float render_text(const char* text, primitive::Rectangle rect, text::Alignment alignment = text::Alignment::CENTER, float scale = 1.f);

        /**
         * @brief Schedules a rectangle drawing
         * @param rect Position and size
         * @param color Color
         */
        void render_rectangle(const primitive::Rectangle& rect, const primitive::Color& color);

        /**
         * @brief Schedules a textured rectangle drawing (the texture is stretched)
         * @param rect Position and size
         * @param textureId Index to the texture
         */
        void render_rectangle(const primitive::Rectangle& rect, int textureId);

        /**
         * @brief Schedules a triangle drawing
         * @param triangle Position and size
         * @param color Color
         */
        void render_triangle(const primitive::Triangle& triangle, const primitive::Color& color);

    private:
        bool handle_window_focus(const char* window, bool claiming);

        void render_text_quads(const std::vector<stbtt_aligned_quad>& quads,
                               float horizontalAlignment, float verticalAlignment);

        void render_windows();

    private:
        friend reig::detail::Mouse;
        friend reig::detail::MouseButton;

        detail::Window* get_current_window();

        const char* mDraggedWindow = nullptr;
        std::vector<detail::Window> mPreviousWindows;
        std::vector<detail::Window> mQueuedWindows;
        std::vector<primitive::Figure> mDrawData;

        detail::Font mFont;
        Config mConfig;

        RenderHandler mRenderHandler = nullptr;
        std::any mUserPtr;
        unsigned mFrameCounter = 0;
    };
}

#endif //REIG_CONTEXT_H
