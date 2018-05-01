#ifndef REIG_CONTEXT_H
#define REIG_CONTEXT_H

#include "context_fwd.h"
#include "window.h"
#include "mouse.h"
#include "keyboard.h"
#include "text.h"
#include "config.h"
#include "gsl.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "stb_truetype.h"
#pragma GCC diagnostic pop
#include <vector>
#include <any>
#include <string>

namespace reig {
    using RenderHandler = void(*)(const DrawData&, std::any userPtr);

    namespace detail {
        struct Font {
            std::vector<stbtt_bakedchar> baked_chars;
            float height = 0.f;
            int texture_id = 0;
            int bitmap_width = 0;
            int bitmap_height = 0;
        };
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
         * @param render_handler A C function pointer to a rendering callback
         * The handler should return void and take in const DrawData& and void*
         */
        void set_render_handler(RenderHandler render_handler);

        void set_user_ptr(std::any user_ptr);

        struct FontBitmap {
            std::vector<uint8_t> bitmap;
            int width = 0;
            int height = 0;
        };

        /**
         * @brief Sets reig's font to be used for labels
         * @param font_file_path The path to fonts .ttf file.
         * @param texture_id This id will be passed by reig to render_handler with text vertices
         * @param font_height_in_px Font's pixel size
         * @return Returns the bitmap, which is used to create a texture by user.
         * Set returned bitmap field to nullptr, to avoid deletion
         */
        FontBitmap set_font(gsl::czstring font_file_path, int texture_id, float font_height_in_px);

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
        /**
         * Starts a new window, with the given title and starting coordinates.
         * The windows is given an id, that is equals to the title
         */
        void start_window(gsl::czstring title, float default_x, float default_y);

        /**
         * Same as start_window without the id parameter.
         *
         * Starts a new window, with the given id, title and starting coordinates.
         * Use this one with a constant id, if you know the window title can change
         */
        void start_window(gsl::czstring id, gsl::czstring title, float default_x, float default_y);

        void end_window();

        void fit_rect_in_window(primitive::Rectangle& rect);

        // Primitive renders
        /**
         * @brief Render some text
         * @param text Text to be displayed
         * @param rect Text's bounding box
         * @return x coordinate after printing
         */
        float render_text(gsl::czstring text, primitive::Rectangle rect, text::Alignment alignment = text::Alignment::kCenter, float scale = 1.f);

        /**
         * @brief Schedules a rectangle drawing
         * @param rect Position and size
         * @param color Color
         */
        void render_rectangle(const primitive::Rectangle& rect, const primitive::Color& color);

        /**
         * @brief Schedules a textured rectangle drawing (the texture is stretched)
         * @param rect Position and size
         * @param texture_id Index to the texture
         */
        void render_rectangle(const primitive::Rectangle& rect, int texture_id);

        /**
         * @brief Schedules a triangle drawing
         * @param triangle Position and size
         * @param color Color
         */
        void render_triangle(const primitive::Triangle& triangle, const primitive::Color& color);

    private:
        DrawData& get_current_draw_data_buffer();

        float render_text(DrawData& draw_data, gsl::czstring text, primitive::Rectangle rect,
                          text::Alignment alignment = text::Alignment::kCenter, float scale = 1.0f);

        static void render_rectangle(DrawData& draw_data, const primitive::Rectangle& rect,
                                     const primitive::Color& color);

        static void render_rectangle(DrawData& draw_data, const primitive::Rectangle& rect, int texture_id);

        static void render_triangle(DrawData& draw_data, const primitive::Triangle& triangle,
                                    const primitive::Color& color);

        static void render_text_quads(DrawData& draw_data, const std::vector<stbtt_aligned_quad>& quads,
                                      float horizontal_alignment, float vertical_alignment, int font_texture_id);

        void render_windows();

        void update_window_layers();

        void remove_unqueued_windows();

        bool handle_window_focus(const detail::Window& window, bool is_claiming);

        void handle_window_input(detail::Window& window);

        bool is_window_header_point_visible(const detail::Window& window, const primitive::Point& point);

        bool is_window_body_point_visible(const primitive::Point& point);

        friend ::reig::detail::Mouse;
        friend ::reig::detail::MouseButton;

        gsl::czstring _dragged_window = nullptr;
        detail::Window* _queued_window = nullptr;
        std::vector<detail::Window> _windows;
        DrawData _free_draw_data;

        detail::Font _font;
        Config _config;

        RenderHandler _render_handler = nullptr;
        std::any _user_ptr;
        unsigned _frame_counter = 0;
    };
}

#endif //REIG_CONTEXT_H
