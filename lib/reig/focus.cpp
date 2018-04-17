#include "focus.h"

namespace reig {
    Focus::FocusId Focus::create_id() {
        return ++mFocusCounter;
    }

    bool Focus::claim(const FocusId& focusId) {
        if (focusId.mId > mCurrentFocus.mId && !mFocusedWindow) {
            mCurrentFocus = focusId;
        }
        return mCurrentFocus.mId == focusId.mId;
    }

    void Focus::release(const FocusId& focusId) {
        if (mCurrentFocus.mId == focusId.mId) {
            mCurrentFocus = 0;
        }
    }

    bool Focus::handle(const FocusId& focusId, bool claiming) {
        if (claiming) {
            return claim(focusId);
        } else {
            release(focusId);
            return false;
        }
    }

    void Focus::reset_counter() {
        // FIXME: I'm a dirty hack
        if (mCurrentFocus.mId > mFocusCounter) {
            mCurrentFocus = 0;
        }
        mFocusCounter = 0;
    }

    bool Focus::claim_for_window(const char* window) {
        if (!mFocusedWindow) {
            mFocusedWindow = window;
        }
        return mFocusedWindow == window;
    }

    void Focus::release_from_window(const char* window) {
        if (mFocusedWindow == window) {
            mFocusedWindow = nullptr;
        }
    }
}