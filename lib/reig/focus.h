#ifndef REIG_FOCUS_H
#define REIG_FOCUS_H

#include "fwd.h"

namespace reig {
    class Focus {
    public:
        Focus() = default;

        Focus(const Focus& other) = delete;

        Focus(Focus&& other) noexcept = delete;

        Focus& operator=(const Focus& other) = delete;

        Focus& operator=(Focus&& other) noexcept = delete;

        int create_id();

        bool claim(int focusId);

        void release(int focusId);

        bool handle(int focusId, bool claiming);

    private:
        friend Context;

        void reset_counter();

        bool claim_for_window(const char* window);

        void release_from_window(const char* window);

        const char* mFocusedWindow = nullptr;
        int mCurrentFocus = 0;
        unsigned mFocusCounter = 0;
    };
}

#endif //REIG_FOCUS_H
