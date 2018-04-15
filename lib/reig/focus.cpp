#include "focus.h"

namespace reig {
    int Focus::create_id() {
        return ++mFocusCounter;
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

    bool Focus::handle(int focusId, bool claiming) {
        if (claiming) {
            return claim(focusId);
        } else {
            release(focusId);
            return false;
        }
    }

    void Focus::reset_counter() {
        mFocusCounter = 0;
    }
}