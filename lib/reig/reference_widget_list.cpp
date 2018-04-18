#include "reference_widget_list.h"
#include <unordered_map>

namespace reig::reference_widget {
    float& detail::get_scroll_value(const char* title) {
        static std::unordered_map<const char*, float> scrollValues;
        return scrollValues[title];
    }
}
