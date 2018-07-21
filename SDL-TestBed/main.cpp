#include <reig/context.h>
#include <reig/reference_widget.h>
#include <reig/reference_widget_list.h>
#include <reig/reference_widget_entry.h>
#include <SDL2_gfxPrimitives.h>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
#include <chrono>

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
        namespace chrono = std::chrono;
        std::vector<long> measurements;
        mFpsString = "? us per frame";

        while(true) {
            auto start_timestamp = chrono::steady_clock::now();
            mGui.ctx.start_frame();

            if(!handle_input_events()) {
                break;
            }

            draw_gui();

            auto end_timestamp = chrono::steady_clock::now();
            auto ticks = chrono::duration_cast<chrono::microseconds>(end_timestamp - start_timestamp).count();
            measurements.push_back(ticks);

            if (measurements.size() >= 5) {
                double count = measurements.size();
                double avg_us_per_frame = 0.0f;
                for (auto measurement : measurements) {
                    avg_us_per_frame += measurement / count;
                }
                measurements.clear();
                mFpsString = str(boost::format("%.0f us per frame") % avg_us_per_frame);
            }
            render_frame();
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
        mGui.ctx.set_config(reig::Config()
                                    .font_bitmap_size(1024, 1024)
                                    .window_colors(colors::kRed | 200_a, colors::kMediumGrey | 150_a));
        mGui.ctx.set_render_handler(&gui_handler);
        mGui.ctx.set_user_ptr(this);

        mGui.font.data = mGui.ctx.set_font("/usr/share/fonts/TTF/DejaVuSans.ttf", mGui.font.id, 20.f);
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

    void static gui_handler(const reig::DrawData& drawData, std::any userPtr) {
        auto* self = std::any_cast<Main*>(userPtr);
        namespace colors = reig::primitive::colors;

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
                        static_cast<Sint16>(vertices[indices[i  ]].position.x),
                        static_cast<Sint16>(vertices[indices[i  ]].position.y),
                        static_cast<Sint16>(vertices[indices[i+1]].position.x),
                        static_cast<Sint16>(vertices[indices[i+1]].position.y),
                        static_cast<Sint16>(vertices[indices[i+2]].position.x),
                        static_cast<Sint16>(vertices[indices[i+2]].position.y),
                        colors::to_uint(vertices[i].color)
                    );
                }
            }
            else if(fig.texture() == self->mGui.font.id) {
                SDL_Rect src;
                src.x = static_cast<int>(vertices[0].texCoord.x * self->mGui.font.data.width);
                src.y = static_cast<int>(vertices[0].texCoord.y * self->mGui.font.data.height);
                src.w = static_cast<int>(vertices[2].texCoord.x * self->mGui.font.data.width  - src.x);
                src.h = static_cast<int>(vertices[2].texCoord.y * self->mGui.font.data.height - src.y);
                SDL_Rect dst;
                dst.x = static_cast<int>(vertices[0].position.x);
                dst.y = static_cast<int>(vertices[0].position.y);
                dst.w = static_cast<int>(vertices[2].position.x - dst.x);
                dst.h = static_cast<int>(vertices[2].position.y - dst.y);
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
                        mGui.ctx.mouse.left_button.press(evt.button.x, evt.button.y);
                    }
                    break;
                }

                case SDL_MOUSEBUTTONUP: {
                    if (evt.button.button == SDL_BUTTON_LEFT) {
                        mGui.ctx.mouse.left_button.release();
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
                mGui.ctx.keyboard.press_special_key(reig::Key::kReturn);
                break;
            }

            case SDLK_BACKSPACE: {
                mGui.ctx.keyboard.press_special_key(reig::Key::kBackspace);
                break;
            }

            case SDLK_ESCAPE: {
                mGui.ctx.keyboard.press_special_key(reig::Key::kEscape);
                break;
            }

            default: {
                mGui.ctx.keyboard.press_key(evt.key.keysym.sym);
                break;
            }
        }
        if (evt.key.keysym.mod & KMOD_SHIFT) { // NOLINT
            mGui.ctx.keyboard.press_modifier(reig::KeyModifier::kShift);
        }
    }

    struct Window {
        Window(const char* id, std::string title, float x, float y) : title{move(title)}, id{id}, x{x}, y{y} {}
        Window(std::string title, float x, float y) : title{move(title)}, x{x}, y{y} {}

        std::string title;
        const char* id = nullptr;
        float x = 0;
        float y = 0;
    };

    void start_window(Window& window) {
        if (window.id) {
            mGui.ctx.start_window(window.id, window.title.c_str(), window.x, window.y);
        } else {
            mGui.ctx.start_window(window.title.c_str(), window.x, window.y);
        }
    }

    float mFontScale = 1.0f;

    void draw_gui() {
        widget::label(mGui.ctx, mFpsString.c_str(), {0, 0, 128, 32}, reig::text::Alignment::kRight);

        widget::slider(mGui.ctx, {350, 680, 300, 20}, colors::kGreen, mFontScale, 0.0f, 2.0f, 0.05f);
        primitive::Rectangle rect{0, 700, 1000, 40};
        widget::label(mGui.ctx, "The quick brown fox jumps over the lazy dog", rect,
                      reig::text::Alignment::kCenter, mFontScale);

        draw_buttons();
        draw_checkboxes();
        draw_sliders();
        draw_text_entries();
        draw_list();
    }

    Window mButtonsWindow {"Buttons", 30, 30};
    Window mCheckboxesWindow {"Checkboxes", 200, 30};
    Window mSlidersWindow {"Sliders", 430, 30};
    Window mTextEntryWindow {"entry_window", "Text entries", 30, 250};
    Window mListWindow {"List", 800, 30};

    void draw_buttons() {
        primitive::Rectangle rect{40, 0, 100, 30};
        primitive::Color color{120_r, 100_g, 150_b};

        start_window(mButtonsWindow);
        for (int i = 0; i < 4; ++i) {
            rect = {rect.x - 10, 40.0f * i, rect.width, rect.height};
            color = color + 25_r + 25_g;

            std::string title = boost::str(boost::format("some %d") % (i + 1));

            if (widget::button(mGui.ctx, title.c_str(), rect, color)) {
                std::cout << boost::format("Button {%s} pressed\n") % title;
            }
        }
    }

    void draw_checkboxes() {
        start_window(mCheckboxesWindow);

        primitive::Color color{150_r, 115_g, 140_b};
        primitive::Rectangle rect = {0, 0, 40, 20};

        static bool checkBox1 = false;
//        if(widget::textured_checkbox{rect, 0, mGui.font.id, checkBox1}.use(mGui.ctx)) {
        if (widget::checkbox(mGui.ctx, rect, color, checkBox1)) {
            widget::label(mGui.ctx, "checked", {rect.x + rect.width + 5, rect.y, 80, rect.height});
        }

        color = color - 100_r + 100_g + 100_b;
        rect = {rect.x + 80, rect.y + 50, 50, 50};
        static bool checkBox2 = true;
        if (widget::checkbox(mGui.ctx, rect, color, checkBox2)) {
            widget::label(mGui.ctx, "o!", {rect.x + rect.width + 5, rect.y, 50, rect.height}, reig::text::Alignment::kLeft);
        } else {
            widget::label(mGui.ctx, "o-O-o!", {rect.x + rect.width + 5, rect.y, 50, rect.height}, reig::text::Alignment::kLeft);
        }

        color = colors::kWhite;
        rect = {rect.x + 80, rect.y - 50, 25, 25};
        static bool checkBox3 = false;
        widget::checkbox(mGui.ctx, rect, color, checkBox3);
    }

    void draw_sliders() {
        start_window(mSlidersWindow);

        primitive::Rectangle rect{50, 0, 150, 30};
        primitive::Color color{120_r, 150_g, 150_b};

        static float sliderValue0 = 20;
//        if (widget::textured_slider{rect, 0, mGui.font.id, sliderValue0, 20, 40, 5}.use(mGui.ctx)) {
        if (widget::slider(mGui.ctx, rect, color, sliderValue0, 20, 40, 5)) {
            std::cout << boost::format("Slider 0: new value %.2f\n") % sliderValue0;
        }

        rect = {rect.x, rect.y + 40, rect.width + 50, rect.height};
        color = color + 50_g;
        static float sliderValue1 = 5.4f;
        if (widget::slider(mGui.ctx, rect, color, sliderValue1, 3, 7, 0.1f)) {
            std::cout << "Slider 1: new value " << sliderValue1 << std::endl;
        }

        rect = {rect.x, rect.y + 40, rect.width + 80, rect.height + 10};
        static float sliderValue2 = 0.3f;
        if (widget::slider(mGui.ctx, rect, {220_r, 200_g, 150_b}, sliderValue2, 0.1f, 0.5f, 0.05f)) {
            std::cout << "Slider 2: new value " << sliderValue2 << std::endl;
        }

        static float scrollValue0 = 0.0f;

        rect = {0, 5, 30, 200};
        widget::scrollbar(mGui.ctx, rect, colors::kBlack, scrollValue0, 1000.0f);

        rect = {rect.x + 50, rect.y + 150, rect.height, rect.width};
        widget::scrollbar(mGui.ctx, rect, colors::kBlack, scrollValue0, 1000.0f);
    }

    void draw_text_entries() {
        start_window(mTextEntryWindow);
        primitive::Rectangle rect {0, 0, 300, 40};

        using reig::reference_widget::EntryOutput;

        static std::string entry1;
        if (widget::entry(mGui.ctx, "Entry 1", rect, colors::kViolet, entry1) == EntryOutput::kModified) {
            std::cout << "Entry 1: " << entry1 << '\n';
        }

        rect.y += 50;
        if (widget::entry(mGui.ctx, "Entry 2", rect, colors::kBlack, mTextEntryWindow.title) == EntryOutput::kModified) {
            std::cout << "Entry 2: " << mTextEntryWindow.title << '\n';
        }
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
        static bool listShown = false;

        start_window(mListWindow);

        widget::label(mGui.ctx, "Show list:", {0, 0, 80, 30}, reig::text::Alignment::kLeft);
        if (widget::checkbox(mGui.ctx, {85, 0, 30, 30}, colors::kWhite, listShown)) {
            primitive::Rectangle rect = {0, 35, 280, 280};
            widget::list(mGui.ctx, "Test", rect, colors::kBlue, foos,
                         [](const Foo& foo) {
                             return foo.name.c_str();
                         },
                         [](int position, const Foo& foo) {
                             std::cout << "Clicked on " << position << "th foo: " << foo.name << '\n';
                         });

            using reig::reference_widget::EntryOutput;

            rect = {rect.x, rect.y + rect.height + 10, rect.width - 120, 40};
            switch (widget::entry(mGui.ctx, "Add item", rect, colors::kDarkGrey, itemName)) {
                case EntryOutput::kSubmitted: {
                    foos.push_back(Foo{itemName});
                    break;
                }

                case EntryOutput::kCancelled: {
                    itemName.clear();
                    break;
                }

                default:
                    break;
            }

            rect = {rect.x + rect.width + 5, rect.y, 35, 40};
            if (widget::button(mGui.ctx, "+", rect, colors::kYellow)) {
                foos.push_back(Foo{std::to_string(foos.size())});
            }

            rect = {0, rect.y + rect.height + 5, 280, 40};
            if (widget::button(mGui.ctx, "Remove last", rect, colors::kRed)) {
                if (!foos.empty()) {
                    foos.pop_back();
                }
            }
        }
    }

    void render_frame() {
        SDL_SetRenderDrawColor(mSdl.renderer, 50, 50, 50, 255);
        SDL_RenderClear(mSdl.renderer);

        mGui.ctx.end_frame();

        int mx, my;
        int state = SDL_GetMouseState(&mx, &my);
        if((state & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK) { //NOLINT
            filledCircleRGBA(mSdl.renderer, static_cast<Sint16>(mx), static_cast<Sint16>(my),
                             15, 150, 220, 220, 150);
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
