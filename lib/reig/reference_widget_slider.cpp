#include "reference_widget.h"
#include "internal.h"
#include "context.h"


namespace reig::reference_widget {
    using namespace primitive;

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
        if (size < 1) size = 1;
        coord += offset * size;
    }

    float get_distance_to_slider(float mouseCursorCoord, float cursorSize, float sliderCursorCoord) {
        auto halfCursorSize = cursorSize / 2;
        sliderCursorCoord += halfCursorSize;
        return mouseCursorCoord - sliderCursorCoord;
    }

    void progress_slider_value(float mouseCursorCoord, float cursorSize,
                               float sliderCursorCoord, float step, float& value) {
        float distance = get_distance_to_slider(mouseCursorCoord, cursorSize, sliderCursorCoord);
        if (internal::abs(distance) > cursorSize / 2) {
            value += static_cast<int>(distance / cursorSize) * step;
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
        bool holdingClickOnSlider = false;
        bool valueChanged = false;
    };

    SliderModel tweak_slider_model(Context& ctx, float& aValueRef, const SliderValues& values,
                                   Rectangle outlineArea, Rectangle baseArea, Rectangle cursorArea) {
        bool hoveringOverCursor = ctx.mouse.is_hovering_over_rect(outlineArea);
        bool holdingClickOnSlider = ctx.mouse.leftButton.clicked_in_rect(outlineArea)
                                    && ctx.mouse.leftButton.is_held();

        if (holdingClickOnSlider) {
            baseArea = internal::decrease_rect(baseArea, 2);
            cursorArea = internal::decrease_rect(cursorArea, 4);
        }

        bool valueChanged = false;
        if (aValueRef != values.value) {
            aValueRef = internal::clamp(values.value, values.min, values.max);
            valueChanged = true;
        }

        return {baseArea, outlineArea, cursorArea, hoveringOverCursor, holdingClickOnSlider, valueChanged};
    }

    SliderModel get_slider_model(Context& ctx, Rectangle outlineArea, float& aValueRef, float aMin, float aMax, float aStep) {
        ctx.fit_rect_in_window(outlineArea);

        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);

        auto values = prepare_slider_values(aMin, aMax, aValueRef, aStep);

        SliderOrientation orientation = calculate_slider_orientation(baseArea);

        auto cursorArea = internal::decrease_rect(baseArea, 4);
        if (orientation == SliderOrientation::HORIZONTAL) {
            size_slider_cursor(cursorArea.x, cursorArea.width, values.valuesNum, values.offset);
        } else {
            size_slider_cursor(cursorArea.y, cursorArea.height, values.valuesNum, values.offset);
        }

        bool hoveringOverCursor = ctx.mouse.is_hovering_over_rect(outlineArea);
        bool holdingClickOnSlider = ctx.mouse.leftButton.clicked_in_rect(outlineArea)
                                    && ctx.mouse.leftButton.is_held();

        if (holdingClickOnSlider) {
            if (orientation == SliderOrientation::HORIZONTAL) {
                progress_slider_value(ctx.mouse.get_cursor_pos().x, cursorArea.width, cursorArea.x,
                                      aStep, values.value);
            } else {
                progress_slider_value(ctx.mouse.get_cursor_pos().y, cursorArea.height, cursorArea.y,
                                      aStep, values.value);
            }
        } else if (ctx.mouse.get_scrolled() != 0 && hoveringOverCursor) {
            values.value += static_cast<int>(ctx.mouse.get_scrolled()) * aStep;
        }
        return tweak_slider_model(ctx, aValueRef, values, outlineArea, baseArea, cursorArea);
    }

    void size_scrollbar_cursor(float& coord, float& size, float step, int offset, float viewSize) {
        float scale = size / viewSize;
        if (scale <= 1.0f) {
            coord += offset * step * scale;
            size = internal::max(1.0f, scale * size);
            size = internal::min(size, viewSize);
        }
    }

    SliderValues prepare_scrollbar_values(float maxScroll, float value, float step) {
        float min = 0.0f;
        float max = internal::max(0.f, maxScroll);
        float clampedValue = internal::clamp(value, min, max);
        return SliderValues{
                min, max, clampedValue,
                static_cast<int>((value - min) / step),
                static_cast<int>((max - min) / step + 1)
        };
    }

    void progress_scrollbar_value(float mouseCursorCoord, float cursorSize,
                                  float sliderCursorCoord, float step, float& value) {
        float distance = get_distance_to_slider(mouseCursorCoord, cursorSize, sliderCursorCoord);
        if (internal::abs(distance) > cursorSize / 2) {
            value += static_cast<int>(distance * step) / cursorSize;
        }
    }

    SliderModel get_scrollbar_model(Context& ctx, Rectangle outlineArea, float viewSize, float& aValueRef) {
        ctx.fit_rect_in_window(outlineArea);
        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);

        auto step = ctx.get_font_size();

        SliderOrientation orientation = calculate_slider_orientation(baseArea);
        SliderValues values;
        if (orientation == SliderOrientation::VERTICAL) {
            values = prepare_scrollbar_values(viewSize - baseArea.height, aValueRef, step);
        } else {
            values = prepare_scrollbar_values(viewSize - baseArea.width, aValueRef, step);
        }

        auto cursorArea = internal::decrease_rect(baseArea, 4);
        if (orientation == SliderOrientation::HORIZONTAL) {
            size_scrollbar_cursor(cursorArea.x, cursorArea.width, step, values.offset, viewSize);
        } else {
            size_scrollbar_cursor(cursorArea.y, cursorArea.height, step, values.offset, viewSize);
        }

        bool hoveringOverCursor = ctx.mouse.is_hovering_over_rect(outlineArea);
        bool holdingClickOnSlider = ctx.mouse.leftButton.clicked_in_rect(outlineArea)
                                    && ctx.mouse.leftButton.is_held();

        if (holdingClickOnSlider) {
            if (orientation == SliderOrientation::HORIZONTAL) {
                progress_scrollbar_value(ctx.mouse.get_cursor_pos().x, cursorArea.width, cursorArea.x,
                                         step, values.value);
            } else {
                progress_scrollbar_value(ctx.mouse.get_cursor_pos().y, cursorArea.height, cursorArea.y,
                                         step, values.value);
            }
        } else if (ctx.mouse.get_scrolled() != 0 && hoveringOverCursor) {
            values.value += static_cast<int>(ctx.mouse.get_scrolled()) * step;
        }
        return tweak_slider_model(ctx, aValueRef, values, outlineArea, baseArea, cursorArea);
    }

    void draw_slider_model(Context& ctx, const SliderModel& model, const Color& baseColor) {
        Color frameColor = internal::get_yiq_contrast(baseColor);
        ctx.render_rectangle(model.outlineArea, frameColor);
        ctx.render_rectangle(model.baseArea, baseColor);

        if (model.hoveringOverCursor) {
            frameColor = internal::lighten_color_by(frameColor, 30);
        }
        if (model.holdingClickOnSlider) {
            frameColor = internal::lighten_color_by(frameColor, 30);
        }
        ctx.render_rectangle(model.cursorArea, frameColor);
    }

    bool slider(Context& ctx, Rectangle boundingBox, Color baseColor,
                float& valueRef, float min, float max, float step) {
        auto model = get_slider_model(ctx, boundingBox, valueRef, min, max, step);

        draw_slider_model(ctx, model, baseColor);

        return model.valueChanged;
    }

    bool scrollbar(Context& ctx, Rectangle boundingBox, Color baseColor,
                   float& valueRef, float viewSize) {
        auto model = get_scrollbar_model(ctx, boundingBox, viewSize, valueRef);

        draw_slider_model(ctx, model, baseColor);

        return model.valueChanged;
    }

    bool textured_slider(Context& ctx, Rectangle boundingBox, int baseTexture, int cursorTexture,
                              float& valueRef, float min, float max, float step) {
        auto model = get_slider_model(ctx, boundingBox, valueRef, min, max, step);

        ctx.render_rectangle(model.outlineArea, baseTexture);
        ctx.render_rectangle(model.cursorArea, cursorTexture);

        return model.valueChanged;
    }
}
