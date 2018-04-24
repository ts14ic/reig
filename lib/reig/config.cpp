#include "config.h"

namespace reig {
    ConfigBuilder Config::builder() {
        return ConfigBuilder();
    }

    Config::Config(const ConfigBuilder& builder) {
        _are_windows_textured = builder._are_windows_textured;
        if (_are_windows_textured) {
            _title_bar_bg_texture_id = builder._title_bar_bg_texture_id;
            _window_bg_texture_id = builder._window_bg_texture_id;
        } else {
            _title_bar_bg_color = builder._title_bar_bg_color;
            _window_bg_color = builder._window_bg_color;
        }
        _font_bitmap_width = builder._font_bitmap_width;
        _font_bitmap_height = builder._font_bitmap_height;
    }

    ConfigBuilder::ConfigBuilder() = default;

    ConfigBuilder& ConfigBuilder::window_colors(const primitive::Color& title_background,
                                                const primitive::Color& window_background) {
        _title_bar_bg_color = title_background;
        _window_bg_color = window_background;
        _are_windows_textured = false;
        return *this;
    }

    ConfigBuilder& ConfigBuilder::window_textures(int title_texture, int background_texture) {
        _title_bar_bg_texture_id = title_texture;
        _window_bg_texture_id = background_texture;
        _are_windows_textured = true;
        return *this;
    }

    ConfigBuilder& ConfigBuilder::font_bitmap_size(int width, int height) {
        _font_bitmap_width = width;
        _font_bitmap_height = height;
        return *this;
    }

    Config ConfigBuilder::build() {
        return Config(*this);
    }

}