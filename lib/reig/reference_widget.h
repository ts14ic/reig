#ifndef REIG_REFERENCE_WIDGET_H
#define REIG_REFERENCE_WIDGET_H

#include "fwd.h"
#include "text.h"
#include "primitive.h"

namespace reig::reference_widget {
    /**
     * @brief Render a titled button
     *
     * @param title Text to be displayed on button
     * @param box Button's bounding box
     * @param color Button's base color
     *
     * @return True if the button was clicked, false otherwise
     */
    bool button(reig::Context& ctx, const char* title, primitive::Rectangle boundingBox, primitive::Color baseColor);

    /**
     * @brief Render a titled textured button
     * @param box Button's bouding box
     * @param baseTexture Button's texture index, when idle
     * @param hoverTexture Button's texture index, when button is hoverred
     * @return True if the button was clicked, false otherwise
     */
    bool textured_button(Context& ctx, const char* title, primitive::Rectangle boundingBox,
                         int mHoverTexture, int mBaseTexture);

    /**
     * @brief Render a label, which will be enclosed in the current window, if any
     * @param text Text to be displayed
     * @param boundingBox Text's bounding box
     */
    void label(Context& ctx, char const* title, primitive::Rectangle boundingBox,
               text::Alignment alignment = text::Alignment::CENTER, float fontScale = 1.f);

    /**
     * @brief Renders a slider.
     * @param box Slider's bounding box
     * @param color Slider's base color
     * @param value A reference to the value to be represented and changed
     * @param min The lowest represantable value
     * @param max The highest represantable value
     * @param step The discrete portion by which the value can change
     * @return True if value changed
     */
    bool slider(Context& ctx, primitive::Rectangle boundingBox, primitive::Color baseColor,
                float& valueRef, float min, float max, float step);

    /**
     * @brief
     * @brief Renders a slider.
     * @param box Slider's bounding box
     * @param baseTexture Slider's base texture index
     * @param cursorTexture Slider's cursor texture index
     * @param value A reference to the value to be represented and changed
     * @param min The lowest represantable value
     * @param max The highest represantable value
     * @param step The discrete portion by which the value can change
     * @return True if value changed
     */
    bool textured_slider(Context& ctx, primitive::Rectangle boundingBox, int baseTexture, int cursorTexture,
                         float& valueRef, float min, float max, float step);

    /**
     * @brief Renders a vertical scrollbar
     * @param box Scrollbar's position and size
     * @param color Checkbox's base color
     * @param value A reference to the float to be changed
     * @return True if value changed
     */
    bool scrollbar(Context& ctx, primitive::Rectangle boundingBox, primitive::Color baseColor,
                   float& valueRef, float viewSize);

    /**
     * @brief Renders a checkbox
     * @param box Checkbox's position and size
     * @param color Checkbox's base color
     * @param value A reference to the bool to be changed
     * @return True if value changed
     */
    bool checkbox(Context& ctx, primitive::Rectangle boundingBox, primitive::Color baseColor, bool& valueRef);

    /**
     * @brief Renders a textured checkbox
     * @param box Checkbox's position and size
     * @param baseTexture Checkbox's base texture
     * @param tickTexture Checkbox's tick texture
     * @param value A reference to the bool to be changed
     * @return True if value changed
     */
    bool textured_checkbox(Context& ctx, primitive::Rectangle boundingBox, int baseTexture, int checkTexture,
                           bool& valueRef);
}

#endif //REIG_REFERENCE_WIDGET_H
