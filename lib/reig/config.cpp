#include "config.h"

namespace reig {
    Config::Config(const Config::Builder& builder) {
        _fill_mode = builder.fill_mode();
        _window_bg_texture_id = builder.window_bg_texture_id();
        _window_bg_color = builder.window_bg_color();
        _title_bar_bg_texture_id = builder.title_bar_bg_texture_id();
        _title_bar_bg_color = builder.title_bar_bg_color();
        _font_bitmap_width = builder.font_bitmap_width();
        _font_bitmap_height = builder.font_bitmap_height();
    }

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

    Config::Builder::Builder() = default;

    Config Config::Builder::build() {
        return Config{*this};
    }

    Config::Builder& Config::Builder::set_window_colors(const primitive::Color& title_background,
                                                        const primitive::Color& window_background) {
        _fill_mode = FillMode::kColored;
        _title_bar_bg_color = title_background;
        _window_bg_color = window_background;
        return *this;
    }

    Config::Builder& Config::Builder::set_window_textures(int title_texture, int background_texture) {
        _fill_mode = FillMode::kTextured;
        _title_bar_bg_texture_id = title_texture;
        _window_bg_texture_id = background_texture;
        return *this;
    }

    Config::Builder& Config::Builder::set_font_bitmap_size(int width, int height) {
        if (width <= 0) throw std::invalid_argument{"width must be positive"};
        if (height <= 0) throw std::invalid_argument{"height must be postiive"};
        _font_bitmap_width = width;
        _font_bitmap_height = height;
        return *this;
    }

    const primitive::Color& Config::Builder::window_bg_color() const {
        return _window_bg_color;
    }

    const primitive::Color& Config::Builder::title_bar_bg_color() const {
        return _title_bar_bg_color;
    }

    FillMode Config::Builder::fill_mode() const {
        return _fill_mode;
    }

    int Config::Builder::window_bg_texture_id() const {
        return _window_bg_texture_id;
    }

    int Config::Builder::title_bar_bg_texture_id() const {
        return _title_bar_bg_texture_id;
    }

    int Config::Builder::font_bitmap_width() const {
        return _font_bitmap_width;
    }

    int Config::Builder::font_bitmap_height() const {
        return _font_bitmap_height;
    }
}