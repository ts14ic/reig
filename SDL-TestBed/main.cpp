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
    reig::Context::FontBitmap data;
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
        mGui.ctx.set_config(reig::Config::builder()
                                    .fontBitmapSizes(1024, 1024)
                                    .windowColors(colors::red | 200_a, colors::white | 50_a)
                                    .build());
        mGui.ctx.set_render_handler(&gui_handler);
        mGui.ctx.set_user_ptr(this);

        mGui.font.data = mGui.ctx.set_font("/usr/share/fonts/TTF/impact.ttf", mGui.font.id, 20.f);
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

    struct Window {
        std::string title;
        float x = 0;
        float y = 0;
    };

    void start_window(Window& window) {
        mGui.ctx.start_window(window.title.c_str(), window.x, window.y);
    }

    float mFontScale = 1.0f;

    void draw_gui() {
        widget::label{mFpsString.c_str(), {0, 0, 128, 32}, reig::text::Alignment::CENTER}.use(mGui.ctx);

        widget::slider{{350, 680, 300, 20}, colors::green, mFontScale, 0.0f, 2.0f, 0.05f}.use(mGui.ctx);
        Rectangle rect{0, 700, 1000, 40};
        widget::label{"The quick brown fox jumps over the lazy dog", rect,
                      reig::text::Alignment::CENTER, mFontScale}.use(mGui.ctx);

        draw_buttons();
        draw_checkboxes();
        draw_sliders();
        draw_text_entries();
        draw_list();
    }

    Window mButtonsWindow {"Buttons", 30, 30};
    Window mCheckboxesWindow {"Checkboxes", 200, 30};
    Window mSlidersWindow {"Sliders", 430, 30};
    Window mTextEntryWindow {"Text entry", 30, 250};
    Window mListWindow {"List", 800, 30};

    void draw_buttons() {
        primitive::Rectangle rect{40, 0, 100, 30};
        primitive::Color color{120_r, 100_g, 150_b};

        start_window(mButtonsWindow);
        for (int i = 0; i < 4; ++i) {
            rect.x -= 10;
            rect.y = 40 * i;
            color = color + 25_r + 25_g;
            std::string title = "some  " + std::to_string(i + 1);
//            if(widget::textured_button{title.c_str(), rect, mGui.font.id, 0}.use(mGui.ctx)) {
            if (widget::button{title.c_str(), rect, color}.use(mGui.ctx)) {
                std::cout << "Button " << (i + 1) << ": pressed" << std::endl;
            }
        }
    }

    void draw_checkboxes() {
        start_window(mCheckboxesWindow);

        primitive::Color color{150_r, 115_g, 140_b};
        primitive::Rectangle rect = {0, 0, 40, 20};

        static bool checkBox1 = false;
        if(widget::checkbox{rect, color, checkBox1}.use(mGui.ctx)) {
//        if(widget::textured_checkbox{rect, 0, mGui.font.id, checkBox1}.use(mGui.ctx)) {
            std::cout << "Checkbox 1: new value " << checkBox1 << std::endl;
        }

        color = color - 100_r + 100_g + 100_b;
        rect = {rect.x + 80, rect.y + 50, 50, 50};
        static bool checkBox2 = true;
        if(widget::checkbox{rect, color, checkBox2}.use(mGui.ctx)) {
            std::cout << "Checkbox 2: new value " << checkBox2 << std::endl;
        }

        color = colors::white;
        rect = {rect.x + 80, rect.y - 50, 25, 25};
        static bool checkBox3 = false;
        if(widget::checkbox{rect, color, checkBox3}.use(mGui.ctx)) {
            std::cout << "Checkbox 3: new value " << checkBox3 << std::endl;
        }
    }

    void draw_sliders() {
        start_window(mSlidersWindow);

        primitive::Rectangle rect{50, 0, 150, 30};
        primitive::Color color{120_r, 150_g, 150_b};

        static float sliderValue0 = 20;
        if (widget::slider{rect, color, sliderValue0, 20, 40, 5}.use(mGui.ctx)) {
//        if (widget::textured_slider{rect, 0, mGui.font.id, sliderValue0, 20, 40, 5}.use(mGui.ctx)) {
            std::cout << "Slider 1: new value " << sliderValue0 << std::endl;
        }

        rect = {rect.x, rect.y + 40, rect.width + 50, rect.height};
        color = color + 50_g;
        static float sliderValue1 = 5.4f;
        if (widget::slider{rect, color, sliderValue1, 3, 7, 0.1f}.use(mGui.ctx)) {
            std::cout << "Slider 2: new value " << sliderValue1 << std::endl;
        }

        rect = {rect.x, rect.y + 40, rect.width + 80, rect.height + 10};
        static float sliderValue2 = 0.3f;
        if (widget::slider{rect, {220_r, 200_g, 150_b}, sliderValue2, 0.1f, 0.5f, 0.05f}.use(mGui.ctx)) {
            std::cout << "Slider 2: new value " << sliderValue2 << std::endl;
        }

        rect = {0, 5, 30, 200};
        static float scrollValue0 = 0.0f;
        if (widget::scrollbar{rect, colors::black, scrollValue0, 1000.0f}.use(mGui.ctx)) {
            std::cout << "Scrolled: " << scrollValue0 << '\n';
        }

        rect = {rect.x + 50, rect.y + 150, rect.width + 250, 30};
        widget::scrollbar{rect, colors::black, scrollValue0, 1000.0f}.use(mGui.ctx);
    }

    void draw_text_entries() {
        start_window(mTextEntryWindow);
        primitive::Rectangle rect {0, 0, 300, 40};
        primitive::Color color {120_r, 100_g, 150_b};

        static std::string entry1;
        widget::entry("Entry 1", rect, colors::violet, entry1, [](const std::string& input) {
            std::cout << "Entry 1: " << input << '\n';
        }).use(mGui.ctx);

        rect.y += 50;
        static std::string entry2;
        widget::entry("Entry 2", rect, colors::black, mTextEntryWindow.title, [](const std::string& input) {
            std::cout << "Entry 2: " << input << '\n';
        }).use(mGui.ctx);
    }

    void draw_list() {
        // assume these are members
        struct Foo {
            const std::string name;
        };
        static std::vector<Foo> foos {
                {"Zero"}, {"One"}, {"Two"}, {"Three"}, {"Four"}, {"Five"},
                {"Six"}, {"Seven"}, {"Eight"}, {"Nine"}, {"Ten"}
        };
        static std::string itemName;

        start_window(mListWindow);

        primitive::Rectangle rect = {0, 0, 280, 280};
        widget::list(
                "Test", rect, colors::blue, foos,
                [](const Foo& foo) {
                    return foo.name.c_str();
                },
                [](int position, const Foo& foo) {
                    std::cout << "Clicked on " << position << "th foo: " << foo.name << '\n';
                }
        ).use(mGui.ctx);

        rect = {rect.x, rect.y + rect.height + 10, rect.width - 120, 40};
        widget::entry("Add item", rect, colors::darkGrey, itemName, [](const std::string&) {}).use(mGui.ctx);

        rect = {rect.x + rect.width + 5, rect.y, 30, 40};
        if (widget::button{"+", rect, colors::green}.use(mGui.ctx)) {
            if (!itemName.empty()) {
                foos.push_back(Foo{itemName});
            }
        }

        rect = {rect.x + rect.width + 5, rect.y, 35, 40};
        if (widget::button{"Cl", rect, colors::violet}.use(mGui.ctx)) {
            itemName.clear();
        }

        rect = {rect.x + rect.width + 5, rect.y, 35, 40};
        if (widget::button{"+", rect, colors::yellow}.use(mGui.ctx)) {
            foos.push_back(Foo{std::to_string(foos.size())});
        }

        rect = {0, rect.y + rect.height + 5, 280, 40};
        if (widget::button{"Remove last", rect, colors::red}.use(mGui.ctx)) {
            if (!foos.empty()) {
                foos.pop_back();
            }
        }
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
