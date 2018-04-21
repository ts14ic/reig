#ifndef REIG_REFERENCE_WIDGET_LIST_H
#define REIG_REFERENCE_WIDGET_LIST_H

#include "context.h"
#include "internal.h"
#include "reference_widget.h"

namespace reig::reference_widget {
    namespace detail {
        // FIXME: This cache can't be dropped
        float& get_scroll_value(const void* scrollbar);

        template <typename Iter, typename Adapter, typename Action>
        struct list {
            const char* mTitle = "";
            Rectangle mBoundingBox;
            Color mBaseColor;
            Iter mBegin;
            Iter mEnd;
            Adapter& mAdapter;
            Action& mAction;

            void use(Context& ctx) const;
        };
    }

    template <typename Range, typename Adapter, typename Action>
    auto list(const char* title, const Rectangle& rectangle, const Color& baseColor,
              Range&& range, Adapter&& adapter, Action&& action) {
        using std::begin;
        using std::end;
        return detail::list<decltype(begin(range)), Adapter, Action>
                {title, rectangle, baseColor, begin(range), end(range), adapter, action};
    };
}

template <typename Iter, typename Adapter, typename Action>
void reig::reference_widget::detail::list<Iter, Adapter, Action>::use(reig::Context& ctx) const {
    {
        Rectangle listArea = mBoundingBox;
        ctx.fit_rect_in_window(listArea);

        using namespace colors::operators;
        using namespace colors::literals;
        ctx.render_rectangle(listArea, internal::get_yiq_contrast(mBaseColor - 50_a));
        ctx.render_rectangle(internal::decrease_rect(listArea, 2), mBaseColor - 50_a);
    }

    auto& scrolled = detail::get_scroll_value(this);
    float fontHeight = ctx.get_font_size();
    auto skippedItemCount = static_cast<int>(scrolled / fontHeight);

    float y = mBoundingBox.y;
    float maxY = get_y2(mBoundingBox);
    float scrollbarWidth = 30.0f;
    for (auto it = mBegin + skippedItemCount; it != mEnd && y < maxY; ++it, y += fontHeight) {
        Rectangle itemFrameBox = {mBoundingBox.x + scrollbarWidth, y, mBoundingBox.width, fontHeight};
        internal::trim_rect_in_other(itemFrameBox, mBoundingBox);

        if (button(ctx, mAdapter(*it), itemFrameBox, mBaseColor)) {
            mAction(it - mBegin, *it);
        }
    }

    auto itemCount = mEnd - mBegin;
    Rectangle scrollbarArea {mBoundingBox.x, mBoundingBox.y, scrollbarWidth, mBoundingBox.height};
    scrollbar(ctx, scrollbarArea, mBaseColor, scrolled, itemCount * fontHeight);
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
