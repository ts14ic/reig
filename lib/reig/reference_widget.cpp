#include "reference_widget.h"
#include "internal.h"
#include "context.h"

namespace reig::reference_widget {
    using namespace primitive;

    struct ButtonModel {
        const char* title = nullptr;
        Rectangle outlineArea;
        Rectangle baseArea;
        bool hoveringOverArea = false;
        bool justClicked = false;
        bool holdingClick = false;
    };

    template <typename Button>
    ButtonModel get_button_model(Context& ctx, const Button& button) {
        auto focusId = ctx.focus.create_id();
        Rectangle outlineArea = button.mBoundingBox;
        ctx.fit_rect_in_window(outlineArea);

        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
        bool hoveringOverArea = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), outlineArea);
        bool isFocused = ctx.focus.handle(focusId, hoveringOverArea);

        bool justClicked = false;
        bool holdingClick = false;
        if (isFocused) {
            bool clickedInArea = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), outlineArea);
            justClicked = ctx.mouse.leftButton.is_clicked() && clickedInArea;
            holdingClick = ctx.mouse.leftButton.is_pressed() && clickedInArea;

            if (holdingClick) {
                baseArea = internal::decrease_rect(baseArea, 4);
            }
        }

        return {nullptr, outlineArea, baseArea, hoveringOverArea, justClicked, holdingClick};
    }

    bool button::use(Context& ctx) const {
        auto model = get_button_model(ctx, *this);

        Color innerColor{mBaseColor};
        if (model.hoveringOverArea) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        if (model.holdingClick) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        ctx.render_rectangle(model.outlineArea, internal::get_yiq_contrast(mBaseColor));
        ctx.render_rectangle(model.baseArea, innerColor);
        ctx.render_text(mTitle.c_str(), model.baseArea);

        return model.justClicked;
    }

    void render_colored_button(Context& ctx, const ButtonModel& model, const Color& primaryColor) {
        Color innerColor{primaryColor};
        if (model.hoveringOverArea) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        if (model.holdingClick) {
            innerColor = internal::lighten_color_by(innerColor, 30);
        }
        ctx.render_rectangle(model.outlineArea, internal::get_yiq_contrast(innerColor));
        ctx.render_rectangle(model.baseArea, innerColor);
        ctx.render_text(model.title, model.baseArea);
    }

    void button::use(reig::Context& ctx, std::function<void()> callback) {
        Rectangle outlineArea = mBoundingBox;
        ctx.fit_rect_in_window(outlineArea);

        ctx.with_focus(outlineArea, [=, *this, &ctx](const Focus2& focus) {
            Rectangle baseArea = internal::decrease_rect(outlineArea, 4);

            ButtonModel model;
            model.title = mTitle.c_str();
            model.outlineArea = outlineArea;
            model.baseArea = baseArea;

            switch (focus) {
                case Focus2::HOVER: {
                    model.hoveringOverArea = true;
                    break;
                }

                case Focus2::CLICK: {
                    model.justClicked = true;
                    break;
                }

                case Focus2::HOLD: {
                    model.holdingClick = true;
                    model.baseArea = internal::decrease_rect(baseArea, 2);
                    break;
                }

                default: {
                    break;
                }
            }

            render_colored_button(ctx, model, mBaseColor);

            if (model.justClicked) {
                callback();
            }
        });
    }

    bool textured_button::use(Context& ctx) const {
        auto model = get_button_model(ctx, *this);

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

    struct CheckboxModel {
        Rectangle baseArea;
        Rectangle outlineArea;
        Rectangle checkArea;
        bool isFocused = false;
        bool hoveringOverArea = false;
        bool valueChanged = false;
    };

    template <typename Checkbox>
    CheckboxModel get_checkbox_model(Context& ctx, const Checkbox& checkbox) {
        auto focusId = ctx.focus.create_id();
        Rectangle outlineArea = checkbox.mBoundingBox;
        ctx.fit_rect_in_window(outlineArea);

        bool hoveringOverArea = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), outlineArea);
        bool isFocused = ctx.focus.handle(focusId, hoveringOverArea);

        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);
        Rectangle checkArea = internal::decrease_rect(baseArea, 4);

        bool justClicked = ctx.mouse.leftButton.is_clicked()
                           && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
        if (isFocused) {
            if (justClicked) {
                checkbox.mValueRef = !checkbox.mValueRef;
            }

            bool holdingClick = ctx.mouse.leftButton.is_pressed()
                                && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
            if (holdingClick) {
                baseArea = internal::decrease_rect(baseArea, 4);
                checkArea = internal::decrease_rect(checkArea, 4);
            }
        }

        return {baseArea, outlineArea, checkArea, isFocused, hoveringOverArea, justClicked};
    }

    bool checkbox::use(Context& ctx) const {
        auto model = get_checkbox_model(ctx, *this);

        Color secondaryColor = internal::get_yiq_contrast(mBaseColor);
        ctx.render_rectangle(model.outlineArea, secondaryColor);
        ctx.render_rectangle(model.baseArea,
                             model.hoveringOverArea && model.isFocused
                             ? internal::lighten_color_by(mBaseColor, 30)
                             : mBaseColor);

        if (mValueRef) {
            ctx.render_rectangle(model.checkArea, secondaryColor);
        }

        return model.valueChanged;
    }

    bool textured_checkbox::use(Context& ctx) const {
        auto model = get_checkbox_model(ctx, *this);

        ctx.render_rectangle(model.outlineArea, mBaseTexture);

        if (mValueRef) {
            ctx.render_rectangle(model.checkArea, mCheckTexture);
        }

        return model.valueChanged;
    }
}
