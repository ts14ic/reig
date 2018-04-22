#ifndef REIG_REFERENCE_WIDGET_ENTRY_H
#define REIG_REFERENCE_WIDGET_ENTRY_H

#include "context.h"
#include <string>

namespace reig::reference_widget {
    enum class EntryOuput {
        UNMODIFIED,
        MODIFIED,
        SUBMITTED,
        CANCELLED
    };

    template <typename Char>
    EntryOuput entry(Context& ctx, const char* title, const primitive::Rectangle& boundingArea,
                     const primitive::Color& primaryColor, std::basic_string<Char>& value);
}

#include "reference_widget_entry.tcc"

#endif //REIG_REFERENCE_WIDGET_ENTRY_H
