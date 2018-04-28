#ifndef REIG_REFERENCE_WIDGET_ENTRY_H
#define REIG_REFERENCE_WIDGET_ENTRY_H

#include "context.h"
#include <string>

namespace reig::reference_widget {
    enum class EntryOuput {
        kUnmodified,
        kModified,
        kSubmitted,
        kCancelled
    };

    template <typename Char>
    EntryOuput entry(Context& ctx, const char* title, primitive::Rectangle bounding_box,
                     const primitive::Color& base_color, std::basic_string<Char>* value);
}

#include "reference_widget_entry.tcc"

#endif //REIG_REFERENCE_WIDGET_ENTRY_H
