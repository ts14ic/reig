#ifndef REIG_REFERENCE_WIDGET_ENTRY_TCC
#define REIG_REFERENCE_WIDGET_ENTRY_TCC

#include "reference_widget_entry.h"
#include "internal.h"

namespace reig::reference_widget {
    namespace detail {
        struct EntryModel {
            const Rectangle outlineArea;
            const Rectangle baseArea;
            const Rectangle caretArea;
            const bool isSelected = false;
            const bool holdingClick = false;
        };

        template <typename C>
        void display_entry_model(Context& ctx, const EntryModel& model,
                                 Color primaryColor, const std::basic_string<C>& valueRef, const char* title);
    }

    template <typename C>
    EntryOuput entry(reig::Context& ctx, const char* title, const Rectangle& boundingArea,
                     const Color& primaryColor, std::basic_string<C>& value) {
        Rectangle outlineArea = boundingArea;
        ctx.fit_rect_in_window(outlineArea);

        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
        Rectangle caretArea {0, baseArea.y, 0, baseArea.height};
        bool isSelected = ctx.mouse.leftButton.clicked_in_rect(outlineArea);
        bool holdingClick = isSelected && ctx.mouse.leftButton.is_held();

        if (holdingClick) {
            baseArea = internal::decrease_rect(baseArea, 4);
        }

        EntryOuput output = EntryOuput::textUntouched;
        if (isSelected) {
            if ((ctx.get_frame_counter() / 30) % 2 == 0) {
                caretArea = internal::decrease_rect(caretArea, 10);
                caretArea.width = 2;
            }

            Key keyType = ctx.keyboard.get_pressed_key_type();
            switch (keyType) {
                case Key::CHAR: {
                    value += ctx.keyboard.get_pressed_char();
                    output = EntryOuput::textModified;
                    break;
                }

                case Key::BACKSPACE: {
                    using std::empty;
                    if (!empty(value)) {
                        value.pop_back();
                        output = EntryOuput::textModified;
                    }
                    break;
                }

                case Key::RETURN: {
                    output = EntryOuput::textSubmitted;
                    break;
                }

                case Key::ESCAPE: {
                    ctx.mouse.leftButton.press(outlineArea.x, outlineArea.y);
                    ctx.mouse.leftButton.release();
                    output = EntryOuput::textCancelled;
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
                                     Color primaryColor, const std::basic_string<C>& valueRef, const char* title) {
        Color secondaryColor = internal::get_yiq_contrast(primaryColor);

        ctx.render_rectangle(model.outlineArea, secondaryColor);
        if (model.holdingClick) {
            primaryColor = internal::lighten_color_by(primaryColor, 30);
        }
        ctx.render_rectangle(model.baseArea, primaryColor);
        if (model.isSelected) {
            float caretX = ctx.render_text(valueRef.c_str(), model.baseArea, text::Alignment::LEFT);

            Rectangle caretArea = model.caretArea;
            caretArea.x = caretX;
            internal::trim_rect_in_other(caretArea, model.baseArea);
            ctx.render_rectangle(caretArea, secondaryColor);
        } else {
            ctx.render_text(valueRef.empty() ? title : valueRef.c_str(), model.baseArea, text::Alignment::LEFT);
        }
    }
}

#endif //REIG_REFERENCE_WIDGET_ENTRY_TCC
