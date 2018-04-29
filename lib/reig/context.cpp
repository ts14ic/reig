#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION

#include "context.h"
#include "exception.h"
#include "maths.h"
#include <memory>
#include <algorithm>

using namespace reig::primitive;
using reig::detail::Window;
using std::unique_ptr;
using std::reference_wrapper;
using std::vector;

namespace reig {
    Context::Context() : Context{Config::builder().build()} {}

    Context::Context(const Config& config)
            : mouse{*this}, _config{config} {}

    void Context::set_config(const Config& config) {
        _config = config;
    }

    void Context::set_render_handler(RenderHandler render_handler) {
        _render_handler = render_handler;
    }

    void Context::set_user_ptr(std::any user_ptr) {
        using std::move;
        _user_ptr = move(user_ptr);
    }

    vector<uint8_t> read_font_into_buffer(gsl::czstring const font_file_path) {
        using exception::FailedToLoadFontException;

        auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(std::fopen(font_file_path, "rb"), &std::fclose);
        if (!file) throw FailedToLoadFontException::could_not_open_file(font_file_path);

        std::fseek(file.get(), 0, SEEK_END);
        long file_pos = ftell(file.get());
        if (file_pos < 0) throw FailedToLoadFontException::invalid_file(font_file_path);

        auto file_size = math::integral_cast<size_t>(file_pos);
        std::rewind(file.get());

        auto ttf_buffer = std::vector<unsigned char>(file_size);
        std::fread(ttf_buffer.data(), 1, file_size, file.get());
        return ttf_buffer;
    }

    Context::FontBitmap Context::set_font(gsl::czstring font_file_path, int texture_id, float font_height_in_px) {
        using exception::FailedToLoadFontException;

        if (texture_id == 0) throw FailedToLoadFontException::no_texture_id(font_file_path);
        if (font_height_in_px <= 0) throw FailedToLoadFontException::invalid_height(font_file_path, font_height_in_px);

        auto ttf_buffer = read_font_into_buffer(font_file_path);

        // We want all ASCII chars from space to backspace
        int const num_chars = 96;
        int bitmap_width = _config._font_bitmap_width;
        int bitmap_height = _config._font_bitmap_height;

        using std::data;
        auto baked_chars = std::vector<stbtt_bakedchar>(num_chars);
        auto bitmap = vector<uint8_t>(math::integral_cast<size_t>(bitmap_width * bitmap_height));

        int baked_height = stbtt_BakeFontBitmap(ttf_buffer.data(), 0, font_height_in_px, bitmap.data(),
                                               bitmap_width, bitmap_height, ' ', num_chars, data(baked_chars));
        if (baked_height < 0 || baked_height > bitmap_height) {
            throw FailedToLoadFontException::could_not_fit_characters(font_file_path, font_height_in_px,
                                                                      bitmap_width, bitmap_height);
        } else {
            bitmap_height = baked_height;
        }

        using std::move;
        // If all successful, replace current font data
        _font.baked_chars = move(baked_chars);
        _font.texture_id = texture_id;
        _font.bitmap_width = bitmap_width;
        _font.bitmap_height = bitmap_height;
        _font.height = font_height_in_px;

        return FontBitmap{bitmap, bitmap_width, bitmap_height};
    }

    float Context::get_font_size() const {
        return _font.height;
    }

    void Context::end_frame() {
        if (!_render_handler) {
            throw exception::NoRenderHandlerException{};
        }
        end_window();

        update_previous_windows();
        cleanup_previous_windows();

        _render_handler(_free_draw_data, _user_ptr);
        _free_draw_data.clear();
        render_windows();

        _queued_windows.clear();
    }

    void Context::update_previous_windows() {
        for (auto it = _queued_windows.rbegin(); it != _queued_windows.rend(); ++it) {
            auto& queued_window = *it;
            auto previous_window = std::find_if(_previous_windows.begin(), _previous_windows.end(),
                                               [&queued_window](const Window& window) {
                                                   return queued_window.id() == window.id();
                                               });

            if (previous_window != _previous_windows.end()) {
                previous_window->set_width(queued_window.width());
                previous_window->set_height(queued_window.height());
                previous_window->set_x(queued_window.x());
                previous_window->set_y(queued_window.y());
            } else {
                _previous_windows.insert(_previous_windows.begin(), queued_window);
            }
        }

        if (!mouse.left_button.is_clicked()) return;

        for (auto it = _previous_windows.begin(); it != _previous_windows.end(); ++it) {
            if (mouse.left_button.just_clicked_in_window(detail::get_full_window_rect(*it))) {
                std::iter_swap(it, _previous_windows.begin());
                break;
            }
        }
    }

