#include "reference_widget.h"
#include "context.h"
#include <cassert>

using namespace reig::primitive;

namespace reig::reference_widget {
    struct ButtonModel {
        const bool is_hovering_over_area = false;
        const bool has_just_clicked = false;
        const bool is_holding_click = false;
    };

    ButtonModel get_button_model(Context& ctx, Rectangle* bounding_box) {
        ctx.fit_rect_in_window(*bounding_box);

        bool is_hovering_over_area = ctx.mouse.is_hovering_over_rect(*bounding_box);
        bool has_just_clicked = ctx.mouse.left_button.just_clicked_in_rect(*bounding_box);
        bool is_holding_click = is_hovering_over_area
                             && ctx.mouse.left_button.clicked_in_rect(*bounding_box)
                             && ctx.mouse.left_button.is_held();

        return {is_hovering_over_area, has_just_clicked, is_holding_click};
    }

    bool button(Context& ctx, const char* title, Rectangle bounding_box, Color base_color) {
        auto model = get_button_model(ctx, &bounding_box);

        Color inner_color{base_color};
        if (model.is_hovering_over_area) {
            inner_color = colors::lighten_color_by(inner_color, 30);
        }
        Rectangle base_area;
        if (model.is_holding_click) {
            inner_color = colors::lighten_color_by(inner_color, 30);
            base_area = decrease_rect(bounding_box, 6);
        } else {
            base_area = decrease_rect(bounding_box, 4);
        }

        ctx.render_rectangle(bounding_box, colors::get_yiq_contrast(inner_color));
        ctx.render_rectangle(base_area, inner_color);
        ctx.render_text(title, base_area);

        return model.has_just_clicked;
    }

    bool textured_button(Context& ctx, const char* title, Rectangle bounding_box,
                         int hover_texture, int base_texture) {
        auto model = get_button_model(ctx, &bounding_box);

        int texture = base_texture;
        if (model.is_holding_click || model.is_hovering_over_area) {
            texture = hover_texture;
        }
        ctx.render_rectangle(bounding_box, texture);
        ctx.render_text(title, bounding_box);

        return model.has_just_clicked;
    }

    void label(Context& ctx, char const* title, Rectangle bounding_box, text::Alignment alignment, float font_scale) {
        ctx.fit_rect_in_window(bounding_box);
        ctx.render_text(title, bounding_box, alignment, font_scale);
    }

    struct CheckboxModel {
        const bool is_hovering_over_area = false;
        const bool has_just_clicked = false;
        const bool is_holding_click = false;
    };

    CheckboxModel get_checkbox_model(Context& ctx, Rectangle* bounding_box, bool* value) {
        ctx.fit_rect_in_window(*bounding_box);
        bool is_hovering_over_area = ctx.mouse.is_hovering_over_rect(*bounding_box);
        bool has_just_clicked = ctx.mouse.left_button.just_clicked_in_rect(*bounding_box);
        bool is_holding_click = ctx.mouse.left_button.clicked_in_rect(*bounding_box)
                                && ctx.mouse.left_button.is_held();
        if (has_just_clicked) {
            *value = !*value;
        }

        return {is_hovering_over_area, has_just_clicked, is_holding_click};
    }

    bool checkbox(Context& ctx, Rectangle bounding_box, Color base_color, bool* value) {
        assert(value != nullptr && "Can't represent a null bool");
        auto model = get_checkbox_model(ctx, &bounding_box, value);

        Rectangle base_area = decrease_rect(bounding_box, 4);
        Rectangle check_area = decrease_rect(base_area, 4);
        if (model.has_just_clicked) {
            base_area = decrease_rect(base_area, 4);
            check_area = decrease_rect(check_area, 4);
        }
        if (model.is_holding_click) {
            base_area = decrease_rect(base_area, 4);
            check_area = decrease_rect(check_area, 4);
        }

        Color secondary_color = colors::get_yiq_contrast(base_color);
        ctx.render_rectangle(bounding_box, secondary_color);
        ctx.render_rectangle(base_area,
                             model.is_hovering_over_area
                             ? colors::lighten_color_by(base_color, 30)
                             : base_color);
        if (*value) {
            ctx.render_rectangle(check_area, secondary_color);
        }

        return *value;
    }

    bool textured_checkbox(Context& ctx, primitive::Rectangle bounding_box,
                           int base_texture, int check_texture, bool* value) {
        auto model = get_checkbox_model(ctx, &bounding_box, value);

        Rectangle check_area = decrease_rect(bounding_box, 8);
        if (model.has_just_clicked) {
            check_area = decrease_rect(check_area, 4);
        }
        if (model.is_holding_click) {
            check_area = decrease_rect(check_area, 4);
        }

        ctx.render_rectangle(bounding_box, base_texture);
        if (*value) {
            ctx.render_rectangle(check_area, check_texture);
        }

        return *value;
    }
}
