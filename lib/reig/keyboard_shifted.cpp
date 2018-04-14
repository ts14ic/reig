#include "keyboard.h"
#include <cctype>
#include <unordered_map>

namespace reig::detail {
    int shift_to_upper(int ch) {
        static std::unordered_map<int, int> shifted{
                {'1', '!'},
                {'2', '@'},
                {'3', '#'},
                {'4', '$'},
                {'5', '%'},
                {'6', '^'},
                {'7', '&'},
                {'8', '*'},
                {'9', '('},
                {'0', ')'},
                {'`', '~'},
                {'-', '_'},
                {'=', '+'},
                {'\\', '|'},
                {'[', '{'},
                {']', '}'},
                {';', ':'},
                {'\'', '"'},
                {',', '<'},
                {'.', '>'},
                {'/', '?'},
        };
        auto it = shifted.find(ch);
        if (it != shifted.end()) {
            return it->second;
        }
        return std::toupper(static_cast<unsigned char>(ch));
    }
}