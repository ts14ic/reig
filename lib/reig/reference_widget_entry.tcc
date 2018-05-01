#ifndef REIG_REFERENCE_WIDGET_ENTRY_TCC
#define REIG_REFERENCE_WIDGET_ENTRY_TCC

#include "reference_widget_entry.h"

namespace reig::reference_widget {
    namespace detail {
        struct EntryModel {
            const primitive::Rectangle bounding_box;
            const primitive::Rectangle caret_box;
            const bool is_selected = false;
            const bool is_holding_click = false;
        };

        template <typename C>
        void display_entry_model(Context& ctx, const EntryModel& model, primitive::Color primary_color,
                                 const std::basic_string<C>& value_ref, gsl::czstring title);
    }

    template <typename C>
    EntryOutput entry(reig::Context& ctx, gsl::czstring title, primitive::Rectangle bounding_box,
                     const primitive::Color& base_color, std::basic_string<C>& value) {
        using namespace primitive;
        ctx.fit_rect_in_window(bounding_box);

        Rectangle caret_area {0, bounding_box.y + 2, 0, bounding_box.height - 4};
        bool is_selected = ctx.mouse.left_button.clicked_in_rect(bounding_box);
        bool is_holding_click = is_selected && ctx.mouse.left_button.is_held();

        EntryOutput output = EntryOutput::kUnmodified;
        if (is_selected) {
            if ((ctx.get_frame_counter() / 30) % 2 == 0) {
                caret_area = decrease_rect(caret_area, 10);
                caret_area.width = 2;
            }

            Key key_type = ctx.keyboard.get_pressed_key_type();
            switch (key_type) {
                case Key::kChar: {
                    value += ctx.keyboard.get_pressed_char();
                    output = EntryOutput::kModified;
                    break;
                }

                case Key::kBackspace: {
                    using std::empty;
                    if (!empty(value)) {
                        value.pop_back();
                        output = EntryOutput::kModified;
                    }
                    break;
                }

                case Key::kReturn: {
                    output = EntryOutput::kSubmitted;
                    break;
                }

                case Key::kEscape: {
                    ctx.mouse.left_button.press(bounding_box.x, bounding_box.y);
                    ctx.mouse.left_button.release();
                    output = EntryOutput::kCancelled;
                    break;
                }

                default: {
                    break;
                }
            }
        }

        detail::EntryModel model{bounding_box, caret_area, is_selected, is_holding_click};

        display_entry_model(ctx, model, base_color, value, title);

        return output;
    }

    template <typename C>
    void detail::display_entry_model(Context& ctx, const EntryModel& model,
                                     primitive::Color primary_color,
                                     const std::basic_string<C>& value_ref,
                                     gsl::czstring title) {
        using namespace primitive;
        Color secondary_color = colors::get_yiq_contrast(primary_color);

        Rectangle base_area = decrease_rect(model.bounding_box, 4);

        ctx.render_rectangle(model.bounding_box, secondary_color);
        if (model.is_holding_click) {
            base_area = decrease_rect(base_area, 4);
            primary_color = colors::lighten_color_by(primary_color, 30);
        }
        ctx.render_rectangle(base_area, primary_color);
        if (model.is_selected) {
            float caretX = ctx.render_text(value_ref.c_str(), base_area, text::Alignment::kLeft);

            Rectangle caret_area = model.caret_box;
            caret_area.x = caretX;
            trim_rect_in_other(caret_area, base_area);
            ctx.render_rectangle(caret_area, secondary_color);
        } else {
            ctx.render_text(value_ref.empty() ? title : value_ref.c_str(), base_area, text::Alignment::kLeft);
        }
    }
}

#endif //REIG_REFERENCE_WIDGET_ENTRY_TCC
