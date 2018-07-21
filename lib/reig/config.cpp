#include "config.h"

namespace reig {
    ConfigBuilder Config::builder() {
        return ConfigBuilder();
    }

    Config::Config(const ConfigBuilder& builder) {
        _are_windows_textured = builder.are_windows_textured();
        if (_are_windows_textured) {
            _title_bar_bg_texture_id = builder.title_bar_bg_texture_id();
            _window_bg_texture_id = builder.window_bg_texture_id();
        } else {
            _title_bar_bg_color = builder.title_bar_bg_color();
            _window_bg_color = builder.window_bg_color();
        }
        _font_bitmap_width = builder.font_bitmap_width();
        _font_bitmap_height = builder.font_bitmap_height();
    }

    const primitive::Color& Config::window_bg_color() const {
        return _window_bg_color;
    }

    const primitive::Color& Config::title_bar_bg_color() const {
        return _title_bar_bg_color;
    }

    bool Config::are_windows_textured() const {
        return _are_windows_textured;
    }

    int Config::window_bg_texture_id() const {
        return _window_bg_texture_id;
    }

    int Config::title_bar_bg_texture_id() const {
        return _title_bar_bg_texture_id;
    }

    int Config::font_bitmap_width() const {
        return _font_bitmap_width;
    }

    int Config::font_bitmap_height() const {
        return _font_bitmap_height;
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

    const primitive::Color& ConfigBuilder::window_bg_color() const {
        return _window_bg_color;
    }

    const primitive::Color& ConfigBuilder::title_bar_bg_color() const {
        return _title_bar_bg_color;
    }

    bool ConfigBuilder::are_windows_textured() const {
        return _are_windows_textured;
    }

    int ConfigBuilder::window_bg_texture_id() const {
        return _window_bg_texture_id;
    }

    int ConfigBuilder::title_bar_bg_texture_id() const {
        return _title_bar_bg_texture_id;
    }

    int ConfigBuilder::font_bitmap_width() const {
        return _font_bitmap_width;
    }

    int ConfigBuilder::font_bitmap_height() const {
        return _font_bitmap_height;
    }

}