#ifndef REIG_FOCUS_H
#define REIG_FOCUS_H

#include "fwd.h"

namespace reig {
    class [[deprecated]] Focus {
    public:
        [[deprecated]]
        Focus() = default;

        [[deprecated]]
        Focus(const Focus& other) = delete;

        [[deprecated]]
        Focus(Focus&& other) noexcept = delete;

        [[deprecated]]
        Focus& operator=(const Focus& other) = delete;

        [[deprecated]]
        Focus& operator=(Focus&& other) noexcept = delete;

        [[deprecated]]
        struct FocusId;

        [[deprecated]]
        FocusId create_id();

        [[deprecated]]
        bool claim(const FocusId& focusId);

        [[deprecated]]
        void release(const FocusId& focusId);

        [[deprecated]]
        bool handle(const FocusId& focusId, bool claiming);

        struct FocusId {
        private:
            [[deprecated]]
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
