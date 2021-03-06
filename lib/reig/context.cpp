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
    Context::Context() : Context{Config::Builder{}.build()} {}

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
        int bitmap_width = _config.font_bitmap_width();
        int bitmap_height = _config.font_bitmap_height();

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

        update_window_layers();
        remove_unqueued_windows();

        _render_handler(_free_draw_data, _user_ptr);
        _free_draw_data.clear();
        render_windows();
    }

    void Context::update_window_layers() {
        if (!mouse.left_button.is_clicked()) return;

        for (auto it = _windows.begin(); it != _windows.end(); ++it) {
            if (mouse.left_button.just_clicked_in_window(detail::get_window_full_rect(*it))) {
                std::iter_swap(it, _windows.begin());
                break;
            }
        }
    }

    void Context::remove_unqueued_windows() {
        auto remove_from = std::remove_if(_windows.begin(), _windows.end(), [](const Window& window) {
            return !window.is_queued();
        });
        _windows.erase(remove_from, _windows.end());
    }

    bool Context::claim_window_focus(const Window& window) {
        if (!_dragged_window) {
            _dragged_window = window.id();
        }
        return _dragged_window == window.id();
    }

    bool Context::release_window_focus(const Window& window) {
        if (_dragged_window == window.id()) {
            _dragged_window = nullptr;
        }
        return _dragged_window != window.id();
    }

    void Context::start_window(gsl::czstring title, float default_x, float default_y) {
        start_window(title, title, default_x, default_y);
    }

    void Context::start_window(gsl::czstring id, gsl::czstring title, float default_x, float default_y) {
        if (!_windows.empty()) end_window();

        auto window = std::find_if(_windows.begin(), _windows.end(), [id](const Window& window) {
            return window.id() == id;
        });
        if (window != _windows.end()) {
            detail::restart_window(*window, title);
            _queued_window = &*window;
        } else {
            _windows.emplace(_windows.begin(), id, title, default_x, default_y, 0, 0, _font.height + 8);
            _queued_window = &_windows.front();
        }
    }

    void Context::render_windows() {
        for(auto it = _windows.rbegin(); it != _windows.rend(); ++it) {
            auto& current_window = *it;

            auto current_widget_data = move(current_window.draw_data());
            current_window.draw_data().clear();

            auto header_rect = get_window_header_rect(current_window);
            auto minimize_rect = get_window_minimize_rect(current_window);
            auto title_rect = decrease_rect(header_rect, 4);
            auto body_rect = get_window_body_rect(current_window);

            if (_config.fill_mode() == FillMode::kTextured) {
                render_rectangle(current_window.draw_data(), header_rect, _config.title_bar_bg_texture_id());
            } else {
                auto frame_color = it != _windows.rend() - 1
                                   ? colors::dim_color_by(_config.title_bar_bg_color(), 127)
                                   : _config.title_bar_bg_color();
                render_rectangle(current_window.draw_data(), header_rect, frame_color);
            }
            render_rectangle(current_window.draw_data(), minimize_rect, colors::kLightGrey);
            render_rectangle(current_window.draw_data(), decrease_rect(minimize_rect, 2), colors::kBlack);
            if (current_window.is_collapsed()) {
                minimize_rect = decrease_rect(minimize_rect, 8);
                render_rectangle(current_window.draw_data(), minimize_rect, colors::kLightGrey);
            } else {
                minimize_rect = decrease_rect(minimize_rect, 12);
                minimize_rect.x = minimize_rect.x - 2;
                minimize_rect.y = minimize_rect.y - 2;
                render_rectangle(current_window.draw_data(), minimize_rect, colors::kLightGrey);
                minimize_rect.x += 4;
                minimize_rect.y += 4;
                render_rectangle(current_window.draw_data(), minimize_rect, colors::kLightGrey);
            }
            render_text(current_window.draw_data(), current_window.title(), title_rect);
            if (_config.fill_mode() == FillMode::kTextured) {
                render_rectangle(current_window.draw_data(), body_rect, _config.window_bg_texture_id());
            } else {
                auto frame_color = it != _windows.rend() - 1
                                   ? colors::dim_color_by(_config.title_bar_bg_color(), 127)
                                   : _config.title_bar_bg_color();

                int thickness = 1;
                render_rectangle(current_window.draw_data(), decrease_rect(body_rect, thickness),
                                 _config.window_bg_color());

                auto frame = get_rect_frame(body_rect, thickness);
                for (const auto& frame_rect : frame) {
                    render_rectangle(current_window.draw_data(), frame_rect, frame_color);
                }
            }

            _render_handler(current_window.draw_data(), _user_ptr);
            _render_handler(current_widget_data, _user_ptr);
            // current_widget_data is likely to have larger capacity, so place it back
            current_widget_data.clear();
            current_window.draw_data() = move(current_widget_data);
        }
    }

    void Context::end_window() {
        if (_windows.empty()) return;

        if (_queued_window != nullptr) {
            handle_window_input(*_queued_window);
            _queued_window = nullptr;
        }
    }

    void Context::handle_window_input(detail::Window& window) {
        bool clicked_minimize = is_point_in_rect(mouse.left_button.get_clicked_pos(), get_window_minimize_rect(window));

        if (mouse.left_button.is_clicked()
            && clicked_minimize
            && is_window_header_point_visible(window, mouse.left_button.get_clicked_pos())
            && claim_window_focus(window)) {
            window.set_collapsed(!window.is_collapsed());
        } else if (mouse.left_button.is_held()
                   && !clicked_minimize
                   && is_window_header_point_visible(window, mouse.left_button.get_clicked_pos())
                   && claim_window_focus(window)) {
            Point moved{
                    mouse.get_cursor_pos().x - mouse.left_button.get_clicked_pos().x,
                    mouse.get_cursor_pos().y - mouse.left_button.get_clicked_pos().y
            };

            window.set_x(window.x() + moved.x);
            window.set_y(window.y() + moved.y);
            mouse.left_button._clicked_pos.x += moved.x;
            mouse.left_button._clicked_pos.y += moved.y;
        } else {
            release_window_focus(window);
        }
    }

    bool Context::is_window_header_point_visible(const Window& window, const Point& point) {
        auto header_rect = get_window_header_rect(window);
        bool point_in_header = is_point_in_rect(point, header_rect);
        if (!point_in_header) return false;

        for (auto& previous_window : _windows) {
            if (is_point_in_rect(point, get_window_full_rect(previous_window))) {
                return window.id() == previous_window.id();
            }
        }
        return true;
    }

    bool reig::Context::is_window_body_point_visible(const primitive::Point& point) {
        for (auto& window : _windows) {
            if (is_point_in_rect(point, get_window_full_rect(window))) {
                if (_queued_window) {
                    return _queued_window->id() == window.id();
                } else {
                    return false;
                }
            }
        }
        return _queued_window != nullptr ? !_queued_window->is_collapsed() : true;
    }

    void Context::fit_rect_in_window(Rectangle& rect) {
        if (_queued_window != nullptr) {
            detail::fit_rect_in_window(rect, *_queued_window);
        }
    }

    DrawData* Context::get_current_draw_data_buffer() {
        if (_queued_window) {
            return !_queued_window->is_collapsed() ? &_queued_window->draw_data() : nullptr;
        } else {
            return &_free_draw_data;
        }
    }

    void Context::start_frame() {
        mouse.left_button._is_clicked = false;
        mouse._scrolled = 0.f;

        keyboard.reset();

        for (auto& window : _windows) {
            window.set_queued(false);
        }

        ++_frame_counter;
    }

    unsigned Context::get_frame_counter() const {
        return _frame_counter;
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
        auto* buffer = get_current_draw_data_buffer();
        if (buffer != nullptr) {
            return render_text(*buffer, text, rect, alignment, scale);
        }
        return rect.x;
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

    void Context::render_rectangle(const Rectangle& rect, const Color& color) {
        auto* buffer = get_current_draw_data_buffer();
        if (buffer != nullptr) {
            render_rectangle(*buffer, rect, color);
        }
    }

    void Context::render_rectangle(const Rectangle& rect, int texture_id) {
        auto* buffer = get_current_draw_data_buffer();
        if (buffer != nullptr) {
            render_rectangle(*buffer, rect, texture_id);
        }
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