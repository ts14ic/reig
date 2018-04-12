#include "reference_widget.h"
#include "internal.h"
#include "context.h"

using namespace reig::primitive;
namespace internal = reig::internal;

struct ButtonModel {
    Rectangle outlineArea;
    Rectangle baseArea;
    bool hoveringOverArea = false;
    bool justClicked = false;
    bool holdingClick = false;
};

template <typename B>
ButtonModel get_button_model(reig::Context& ctx, const B& button) {
    Rectangle baseArea {button.mBoundingBox};
    ctx.fit_rect_in_window(baseArea);

    bool hoveringOverArea = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), baseArea);
    Rectangle outlineArea {baseArea};
    baseArea = internal::decrease_rect(baseArea, 4);
    bool clickedInArea = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
    bool justClicked = ctx.mouse.leftButton.is_clicked() && clickedInArea;
    bool holdingClick = ctx.mouse.leftButton.is_pressed() && clickedInArea;

    return {outlineArea, baseArea, hoveringOverArea, justClicked, holdingClick};
}

bool reig::reference_widget::button::draw(reig::Context& ctx) const {
    auto model = get_button_model(ctx, *this);

    Color innerColor {mBaseColor};
    if (model.hoveringOverArea) {
        innerColor = internal::lighten_color_by(innerColor, 30);
    }
    if (model.holdingClick) {
        innerColor = internal::lighten_color_by(innerColor, 30);
    }
    ctx.render_rectangle(model.outlineArea, internal::get_yiq_contrast(mBaseColor));
    ctx.render_rectangle(model.baseArea, innerColor);
    ctx.render_text(mTitle, model.baseArea);

    return model.justClicked;
}

bool reig::reference_widget::textured_button::draw(reig::Context& ctx) const {
    auto model = get_button_model(ctx, *this);

    int texture = mBaseTexture;
    if(model.holdingClick || model.hoveringOverArea) {
        texture = mHoverTexture;
    }
    ctx.render_rectangle(model.outlineArea, texture);
    ctx.render_text(mTitle, model.outlineArea);

    return model.justClicked;
}

void reig::reference_widget::label::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);
    ctx.render_text(mTitle, boundingBox);
}

struct SliderValues {
    float min = 0.0f;
    float max = 0.0f;
    float value = 0.0f;
    int offset = 0;
    int valuesNum = 0;
};

SliderValues prepare_slider_values(float aMin, float aMax, float aValue, float aStep) {
    float min = internal::min(aMin, aMax);
    float max = internal::max(aMin, aMax);
    return SliderValues{
            min, max, internal::clamp(aValue, min, max),
            static_cast<int>((aValue - min) / aStep),
            static_cast<int>((max - min) / aStep + 1)
    };
};

void size_slider_cursor(float& coord, float& size, int valuesNum, int offset) {
    size /= valuesNum;
    if(size < 1) size = 1;
    coord += offset * size;
}

void progress_slider_value(float mouseCursorCoord, float cursorSize, float cursorCoord,
                           float min, float max, float step, float& value) {
    auto halfCursorSize = cursorSize / 2;
    auto distanceToMouseCoord = mouseCursorCoord - cursorCoord - halfCursorSize;

    if (internal::abs(distanceToMouseCoord) > halfCursorSize) {
        value += static_cast<int>(distanceToMouseCoord / cursorSize) * step;
        value = internal::clamp(value, min, max);
    }
}

enum class SliderOrientation {
    HORIZONTAL, VERTICAL
};

SliderOrientation calculate_slider_orientation(const Rectangle& rect) {
    return rect.height > rect.width
           ? SliderOrientation::VERTICAL
           : SliderOrientation::HORIZONTAL;
}

struct SliderModel {
    Rectangle baseArea;
    Rectangle outlineArea;
    Rectangle cursorArea;
    bool hoveringOverCursor = false;
    bool holdingClickOnCursor = false;
    float value = 0.0f;
};

template <typename Slider>
SliderModel get_slider_model(reig::Context& ctx, const Slider& slider) {
    Rectangle baseArea = slider.mBoundingBox;
    ctx.fit_rect_in_window(baseArea);

    Rectangle outlineArea = baseArea;
    baseArea = internal::decrease_rect(baseArea, 4);

    auto [min, max, value, offset, valuesNum] = prepare_slider_values(slider.mMin, slider.mMax, slider.mValueRef, slider.mStep);

    SliderOrientation orientation = calculate_slider_orientation(baseArea);

    auto cursorArea = internal::decrease_rect(baseArea, 4);
    if(orientation == SliderOrientation::HORIZONTAL) {
        size_slider_cursor(cursorArea.x, cursorArea.width, valuesNum, offset);
    } else {
        size_slider_cursor(cursorArea.y, cursorArea.height, valuesNum, offset);
    }

    bool hoveringOverCursor = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), cursorArea);
    bool holdingClick = ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
    bool holdingClickOnCursor = ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), cursorArea);

    if (holdingClick) {
        if(orientation == SliderOrientation::HORIZONTAL) {
            progress_slider_value(ctx.mouse.get_cursor_pos().x, cursorArea.width, cursorArea.x, min, max, slider.mStep, value);
        } else {
            progress_slider_value(ctx.mouse.get_cursor_pos().y, cursorArea.height, cursorArea.y, min, max, slider.mStep, value);
        }
    } else if (ctx.mouse.get_scrolled() != 0 && internal::is_boxed_in(ctx.mouse.get_cursor_pos(), baseArea)) {
        value += static_cast<int>(ctx.mouse.get_scrolled()) * slider.mStep;
        value = internal::clamp(value, min, max);
    }

    return {baseArea, outlineArea, cursorArea, hoveringOverCursor, holdingClickOnCursor, value};
}

