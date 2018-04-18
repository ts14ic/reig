#include "focus.h"

namespace reig {
    FocusCallback::FocusCallback(const primitive::Rectangle& zone, FocusAreaCallback_t callback)
            : rect(zone), callback(move(callback)) {}
}