#ifndef REIG_CONFIG_H
#define REIG_CONFIG_H

#include "context_fwd.h"
#include "primitive.h"

namespace reig {
    class ConfigBuilder;

    class Config {
    public:
        explicit Config(const ConfigBuilder& builder);

        static ConfigBuilder builder();

        const primitive::Color& window_bg_color() const;

        const primitive::Color& title_bar_bg_color() const;

        bool are_windows_textured() const;

        int window_bg_texture_id() const;

        int title_bar_bg_texture_id() const;

        int font_bitmap_width() const;

        int font_bitmap_height() const;

    private:
        primitive::Color _window_bg_color;
        primitive::Color _title_bar_bg_color;
        bool _are_windows_textured;
        int _window_bg_texture_id;
        int _title_bar_bg_texture_id;
        int _font_bitmap_width;
        int _font_bitmap_height;
    };

    class ConfigBuilder {
    public:
        ConfigBuilder();

        ConfigBuilder& window_colors(const primitive::Color& title_background,
                                     const primitive::Color& window_background);

        ConfigBuilder& window_textures(int title_texture, int background_texture);

        ConfigBuilder& font_bitmap_size(int width, int height);

        const primitive::Color& window_bg_color() const;

        const primitive::Color& title_bar_bg_color() const;

        bool are_windows_textured() const;

        int window_bg_texture_id() const;

        int title_bar_bg_texture_id() const;

        int font_bitmap_width() const;

        int font_bitmap_height() const;

        Config build();

    private:
        primitive::Color _window_bg_color = primitive::colors::kTransparent;
        primitive::Color _title_bar_bg_color = primitive::colors::kWhite;
        bool _are_windows_textured = false;
        int _window_bg_texture_id = 0;
        int _title_bar_bg_texture_id = 0;
        int _font_bitmap_width = 512;
        int _font_bitmap_height = 512;
    };
}

#endif //REIG_CONFIG_H
