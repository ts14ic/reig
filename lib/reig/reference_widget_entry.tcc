#ifndef REIG_REFERENCE_WIDGET_ENTRY_TCC
#define REIG_REFERENCE_WIDGET_ENTRY_TCC

#include "reference_widget_entry.h"

namespace reig::reference_widget {
    namespace detail {
        struct EntryModel {
            const primitive::Rectangle outlineArea;
            const primitive::Rectangle baseArea;
            const primitive::Rectangle caretArea;
            const bool isSelected = false;
            const bool holdingClick = false;
        };

        template <typename C>
        void display_entry_model(Context& ctx, const EntryModel& model, primitive::Color primaryColor,
                                 const std::basic_string<C>& valueRef, const char* title);
    }

    template <typename C>
    EntryOuput entry(reig::Context& ctx, const char* title, const primitive::Rectangle& boundingArea,
                     const primitive::Color& primaryColor, std::basic_string<C>& value) {
        using namespace primitive;
        Rectangle outlineArea = boundingArea;
        ctx.fit_rect_in_window(outlineArea);

        Rectangle baseArea = decrease_rect(outlineArea, 4);
        Rectangle caretArea {0, baseArea.y, 0, baseArea.height};
        bool isSelected = ctx.mouse.left_button.clicked_in_rect(outlineArea);
        bool holdingClick = isSelected && ctx.mouse.left_button.is_held();

        if (holdingClick) {
            baseArea = decrease_rect(baseArea, 4);
        }

        EntryOuput output = EntryOuput::kUnmodified;
        if (isSelected) {
            if ((ctx.get_frame_counter() / 30) % 2 == 0) {
                caretArea = decrease_rect(caretArea, 10);
                caretArea.width = 2;
            }

            Key keyType = ctx.keyboard.get_pressed_key_type();
            switch (keyType) {
                case Key::kChar: {
                    value += ctx.keyboard.get_pressed_char();
                    output = EntryOuput::kModified;
                    break;
                }

                case Key::kBackspace: {
                    using std::empty;
                    if (!empty(value)) {
                        value.pop_back();
                        output = EntryOuput::kModified;
                    }
                    break;
                }

                case Key::kReturn: {
                    output = EntryOuput::kSubmitted;
                    break;
                }

                case Key::kEscape: {
                    ctx.mouse.left_button.press(outlineArea.x, outlineArea.y);
                    ctx.mouse.left_button.release();
                    output = EntryOuput::kCancelled;
                    break;
                }

                default: {
                    break;
                }
            }
        }

        detail::EntryModel model{outlineArea, baseArea, caretArea, isSelected, holdingClick};

        display_entry_model(ctx, model, primaryColor, value, title);

        return output;
    }

    template <typename C>
    void detail::display_entry_model(Context& ctx, const EntryModel& model,
                                     primitive::Color primaryColor, const std::basic_string<C>& valueRef, const char* title) {
        using namespace primitive;
        Color secondaryColor = colors::get_yiq_contrast(primaryColor);

        ctx.render_rectangle(model.outlineArea, secondaryColor);
        if (model.holdingClick) {
            primaryColor = colors::lighten_color_by(primaryColor, 30);
        }
        ctx.render_rectangle(model.baseArea, primaryColor);
        if (model.isSelected) {
            float caretX = ctx.render_text(valueRef.c_str(), model.baseArea, text::Alignment::kLeft);

            Rectangle caretArea = model.caretArea;
            caretArea.x = caretX;
            trim_rect_in_other(caretArea, model.baseArea);
            ctx.render_rectangle(caretArea, secondaryColor);
        } else {
            ctx.render_text(valueRef.empty() ? title : valueRef.c_str(), model.baseArea, text::Alignment::kLeft);
        }
    }
}

#endif //REIG_REFERENCE_WIDGET_ENTRY_TCC
