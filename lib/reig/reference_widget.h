#ifndef REIG_REFERENCE_WIDGET_H
#define REIG_REFERENCE_WIDGET_H

#include "context_fwd.h"
#include "text.h"
#include "primitive.h"

namespace reig::reference_widget {
    /**
     * @brief Render a titled button
     *
     * @param title Text to be displayed on button
     * @param bounding_box Button's bounding box
     * @param base_color Button's base color
     *
     * @return True if the button was clicked, false otherwise
     */
    bool button(reig::Context& ctx, const char* title, primitive::Rectangle bounding_box, primitive::Color base_color);

    /**
     * @brief Render a titled textured button
     * @param bounding_box Button's bouding box
     * @param base_texture Button's texture index, when idle
     * @param hover_texture Button's texture index, when button is hovered
     * @return True if the button was clicked, false otherwise
     */
    bool textured_button(Context& ctx, const char* title, primitive::Rectangle bounding_box,
                         int hover_texture, int base_texture);

    /**
     * @brief Render a label, which will be enclosed in the current window, if any
     * @param text Text to be displayed
     * @param bounding_box Text's bounding box
     * @param alignment Text's alignment
     * @param font_scale Text's font scale relative to size set in set_font
     */
    void label(Context& ctx, const char* title, primitive::Rectangle bounding_box,
               text::Alignment alignment = text::Alignment::kCenter, float font_scale = 1.f);

    /**
     * @brief Renders a slider.
     * @param bounding_box Slider's bounding box
     * @param base_color Slider's base color
     * @param value A reference to the value to be represented and changed
     * @param min The lowest represantable value
     * @param max The highest represantable value
     * @param step The discrete portion by which the value can change
     * @return True if value changed
     */
    bool slider(Context& ctx, primitive::Rectangle bounding_box, primitive::Color base_color,
                float& value, float min, float max, float step);

    /**
     * @brief Renders a slider.
     * @param bounding_box Slider's bounding box
     * @param base_texture Slider's base texture index
     * @param cursor_texture Slider's cursor texture index
     * @param value A reference to the value to be represented and changed
     * @param min The lowest represantable value
     * @param max The highest represantable value
     * @param step The discrete portion by which the value can change
     * @return True if value changed
     */
    bool textured_slider(Context& ctx, primitive::Rectangle bounding_box,
                         int base_texture, int cursor_texture,
                         float& value, float min, float max, float step);

    /**
     * @brief Renders a vertical scrollbar
     * @param bounding_box Scrollbar's position and size
     * @param base_color Checkbox's base color
     * @param value A reference to the float to be changed
     * @return True if value changed
     */
    bool scrollbar(Context& ctx, primitive::Rectangle bounding_box, primitive::Color base_color,
                   float& value, float view_size);

    /**
     * @brief Renders a checkbox
     * @param bounding_box Checkbox's position and size
     * @param base_color Checkbox's base color
     * @param value A reference to the bool to be represented
     * @return True if value is true
     */
    bool checkbox(Context& ctx, primitive::Rectangle bounding_box, primitive::Color base_color, bool& value);

    /**
     * @brief Renders a textured checkbox
     * @param bounding_box Checkbox's position and size
     * @param base_texture Checkbox's base texture
     * @param check_texture Checkbox's filling texture
     * @param value A reference to the bool to be represented
     * @return True if value is true
     */
    bool textured_checkbox(Context& ctx, primitive::Rectangle bounding_box, int base_texture, int check_texture,
                           bool& value);
}

#endif //REIG_REFERENCE_WIDGET_H
