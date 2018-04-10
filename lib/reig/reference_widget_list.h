#ifndef REIG_REFERENCE_WIDGET_LIST_H
#define REIG_REFERENCE_WIDGET_LIST_H

#include "context.h"

namespace reig::reference_widget {
    namespace detail {
        template <typename Iter, typename Adapter, typename Action>
        struct list {
            const char* mTitle = "";
            primitive::Rectangle mBoundingBox;
            primitive::Color mBaseColor;
            Iter mBegin;
            Iter mEnd;
            Adapter& adapter;
            Action& action;

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
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
