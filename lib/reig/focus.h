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

        struct FocusId;

        FocusId create_id();

        bool claim(const FocusId& focusId);

        void release(const FocusId& focusId);

        bool handle(const FocusId& focusId, bool claiming);

        struct FocusId {
        private:
            /*implicit*/ FocusId(int id) { // NOLINT
                mId = id;
            };

            friend Focus;
            int mId;
        };

    private:
        friend Context;

        void reset_counter();

        bool claim_for_window(const char* window);

        void release_from_window(const char* window);

        const char* mFocusedWindow = nullptr;
        FocusId mCurrentFocus = 0;
        int mFocusCounter = 0;
    };
}

#endif //REIG_FOCUS_H
