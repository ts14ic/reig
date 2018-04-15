#include "StopWatch.h"

#include <reig/context.h>
#include <reig/reference_widget.h>
#include <reig/reference_widget_list.h>
#include <reig/reference_widget_entry.h>

#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>

#include <iostream>
#include <iomanip>

using namespace std::string_literals;
using namespace reig::primitive::colors::literals;
using namespace reig::primitive::colors::operators;
namespace widget = reig::reference_widget;
namespace primitive = reig::primitive;
namespace colors = reig::primitive::colors;

struct Sdl {
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    int width = 1366;
    int height = 768;
};

struct Font {
    reig::Context::FontData data;
    SDL_Texture* tex = nullptr;
    int id = 100;
};

struct Gui {
    reig::Context ctx;
    Font font;
};

class Main {
public:
    Main() {
        setup_sdl();
        setup_reig();
    }

    int run() {
        int previousFrameTimestamp;
        while(true) {
            previousFrameTimestamp = SDL_GetTicks();
            mGui.ctx.start_new_frame();

            if(!handle_input_events()) {
                break;
            }

            draw_gui();

            render_frame(previousFrameTimestamp);
        }

        return 0;
    }

private:
    void setup_sdl() {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
        mSdl.window = SDL_CreateWindow(
                "reig SDL testbed",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, //NOLINT
                mSdl.width, mSdl.height,
                SDL_WINDOW_SHOWN
        );
        mSdl.renderer = SDL_CreateRenderer(
                mSdl.window, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
    }

    void setup_reig() {
        mGui.ctx.set_render_handler(&gui_handler);
        mGui.ctx.set_user_ptr(this);

        mGui.font.data = mGui.ctx.set_font("/usr/share/fonts/TTF/impact.ttf", mGui.font.id, 32.f);
        auto* surf = SDL_CreateRGBSurfaceFrom(
                mGui.font.data.bitmap.data(), mGui.font.data.width, mGui.font.data.height, 8, mGui.font.data.width,
                0, 0, 0, 0
        );
        SDL_Color colors[256];
        {
            Uint8 v = 0;
            for(SDL_Color& color: colors) {
                color.r = color.g = color.b = 0xFF;
                color.a = v++;
            }
        }
        if(SDL_SetPaletteColors(surf->format->palette, colors, 0, 256) < 0) {
            std::cerr << "Failed to set palette\n";
            exit(1);
        }
        mGui.font.tex = SDL_CreateTextureFromSurface(mSdl.renderer, surf);
        if(SDL_SetTextureBlendMode(mGui.font.tex, SDL_BLENDMODE_BLEND) < 0) {
            std::cerr << "Failed to set texture blend mode: ["<< SDL_GetError() << "]\n";
            exit(0);
        }
    }

    static void gui_handler(reig::Context::DrawData const& drawData, std::any& userPtr) {
        namespace colors = reig::primitive::colors;
        auto* self = std::any_cast<Main*>(userPtr);

        for(auto const& fig : drawData) {
            auto const& vertices = fig.vertices();
            auto const& indices = fig.indices();
            auto number = indices.size();

            if(number % 3 != 0) {
                continue;
            }

            if(fig.texture() == 0) {
                for(auto i = 0ul; i < number; i += 3) {
                    filledTrigonColor(
                        self->mSdl.renderer,
                        vertices[indices[i  ]].position.x,
                        vertices[indices[i  ]].position.y,
                        vertices[indices[i+1]].position.x,
                        vertices[indices[i+1]].position.y,
                        vertices[indices[i+2]].position.x,
                        vertices[indices[i+2]].position.y,
                        colors::to_uint(vertices[i].color)
                    );
                }
            }
            else if(fig.texture() == self->mGui.font.id) {
                SDL_Rect src;
                src.x = vertices[0].texCoord.x * self->mGui.font.data.width;
                src.y = vertices[0].texCoord.y * self->mGui.font.data.height;
                src.w = vertices[2].texCoord.x * self->mGui.font.data.width  - src.x;
                src.h = vertices[2].texCoord.y * self->mGui.font.data.height - src.y;
                SDL_Rect dst;
                dst.x = vertices[0].position.x;
                dst.y = vertices[0].position.y;
                dst.w = vertices[2].position.x - dst.x;
                dst.h = vertices[2].position.y - dst.y;
                SDL_RenderCopy(self->mSdl.renderer, self->mGui.font.tex, &src, &dst);
            }
        }
    }

    bool handle_input_events() {
        for(SDL_Event evt; SDL_PollEvent(&evt);) {
            switch(evt.type) {
                case SDL_QUIT: {
                    return false;
                }

                case SDL_MOUSEMOTION: {
                    mGui.ctx.mouse.place(evt.motion.x, evt.motion.y);
                    break;
                }

                case SDL_MOUSEWHEEL: {
                    mGui.ctx.mouse.scroll(-evt.wheel.y);
                    break;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    if (evt.button.button == SDL_BUTTON_LEFT) {
                        mGui.ctx.mouse.leftButton.press(evt.button.x, evt.button.y);
                    }
                    break;
                }

                case SDL_MOUSEBUTTONUP: {
                    if (evt.button.button == SDL_BUTTON_LEFT) {
                        mGui.ctx.mouse.leftButton.release();
                    }
                    break;
                }

                case SDL_KEYDOWN: {
                    handle_key_event(evt);
                    break;
                }

                default:;
            }
        }
        return true;
    }

    void handle_key_event(SDL_Event& evt) {
        switch (evt.key.keysym.sym) {
            case SDLK_RETURN: {
                mGui.ctx.keyboard.press_special_key(reig::Key::RETURN);
                break;
            }

            case SDLK_BACKSPACE: {
                mGui.ctx.keyboard.press_special_key(reig::Key::BACKSPACE);
                break;
            }

            case SDLK_ESCAPE: {
                mGui.ctx.keyboard.press_special_key(reig::Key::ESCAPE);
                break;
            }

            default: {
                mGui.ctx.keyboard.press_key(evt.key.keysym.sym);
                break;
            }
        }
        if (evt.key.keysym.mod & KMOD_SHIFT) { // NOLINT
            mGui.ctx.keyboard.press_modifier(reig::KeyModifier::SHIFT);
        }
    }

    float winX = 10;
    float winY = 10;

    void draw_gui() {
        primitive::Rectangle rect {40, 0, 100, 30};
        primitive::Color color {120_r, 100_g, 150_b};

        mGui.ctx.start_window(mFpsString.c_str(), winX, winY);
        for(int i = 0; i < 4; ++i) {
            rect.x -= 10; rect.y = 40 * i;
            color = color + 25_r + 25_g;
            std::string title = "some  " + std::to_string(i + 1);
//                if(mGui.ctx.enqueue(widget::textured_button{title.c_str(), rect, mGui.font.id, 0})) {
            if(widget::button{title.c_str(), rect, color}.use(mGui.ctx)) {
                std::cout << "Button " << (i + 1) << ": pressed" << std::endl;
            }
        }

        color = color + 50_g;
        rect.y += 40; rect.width += 50;
        static float sliderValue0 = 20;
//            if (mGui.ctx.enqueue(widget::slider{rect, color, sliderValue0, 20, 40, 5})) {
        if (widget::textured_slider{rect, 0, mGui.font.id, sliderValue0, 20, 40, 5}.use(mGui.ctx)) {
            std::cout << "Slider 1: new value " << sliderValue0 << std::endl;
        }

        color = color + 50_g;
        rect.y += 40;
        rect.width += 50;

        static float sliderValue1 = 5.4f;
        if (widget::slider{rect, color, sliderValue1, 3, 7, 0.1f}.use(mGui.ctx)) {
            std::cout << "Slider 2: new value " << sliderValue1 << std::endl;
        }

        static float sliderValue2 = 0.3f;
        rect.y += 40; rect.width += 50; rect.height += 10;
        if (widget::slider{rect, {220_r, 200_g, 150_b}, sliderValue2, 0.1f, 0.5f, 0.05f}.use(mGui.ctx)) {
            std::cout << "Slider 2: new value " << sliderValue2 << std::endl;
        }

        static bool checkBox1 = false;
        color = color + 15_r - 35_r - 10_b;
        rect.x += 270; rect.width = 40; rect.height = 20;
//            if(mGui.ctx.enqueue(widget::checkbox{rect, color, checkBox1})) {
        if(widget::textured_checkbox{rect, 0, mGui.font.id, checkBox1}.use(mGui.ctx)) {
            std::cout << "Checkbox 1: new value " << checkBox1 << std::endl;
        }

        static bool checkBox2 = true;
        color = color - 100_r + 100_g + 100_b;
        rect.y -= 100; rect.width = rect.height = 50;
        if(widget::checkbox{rect, color, checkBox2}.use(mGui.ctx)) {
            std::cout << "Checkbox 2: new value " << checkBox2 << std::endl;
        }

        static bool checkBox3 = false;
        color = colors::white;
        rect.y += 60; rect.width = rect.height = 25;
        if(widget::checkbox{rect, color, checkBox3}.use(mGui.ctx)) {
            std::cout << "Checkbox 3: new value " << checkBox3 << std::endl;
        }

        static float scroll1 = 0.0f;
        rect.x += 60; rect.y = 5; rect.width = 30; rect.height = 280;
        if(widget::scrollbar{rect, colors::black, scroll1, 1000.0f}.use(mGui.ctx)) {
            std::cout << "Scrolled: " << scroll1 << '\n';
        }

        rect.x = 5; rect.y += 300; rect.width = 280; rect.height = 30;
        widget::scrollbar{rect, colors::black, scroll1, 1000.0f}.use(mGui.ctx);

        rect.y = 5; rect.x += 370; rect.height = 280;

        struct Foo {
            const std::string name;
        };
        static std::vector<Foo> foos {
                {"Zero"}, {"One"}, {"Two"}, {"Three"}, {"Four"}, {"Five"},
                {"Six"}, {"Seven"}, {"Eight"}, {"Nine"}, {"Ten"}
        };
        widget::list(
                "Test", rect, colors::blue, foos,
                [](const Foo& foo) {
                    return foo.name.c_str();
                },
                [](int position, const Foo& foo) {
                    std::cout << "Clicked on " << position << "th foo: " << foo.name << '\n';
                }
        ).use(mGui.ctx);

        rect.y += 300;
        rect.height = 40;

        static std::string input {"Hello )"};

        widget::entry("Entry 2", rect, colors::violet, input, [](const std::string& input) {
            std::cout << "Entry 2: " << input << '\n';
        }).use(mGui.ctx);

        mGui.ctx.end_window();
    }

    void render_frame(int& previousFrameTimestamp) {
        SDL_SetRenderDrawColor(mSdl.renderer, 50, 50, 50, 255);
        SDL_RenderClear(mSdl.renderer);

        auto ticks = SDL_GetTicks() - previousFrameTimestamp;
        if(ticks != 0) {
            ticks = 1000 / ticks;
            mFpsString = std::to_string(ticks) + " FPS";
        }
        mGui.ctx.render_all();

        int mx, my;
        int state = SDL_GetMouseState(&mx, &my);
        if((state & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK) { //NOLINT
            filledCircleRGBA(mSdl.renderer, mx, my, 15, 150, 220, 220, 150);
        }

        SDL_RenderPresent(mSdl.renderer);
    }

private:
    std::string mFpsString;
    Sdl mSdl;
    Gui mGui;
};

int main(int, char*[]) {
    return Main{}.run();
}
