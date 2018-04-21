#ifndef REIG_REFERENCE_WIDGET_ENTRY_H
#define REIG_REFERENCE_WIDGET_ENTRY_H

#include "context.h"
#include <string>

namespace reig::reference_widget {
    enum class EntryOuput {
        textUntouched,
        textModified,
        textSubmitted,
        textCancelled
    };

    template <typename Char>
    EntryOuput entry(Context& ctx, const char* title, const Rectangle& boundingArea, const Color& primaryColor,
                     std::basic_string<Char>& value);
}

#include "reference_widget_entry.tcc"

#endif //REIG_REFERENCE_WIDGET_ENTRY_H
