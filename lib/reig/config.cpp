#include "config.h"

namespace reig {
    Config::Config() = default;

    const primitive::Color& Config::window_bg_color() const {
        throw_if_not_colored();
        return _window_bg_color;
    }

    void Config::throw_if_not_colored() const {
        if (_fill_mode != FillMode::kColored) {
            throw std::logic_error{"invalid state: is textured"};
        }
    }

    const primitive::Color& Config::title_bar_bg_color() const {
        throw_if_not_colored();
        return _title_bar_bg_color;
    }

    FillMode Config::fill_mode() const {
        return _fill_mode;
    }

    int Config::window_bg_texture_id() const {
        throw_if_not_textured();
        return _window_bg_texture_id;
    }

    void Config::throw_if_not_textured() const {
        if (_fill_mode != FillMode::kTextured) {
            throw std::logic_error{"invalid state: is colored"};
        }
    }

    int Config::title_bar_bg_texture_id() const {
        throw_if_not_textured();
        return _title_bar_bg_texture_id;
    }

    int Config::font_bitmap_width() const {
        return _font_bitmap_width;
    }

    int Config::font_bitmap_height() const {
        return _font_bitmap_height;
    }

    Config& Config::window_colors(const primitive::Color& title_background,
                                  const primitive::Color& window_background) {
        _fill_mode = FillMode::kColored;
        _title_bar_bg_color = title_background;
        _window_bg_color = window_background;
        return *this;
    }

    Config& Config::window_textures(int title_texture, int background_texture) {
        _fill_mode = FillMode::kTextured;
        _title_bar_bg_texture_id = title_texture;
        _window_bg_texture_id = background_texture;
        return *this;
    }

    Config& Config::font_bitmap_size(int width, int height) {
        _font_bitmap_width = width;
        _font_bitmap_height = height;
        return *this;
    }
}