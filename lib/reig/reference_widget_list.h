#ifndef REIG_REFERENCE_WIDGET_LIST_H
#define REIG_REFERENCE_WIDGET_LIST_H

#include "context.h"
#include "internal.h"
#include "reference_widget.h"

namespace reig::reference_widget {
    namespace detail {
        // FIXME: This cache can't be dropped
        float& get_scroll_value(const char* title);

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

        struct ItemModel {
            Rectangle itemFrameBox;
            Rectangle itemBox;
            bool hoveringOnItem = false;
            bool holdingClickOnItem = false;
        };

        template <typename List, typename Iter>
        ItemModel get_item_model(Context& ctx, const List& list, const Rectangle& listArea, float itemY, float fontHeight, const Iter& it, const Iter& begin) {
            Rectangle itemFrameBox = {listArea.x, itemY, listArea.width, fontHeight};
            internal::fit_rect_in_other(itemFrameBox, listArea);
            Rectangle itemBox = internal::decrease_rect(itemFrameBox, 4);

            auto is_in_bounds = [&](const Point& pt) {
                return internal::is_boxed_in(pt, itemBox)
                       && internal::is_boxed_in(pt, listArea);
            };

            bool hoveringOnItem = is_in_bounds(ctx.mouse.get_cursor_pos());
            bool clickedOnItem = is_in_bounds(ctx.mouse.leftButton.get_clicked_pos());
            bool holdingClickOnItem = ctx.mouse.leftButton.is_pressed() && clickedOnItem;
            if (clickedOnItem) {
                if (ctx.mouse.leftButton.is_clicked()) {
                    list.mAction(it - begin, *it);
                }
            }

            return {itemFrameBox, itemBox, hoveringOnItem, holdingClickOnItem};
        }
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
    int scrollbarWidth = 30;
    Rectangle listArea = mBoundingBox;
    listArea.x += scrollbarWidth;
    ctx.fit_rect_in_window(listArea);

    auto& scrolled = detail::get_scroll_value(mTitle);
    float fontHeight = ctx.get_font_size();
    auto itemCount = mEnd - mBegin;
    auto skippedItemCount = static_cast<int>(scrolled / fontHeight);

    float y = listArea.y;
    for (auto it = mBegin + skippedItemCount; it != mEnd && y < listArea.y + listArea.height; ++it, y += fontHeight) {
        auto model = get_item_model(ctx, *this, listArea, y, fontHeight, it, mBegin);

        Color primaryColor = mBaseColor;
        if (model.hoveringOnItem) {
            primaryColor = internal::lighten_color_by(primaryColor, 30);
        }
        if (model.holdingClickOnItem) {
            primaryColor = internal::lighten_color_by(primaryColor, 30);
        }

        Color secondaryColor = internal::get_yiq_contrast(primaryColor);
        ctx.render_rectangle(model.itemFrameBox, secondaryColor);
        ctx.render_rectangle(model.itemBox, primaryColor);

        ctx.render_text(mAdapter(*it), model.itemBox);
    }

    auto scrollbarArea = mBoundingBox;
    scrollbarArea.width = scrollbarWidth;
    ctx.enqueue(scrollbar{scrollbarArea, mBaseColor, scrolled, itemCount * fontHeight});
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
