#ifndef REIG_CONFIG_H
#define REIG_CONFIG_H

#include "context_fwd.h"
#include "primitive.h"

namespace reig {
    enum class FillMode {
        kColored,
        kTextured,
    };

    class Config {
    public:
        class Builder;

        explicit Config(const Builder& builder);

        /**
         * @return A value explaining whether the windows are textured or colored
         */
        FillMode fill_mode() const;

        /**
         * @return The color used for window backgrounds
         * @throws std::logic_error if the windows are textured
         */
        const primitive::Color& window_bg_color() const;

        /**
         * @return The color used for window title backgrounds
         * @throws std::logic_error if the windows are textured
         */
        const primitive::Color& title_bar_bg_color() const;

        /**
         * @return The texture identifier used for window backgrounds
         * @throws std::logic_error if the windows are colored
         */
        int window_bg_texture_id() const;

        /**
         * @return The texture identifier used for window title backgrounds
         * @throws std::logic_error if the windows are colored
         */
        int title_bar_bg_texture_id() const;

        int font_bitmap_width() const;

        int font_bitmap_height() const;

        class Builder {
        public:
            Builder();

            Config build();

            Builder& set_window_colors(const primitive::Color& title_background,
                                       const primitive::Color& window_background);

            Builder& set_window_textures(int title_texture, int background_texture);

            Builder& set_font_bitmap_size(int width, int height);

            const primitive::Color& window_bg_color() const;

            const primitive::Color& title_bar_bg_color() const;

            FillMode fill_mode() const;

            int window_bg_texture_id() const;

            int title_bar_bg_texture_id() const;

            int font_bitmap_width() const;

            int font_bitmap_height() const;

        private:
            FillMode _fill_mode = FillMode::kColored;
            int _window_bg_texture_id = 0;
            int _title_bar_bg_texture_id = 0;
            primitive::Color _window_bg_color;
            primitive::Color _title_bar_bg_color;
            int _font_bitmap_width = 512;
            int _font_bitmap_height = 512;
        };

    private:
        void throw_if_not_textured() const;

        void throw_if_not_colored() const;

        FillMode _fill_mode;
        int _window_bg_texture_id;
        int _title_bar_bg_texture_id;
        primitive::Color _window_bg_color;
        primitive::Color _title_bar_bg_color;
        int _font_bitmap_width;
        int _font_bitmap_height;
    };
}

#endif //REIG_CONFIG_H
