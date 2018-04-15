#include "reference_widget.h"
#include "internal.h"
#include "context.h"

using namespace reig::primitive;
namespace internal = reig::internal;

struct ButtonModel {
    Rectangle outlineArea;
    Rectangle baseArea;
    bool isFocused = false;
    bool hoveringOverArea = false;
    bool justClicked = false;
    bool holdingClick = false;
};

template <typename Button>
ButtonModel get_button_model(reig::Context& ctx, const Button& button) {
    auto focusId = ctx.focus.create_id();
    Rectangle outlineArea = button.mBoundingBox;
    ctx.fit_rect_in_window(outlineArea);

    Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
    bool hoveringOverArea = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), outlineArea);
    if (hoveringOverArea) {
        ctx.focus.claim(focusId);
    } else {
        ctx.focus.release(focusId);
    }

    bool justClicked = false;
    bool holdingClick = false;
    if (ctx.focus.is_focused(focusId)) {
        bool clickedInArea = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), outlineArea);
        justClicked = ctx.mouse.leftButton.is_clicked() && clickedInArea;
        holdingClick = ctx.mouse.leftButton.is_pressed() && clickedInArea;

        if (holdingClick) {
            baseArea = internal::decrease_rect(baseArea, 4);
        }
    }

    return {outlineArea, baseArea, ctx.focus.is_focused(focusId), hoveringOverArea, justClicked, holdingClick};
}

bool reig::reference_widget::button::use(reig::Context& ctx) const {
    auto model = get_button_model(ctx, *this);

    Color innerColor{mBaseColor};
    if (model.hoveringOverArea && model.isFocused) {
        innerColor = internal::lighten_color_by(innerColor, 30);
    }
    if (model.holdingClick && model.isFocused) {
        innerColor = internal::lighten_color_by(innerColor, 30);
    }
    ctx.render_rectangle(model.outlineArea, internal::get_yiq_contrast(mBaseColor));
    ctx.render_rectangle(model.baseArea, innerColor);
    ctx.render_text(mTitle, model.baseArea);

    return model.justClicked;
}

bool reig::reference_widget::textured_button::use(reig::Context& ctx) const {
    auto model = get_button_model(ctx, *this);

    int texture = mBaseTexture;
    if (model.holdingClick || model.hoveringOverArea) {
        texture = mHoverTexture;
    }
    ctx.render_rectangle(model.outlineArea, texture);
    ctx.render_text(mTitle, model.outlineArea);

    return model.justClicked;
}

void reig::reference_widget::label::use(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);
    ctx.render_text(mTitle, boundingBox);
}

struct CheckboxModel {
    Rectangle baseArea;
    Rectangle outlineArea;
    Rectangle checkArea;
    bool valueChanged = false;
};

template <typename Checkbox>
CheckboxModel get_checkbox_model(reig::Context& ctx, const Checkbox& checkbox) {
    Rectangle outlineArea = checkbox.mBoundingBox;
    ctx.fit_rect_in_window(outlineArea);

    Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
    Rectangle checkArea = internal::decrease_rect(baseArea, 4);

    bool justClicked = ctx.mouse.leftButton.is_clicked()
                       && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
    bool holdingClick = ctx.mouse.leftButton.is_pressed()
                        && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
    if (justClicked) {
        checkbox.mValueRef = !checkbox.mValueRef;
    }
    if (holdingClick) {
        baseArea = internal::decrease_rect(baseArea, 4);
        checkArea = internal::decrease_rect(checkArea, 4);
    }

    return {baseArea, outlineArea, checkArea, justClicked};
}

bool reig::reference_widget::checkbox::use(reig::Context& ctx) const {
    auto model = get_checkbox_model(ctx, *this);

    Color secondaryColor = internal::get_yiq_contrast(mBaseColor);
    ctx.render_rectangle(model.outlineArea, secondaryColor);
    ctx.render_rectangle(model.baseArea, mBaseColor);

    if (mValueRef) {
        ctx.render_rectangle(model.checkArea, secondaryColor);
    }

    return model.valueChanged;
}

bool reig::reference_widget::textured_checkbox::use(reig::Context& ctx) const {
    auto model = get_checkbox_model(ctx, *this);

    ctx.render_rectangle(model.outlineArea, mBaseTexture);

    if (mValueRef) {
        ctx.render_rectangle(model.checkArea, mCheckTexture);
    }

    return model.valueChanged;
}