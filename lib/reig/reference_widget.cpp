#include "reference_widget.h"
#include "internal.h"
#include "context.h"

namespace reig::reference_widget {
    using namespace primitive;

    struct ButtonModel {
        Rectangle outlineArea;
        Rectangle baseArea;
        bool hoveringOverArea = false;
        bool justClicked = false;
        bool holdingClick = false;
    };

    ButtonModel get_button_model(Context& ctx, Rectangle outlineArea) {
        ctx.fit_rect_in_window(outlineArea);

        ButtonModel model;
        model.hoveringOverArea = ctx.mouse.is_hovering_over_rect(outlineArea);
        model.justClicked = ctx.mouse.leftButton.just_clicked_in_rect(outlineArea);
        model.holdingClick = model.hoveringOverArea
                             && ctx.mouse.leftButton.clicked_in_rect(outlineArea)
                             && ctx.mouse.leftButton.is_held();

        model.outlineArea = outlineArea;
        model.baseArea = model.holdingClick
                         ? internal::decrease_rect(outlineArea, 6)
                         : internal::decrease_rect(outlineArea, 4);

        return model;
    }

    bool button::use(reig::Context& ctx) const {
        auto model = get_button_model(ctx, mBoundingBox);

        Color innerColor{mBaseColor};
        if (model.hoveringOverArea) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        if (model.holdingClick) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        ctx.render_rectangle(model.outlineArea, internal::get_yiq_contrast(innerColor));
        ctx.render_rectangle(model.baseArea, innerColor);
        ctx.render_text(mTitle, model.baseArea);

        return model.justClicked;
    }

    bool textured_button::use(Context& ctx) const {
        auto model = get_button_model(ctx, mBoundingBox);

        int texture = mBaseTexture;
        if (model.holdingClick || model.hoveringOverArea) {
            texture = mHoverTexture;
        }
        ctx.render_rectangle(model.outlineArea, texture);
        ctx.render_text(mTitle, model.outlineArea);

        return model.justClicked;
    }

    void label::use(Context& ctx) const {
        Rectangle boundingBox = this->mBoundingBox;
        ctx.fit_rect_in_window(boundingBox);
        ctx.render_text(mTitle, boundingBox, mAlignment, mFontScale);
    }

//    struct CheckboxModel {
//        Rectangle baseArea;
//        Rectangle outlineArea;
//        Rectangle checkArea;
//        bool hoveringOverArea = false;
//        bool valueChanged = false;
//    };
//
//    template <typename Checkbox>
//    CheckboxModel get_checkbox_model(Context& ctx, const Checkbox& checkbox, const Rectangle& outlineArea, const Focus& focus) {
//        bool hoveringOverArea = focus == Focus::HOVER;
//
//        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
//        Rectangle checkArea = internal::decrease_rect(baseArea, 4);
//
//        bool justClicked = focus == Focus::CLICK;
//        if (justClicked) {
//            baseArea = internal::decrease_rect(baseArea, 4);
//            checkArea = internal::decrease_rect(checkArea, 4);
//            checkbox.mValueRef = !checkbox.mValueRef;
//        }
//
//        bool holdingClick = focus == Focus::HOLD;
//        if (holdingClick) {
//            baseArea = internal::decrease_rect(baseArea, 4);
//            checkArea = internal::decrease_rect(checkArea, 4);
//        }
//
//        return {baseArea, outlineArea, checkArea, hoveringOverArea, justClicked};
//    }
//
//    void checkbox::use(Context& ctx, std::function<void()> callback) const {
//        Rectangle outlineArea = mBoundingBox;
//        ctx.fit_rect_in_window(outlineArea);
//
//        ctx.with_focus(outlineArea, [=, *this, &ctx](const Focus& focus) {
//            auto model = get_checkbox_model(ctx, *this, outlineArea, focus);
//
//            Color secondaryColor = internal::get_yiq_contrast(mBaseColor);
//            ctx.render_rectangle(model.outlineArea, secondaryColor);
//            ctx.render_rectangle(model.baseArea,
//                                 model.hoveringOverArea
//                                 ? internal::lighten_color_by(mBaseColor, 30)
//                                 : mBaseColor);
//
//            if (mValueRef) {
//                ctx.render_rectangle(model.checkArea, secondaryColor);
//            }
//
//            if (model.valueChanged) {
//                callback();
//            }
//        });
//    }
//
//    void textured_checkbox::use(Context& ctx, std::function<void()> callback) const {
//        Rectangle outlineArea = mBoundingBox;
//        ctx.fit_rect_in_window(outlineArea);
//
//        ctx.with_focus(outlineArea, [=, *this, &ctx](const Focus& focus2) {
//            auto model = get_checkbox_model(ctx, *this, outlineArea, focus2);
//
//            ctx.render_rectangle(model.outlineArea, mBaseTexture);
//
//            if (mValueRef) {
//                ctx.render_rectangle(model.checkArea, mCheckTexture);
//            }
//
//            if (model.valueChanged) {
//                callback();
//            }
//        });
//    }
}
