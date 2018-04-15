#include "focus.h"

namespace reig {
    int Focus::create_id() {
        return ++mFocusCounter;
    }

    bool Focus::is_focused(int focusId) const {
        return mCurrentFocus == focusId;
    }

    bool Focus::claim(int focusId) {
        if (focusId > mCurrentFocus) {
            mCurrentFocus = focusId;
        }
        return mCurrentFocus == focusId;
    }

    void Focus::release(int focusId) {
        if (mCurrentFocus == focusId) {
            mCurrentFocus = 0;
        }
    }

    void Focus::reset_counter() {
        mFocusCounter = 0;
    }
}