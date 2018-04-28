#ifndef REIG_REFERENCE_WIDGET_LIST_TCC
#define REIG_REFERENCE_WIDGET_LIST_TCC

#include "reference_widget_list.h"

namespace reig::reference_widget {
    template <typename Range, typename Adapter, typename Action>
    void list(Context& ctx, const char* scrollbar_id, const primitive::Rectangle& bounding_box,
              const primitive::Color& base_color, Range&& range, Adapter&& adapter, Action&& action) {
        using namespace primitive;
        using std::begin;
        using std::end;
        auto elem_begin = begin(range);
        auto elem_end = end(range);
        {
            Rectangle list_area = bounding_box;
            ctx.fit_rect_in_window(list_area);

            using namespace colors::operators;
            using namespace colors::literals;
            ctx.render_rectangle(list_area, colors::get_yiq_contrast(base_color - 50_a));
            ctx.render_rectangle(decrease_rect(list_area, 2), base_color - 50_a);
        }

        auto& scrolled = detail::get_scroll_value(scrollbar_id);
        float font_height = ctx.get_font_size();
        auto skipped_item_count = static_cast<int>(scrolled / font_height);

        float y = bounding_box.y;
        float max_y = get_y2(bounding_box);
        float scrollbar_width = 30.0f;
        for (auto it = elem_begin + skipped_item_count; it != elem_end && y < max_y; ++it, y += font_height) {
            Rectangle item_frame_box = {bounding_box.x + scrollbar_width, y, bounding_box.width, font_height};
            trim_rect_in_other(item_frame_box, bounding_box);

            if (button(ctx, adapter(*it), item_frame_box, base_color)) {
                action(it - elem_begin, *it);
            }
        }

        auto itemCount = elem_end - elem_begin;
        Rectangle scrollbar_area{bounding_box.x, bounding_box.y, scrollbar_width, bounding_box.height};
        scrollbar(ctx, scrollbar_area, base_color, scrolled, itemCount * font_height);
    };
}

#endif //REIG_REFERENCE_WIDGET_LIST_TCC
