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
            Rectangle mBoundingBox;
            Color mBaseColor;
            Iter mBegin;
            Iter mEnd;
            Adapter& mAdapter;
            Action& mAction;

            void use(Context& ctx) const;
        };

        struct ItemModel {
            Rectangle itemFrameBox;
            Rectangle itemBox;
            bool hoveringOnItem = false;
            bool holdingClickOnItem = false;
        };

        template <typename List, typename Iter>
        ItemModel get_item_model(Context& ctx, const List& list, const Rectangle& listArea,
                                 float itemY, float fontHeight, const Iter& it, const Iter& begin) {
            Rectangle itemFrameBox = {listArea.x, itemY, listArea.width, fontHeight};
            internal::trim_rect_in_other(itemFrameBox, listArea);
            Rectangle itemBox = internal::decrease_rect(itemFrameBox, 4);

            bool hoveringOnItem = internal::is_boxed_in(ctx.mouse.get_cursor_pos(), itemBox);
            bool clickedOnItem = internal::is_boxed_in(ctx.mouse.leftButton.get_clicked_pos(), itemBox);
            bool holdingClickOnItem = ctx.mouse.leftButton.is_pressed() && clickedOnItem;
            if (clickedOnItem) {
                if (ctx.mouse.leftButton.is_clicked()) {
                    list.mAction(it - begin, *it);
                }
            }
            if(holdingClickOnItem) {
                itemBox = internal::decrease_rect(itemBox, 4);
            }

            return {itemFrameBox, itemBox, hoveringOnItem, holdingClickOnItem};
        }

        template <typename Adapter, typename Iter>
        void draw_item_model(Context& ctx, const ItemModel& model, const Color& baseColor, Adapter&& adapter, Iter&& it) {
            Color primaryColor = baseColor;
            if (model.hoveringOnItem) {
                primaryColor = internal::lighten_color_by(primaryColor, 30);
            }
            if (model.holdingClickOnItem) {
                primaryColor = internal::lighten_color_by(primaryColor, 30);
            }

            Color secondaryColor = internal::get_yiq_contrast(primaryColor);
            ctx.render_rectangle(model.itemFrameBox, secondaryColor);
            ctx.render_rectangle(model.itemBox, primaryColor);

            ctx.render_text(adapter(*it), model.itemBox);
        }
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
    int scrollbarWidth = 30;
    Rectangle listArea = mBoundingBox;
    listArea.x += scrollbarWidth;
    ctx.fit_rect_in_window(listArea);

    auto& scrolled = detail::get_scroll_value(mTitle);
    float fontHeight = ctx.get_font_size();
    auto itemCount = mEnd - mBegin;
    auto skippedItemCount = static_cast<int>(scrolled / fontHeight);

    float y = listArea.y;
    float maxY = listArea.y + listArea.height;
    for (auto it = mBegin + skippedItemCount; it != mEnd && y < maxY; ++it, y += fontHeight) {
        auto model = get_item_model(ctx, *this, listArea, y, fontHeight, it, mBegin);
        draw_item_model(ctx, model, mBaseColor, mAdapter, it);
    }

    auto scrollbarArea = mBoundingBox;
    scrollbarArea.width = scrollbarWidth;
    scrollbar{scrollbarArea, mBaseColor, scrolled, itemCount * fontHeight}.use(ctx);
}

#endif //REIG_REFERENCE_WIDGET_LIST_H
