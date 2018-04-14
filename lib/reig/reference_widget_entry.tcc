#ifndef REIG_REFERENCE_WIDGET_ENTRY_TCC
#define REIG_REFERENCE_WIDGET_ENTRY_TCC

#include "internal.h"

namespace reig::reference_widget {
    template <template <class, class> typename Entry, typename String, typename Action>
    auto detail::get_entry_model(reig::Context& ctx, const Entry<String, Action>& entry) -> EntryModel {
        Rectangle outlineArea = entry.mBoundingArea;
        ctx.fit_rect_in_window(outlineArea);

        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
        bool hoveringOverArea = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), outlineArea);
        bool clickedInArea = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
        bool isInputModified = false;
        if (clickedInArea) {
            baseArea = internal::decrease_rect(baseArea, 4);

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

        return EntryModel{outlineArea, baseArea, clickedInArea, hoveringOverArea, isInputModified};
    }

    template <typename Char, typename Action>
    void detail::ref_entry<Char, Action>::use(reig::Context& ctx) const {
        auto model = get_entry_model(ctx, *this);

        ctx.render_rectangle(model.outlineArea, internal::get_yiq_contrast(mPrimaryColor));
        auto primaryColor = mPrimaryColor;
        if (model.isHoveringOverArea) {
            primaryColor = internal::lighten_color_by(primaryColor, 30);
        }
        ctx.render_rectangle(model.baseArea, primaryColor);
        if (model.isSelected) {
            ctx.render_text(mValueRef.c_str(), model.baseArea);
        } else {
            ctx.render_text(mValueRef.empty() ? mTitle : mValueRef.c_str(), model.baseArea);
        }

        if (model.isInputModified) {
            mAction(mValueRef);
        }
    }
}

#endif //REIG_REFERENCE_WIDGET_ENTRY_TCC