    void Context::cleanup_previous_windows() {
        auto remove_from = std::remove_if(_previous_windows.begin(), _previous_windows.end(),
                                         [this](const Window& previous_window) {
                                             return std::none_of(_queued_windows.begin(), _queued_windows.end(),
                                                                     [&previous_window](const Window& window) {
                                                                         return window.id() == previous_window.id();
                                                                     });
                                         });
        _previous_windows.erase(remove_from, _previous_windows.end());
    }

    bool Context::handle_window_focus(const Window& window, bool is_claiming) {
        if (is_claiming) {
            if (!_dragged_window) {
                _dragged_window = window.id();
            }
            return _dragged_window == window.id();
        } else {
            if (_dragged_window == window.id()) {
                _dragged_window = nullptr;
            }
            return _dragged_window != window.id();
        }
    }

    void Context::start_frame() {
        mouse.left_button._is_clicked = false;
        mouse._scrolled = 0.f;

        keyboard.reset();

        ++_frame_counter;
    }

    unsigned Context::get_frame_counter() const {
        return _frame_counter;
    }

    detail::Window* Context::get_current_window() {
        if (!_queued_windows.empty() && !_queued_windows.back().is_finished()) {
            return &_queued_windows.back();
        } else {
            return nullptr;
        }
    }

    void Context::start_window(gsl::czstring title, float default_x, float default_y) {
        start_window(title, title, default_x, default_y);
    }

    void Context::start_window(gsl::czstring id, gsl::czstring title, float default_x, float default_y) {
        if (!_queued_windows.empty()) end_window();

        auto previous_window = std::find_if(_previous_windows.begin(), _previous_windows.end(),
                                           [id](const Window& window) {
                                               return window.id() == id;
                                           });
        if (previous_window != _previous_windows.end()) {
            _queued_windows.emplace_back(id, title, previous_window->x(), previous_window->y(), 0, 0, _font.height + 8);
        } else {
            _queued_windows.emplace_back(id, title, default_x, default_y, 0, 0, _font.height + 8);
        }
    }

    void Context::render_windows() {
        vector<reference_wrapper<Window>> ordered_windows;
        for (auto pit = _previous_windows.rbegin(); pit != _previous_windows.rend(); ++pit) {
            auto& previous_window = *pit;
            auto qit = std::find_if(_queued_windows.begin(), _queued_windows.end(),
                                   [&previous_window](const Window& window) {
                                       return previous_window.id() == window.id();
                                   });
            ordered_windows.push_back(std::ref(*qit));
        }

        for(auto it = ordered_windows.begin(); it != ordered_windows.end(); ++it) {
            auto& current_window = it->get();

            auto current_widget_data = move(current_window.draw_data());
            current_window.draw_data().clear();

            auto header_rect = detail::get_window_header_rect(current_window);
            Triangle header_triangle{
                    {current_window.x() + 3.f, current_window.y() + 3.f},
                    {current_window.x() + 3.f + current_window.title_bar_height(), current_window.y() + 3.f},
                    {current_window.x() + 3.f + current_window.title_bar_height() / 2.f,
                     current_window.y() + current_window.title_bar_height() - 3.f}
            };
            auto title_rect = decrease_rect(header_rect, 4);
            auto body_rect = detail::get_window_body_rect(current_window);

            auto frame_color = it != ordered_windows.end() - 1
                              ? colors::dim_color_by(_config._title_bar_bg_color, 127)
                              : _config._title_bar_bg_color;

            if (_config._are_windows_textured) {
                render_rectangle(current_window.draw_data(), header_rect, _config._title_bar_bg_texture_id);
            } else {
                render_rectangle(current_window.draw_data(), header_rect, frame_color);
            }
            render_triangle(current_window.draw_data(), header_triangle, colors::kLightGrey);
            render_text(current_window.draw_data(), current_window.title(), title_rect);
            if (_config._are_windows_textured) {
                render_rectangle(current_window.draw_data(), body_rect, _config._window_bg_texture_id);
            } else {
                int thickness = 1;
                render_rectangle(current_window.draw_data(), decrease_rect(body_rect, thickness),
                                 _config._window_bg_color);

                auto frame = get_rect_frame(body_rect, thickness);
                for (const auto& frame_rect : frame) {
                    render_rectangle(current_window.draw_data(), frame_rect, frame_color);
                }
            }

            _render_handler(current_window.draw_data(), _user_ptr);
            current_window.draw_data().clear();

            _render_handler(current_widget_data, _user_ptr);
        }
    }

