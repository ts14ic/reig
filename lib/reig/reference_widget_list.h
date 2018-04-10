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
    Rectangle boundingBox = {mBoundingBox};
    ctx.fit_rect_in_window(boundingBox);

    float fontHeight = ctx.get_font_size();
    float y = boundingBox.y;
    for (auto it = mBegin; it != mEnd; ++it, y += fontHeight) {
        Rectangle itemBox = {boundingBox.x, y, boundingBox.width, fontHeight};

        Color color = mBaseColor;

        bool hoveringOnItem = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), itemBox);
        bool clickedInBox = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), boundingBox);
        bool isItemClicked = ctx.mouse.leftButton.is_clicked()
                             && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), itemBox);
        if (hoveringOnItem) {
            color = internal::lighten_color_by(color, 30);

            if (isItemClicked && clickedInBox) {
                color = internal::lighten_color_by(color, 30);

                mAction(it - mBegin, *it);
            }
        }

        internal::render_widget_frame(ctx, itemBox, color);
        ctx.render_text(mAdapter(*it), itemBox);
    }
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
