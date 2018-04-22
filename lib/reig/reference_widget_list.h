#ifndef REIG_REFERENCE_WIDGET_LIST_H
#define REIG_REFERENCE_WIDGET_LIST_H

#include "context.h"
#include "reference_widget.h"

namespace reig::reference_widget {
    namespace detail {
        // FIXME: This cache can't be dropped
        float& get_scroll_value(const void* scrollbar);
    }

    template <typename Range, typename Adapter, typename Action>
    auto list(Context& ctx, const char* scrollbarId, const primitive::Rectangle& boundingBox, const primitive::Color& baseColor,
              Range&& range, Adapter&& adapter, Action&& action) {
        using namespace primitive;
        using std::begin;
        using std::end;
        auto elemBegin = begin(range);
        auto elemEnd = end(range);
        {
            Rectangle listArea = boundingBox;
            ctx.fit_rect_in_window(listArea);

            using namespace colors::operators;
            using namespace colors::literals;
            ctx.render_rectangle(listArea, colors::get_yiq_contrast(baseColor - 50_a));
            ctx.render_rectangle(decrease_rect(listArea, 2), baseColor - 50_a);
        }

        auto& scrolled = detail::get_scroll_value(scrollbarId);
        float fontHeight = ctx.get_font_size();
        auto skippedItemCount = static_cast<int>(scrolled / fontHeight);

        float y = boundingBox.y;
        float maxY = get_y2(boundingBox);
        float scrollbarWidth = 30.0f;
        for (auto it = elemBegin + skippedItemCount; it != elemEnd && y < maxY; ++it, y += fontHeight) {
            Rectangle itemFrameBox = {boundingBox.x + scrollbarWidth, y, boundingBox.width, fontHeight};
            trim_rect_in_other(itemFrameBox, boundingBox);

            if (button(ctx, adapter(*it), itemFrameBox, baseColor)) {
                action(it - elemBegin, *it);
            }
        }

        auto itemCount = elemEnd - elemBegin;
        Rectangle scrollbarArea {boundingBox.x, boundingBox.y, scrollbarWidth, boundingBox.height};
        scrollbar(ctx, scrollbarArea, baseColor, scrolled, itemCount * fontHeight);
    };
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
