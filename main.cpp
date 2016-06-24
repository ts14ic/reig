#include "reig.h"
#include <iostream>
#include <UnitTest++/UnitTest++.h>

SUITE(Basic) {
    TEST(Button) {
        namespace gui = reig;
        gui::Context ctx;
        auto btnRect  = gui::Rectangle{10, 10, 100, 30};
        auto btnColor = gui::Color{100, 255, 100};
        
        // mouse is at [0,0]
        CHECK(! ctx.button(nullptr, btnRect, btnColor));
        
        // mouse completely inside
        ctx.move_mouse(20, 30);
        CHECK(ctx.button(nullptr, btnRect, btnColor));
        
        // mouse moved on border
        ctx.move_mouse(0, 20);
        CHECK(! ctx.button(nullptr, btnRect, btnColor));
        
        // mouse inside again
        ctx.place_mouse(11, 11);
        CHECK(ctx.button(nullptr, btnRect, btnColor));
        
        // mouse not inside by x
        ctx.place_mouse(100, 40);
        CHECK(! ctx.button(nullptr, btnRect, btnColor));
    }
}

int main(int, char*[]) {
    return UnitTest::RunAllTests();
}
