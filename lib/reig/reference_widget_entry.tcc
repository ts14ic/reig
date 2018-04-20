#ifndef REIG_REFERENCE_WIDGET_ENTRY_TCC
#define REIG_REFERENCE_WIDGET_ENTRY_TCC

#include "reference_widget_entry.h"
#include "internal.h"

namespace reig::reference_widget {
//    namespace detail {
//        struct EntryModel {
//            const Rectangle outlineArea;
//            const Rectangle baseArea;
//            const Rectangle caretArea;
//            const bool isSelected = false;
//            const bool isHoveringOverArea = false;
//        };
//
//        template <typename E>
//        void display_entry_model(Context& ctx, const EntryModel& model, const E& entry);
//    }
//
//    template <typename Char, typename Action>
//    void detail::ref_entry<Char, Action>::use(reig::Context& ctx) const {
//        Rectangle outlineArea = mBoundingArea;
//        ctx.fit_rect_in_window(outlineArea);
//
//        ctx.with_focus(outlineArea, [=, *this, &ctx](const Focus& focus) {
//            Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
//            Rectangle caretArea {0, baseArea.y, 0, baseArea.height};
//            bool hoveringOverArea = focus == Focus::HOVER;
//            bool isSelected = focus == Focus::SELECT;
//
//            bool isInputModified = false;
//            if (isSelected) {
//                baseArea = internal::decrease_rect(baseArea, 4);
//
//                if ((ctx.get_frame_counter() / 30) % 2 == 0) {
//                    caretArea = internal::decrease_rect(caretArea, 10);
//                    caretArea.width = 2;
//                }
//
//                Key keyType = ctx.keyboard.get_pressed_key_type();
//                switch (keyType) {
//                    case Key::CHAR: {
//                        mValueRef += ctx.keyboard.get_pressed_char();
//                        isInputModified = true;
//                        break;
//                    }
//
//                    case Key::BACKSPACE: {
//                        using std::empty;
//                        if (!empty(mValueRef)) {
//                            mValueRef.pop_back();
//                            isInputModified = true;
//                        }
//                        break;
//                    }
//
//                    case Key::RETURN:
//                    case Key::ESCAPE: {
//                        ctx.mouse.leftButton.press(outlineArea.x, outlineArea.y);
//                        ctx.mouse.leftButton.release();
//                        break;
//                    }
//
//                    default: {
//                        break;
//                    }
//                }
//            }
//
//            EntryModel model{outlineArea, baseArea, caretArea, isSelected, hoveringOverArea};
//
//            display_entry_model(ctx, model, *this);
//
//            if (isInputModified) {
//                mAction(mValueRef);
//            }
//        });
//    }
//
//    template <typename E>
//    void detail::display_entry_model(reig::Context& ctx, const reig::reference_widget::detail::EntryModel& model, const E& entry) {
//        Color secondaryColor = internal::get_yiq_contrast(entry.mPrimaryColor);
//
//        ctx.render_rectangle(model.outlineArea, secondaryColor);
//        auto primaryColor = entry.mPrimaryColor;
//        if (model.isHoveringOverArea) {
//            primaryColor = internal::lighten_color_by(primaryColor, 30);
//        }
//        ctx.render_rectangle(model.baseArea, primaryColor);
//        if (model.isSelected) {
//            float caretX = ctx.render_text(entry.mValueRef.c_str(), model.baseArea, text::Alignment::LEFT);
//
//            Rectangle caretArea = model.caretArea;
//            caretArea.x = caretX;
//            internal::trim_rect_in_other(caretArea, model.baseArea);
//            ctx.render_rectangle(caretArea, secondaryColor);
//        } else {
//            ctx.render_text(entry.mValueRef.empty() ? entry.mTitle : entry.mValueRef.c_str(), model.baseArea, text::Alignment::LEFT);
//        }
//    }
}

#endif //REIG_REFERENCE_WIDGET_ENTRY_TCC
