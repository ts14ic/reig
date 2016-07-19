#include "StopWatch.h"
#include <SDL_timer.h>

StopWatch::StopWatch()
: _startedAt{0ul}
{
}

void StopWatch::start() {
    _startedAt = SDL_GetTicks();
}

unsigned StopWatch::get_ticks() const {
    return SDL_GetTicks() - _startedAt;
}

bool StopWatch::passed(unsigned ticks) const {
    return get_ticks() >= ticks;
}
