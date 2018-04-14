#ifndef REIG_KEYBOARD_H
#define REIG_KEYBOARD_H

#include <vector>

namespace reig::detail {
    class Keyboard {
    public:
        Keyboard() = default;

        Keyboard(const Keyboard& other) = delete;

        Keyboard(Keyboard&& other) = delete;

        Keyboard& operator=(const Keyboard& other) = delete;

        Keyboard& operator=(Keyboard&& other) = delete;

        void press(int keycode);

        void release(int keycode);

        void reset();

    private:
        std::vector<int> mKeyCodes;
    };
}

#endif //REIG_KEYBOARD_H