    void Context::end_window() {
        if (_queued_windows.empty()) return;

        auto& current_window = *get_current_window();

        current_window.finish();
        current_window.set_width(current_window.width() + 4);
        current_window.set_height(current_window.height() + 4);

        handle_window_input(current_window);
    }

    void Context::handle_window_input(detail::Window& window) {
        if (mouse.left_button.is_held()
            && is_window_header_point_visible(window, mouse.left_button.get_clicked_pos())
            && handle_window_focus(window, true)) {
            Point moved{
                    mouse.get_cursor_pos().x - mouse.left_button.get_clicked_pos().x,
                    mouse.get_cursor_pos().y - mouse.left_button.get_clicked_pos().y
            };

            window.set_x(window.x() + moved.x);
            window.set_y(window.y() + moved.y);
            mouse.left_button._clicked_pos.x += moved.x;
            mouse.left_button._clicked_pos.y += moved.y;
        } else {
            handle_window_focus(window, false);
        }
    }

    bool Context::is_window_header_point_visible(const Window& window, const Point& point) {
        for (auto& previous_window : _previous_windows) {
            if (is_point_in_rect(point, get_window_header_rect(window))) {
                return window.id() == previous_window.id();
            }
        }
        return false;
    }

    bool reig::Context::is_window_body_point_visible(const primitive::Point& point) {
        auto* current_window = get_current_window();
        for (auto& previousWindow : _previous_windows) {
            if (is_point_in_rect(point, get_window_body_rect(previousWindow))) {
                if (current_window) {
                    return current_window->id() == previousWindow.id();
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    void Context::fit_rect_in_window(Rectangle& rect) {
        if (!_queued_windows.empty()) {
            detail::fit_rect_in_window(rect, _queued_windows.back());
        }
    }

    bool has_alignment(text::Alignment container, text::Alignment alignment) {
        auto container_as_uint = static_cast<unsigned>(container);
        auto alignment_as_uint = static_cast<unsigned>(alignment);
        return (alignment_as_uint & container_as_uint) == alignment_as_uint;
    }

    inline stbtt_aligned_quad get_char_quad(int charIndex, float& x, float& y, const detail::Font& font) {
        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(data(font.baked_chars), font.bitmap_width, font.bitmap_height, charIndex, &x, &y, &quad, true);
        return quad;
    }

    inline Point scale_quad(stbtt_aligned_quad& quad, float scale, float x, float previous_x) {
        float anti_scale = 1.0f - scale;

        float scaling_horizontal_offset = (x - previous_x) * anti_scale;
        quad.x1 -= scaling_horizontal_offset;
        float scaling_vertical_offset = (quad.y1 - quad.y0) * anti_scale;
        quad.y0 += scaling_vertical_offset;

        return Point{scaling_horizontal_offset, scaling_vertical_offset};
    }

    float Context::render_text(gsl::czstring text, const Rectangle rect, text::Alignment alignment, float scale) {
        return render_text(get_current_draw_data_buffer(), text, rect, alignment, scale);
    }

    float Context::render_text(DrawData& draw_data, gsl::czstring text, Rectangle rect, text::Alignment alignment,
                               float scale) {
        if (_font.baked_chars.empty() || !text) return rect.x;

        float x = rect.x;
        float y = rect.y + rect.height;

        float min_y = y;
        float max_y = y;

        vector<stbtt_aligned_quad> quads;
        quads.reserve(20);

        auto from_char = int{' '};
        int to_char = from_char + 95;
        int fallback_char = to_char; // The backspace character
        for (int ch = *text; ch != '\0'; ch = *++text) {
            if (!(ch >= from_char && ch <= to_char)) {
                ch = fallback_char;
            }

            float previous_x = x;
            auto quad = get_char_quad(ch - from_char, x, y, _font);

            auto scaling_offsets = scale_quad(quad, scale, x, previous_x);
            x -= scaling_offsets.x;

            if (quad.x0 > get_x2(rect)) {
                break;
            }
            quad.x1 = math::min(quad.x1, get_x2(rect));
//            quad.y0 = internal::max(quad.y0, rect.y);
//            quad.y1 = internal::min(quad.y1, get_y2(rect));

            min_y = math::min(min_y, quad.y0);
            max_y = math::max(max_y, quad.y1);

            quads.push_back(quad);
        }

        float text_height = max_y - min_y;
        float text_width = 0.0f;
        if (!quads.empty()) {
            text_width = quads.back().x1 - quads.front().x0;
        }

        float horizontal_alignment =
                has_alignment(alignment, text::Alignment::kRight) ? rect.width - text_width :
                has_alignment(alignment, text::Alignment::kLeft) ? 0.0f :
                (rect.width - text_width) * 0.5f;
        float vertical_alignment =
                has_alignment(alignment, text::Alignment::kTop) ? -(rect.height - text_height) :
                has_alignment(alignment, text::Alignment::kBottom) ? 0.0f :
                (rect.height - text_height) * -0.5f;

        render_text_quads(draw_data, quads, horizontal_alignment, vertical_alignment, _font.texture_id);

        return x;
    }

    DrawData& Context::get_current_draw_data_buffer() {
        auto* current_window = get_current_window();
        if (current_window) {
            return current_window->draw_data();
        } else {
            return _free_draw_data;
        }
    }

    void Context::render_rectangle(const Rectangle& rect, const Color& color) {
        render_rectangle(get_current_draw_data_buffer(), rect, color);
    }

    void Context::render_rectangle(const Rectangle& rect, int texture_id) {
        render_rectangle(get_current_draw_data_buffer(), rect, texture_id);
    }

    void Context::render_triangle(const Triangle& triangle, const Color& color) {
        render_triangle(get_current_draw_data_buffer(), triangle, color);
    }

    void Context::render_rectangle(DrawData& draw_data, const Rectangle& rect, const Color& color) {
        vector<Vertex> vertices{
                {{rect.x,       rect.y},       {}, color},
                {{get_x2(rect), rect.y},       {}, color},
                {{get_x2(rect), get_y2(rect)}, {}, color},
                {{rect.x,       get_y2(rect)}, {}, color}
        };
        vector<int> indices{0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices);
        draw_data.push_back(fig);
    }

    void Context::render_rectangle(DrawData& draw_data, const Rectangle& rect, int texture_id) {
        vector<Vertex> vertices{
                {{rect.x,       rect.y},       {0.f, 0.f}, {}},
                {{get_x2(rect), rect.y},       {1.f, 0.f}, {}},
                {{get_x2(rect), get_y2(rect)}, {1.f, 1.f}, {}},
                {{rect.x,       get_y2(rect)}, {0.f, 1.f}, {}}
        };
        vector<int> indices{0, 1, 2, 2, 3, 0};

        Figure fig;
        fig.form(vertices, indices, texture_id);
        draw_data.push_back(fig);
    }

    void Context::render_triangle(DrawData& draw_data, const Triangle& triangle, const Color& color) {
        vector<Vertex> vertices{
                {{triangle.pos0}, {}, color},
                {{triangle.pos1}, {}, color},
                {{triangle.pos2}, {}, color}
        };
        vector<int> indices = {0, 1, 2};

        Figure fig;
        fig.form(vertices, indices);
        draw_data.push_back(fig);
    }

    void Context::render_text_quads(DrawData& draw_data, const std::vector<stbtt_aligned_quad>& quads,
                                    float horizontal_alignment, float vertical_alignment, int font_texture_id) {
        for (auto& q : quads) {
            vector<Vertex> vertices{
                    {{q.x0 + horizontal_alignment, q.y0 + vertical_alignment}, {q.s0, q.t0}, {}},
                    {{q.x1 + horizontal_alignment, q.y0 + vertical_alignment}, {q.s1, q.t0}, {}},
                    {{q.x1 + horizontal_alignment, q.y1 + vertical_alignment}, {q.s1, q.t1}, {}},
                    {{q.x0 + horizontal_alignment, q.y1 + vertical_alignment}, {q.s0, q.t1}, {}}
            };
            vector<int> indices{0, 1, 2, 2, 3, 0};

            Figure fig;
            fig.form(vertices, indices, font_texture_id);
            draw_data.push_back(fig);
        }
    }
}