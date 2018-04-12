#include "reference_widget.h"
#include "internal.h"
#include "context.h"

using namespace reig::primitive;
namespace internal = reig::internal;

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

    auto values = prepare_slider_values(slider.mMin, slider.mMax, slider.mValueRef, slider.mStep);

    SliderOrientation orientation = calculate_slider_orientation(baseArea);

    auto cursorArea = internal::decrease_rect(baseArea, 4);
    if(orientation == SliderOrientation::HORIZONTAL) {
        size_slider_cursor(cursorArea.x, cursorArea.width, values.valuesNum, values.offset);
    } else {
        size_slider_cursor(cursorArea.y, cursorArea.height, values.valuesNum, values.offset);
    }

    bool hoveringOverCursor = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), cursorArea);
    bool holdingClick = ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
    bool holdingClickOnCursor = ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), cursorArea);

    if (holdingClick) {
        if(orientation == SliderOrientation::HORIZONTAL) {
            progress_slider_value(ctx.mouse.get_cursor_pos().x, cursorArea.width, cursorArea.x,
                                  values.min, values.max, slider.mStep, values.value);
        } else {
            progress_slider_value(ctx.mouse.get_cursor_pos().y, cursorArea.height, cursorArea.y,
                                  values.min, values.max, slider.mStep, values.value);
        }
    } else if (ctx.mouse.get_scrolled() != 0 && internal::is_boxed_in(ctx.mouse.get_cursor_pos(), baseArea)) {
        values.value += static_cast<int>(ctx.mouse.get_scrolled()) * slider.mStep;
        values.value = internal::clamp(values.value, values.min, values.max);
    }

    return {baseArea, outlineArea, cursorArea, hoveringOverCursor, holdingClickOnCursor, values.value};
}

template <typename Slider>
void draw_slider_model(reig::Context& ctx, const SliderModel& model, const Slider& slider) {
    Color frameColor = internal::get_yiq_contrast(slider.mBaseColor);
    ctx.render_rectangle(model.outlineArea, frameColor);
    ctx.render_rectangle(model.baseArea, slider.mBaseColor);

    if (model.hoveringOverCursor) {
        frameColor = internal::lighten_color_by(frameColor, 30);
    }
    if (model.holdingClickOnCursor) {
        frameColor = internal::lighten_color_by(frameColor, 30);
    }
    ctx.render_rectangle(model.cursorArea, frameColor);
}

bool reig::reference_widget::slider::draw(reig::Context& ctx) const {
    auto model = get_slider_model(ctx, *this);

    draw_slider_model(ctx, model, *this);

    if (mValueRef != model.value) {
        mValueRef = model.value;
        return true;
    } else {
        return false;
    }
}

bool reig::reference_widget::textured_slider::draw(reig::Context& ctx) const {
    auto model = get_slider_model(ctx, *this);

    ctx.render_rectangle(model.outlineArea, mBaseTexture);
    ctx.render_rectangle(model.cursorArea, mCursorTexture);

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

template <typename Scrollbar>
SliderModel get_scrollbar_model(reig::Context& ctx, const Scrollbar& scrollbar) {
    Rectangle baseArea = scrollbar.mBoundingBox;
    ctx.fit_rect_in_window(baseArea);

    Rectangle outlineArea = baseArea;
    baseArea = internal::decrease_rect(baseArea, 4);

    auto step = ctx.get_font_size() / 2.0f;
    auto [min, max, value, offset, valuesNum] = prepare_slider_values(0.0f, scrollbar.mViewSize - baseArea.height, scrollbar.mValueRef, step);

    SliderOrientation orientation = calculate_slider_orientation(baseArea);

    // Render the cursor
    auto cursorArea = internal::decrease_rect(baseArea, 4);
    if(orientation == SliderOrientation::HORIZONTAL) {
        size_scrollbar_cursor(cursorArea.x, cursorArea.width, step, offset, scrollbar.mViewSize);
    } else {
        size_scrollbar_cursor(cursorArea.y, cursorArea.height, step, offset, scrollbar.mViewSize);
    }

    bool hoveringOverCursor = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), cursorArea);
    bool holdingClick = ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), baseArea);
    bool holdingClickOnCursor = ctx.mouse.leftButton.is_pressed() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), cursorArea);

    if (holdingClick) {
        if(orientation == SliderOrientation::HORIZONTAL) {
            progress_slider_value(ctx.mouse.get_cursor_pos().x, cursorArea.width, cursorArea.x, min, max, step, value);
        } else {
            progress_slider_value(ctx.mouse.get_cursor_pos().y, cursorArea.height, cursorArea.y, min, max, step, value);
        }
    } else if (ctx.mouse.get_scrolled() != 0 && internal::is_boxed_in(ctx.mouse.get_cursor_pos(), baseArea)) {
        value += static_cast<int>(ctx.mouse.get_scrolled()) * step;
        value = internal::clamp(value, min, max);
    }

    return {baseArea, outlineArea, cursorArea, hoveringOverCursor, holdingClickOnCursor, value};
}

bool reig::reference_widget::scrollbar::draw(reig::Context& ctx) const {
    auto model = get_scrollbar_model(ctx, *this);

    draw_slider_model(ctx, model, *this);

    if (mValueRef != model.value) {
        mValueRef = model.value;
        return true;
    } else {
        return false;
    }
}
