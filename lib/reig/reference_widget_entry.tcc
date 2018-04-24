#ifndef REIG_REFERENCE_WIDGET_ENTRY_TCC
#define REIG_REFERENCE_WIDGET_ENTRY_TCC

#include "reference_widget_entry.h"

namespace reig::reference_widget {
    namespace detail {
        struct EntryModel {
            const primitive::Rectangle outline_area;
            const primitive::Rectangle base_area;
            const primitive::Rectangle caret_area;
            const bool is_selected = false;
            const bool is_holding_click = false;
        };

        template <typename C>
        void display_entry_model(Context& ctx, const EntryModel& model, primitive::Color primary_color,
                                 const std::basic_string<C>& value_ref, const char* title);
    }

    template <typename C>
    EntryOuput entry(reig::Context& ctx, const char* title, const primitive::Rectangle& bounding_area,
                     const primitive::Color& base_color, std::basic_string<C>& value_ref) {
        using namespace primitive;
        Rectangle outline_area = bounding_area;
        ctx.fit_rect_in_window(outline_area);

        Rectangle base_area = decrease_rect(outline_area, 4);
        Rectangle caret_area {0, base_area.y, 0, base_area.height};
        bool is_selected = ctx.mouse.left_button.clicked_in_rect(outline_area);
        bool is_holding_click = is_selected && ctx.mouse.left_button.is_held();

        if (is_holding_click) {
            base_area = decrease_rect(base_area, 4);
        }

        EntryOuput output = EntryOuput::kUnmodified;
        if (is_selected) {
            if ((ctx.get_frame_counter() / 30) % 2 == 0) {
                caret_area = decrease_rect(caret_area, 10);
                caret_area.width = 2;
            }

            Key key_type = ctx.keyboard.get_pressed_key_type();
            switch (key_type) {
                case Key::kChar: {
                    value_ref += ctx.keyboard.get_pressed_char();
                    output = EntryOuput::kModified;
                    break;
                }

                case Key::kBackspace: {
                    using std::empty;
                    if (!empty(value_ref)) {
                        value_ref.pop_back();
                        output = EntryOuput::kModified;
                    }
                    break;
                }

                case Key::kReturn: {
                    output = EntryOuput::kSubmitted;
                    break;
                }

                case Key::kEscape: {
                    ctx.mouse.left_button.press(outline_area.x, outline_area.y);
                    ctx.mouse.left_button.release();
                    output = EntryOuput::kCancelled;
                    break;
                }

                default: {
                    break;
                }
            }
        }

        detail::EntryModel model{outline_area, base_area, caret_area, is_selected, is_holding_click};

        display_entry_model(ctx, model, base_color, value_ref, title);

        return output;
    }

    template <typename C>
    void detail::display_entry_model(Context& ctx, const EntryModel& model,
                                     primitive::Color primary_color, const std::basic_string<C>& value_ref, const char* title) {
        using namespace primitive;
        Color secondary_color = colors::get_yiq_contrast(primary_color);

        ctx.render_rectangle(model.outline_area, secondary_color);
        if (model.is_holding_click) {
            primary_color = colors::lighten_color_by(primary_color, 30);
        }
        ctx.render_rectangle(model.base_area, primary_color);
        if (model.is_selected) {
            float caretX = ctx.render_text(value_ref.c_str(), model.base_area, text::Alignment::kLeft);

            Rectangle caret_area = model.caret_area;
            caret_area.x = caretX;
            trim_rect_in_other(caret_area, model.base_area);
            ctx.render_rectangle(caret_area, secondary_color);
        } else {
            ctx.render_text(value_ref.empty() ? title : value_ref.c_str(), model.base_area, text::Alignment::kLeft);
        }
    }
}

#endif //REIG_REFERENCE_WIDGET_ENTRY_TCC
