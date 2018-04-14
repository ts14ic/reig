#ifndef REIG_REFERENCE_WIDGET_ENTRY_H
#define REIG_REFERENCE_WIDGET_ENTRY_H

#include "context.h"
#include <string>

namespace reig::reference_widget {
    namespace detail {
        template <typename Char, typename Action>
        struct ref_entry {
            const char* const mTitle;
            const Rectangle& mBoundingArea;
            const Color& mPrimaryColor;
            std::basic_string<Char>& mValueRef;
            Action& mAction;

            void use(Context& ctx) const;
        };

        struct EntryModel {
            Rectangle outlineArea;
            Rectangle baseArea;
            bool isSelected = false;
            bool isHoveringOverArea = false;
            bool isInputModified = false;
        };

        template <template <class, class> typename Entry, typename String, typename Action>
        EntryModel get_entry_model(Context& ctx, const Entry<String, Action>& entry);
    }

    template <typename Char, typename Action>
    auto entry(const char* title, const Rectangle& boundingArea, const Color& primaryColor,
               std::basic_string<Char>& string, Action&& action) {
        return detail::ref_entry<Char, Action>{title, boundingArea, primaryColor, string, action};
    };
}

#include "reference_widget_entry.tcc"

#endif //REIG_REFERENCE_WIDGET_ENTRY_H
