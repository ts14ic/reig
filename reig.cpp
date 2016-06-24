#include "reig.h"

bool reig::helpers::in_box(Point const& pt, Rectangle const& box) {
    return between(pt.x, box.x, box.x + box.w) && between(pt.y, box.y, box.y + box.h);
}

void reig::Context::start_new_frame() {
    _mousePrevPos = _mouseCurrPos;
    _mouseClicked = false;
}

void reig::Context::move_mouse(float_t difx, float_t dify) {
    _mouseCurrPos.x += difx;
    _mouseCurrPos.y += dify;
}

void reig::Context::place_mouse(float_t x, float_t y) {
    _mouseCurrPos.x = x;
    _mouseCurrPos.y = y;
}

void reig::Context::click_mouse(bool clicked) {
    _mouseClicked = clicked;
}

bool reig::Context::button(Rectangle aRect, Color aColor) {
    bool inBox = in_box(_mouseCurrPos, aRect);
    return _mouseClicked && inBox;
}
