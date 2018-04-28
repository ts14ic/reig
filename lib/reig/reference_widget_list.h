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
    void list(Context& ctx, const char* scrollbar_id, const primitive::Rectangle& bounding_box,
              const primitive::Color& base_color, Range&& range, Adapter&& adapter, Action&& action);
}

#include "reference_widget_list.tcc"

#endif //REIG_REFERENCE_WIDGET_LIST_H
