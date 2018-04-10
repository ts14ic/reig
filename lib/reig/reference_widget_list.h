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

    template <typename Iter, typename Adapter, typename Action>
    detail::list<Iter, Adapter, Action>
    list(const char* title,
         const primitive::Rectangle& rectangle,
         const primitive::Color& baseColor,
         Iter&& begin,
         Iter&& end,
         Adapter&& adapter,
         Action&& action
    ) {
        return {title, rectangle, baseColor, begin, end, adapter, action};
    };
}

template <typename Iter, typename Adapter, typename Action>
void reig::reference_widget::detail::list<Iter, Adapter, Action>::draw(reig::Context& ctx) const {
    using namespace reig::primitive;
    Rectangle boundingBox = {mBoundingBox};
    ctx.fit_rect_in_window(boundingBox);

    ctx.render_rectangle(boundingBox, mBaseColor);

    float fontHeight = ctx.get_font_size();
    float y = boundingBox.y;
    for(auto it = mBegin; it != mEnd; ++it) {
        Rectangle itemBox = {boundingBox.x, y, boundingBox.width, fontHeight};
        ctx.render_text(mAdapter(*it), itemBox);
        y += fontHeight;

        if(ctx.mouse.leftButton.is_clicked() && internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), itemBox)) {
            mAction(it - mBegin, *it);
        }
    }
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
