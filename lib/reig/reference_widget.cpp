#include "reference_widget.h"
#include "context.h"

using namespace reig::primitive;

namespace reig::reference_widget {
    struct ButtonModel {
        const Rectangle bounding_box;
        const bool is_hovering_over_area = false;
        const bool has_just_clicked = false;
        const bool is_holding_click = false;
    };

    ButtonModel get_button_model(Context& ctx, Rectangle outline_area) {
        ctx.fit_rect_in_window(outline_area);

        bool is_hovering_over_area = ctx.mouse.is_hovering_over_rect(outline_area);
        bool has_just_clicked = ctx.mouse.left_button.just_clicked_in_rect(outline_area);
        bool is_holding_click = is_hovering_over_area
                             && ctx.mouse.left_button.clicked_in_rect(outline_area)
                             && ctx.mouse.left_button.is_held();

        return {outline_area, is_hovering_over_area, has_just_clicked, is_holding_click};
    }

    bool button(Context& ctx, const char* title, Rectangle bounding_box, Color base_color) {
        auto model = get_button_model(ctx, bounding_box);

        Color inner_color{base_color};
        if (model.is_hovering_over_area) {
            inner_color = colors::lighten_color_by(inner_color, 30);
        }
        Rectangle base_area;
        if (model.is_holding_click) {
            inner_color = colors::lighten_color_by(inner_color, 30);
            base_area = decrease_rect(model.bounding_box, 6);
        } else {
            base_area = decrease_rect(model.bounding_box, 4);
        }

        ctx.render_rectangle(model.bounding_box, colors::get_yiq_contrast(inner_color));
        ctx.render_rectangle(base_area, inner_color);
        ctx.render_text(title, base_area);

        return model.has_just_clicked;
    }

    bool textured_button(Context& ctx, const char* title, Rectangle bounding_box,
                         int hover_texture, int base_texture) {
        auto model = get_button_model(ctx, bounding_box);

        int texture = base_texture;
        if (model.is_holding_click || model.is_hovering_over_area) {
            texture = hover_texture;
        }
        ctx.render_rectangle(model.bounding_box, texture);
        ctx.render_text(title, model.bounding_box);

        return model.has_just_clicked;
    }

    void label(Context& ctx, char const* title, Rectangle bounding_box, text::Alignment alignment, float font_scale) {
        ctx.fit_rect_in_window(bounding_box);
        ctx.render_text(title, bounding_box, alignment, font_scale);
    }

    struct CheckboxModel {
        Rectangle base_area;
        Rectangle outline_area;
        Rectangle check_area;
        bool is_hovering_over_area = false;
        bool has_value_changed = false;
    };

    CheckboxModel get_checkbox_model(Context& ctx, Rectangle outlineArea, bool& value_ref) {
        ctx.fit_rect_in_window(outlineArea);
        bool is_hovering_over_area = ctx.mouse.is_hovering_over_rect(outlineArea);

        Rectangle base_area = decrease_rect(outlineArea, 4);
        Rectangle check_area = decrease_rect(base_area, 4);

        bool has_just_clicked = ctx.mouse.left_button.just_clicked_in_rect(outlineArea);
        if (has_just_clicked) {
            base_area = decrease_rect(base_area, 4);
            check_area = decrease_rect(check_area, 4);
            value_ref = !value_ref;
        }

        bool holding_click = ctx.mouse.left_button.clicked_in_rect(outlineArea) && ctx.mouse.left_button.is_held();
        if (holding_click) {
            base_area = decrease_rect(base_area, 4);
            check_area = decrease_rect(check_area, 4);
        }

        return {base_area, outlineArea, check_area, is_hovering_over_area, has_just_clicked};
    }

    bool checkbox(Context& ctx, Rectangle bounding_box, Color base_color, bool& value_ref) {
        auto model = get_checkbox_model(ctx, bounding_box, value_ref);

        Color secondary_color = colors::get_yiq_contrast(base_color);
        ctx.render_rectangle(model.outline_area, secondary_color);
        ctx.render_rectangle(model.base_area,
                             model.is_hovering_over_area
                             ? colors::lighten_color_by(base_color, 30)
                             : base_color);

        if (value_ref) {
            ctx.render_rectangle(model.check_area, secondary_color);
        }

        return value_ref;
    }

    bool textured_checkbox(Context& ctx, primitive::Rectangle bounding_box, int base_texture, int check_texture,
                                bool& value_ref) {
        auto model = get_checkbox_model(ctx, bounding_box, value_ref);

        ctx.render_rectangle(model.outline_area, base_texture);

        if (value_ref) {
            ctx.render_rectangle(model.check_area, check_texture);
        }

        return value_ref;
    }
}
