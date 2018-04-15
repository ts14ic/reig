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
            const bool isFocused = false;
            const bool isSelected = false;
            const bool isHoveringOverArea = false;
            const bool isInputModified = false;
        };

        template <template <class, class> typename Entry, typename String, typename Action>
        EntryModel get_entry_model(Context& ctx, const Entry<String, Action>& entry);
    }

    template <template <class, class> typename Entry, typename String, typename Action>
    auto detail::get_entry_model(reig::Context& ctx, const Entry<String, Action>& entry) -> EntryModel {
        int focusId = ctx.focus.create_id();
        Rectangle outlineArea = entry.mBoundingArea;
        ctx.fit_rect_in_window(outlineArea);

        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
        Rectangle caretArea {0, baseArea.y, 0, baseArea.height};
        bool hoveringOverArea = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), outlineArea);
        bool clickedInArea = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
        bool isFocused = ctx.focus.handle(focusId, clickedInArea);
        bool isInputModified = false;
        if (clickedInArea && isFocused) {
            baseArea = internal::decrease_rect(baseArea, 4);

            if ((ctx.get_frame_counter() / 30) % 2 == 0) {
                caretArea = internal::decrease_rect(caretArea, 10);
                caretArea.width = 5;
            }

            Key keyType = ctx.keyboard.get_pressed_key_type();
            switch (keyType) {
                case Key::CHAR: {
                    entry.mValueRef += ctx.keyboard.get_pressed_char();
                    isInputModified = true;
                    break;
                }

                case Key::BACKSPACE: {
                    using std::empty;
                    if (!empty(entry.mValueRef)) {
                        entry.mValueRef.pop_back();
                        isInputModified = true;
                    }
                    break;
                }

                case Key::RETURN:
                case Key::ESCAPE: {
                    ctx.mouse.leftButton.press(outlineArea.x, outlineArea.y);
                    ctx.mouse.leftButton.release();
                    break;
                }

                default: {
                    break;
                }
            }
        }

        return EntryModel{outlineArea, baseArea, caretArea, isFocused, clickedInArea, hoveringOverArea, isInputModified};
    }

    template <typename Char, typename Action>
    void detail::ref_entry<Char, Action>::use(reig::Context& ctx) const {
        auto model = get_entry_model(ctx, *this);

        Color secondaryColor = internal::get_yiq_contrast(mPrimaryColor);

        ctx.render_rectangle(model.outlineArea, secondaryColor);
        auto primaryColor = mPrimaryColor;
        if (model.isHoveringOverArea && model.isFocused) {
            primaryColor = internal::lighten_color_by(primaryColor, 30);
        }
        ctx.render_rectangle(model.baseArea, primaryColor);
        if (model.isSelected && model.isFocused) {
            float caretX = ctx.render_text(mValueRef.c_str(), model.baseArea, text::Alignment::LEFT);

            Rectangle caretArea = model.caretArea;
            caretArea.x = caretX;
            internal::trim_rect_in_other(caretArea, model.baseArea);
            ctx.render_rectangle(caretArea, secondaryColor);
        } else {
            ctx.render_text(mValueRef.empty() ? mTitle : mValueRef.c_str(), model.baseArea, text::Alignment::LEFT);
        }

        if (model.isInputModified) {
            mAction(mValueRef);
        }
    }
}

#endif //REIG_REFERENCE_WIDGET_ENTRY_TCC
