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

    bool button(Context& ctx, const char* aTitle, Rectangle boundingBox, Color baseColor) {
        auto model = get_button_model(ctx, boundingBox);

        Color innerColor{baseColor};
        if (model.hoveringOverArea) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        if (model.holdingClick) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        ctx.render_rectangle(model.outlineArea, internal::get_yiq_contrast(innerColor));
        ctx.render_rectangle(model.baseArea, innerColor);
        ctx.render_text(aTitle, model.baseArea);

        return model.justClicked;
    }

    bool textured_button(Context& ctx, const char* title, Rectangle boundingBox,
                         int mHoverTexture, int mBaseTexture) {
        auto model = get_button_model(ctx, boundingBox);

        int texture = mBaseTexture;
        if (model.holdingClick || model.hoveringOverArea) {
            texture = mHoverTexture;
        }
        ctx.render_rectangle(model.outlineArea, texture);
        ctx.render_text(title, model.outlineArea);

        return model.justClicked;
    }

    void label(Context& ctx, char const* title, Rectangle boundingBox, text::Alignment alignment, float fontScale) {
        ctx.fit_rect_in_window(boundingBox);
        ctx.render_text(title, boundingBox, alignment, fontScale);
    }

    struct CheckboxModel {
        Rectangle baseArea;
        Rectangle outlineArea;
        Rectangle checkArea;
        bool hoveringOverArea = false;
        bool valueChanged = false;
    };

    CheckboxModel get_checkbox_model(Context& ctx, Rectangle outlineArea, bool& aValueRef) {
        ctx.fit_rect_in_window(outlineArea);
        bool hoveringOverArea = ctx.mouse.is_hovering_over_rect(outlineArea);

        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
        Rectangle checkArea = internal::decrease_rect(baseArea, 4);

        bool justClicked = ctx.mouse.leftButton.just_clicked_in_rect(outlineArea);
        if (justClicked) {
            baseArea = internal::decrease_rect(baseArea, 4);
            checkArea = internal::decrease_rect(checkArea, 4);
            aValueRef = !aValueRef;
        }

        bool holdingClick = ctx.mouse.leftButton.clicked_in_rect(outlineArea) && ctx.mouse.leftButton.is_held();
        if (holdingClick) {
            baseArea = internal::decrease_rect(baseArea, 4);
            checkArea = internal::decrease_rect(checkArea, 4);
        }

        return {baseArea, outlineArea, checkArea, hoveringOverArea, justClicked};
    }

    bool checkbox(Context& ctx, Rectangle boundingBox, Color baseColor, bool& valueRef) {
        auto model = get_checkbox_model(ctx, boundingBox, valueRef);

        Color secondaryColor = internal::get_yiq_contrast(baseColor);
        ctx.render_rectangle(model.outlineArea, secondaryColor);
        ctx.render_rectangle(model.baseArea,
                             model.hoveringOverArea
                             ? internal::lighten_color_by(baseColor, 30)
                             : baseColor);

        if (valueRef) {
            ctx.render_rectangle(model.checkArea, secondaryColor);
        }

        return model.valueChanged;
    }

    bool textured_checkbox(Context& ctx, primitive::Rectangle boundingBox, int baseTexture, int checkTexture,
                                bool& valueRef) {
        auto model = get_checkbox_model(ctx, boundingBox, valueRef);

        ctx.render_rectangle(model.outlineArea, baseTexture);

        if (valueRef) {
            ctx.render_rectangle(model.checkArea, checkTexture);
        }

        return model.valueChanged;
    }
}