bool reig::reference_widget::slider::draw(reig::Context& ctx) const {
    auto model = get_slider_model(ctx, *this);

    Color baseColor = internal::get_yiq_contrast(mBaseColor);
    ctx.render_rectangle(model.outlineArea, baseColor);
    ctx.render_rectangle(model.baseArea, mBaseColor);

    if (model.hoveringOverCursor) {
        baseColor = internal::lighten_color_by(baseColor, 30);
    }
    if (model.holdingClickOnCursor) {
        baseColor = internal::lighten_color_by(baseColor, 30);
    }
    ctx.render_rectangle(model.cursorArea, baseColor);

    if (mValueRef != model.value) {
        mValueRef = model.value;
        return true;
    } else {
        return false;
    }
}

void size_scrollbar_cursor(float& coord, float& size, float step, int offset, float viewSize) {
    float scale = size / viewSize;
    size *= scale;
    if(size < 1) size = 1;
    coord += offset * step * scale;
}

bool reig::reference_widget::scrollbar::draw(reig::Context& ctx) const {
    Rectangle boundingBox = {mBoundingBox};
    ctx.fit_rect_in_window(boundingBox);

    internal::render_widget_frame(ctx, boundingBox, mBaseColor);

    auto step = ctx.get_font_size() / 2.0f;
    auto [min, max, value, offset, valuesNum] = prepare_slider_values(0.0f, mViewSize - boundingBox.height, mValueRef, step);

    SliderOrientation orientation = calculate_slider_orientation(boundingBox);

    // Render the cursor
    auto cursorBox = internal::decrease_rect(boundingBox, 4);
    if(orientation == SliderOrientation::HORIZONTAL) {
        size_scrollbar_cursor(cursorBox.x, cursorBox.width, step, offset, mViewSize);
    } else {
        size_scrollbar_cursor(cursorBox.y, cursorBox.height, step, offset, mViewSize);
    }

    auto cursorColor = internal::get_yiq_contrast(mBaseColor);
    if (internal::is_boxed_in(ctx.mouse.get_cursor_pos(), cursorBox)) {
        cursorColor = internal::lighten_color_by(cursorColor, 50);
    }
    ctx.render_rectangle(cursorBox, cursorColor);

    if (ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        if(orientation == SliderOrientation::HORIZONTAL) {
            progress_slider_value(ctx.mouse.get_cursor_pos().x, cursorBox.width, cursorBox.x, min, max, step, value);
        } else {
            progress_slider_value(ctx.mouse.get_cursor_pos().y, cursorBox.height, cursorBox.y, min, max, step, value);
        }
    } else if (ctx.mouse.get_scrolled() != 0 && internal::is_boxed_in(ctx.mouse.get_cursor_pos(), boundingBox)) {
        value += static_cast<int>(ctx.mouse.get_scrolled()) * step;
        value = internal::clamp(value, min, max);
    }

    if (mValueRef != value) {
        mValueRef = value;
        return true;
    } else {
        return false;
    }
}

bool reig::reference_widget::textured_slider::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);

    // Render slider's base
    ctx.render_rectangle(boundingBox, mBaseTexture);

    auto [min, max, value, offset, valuesNum] = prepare_slider_values(mMin, mMax, mValueRef, mStep);

    // Render the cursor
    auto cursorBox = internal::decrease_rect(boundingBox, 8);
    auto orientation = calculate_slider_orientation(boundingBox);
    if(orientation == SliderOrientation::HORIZONTAL) {
        size_slider_cursor(cursorBox.x, cursorBox.width, valuesNum, offset);
    } else {
        size_slider_cursor(cursorBox.y, cursorBox.height, valuesNum, offset);
    }
    ctx.render_rectangle(cursorBox, mCursorTexture);

    if (ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        if(orientation == SliderOrientation::HORIZONTAL) {
            progress_slider_value(ctx.mouse.get_cursor_pos().x, cursorBox.width, cursorBox.x, min, max, mStep, value);
        } else {
            progress_slider_value(ctx.mouse.get_cursor_pos().y, cursorBox.height, cursorBox.y, min, max, mStep, value);
        }
    } else if (ctx.mouse.get_scrolled() != 0 && internal::is_boxed_in(ctx.mouse.get_cursor_pos(), boundingBox)) {
        value += static_cast<int>(ctx.mouse.get_scrolled()) * mStep;
        value = internal::clamp(value, min, max);
    }

    if (mValueRef != value) {
        mValueRef = value;
        return true;
    } else {
        return false;
    }
}

bool reig::reference_widget::checkbox::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);

    internal::render_widget_frame(ctx, boundingBox, mBaseColor);

    // Render check
    if (mValueRef) {
        boundingBox = internal::decrease_rect(boundingBox, 4);
        Color contrastColor = internal::get_yiq_contrast(mBaseColor);
        ctx.render_rectangle(boundingBox, contrastColor);
    }

    // True if state changed
    if (ctx.mouse.leftButton.is_clicked() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        mValueRef = !mValueRef;
        return true;
    } else {
        return false;
    }
}

bool reig::reference_widget::textured_checkbox::draw(reig::Context& ctx) const {
    Rectangle boundingBox = this->mBoundingBox;
    ctx.fit_rect_in_window(boundingBox);

    // Render checkbox's base
    ctx.render_rectangle(boundingBox, mBaseTexture);

    // Render check
    if (mValueRef) {
        boundingBox = internal::decrease_rect(boundingBox, 8);
        ctx.render_rectangle(boundingBox, mCheckTexture);
    }

    // True if state changed
    if (ctx.mouse.leftButton.is_clicked() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox)) {
        mValueRef = !mValueRef;
        return true;
    } else {
        return false;
    }
}