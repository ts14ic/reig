#ifndef REIG_CONTEXT_H
#define REIG_CONTEXT_H

#include "defs.h"
#include "mouse.h"
#include "stb_truetype.h"
#include <vector>
#include <any>

namespace reig {
    namespace detail {
        struct Font {
            std::vector<stbtt_bakedchar> mBakedChars;
            float mSize = 0.f;
            int mTextureId = 0;
            int mWidth = 0;
            int mHeight = 0;
        };

        struct Window {
            std::vector<primitive::Figure> mDrawData;
            char const* mTitle = nullptr;
            float* mX = nullptr;
            float* mY = nullptr;
            float mWidth = 0.f;
            float mHeight = 0.f;
            float mTitleBarHeight = 0.f;
            bool mIsStarted = false;

            /**
             * Increase the window's width and height to fit rect's bottom right point
             * Shift rect's y down to accommodate window's title bar
             * Reset rect's position if it's top left corner can't be fitted
             * @param rect The rectangle to accommodate
             */
            void fit_rect(primitive::Rectangle& rect);
        };
    }

    /**
     * @class Context
     * @brief Used to pump in input and request gui creation
     */
    class Context {
    public:
        Context() = default;

        using DrawData = std::vector<primitive::Figure>;
        using RenderHandler = void (*)(DrawData const&, std::any&);

        /**
         * @brief Set's a user function, which will draw the gui, based
         * @param handler A C function pointer to a rendering callback
         * The handler should return void and take in DrawData const& and void*
         */
        void set_render_handler(RenderHandler handler);

        /**
         * @brief Set a user pointer to store in the context.
         * @param ptr A user data pointer.
         * The pointer is then automatically passed to callbacks.
         */
        void set_user_ptr(std::any ptr);

        /**
         * @brief Gets the stored user pointer
         */
        std::any const& get_user_ptr() const;

        struct FontData {
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
        FontData set_font(char const* fontFilePath, int textureId, float fontHeightPx);

        float get_font_size() const;

        /**
         * @brief Resets draw data and inputs
         */
        void start_new_frame();

        /**
         * @brief Uses stored drawData and draws everything using the user handler
         */
        void render_all();

        // Inputs
        detail::Mouse mouse;

        // Widget renders
        void start_window(char const* title, float& x, float& y);

        void end_window();

        void fit_rect_in_window(primitive::Rectangle& rect);

        // Primitive renders
        /**
         * @brief Render some text
         * @param text Text to be displayed
         * @param box Text's bounding box
         */
        void render_text(char const* text, primitive::Rectangle box);

        /**
         * @brief Schedules a rectangle drawing
         * @param rect Position and size
         * @param color Color
         */
        void render_rectangle(primitive::Rectangle const& rect, primitive::Color const& color);

        /**
         * @brief Schedules a textured rectangle drawing (the texture is stretched)
         * @param rect Position and size
         * @param texture Index to the texture
         */
        void render_rectangle(primitive::Rectangle const& rect, int texture);

        /**
         * @brief Schedules a triangle drawing
         * @param triangle Position and size
         * @param color Color
         */
        void render_triangle(primitive::Triangle const& triangle, primitive::Color const& color);

    private:
        std::vector<primitive::Figure> mDrawData;
        detail::Font mFont;
        detail::Window mCurrentWindow;

        RenderHandler mRenderHandler = nullptr;
        std::any mUserPtr;
    };
}

#endif //REIG_CONTEXT_H
