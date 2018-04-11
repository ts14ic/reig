#ifndef REIG_REFERENCE_WIDGET_LIST_H
#define REIG_REFERENCE_WIDGET_LIST_H

#include "context.h"
#include "internal.h"

namespace reig::reference_widget {
    namespace detail {
        template <typename Iter, typename Adapter, typename Action>
        struct list {
            const char* mTitle = "";
            primitive::Rectangle mBoundingBox;
            primitive::Color mBaseColor;
            Iter mBegin;
            Iter mEnd;
            Adapter& mAdapter;
            Action& mAction;

            void draw(Context& ctx) const;
        };
    }

    template <typename Range, typename Adapter, typename Action>
    auto list(const char* title,
              const primitive::Rectangle& rectangle,
              const primitive::Color& baseColor,
              Range&& range,
              Adapter&& adapter,
              Action&& action) -> detail::list<decltype(std::begin(range)), Adapter, Action> {
        return {title, rectangle, baseColor, std::begin(range), std::end(range), adapter, action};
    };
}

template <typename Iter, typename Adapter, typename Action>
void reig::reference_widget::detail::list<Iter, Adapter, Action>::draw(reig::Context& ctx) const {
    using namespace reig::primitive;
    Rectangle listBox = {mBoundingBox};
    ctx.fit_rect_in_window(listBox);

    float fontHeight = ctx.get_font_size();
    float y = listBox.y;
    for (auto it = mBegin; it != mEnd; ++it, y += fontHeight) {
        Rectangle itemBox = {listBox.x, y, listBox.width, fontHeight};
        auto is_in_bounds = [&](const Point& pt) {
            return internal::is_boxed_in(pt, itemBox)
                   && internal::is_boxed_in(pt, listBox);
        };

        Color color = mBaseColor;
        bool isHoveringOnItem = is_in_bounds(ctx.mouse.get_cursor_pos());
        if (isHoveringOnItem) {
            color = internal::lighten_color_by(color, 30);
        }

        bool clickedOnItem = is_in_bounds(ctx.mouse.leftButton.get_clicked_pos());
        if (clickedOnItem) {
            if (ctx.mouse.leftButton.is_clicked()) {
                mAction(it - mBegin, *it);
            }

            if (ctx.mouse.leftButton.is_pressed()) {
                color = internal::lighten_color_by(color, 30);
            }
        }

        internal::render_widget_frame(ctx, itemBox, color);
        ctx.render_text(mAdapter(*it), itemBox);
    }
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
