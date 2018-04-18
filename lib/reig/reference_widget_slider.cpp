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

    template <typename S>
    SliderModel get_slider_base_model(Context& ctx, const S& slider, SliderValues& values,
                                      const Rectangle& outlineArea, const Rectangle& baseArea,
                                      const Rectangle& cursorArea, const Focus& focus) {
        SliderModel model;
        model.outlineArea = outlineArea;
        model.baseArea = baseArea;
        model.cursorArea = cursorArea;

        switch (focus) {
            case Focus::HOVER: {
                model.hoveringOverCursor = true;
                break;
            }

            case Focus::HOLD: {
                model.holdingClickOnSlider = true;
                break;
            }

            default: {
                break;
            }
        }

        if (model.holdingClickOnSlider) {
            model.baseArea = internal::decrease_rect(model.baseArea, 2);
            model.cursorArea = internal::decrease_rect(model.cursorArea, 4);
        }

        if (slider.mValueRef != values.value) {
            slider.mValueRef = internal::clamp(values.value, values.min, values.max);
            model.valueChanged = true;
        }

        return model;
    }

    template <typename Slider>
    SliderModel get_slider_model(Context& ctx, const Slider& slider, const Rectangle& outlineArea, const Focus& focus) {
        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);

        auto values = prepare_slider_values(slider.mMin, slider.mMax, slider.mValueRef, slider.mStep);

        SliderOrientation orientation = calculate_slider_orientation(baseArea);

        auto cursorArea = internal::decrease_rect(baseArea, 4);
        if (orientation == SliderOrientation::HORIZONTAL) {
            size_slider_cursor(cursorArea.x, cursorArea.width, values.valuesNum, values.offset);
        } else {
            size_slider_cursor(cursorArea.y, cursorArea.height, values.valuesNum, values.offset);
        }

        if (focus == Focus::HOLD) {
            if (orientation == SliderOrientation::HORIZONTAL) {
                progress_slider_value(ctx.mouse.get_cursor_pos().x, cursorArea.width, cursorArea.x,
                                      slider.mStep, values.value);
            } else {
                progress_slider_value(ctx.mouse.get_cursor_pos().y, cursorArea.height, cursorArea.y,
                                      slider.mStep, values.value);
            }
        } else if (ctx.mouse.get_scrolled() != 0 && focus == Focus::HOVER) {
            values.value += static_cast<int>(ctx.mouse.get_scrolled()) * slider.mStep;
        }
        return get_slider_base_model(ctx, slider, values, outlineArea, baseArea,
                                     cursorArea, focus);
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

    template <typename Scrollbar>
    SliderModel get_scrollbar_model(Context& ctx, const Scrollbar& scrollbar, const Rectangle& outlineArea, const Focus& focus) {
        Rectangle baseArea = internal::decrease_rect(outlineArea, 4);

        auto step = ctx.get_font_size();
        auto values = prepare_scrollbar_values(scrollbar.mViewSize - baseArea.height, scrollbar.mValueRef, step);

        SliderOrientation orientation = calculate_slider_orientation(baseArea);

        auto cursorArea = internal::decrease_rect(baseArea, 4);
        if (orientation == SliderOrientation::HORIZONTAL) {
            size_scrollbar_cursor(cursorArea.x, cursorArea.width, step, values.offset, scrollbar.mViewSize);
        } else {
            size_scrollbar_cursor(cursorArea.y, cursorArea.height, step, values.offset, scrollbar.mViewSize);
        }

        if (focus == Focus::HOLD) {
            if (orientation == SliderOrientation::HORIZONTAL) {
                progress_scrollbar_value(ctx.mouse.get_cursor_pos().x, cursorArea.width, cursorArea.x,
                                         step, values.value);
            } else {
                progress_scrollbar_value(ctx.mouse.get_cursor_pos().y, cursorArea.height, cursorArea.y,
                                         step, values.value);
            }
        } else if (ctx.mouse.get_scrolled() != 0 && focus == Focus::HOVER) {
            values.value += static_cast<int>(ctx.mouse.get_scrolled()) * step;
        }
        return get_slider_base_model(ctx, scrollbar, values, outlineArea, baseArea,
                                     cursorArea, focus);
    }

    template <typename Slider>
    void draw_slider_model(Context& ctx, const SliderModel& model, const Slider& slider) {
        Color frameColor = internal::get_yiq_contrast(slider.mBaseColor);
        ctx.render_rectangle(model.outlineArea, frameColor);
        ctx.render_rectangle(model.baseArea, slider.mBaseColor);

        if (model.hoveringOverCursor) {
            frameColor = internal::lighten_color_by(frameColor, 30);
        }
        if (model.holdingClickOnSlider) {
            frameColor = internal::lighten_color_by(frameColor, 30);
        }
        ctx.render_rectangle(model.cursorArea, frameColor);
    }

    void slider::use(Context& ctx, std::function<void()> callback) const {
        Rectangle outlineArea = mBoundingBox;
        ctx.fit_rect_in_window(outlineArea);

        ctx.with_focus(outlineArea, [=, *this, &ctx](const Focus& focus) {
            auto model = get_slider_model(ctx, *this, outlineArea, focus);

            if (model.valueChanged) {
                callback();
            }

            draw_slider_model(ctx, model, *this);
        });
    }

    void scrollbar::use(Context& ctx, std::function<void()> callback) const {
        Rectangle outlineArea = mBoundingBox;
        ctx.fit_rect_in_window(outlineArea);

        ctx.with_focus(outlineArea, [=, *this, &ctx](const Focus& focus) {
            auto model = get_scrollbar_model(ctx, *this, outlineArea, focus);

            if (model.valueChanged) {
                callback();
            }

            draw_slider_model(ctx, model, *this);
        });
    }

    void textured_slider::use(Context& ctx, std::function<void()> callback) const {
        Rectangle outlineArea = mBoundingBox;
        ctx.fit_rect_in_window(outlineArea);

        ctx.with_focus(outlineArea, [=, *this, &ctx](const Focus& focus) {
            auto model = get_slider_model(ctx, *this, mBoundingBox, Focus::NONE);

            if (model.valueChanged) {
                callback();
            }

            ctx.render_rectangle(model.outlineArea, mBaseTexture);
            ctx.render_rectangle(model.cursorArea, mCursorTexture);
        });
    }
}
