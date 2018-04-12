#include "reference_widget.h"
#include "internal.h"
#include "context.h"

using namespace reig::primitive;
namespace internal = reig::internal;

bool reig::reference_widget::button::draw(reig::Context& ctx) const {
    Rectangle box = this->mBoundingBox;
    ctx.fit_rect_in_window(box);

    // Render button outline first
    Color outlineCol = internal::get_yiq_contrast(mBaseColor);
    ctx.render_rectangle(box, outlineCol);

    Color color = this->mBaseColor;
    // if cursor is over the button, highlight it
    if (internal::is_boxed_in(ctx.mouse.get_cursor_pos(), box)) {
        color = internal::lighten_color_by(color, 50);
    }

    // see, if clicked the inner part of button
    box = internal::decrease_box(box, 4);
    bool clickedInBox = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), box);

    // highlight even more, if clicked
    if (ctx.mouse.leftButton.is_pressed() && clickedInBox) {
        color = internal::lighten_color_by(color, 50);
    }

    // render the inner part of button
    ctx.render_rectangle(box, color);
    // render button's title
    ctx.render_text(mTitle, box);

    return ctx.mouse.leftButton.is_clicked() && clickedInBox;
}


bool reig::reference_widget::textured_button::draw(reig::Context& ctx) const {
    Rectangle box = this->mBoundingBox;
    ctx.fit_rect_in_window(box);

    bool clickedInBox = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), box);
    bool hoveredOnBox = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), box);

    if((ctx.mouse.leftButton.is_pressed() && clickedInBox) || hoveredOnBox) {
        ctx.render_rectangle(box, mBaseTexture);
    } else {
        ctx.render_rectangle(box, mHoverTexture);
    }

    box = internal::decrease_box(box, 8);
    ctx.render_text(mTitle, box);

    return ctx.mouse.leftButton.is_clicked() && clickedInBox;
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

bool reig::reference_widget::slider::draw(reig::Context& ctx) const {
    Rectangle boundingBox = {mBoundingBox};
    ctx.fit_rect_in_window(boundingBox);

    internal::render_widget_frame(ctx, boundingBox, mBaseColor);

    auto [min, max, value, offset, valuesNum] = prepare_slider_values(mMin, mMax, mValueRef, mStep);

    SliderOrientation orientation = calculate_slider_orientation(boundingBox);

    // Render the cursor
    auto cursorBox = internal::decrease_box(boundingBox, 4);
    if(orientation == SliderOrientation::HORIZONTAL) {
        size_slider_cursor(cursorBox.x, cursorBox.width, valuesNum, offset);
    } else {
        size_slider_cursor(cursorBox.y, cursorBox.height, valuesNum, offset);
    }

    auto cursorColor = internal::get_yiq_contrast(mBaseColor);
    if (internal::is_boxed_in(ctx.mouse.get_cursor_pos(), cursorBox)) {
        cursorColor = internal::lighten_color_by(cursorColor, 50);
    }
    ctx.render_rectangle(cursorBox, cursorColor);

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
    auto cursorBox = internal::decrease_box(boundingBox, 4);
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
    auto cursorBox = internal::decrease_box(boundingBox, 8);
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
        boundingBox = internal::decrease_box(boundingBox, 4);
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
        boundingBox = internal::decrease_box(boundingBox, 8);
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