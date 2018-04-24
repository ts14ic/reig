#include "reference_widget_list.h"
#include <unordered_map>

namespace reig::reference_widget {
    float& detail::get_scroll_value(const void* scrollbar) {
        static std::unordered_map<const void*, float> scroll_values;
        return scroll_values[scrollbar];
    }
}
